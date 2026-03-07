#include <parser.h>
#include <stdlib.h>
#include <stdio.h>

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct ast_node *ast_node_alloc(u0)
{
  struct ast_node *n = malloc(sizeof(struct ast_node));

  if (NULL == n)
    parser_die(NULL, 1, "malloc() failed in ast_node_alloc()");

  return n;
}

struct ast_node *ast_node_create(enum ast_node_type type)
{
  struct ast_node  *n  = ast_node_alloc();
  struct ast_node **ch = malloc(AST_INIT_CAPACITY * sizeof(struct ast_node *));

  if (NULL == ch)
  {
    free(n);
    parser_die(NULL, 1, "malloc() failed in ast_node_create()");
  }

  *n = (struct ast_node)
  {
    .type      = type,
    .children  = ch,
    .children_size = 0,
    .capacity  = AST_INIT_CAPACITY,
  };

  return n;
}

u0 ast_node_free(struct ast_node **n)
{
  if (NULL == n || NULL == *n) return;

  for (u32 i = 0; i < (*n)->children_size; i++)
    ast_node_free(&(*n)->children[i]);

  free((*n)->children);
  free(*n);
  *n = NULL;
}

/* ============================================================
 *  Children management
 * ============================================================ */

static u0 ast_node_expand(struct ast_node *n)
{
  u32               new_capacity = n->capacity * 2;
  struct ast_node **new_children = realloc(n->children, new_capacity * sizeof(struct ast_node *));

  if (NULL == new_children)
    parser_die(NULL, 1, "realloc() failed in ast_node_expand()");

  n->children = new_children;
  n->capacity = new_capacity;
}

u0 ast_node_push(struct ast_node *parent, struct ast_node *child)
{
  if (NULL == parent || NULL == child) return;

  if (parent->children_size >= parent->capacity)
    ast_node_expand(parent);

  parent->children[parent->children_size++] = child;
}

/* ============================================================
 *  Convenience constructors
 * ============================================================ */

struct ast_node *ast_node_instr(enum rv32i_instr instr)
{
  struct ast_node *n = ast_node_create(AST_INSTR);
  n->as_instr.instr  = instr;
  return n;
}

struct ast_node *ast_node_label(const char *name)
{
  struct ast_node *n = ast_node_create(AST_LABEL);
  n->as_label.name   = name;
  n->as_label.addr   = 0;
  return n;
}

struct ast_node *ast_node_label_ref(const char *name)
{
  struct ast_node *n   = ast_node_create(AST_LABEL_REF);
  n->as_label_ref.name = name;
  return n;
}

struct ast_node *ast_node_section(u32 addr)
{
  struct ast_node *n = ast_node_create(AST_SECTION);
  n->as_section.addr = addr;
  return n;
}

struct ast_node *ast_node_reg(u8 reg)
{
  struct ast_node *n = ast_node_create(AST_REG);
  n->as_reg.reg      = reg;
  return n;
}

struct ast_node *ast_node_imm(i32 value)
{
  struct ast_node *n  = ast_node_create(AST_IMM);
  n->as_imm.value     = value;
  return n;
}

/* ============================================================
 *  Parser state
 * ============================================================ */

struct parser
{
  struct token_data **tokens;
  u32                 size;
  u32                 pos;
};

static struct parser parser_init(struct lexer *l)
{
  return (struct parser)
  {
    .tokens = l->tokens,
    .size   = l->size,
    .pos    = 0,
  };
}

static struct token_data *parser_peek(struct parser *p)
{
  if (p->pos >= p->size) return NULL;
  return p->tokens[p->pos];
}

static struct token_data *parser_advance(struct parser *p)
{
  if (p->pos >= p->size) return NULL;
  return p->tokens[p->pos++];
}

static i32 parser_at_end(struct parser *p)
{
  return p->pos >= p->size;
}

/* Consumes and returns the current token if it matches `expected`.
   Crashes with a descriptive message otherwise. */
static struct token_data *parser_expect(struct parser *p, enum token_type expected)
{
  if(parser_at_end(p))
    parser_die(NULL, 1,
      "parser_expect: unexpected end of token stream (expected %s)",
      token_to_str(expected));

  struct token_data *t = parser_peek(p);

  if(t->type != expected)
    parser_die(NULL, 1,
      "parser_expect: expected %s but got %s (\"%s\")",
      token_to_str(expected),
      token_to_str(t->type),
      t->value);

  return parser_advance(p);
}

/* ============================================================
 *  Operand parsers
 *
 *  parse_reg        → TOKEN_REGISTER  → AST_REG
 *  parse_imm        → TOKEN_NUMBER    → AST_IMM
 *  parse_label_ref  → TOKEN_LABEL_REF → AST_LABEL_REF
 *  parse_mem        → NUMBER(REG)     → AST_IMM  + AST_REG  (children of instr)
 * ============================================================ */

static struct ast_node *parse_reg(struct parser *p)
{
  struct token_data *t = parser_expect(p, TOKEN_REGISTER);
  i32 idx = rv32_reg_lookup(t->value);
  return ast_node_reg((u8)idx);
}

static struct ast_node *parse_imm(struct parser *p)
{
  struct token_data *t = parser_expect(p, TOKEN_NUMBER);
  i32 val = (i32)atoi(t->value);
  return ast_node_imm(val);
}

static struct ast_node *parse_label_ref(struct parser *p)
{
  struct token_data *t = parser_expect(p, TOKEN_LABEL_REF);
  /* skip the leading '@' */
  return ast_node_label_ref(t->value + 1);
}

/* Parses  imm ( reg )  —  pushes imm then reg as children of instr */
static u0 parse_mem(struct parser *p, struct ast_node *instr)
{
  struct token_data *t;

  t = parser_expect(p, TOKEN_NUMBER);
  ast_node_push(instr, ast_node_imm((i32)atoi(t->value)));

  parser_expect(p, TOKEN_LPAREN);

  t = parser_expect(p, TOKEN_REGISTER);
  ast_node_push(instr, ast_node_reg((u8)rv32_reg_lookup(t->value)));

  parser_expect(p, TOKEN_RPAREN);
}

/* ============================================================
 *  Instruction operand dispatch
 *
 *  The token stream encodes operands according to instruction
 *  type.  We look up the type from the opcode table and parse
 *  the right operand sequence:
 *
 *  R-type :  rd , rs1 , rs2
 *  I-type arithmetic / jump:
 *            rd , rs1 , imm
 *  I-type load:
 *            rd , imm ( rs1 )
 *  S-type :  rs2 , imm ( rs1 )
 *  B-type :  rs1 , rs2 , imm_or_label_ref
 *  U-type :  rd , imm
 *  J-type :  rd , imm_or_label_ref
 *
 *  ECALL / EBREAK have no operands.
 * ============================================================ */

static i32 is_load(enum rv32i_instr idx)
{
  return idx == RV32I_LB  || idx == RV32I_LH  || idx == RV32I_LW
      || idx == RV32I_LBU || idx == RV32I_LHU;
}

/* Peek at the next *meaningful* token to decide imm vs label ref */
static i32 next_is_label_ref(struct parser *p)
{
  /* skip comma if present */
  u32 look = p->pos;
  if (look < p->size && p->tokens[look]->type == TOKEN_COMMA) look++;
  if (look >= p->size) return 0;
  return p->tokens[look]->type == TOKEN_LABEL_REF;
}

static u0 skip_comma(struct parser *p)
{
  if (!parser_at_end(p) && parser_peek(p)->type == TOKEN_COMMA)
    parser_advance(p);
}

static u0 parse_instr_operands(struct parser *p,
                                struct ast_node      *node,
                                enum rv32i_instr      idx)
{
  const struct rv32ii_opcode_entry *e = rv32ii_instr_from_enum(idx);

  /* no operands */
  if (idx == RV32I_ECALL || idx == RV32I_EBREAK)
    return;

  switch (e->type)
  {
    /* ---- R-type : rd , rs1 , rs2 ---- */
    case R_TYPE:
      ast_node_push(node, parse_reg(p));   skip_comma(p);
      ast_node_push(node, parse_reg(p));   skip_comma(p);
      ast_node_push(node, parse_reg(p));
      break;

    /* ---- I-type ---- */
    case I_TYPE:
      if (is_load(idx))
      {
        /* rd , imm ( rs1 ) */
        ast_node_push(node, parse_reg(p));
        skip_comma(p);
        parse_mem(p, node);
      }
      else if (idx == RV32I_JALR)
      {
        /* rd , rs1 , imm */
        ast_node_push(node, parse_reg(p));   skip_comma(p);
        ast_node_push(node, parse_reg(p));   skip_comma(p);
        ast_node_push(node, parse_imm(p));
      }
      else
      {
        /* rd , rs1 , imm  (arithmetic immediates, shifts) */
        ast_node_push(node, parse_reg(p));   skip_comma(p);
        ast_node_push(node, parse_reg(p));   skip_comma(p);
        ast_node_push(node, parse_imm(p));
      }
      break;

    /* ---- S-type : rs2 , imm ( rs1 ) ---- */
    case S_TYPE:
      ast_node_push(node, parse_reg(p));
      skip_comma(p);
      parse_mem(p, node);
      break;

    /* ---- B-type : rs1 , rs2 , imm | @label ---- */
    case B_TYPE:
      ast_node_push(node, parse_reg(p));   skip_comma(p);
      ast_node_push(node, parse_reg(p));   skip_comma(p);
      if (next_is_label_ref(p))
        ast_node_push(node, parse_label_ref(p));
      else
        ast_node_push(node, parse_imm(p));
      break;

    /* ---- U-type : rd , imm ---- */
    case U_TYPE:
      ast_node_push(node, parse_reg(p));   skip_comma(p);
      ast_node_push(node, parse_imm(p));
      break;

    /* ---- J-type : rd , imm | @label ---- */
    case J_TYPE:
      ast_node_push(node, parse_reg(p));   skip_comma(p);
      if (next_is_label_ref(p))
        ast_node_push(node, parse_label_ref(p));
      else
        ast_node_push(node, parse_imm(p));
      break;
  }
}

/* ============================================================
 *  Section header  →  .SECTION  number?
 *
 *  .TEXT         → addr = 0  (default, no number follows)
 *  .DATA         → addr = 0
 *  .ADDR 0x1000  → addr = that number   (TOKEN_NUMBER after section)
 *
 *  We just peek: if the next token is a number we consume it.
 * ============================================================ */

static struct ast_node *parse_section(struct parser *p)
{
  struct token_data *t    = parser_advance(p); /* TOKEN_SECTION */
  u32                addr = 0;

  if (!parser_at_end(p) && parser_peek(p)->type == TOKEN_NUMBER)
  {
    struct token_data *num = parser_advance(p);
    addr = (u32)atoi(num->value);
  }

  (u0)t;   /* section name available as t->value if needed later */
  return ast_node_section(addr);
}

/* ============================================================
 *  Top-level parse
 * ============================================================ */

struct ast_node *parser_build(struct lexer *l)
{
  if (NULL == l) return NULL;

  struct parser    p    = parser_init(l);
  struct ast_node *root = ast_node_create(AST_PROGRAM);

  while (!parser_at_end(&p))
  {
    struct token_data *t = parser_peek(&p);

    switch (t->type)
    {
      /* ---- section directive ---- */
      case TOKEN_SECTION:
      {
        struct ast_node *sec = parse_section(&p);
        ast_node_push(root, sec);
        break;
      }

      /* ---- label definition ---- */
      case TOKEN_LABEL:
      {
        parser_advance(&p);
        /* skip leading '&' */
        struct ast_node *lbl = ast_node_label(t->value + 1);
        ast_node_push(root, lbl);
        break;
      }

      /* ---- instruction ---- */
      case TOKEN_INSTR:
      {
        parser_advance(&p);
        const struct rv32ii_opcode_entry *e = rv32ii_instr_from_label(t->value);
        struct ast_node *instr = ast_node_instr(e->index);
        parse_instr_operands(&p, instr, e->index);
        ast_node_push(root, instr);
        break;
      }

      /* ---- skip anything unexpected without crashing ---- */
      default:
        parser_advance(&p);
        break;
    }
  }

  return root;
}

/* ============================================================
 *  Debug
 * ============================================================ */

static const char *ast_node_type_str(enum ast_node_type t)
{
  switch (t)
  {
    case AST_INSTR:     return "INSTR";
    case AST_LABEL:     return "LABEL";
    case AST_LABEL_REF: return "LABEL_REF";
    case AST_SECTION:   return "SECTION";
    case AST_PROGRAM:   return "PROGRAM";
    case AST_REG:       return "REG";
    case AST_IMM:       return "IMM";
  }
  return "";
}

u0 ast_node_print(struct ast_node *n, u32 depth)
{
  if (NULL == n) return;

  for (u32 i = 0; i < depth; i++) printf("  ");

  switch (n->type)
  {
    case AST_INSTR:
      printf("AST(INSTR, %s)\n", rv32ii_instr_label(n->as_instr.instr));
      break;
    case AST_LABEL:
      printf("AST(LABEL, &%s)\n", n->as_label.name);
      break;
    case AST_LABEL_REF:
      printf("AST(LABEL_REF, @%s)\n", n->as_label_ref.name);
      break;
    case AST_SECTION:
      printf("AST(SECTION, addr=%u)\n", n->as_section.addr);
      break;
    case AST_PROGRAM:
      printf("AST(PROGRAM)\n");
      break;
    case AST_REG:
      printf("AST(REG, %u)\n", n->as_reg.reg);
      break;
    case AST_IMM:
      printf("AST(IMM, %d)\n", n->as_imm.value);
      break;
    default:
      printf("AST(%s)\n", ast_node_type_str(n->type));
      break;
  }

  for (u32 i = 0; i < n->children_size; i++)
    ast_node_print(n->children[i], depth + 1);
}

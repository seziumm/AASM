#ifndef _DIRECTIVE_UTILS_H
#define _DIRECTIVE_UTILS_H

#include <string.h>
#include <stdio.h>
#include <utils/common.h>
#include <utils/type.h>
#include <token/token_array.h>

struct codegen_ctx;

/* ============================================================
 *  Directive types
 * ============================================================ */

enum directive_type
{
  /* ---- sections ------------------------------------------- */
  DIRECTIVE_SECTION,    /* .CODE .DATA .TEXT .BSS .RODATA       */

  /* ---- integer data --------------------------------------- */
  DIRECTIVE_BYTE,       /* .BYTE   — 1 byte                     */
  DIRECTIVE_2BYTE,      /* .2BYTE  — 2 bytes little-endian      */
  DIRECTIVE_4BYTE,      /* .4BYTE  — 4 bytes little-endian      */
  DIRECTIVE_8BYTE,      /* .8BYTE  — 8 bytes little-endian      */
  DIRECTIVE_HALF,       /* .HALF   — 2 bytes (alias .2BYTE)     */
  DIRECTIVE_WORD,       /* .WORD   — 4 bytes (alias .4BYTE)     */
  DIRECTIVE_DWORD,      /* .DWORD  — 8 bytes (alias .8BYTE)     */
  DIRECTIVE_QUAD,       /* .QUAD   — 8 bytes (alias .8BYTE)     */

  /* ---- string / ascii data -------------------------------- */
  DIRECTIVE_STRING,     /* .STRING — bytes + NUL terminator     */
  DIRECTIVE_ASCII,      /* .ASCII  — bytes, NO NUL              */
  DIRECTIVE_ASCIZ,      /* .ASCIZ  — bytes + NUL (= .STRING)    */

  /* ---- alignment ------------------------------------------ */
  DIRECTIVE_ALIGN,      /* .ALIGN  n — align to 2^n boundary    */
  DIRECTIVE_BALIGN,     /* .BALIGN n — align to n bytes         */
  DIRECTIVE_P2ALIGN,    /* .P2ALIGN n — align to 2^n (= .ALIGN) */

  /* ---- space / fill --------------------------------------- */
  DIRECTIVE_ZERO,       /* .ZERO   n — emit n zero bytes        */
  DIRECTIVE_SKIP,       /* .SKIP   n — emit n zero bytes        */
  DIRECTIVE_SPACE,      /* .SPACE  n — emit n zero bytes        */

  /* ---- symbols -------------------------------------------- */
  DIRECTIVE_GLOBAL,     /* .GLOBAL sym — mark symbol global     */
  DIRECTIVE_GLOBL,      /* .GLOBL  sym — alias .GLOBAL          */
  DIRECTIVE_LOCAL,      /* .LOCAL  sym — mark symbol local      */
  DIRECTIVE_WEAK,       /* .WEAK   sym — mark symbol weak       */
  DIRECTIVE_EQU,        /* .EQU sym, val — define constant      */
  DIRECTIVE_SET,        /* .SET sym, val — alias .EQU           */

  /* ---- type / size (ELF) ---------------------------------- */
  DIRECTIVE_TYPE,       /* .TYPE   sym, @type                   */
  DIRECTIVE_SIZE,       /* .SIZE   sym, expr                    */

  /* ---- misc ----------------------------------------------- */
  DIRECTIVE_FILE,       /* .FILE   "name" — debug info          */
  DIRECTIVE_OPTION,     /* .OPTION rvc/norvc/pic/nopic          */
  DIRECTIVE_ATTRIBUTE,  /* .ATTRIBUTE tag, value                */
};

static inline const char *directive_type_to_str(enum directive_type type)
{
  switch (type)
  {
    case DIRECTIVE_SECTION:   return "DIRECTIVE_SECTION";
    case DIRECTIVE_BYTE:      return "DIRECTIVE_BYTE";
    case DIRECTIVE_2BYTE:     return "DIRECTIVE_2BYTE";
    case DIRECTIVE_4BYTE:     return "DIRECTIVE_4BYTE";
    case DIRECTIVE_8BYTE:     return "DIRECTIVE_8BYTE";
    case DIRECTIVE_HALF:      return "DIRECTIVE_HALF";
    case DIRECTIVE_WORD:      return "DIRECTIVE_WORD";
    case DIRECTIVE_DWORD:     return "DIRECTIVE_DWORD";
    case DIRECTIVE_QUAD:      return "DIRECTIVE_QUAD";
    case DIRECTIVE_STRING:    return "DIRECTIVE_STRING";
    case DIRECTIVE_ASCII:     return "DIRECTIVE_ASCII";
    case DIRECTIVE_ASCIZ:     return "DIRECTIVE_ASCIZ";
    case DIRECTIVE_ALIGN:     return "DIRECTIVE_ALIGN";
    case DIRECTIVE_BALIGN:    return "DIRECTIVE_BALIGN";
    case DIRECTIVE_P2ALIGN:   return "DIRECTIVE_P2ALIGN";
    case DIRECTIVE_ZERO:      return "DIRECTIVE_ZERO";
    case DIRECTIVE_SKIP:      return "DIRECTIVE_SKIP";
    case DIRECTIVE_SPACE:     return "DIRECTIVE_SPACE";
    case DIRECTIVE_GLOBAL:    return "DIRECTIVE_GLOBAL";
    case DIRECTIVE_GLOBL:     return "DIRECTIVE_GLOBL";
    case DIRECTIVE_LOCAL:     return "DIRECTIVE_LOCAL";
    case DIRECTIVE_WEAK:      return "DIRECTIVE_WEAK";
    case DIRECTIVE_EQU:       return "DIRECTIVE_EQU";
    case DIRECTIVE_SET:       return "DIRECTIVE_SET";
    case DIRECTIVE_TYPE:      return "DIRECTIVE_TYPE";
    case DIRECTIVE_SIZE:      return "DIRECTIVE_SIZE";
    case DIRECTIVE_FILE:      return "DIRECTIVE_FILE";
    case DIRECTIVE_OPTION:    return "DIRECTIVE_OPTION";
    case DIRECTIVE_ATTRIBUTE: return "DIRECTIVE_ATTRIBUTE";
  }
  die(1, "directive_type_to_str: unknown type %d", type);
  return "";
}

/* ============================================================
 *  Build function pointer
 * ============================================================ */

typedef u0 (*directive_pass1_fn)(u32 *pc, u32 *pos, struct token_array *ta);
typedef u0 (*directive_build_fn)(struct codegen_ctx *ctx, u32 *pos);

/* ============================================================
 *  Forward declarations of build functions
 * ============================================================ */

/* pass1 functions */
static inline u0 directive_pass1_skip     (u32 *pc, u32 *pos, struct token_array *ta);
static inline u0 directive_pass1_fixed1   (u32 *pc, u32 *pos, struct token_array *ta);
static inline u0 directive_pass1_fixed2   (u32 *pc, u32 *pos, struct token_array *ta);
static inline u0 directive_pass1_fixed4   (u32 *pc, u32 *pos, struct token_array *ta);
static inline u0 directive_pass1_fixed8   (u32 *pc, u32 *pos, struct token_array *ta);
static inline u0 directive_pass1_string   (u32 *pc, u32 *pos, struct token_array *ta);
static inline u0 directive_pass1_ascii    (u32 *pc, u32 *pos, struct token_array *ta);
static inline u0 directive_pass1_align    (u32 *pc, u32 *pos, struct token_array *ta);
static inline u0 directive_pass1_balign   (u32 *pc, u32 *pos, struct token_array *ta);
static inline u0 directive_pass1_zero     (u32 *pc, u32 *pos, struct token_array *ta);
static inline u0 directive_pass1_consume1 (u32 *pc, u32 *pos, struct token_array *ta);
static inline u0 directive_pass1_consume3 (u32 *pc, u32 *pos, struct token_array *ta);

/* build functions */
static inline u0 directive_build_section  (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_byte     (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_2byte    (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_4byte    (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_8byte    (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_string   (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_ascii    (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_align    (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_balign   (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_zero     (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_symbol   (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_equ      (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_type     (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_size     (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_file     (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_option   (struct codegen_ctx *ctx, u32 *pos);
static inline u0 directive_build_attribute(struct codegen_ctx *ctx, u32 *pos);

/* ============================================================
 *  Directive descriptor
 * ============================================================ */

struct directive
{
  const char           *label;
  enum directive_type   type;
  directive_pass1_fn    pass1_fn; /* advance pc+pos in pass 1, no I/O */
  directive_build_fn    build_fn; /* emit bytes in pass 2             */
};

/* ============================================================
 *  Directive table
 * ============================================================ */

static const struct directive directive_array[] =
{
  /* ---- sections ------------------------------------------- */
  { ".CODE",      DIRECTIVE_SECTION,   directive_pass1_skip,     directive_build_section   },
  { ".DATA",      DIRECTIVE_SECTION,   directive_pass1_skip,     directive_build_section   },
  { ".TEXT",      DIRECTIVE_SECTION,   directive_pass1_skip,     directive_build_section   },
  { ".BSS",       DIRECTIVE_SECTION,   directive_pass1_skip,     directive_build_section   },
  { ".RODATA",    DIRECTIVE_SECTION,   directive_pass1_skip,     directive_build_section   },

  /* ---- integer data --------------------------------------- */
  { ".BYTE",      DIRECTIVE_BYTE,      directive_pass1_fixed1,   directive_build_byte      },
  { ".2BYTE",     DIRECTIVE_2BYTE,     directive_pass1_fixed2,   directive_build_2byte     },
  { ".4BYTE",     DIRECTIVE_4BYTE,     directive_pass1_fixed4,   directive_build_4byte     },
  { ".8BYTE",     DIRECTIVE_8BYTE,     directive_pass1_fixed8,   directive_build_8byte     },
  { ".HALF",      DIRECTIVE_HALF,      directive_pass1_fixed2,   directive_build_2byte     },
  { ".WORD",      DIRECTIVE_WORD,      directive_pass1_fixed4,   directive_build_4byte     },
  { ".DWORD",     DIRECTIVE_DWORD,     directive_pass1_fixed8,   directive_build_8byte     },
  { ".QUAD",      DIRECTIVE_QUAD,      directive_pass1_fixed8,   directive_build_8byte     },

  /* ---- string / ascii ------------------------------------- */
  { ".STRING",    DIRECTIVE_STRING,    directive_pass1_string,   directive_build_string    },
  { ".ASCIZ",     DIRECTIVE_ASCIZ,     directive_pass1_string,   directive_build_string    },
  { ".ASCII",     DIRECTIVE_ASCII,     directive_pass1_ascii,    directive_build_ascii     },

  /* ---- alignment ------------------------------------------ */
  { ".ALIGN",     DIRECTIVE_ALIGN,     directive_pass1_align,    directive_build_align     },
  { ".P2ALIGN",   DIRECTIVE_P2ALIGN,   directive_pass1_align,    directive_build_align     },
  { ".BALIGN",    DIRECTIVE_BALIGN,    directive_pass1_balign,   directive_build_balign    },

  /* ---- space / fill --------------------------------------- */
  { ".ZERO",      DIRECTIVE_ZERO,      directive_pass1_zero,     directive_build_zero      },
  { ".SKIP",      DIRECTIVE_SKIP,      directive_pass1_zero,     directive_build_zero      },
  { ".SPACE",     DIRECTIVE_SPACE,     directive_pass1_zero,     directive_build_zero      },

  /* ---- symbols — no bytes --------------------------------- */
  { ".GLOBAL",    DIRECTIVE_GLOBAL,    directive_pass1_consume1, directive_build_symbol    },
  { ".GLOBL",     DIRECTIVE_GLOBL,     directive_pass1_consume1, directive_build_symbol    },
  { ".LOCAL",     DIRECTIVE_LOCAL,     directive_pass1_consume1, directive_build_symbol    },
  { ".WEAK",      DIRECTIVE_WEAK,      directive_pass1_consume1, directive_build_symbol    },
  { ".EQU",       DIRECTIVE_EQU,       directive_pass1_consume3, directive_build_equ       },
  { ".SET",       DIRECTIVE_SET,       directive_pass1_consume3, directive_build_equ       },

  /* ---- type / size ---------------------------------------- */
  { ".TYPE",      DIRECTIVE_TYPE,      directive_pass1_consume3, directive_build_type      },
  { ".SIZE",      DIRECTIVE_SIZE,      directive_pass1_consume3, directive_build_size      },

  /* ---- misc ----------------------------------------------- */
  { ".FILE",      DIRECTIVE_FILE,      directive_pass1_consume1, directive_build_file      },
  { ".OPTION",    DIRECTIVE_OPTION,    directive_pass1_consume1, directive_build_option    },
  { ".ATTRIBUTE", DIRECTIVE_ATTRIBUTE, directive_pass1_consume3, directive_build_attribute },
};

#define DIRECTIVE_ARRAY_SIZE (sizeof(directive_array) / sizeof(directive_array[0]))

/* ============================================================
 *  Lookup utils
 * ============================================================ */

static inline const struct directive *directive_from_label(const char *label)
{
  for (u32 i = 0; i < DIRECTIVE_ARRAY_SIZE; i++)
    if (strcmp(directive_array[i].label, label) == 0)
      return &directive_array[i];
  return NULL;
}

static inline const struct directive *directive_expect_label(const char *label)
{
  const struct directive *d = directive_from_label(label);
  if (unlikely(NULL == d))
    die(1, "directive_expect_label: unknown directive '%s'", label);
  return d;
}

static inline i32 directive_is_label(const char *label)
{
  return directive_from_label(label) != NULL;
}

/* ============================================================
 *  Dispatch
 * ============================================================ */

static inline u0 directive_build(const struct directive *d,
                                  struct codegen_ctx     *ctx,
                                  u32                    *pos)
{
  expect(NULL != d);
  if (unlikely(NULL == d->build_fn))
    die(1, "directive_build: no build_fn for '%s'", d->label);
  d->build_fn(ctx, pos);
}

static inline u0 directive_pass1(const struct directive *d,
                                  u32                    *pc,
                                  u32                    *pos,
                                  struct token_array     *ta)
{
  expect(NULL != d);
  if (unlikely(NULL == d->pass1_fn))
    die(1, "directive_pass1: no pass1_fn for '%s'", d->label);
  d->pass1_fn(pc, pos, ta);
}

/* ============================================================
 *  Pass 1 function bodies
 *  No I/O — only advance pc and pos.
 * ============================================================ */

#include <gen/codegen.h>
#include <token/token_type.h>
#include <stdlib.h>

/* sections — no bytes, no value token */
static inline u0 directive_pass1_skip(u32 *pc, u32 *pos, struct token_array *ta)
{
  (u0)pc; (u0)pos; (u0)ta;
}

/* fixed 1-byte data — consume 1 value token */
static inline u0 directive_pass1_fixed1(u32 *pc, u32 *pos, struct token_array *ta)
{
  (u0)ta; *pc += 1; *pos += 1;
}

/* fixed 2-byte data */
static inline u0 directive_pass1_fixed2(u32 *pc, u32 *pos, struct token_array *ta)
{
  (u0)ta; *pc += 2; *pos += 1;
}

/* fixed 4-byte data */
static inline u0 directive_pass1_fixed4(u32 *pc, u32 *pos, struct token_array *ta)
{
  (u0)ta; *pc += 4; *pos += 1;
}

/* fixed 8-byte data */
static inline u0 directive_pass1_fixed8(u32 *pc, u32 *pos, struct token_array *ta)
{
  (u0)ta; *pc += 8; *pos += 1;
}

/* .STRING / .ASCIZ — strlen(value) + 1 NUL byte */
static inline u0 directive_pass1_string(u32 *pc, u32 *pos, struct token_array *ta)
{
  if (unlikely(*pos >= ta->size || ta->tokens[*pos]->type != TOKEN_STRING))
    die(1, "directive_pass1_string: expected TOKEN_STRING");
  *pc  += (u32)strlen(ta->tokens[*pos]->value) + 1;
  *pos += 1;
}

/* .ASCII — strlen(value), no NUL */
static inline u0 directive_pass1_ascii(u32 *pc, u32 *pos, struct token_array *ta)
{
  if (unlikely(*pos >= ta->size || ta->tokens[*pos]->type != TOKEN_STRING))
    die(1, "directive_pass1_ascii: expected TOKEN_STRING");
  *pc  += (u32)strlen(ta->tokens[*pos]->value);
  *pos += 1;
}

/* .ALIGN / .P2ALIGN n — pad to 2^n boundary */
static inline u0 directive_pass1_align(u32 *pc, u32 *pos, struct token_array *ta)
{
  if (unlikely(*pos >= ta->size || ta->tokens[*pos]->type != TOKEN_NUMBER))
    die(1, "directive_pass1_align: expected TOKEN_NUMBER");
  u32 n     = (u32)atoi(ta->tokens[*pos]->value);
  u32 align = 1u << n;
  u32 rem   = *pc & (align - 1);
  if (rem != 0) *pc += align - rem;
  *pos += 1;
}

/* .BALIGN n — pad to n boundary */
static inline u0 directive_pass1_balign(u32 *pc, u32 *pos, struct token_array *ta)
{
  if (unlikely(*pos >= ta->size || ta->tokens[*pos]->type != TOKEN_NUMBER))
    die(1, "directive_pass1_balign: expected TOKEN_NUMBER");
  u32 align = (u32)atoi(ta->tokens[*pos]->value);
  u32 rem   = *pc & (align - 1);
  if (rem != 0) *pc += align - rem;
  *pos += 1;
}

/* .ZERO / .SKIP / .SPACE n — n zero bytes */
static inline u0 directive_pass1_zero(u32 *pc, u32 *pos, struct token_array *ta)
{
  if (unlikely(*pos >= ta->size || ta->tokens[*pos]->type != TOKEN_NUMBER))
    die(1, "directive_pass1_zero: expected TOKEN_NUMBER");
  *pc  += (u32)atoi(ta->tokens[*pos]->value);
  *pos += 1;
}

/* consume 1 token (symbol directives, .FILE, .OPTION) */
static inline u0 directive_pass1_consume1(u32 *pc, u32 *pos, struct token_array *ta)
{
  (u0)pc; (u0)ta; *pos += 1;
}

/* consume 3 tokens: sym , val  (.EQU, .TYPE, .SIZE, .ATTRIBUTE) */
static inline u0 directive_pass1_consume3(u32 *pc, u32 *pos, struct token_array *ta)
{
  (u0)pc; (u0)ta; *pos += 3;
}

/* ============================================================
 *  Build function bodies
 * ============================================================ */

#include <gen/codegen.h>
#include <token/token_type.h>
#include <stdlib.h>

/* .CODE .DATA .TEXT .BSS .RODATA — no bytes */
static inline u0 directive_build_section(struct codegen_ctx *ctx, u32 *pos)
{
  (u0)ctx; (u0)pos;
}

/* .BYTE — 1 byte signed integer */
static inline u0 directive_build_byte(struct codegen_ctx *ctx, u32 *pos)
{
  struct token *t = cg_expect(ctx, pos, TOKEN_NUMBER);
  u8 val = (u8)(i8)atoi(t->value);
  fwrite(&val, 1, 1, ctx->out);
  ctx->pc += 1;
}

/* .2BYTE / .HALF — 2 bytes little-endian */
static inline u0 directive_build_2byte(struct codegen_ctx *ctx, u32 *pos)
{
  struct token *t = cg_expect(ctx, pos, TOKEN_NUMBER);
  u16 val = (u16)(i16)atoi(t->value);
  fwrite(&val, 2, 1, ctx->out);
  ctx->pc += 2;
}

/* .4BYTE / .WORD — 4 bytes little-endian */
static inline u0 directive_build_4byte(struct codegen_ctx *ctx, u32 *pos)
{
  struct token *t = cg_expect(ctx, pos, TOKEN_NUMBER);
  u32 val = (u32)(i32)atoi(t->value);
  fwrite(&val, 4, 1, ctx->out);
  ctx->pc += 4;
}

/* .8BYTE / .DWORD / .QUAD — 8 bytes little-endian */
static inline u0 directive_build_8byte(struct codegen_ctx *ctx, u32 *pos)
{
  struct token *t = cg_expect(ctx, pos, TOKEN_NUMBER);
  u64 val = (u64)(i64)atoll(t->value);
  fwrite(&val, 8, 1, ctx->out);
  ctx->pc += 8;
}

/* .STRING / .ASCIZ — bytes + NUL terminator */
static inline u0 directive_build_string(struct codegen_ctx *ctx, u32 *pos)
{
  struct token *t = cg_expect(ctx, pos, TOKEN_STRING);
  u32 len = (u32)strlen(t->value);
  fwrite(t->value, 1, len + 1, ctx->out);
  ctx->pc += len + 1;
}

/* .ASCII — bytes without NUL terminator */
static inline u0 directive_build_ascii(struct codegen_ctx *ctx, u32 *pos)
{
  struct token *t = cg_expect(ctx, pos, TOKEN_STRING);
  u32 len = (u32)strlen(t->value);
  fwrite(t->value, 1, len, ctx->out);
  ctx->pc += len;
}

/* .ALIGN / .P2ALIGN n — pad with zeros to 2^n byte boundary */
static inline u0 directive_build_align(struct codegen_ctx *ctx, u32 *pos)
{
  struct token *t = cg_expect(ctx, pos, TOKEN_NUMBER);
  u32 n     = (u32)atoi(t->value);
  u32 align = 1u << n;
  u32 rem   = ctx->pc & (align - 1);

  if (rem != 0)
  {
    u32 pad = align - rem;
    for (u32 i = 0; i < pad; i++)
    {
      u8 zero = 0;
      fwrite(&zero, 1, 1, ctx->out);
    }
    ctx->pc += pad;
  }
}

/* .BALIGN n — pad with zeros to n byte boundary (n must be power of 2) */
static inline u0 directive_build_balign(struct codegen_ctx *ctx, u32 *pos)
{
  struct token *t = cg_expect(ctx, pos, TOKEN_NUMBER);
  u32 align = (u32)atoi(t->value);
  u32 rem   = ctx->pc & (align - 1);

  if (rem != 0)
  {
    u32 pad = align - rem;
    for (u32 i = 0; i < pad; i++)
    {
      u8 zero = 0;
      fwrite(&zero, 1, 1, ctx->out);
    }
    ctx->pc += pad;
  }
}

/* .ZERO / .SKIP / .SPACE n — emit n zero bytes */
static inline u0 directive_build_zero(struct codegen_ctx *ctx, u32 *pos)
{
  struct token *t = cg_expect(ctx, pos, TOKEN_NUMBER);
  u32 n = (u32)atoi(t->value);
  for (u32 i = 0; i < n; i++)
  {
    u8 zero = 0;
    fwrite(&zero, 1, 1, ctx->out);
  }
  ctx->pc += n;
}

/* .GLOBAL / .GLOBL / .LOCAL / .WEAK sym — symbol visibility.
 * No bytes emitted — would need an ELF symbol table to act on these. */
static inline u0 directive_build_symbol(struct codegen_ctx *ctx, u32 *pos)
{
  (u0)cg_expect(ctx, pos, TOKEN_LABEL); /* consume the symbol name token */
}

/* .EQU / .SET sym, value — constant definition.
 * No bytes emitted — would need a constant table to act on these. */
static inline u0 directive_build_equ(struct codegen_ctx *ctx, u32 *pos)
{
  (u0)cg_expect(ctx, pos, TOKEN_LABEL);  /* sym  */
  (u0)cg_expect(ctx, pos, TOKEN_COMMA);  /* ,    */
  (u0)cg_expect(ctx, pos, TOKEN_NUMBER); /* val  */
}

/* .TYPE sym, @type — ELF symbol type annotation, no bytes. */
static inline u0 directive_build_type(struct codegen_ctx *ctx, u32 *pos)
{
  (u0)cg_expect(ctx, pos, TOKEN_LABEL);  /* sym   */
  (u0)cg_expect(ctx, pos, TOKEN_COMMA);  /* ,     */
  (u0)cg_expect(ctx, pos, TOKEN_LABEL);  /* @type */
}

/* .SIZE sym, expr — ELF symbol size annotation, no bytes. */
static inline u0 directive_build_size(struct codegen_ctx *ctx, u32 *pos)
{
  (u0)cg_expect(ctx, pos, TOKEN_LABEL);  /* sym  */
  (u0)cg_expect(ctx, pos, TOKEN_COMMA);  /* ,    */
  (u0)cg_expect(ctx, pos, TOKEN_NUMBER); /* expr */
}

/* .FILE "name" — debug info filename, no bytes. */
static inline u0 directive_build_file(struct codegen_ctx *ctx, u32 *pos)
{
  (u0)cg_expect(ctx, pos, TOKEN_STRING);
}

/* .OPTION rvc/norvc/pic/nopic — assembler option, no bytes. */
static inline u0 directive_build_option(struct codegen_ctx *ctx, u32 *pos)
{
  (u0)cg_expect(ctx, pos, TOKEN_INSTR); /* option name is a bare word */
}

/* .ATTRIBUTE tag, value — ABI attribute, no bytes. */
static inline u0 directive_build_attribute(struct codegen_ctx *ctx, u32 *pos)
{
  (u0)cg_expect(ctx, pos, TOKEN_NUMBER); /* tag   */
  (u0)cg_expect(ctx, pos, TOKEN_COMMA);  /* ,     */
  (u0)cg_expect(ctx, pos, TOKEN_NUMBER); /* value */
}

#endif /* _DIRECTIVE_UTILS_H */

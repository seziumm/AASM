#include <rv32/rv32_type.h>
#include <rv32/rv32_reg.h>
#include <rv32/rv32_instr.h>
#include <parser.h>
#include <codegen.h>
#include <stdio.h>

/* ============================================================
 *  Lifecycle
 * ============================================================ */

static inline u0 codegen_expect_node_type(struct ast_node *node, enum ast_node_type t)
{
    if(node->type != t)
    {
        codegen_die(NULL, 1,"Expected %s but found %s", "", ""); 
    }
}

static inline u8 codegen_expect_reg(struct ast_node *node)
{
    codegen_expect_node_type(node, AST_REG); 
    return node->as_reg.reg;
}

static inline i32 codegen_expect_imm(struct ast_node *node)
{
    codegen_expect_node_type(node, AST_IMM); 
    return node->as_imm.value;
}

u0 codegen_build_label(struct ast_node *node, u32 *pc)
{
    codegen_expect_node_type(node, AST_LABEL);

    *pc += 4;

    // TODO add label to the current hast_label

}


u0 codegen_build_instr(struct ast_node *node, u32 *pc)
{
    codegen_expect_node_type(node, AST_INSTR);

    *pc += 4;
    
    enum rv32i_instr t = node->as_instr.instr;
    const struct rv32ii_opcode_entry *e = rv32ii_instr_from_enum(t);

    u32 params = params_of_instr(e->type);

    u8 rd   = 0;
    u8 rs1  = 0;
    u8 rs2  = 0;
    i32 imm = 0;

    if (t == RV32I_ECALL || t == RV32I_EBREAK)
    {
        return;
    }


  switch (e->type)
  {
    case R_TYPE:
        {
            if(node->children_size != params)
            {
                codegen_die(NULL, 1, "ERROR R_TYPE HAS A DIFFERENT TYPE OF PARAMS");
            }

            rd  = codegen_expect_reg(node->children[0]);
            rs1 = codegen_expect_reg(node->children[1]);
            rs2 = codegen_expect_reg(node->children[2]);
            break;
        }

    case I_TYPE:
        {
            if (rv32ii_is_load(t))
            {
                ast_node_push(node, parse_reg(p));
                parse_mem(p, node); // this is as the imm, reg
            }
            else
            {
                ast_node_push(node, parse_reg(p));
                ast_node_push(node, parse_reg(p));
                ast_node_push(node, parse_imm(p));
            }
            break;

        }

    case S_TYPE:
      ast_node_push(node, parse_reg(p));
      parse_mem(p, node);
      break;

    case B_TYPE:
      ast_node_push(node, parse_reg(p));
      ast_node_push(node, parse_reg(p));

      if (next_is_label_ref(p))
        ast_node_push(node, parse_label_ref(p));
      else
        ast_node_push(node, parse_imm(p));
      break;

    case U_TYPE:
      ast_node_push(node, parse_reg(p));
      ast_node_push(node, parse_imm(p));
      break;

    case J_TYPE:
      ast_node_push(node, parse_reg(p));
      if (next_is_label_ref(p))
        ast_node_push(node, parse_label_ref(p));
      else
        ast_node_push(node, parse_imm(p));
      break;
  }

    rv32_encode(instr, rd, rs1, rs2, imm);

}

static u0 codegen_fill_pc(u32 *pc, u32 pc_target)
{
    if(pc_target < *pc)
    {
        codegen_die(NULL, 1, "PC TARGET %d IS SMALLER THAN CURRENT PC %d", pc_target , *pc);
    }

    while(*pc < pc_target)
    {
        printf("0x00000000 %u\tTARGET %u\n", *pc, pc_target);
        (*pc) += 4;
    }

}

u0 codegen_build_section(struct ast_node *node, u32 *pc)
{
    codegen_expect_node_type(node, AST_SECTION);

    codegen_fill_pc(pc, node->as_section.addr);

    for(u32 i = 0; i < node->children_size; i++)
    {
        struct ast_node *child = node->children[i];

        if(child->type == AST_LABEL)
        {
            codegen_build_label(child, pc);
        }
        else if(child->type == AST_INSTR)
        {
            codegen_build_instr(child, pc);
        }

        codegen_die(NULL, 1, "EXPECTED AST_LABEL OR AST_INSTR BUT FOUND OTHER");
    }
}


u0 codegen_build_program(struct ast_node *node, u32 *pc)
{
    codegen_expect_node_type(node, AST_PROGRAM);

    for(u32 i = 0; i < node->children_size; i++)
    {
        struct ast_node *child = node->children[i];
        if(child->type == AST_SECTION)
        {
            codegen_build_section(child, pc);
        }

        codegen_die(NULL, 1, "EXPECTED AST_SECTION BUT FOUND" );
    }
}


u0 codegen_compile(struct ast_node *root)
{
  if(NULL == root) 
  {
    codegen_die(NULL, 1, "codegen_build() failed because root is NULL");
  }

  u32 pc = CODEGEN_PC_INIT_ADDR;
  printf("%u\n", pc);

  codegen_build_program(root, &pc);
}


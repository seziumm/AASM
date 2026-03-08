#ifndef _CODEGEN_H
#define _CODEGEN_H

#include <parser.h>
#include <common.h>
#include <type.h>

#define CODEGEN_PC_INIT_ADDR       0x80000000

/* Prints codegen state then calls die().
   Used for unrecoverable codegen errors. */
#define codegen_die(cg, err, ...)  \
  do                               \
  {                                \
    /*codegen_print(cg);*/         \
    die(err, __VA_ARGS__);         \
  } while (0)

u0 codegen_compile(struct ast_node *root);


#endif

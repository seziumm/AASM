#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <token/token_array.h>

#define TOKENIZER_INIT_CAPACITY 4

struct tokenizer
{
  struct token_array table;
};


struct tokenizer *tokenizer_compile(const char *src);
u0                tokenizer_free   (struct tokenizer **t);
u0                tokenizer_print  (struct tokenizer  *t);

u32 tok_whitespace(struct tokenizer *t, const char *c);
u32 tok_comment   (struct tokenizer *t, const char *c);
u32 tok_comma     (struct tokenizer *t, const char *c);
u32 tok_directive (struct tokenizer *t, const char *c);
u32 tok_label     (struct tokenizer *t, const char *c);
u32 tok_label_ref (struct tokenizer *t, const char *c);
u32 tok_number    (struct tokenizer *t, const char *c);
u32 tok_word      (struct tokenizer *t, const char *c);
u32 tok_string    (struct tokenizer *t, const char *c);

#endif /* _TOKENIZER_H */

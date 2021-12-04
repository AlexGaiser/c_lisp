#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* Fake readline function */
char *readline(char *prompt)
{
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char *cpy = malloc(strlen(buffer) + 1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy) - 1] = '\0';
  return cpy;
}

/* Fake add_history function */
void add_history(char *unused) {}

/* Otherwise include the editline headers */
#else
#include <editline/readline.h>
#endif

int number_of_nodes(mpc_ast_t *t)
{
  int children_num = t->children_num;
  if (children_num == 0)
  {
    return 1;
  }

  if (children_num >= 1)
  {
    int total = 1;

    for (int i = 0; i < children_num; i++)
    {
      total = total + number_of_nodes(t->children[i]);
    }
    return total;
  }
  return 0;
}

int main(int argc, char const *argv[])
{
  /* Create Some Parsers */
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Operator = mpc_new("operator");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Lispy = mpc_new("lispy");

  /* Define them with the following Language */
  mpca_lang(MPCA_LANG_DEFAULT,
            "                                                     \
    number   : /-?[0-9]+/ ;                             \
    operator : '+' | '-' | '*' | '/' ;                  \
    expr     : <number> | '(' <operator> <expr>+ ')' ;  \
    lispy    : /^/ <operator> <expr>+ /$/ ;             \
  ",
            Number, Operator, Expr, Lispy);
  puts("A_Lisp Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

  while (1)
  {
    char *input = readline("alisp> ");
    add_history(input);
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r))
    {
      /* On Success Print the AST */
      mpc_ast_t *a = r.output;
      printf("Tag: %s\n", a->tag);
      printf("Contents %s\n", a->contents);
      printf("Num Children: %i\n", a->children_num);

      // Getting child nodes

      // first child:
      mpc_ast_t *c0 = a->children[0];
      printf("First Child Contents: %s\n", c0->contents);
      printf("First Child Number of children: %i\n", c0->children_num);

      int total_nodes = number_of_nodes(a);
      printf("total number of nodes is: %i", total_nodes);
      mpc_ast_print(r.output);
      mpc_ast_delete(r.output);
    }
    else
    {
      /* Otherwise Print the Error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    free(input);
  }
  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  return 0;
}

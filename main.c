#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"
// #include <string.h>

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

enum
{
  LVAL_NUM,
  LVAL_ERR
};
enum
{
  LERR_DIV_ZERO,
  LERR_BAD_OP,
  LERR_BAD_NUM
};

typedef struct
{
  int type;
  long num;
  int err;
} lval;

// Functions for constructing our Lisp Values (from structs)

lval create_lval_num(long value)
{
  lval val;
  val.type = LVAL_NUM;
  val.num = value;
  // val.err = NULL;

  return val;
}

lval create_lval_error(int ecode)
{
  lval val;
  val.err = ecode;
  val.type = LVAL_ERR;
  // val.num = NULL;

  return val;
}

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

void lval_print(lval v)
{
  switch (v.type)
  {
  // If receive number, just print it immediately;
  case LVAL_NUM:
    printf("%li\n", v.num);
    break;

  // If receive error, we handle appropriately, printing the correct error
  case LVAL_ERR:
    if (v.err == LERR_DIV_ZERO)
    {
      printf("Error: Division By Zero!\n");
    }
    if (v.err == LERR_BAD_OP)
    {
      printf("Error: Invalid Operator!\n");
    }
    if (v.err == LERR_BAD_NUM)
    {
      printf("Error: Invalid Number!\n");
    }
    break;
  }
}
lval eval_op(lval x, char *op, lval y)
{

  if (x.type == LVAL_ERR)
  {
    return x;
  }
  if (y.type == LVAL_ERR)
  {
    return y;
  }

  if (strcmp(op, "+") == 0)
  {
    return create_lval_num(x.num + y.num);
  }
  if (strcmp(op, "-") == 0)
  {
    return create_lval_num(x.num - y.num);
  }
  if (strcmp(op, "*") == 0)
  {
    return create_lval_num(x.num * y.num);
  }
  if (strcmp(op, "/") == 0)
  {
    return y.num == 0
               ? create_lval_error(LERR_DIV_ZERO)
               : create_lval_num(x.num / y.num);
  }
  return create_lval_error(LERR_BAD_OP);
}

lval eval(mpc_ast_t *t)
{
  if (strstr(t->tag, "number"))
  {
    errno = 0;
    long n = strtol(t->contents, NULL, 10);
    return errno != ERANGE
               ? create_lval_num(n)
               : create_lval_error(LERR_BAD_NUM);
  }

  char *op = t->children[1]->contents; // operator is always second child

  lval val = eval(t->children[2]); // we store the third child in val

  int i = 3;

  while (strstr(t->children[i]->tag, "expr"))
  {
    val = eval_op(val, op, eval(t->children[i]));
    i++;
  }
  return val;
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
  puts("A_Lisp Version 0.0.0.0.4");
  puts("Press Ctrl+c to Exit\n");

  while (1)
  {
    char *input = readline("alisp> ");
    add_history(input);
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r))
    {
      // On Success Print the Result!
      lval result = eval(r.output);
      lval_print(result);

      mpc_ast_delete(r.output);
    }
    else
    {
      // If Error, print it here
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    free(input);
  }
  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  return 0;
}

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
  LVAL_ERR,
  LVAL_NUM,
  LVAL_SYM,
  LVAL_SEXPR
};

enum
{
  LERR_DIV_ZERO,
  LERR_BAD_OP,
  LERR_BAD_NUM
};

typedef struct lval
{
  int type;
  long num;
  /* Error and Symbol types have some string data */
  char *err;
  char *sym;
  /* Count and Pointer to a list of "lval*" */
  int count;
  struct lval **cell;
} lval;

// Forward declaring lval_print
void lval_print(lval *v);

// Functions for constructing our Lisp Values (from structs)

lval *create_lval_num(long x)
{
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

lval *create_lval_err(char *m)
{
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(m) + 1);
  strcpy(v->err, m);
  return v;
}

/* Construct a pointer to a new Symbol lval */
lval *create_lval_sym(char *s)
{
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

lval *create_lval_sexpr(void)
{
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
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
void lval_expr_print(lval *v, char open, char close)
{
  putchar(open);
  for (int i = 0; i < v->count; i++)
  {

    /* Print Value contained within */
    lval_print(v->cell[i]);

    /* Don't print trailing space if last element */
    if (i != (v->count - 1))
    {
      putchar(' ');
    }
  }
  putchar(close);
}
void lval_print(lval *v)
{
  switch (v->type)
  {
  case LVAL_NUM:
    printf("%li", v->num);
    break;
  case LVAL_ERR:
    printf("Error: %s", v->err);
    break;
  case LVAL_SYM:
    printf("%s", v->sym);
    break;
  case LVAL_SEXPR:
    lval_expr_print(v, '(', ')');
    break;
  }
}

void lval_println(lval *v)
{
  lval_print(v);
  putchar('\n');
}
lval *lval_read_num(mpc_ast_t *t)
{
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ? create_lval_num(x) : create_lval_err("invalid number");
}

lval *lval_add(lval *v, lval *x)
{
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval *) * v->count);
  v->cell[v->count - 1] = x;
  return v;
}

lval *lval_read(mpc_ast_t *t)
{

  /* If Symbol or Number return conversion to that type */
  if (strstr(t->tag, "number"))
  {
    return lval_read_num(t);
  }
  if (strstr(t->tag, "symbol"))
  {
    return create_lval_sym(t->contents);
  }

  /* If root (>) or sexpr then create empty list */
  lval *x = NULL;
  if (strcmp(t->tag, ">") == 0)
  {
    x = create_lval_sexpr();
  }
  if (strstr(t->tag, "sexpr"))
  {
    x = create_lval_sexpr();
  }

  /* Fill this list with any valid expression contained within */
  for (int i = 0; i < t->children_num; i++)
  {
    if (strcmp(t->children[i]->contents, "(") == 0)
    {
      continue;
    }
    if (strcmp(t->children[i]->contents, ")") == 0)
    {
      continue;
    }
    if (strcmp(t->children[i]->tag, "regex") == 0)
    {
      continue;
    }
    x = lval_add(x, lval_read(t->children[i]));
  }

  return x;
}

void lval_del(lval *v)
{

  switch (v->type)
  {
  /* Do nothing special for number type */
  case LVAL_NUM:
    break;

  /* For Err or Sym free the string data */
  case LVAL_ERR:
    free(v->err);
    break;
  case LVAL_SYM:
    free(v->sym);
    break;

  /* If Sexpr then delete all elements inside */
  case LVAL_SEXPR:
    for (int i = 0; i < v->count; i++)
    {
      lval_del(v->cell[i]);
    }
    /* Also free the memory allocated to contain the pointers */
    free(v->cell);
    break;
  }

  /* Free the memory allocated for the "lval" struct itself */
  free(v);
}

// lval *eval_op(lval x, char *op, lval y)
// {

//   if (x.type == LVAL_ERR)
//   {
//     return x;
//   }
//   if (y.type == LVAL_ERR)
//   {
//     return y;
//   }

//   if (strcmp(op, "+") == 0)
//   {
//     return create_lval_num(x.num + y.num);
//   }
//   if (strcmp(op, "-") == 0)
//   {
//     return create_lval_num(x.num - y.num);
//   }
//   if (strcmp(op, "*") == 0)
//   {
//     return create_lval_num(x.num * y.num);
//   }
//   if (strcmp(op, "/") == 0)
//   {
//     return y.num == 0
//                ? create_lval_error(LERR_DIV_ZERO)
//                : create_lval_num(x.num / y.num);
//   }
//   return create_lval_error(LERR_BAD_OP);
// }

// lval eval(mpc_ast_t *t)
// {
//   if (strstr(t->tag, "number"))
//   {
//     errno = 0;
//     long n = strtol(t->contents, NULL, 10);
//     return errno != ERANGE
//                ? create_lval_num(n)
//                : create_lval_error(LERR_BAD_NUM);
//   }

//   char *op = t->children[1]->contents; // operator is always second child

//   lval val = eval(t->children[2]); // we store the third child in val

//   int i = 3;

//   while (strstr(t->children[i]->tag, "expr"))
//   {
//     val = eval_op(val, op, eval(t->children[i]));
//     i++;
//   }
//   return val;
// }

lval *lval_eval_sexpr(lval *v);

lval *lval_eval(lval *v)
{
  // If passed an S Expression, evaluate it using lval_eval_sexpr
  if (v->type == LVAL_SEXPR)
  {
    return lval_eval_sexpr(v);
  }
  // If any other type, return it unmodified
  return v;
}

lval *lval_pop(lval *v, int i)
{
  /* Find the item at "i" */
  lval *x = v->cell[i];

  /* Shift memory after the item at "i" over the top */
  memmove(&v->cell[i], &v->cell[i + 1],
          sizeof(lval *) * (v->count - i - 1));

  /* Decrease the count of items in the list */
  v->count--;

  /* Reallocate the memory used */
  v->cell = realloc(v->cell, sizeof(lval *) * v->count);
  return x;
}
lval *lval_take(lval *v, int i)
{
  lval *x = lval_pop(v, i);
  lval_del(v);
  return x;
}
lval *builtin_op(lval *a, char *op)
{

  /* Ensure all arguments are numbers */
  for (int i = 0; i < a->count; i++)
  {
    if (a->cell[i]->type != LVAL_NUM)
    {
      lval_del(a);
      return create_lval_err("Cannot operate on non-number!");
    }
  }

  /* Pop the first element */
  lval *x = lval_pop(a, 0);

  /* If no arguments and sub then perform unary negation */
  if ((strcmp(op, "-") == 0) && a->count == 0)
  {
    x->num = -x->num;
  }

  /* While there are still elements remaining */
  while (a->count > 0)
  {

    /* Pop the next element */
    lval *y = lval_pop(a, 0);

    if (strcmp(op, "+") == 0)
    {
      x->num += y->num;
    }
    if (strcmp(op, "-") == 0)
    {
      x->num -= y->num;
    }
    if (strcmp(op, "*") == 0)
    {
      x->num *= y->num;
    }
    if (strcmp(op, "/") == 0)
    {
      if (y->num == 0)
      {
        lval_del(x);
        lval_del(y);
        x = create_lval_err("Division By Zero!");
        break;
      }
      x->num /= y->num;
    }

    lval_del(y);
  }

  lval_del(a);
  return x;
}

lval *lval_eval_sexpr(lval *v)
{

  // We run eval for each of the children
  for (int i = 0; i < v->count; i++)
  {
    v->cell[i] = lval_eval(v->cell[i]);
  }

  // Check for errors
  for (int i = 0; i < v->count; i++)
  {
    if (v->cell[i]->type == LVAL_ERR)
    {
      return lval_take(v, i);
    }
  }

  // If it is an empty expression, return it
  if (v->count == 0)
  {
    return v;
  }

  /* Single Expression */
  if (v->count == 1)
  {
    return lval_take(v, 0);
  }

  /* Ensure First Element is Symbol */
  lval *f = lval_pop(v, 0);
  if (f->type != LVAL_SYM)
  {
    lval_del(f);
    lval_del(v);
    return create_lval_err("S-expression Does not start with symbol!");
  }

  /* Call builtin with operator */
  lval *result = builtin_op(v, f->sym);
  lval_del(f);
  return result;
}
int main(int argc, char const *argv[])
{
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Symbol = mpc_new("symbol");
  mpc_parser_t *Sexpr = mpc_new("sexpr");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Lispy = mpc_new("lispy");

  mpca_lang(MPCA_LANG_DEFAULT,
            "                                          \
    number : /-?[0-9]+/ ;                    \
    symbol : '+' | '-' | '*' | '/' ;         \
    sexpr  : '(' <expr>* ')' ;               \
    expr   : <number> | <symbol> | <sexpr> ; \
    lispy  : /^/ <expr>* /$/ ;               \
  ",
            Number, Symbol, Sexpr, Expr, Lispy);
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
      lval *x = lval_eval(lval_read(r.output));
      lval_println(x);
      lval_del(x);
    }
    else
    {
      // If Error, print it here
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    free(input);
  }
  mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
  return 0;
}

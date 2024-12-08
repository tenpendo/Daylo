#ifndef LVAL_H
#define LVAL_H

#include <stdio.h>
#include "mpc.h"
#include "types.h"

/* Declare New lval Struct */
struct lval {
  int type;
  /*Basic Types*/
  long num;
  double dec;
  char* err;
  char* sym;
  char* str;
  /* Function */
  lbuiltin builtin;
  lenv* env;
  lval* formals;
  lval* body;
  /* Count and Pointer to a list of "lval*" */
  int count;
  struct lval** cell;
};

/* Create Enumeration of Possible lval Types */
enum { LVAL_ERR, LVAL_NUM, LVAL_DEC, LVAL_SEXPR, LVAL_SYM, LVAL_QEXPR, LVAL_FUN, LVAL_STR };

/* Function Declarations */
lval* lval_num(long x);
lval* lval_dec(double x);
lval* lval_err(char* fmt, ...);
lval* lval_sym(char* s);
lval* lval_str(char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
lval* lval_fun(lbuiltin func);
lval* lval_lambda(lval* formals, lval* body);
lval* lval_copy(lval* v);
lval* lval_add(lval* v, lval* x);
lval* lval_join(lval* x, lval* y);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* lval_eval(lenv* e, lval* v);
lval* lval_call(lenv* e, lval* f, lval* v);
lval* lval_read(mpc_ast_t* t);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read_str(mpc_ast_t* t);

void lval_del(lval* v);
void lval_print(lval* v);
void lval_println(lval* v);
void lval_expr_print(lval* v, char open, char close);
void lval_print_str(lval* v);

int lval_eq(lval* x, lval* y);

char* ltype_name(int t);

#endif
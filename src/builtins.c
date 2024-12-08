#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include "lval.h"
#include "lenv.h"

#define LASSERT(args, cond, fmt, ...) \
  if (!(cond)) { \
    lval* err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args); \
    return err; \
  }

#define LASSERT_TYPE(func, args, index, expect) \
  LASSERT(args, args->cell[index]->type == expect, \
    "Function '%s' passed incorrect type for argument %i. " \
    "Got %s, Expected %s.", \
    func, index, ltype_name(args->cell[index]->type), ltype_name(expect))

#define LASSERT_NUM(func, args, num) \
  LASSERT(args, args->count == num, \
    "Function '%s' passed incorrect number of arguments. " \
    "Got %i, Expected %i.", \
    func, args->count, num)

#define LASSERT_NOT_EMPTY(func, args, index) \
  LASSERT(args, args->cell[index]->count != 0, \
    "Function '%s' passed {} for argument %i.", func, index)

lval* builtin_op(lenv* e, lval* a, char* op) {
  /* Ensure all arguments are numbers */
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM && a->cell[i]->type != LVAL_DEC) {
      lval_del(a);
      return lval_err("Cannot operate on non-number!");
    }
  }

  /* Pop the first element */
  lval* x = lval_pop(a, 0);

  /* If no arguments and sub then perform unary negation */
  if ((strcmp(op, "-") == 0) && a->count == 0) {
    if (x->type == LVAL_NUM) {
      x->num = -x->num;
    } else {
      x->dec = -x->dec;
    }
  }

  /* While there are still elements remaining */
  while (a->count > 0) {
    lval* y = lval_pop(a, 0);

    double x_val = (x->type == LVAL_NUM) ? (double)x->num : x->dec;
    double y_val = (y->type == LVAL_NUM) ? (double)y->num : y->dec;

    if (strcmp(op, "+") == 0) { x_val += y_val; }
    if (strcmp(op, "-") == 0) { x_val -= y_val; }
    if (strcmp(op, "*") == 0) { x_val *= y_val; }
    if (strcmp(op, "/") == 0) {
      if (y_val == 0) {
        lval_del(x); lval_del(y); lval_del(a);
        return lval_err("Division By Zero!");
      }
      x_val /= y_val;
    }
    if (strcmp(op, "%") == 0) {
      if (y_val == 0) {
        lval_del(x); lval_del(y); lval_del(a);
        return lval_err("Division By Zero!");
      }
      x_val = fmod(x_val, y_val);
    }

    lval_del(y);
    
    /* Convert result to most appropriate type */
    if (floor(x_val) == x_val) {
      x->type = LVAL_NUM;
      x->num = (long)x_val;
    } else {
      x->type = LVAL_DEC;
      x->dec = x_val;
    }
  }

  lval_del(a);
  return x;
}

lval* builtin_add(lenv* e, lval* a) { return builtin_op(e, a, "+"); }
lval* builtin_sub(lenv* e, lval* a) { return builtin_op(e, a, "-"); }
lval* builtin_mul(lenv* e, lval* a) { return builtin_op(e, a, "*"); }
lval* builtin_div(lenv* e, lval* a) { return builtin_op(e, a, "/"); }
lval* builtin_mod(lenv* e, lval* a) { return builtin_op(e, a, "%"); }

lval* builtin_var(lenv* e, lval* a, char* func) {
  LASSERT_TYPE(func, a, 0, LVAL_QEXPR);
  
  lval* syms = a->cell[0];
  for (int i = 0; i < syms->count; i++) {
    LASSERT(a, syms->cell[i]->type == LVAL_SYM,
      "Function '%s' cannot define non-symbol. "
      "Got %s, Expected %s.", func,
      ltype_name(syms->cell[i]->type), ltype_name(LVAL_SYM));
  }
  
  LASSERT(a, syms->count == a->count-1,
    "Function '%s' passed too many arguments. "
    "Got %i, Expected %i.", func, 
    syms->count, a->count-1);
    
  for (int i = 0; i < syms->count; i++) {
    if (strcmp(func, "def") == 0) {
      lenv_def(e, syms->cell[i], a->cell[i+1]);
    }
    if (strcmp(func, "=") == 0) {
      lenv_put(e, syms->cell[i], a->cell[i+1]);
    }
  }
  
  lval_del(a);
  return lval_sexpr();
}

lval* builtin_def(lenv* e, lval* a) { return builtin_var(e, a, "def"); }
lval* builtin_put(lenv* e, lval* a) { return builtin_var(e, a, "="); }

lval* builtin_lambda(lenv* e, lval* a) {
  LASSERT_NUM("\\", a, 2);
  LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
  LASSERT_TYPE("\\", a, 1, LVAL_QEXPR);

  for (int i = 0; i < a->cell[0]->count; i++) {
    LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
      "Cannot define non-symbol. Got %s, Expected %s.",
      ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
  }

  lval* formals = lval_pop(a, 0);
  lval* body = lval_pop(a, 0);
  lval_del(a);

  return lval_lambda(formals, body);
}

lval* builtin_list(lenv* e, lval* a) {
  a->type = LVAL_QEXPR;
  return a;
}

lval* builtin_head(lenv* e, lval* a) {
  LASSERT_NUM("head", a, 1);
  LASSERT_TYPE("head", a, 0, LVAL_QEXPR);
  LASSERT_NOT_EMPTY("head", a, 0);

  lval* v = lval_take(a, 0);
  while (v->count > 1) { lval_del(lval_pop(v, 1)); }
  return v;
}

lval* builtin_tail(lenv* e, lval* a) {
  LASSERT_NUM("tail", a, 1);
  LASSERT_TYPE("tail", a, 0, LVAL_QEXPR);
  LASSERT_NOT_EMPTY("tail", a, 0);

  lval* v = lval_take(a, 0);
  lval_del(lval_pop(v, 0));
  return v;
}

lval* builtin_eval(lenv* e, lval* a) {
  LASSERT_NUM("eval", a, 1);
  LASSERT_TYPE("eval", a, 0, LVAL_QEXPR);

  lval* x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
}

lval* builtin_join(lenv* e, lval* a) {
  for (int i = 0; i < a->count; i++) {
    LASSERT_TYPE("join", a, i, LVAL_QEXPR);
  }

  lval* x = lval_pop(a, 0);
  while (a->count) {
    x = lval_join(x, lval_pop(a, 0));
  }

  lval_del(a);
  return x;
}

lval* builtin_min(lenv* e, lval* a) {
  LASSERT(a, a->count > 0, "min function passed no arguments!");

  lval* x = lval_pop(a, 0);
  double min = (x->type == LVAL_NUM) ? (double)x->num : x->dec;
  lval_del(x);

  while (a->count > 0) {
    x = lval_pop(a, 0);
    double val = (x->type == LVAL_NUM) ? (double)x->num : x->dec;
    if (val < min) min = val;
    lval_del(x);
  }

  lval_del(a);
  return (floor(min) == min) ? lval_num((long)min) : lval_dec(min);
}

lval* builtin_max(lenv* e, lval* a) {
  LASSERT(a, a->count > 0, "max function passed no arguments!");

  lval* x = lval_pop(a, 0);
  double max = (x->type == LVAL_NUM) ? (double)x->num : x->dec;
  lval_del(x);

  while (a->count > 0) {
    x = lval_pop(a, 0);
    double val = (x->type == LVAL_NUM) ? (double)x->num : x->dec;
    if (val > max) max = val;
    lval_del(x);
  }

  lval_del(a);
  return (floor(max) == max) ? lval_num((long)max) : lval_dec(max);
}

lval* builtin_cmp(lenv* e, lval* a, char* op) {
  LASSERT_NUM(op, a, 2);
  
  int r;
  if (strcmp(op, "==") == 0) { r = lval_eq(a->cell[0], a->cell[1]); }
  if (strcmp(op, "!=") == 0) { r = !lval_eq(a->cell[0], a->cell[1]); }
  
  if (strcmp(op, ">") == 0 || strcmp(op, "<") == 0 ||
      strcmp(op, ">=") == 0 || strcmp(op, "<=") == 0) {
    LASSERT_TYPE(op, a, 0, LVAL_NUM);
    LASSERT_TYPE(op, a, 1, LVAL_NUM);
    
    if (strcmp(op, ">") == 0) { r = (a->cell[0]->num > a->cell[1]->num); }
    if (strcmp(op, "<") == 0) { r = (a->cell[0]->num < a->cell[1]->num); }
    if (strcmp(op, ">=") == 0) { r = (a->cell[0]->num >= a->cell[1]->num); }
    if (strcmp(op, "<=") == 0) { r = (a->cell[0]->num <= a->cell[1]->num); }
  }
  
  lval_del(a);
  return lval_num(r);
}

lval* builtin_gt(lenv* e, lval* a) { return builtin_cmp(e, a, ">"); }
lval* builtin_lt(lenv* e, lval* a) { return builtin_cmp(e, a, "<"); }
lval* builtin_ge(lenv* e, lval* a) { return builtin_cmp(e, a, ">="); }
lval* builtin_le(lenv* e, lval* a) { return builtin_cmp(e, a, "<="); }
lval* builtin_eq(lenv* e, lval* a) { return builtin_cmp(e, a, "=="); }
lval* builtin_ne(lenv* e, lval* a) { return builtin_cmp(e, a, "!="); }

lval* builtin_if(lenv* e, lval* a) {
  LASSERT_NUM("if", a, 3);
  LASSERT_TYPE("if", a, 0, LVAL_NUM);
  LASSERT_TYPE("if", a, 1, LVAL_QEXPR);
  LASSERT_TYPE("if", a, 2, LVAL_QEXPR);

  lval* x;
  a->cell[1]->type = LVAL_SEXPR;
  a->cell[2]->type = LVAL_SEXPR;

  if (a->cell[0]->num) {
    x = lval_eval(e, lval_pop(a, 1));
  } else {
    x = lval_eval(e, lval_pop(a, 2));
  }

  lval_del(a);
  return x;
}

lval* builtin_or(lenv* e, lval* a) {
  LASSERT_NUM("||", a, 2);
  LASSERT_TYPE("||", a, 0, LVAL_NUM);
  LASSERT_TYPE("||", a, 1, LVAL_NUM);

  int r = (a->cell[0]->num != 0) || (a->cell[1]->num != 0);
  lval_del(a);
  return lval_num(r);
}

lval* builtin_and(lenv* e, lval* a) {
  LASSERT_NUM("&&", a, 2);
  LASSERT_TYPE("&&", a, 0, LVAL_NUM);
  LASSERT_TYPE("&&", a, 1, LVAL_NUM);

  int r = (a->cell[0]->num != 0) && (a->cell[1]->num != 0);
  lval_del(a);
  return lval_num(r);
}

lval* builtin_not(lenv* e, lval* a) {
  LASSERT_NUM("!", a, 1);
  LASSERT_TYPE("!", a, 0, LVAL_NUM);

  int r = !(a->cell[0]->num);
  lval_del(a);
  return lval_num(r);
}

lval* builtin_load(lenv* e, lval* a) {
  LASSERT_NUM("load", a, 1);
  LASSERT_TYPE("load", a, 0, LVAL_STR);

  /* Parse File given by string name */
  mpc_result_t r;
  if (mpc_parse_contents(a->cell[0]->str, Daylo, &r)) {
    /* Read contents */
    lval* expr = lval_read(r.output);
    mpc_ast_delete(r.output);

    /* Evaluate each Expression */
    while (expr->count) {
      lval* x = lval_eval(e, lval_pop(expr, 0));
      if (x->type == LVAL_ERR) { lval_println(x); }
      lval_del(x);
    }

    lval_del(expr);
    lval_del(a);
    return lval_sexpr();
  } else {
    /* Get Parse Error as String */
    char* err_msg = mpc_err_string(r.error);
    mpc_err_delete(r.error);

    /* Create new error message */
    lval* err = lval_err("Could not load Library %s", err_msg);
    free(err_msg);
    lval_del(a);
    return err;
  }
}

lval* builtin_print(lenv* e, lval* a) {
  /* Print each argument followed by a space */
  for (int i = 0; i < a->count; i++) {
    lval_print(a->cell[i]); 
    putchar(' ');
  }

  /* Print a newline and delete arguments */
  putchar('\n');
  lval_del(a);
  return lval_sexpr();
}

lval* builtin_error(lenv* e, lval* a) {
  LASSERT_NUM("error", a, 1);
  LASSERT_TYPE("error", a, 0, LVAL_STR);

  /* Construct Error from first argument */
  lval* err = lval_err(a->cell[0]->str);

  /* Delete arguments and return */
  lval_del(a);
  return err;
}
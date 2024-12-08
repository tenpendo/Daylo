#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"
#include "lval.h"
#include "lenv.h"
#include "builtins.h"

/* If we are compiling on Windows compile these functions */
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* Fake readline function */
char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

/* Otherwise include the editline headers */
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

/* Parser Declarations */
mpc_parser_t* Number;
mpc_parser_t* Symbol;
mpc_parser_t* String;
mpc_parser_t* Comment;
mpc_parser_t* Sexpr;
mpc_parser_t* Qexpr;
mpc_parser_t* Expr;
mpc_parser_t* Daylo;

int main(int argc, char** argv) {
  /* Initialize parsers */
  Number = mpc_new("number");
  String = mpc_new("string");
  Symbol = mpc_new("symbol");
  Comment = mpc_new("comment");
  Sexpr = mpc_new("sexpr");
  Qexpr = mpc_new("qexpr");
  Expr = mpc_new("expr");
  Daylo = mpc_new("Daylo");

  /* Define grammar */
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
      number   : /-?[0-9]+\\.?[0-9]*/ ;                  \
      string   : /\"(\\\\.|[^\"])*\"/ ;                  \
      symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;      \
      comment  : /;[^\\r\\n]*/ ;                         \
      sexpr    : '(' <expr>* ')' ;                       \
      qexpr    : '{' <expr>* '}' ;                       \
      expr     : <number>  | <string> | <symbol> |       \
                 <comment> | <sexpr>  | <qexpr> ;        \
      Daylo    : /^/ <expr>* /$/ ;                       \
    ",
    Number, String, Symbol, Comment, Sexpr, Qexpr, Expr, Daylo);

  /* Create environment */
  lenv* e = lenv_new();
  lenv_add_builtins(e);

  /* Interactive Prompt or File Loading */
  if (argc >= 2) {
    /* loop over each supplied filename (starting from 1) */
    for (int i = 1; i < argc; i++) {
      /* Argument list with a single argument, the filename */
      lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));
      
      /* Pass to builtin load and get the result */
      lval* x = builtin_load(e, args);
      
      /* If the result is an error be sure to print it */
      if (x->type == LVAL_ERR) { lval_println(x); }
      lval_del(x);
    }
  } else {
    /* Print Version and Exit Information */
    puts("Daylo Version 0.0.0.0.2");
    puts("Press Ctrl+c to Exit\n");

    /* In a never ending loop */
    while (1) {
      /* Output our prompt and get input */
      char* input = readline("Daylo> ");
      add_history(input);

      /* Attempt to parse the user input */
      mpc_result_t r;
      if (mpc_parse("<stdin>", input, Daylo, &r)) {
        /* On success evaluate the AST */
        lval* x = lval_eval(e, lval_read(r.output));
        lval_println(x);
        lval_del(x);
        
        mpc_ast_delete(r.output);
      } else {
        /* Otherwise print the error */
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
      }

      /* Free retrieved input */
      free(input);
    }
  }

  /* Cleanup */
  lenv_del(e);
  mpc_cleanup(8, Number, String, Symbol, Comment, Sexpr, Qexpr, Expr, Daylo);

  return 0;
}
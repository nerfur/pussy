#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"

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

#elif __OpenBSD__

#include <readline/readline.h>
#include <readline/history.h> 

#else 

#include <editline/readline.h>
#include <editline/history.h>

#endif

long eval_op(long x, char* op, long y) {
  if (strcmp(op, "+") == 0) { return x + y; }
  if (strcmp(op, "-") == 0) { return x - y; }
  if (strcmp(op, "*") == 0) { return x * y; }
  if (strcmp(op, "/") == 0) { return x / y; }
  return 0;
}

long eval(mpc_ast_t* t) {
  
  /* If tagged as number return it directly, otherwise expression. */ 
  if (strstr(t->tag, "number")) { return atoi(t->contents); }
  
  /* The operator is always second child. */
  char* op = t->children[1]->contents;
  
  /* We store the third child in `x` */
  long x = eval(t->children[2]);
  
  /* Iterate the remaining children, combining using our operator */
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }
  
  return x;  
}

int main(int argc, char** argv) {

/* Create Some Parsers */
mpc_parser_t* Number   = mpc_new("number");
mpc_parser_t* Operator = mpc_new("operator");
mpc_parser_t* Expr     = mpc_new("expr");
mpc_parser_t* Pussy    = mpc_new("pussy");

/* Define them with the following Language */
mpca_lang(MPCA_LANG_DEFAULT,
  "                                                     \
    number   : /-?[0-9]+/ ;                             \
    operator : '+' | '-' | '*' | '/' ;                  \
    expr     : <number> | '(' <operator> <expr>+ ')' ;  \
    pussy    : /^/ <operator> <expr>+ /$/ ;             \
  ",
  Number, Operator, Expr, Pussy);

  /* Print Version and Exit Information */
  puts("Pussy Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

  /* In a never ending loop */
  while (1) {

  /* Output our prompt */
    char* input = readline("=^.^= ? ");

    /* Adding entered command to history */
    add_history(input);

    /* Attempt to Parse the user Input */
    mpc_result_t r; 
    if (mpc_parse("<stdin>", input, Pussy, &r)) {
      /* On Success Print the AST */
      long result = eval(r.output);
      printf("%li\n\nPurr.\n", result);
      mpc_ast_delete(r.output);
    } else {
      /* Otherwise Print the Error */
      mpc_err_print(r.error);
      printf("\n\n Meow! \n");
      mpc_err_delete(r.error);
}


    /* Freeing variable's memory */
    free(input);
  }

/* Undefine and Delete our Parsers */
mpc_cleanup(4, Number, Operator, Expr, Pussy);

  return 0;
}

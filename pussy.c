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

/* Declare New pval Struct */
typedef struct pval {
  int type;
  long num;
  char* err;
  char* sym;
  int count;
  struct pval** cell;
} pval;

/* Create Enumeration of Possible pval Types */
enum { PVAL_NUM, PVAL_ERR, PVAL_SYM, PVAL_SEXPR };

/* Create Enumeration of Possible Error Types */
enum { PERR_DIV_ZERO, PERR_BAD_OP, PERR_BAD_NUM };

/* Create a new number type pointer pval */
pval* pval_num(long x) {
  pval* v = malloc(sizeof(pval));
  v->type = PVAL_NUM;
  v->num  = x;
  return v;
}

/* Create a new error type pointer pval */
pval* pval_err(char* m) {
  pval* v = malloc(sizeof(pval));
  v->type = PVAL_ERR;
  v->err  = malloc(strlen(m)+1);
  strcpy(v->err, m);
  return v;
}

/* Create a new symbol type pointer pval */
pval* pval_sym(char* s) {
  pval* v = malloc(sizeof(pval));
  v->type = PVAL_SYM;
  v->sym  = malloc(strlen(s)+1);
  strcpy(v->sym, m);
  return v;
}

/* Create a new S-Expression type pointer pval */
pval* pval_sexpr(void) {
  pval* v = malloc(sizeof(pval));
  v->type = PVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v; 
}

/*delete and free pval */

void pval_delete (pval* v) {
  /* detect type and prepare content of structure for clean free */
  switch (v->type) {
    case PVAL_NUM: break;
    case PVAL_ERR: free(v->err); break;
    case PVAL_SYM: free(v->sym); break; 
    case PVAL_SEXPR:
      for (int i = 0; i < v->count; i++) {
	pval_del(v->cell[i]);
      }
      free(v->cell);
    break;
  }
  free(v);
}

/* Print an "pval" */
void pval_print(pval v) {
  switch (v.type) {
    /* In the case the type is a number print it, then 'break' out of the switch. */
    case PVAL_NUM: printf("%li\n\nPurr.", v.num); break;

    /* In the case the type is an error */
    case PVAL_ERR:
      /* Check What exact type of error it is and print it */
      if (v.err == PERR_DIV_ZERO) { printf("Error: Division By Zero!"); }
      if (v.err == PERR_BAD_OP)   { printf("Error: Invalid Operator!"); }
      if (v.err == PERR_BAD_NUM)  { printf("Error: Invalid Number!"); }
      printf("\n\nMeow?");
    break;
  }
}

void pval_println(pval v) { pval_print(v); putchar('\n'); }

pval eval_op(pval x, char* op, pval y) {

  /* If either value is an error return it */
  if (x.type == PVAL_ERR) { return x; }
  if (y.type == PVAL_ERR) { return y; }

  /* do math */
  if (strcmp(op, "+") == 0) { return pval_num(x.num + y.num); }
  if (strcmp(op, "-") == 0) { return pval_num(x.num - y.num); }
  if (strcmp(op, "*") == 0) { return pval_num(x.num * y.num); }
  if (strcmp(op, "/") == 0) { 
     /* If second operand is zero return error instead of result */
     return y.num == 0 ? pval_err(PERR_DIV_ZERO) : pval_num(x.num / y.num);
  }
  
    return pval_err(PERR_BAD_OP);
}

pval eval(mpc_ast_t* t) {
  
  /* If tagged as number return it directly, otherwise expression. */ 
  if (strstr(t->tag, "number")) { 
     errno = 0;
     long x = strtol(t->contents, NULL, 10);
     return errno != ERANGE ? pval_num(x) : pval_err(PERR_BAD_NUM);
  }
  
  /* The operator is always second child. */
  char* op = t->children[1]->contents;  
  pval x = eval(t->children[2]);
  
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
mpc_parser_t* Symbol   = mpc_new("symbol");
mpc_parser_t* Sexpr    = mpc_new("sexpr");
mpc_parser_t* Expr     = mpc_new("expr");
mpc_parser_t* Pussy    = mpc_new("pussy");

/* Define them with the following Language */
mpca_lang(MPCA_LANG_DEFAULT,
  "                                                     \
    number   : /-?[0-9]+/ ;                             \
    symbol   : '+' | '-' | '*' | '/' ;                  \
    sexpr    : '(' <expr>* ')' ;			\
    expr     : <number> | <symbol> | <sexpr> ;		\
    pussy    : /^/ <expr>* /$/ ;             		\
  ",
  Number, Symbol, Sexpr, Expr, Pussy);

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
      pval result = eval(r.output);
      pval_println(result);
      mpc_ast_delete(r.output);
    } else {
      /* Otherwise Print the Error */
      mpc_err_print(r.error);
      printf("\nMeow! \n");
      mpc_err_delete(r.error);
}


    /* Freeing variable's memory */
    free(input);
  }

/* Undefine and Delete our Parsers */
mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Pussy);

  return 0;
}

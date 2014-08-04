#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char** argv) {

  /* Print Version and Exit Information */
  puts("Pussy Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

  /* In a never ending loop */
  while (1) {

    /* Output our prompt */
    char* input = readline("=^.^= ? ");

    /* Adding entered command to history */
    add_history(input);

    /* Echo input back to user */
    printf("\n%s\nPurr.\n", input);

    /* Freeing variable's memory */
    free(input);
  }

  return 0;
}

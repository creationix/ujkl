#include <unistd.h>
#include "line-reader.c"
#include "symbol.c"

static const char *onLine(uint8_t *line, uint8_t len) {
  writeString(line, len);
  writeChar('\n');
  return 0;
}

#include <stdio.h>

int main() {
  printf("+ %d\n", matchSymbol("+", 1));
  printf("and %d\n", matchSymbol("and", 0));
  printf("this %d\n", matchSymbol("this", 0));
  printf("is %d\n", matchSymbol("is", 0));
  printf("a %d\n", matchSymbol("a", 0));
  printf("this %d\n", matchSymbol("this", 0));
  printf("this %d\n", matchSymbol("this", 0));
  printf("and %d\n", matchSymbol("and", 0));
  printf("is %d\n", matchSymbol("is", 0));
  printf("def %d\n", matchSymbol("def", 0));
  printf("let %d\n", matchSymbol("let", 0));
  printf("set %d\n", matchSymbol("set", 0));
  printf("- %d\n", matchSymbol("-", 0));

  startEditor("> ", onLine);
  while(stepEditor());
  stopEditor();
  return 0;
}

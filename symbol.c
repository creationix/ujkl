#ifndef SYMBOL_C
#define SYMBOL_C

#include <stdlib.h>
#include "types.h"

static const builtin_t builtins[] = {
  // atoms
  {"nil", 0},
  {"true", 0},
  {"false", 0},
  // builtins
  {"def", 0},
  {"set", 0},
  {".", 0},
  {"index", 0},
  {"lambda", 0},
  {"λ", 0},
  {"if", 0},
  {"unless", 0},
  {"?", 0},
  {"and", 0},
  {"or", 0},
  {"not", 0},
  {"print", 0},
  {"list", 0},
  {"read", 0},
  {"write", 0},
  {"exec", 0},
  {"escape", 0},
  {"sleep", 0},
  {"macro", 0},
  {"concat", 0},
  {"flat", 0},
  {"join", 0},
  {"shuffle", 0},
  // variable
  {"for", 0},
  {"for*", 0},
  {"map", 0},
  {"map*", 0},
  {"i-map", 0},
  {"i-map*", 0},
  {"iter", 0},
  {"reduce", 0},
  {"while", 0},
  {"do", 0},
  // operator
  {"+", 0},
  {"-", 0},
  {"*", 0},
  {"×", 0},
  {"/", 0},
  {"÷", 0},
  {"%", 0},
  {"<", 0},
  {"<=", 0},
  {"≤", 0},
  {">", 0},
  {">=", 0},
  {"≥", 0},
  {"=", 0},
  {"!=", 0},
  {"≠", 0},
  {0, 0},
};

static char *symbols;
static size_t symbols_len;

const char *getSymbol(int index) {
  if (index >= 0) {
    return builtins[index].name;
  }
  const char *names = symbols;
  while (++index) {
    while (*names++);
  }
  return names;
}

// Negative index is used for user-defined symbols
int matchSymbol(const char *word, int len) {
  // Calculate the length if not given (assuming null terminated)
  if (!len) { while(word[len]) {len++;} }
  // Find the builtin index with matching name
  int i = 0;
  const char *name;
  while ((name = builtins[i].name)) {
    int j = 0;
    while (name[j] && j < len && word[j] == name[j]) { j++; }
    if (j == len && name[j] == 0) {
      return i;
    }
    i++;
  }
  // If this is the first user-defined symbol, initialize table
  if (!symbols) {
    // Allocate the initial symbols buffer
    symbols_len = (size_t)(len + 2);
    symbols = malloc(symbols_len);
    for (int j = 0; j < len; j++) {
      symbols[j] = word[j];
    }
    symbols[len] = 0;
    symbols[len + 1] = 0;
    return -1;
  }

  // Otherwise look for symbol.
  i = -1;
  char *names = symbols;
  while (*names) {
    int j = 0;
    while (*names && j < len && *names == word[j]) {
      j++;
      names++;
    }
    if (j == len && !(*names)) {
      return i;
    }
    while (*names++);
    i--;
  }
  // Allocate more space if needed
  size_t needed = (size_t)((names - symbols) + len + 2);
  if (needed > symbols_len) {
    symbols_len = needed;
    symbols = realloc(symbols, symbols_len);
  }
  // Append the new symbol to the list
  for (int j = 0; j < len; j++) {
    *names++ = word[j];
  }
  *names++ = 0;
  *names++ = 0;
  return i;
}

#endif

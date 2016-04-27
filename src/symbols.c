
#ifndef SYMBOLS_C
#define SYMBOLS_C

#include <stdlib.h>

#ifndef SYMBOLS_BLOCK_SIZE
#define SYMBOLS_BLOCK_SIZE 128
#endif

#ifndef API
#define API
#endif

typedef struct {
  const char* name;
  void* data;
} builtin_t;

static char *symbols;
static size_t symbols_len;
API const builtin_t *builtins;

static void symbols_resize(size_t needed) {
  if (needed < symbols_len) return;
  // Allocate memory in blocks to reduce fragmentation
  // and batch allocations.
  size_t new_len = needed + (SYMBOLS_BLOCK_SIZE - needed % SYMBOLS_BLOCK_SIZE);
  symbols = realloc(symbols, new_len);
  for (size_t j = symbols_len; j < new_len; j++) {
    symbols[j] = 0;
  }
  symbols_len = new_len;
}

// Resolve a symbol index to a null-terminated string.
API const char *symbols_get(int index) {
  if (index >= 0) {
    return builtins[index].name;
  }
  const char *names = symbols;
  while (++index) {
    while (*names++);
  }
  return names;
}


// Lookup or store a symbol.  Builtins will return positive indexes and
// user-defined symbols will be negative.
API int symbols_set(const char *word, size_t len) {
  // Calculate the length if not given (assuming null terminated)
  if (!len) { while(word[len]) {len++;} }

  // First try to match against builtins and return positive index if found.
  int idx = 0;
  const char *name;
  while ((name = builtins[idx].name)) {
    size_t j = 0;
    while (name[j] && j < len && word[j] == name[j]) { j++; }
    if (j == len && name[j] == 0) {
      return idx;
    }
    idx++;
  }
  idx = -1;

  // If not found, look for symbol in user table.
  // This is a giant `char*` containing consecutives null terminated strings.
  size_t i = 0; // This is our byte offset into the symbols table
  while (i < symbols_len && symbols[i]) {
    size_t j = 0;
    // Match the body of the word against the table entry.
    while (i < symbols_len && j < len && symbols[i] == word[j]) {
      i++;
      j++;
    }
    // If it matched completely, we found it!
    if (j == len && symbols[i] == 0) {
      return idx;
    }
    // If we didn't match, skip to end of word.
    while (i < symbols_len && symbols[i++]);
    // Decrement the logical index.
    idx--;
  }

  // Allocate more space if needed
  symbols_resize(i + len + 1);

  // Append the new symbol to the list
  for (size_t j = 0; j < len; j++) {
    symbols[i + j] = word[j];
  }
  symbols[i + len] = 0;
  return idx;
}

#endif

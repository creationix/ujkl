#include <assert.h>

#define SYMBOLS_BLOCK_SIZE 128
#define PAIRS_BLOCK_SIZE 16
#define THEME tim
#define API static

#include "src/builtins.c"
#include "src/data.c"
#include "src/dump.c"
#include "src/editor.c"
#include "src/print.c"
#include "src/symbols.c"

#include "test.c"

static void parse(const char *data) {
  print(data);
  print_char('\n');
}

int main() {
  setup_builtins();

  test();

  // Start the line editor
  prompt = "> ";
  onLine = parse;
  while (editor_step());

  return 0;

}

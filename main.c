#include <assert.h>

#define SYMBOLS_BLOCK_SIZE 128
#define PAIRS_BLOCK_SIZE 16
#define THEME tim
#define API static

#include "src/data.c"
#include "src/dump.c"
#include "src/editor.c"
#include "src/print.c"
#include "src/runtime.c"
#include "src/symbols.c"

#include "test.c"

static void parse(const char *data) {
  print(data);
  print_char('\n');
}

static value_t _list(value_t env, value_t args) {
  (void)(env);
  while (args.type == PairType) {
    pair_t pair = getPair(args);
    pair.left = eval(env, pair.left);
    setPair(args, pair);
    args = pair.right;
  }
  return args;
}

// Passed values through unaffected
static value_t _quote(value_t env, value_t args) {
  (void)(env);
  return args;
}

static value_t _add(value_t env, value_t args) {
  int sum = 0;
  args = _list(env, args);
  while (args.type == PairType) {
    print("\n*");
    dump(args);
    pair_t pair = getPair(args);
    if (pair.left.type == IntegerType) {
      sum += pair.left.data;
    }
    args = pair.right;
  }
  return Integer(sum);
}

fn2(_mget, map, key, return mget(map, key);)
fn3(_mset, map, key, value, return mset(map, key, value);)
fn2(_mhas, map, key, return mhas(map, key);)

static const builtin_t *functions = (const builtin_t[]){
  {"list", _list},
  {"quote", _quote},
  {"+", _add},
  {"mget", _mget},
  {"mset", _mset},
  {"mhas", _mhas},
  {0,0},
};

int main() {
  symbols_init(functions);

  test();

  // Start the line editor
  prompt = "> ";
  onLine = parse;
  while (editor_step());

}

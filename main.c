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
  value_t node = args;
  while (node.type == PairType) {
    pair_t pair = getPair(node);
    pair.left = eval(env, pair.left);
    setPair(node, pair);
    node = pair.right;
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
    pair_t pair = getPair(args);
    if (pair.left.type == IntegerType) {
      sum += pair.left.data;
    }
    args = pair.right;
  }
  return Integer(sum);
}

static value_t _sub(value_t env, value_t args) {
  int sum;
  args = _list(env, args);
  bool first = true;
  while (args.type == PairType) {
    pair_t pair = getPair(args);
    if (pair.left.type == IntegerType) {
      if (first) {
        first = false;
        sum = pair.left.data;
      }
      else {
        sum -= pair.left.data;
      }
    }
    args = pair.right;
  }
  return first ? Undefined : Integer(sum);
}

static value_t _mul(value_t env, value_t args) {
  int sum = 1;
  args = _list(env, args);
  while (args.type == PairType) {
    pair_t pair = getPair(args);
    if (pair.left.type == IntegerType) {
      sum *= pair.left.data;
    }
    args = pair.right;
  }
  return Integer(sum);
}

static value_t _div(value_t env, value_t args) {
  int sum;
  args = _list(env, args);
  bool first = true;
  while (args.type == PairType) {
    pair_t pair = getPair(args);
    if (pair.left.type == IntegerType) {
      if (first) {
        first = false;
        sum = pair.left.data;
      }
      else {
        sum /= pair.left.data;
      }
    }
    else {
      return Undefined;
    }
    args = pair.right;
  }
  return first ? Undefined : Integer(sum);
}

fn2(_mget, map, key, return mget(map, key);)
fn3(_mset, map, key, value, return mset(map, key, value);)
fn2(_mhas, map, key, return mhas(map, key);)

static const builtin_t *functions = (const builtin_t[]){
  {"list", _list},
  {"quote", _quote},
  {"+", _add},
  {"-", _sub},
  {"*", _mul},
  {"/", _div},
  {"mget", _mget},
  {"mset", _mset},
  {"mhas", _mhas},
  {0,0},
};

int main() {
  symbols_init(functions);

  value_t env = List(
    Mapping(name, Symbol("Tim")),
    Mapping(age, Integer(34)),
    Mapping(isProgrammer, True)
  );
  mset(env, Symbol("tim"), reverse(env));
  mset(env, Symbol("add"), Symbol("+"));

  print("env: ");
  dump(env);

  const value_t *expressions = (const value_t[]){
    List(Symbol("-"),Integer(5),Integer(2)),
    List(Symbol("+"),Integer(1),Integer(2),Integer(3)),
    List(Symbol("add"),Integer(1),Integer(2),Integer(3)),
    List(Symbol("+"),
      List(Symbol("*"),Integer(2),Integer(3)),
      List(Symbol("/"),Integer(10),Integer(3)),
      Integer(1)
    ),
    List(Symbol("+"),Integer(1),Integer(2)),
    List(Integer(1),Integer(2),Integer(3)),
    Integer(42),
    Symbol("+"),
    Symbol("what"),
    Symbol("name"),
    List(Symbol("list"), Symbol("age"), Symbol("isProgrammer")),
    List(Symbol("quote"), Symbol("age"), Symbol("isProgrammer")),
    List(Symbol("age"), Symbol("isProgrammer")),
    Symbol("tim"),
    List(Symbol("tim")),
    Nil,
  };

  for (int i = 0; !isNil(expressions[i]); i++) {
    print("> ");
    dump(expressions[i]);
    dump(eval(env, expressions[i]));
  }

  // Start the line editor
  prompt = "> ";
  onLine = parse;
  while (editor_step());

}

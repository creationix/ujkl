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

// Define a function
static value_t _def(value_t env, value_t args) {
  value_t fn = cdr(args);
  mset(env, car(args), fn);
  return fn;
}

static value_t _set(value_t env, value_t args) {
  value_t value = eval(env, cdar(args));
  mset(env, car(args), value);
  return value;
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

static value_t _print(value_t env, value_t args) {
  args = _list(env, args);
  value_t node = args;
  while (node.type == PairType) {
    pair_t pair = getPair(node);
    _dump(pair.left, Nil);
    print_char(' ');
    node = pair.right;
  }
  print(COFF"\n");
  return Nil;
}

fn2(_mget, map, key, return mget(map, key);)
fn3(_mset, map, key, value, return mset(map, key, value);)
fn2(_mhas, map, key, return mhas(map, key);)

static const builtin_t *functions = (const builtin_t[]){
  {"list", _list},
  {"quote", _quote},
  {"def", _def},
  {"set", _set},
  {"+", _add},
  {"-", _sub},
  {"*", _mul},
  {"/", _div},
  {"mget", _mget},
  {"mset", _mset},
  {"mhas", _mhas},
  {"print", _print},
  {0,0},
};

int main() {
  symbols_init(functions);

  value_t env = List(
    Mapping(name, Symbol("Tim")),
    Mapping(age, Integer(34)),
    Mapping(isProgrammer, True)
  );
  env = cons(cons(Symbol("tim"),env),env);
  // mset(env, Symbol("tim"), env);
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
    List(Symbol("def"),
      Symbol("greet"),
      List(Symbol("person")),
      List(Symbol("print"),
        cons(Symbol("quote"),Symbol("hello")),
        List(Symbol("mget"),Symbol("person"),cons(Symbol("quote"),Symbol("name"))))
    ),
    List(Symbol("set"),
      Symbol("tim"),
      List(
        Symbol("quote"),
        Mapping(name,Symbol("Tim")),
        Mapping(age,Integer(34)),
        Mapping(is-programmer,True)
      )
    ),
    List(Symbol("greet"),Symbol("tim")),
    cons(Symbol("list"),Integer(42)),
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

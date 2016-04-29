#ifndef BUILTINS_C
#define BUILTINS_C

#include "types.h"

// input is (environment . args)
// where environment is a map and args is a cons list

API value_t eval(value_t env, value_t expr) {
  if (expr.type == SymbolType) {
    // User symbols look up value in environment,
    // builtins return themselves.
    return expr.data < 0 ? mget(env, expr) : expr;
  }
  if (expr.type == PairType) {
    value_t fn = eval(env, car(expr));
    // Native function.
    if (fn.type == SymbolType && fn.data >= 0) {
      return symbols_get_fn(fn.data)(env, cdr(expr));
    }
    // TODO: interpret user-defined function instance
    return Undefined;
  }
  return expr;
}

#define fn2(name, a, b, body) \
static value_t name(value_t env, value_t args) { \
  value_t a = eval(env, car(args)); \
  args = cdr(args);                 \
  value_t b = eval(env, car(args)); \
  body \
}

#define fn3(name, a, b, c, body) \
static value_t name(value_t env, value_t args) { \
  value_t a = eval(env, car(args)); \
  args = cdr(args);                 \
  value_t b = eval(env, car(args)); \
  args = cdr(args);                 \
  value_t c = eval(env, car(args)); \
  body \
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


API void setup_builtins() {
  symbols_init(functions);
}


// "def",
// "set",
// ".",
// "index",
// "lambda",
// "λ",
// "if",
// "unless",
// "?",
// "and",
// "or",
// "not",
// "print",
// "list",
// "read",
// "write",
// "exec",
// "escape",
// "sleep",
// "macro",
// "concat",
// "flat",
// "join",
// "shuffle",
// // variable
// "for",
// "for*",
// "map",
// "map*",
// "i-map",
// "i-map*",
// "iter",
// "reduce",
// "while",
// "do",
// // operator
// "+",
// "-",
// "*",
// "×",
// "/",
// "÷",
// "%",
// "<",
// "<=",
// "≤",
// ">",
// ">=",
// "≥",
// "=",
// "!=",
// "≠",

#endif

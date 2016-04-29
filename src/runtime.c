#ifndef RUNTIME_C
#define RUNTIME_C

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

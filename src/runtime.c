#ifndef RUNTIME_C
#define RUNTIME_C

#include "types.h"

// input is (environment . args)
// where environment is a map and args is a cons list


#ifdef TRACE
static int indent = 0;
static const char *space = "                                                  ";
#endif

API value_t eval(value_t env, value_t expr) {

#ifdef TRACE
  // print_string(space, indent);
  // print("env: ");
  // dump(env);
  print_string(space, indent);
  print("in:  ");
  dump(expr);
  indent++;
#endif

  if (expr.type == SymbolType) {
    // User symbols look up value in environment,
    // builtins return themselves.
    expr =  expr.data < 0 ? mget(env, expr) : expr;
  }
  else if (expr.type == PairType) {
    value_t fn = eval(env, car(expr));
    // Native function.
    if (fn.type == SymbolType && fn.data >= 0) {
      expr = symbols_get_fn(fn.data)(env, cdr(expr));
    }
    else {
      expr = cdr(expr);
      // Apply arguments to parameters
      value_t subEnv = Nil;
      value_t params = car(fn);
      value_t args = expr;
      while (params.type == PairType) {
        value_t name = car(params);
        value_t value;
        if (args.type == PairType) {
          value = eval(env, car(args));
          args = cdr(args);
        }
        else {
          value = Undefined;
        }
        subEnv = mset(subEnv, name, value);
        params = cdr(params);
      }
      value_t body = cdr(fn);
      expr = Undefined;
      while (body.type == PairType) {
        expr = eval(subEnv, car(body));
        body = cdr(body);
      }
    }
  }
#ifdef TRACE
  indent--;
  print_string(space, indent);
  print("out: ");
  dump(expr);
#endif
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

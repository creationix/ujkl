#ifndef RUNTIME_C
#define RUNTIME_C

#include "types.h"

// input is (environment . args)
// where environment is a map and args is a cons list

#ifdef TRACE
static int indent = 0;
static const char *space = "                                                  ";

#ifdef TRACE_FULL
static void print_type(value_t val, int depth) {
  switch (val.type) {
    case PairType: print("Pair"); break;
    case AtomType: print("Atom"); break;
    case IntegerType: print("Integer"); break;
    case SymbolType: print("Symbol"); break;
  }
  print_char(':');
  print_int(val.data);
  if (val.type == PairType && depth >= 0) {
    pair_t pair = get_pair(val);
    print_char('<');
    print_type(pair.left, depth - 1);
    print_char(' ');
    print_type(pair.right, depth - 1);
    print_char('>');
  }
}

static void full_dump(value_t val) {
  print_type(val,2);
  print(" ");
  dump(val);
}
#else
#define full_dump dump
#endif
#endif

// FN is pre-evaluated
// args is list of unevaluated arguments
API value_t apply(value_t env, value_t fn, value_t args) {

  // Native function.
  if (fn.type == SymbolType && fn.data >= 0) {
    api_fn native = symbols_get_fn(fn.data);
    return native(env, args);
  }

  // Create a new empty environment (no closures for now)
  value_t subEnv = Nil;
  // Apply arguments to parameters
  value_t params = car(fn);
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
    subEnv = table_set(subEnv, name, value);
    params = cdr(params);
  }

  // Run each value in the body one at a time returning the last value
  value_t body = cdr(fn);
  return list_each(subEnv, body, eval);
}

API value_t eval(value_t env, value_t expr) {

#ifdef TRACE
  // print_string(space, indent);
  // print("env: ");
  // dump(env);
  print_string(space, indent);
  print("in:  ");
  full_dump(expr);
  indent++;
#endif

  // Resolve user variables to entry in environment.
  // Builtins return themselves.
  if (expr.type == SymbolType) {
    expr = 1 ? expr : Nil;
    expr = expr.data < 0 ? table_get(env, expr) : expr;
  }

  else if (expr.type == PairType) {
    expr = apply(env, eval(env, car(expr)), cdr(expr));
  }
#ifdef TRACE
  indent--;
  print_string(space, indent);
  print("out: ");
  full_dump(expr);
#endif
  return expr;
}

#endif

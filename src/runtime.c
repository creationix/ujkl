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


static value_t __eval(value_t env, value_t val) {
  // Symbols look up in environment or return self for builtins.
  if (val.type == SymbolType) {
    return val.data < 0 ? table_get(env, val) : val;
  }
  // Simple types are returned unchanged.
  if (val.type != PairType) return val;
  value_t head = next(&val);
  if (head.type == SymbolType && head.data >= 0 && head.data < first_fn) {
    // For keywords, inject environment and don't evaluate arguments.
    return apply(head, cons(env, val));
  }
  // For everything else, pre-eval the arguments.
  value_t copy = cons(eval(env, head), Nil);
  value_t cnode = copy;
  while (val.type == PairType) {
    value_t nextNode = cons(eval(env, next(&val)), Nil);
    set_cdr(cnode, nextNode);
    cnode = nextNode;
  }
  #ifdef TRACE
    print_string(space, indent - 1);
    print("mid: ");
    full_dump(copy);
  #endif
  // And apply as normal
  head = next(&copy);
  return apply(head, copy);
}

API value_t eval(value_t env, value_t val) {
  #ifdef TRACE
    print_string(space, indent);
    print("in:  ");
    full_dump(val);
    indent++;
  #endif
  value_t res = __eval(env, val);
  #ifdef TRACE
    indent--;
    print_string(space, indent);
    print("out: ");
    full_dump(res);
  #endif
  return res;
}

API value_t block(value_t env, value_t body) {
  value_t result = Undefined;
  while (body.type == PairType) {
    result = eval(env, next(&body));
  }
  return result;
}

// args is fn followed by arguments to apply to fn
API value_t apply(value_t fn, value_t args) {
  // Native function.
  if (fn.type == SymbolType && fn.data >= 0) {
    api_fn native = symbols_get_fn(fn.data);
    return native(args);
  }
  // Create a new empty environment.
  value_t subEnv = Nil;
  // Apply arguments to parameters
  value_t params = next(&fn);
  while (params.type == PairType) {
    subEnv = table_set(subEnv, next(&params), next(&args));
  }
  return block(subEnv, fn);
}


#endif

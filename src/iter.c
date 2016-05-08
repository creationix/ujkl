#ifndef ITER_C
#define ITER_C

#include "types.h"

static void iter_int(value_t iter, value_t ctx, callback_t fn) {
  if (iter.data == 0) return;
  int start, end, incr;
  if (iter.data > 0) {
    start = 0, end = iter.data, incr = 1;
  }
  else {
    start = -iter.data, end = 0, incr = -1;
  }
  while (start != end) {
    fn(ctx, cons(Integer(start), Nil));
    start += incr;
  }
}

static void iter_list(value_t iter, value_t ctx, callback_t fn) {
  while (iter.type == PairType) {
    fn(ctx, cons(next(&iter), Nil));
  }
}

static void iter_sym(value_t iter, value_t ctx, callback_t fn) {
  const char* data = symbols_get_name(iter.data);
  while (*data) {
    fn(ctx, cons(Integer(*data++), Nil));
  }
}

API void iter_any(value_t iter, value_t ctx, callback_t fn) {
  if (iter.type == IntegerType) iter_int(iter, ctx, fn);
  else if (iter.type == SymbolType) iter_sym(iter, ctx, fn);
  else if (is_list(iter)) iter_list(iter, ctx, fn);
  else fn(ctx, cons(iter, Nil));
}

#endif

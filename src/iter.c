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
    start = -iter.data - 1, end = -1, incr = -1;
  }
  while (start != end) {
    value_t args = cons(Integer(start), Nil);
    fn(ctx, args);
    free_cell(args);
    start += incr;
  }
}

static void iter_list(value_t iter, value_t ctx, callback_t fn) {
  while (iter.type == PairType) {
    value_t args = cons(next(&iter), Nil);
    fn(ctx, args);
    free_cell(args);
  }
}

static void iter_sym(value_t iter, value_t ctx, callback_t fn) {
  const char* data = symbols_get_name(iter.data);
  while (*data) {
    value_t args = cons(Integer(*data++), Nil);
    fn(ctx, args);
    free_cell(args);
  }
}

API void iter_any(value_t iter, value_t ctx, callback_t fn) {
  if (iter.type == IntegerType) iter_int(iter, ctx, fn);
  else if (iter.type == SymbolType) iter_sym(iter, ctx, fn);
  else if (is_list(iter)) iter_list(iter, ctx, fn);
  else {
    value_t args = cons(iter, Nil);
    fn(ctx, args);
    free_cell(args);
  }
}

#endif

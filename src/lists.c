#ifndef LISTS_C
#define LISTS_C

#include "types.h"
#include <stdlib.h> // for realloc

API bool is_list(value_t val) {
  while (val.type == PairType) {
    val = cdr(val);
  }
  return isNil(val);
}

API int list_length(value_t list) {
  int len = 0;
  while (list.type == PairType) {
    len++;
    list = cdr(list);
  }
  return len;
}

API value_t list_reverse(value_t list) {
  value_t copy = Nil;
  while (list.type == PairType) {
    pair_t pair = getPair(list);
    copy = cons(pair.left, copy);
    list = pair.right;
  }
  return copy;
}

API value_t list_append(value_t list, value_t data) {
  if (isNil(list)) return data;
  value_t node = list;
  while (node.type == PairType) {
    value_t next = cdr(node);
    if (isNil(node)) {
      set_cdr(node, data);
      return list;
    }
    node = next;
  }
  return Undefined;
}

API value_t list_sort(value_t list) {
  if (isNil(list)) return Nil;
  if (list.type != PairType) return Undefined;
  value_t before = Nil, after = Nil;
  pair_t pair = getPair(list);
  value_t node = pair.right;
  while (node.type == PairType) {
    pair_t next = getPair(node);
    if (pair.left.raw < next.left.raw) {
      before = cons(next.left, before);
    }
    else {
      after = cons(next.left, after);
    }
    node = next.right;
  }
  return list_append(
    list_sort(before),
    cons(pair.left, list_sort(after)));
}

value_t list_custom_sort(value_t list, value_t context, api_fn, sorter) {
  if (isNil(list)) return Nil;
  if (list.type != PairType) return Undefined;
  value_t before = Nil, after = Nil;
  pair_t pair = getPair(list);
  value_t node = pair.right;
  while (node.type == PairType) {
    pair_t next = getPair(node);
    if (pair.left.raw < next.left.raw) {
      before = cons(next.left, before);
    }
    else {
      after = cons(next.left, after);
    }
    node = next.right;
  }
  return list_append(
    list_sort(before),
    cons(pair.left, list_sort(after)));

}
value_t list_concat(value_t parts) {

}
value_t list_append(value_t list, value_t parts) {

}
value_t list_get(value_t set, int index) {

}
value_t list_set(value_t set, int index, value_t value) {

}
bool list_has(value_t list, value_t val) {

}
value_t list_add(value_t list, value_t val) {

}
value_t list_remove(value_t list, value_t val) {

}

#endif

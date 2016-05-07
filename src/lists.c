#ifndef LISTS_C
#define LISTS_C

#include "types.h"

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
    pair_t pair = get_pair(list);
    copy = cons(pair.left, copy);
    list = pair.right;
  }
  return copy;
}

API value_t list_append(value_t list, value_t values) {
  if (isNil(list)) return values;
  value_t node = list;
  while (node.type == PairType) {
    value_t next = cdr(node);
    if (isNil(next)) {
      set_cdr(node, values);
      return list;
    }
    node = next;
  }
  return Undefined;
}

API value_t list_sort(value_t list) {
  if (isNil(list)) return Nil;
  if (list.type != PairType) return TypeError;
  value_t before = Nil, after = Nil;
  pair_t pair = get_pair(list);
  value_t node = pair.right;
  while (node.type == PairType) {
    pair_t next = get_pair(node);
    if (pair.left.raw > next.left.raw) {
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

API value_t list_custom_sort(value_t list, value_t ctx, api_fn sorter) {
  if (isNil(list)) return Nil;
  if (list.type != PairType) return Undefined;
  value_t before = Nil, after = Nil;
  pair_t pair = get_pair(list);
  value_t node = pair.right;
  while (node.type == PairType) {
    pair_t next = get_pair(node);
    if (pair.left.raw < next.left.raw) {
      before = cons(next.left, before);
    }
    else {
      after = cons(next.left, after);
    }
    node = next.right;
  }
  return list_append(
    list_custom_sort(before, ctx, sorter),
    cons(pair.left, list_custom_sort(after, ctx, sorter))
  );

}

API value_t list_get(value_t list, int index) {
  if (index < 0) return Undefined;
  while (index--) {
    list = cdr(list);
  }
  return car(list);
}

API value_t list_set(value_t list, int index, value_t value) {
  if (index < 0) return list;
  value_t node = list;
  while (index--) {
    node = cdr(node);
  }
  set_car(node, value);
  return list;
}

API bool list_has(value_t list, value_t val) {
  while (list.type == PairType) {
    pair_t pair = get_pair(list);
    if (eq(pair.left, val)) return true;
    list = pair.right;
  }
  return false;
}

API value_t list_add(value_t list, value_t val) {
  if (isNil(list)) return cons(val, Nil);
  value_t node = list;
  while (node.type == PairType) {
    pair_t pair = get_pair(node);
    if (eq(pair.left, val)) return list;
    if (isNil(pair.right)) {
      set_cdr(node, cons(val, Nil));
      return list;
    }
    node = pair.right;
  }
  return TypeError;
}

API value_t list_remove(value_t list, value_t val) {
  if (list.type != PairType) return Nil;
  pair_t pair = get_pair(list);
  if (eq(pair.left, val)) return pair.right;
  value_t prev = list;
  value_t node = pair.right;
  while (node.type == PairType) {
    pair = get_pair(node);
    if (eq(pair.left, val)) {
      set_cdr(prev, pair.right);
      break;
    }
    node = pair.right;
  }
  return list;
}

API value_t list_each_r(value_t list, value_t context, api_fn block) {
  value_t result = Undefined;
  while (list.type == PairType) {
    pair_t pair = get_pair(list);
    result = block(context, pair.left);
    list = pair.right;
  }
  return result;
}
API value_t list_each(value_t list, value_t context, api_fn block) {
  return list_reverse(list_each_r(list, context, block));
}
API value_t list_map_r(value_t list, value_t context, api_fn block) {
  value_t result = Nil;
  while (list.type == PairType) {
    pair_t pair = get_pair(list);
    result = cons(block(context, pair.left), result);
    list = pair.right;
  }
  return result;
}
API value_t list_map(value_t list, value_t context, api_fn block) {
  return list_reverse(list_map_r(list, context, block));
}
API value_t list_filter_r(value_t list, value_t context, api_fn block) {
  value_t result = Nil;
  while (list.type == PairType) {
    pair_t pair = get_pair(list);
    if (isTruthy(block(context, pair.left))) {
      result = cons(pair.left, result);
    }
    list = pair.right;
  }
  return result;
}
API value_t list_filter(value_t list, value_t context, api_fn block) {
  return list_reverse(list_filter_r(list, context, block));
}


#endif

#ifndef DATA_C
#define DATA_C

#include "types.h"
#include <stdlib.h> // for realloc

// A list is made up of linked pairs. (value, next)
// A map is made up of linked pairs ((key, value), next)
// A string is a list of integers.


static pair_t *pairs;
static int next_pair;
static int num_pairs;

API pair_t getPair(value_t slot) {
  return (slot.type == PairType) ? pairs[slot.data] : Free;
}

// API bool setPair(value_t slot, pair_t pair) {
//   if (slot.type != PairType) return false;
//   pairs[slot.data] = pair;
//   return true;
// }

API value_t Integer(int32_t val) {
  return (value_t){
    .type = IntegerType,
    .data = val
  };
}

API value_t Symbol(const char* sym) {
  return (value_t){
    .type = SymbolType,
    .data = symbols_set(sym, 0)
  };
}

API value_t SymbolRange(const char* sym, const char* end) {
  return (value_t){
    .type = SymbolType,
    .data = symbols_set(sym, end - sym)
  };
}

API bool eq(value_t a, value_t b) {
  return a.raw == b.raw;
}

API bool isNil(value_t value) {
  return value.raw == Nil.raw;
}

API bool isFree(pair_t pair) {
  return pair.raw == Free.raw;
}

static int find_pair_slot() {
  while (next_pair < num_pairs && !isFree(pairs[next_pair])) {
    // TODO: we should probably also loop around before giving up.
    next_pair++;
  }

  // TODO: we should probably GC at this point.

  // Resize pair backing buffer if need-be
  int needed = next_pair + 1;
  if (needed > num_pairs) {
    // Allocate memory in blocks to reduce fragmentation
    // and batch allocations.
    int new_len = needed + (PAIRS_BLOCK_SIZE - needed % PAIRS_BLOCK_SIZE);
    pairs = realloc(pairs, (size_t)new_len * sizeof(pair_t));
    for (int j = num_pairs; j < new_len; j++) {
      pairs[j] = Free;
    }
    num_pairs = new_len;
  }

  return next_pair;
}


API value_t car(value_t var) {
  return var.type == PairType ? pairs[var.data].left : Undefined;
}

API value_t cdr(value_t var) {
  return var.type == PairType ? pairs[var.data].right : Undefined;
}

API value_t cons(value_t left, value_t right) {
  // For now the GC bits are set to zero.
  // When we write the GC later this may change.
  int slot = find_pair_slot();
  pairs[slot] = (pair_t){
    .left = left,
    .right = right
  };
  return (value_t){
    .type = PairType,
    .data = slot
  };
}


// // Append right to left (mutating left)
// // returns left for convenience
// API value_t append(value_t left, value_t right) {
//   if (isNil(left)) return right;
//   if (isNil(right)) return left;
//   if (left.type != PairType) return Undefined;
//   value_t node = left;
//   pair_t pair = pairs[left.data];
//   while (pair.right.type == PairType) {
//     node = pair.right;
//     pair = pairs[node.data];
//   }
//   if (!isNil(pair.right)) return Undefined;
//   pair.right = right;
//   pairs[node.data].raw = pair.raw;
//   return left;
// }

// create a new list that is reverse of given list
API value_t reverse(value_t src) {
  if (isNil(src)) return Nil;
  if (src.type != PairType) return Undefined;
  pair_t pair = pairs[src.data];
  value_t dst = Nil;
  while (pair.right.type == PairType) {
    dst = cons(pair.left, dst);
    pair = pairs[pair.right.data];
  }
  if (!isNil(pair.right)) return Undefined;
  return cons(pair.left, dst);
}

API value_t mget(value_t map, value_t key) {
  value_t node = map;
  while (node.type == PairType) {
    pair_t pair = pairs[node.data];
    if (pair.left.type != PairType) return Undefined;
    pair_t mapping = pairs[pair.left.data];
    if (eq(mapping.left, key)) return mapping.right;
    node = pair.right;
  }
  return isNil(node) ? Nil : Undefined;
}

API value_t mhas(value_t map, value_t key) {
  value_t node = map;
  while (node.type == PairType) {
    pair_t pair = pairs[node.data];
    if (pair.left.type != PairType) return Undefined;
    pair_t mapping = pairs[pair.left.data];
    if (eq(mapping.left, key)) return True;
    node = pair.right;
  }
  return isNil(node) ? False : Undefined;
}

API value_t mset(value_t map, value_t key, value_t value) {
  // If this is an empty map, create the first mapping and return it.
  if (isNil(map)) {
    return cons(cons(key, value), Nil);
  }
  // Look for an existing entry matching key and update in place
  value_t node = map;
  while (node.type == PairType) {
    pair_t pair = pairs[node.data];
    if (pair.left.type != PairType) return Undefined;
    pair_t mapping = pairs[pair.left.data];
    if (eq(mapping.left, key)) {
      mapping.right = value;
      pairs[pair.left.data].raw = mapping.raw;
      return map;
    }
    // Append a new mapping to the list if not found.
    if (isNil(pair.right)) {
      pair.right = cons(cons(key, value), Nil);
      pairs[node.data].raw = pair.raw;
      return map;
    }
    node = pair.right;
  }
  return Undefined;
}

#endif

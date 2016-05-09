#ifndef DATA_C
#define DATA_C

#include "types.h"
#include <stdlib.h> // for realloc

static pair_t *pairs;
static int next_pair;
static int num_pairs;


API value_t copy(value_t value) {
  if (value.type != PairType) return value;
  pair_t pair = get_pair(value);
  return cons(copy(pair.left), copy(pair.right));
}

API pair_t free_cell(value_t node) {
  if (node.type != PairType) return Free;
  int index = node.data;
  pair_t pair = pairs[index];
  pairs[index] = Free;
  if (index < next_pair) next_pair = index;
  return pair;
}

API value_t free_list(value_t node) {
  while (node.type == PairType) {
    int index = node.data;
    node = pairs[index].right;
    pairs[index] = Free;
    if (index < next_pair) next_pair = index;
  }
  return node;
}

static void mark(value_t node) {
  if (node.type != PairType || pairs[node.data].raw == Free.raw || pairs[node.data].left.gc) return;
  pairs[node.data].left.gc = 1;
  mark(pairs[node.data].left);
  mark(pairs[node.data].right);
}

API int collectgarbage(value_t root) {
  mark(root);
  int num_freed = 0;
  for (int i = num_pairs - 1; i >= 0; i--) {
    if (pairs[i].left.gc) {
      pairs[i].left.gc = 0;
      continue;
    }
    if (pairs[i].raw == Free.raw) {
      continue;
    }
    print("collected: ");
    dump_pair(pairs[i]);
    pairs[i].raw = Free.raw;
    next_pair = i;
    num_freed++;
  }
  return num_freed;
}

API value_t Bool(bool val) {
  return val ? True : False;
}

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

API value_t car(value_t var) {
  return var.type == PairType ? pairs[var.data].left : Undefined;
}

API value_t cdr(value_t var) {
  return var.type == PairType ? pairs[var.data].right : Undefined;
}

API bool set_car(value_t var, value_t val) {
  if (var.type != PairType) return false;
  pairs[var.data].left = val;
  return true;
}

API bool set_cdr(value_t var, value_t val) {
  if (var.type != PairType) return false;
  pairs[var.data].right = val;
  return true;
}

API pair_t get_pair(value_t slot) {
  return (slot.type == PairType) ? pairs[slot.data] : (pair_t){
    .right = TypeError,
    .left = TypeError,
  };
}

API value_t next(value_t *args) {
  if (isNil(*args)) return Undefined;
  pair_t pair = get_pair(*args);
  *args = pair.right;
  return pair.left;
}

API bool eq(value_t a, value_t b) {
  return a.raw == b.raw;
}

API bool isNil(value_t value) {
  return value.raw == Nil.raw;
}

API bool isTruthy(value_t value) {
  return value.type != AtomType || value.data > 0;
}

API bool isFree(pair_t pair) {
  return pair.raw == Free.raw;
}



#endif

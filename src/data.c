#ifndef DATA_C
#define DATA_C

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "symbols.c"

#ifndef PAIRS_BLOCK_SIZE
#define PAIRS_BLOCK_SIZE 16
#endif


typedef enum {
  AtomType,
  IntegerType,
  SymbolType,
  PairType,
} type_t;

typedef union {
  struct {
    int gc : 1;
    type_t type : 2;
    int data : 29;
  };
  uint32_t raw;
} value_t;

typedef union {
  struct {
    value_t left;
    value_t right;
  };
  uint64_t raw;
} pair_t;

static pair_t *pairs;
static int next_pair;
static int num_pairs;

static const value_t Undefined = (value_t){
  .type = AtomType,
  .data = -2
};

static const pair_t Free = (pair_t){
  .raw = ~0ul
};

API const value_t Nil = (value_t){
  .type = AtomType,
  .data = -1
};

API const value_t False = (value_t){
  .type = AtomType,
  .data = 0
};

API const value_t True = (value_t){
  .type = AtomType,
  .data = 1
};


API inline value_t Bool(bool val) {
  return (value_t){
    .type = AtomType,
    .data = val ? 1 : 0
  };
}
API inline value_t Integer(int32_t val) {
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

API inline bool eq(value_t a, value_t b) {
  return a.raw == b.raw;
}

API inline bool isNil(value_t value) {
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
  if (var.type != PairType) return Undefined;
  return pairs[var.data].left;
}

API value_t cdr(value_t var) {
  if (var.type != PairType) return Undefined;
  return pairs[var.data].right;
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

#define List(...) ({\
  value_t values[] = { __VA_ARGS__ }; \
  value_t node = Nil; \
  for (int i = sizeof(values)/sizeof(value_t) - 1; i >= 0; i--) { \
    node = cons(values[i], node); \
  } \
  node; })

#define Mapping(name, value) cons(Symbol(#name),value)

// Append right to left (mutating left)
// returns left for convenience
API value_t append(value_t left, value_t right) {
  if (isNil(left)) return right;
  if (isNil(right)) return left;
  if (left.type != PairType) return Undefined;
  value_t node = left;
  pair_t pair = pairs[left.data];
  while (pair.right.type == PairType) {
    node = pair.right;
    pair = pairs[node.data];
  }
  if (!isNil(pair.right)) return Undefined;
  pair.right = right;
  pairs[node.data].raw = pair.raw;
  return left;
}

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

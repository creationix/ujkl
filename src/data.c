#ifndef DATA_C
#define DATA_C

#include "types.h"
#include <stdlib.h> // for realloc

// A list is made up of linked pairs. (value, next)
// A map is made up of linked pairs ((key, value), next)
// A string is a list of integers.


// (set key value
//      (list key) value...)

// (get key)
// (get (list key))
// (get foo.bar)


static pair_t *pairs;
static int next_pair;
static int num_pairs;

API pair_t getPair(value_t slot) {
  return (slot.type == PairType) ? pairs[slot.data] : Free;
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

API value_t ilen(value_t list) {
  int len = 0;
  while (list.type == PairType) {
    len++;
    list = cdr(list);
  }
  return Integer(len);
}

API value_t iget(value_t list, value_t key) {
  if (key.type != IntegerType) return Undefined;
  int index = key.data;
  if (index < 0) return Undefined;
  value_t node = list;
  while (index--) { node = cdr(node); }
  return car(node);
}

API value_t iset(value_t list, value_t key, value_t value) {
  if (key.type != IntegerType) return Undefined;
  int index = key.data;
  if (index < 0) return Undefined;
  if (list.type != PairType) return Undefined;
  pair_t pair = pairs[list.data];
  value_t node = list;
  while (index--) {
    node = pair.right;
    if (node.type != PairType) return Undefined;
    pair = pairs[node.data];
  }
  pair.left = value;
  pairs[node.data] = pair;
  return key;
}

// API value_t sadd(value_t set, value_t key) {
//   if (isNil(set)) return cons(key, Nil);
//   value_t node = set;
//   while (node.type == PairType) {
//     pair_t pair = pairs[node.data];
//     if (eq(pair.left, key)) return set;
//     if (isNil(pair.right)) {
//       pair.right = cons(key, Nil);
//       pairs[node.data] = pair;
//       return set;
//     }
//     node = pair.right;
//   }
//   return Undefined;
// }
//
// API value_t shas(value_t set, value_t key) {
//   if (isNil(set)) return False;
//   value_t node = set;
//   while (node.type == PairType) {
//     pair_t pair = pairs[node.data];
//     if (eq(pair.left, key)) return True;
//     if (isNil(pair.right)) return False;
//     node = pair.right;
//   }
//   return Undefined;
// }
//
// API value_t sdel(value_t set, value_t key) {
//   if (isNil(set)) return Nil;
//   if (set.type != PairType) return Undefined;
//   pair_t pair = pairs[set.data];
//   if (eq(pair.left, key)) return pair.right;
//   value_t node = pair.right;
//   if (isNil(node)) return set;
//   while (node.type == PairType) {
//     pair_t pair = pairs[node.data];
//     if (eq(pair.left, key)) return True;
//     if (isNil(pair.right)) return False;
//     node = pair.right;
//   }
//   return Undefined;
// }

API value_t get(value_t map, value_t key) {
  while (map.type == PairType) {
    pair_t pair = pairs[map.data];
    if (pair.left.type != PairType) return TypeError;
    pair_t mapping = pairs[pair.left.data];
    if (eq(mapping.left, key)) return mapping.right;
    map = pair.right;
  }
  return Nil;
}

API value_t aget(value_t map, value_t keys) {
  if (isNil(keys)) return map;
  if (keys.type != PairType) return TypeError;
  pair_t keypair = pairs[keys.data];
  while (map.type == PairType) {
    pair_t pair = pairs[map.data];
    if (pair.left.type != PairType) return TypeError;
    pair_t mapping = pairs[pair.left.data];
    if (eq(mapping.left, keypair.left)) {
      return aget(mapping.right, keypair.right);
    }
    map = pair.right;
  }
  return Nil;
}

API value_t has(value_t map, value_t key) {
  while (map.type == PairType) {
    pair_t pair = pairs[map.data];
    if (pair.left.type != PairType) return TypeError;
    pair_t mapping = pairs[pair.left.data];
    if (eq(mapping.left, key)) return True;
    map = pair.right;
  }
  return False;
}

API value_t ahas(value_t map, value_t keys) {
  if (isNil(keys)) return True;
  if (keys.type != PairType) return TypeError;
  pair_t keypair = pairs[keys.data];
  while (map.type == PairType) {
    pair_t pair = pairs[map.data];
    if (pair.left.type != PairType) return TypeError;
    pair_t mapping = pairs[pair.left.data];
    if (eq(mapping.left, keypair.left)) {
      return ahas(mapping.right, keypair.right);
    }
    map = pair.right;
  }
  return False;
}

API value_t del(value_t map, value_t key) {
  value_t prev;
  pair_t prev_pair;
  value_t node = map;
  while (node.type == PairType) {
    pair_t pair = pairs[node.data];
    if (pair.left.type != PairType) return TypeError;
    pair_t mapping = pairs[pair.left.data];
    if (eq(mapping.left, key)) {
      if (eq(node, map)) return pair.right;
      prev_pair.right = pair.right;
      pairs[prev.data] = prev_pair;
      return map;
    }
    prev = node;
    prev_pair = pair;
    node = pair.right;
  }
  if (isNil(node)) return map;
  return TypeError;
}

API value_t adel(value_t map, value_t keys) {
  if (isNil(keys)) return map;
  if (keys.type != PairType) return TypeError;
  pair_t keypair = pairs[keys.data];
  value_t prev;
  pair_t prev_pair;
  value_t node = map;
  while (node.type == PairType) {
    pair_t pair = pairs[node.data];
    if (pair.left.type != PairType) return TypeError;
    pair_t mapping = pairs[pair.left.data];
    if (eq(mapping.left, keypair.left)) {
      if (isNil(keypair.right)) {
        if (eq(node, map)) return pair.right;
        prev_pair.right = pair.right;
        pairs[prev.data] = prev_pair;
        return map;
      }
      mapping.right = adel(mapping.right, keypair.right);
      if (eq(mapping.right, TypeError)) return mapping.right;
      pairs[pair.left.data] = mapping;
      return map;
    }
    prev = node;
    prev_pair = pair;
    node = pair.right;
  }
  if (isNil(node)) return map;
  return TypeError;
}

API value_t each(value_t context, value_t node, api_fn block) {
  value_t result = Undefined;
  while (node.type == PairType) {
    pair_t pair = pairs[node.data];
    result = block(context, pair.left);
    node = pair.right;
  }
  return result;
}

API value_t map(value_t context, value_t node, api_fn block) {
  value_t result = Nil;
  while (node.type == PairType) {
    pair_t pair = pairs[node.data];
    result = cons(block(context, pair.left), result);
    node = pair.right;
  }
  return reverse(result);
}

API value_t set(value_t map, value_t key, value_t value) {
  if (isNil(map)) return cons(cons(key, value), Nil);
  value_t node = map;
  while (node.type == PairType) {
    pair_t pair = pairs[node.data];
    if (pair.left.type != PairType) return TypeError;
    pair_t mapping = pairs[pair.left.data];
    if (eq(mapping.left, key)) {
      mapping.right = value;
      pairs[pair.left.data] = mapping;
      return map;
    }
    if (isNil(pair.right)) {
      pair.right = cons(cons(key, value), Nil);
      pairs[node.data] = pair;
      return map;
    }
    node = pair.right;
  }
  return TypeError;
}

// Map is a list of key/value pairs (possibly with nested maps in values).
// Keys is a list of keys, each for the nesting
// Value is the new value to set or update.
// returns map (possibly changed)
API value_t aset(value_t map, value_t keys, value_t value) {
  if (isNil(keys)) return value;
  if (keys.type != PairType) return TypeError;
  pair_t keypair = pairs[keys.data];
  if (isNil(map)) {
    value = aset(Nil, keypair.right, value);
    return cons(cons(keypair.left, value), Nil);
  }
  value_t node = map;
  while (node.type == PairType) {
    pair_t pair = pairs[node.data];
    if (pair.left.type != PairType) return TypeError;
    pair_t mapping = pairs[pair.left.data];
    if (eq(mapping.left, keypair.left)) {
      mapping.right = aset(mapping.right, keypair.right, value);
      pairs[pair.left.data] = mapping;
      return map;
    }
    if (isNil(pair.right)) {
      value = aset(pair.right, keypair.right, value);
      pair.right = cons(cons(keypair.left, value), Nil);
      pairs[node.data] = pair;
      return map;
    }
    node = pair.right;
  }
  return TypeError;
}

#endif

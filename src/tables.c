#ifndef TABLES_C
#define TABLES_C

#include "types.h"

API bool is_table(value_t val) {
  while (val.type == PairType) {
    pair_t pair = get_pair(val);
    if (pair.left.type != PairType) return false;
    val = pair.right;
  }
  return isNil(val);
}

API value_t table_get(value_t table, value_t key) {
  while (table.type == PairType) {
    pair_t pair = get_pair(table);
    pair_t mapping = get_pair(pair.left);
    if (eq(mapping.left, key)) return mapping.right;
    table = pair.right;
  }
  return Undefined;
}

// API value_t table_aget(value_t table, value_t keys) {
//   if (isNil(keys)) return table;
//   pair_t keypair = get_pair(keys);
//   while (table.type == PairType) {
//     pair_t pair = get_pair(table);
//     if (pair.left.type != PairType) return TypeError;
//     pair_t mapping = get_pair(pair.left);
//     if (eq(mapping.left, keypair.left)) {
//       return table_aget(mapping.right, keypair.right);
//     }
//     table = pair.right;
//   }
//   return Nil;
// }


API value_t table_set(value_t table, value_t key, value_t value) {
  if (isNil(table)) return cons(cons(key, value), Nil);
  value_t node = table;
  while (node.type == PairType) {
    pair_t pair = get_pair(node);
    pair_t mapping = get_pair(pair.left);
    if (eq(mapping.left, key)) {
      set_cdr(pair.left, value);
      return table;
    }
    if (isNil(pair.right)) {
      set_cdr(node, cons(cons(key, value), Nil));
      return table;
    }
    node = pair.right;
  }
  return TypeError;
}

// Map is a list of key/value pairs (possibly with nested maps in values).
// Keys is a list of keys, each for the nesting
// Value is the new value to set or update.
// returns map (possibly changed)
// API value_t table_aset(value_t map, value_t keys, value_t value) {
//   if (isNil(keys)) return value;
//   if (keys.type != PairType) return TypeError;
//   pair_t keypair = get_pair(keys);
//   if (isNil(map)) {
//     value = aset(Nil, keypair.right, value);
//     return cons(cons(keypair.left, value), Nil);
//   }
//   value_t node = map;
//   while (node.type == PairType) {
//     pair_t pair = get_pair(node);
//     if (pair.left.type != PairType) return TypeError;
//     pair_t mapping = get_pair(pair.left);
//     if (eq(mapping.left, keypair.left)) {
//       set_cdr(pair.left, aset(mapping.right, keypair.right, value));
//       return map;
//     }
//     if (isNil(pair.right)) {
//       value = aset(pair.right, keypair.right, value);
//       set_cdr(node, cons(cons(keypair.left, value), Nil));
//       return map;
//     }
//     node = pair.right;
//   }
//   return TypeError;
// }

// API value_t table_has(value_t map, value_t key) {
//   while (map.type == PairType) {
//     pair_t pair = get_pair(map);
//     if (pair.left.type != PairType) return TypeError;
//     pair_t mapping = get_pair(pair.left);
//     if (eq(mapping.left, key)) return True;
//     map = pair.right;
//   }
//   return False;
// }
//
// API value_t table_has(value_t map, value_t keys) {
//   if (isNil(keys)) return True;
//   if (keys.type != PairType) return TypeError;
//   pair_t keypair = get_pair(keys);
//   while (map.type == PairType) {
//     pair_t pair = get_pair(map);
//     if (pair.left.type != PairType) return TypeError;
//     pair_t mapping = get_pair(pair.left);
//     if (eq(mapping.left, keypair.left)) {
//       return ahas(mapping.right, keypair.right);
//     }
//     map = pair.right;
//   }
//   return False;
// }
//
// API value_t table_del(value_t map, value_t key) {
//   value_t prev;
//   pair_t prev_pair;
//   value_t node = map;
//   while (node.type == PairType) {
//     pair_t pair = get_pair(node);
//     if (pair.left.type != PairType) return TypeError;
//     pair_t mapping = get_pair(pair.left);
//     if (eq(mapping.left, key)) {
//       if (eq(node, map)) return pair.right;
//       set_cdr(prev, pair.right);
//       return map;
//     }
//     prev = node;
//     prev_pair = pair;
//     node = pair.right;
//   }
//   if (isNil(node)) return map;
//   return TypeError;
// }
//
// API value_t table_adel(value_t map, value_t keys) {
//   if (isNil(keys)) return map;
//   if (keys.type != PairType) return TypeError;
//   pair_t keypair = get_pair(keys);
//   value_t prev;
//   pair_t prev_pair;
//   value_t node = map;
//   while (node.type == PairType) {
//     pair_t pair = get_pair(node);
//     if (pair.left.type != PairType) return TypeError;
//     pair_t mapping = get_pair(pair.left);
//     if (eq(mapping.left, keypair.left)) {
//       if (isNil(keypair.right)) {
//         if (eq(node, map)) return pair.right;
//         set_cdr(prev, pair.right);
//         return map;
//       }
//       mapping.right = adel(mapping.right, keypair.right);
//       if (eq(mapping.right, TypeError)) return mapping.right;
//       set_cdr(pair.left, pair.right);
//       return map;
//     }
//     prev = node;
//     prev_pair = pair;
//     node = pair.right;
//   }
//   if (isNil(node)) return map;
//   return TypeError;
// }

#endif

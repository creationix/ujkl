#ifndef TABLES_C
#define TABLES_C

#include "types.h"

// A table is a list of key/value pairs (possibly with nested tables in values).

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

API value_t table_aget(value_t table, value_t keys) {
  if (isNil(keys)) return table;
  pair_t keypair = get_pair(keys);
  while (table.type == PairType) {
    pair_t pair = get_pair(table);
    pair_t mapping = get_pair(pair.left);
    if (eq(mapping.left, keypair.left)) {
      return table_aget(mapping.right, keypair.right);
    }
    table = pair.right;
  }
  return Undefined;
}

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
  return table;
}

API value_t table_aset(value_t map, value_t keys, value_t value) {
  if (isNil(keys)) return value;
  pair_t keypair = get_pair(keys);
  if (isNil(map)) {
    value = table_aset(Nil, keypair.right, value);
    return cons(cons(keypair.left, value), Nil);
  }
  value_t node = map;
  while (node.type == PairType) {
    pair_t pair = get_pair(node);
    pair_t mapping = get_pair(pair.left);
    if (eq(mapping.left, keypair.left)) {
      set_cdr(pair.left, table_aset(mapping.right, keypair.right, value));
      return map;
    }
    if (isNil(pair.right)) {
      value = table_aset(pair.right, keypair.right, value);
      set_cdr(node, cons(cons(keypair.left, value), Nil));
      return map;
    }
    node = pair.right;
  }
  return map;
}

API bool table_has(value_t map, value_t key) {
  while (map.type == PairType) {
    pair_t pair = get_pair(map);
    pair_t mapping = get_pair(pair.left);
    if (eq(mapping.left, key)) return true;
    map = pair.right;
  }
  return false;
}

API bool table_ahas(value_t map, value_t keys) {
  if (isNil(keys)) return true;
  pair_t keypair = get_pair(keys);
  while (map.type == PairType) {
    pair_t pair = get_pair(map);
    pair_t mapping = get_pair(pair.left);
    if (eq(mapping.left, keypair.left)) {
      return table_ahas(mapping.right, keypair.right);
    }
    map = pair.right;
  }
  return false;
}

API value_t table_del(value_t map, value_t key) {
  value_t prev;
  value_t node = map;
  while (node.type == PairType) {
    pair_t pair = get_pair(node);
    pair_t mapping = get_pair(pair.left);
    if (eq(mapping.left, key)) {
      if (eq(node, map)) return pair.right;
      set_cdr(prev, pair.right);
      return map;
    }
    prev = node;
    node = pair.right;
  }
  return map;
}

API value_t table_adel(value_t map, value_t keys) {
  if (isNil(keys)) return map;
  pair_t keypair = get_pair(keys);
  value_t prev;
  value_t node = map;
  while (node.type == PairType) {
    pair_t pair = get_pair(node);
    pair_t mapping = get_pair(pair.left);
    if (eq(mapping.left, keypair.left)) {
      if (isNil(keypair.right)) {
        if (eq(node, map)) return pair.right;
        set_cdr(prev, pair.right);
        return map;
      }
      mapping.right = table_adel(mapping.right, keypair.right);
      set_cdr(pair.left, pair.right);
      return map;
    }
    prev = node;
    node = pair.right;
  }
  return map;
}

#endif

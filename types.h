#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef enum {
  Integer,
  Symbol,
  Pair,
  Atom,
} type_t;

typedef enum {
  Nil,
  True,
  False
} atom_t;

typedef union {
  struct {
    type_t type : 2;
    int gc : 1;
    int value : 29;
  };
  uint32_t num;
} value_t;

typedef union {
  struct {
    value_t left;
    value_t right;
  };
  uint64_t num;
} pair_t;

typedef void (*function_t)();

typedef struct {
  const char* name;
  function_t* fn;
} builtin_t;

#endif

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef API
#define API
#endif

#ifndef PAIRS_BLOCK_SIZE
#define PAIRS_BLOCK_SIZE 16
#endif

#ifndef SYMBOLS_BLOCK_SIZE
#define SYMBOLS_BLOCK_SIZE 128
#endif

#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 78
#endif

#ifndef WRITE_BUFFER_LENGTH
#define WRITE_BUFFER_LENGTH 64
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

typedef value_t (*api_fn)(value_t args);
typedef void (*read_fn)(const char *data);

typedef struct {
  const char* name;
  api_fn fn;
} builtin_t;

API value_t quoteSym, listSym;

// Print library so we don't need a full-blown printf.
API bool print(const char* value);
API bool print_int(int num);
API bool print_char(const char c);
API bool print_string(const char* value, size_t len);
API void print_flush();

// Symbol library for resolving between integers and cstrings.
API int first_fn;
API void symbols_init(const builtin_t *fns, int numKeywords);
API int symbols_set(const char *word, size_t len);
API const char *symbols_get_name(int index);
API api_fn symbols_get_fn(int index);

// Prints a value to stdout with newline
API void dump(value_t val);
// Prints multiple values with spaces between them
API void dump_line(value_t val);

// Data
#define EmptySlot ((value_t){.type = AtomType, .data = -10})
#define RangeError ((value_t){.type = AtomType, .data = -5})
#define TypeError ((value_t){.type = AtomType, .data = -4})
#define Dot ((value_t){.type = AtomType, .data = -3})
#define Undefined ((value_t){.type = AtomType, .data = -2})
#define Free ((pair_t){.left = EmptySlot, .right = EmptySlot})
#define Nil ((value_t){.type = AtomType,.data = -1})
#define False ((value_t){.type = AtomType,.data = 0})
#define True ((value_t){.type = AtomType,.data = 1})
#define List(...) __extension__({\
  value_t values[] = { __VA_ARGS__ }; \
  value_t node = Nil; \
  for (int i = sizeof(values)/sizeof(value_t) - 1; i >= 0; i--) { \
    node = cons(values[i], node); \
  } \
  node; })
#define Mapping(name, value) cons(Symbol(#name),value)
API int collectgarbage(value_t root);
API pair_t get_pair(value_t slot);
API value_t next(value_t *args);
API value_t Bool(bool val);
API value_t Integer(int32_t val);
API value_t Symbol(const char* sym);
API value_t SymbolRange(const char* start, const char* end);

API value_t cons(value_t left, value_t right);
API value_t car(value_t var);
API value_t cdr(value_t var);
API bool set_car(value_t var, value_t val);
API bool set_cdr(value_t var, value_t val);
API pair_t get_pair(value_t slot);
API bool eq(value_t a, value_t b);
API bool isNil(value_t value);
API bool isTruthy(value_t value);
API bool isFree(pair_t pair);

API value_t eval(value_t env, value_t val);
API value_t block(value_t env, value_t body);
API value_t apply(value_t fn, value_t args);

// Lists
API bool is_list(value_t val);
API int list_length(value_t list);
API value_t list_reverse(value_t list);
API value_t list_append(value_t list, value_t values);
API value_t list_sort(value_t list);
// API value_t list_custom_sort(value_t list, value_t ctx, api_fn sorter);
API value_t list_get(value_t list, int index);
API value_t list_set(value_t list, int index, value_t value);
API bool list_has(value_t list, value_t val);
API value_t list_add(value_t list, value_t val);
API value_t list_remove(value_t list, value_t val);
// API value_t list_each_r(value_t list, value_t context, api_fn block);
// API value_t list_each(value_t list, value_t context, api_fn block);
// API value_t list_map_r(value_t list, value_t context, api_fn block);
// API value_t list_map(value_t list, value_t context, api_fn block);

// Tables

API bool is_table(value_t val);
API bool table_has(value_t map, value_t key);
API bool table_ahas(value_t map, value_t keys);
API value_t table_get(value_t table, value_t key);
API value_t table_aget(value_t table, value_t keys);
API value_t table_set(value_t table, value_t key, value_t value);
API value_t table_aset(value_t map, value_t keys, value_t value);
API value_t table_del(value_t map, value_t key);
API value_t table_adel(value_t map, value_t keys);

// Iterators
typedef void (*callback_t)(value_t ctx, value_t item);
API void iter_any(value_t iter, value_t ctx, callback_t fn);


#endif

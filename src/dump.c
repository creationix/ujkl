#ifndef DUMP_C
#define DUMP_C

#include <stdio.h>
#include "data.c"

// Jack's Shades-O-Green Theme (256 colors)
// #define COFF "\x1b[0m"
// #define CNIL "\x1b[38;5;30m"
// #define CBOOL "\x1b[38;5;82m"
// #define CINT "\x1b[38;5;34m"
// #define CSYM "\x1b[38;5;226m"
// #define CUNDEF "\x1b[38;5;244m"
// #define CBUILTIN "\x1b[38;5;208m"
// #define CPAREN "\x1b[38;5;31m"
// #define CSEP "\x1b[38;5;160m"

// Tim's Blue-Orange Theme (256 colors)
#define COFF "\x1b[0m"
#define CNIL "\x1b[38;5;63m"
#define CBOOL "\x1b[38;5;202m"
#define CINT "\x1b[38;5;39m"
#define CSYM "\x1b[38;5;252m"
#define CUNDEF "\x1b[38;5;244m"
#define CBUILTIN "\x1b[38;5;214m"
#define CPAREN "\x1b[38;5;24m"
#define CSEP "\x1b[38;5;26m"

//#define COFF "\x1b[0m"
//#define CNIL "\x1b[1;36m"
//#define CBOOL "\x1b[1;33m"
//#define CINT "\x1b[1;35m"
//#define CSYM "\x1b[1;39m"
//#define CUNDEF "\x1b[0;34m"
//#define CBUILTIN "\x1b[1;32m"
//#define CPAREN "\x1b[1;30m"
//#define CSEP "\x1b[1;34m"

static void _dump(value_t val) {
  switch (val.type) {
    case AtomType:
      switch (val.data) {
        case -1: printf(CNIL"nil"); return;
        case 1: printf(CBOOL"true"); return;
        case 0: printf(CBOOL"false"); return;
      }
      printf(CUNDEF"undefined"); return;
    case IntegerType:
      printf(CINT"%d", val.data); return;
    case SymbolType:
      if (val.data < 0) {
        printf(CSYM"%s", symbols_get(val.data));
      }
      else {
        printf(CBUILTIN"%s", symbols_get(val.data));
      }
      return;
    case PairType: {
      pair_t pair = pairs[val.data];
      printf(CPAREN"(");
      if (isNil(pair.right)) {
        _dump(pair.left);
      }
      else if (pair.right.type == PairType) {
        _dump(pair.left);
        while (pair.right.type == PairType) {
          printf(" ");
          pair = pairs[pair.right.data];
          _dump(pair.left);
        }
        if (!isNil(pair.right)) {
          printf(CSEP" . ");
          _dump(pair.right);
        }
      } else {
        _dump(pair.left);
        printf(CSEP" . ");
        _dump(pair.right);
      }
      printf(CPAREN")");
      return;
    }
  }
}

API void dump(value_t val) {
  _dump(val);
  printf(COFF"\n");
}

#endif

#ifndef DUMP_C
#define DUMP_C

#include <stdio.h>
#include "data.c"

#define COFF "\x1b[0m"
#define CNIL "\x1b[38;5;247m"
#define CBOOL "\x1b[38;5;39m"
#define CUNDEF "\x1b[38;5;244m"
#define CINT "\x1b[38;5;208m"
#define CBUILTIN "\x1b[38;5;251m"
#define CSYM "\x1b[38;5;46m"
#define CPAREN "\x1b[38;5;247m"
#define CSEP "\x1b[38;5;240m"

API void dump(value_t val) {
  switch (val.type) {
    case AtomType:
      switch (val.data) {
        case -1: printf(CNIL"()"); return;
        case 1: printf(CBOOL"true"); return;
        case 0: printf(CBOOL"false"); return;
      }
      printf(CUNDEF"undefined"); return;
    case IntegerType:
      printf(CINT"%d", val.data); return;
    case SymbolType:
      if (val.data < 0) {
        printf(CBUILTIN"%s", symbols_get(val.data));
      }
      else {
        printf(CSYM"%s", symbols_get(val.data));
      }
      return;
    case PairType: {
      pair_t pair = pairs[val.data];
      printf(CPAREN"(");
      if (isNil(pair.right)) {
        dump(pair.left);
      }
      else if (pair.right.type == PairType) {
        dump(pair.left);
        while (pair.right.type == PairType) {
          printf(" ");
          pair = pairs[pair.right.data];
          dump(pair.left);
        }
        if (!isNil(pair.right)) {
          dump(pair.right);
        }
      } else {
        dump(pair.left);
        printf(CSEP" . ");
        dump(pair.right);
      }
      printf(CPAREN")");
      return;
    }
  }
}

#endif

#ifndef DUMP_C
#define DUMP_C

#include "print.c"
#include "data.c"

#ifndef THEME
  #define COFF ""
  #define CNIL ""
  #define CBOOL ""
  #define CINT ""
  #define CSYM ""
  #define CUNDEF ""
  #define CBUILTIN ""
  #define CPAREN ""
  #define CSEP ""
#elif THEME == tim
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
#elif THEME == jack
  // Jack's Shades-O-Green Theme (256 colors)
  #define COFF "\x1b[0m"
  #define CNIL "\x1b[38;5;30m"
  #define CBOOL "\x1b[38;5;82m"
  #define CINT "\x1b[38;5;34m"
  #define CSYM "\x1b[38;5;226m"
  #define CUNDEF "\x1b[38;5;244m"
  #define CBUILTIN "\x1b[38;5;208m"
  #define CPAREN "\x1b[38;5;31m"
  #define CSEP "\x1b[38;5;160m"
#else
  #define COFF "\x1b[0m"
  #define CNIL "\x1b[1;36m"
  #define CBOOL "\x1b[1;33m"
  #define CINT "\x1b[1;35m"
  #define CSYM "\x1b[1;39m"
  #define CUNDEF "\x1b[0;34m"
  #define CBUILTIN "\x1b[1;32m"
  #define CPAREN "\x1b[1;30m"
  #define CSEP "\x1b[1;34m"
#endif
static void _dump(value_t val) {
  switch (val.type) {
    case AtomType:
      switch (val.data) {
        case -1: print(CNIL"nil"); return;
        case 1: print(CBOOL"true"); return;
        case 0: print(CBOOL"false"); return;
        default: print(CUNDEF"undefined"); return;
      }
    case IntegerType:
      print(CINT);
      print_int(val.data);
      return;
    case SymbolType:
      if (val.data < 0) print(CSYM);
      else print(CBUILTIN);
      print(symbols_get(val.data));
      return;
    case PairType: {
      pair_t pair = pairs[val.data];
      print(CPAREN"(");
      if (isNil(pair.right)) {
        _dump(pair.left);
      }
      else if (pair.right.type == PairType) {
        _dump(pair.left);
        while (pair.right.type == PairType) {
          print_char(' ');
          pair = pairs[pair.right.data];
          _dump(pair.left);
        }
        if (!isNil(pair.right)) {
          print(CSEP" . ");
          _dump(pair.right);
        }
      } else {
        _dump(pair.left);
        print(CSEP" . ");
        _dump(pair.right);
      }
      print(CPAREN")");
      return;
    }
  }
}

API void dump(value_t val) {
  _dump(val);
  print(COFF"\n");
}

#endif

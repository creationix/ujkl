#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "symbol.c"

// A list is made up of linked pairs. (value, next)
// A map is made up of linked pairs ((key, value), next)
// A string is a list of integers.

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
static const value_t Nil = (value_t){
  .type = AtomType,
  .data = -1
};
static const value_t False = (value_t){
  .type = AtomType,
  .data = 0
};
static const value_t True = (value_t){
  .type = AtomType,
  .data = 1
};
static const pair_t Free = (pair_t){
  .raw = ~0ul
};

static inline value_t Bool(bool val) {
  return (value_t){
    .type = AtomType,
    .data = val ? 1 : 0
  };
}
static inline value_t Integer(int32_t val) {
  return (value_t){
    .type = IntegerType,
    .data = val
  };
}

static inline bool isNil(value_t value) {
  return value.raw == Nil.raw;
}

static inline bool isFalsy(value_t value) {
  return value.raw == Nil.raw ||
         value.raw == False.raw;
}
// static inline bool isTruthy(value_t value) {
//   return !isFalsy(value);
// }

static inline bool isFree(pair_t pair) {
  return pair.raw == Free.raw;
}

static void setup_pairs(int num) {
  num_pairs = num;
  next_pair = 0;
  pairs = malloc((size_t)num_pairs * sizeof(pair_t));
  for (int i = 0; i < num_pairs; i++) {
    pairs[i] = Free;
  }
}

static int find_pair_slot() {
  while (next_pair < num_pairs && !isFree(pairs[next_pair])) {
    next_pair++;
  }
  if (next_pair == num_pairs) {
    // TODO: we should probably first collect garbage and/or
    //       wrap the search at the beginning.
    // If we run out of space, double the storage.
    int old_num = num_pairs;
    num_pairs *= 2;
    printf("Growing pairs storage to %u\n", num_pairs);
    pairs = realloc(pairs, (size_t)num_pairs * sizeof(pair_t));
    for (int i = old_num; i < num_pairs; i++) {
      pairs[i] = Free;
    }
  }
  return next_pair;
}


static value_t Symbol(const char* sym) {
  return (value_t){
    .type = SymbolType,
    .data = matchSymbol(sym, 0)
  };
}

static value_t not(value_t var) {
  return Bool(isFalsy(var));
}

static value_t car(value_t var) {
  if (var.type != PairType) return Undefined;
  return pairs[var.data].left;
}

static value_t cdr(value_t var) {
  if (var.type != PairType) return Undefined;
  return pairs[var.data].right;
}

static value_t cons(value_t left, value_t right) {
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

// Append right to left (mutating left)
// returns left for convenience
static value_t append(value_t left, value_t right) {
  if (left.type != PairType) return Undefined;
  pair_t pair = pairs[left.data];
  while (pair.right.type == PairType) {
    pair = pairs[pair.right.data];
  }
  if (!isNil(pair.right)) return Undefined;
  pair.right = right;
  return left;
}

// create a new list that is reverse of given list
static value_t reverse(value_t src) {
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

void dump(value_t val) {
  switch (val.type) {
    case AtomType:
      switch (val.data) {
        case -1: printf("\x1b[38;5;247m()\x1b[0m"); return;
        case 1: printf("\x1b[38;5;39mtrue\x1b[0m"); return;
        case 2: printf("\x1b[38;5;39mfalse\x1b[0m"); return;
      }
      printf("\x1b[38;5;244mundefined\x1b[0m"); return;
    case IntegerType:
      printf("\x1b[38;5;208m%d\x1b[0m", val.data); return;
    case SymbolType:
      if (val.data < 0) {
        printf("\x1b[38;5;251m%s\x1b[0m", getSymbol(val.data));
      }
      else {
        printf("\x1b[38;5;46m%s\x1b[0m", getSymbol(val.data));
      }
      return;
    case PairType: {
      pair_t pair = pairs[val.data];
      printf("\x1b[38;5;247m(\x1b[0m");
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
        printf(" \x1b[38;5;240m.\x1b[0m ");
        dump(pair.right);
      }
      printf("\x1b[38;5;247m)\x1b[0m");
      return;
    }
  }
}


int main() {
  setup_pairs(10);
  assert((Integer(1)).data == 1);
  assert((Integer(0)).data == 0);
  assert((Integer(-1)).data == -1);
  value_t numbers = cons(
    Integer(1),
    cons(
      Integer(2),
      cons(
        Integer(3),
        cons(
          Integer(4),
          Nil
        )
      )
    )
  );
  value_t tim = cons(
    cons(Symbol("name"),Symbol("Tim")),
    cons(
      cons(Symbol("age"),Integer(34)),
      cons(
        cons(Symbol("isProgrammer"),True),
        Nil
      )
    )
  );
  value_t jack = cons(
    cons(Symbol("name"),Symbol("Jack")),
    cons(
      cons(Symbol("age"),Integer(10)),
      cons(
        cons(Symbol("isProgrammer"),not(False)),
        Nil
      )
    )
  );
  dump(numbers);
  printf("\n");
  dump(cdr(numbers));
  printf("\n");
  dump(cdr(cdr(numbers)));
  printf("\n");
  dump(cdr(cdr(cdr(numbers))));
  printf("\n");
  dump(cdr(cdr(cdr(cdr(numbers)))));
  printf("\n");
  dump(cdr(cdr(cdr(cdr(cdr(numbers))))));
  printf("\n");
  dump(tim);
  printf("\n");
  dump(car(tim));
  printf("\n");
  dump(cdr(tim));
  printf("\n");
  dump(cons(tim, jack));
  printf("\nAppend tim and jack: ");
  dump(append(tim, jack));
  printf("\nJust tim: ");
  dump(tim);
  printf("\nReverse tim: ");
  dump(reverse(tim));
  printf("\nReverse numbers: ");
  dump(reverse(numbers));
  printf("\n");
  dump(reverse(cdr(numbers)));
  printf("\n");
  dump(reverse(cdr(cdr(numbers))));
  printf("\n");
  dump(reverse(cdr(cdr(cdr(numbers)))));
  printf("\n");
  dump(reverse(cdr(cdr(cdr(cdr(numbers))))));
  printf("\n");
  dump(cons(
    Symbol("+"),
    cons(
      Integer(1),
      cons(
        Integer(2),
        Nil
      )
    )
  ));
  printf("\n");
  dump(cons(
    Symbol("def"),
    cons(
      Symbol("add"),
      cons(
        cons(
          Symbol("a"),
          cons(
            Symbol("b"),
            Nil
          )
        ),
        cons(
          cons(
            Symbol("+"),
            cons(
              Symbol("a"),
              cons(
                Symbol("b"),
                Nil
              )
            )
          ),
          Nil
        )
      )
    )
  ));
  printf("\n");
  return 0;
}

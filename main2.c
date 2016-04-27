#include <assert.h>

#define SYMBOLS_BLOCK_SIZE 128
#define PAIRS_BLOCK_SIZE 16
#define API static

#include "src/dump.c"

// A list is made up of linked pairs. (value, next)
// A map is made up of linked pairs ((key, value), next)
// A string is a list of integers.


int main() {
  builtins = (const builtin_t[]){
    // atoms
    {"nil", 0},
    {"true", 0},
    {"false", 0},
    // builtins
    {"def", 0},
    {"set", 0},
    {".", 0},
    {"index", 0},
    {"lambda", 0},
    {"λ", 0},
    {"if", 0},
    {"unless", 0},
    {"?", 0},
    {"and", 0},
    {"or", 0},
    {"not", 0},
    {"print", 0},
    {"list", 0},
    {"read", 0},
    {"write", 0},
    {"exec", 0},
    {"escape", 0},
    {"sleep", 0},
    {"macro", 0},
    {"concat", 0},
    {"flat", 0},
    {"join", 0},
    {"shuffle", 0},
    // variable
    {"for", 0},
    {"for*", 0},
    {"map", 0},
    {"map*", 0},
    {"i-map", 0},
    {"i-map*", 0},
    {"iter", 0},
    {"reduce", 0},
    {"while", 0},
    {"do", 0},
    // operator
    {"+", 0},
    {"-", 0},
    {"*", 0},
    {"×", 0},
    {"/", 0},
    {"÷", 0},
    {"%", 0},
    {"<", 0},
    {"<=", 0},
    {"≤", 0},
    {">", 0},
    {">=", 0},
    {"≥", 0},
    {"=", 0},
    {"!=", 0},
    {"≠", 0},
    {0, 0},
  };

  assert(sizeof(pair_t) == 8);
  assert(sizeof(value_t) == 4);
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
        cons(Symbol("isProgrammer"),True),
        Nil
      )
    )
  );
  printf("tim.name: ");
  dump(mget(tim, Symbol("name")));
  printf(COFF"\ntim.age: ");
  dump(mget(tim, Symbol("age")));
  printf(COFF"\ntim.isProgrammer: ");
  dump(mget(tim, Symbol("isProgrammer")));
  printf(COFF"\ntim.wat: ");
  dump(mget(tim, Symbol("wat")));
  printf(COFF"\nhas name: ");
  dump(mhas(tim, Symbol("name")));
  printf(COFF"\nhas wat: ");
  dump(mhas(tim, Symbol("wat")));
  printf(COFF"\nset name: ");
  dump(tim = mset(tim, Symbol("name"), Symbol("Timbo")));
  printf(COFF"\nset 42: ");
  dump(tim = mset(tim, Integer(42), True));
  printf(COFF"\nset 42 on empty: ");
  dump(mset(Nil, Integer(42), True));
  printf(COFF"\n");
  dump(tim);
  printf(COFF"\n");
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
  printf("\n");
  dump(cons(jack, tim));
  printf(COFF"\nAppend tim and jack: ");
  dump(append(tim, jack));
  printf(COFF"\nJust tim: ");
  dump(tim);
  printf(COFF"\nReverse tim: ");
  dump(reverse(tim));
  printf(COFF"\nReverse numbers: ");
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
  printf(COFF"\n");
  return 0;
}

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
  value_t numbers = List(Integer(1), Integer(2), Integer(3), Integer(4));
  value_t tim = List(
    Mapping(name, Symbol("Tim")),
    Mapping(age, Integer(34)),
    Mapping(isProgrammer, True)
  );
  value_t jack = List(
    Mapping(name, Symbol("Jack")),
    Mapping(age, Integer(10)),
    Mapping(isProgrammer, True)
  );
  printf("tim.name: ");
  dump(mget(tim, Symbol("name")));
  printf("tim.age: ");
  dump(mget(tim, Symbol("age")));
  printf("tim.isProgrammer: ");
  dump(mget(tim, Symbol("isProgrammer")));
  printf("tim.wat: ");
  dump(mget(tim, Symbol("wat")));
  printf("has name: ");
  dump(mhas(tim, Symbol("name")));
  printf("has wat: ");
  dump(mhas(tim, Symbol("wat")));
  printf("set name: ");
  dump(tim = mset(tim, Symbol("name"), Symbol("Timbo")));
  printf("set 42: ");
  dump(tim = mset(tim, Integer(42), True));
  printf("set 42 on empty: ");
  dump(mset(Nil, Integer(42), True));
  dump(tim);
  dump(numbers);
  dump(cdr(numbers));
  dump(cdr(cdr(numbers)));
  dump(cdr(cdr(cdr(numbers))));
  dump(cdr(cdr(cdr(cdr(numbers)))));
  dump(cdr(cdr(cdr(cdr(cdr(numbers))))));
  dump(tim);
  dump(car(tim));
  dump(cdr(tim));
  dump(cons(tim, jack));
  dump(cons(jack, tim));
  printf("Append tim and jack: ");
  dump(append(tim, jack));
  printf("Just tim: ");
  dump(tim);
  printf("Reverse tim: ");
  dump(reverse(tim));
  printf("Reverse numbers: ");
  dump(reverse(numbers));
  dump(reverse(cdr(numbers)));
  dump(reverse(cdr(cdr(numbers))));
  dump(reverse(cdr(cdr(cdr(numbers)))));
  dump(reverse(cdr(cdr(cdr(cdr(numbers))))));
  dump(cons(
    Symbol("+"),
    cons(
      Integer(1),
      cons(
        Integer(2),
        True
      )
    )
  ));
  dump(List(
    Symbol("def"),
    Symbol("add"),
    List(
      Symbol("a"),
      Symbol("b")
    ),
    List(
      Symbol("+"),
      Symbol("a"),
      Symbol("b")
    )
  ));
  return 0;
}

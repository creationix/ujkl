#include <assert.h>

#include "src/dump.c"
#include "src/builtins.c"

void test() {

  assert(sizeof(pair_t) == 8);
  assert(sizeof(value_t) == 4);
  assert((Integer(1)).data == 1);
  assert((Integer(0)).data == 0);
  assert((Integer(-1)).data == -1);

  value_t numbers = List(Integer(1), Integer(2), Integer(3), Integer(4));
  value_t tim = List(
    Mapping(name, Symbol("Tim")),
    Mapping(age, Integer(34)),
    Mapping(isProgrammer, Bool(true))
  );
  value_t jack = List(
    Mapping(name, Symbol("Jack")),
    Mapping(age, Integer(10)),
    Mapping(isProgrammer, True)
  );
  print("tim.name: ");
  dump(mget(tim, Symbol("name")));
  print("tim.age: ");
  dump(mget(tim, Symbol("age")));
  print("tim.isProgrammer: ");
  dump(mget(tim, Symbol("isProgrammer")));
  print("tim.wat: ");
  dump(mget(tim, Symbol("wat")));
  print("has name: ");
  dump(mhas(tim, Symbol("name")));
  print("has wat: ");
  dump(mhas(tim, Symbol("wat")));
  print("set name: ");
  dump(tim = mset(tim, Symbol("name"), Symbol("Timbo")));
  print("set 42: ");
  dump(tim = mset(tim, Integer(42), True));
  print("set 42 on empty: ");
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
  print("Append tim and jack: ");
  dump(append(tim, jack));
  print("Just tim: ");
  dump(tim);
  print("Reverse tim: ");
  dump(reverse(tim));
  print("Reverse numbers: ");
  dump(reverse(numbers));
  dump(reverse(cdr(numbers)));
  dump(reverse(cddr(numbers)));
  dump(reverse(cdddr(numbers)));
  dump(reverse(cddddr(numbers)));
  value_t add = cons(
    Symbol("+"),
    cons(
      Integer(1),
      cons(
        Integer(2),
        Nil
      )
    )
  );
  dump(add);
  dump(eval(Nil, add));
  dump(List(
    Symbol("def"),
    Symbol("sequence"),
    List(
      Symbol("shuffle"),
      List(
        Symbol("concat"),
        List(
          Symbol("map"),
          List(
            Symbol("i"),
            Symbol("size"),
          ),
          List(
            Symbol("if"),
            List(
              Symbol("<"),
              List(
                Symbol("%"),
                Symbol("i"),
                Symbol("width")
              ),
              Symbol("ww")
            ),
            List(
              Symbol("list"),
              True,
              Symbol("i")
            )
          )
        ),
        List(
          Symbol("map"),
          List(
            Symbol("i"),
            Symbol("size"),
          ),
          List(
            Symbol("if"),
            List(
              Symbol("<"),
              List(
                Symbol("÷"),
                Symbol("i"),
                Symbol("width")
              ),
              Symbol("hh")
            ),
            List(
              Symbol("list"),
              False,
              Symbol("i")
            )
          )
        )
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

}

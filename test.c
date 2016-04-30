#include <assert.h>

#include "src/types.h"

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

  value_t env = List(
    Mapping(name, Symbol("Tim")),
    Mapping(age, Integer(34)),
    Mapping(isProgrammer, True)
  );
  env = cons(cons(Symbol("tim"),env),env);
  // mset(env, Symbol("tim"), env);
  mset(env, Symbol("add"), Symbol("+"));

  print("env: ");
  dump(env);

  const value_t *expressions = (const value_t[]){
    List(Symbol("-"),Integer(5),Integer(2)),
    List(Symbol("+"),Integer(1),Integer(2),Integer(3)),
    List(Symbol("add"),Integer(1),Integer(2),Integer(3)),
    List(Symbol("+"),
      List(Symbol("*"),Integer(2),Integer(3)),
      List(Symbol("/"),Integer(10),Integer(3)),
      Integer(1)
    ),
    List(Symbol("+"),Integer(1),Integer(2)),
    List(Integer(1),Integer(2),Integer(3)),
    Integer(42),
    Symbol("+"),
    Symbol("what"),
    Symbol("name"),
    List(Symbol("list"), Symbol("age"), Symbol("isProgrammer")),
    List(Symbol("quote"), Symbol("age"), Symbol("isProgrammer")),
    List(Symbol("age"), Symbol("isProgrammer")),
    Symbol("tim"),
    List(Symbol("tim")),
    List(Symbol("def"),
      Symbol("greet"),
      List(Symbol("person")),
      List(Symbol("print"),
        cons(Symbol("quote"),Symbol("hello")),
        List(Symbol("mget"),Symbol("person"),cons(Symbol("quote"),Symbol("name"))))
    ),
    List(Symbol("set"),
      Symbol("tim"),
      List(
        Symbol("quote"),
        Mapping(name,Symbol("Tim")),
        Mapping(age,Integer(34)),
        Mapping(is-programmer,True)
      )
    ),
    List(Symbol("greet"),Symbol("tim")),
    cons(Symbol("list"),Integer(42)),
    Nil,
  };

  for (int i = 0; !isNil(expressions[i]); i++) {
    print("> ");
    dump(expressions[i]);
    dump(eval(env, expressions[i]));
  }
}

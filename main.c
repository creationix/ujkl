
#define VM_VERSION "MonkeyRocker"
#define SYMBOLS_BLOCK_SIZE 128
#define PAIRS_BLOCK_SIZE 16
#define THEME tim
// #define MAX_PINS 22
// #define TRACE
#define API static

#include "src/data.c"
#include "src/lists.c"
#include "src/tables.c"
#include "src/dump.c"
#include "src/editor.c"
#include "src/print.c"
#include "src/runtime.c"
#include "src/symbols.c"

static value_t repl;

// Look for dots and parse into list of symbols if found.
static value_t getSymbols(const char* start, const char* end) {
  value_t parts = Nil;
  const char* s = start;
  const char* i = s;
  while (i < end) {
    if (*i == '.' && i > s) {
      value_t sym = SymbolRange(s, i);
      parts = cons(sym, parts);
      s = i + 1;
    }
    i++;
  }
  if (isNil(parts)) return SymbolRange(start, end);
  return list_reverse(cons(SymbolRange(s,end), parts));
}

static value_t eval_dump_fn(value_t env, value_t val) {
  dump(eval(env, val));
  return Nil;
}

static void parse(const char *data) {
  value_t stack = Nil;
  value_t value = Nil;
  bool quote = false;
  bool neg = false;
  while (*data) {
    // Skip whitespace
    if (*data == ' ') {
      data++;
    }
    // Push stack when on open paren
    else if (*data == '(' || *data == '[') {
      stack = cons(value, stack);
      value = Nil;
      if (quote) {
        quote = false;
        value = cons(quoteSym, value);
      }
      if (*data == '[') {
        value = cons(listSym, value);
      }
      data++;
    }
    // pop stack on close paren
    else if (*data == ')' || *data == ']') {
      value_t fixed = Nil;
      bool dot = false;
      while (value.type == PairType) {
        value_t slot = car(value);
        if (eq(slot, Dot)) {
          dot = true;
        }
        else {
          if (dot) {
            dot = false;
            fixed = car(fixed);
          }
          fixed = cons(slot, fixed);
        }
        value = cdr(value);
      }
      value = cons(fixed, car(stack));
      stack = cdr(stack);
      data++;
    }
    else if (*data == '\'') {
      quote = true;
      data++;
    }
    else if (*data == '-' && *(data + 1) &&
             *(data + 1) >= '0' && *(data + 1) <= '9') {
      neg = true;
      data++;
    }
    else if (*data >= '0' && *data <= '9') {
      int num = 0;
      while (*data >= '0' && *data <= '9') {
        num = num * 10 + *data - '0';
        data++;
      }
      if (neg) {
        neg = false;
        num = -num;
      }
      value_t atom = Integer(num);
      if (quote) {
        quote = false;
        atom = cons(quoteSym, atom);
      }
      value = cons(atom, value);
    }
    else {
      const char* start = data;
      while (*data && *data != ' ' &&
             *data != '(' && *data != ')' &&
             *data != '[' && *data != ']') {
        data++;
      }
      value_t atom;
      int len = data - start;
      if (len == 1 && start[0] == '.') {
        atom = Dot;
      }
      else if (len == 3 &&
          start[0] == 'n' &&
          start[1] == 'i' &&
          start[2] == 'l') {
        atom = Nil;
      }
      else if (len == 4 &&
          start[0] == 't' &&
          start[1] == 'r' &&
          start[2] == 'u' &&
          start[3] == 'e') {
        atom = True;
      }
      else if (len == 5 &&
          start[0] == 'f' &&
          start[1] == 'a' &&
          start[2] == 'l' &&
          start[3] == 's' &&
          start[4] == 'e') {
        atom = False;
      }
      else {
        atom = getSymbols(start, data);
      }
      if (quote) {
        quote = false;
        atom = cons(quoteSym, atom);
      }
      value = cons(atom, value);
    }
  }
  value = list_reverse(value);

  print("\r\x1b[K");
  print(prompt);
  dump_line(value);
  list_each(value, repl, eval_dump_fn);
}

// Passes values through unaffected
static value_t _quote(value_t env, value_t args) {
  (void)(env);
  return args;
}

// Evals values, but otherwise passes them through
static value_t _list(value_t env, value_t args) {
  return list_map(args, env, eval);
}

// static value_t call_fn(value_t env_fn, value_t value) {
//   value_t env = car(env_fn);
//   value_t fn = cdr(env_fn);
//   return apply(env, fn, cons(value, Nil));
// }

// static value_t _map(value_t env, value_t args) {
//   var2(env, args, list, fn);
//   return map(cons(env, fn), list, call_fn);
// }
//
// static value_t _each(value_t env, value_t args) {
//   var2(env, args, list, fn);
//   return each(cons(env, fn), list, call_fn);
// }


// Define a function
// static value_t _def(value_t env, value_t args) {
//   value_t key = car(args);
//   value_t fn = cdr(args);
//   set(env, key, fn);
//   return key;
// }

// static bool notMap(value_t map) {
//   while (map.type == PairType) {
//     if (car(map).type != PairType) return true;
//     map = cdr(map);
//     if (isNil(map)) return false;
//   }
//   return true;
// }

// static value_t _get(value_t env, value_t args) {
//   value_t key = car(args);
//   return notList(key) ? get(env, key) : aget(env, key);
// }

// static value_t _has(value_t env, value_t args) {
//   bool found = true;
//   while (args.type == PairType) {
//     pair_t pair = pairs[args.data];
//     value_t key = pair.left;
//     value_t res = notList(key) ? has(env, key) : ahas(env, key);
//     if (eq(res, TypeError)) return res;
//     if (eq(res, False)) found = false;
//     args = pair.right;
//   }
//   if (isNil(args)) return Bool(found);
//   return TypeError;
// }

// static value_t _set(value_t env, value_t args) {
//   while (args.type == PairType) {
//     pair_t pair = pairs[args.data];
//     value_t key = pair.left;
//     if (pair.right.type != PairType) return TypeError;
//     pair = pairs[pair.right.data];
//     value_t value = eval(env, pair.left);
//     value_t res = notList(key) ?
//       set(env, key, value) :
//       aset(env, key, value);
//     if (eq(res, TypeError)) return res;
//     args = pair.right;
//   }
//   if (isNil(args)) return True;
//   return TypeError;
// }

// static value_t _del(value_t env, value_t args) {
//   while (args.type == PairType) {
//     pair_t pair = pairs[args.data];
//     value_t key = pair.left;
//     value_t res = notList(key) ?
//       del(env, key) :
//       adel(env, key);
//     if (eq(res, TypeError)) return res;
//     args = pair.right;
//   }
//   if (isNil(args)) return True;
//   return TypeError;
// }

static value_t _cons(value_t env, value_t args) {
  var2(env, args, a, b)
  return cons(a, b);
}

static value_t _car(value_t env, value_t args) {
  var1(env, args, a)
  return car(a);
}

static value_t _cdr(value_t env, value_t args) {
  var1(env, args, a)
  return cdr(a);
}

static value_t _set_car(value_t env, value_t args) {
  var2(env, args, a, b)
  return Bool(set_car(a, b));
}

static value_t _set_cdr(value_t env, value_t args) {
  var2(env, args, a, b)
  return Bool(set_cdr(a, b));
}

// static value_t _add(value_t env, value_t args) {
//   int sum = 0;
//   while (args.type == PairType) {
//     pair_t pair = get_pair(args);
//     value_t value = eval(env, pair.left);
//     if (value.type == IntegerType) {
//       sum += value.data;
//     }
//     args = pair.right;
//   }
//   return Integer(sum);
// }
//
// static value_t _mul(value_t env, value_t args) {
//   int sum = 1;
//   while (args.type == PairType) {
//     pair_t pair = get_pair(args);
//     value_t value = eval(env, pair.left);
//     if (value.type == IntegerType) {
//       sum *= value.data;
//     }
//     args = pair.right;
//   }
//   return Integer(sum);
// }
//
// static value_t _sub(value_t env, value_t args) {
//   var2(env, args, a, b)
//   if (a.type == IntegerType && b.type == IntegerType) {
//     return Integer(a.data - b.data);
//   }
//   return Undefined;
// }
//
// static value_t _div(value_t env, value_t args) {
//   var2(env, args, a, b)
//   if (a.type == IntegerType && b.type == IntegerType) {
//     return Integer(a.data / b.data);
//   }
//   return Undefined;
// }
//
// static value_t _mod(value_t env, value_t args) {
//   var2(env, args, a, b)
//   if (a.type == IntegerType && b.type == IntegerType) {
//     return Integer(a.data % b.data);
//   }
//   return Undefined;
// }

// static value_t _lt(value_t env, value_t args) {
//   var2(env, args, a, b)
//   if (a.type == IntegerType && b.type == IntegerType) {
//     return Bool(a.data < b.data);
//   }
//   return Undefined;
// }
//
// static value_t _lte(value_t env, value_t args) {
//   var2(env, args, a, b)
//   if (a.type == IntegerType && b.type == IntegerType) {
//     return Bool(a.data <= b.data);
//   }
//   return Undefined;
// }
//
// static value_t _gt(value_t env, value_t args) {
//   var2(env, args, a, b)
//   if (a.type == IntegerType && b.type == IntegerType) {
//     return Bool(a.data > b.data);
//   }
//   return Undefined;
// }
//
// static value_t _gte(value_t env, value_t args) {
//   var2(env, args, a, b)
//   if (a.type == IntegerType && b.type == IntegerType) {
//     return Bool(a.data >= b.data);
//   }
//   return Undefined;
// }
//
// static value_t _eq(value_t env, value_t args) {
//   var2(env, args, a, b)
//   return Bool(eq(a,b));
// }
//
// static value_t _neq(value_t env, value_t args) {
//   var2(env, args, a, b)
//   return Bool(!eq(a,b));
// }

static value_t _print(value_t env, value_t args) {
  return list_each(args, env, eval_dump_fn);
}

// fn1(_ilen, list, return ilen(list);)
// fn2(_iget, list, key, return iget(list, key);)
// fn3(_iset, list, key, value, return iset(list, key, value);)

#ifdef MAX_PINS
static bool exported[MAX_PINS];
static bool direction[MAX_PINS]; // false - in, true - out

const char* pinnums[] = {
  "00","01","02","03","04","05","06","07","08","09",
  "10","11","12","13","14","15","16","17","18","19",
  "20","21"};

static bool setup_pin(int pin, bool dir) {
  if (pin < 0 || pin >= MAX_PINS) return true;
  if (!exported[pin]) {
    exported[pin] = true;
    print("Exporting pin ");
    print_int(pin);
    print_char('\n');
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    write(fd, pinnums[pin], 2);
    close(fd);
  }
  if (direction[pin] != dir) {
    direction[pin] = dir;
    print("Setting direction on ");
    print_int(pin);
    print(" to ");
    print(dir ? "OUT" : "IN");
    print_char('\n');
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    write(fd, pinnums[pin], 2);
    close(fd);
  }
  return false;
}

static bool write_pin(int pin, bool state) {
  if (setup_pin(pin, true)) return true;
  print("Setting state of ");
  print_int(pin);
  print(" to ");
  print(state ? "HIGH" : "LOW");
  print_char('\n');
  return false;
}


static value_t _on(value_t env, value_t args) {
  var1(env, args, pin)
  if (pin.type != IntegerType) return TypeError;
  if (write_pin(pin.data, true)) return RangeError;
  return True;
}
static value_t _off(value_t env, value_t args) {
  var1(env, args, pin)
  if (pin.type != IntegerType) return TypeError;
  if (write_pin(pin.data, false)) return RangeError;
  return False;
}
/*
static value_t _toggle(value_t env, value_t args) {
}
static value_t _read(value_t env, value_t args) {
}
static value_t _write(value_t env, value_t args) {
}
*/

#endif

static value_t _is_list(value_t env, value_t args) {
  return Bool(is_list(eval(env, car(args))));
}
static value_t _list_length(value_t env, value_t args) {
  value_t list = eval(env, car(args));
  if (!is_list(list)) return TypeError;
  return Integer(list_length(list));
}
static value_t _list_reverse(value_t env, value_t args) {
  value_t list = eval(env, car(args));
  if (!is_list(list)) return TypeError;
  return list_reverse(list);
}
static value_t _list_append(value_t env, value_t args) {
  value_t list = eval(env, car(args));
  if (!is_list(list)) return TypeError;
  return list_append(list, list_map(cdr(args), env, eval));
}
static value_t _list_concat(value_t env, value_t args) {
  value_t lists = Nil;
  while (args.type == PairType) {
    pair_t pair = get_pair(args);
    value_t list = eval(env, pair.left);
    if (!is_list(list)) return TypeError;
    lists = cons(list, lists);
    args = pair.right;
  }
  if (!isNil(args)) return TypeError;
  value_t combined = Nil;
  while (lists.type == PairType) {
    pair_t pair = get_pair(lists);
    combined = list_append(pair.left, combined);
    lists = pair.right;
  }
  return combined;
}

static value_t _list_sort(value_t env, value_t args) {
  value_t list = eval(env, car(args));
  if (!is_list(list)) return TypeError;
  return list_sort(list);
}

static value_t _list_iget(value_t env, value_t args) {
  value_t list = eval(env, car(args));
  if (!is_list(list)) return TypeError;
  value_t index = eval(env, car(cdr(args)));
  if (index.type != IntegerType) return TypeError;
  return list_get(list, index.data);
}

static value_t _list_iset(value_t env, value_t args) {
  value_t list = eval(env, car(args));
  if (!is_list(list)) return TypeError;
  args = cdr(args);
  value_t index = eval(env, car(args));
  if (index.type != IntegerType) return TypeError;
  args = cdr(args);
  value_t value = car(args);
  return list_set(list, index.data, value);
}

static value_t _list_has(value_t env, value_t args) {
  value_t list = eval(env, car(args));
  if (!is_list(list)) return TypeError;
  args = cdr(args);
  bool has = true;
  while (has && args.type == PairType) {
    has = list_has(list, eval(env, car(args)));
    args = cdr(args);
  }
  return Bool(has);
}
static value_t _list_add(value_t env, value_t args) {
}
static value_t _list_remove(value_t env, value_t args) {
}

static const builtin_t *functions = (const builtin_t[]){
  {"print", _print},
  {"list", _list},
  {"quote", _quote},
  {"eval", eval},
  {"cons", _cons},
  {"car", _car},
  {"cdr", _cdr},
  {"set_car", _set_car},
  {"set_cdr", _set_cdr},
  // {"set", _set},

  {"list?", _is_list},
  {"length?", _list_length},
  {"reverse", _list_reverse},
  {"append!", _list_append},
  {"concat!", _list_concat},
  {"sort", _list_sort},
  {"iget", _list_iget},
  {"iset!", _list_iset},
  {"has?", _list_has},
  {"add!", _list_add},
  {"remove!", _list_remove},
  // - (sort list ((l r)...)->rank) -> list - sort a list with optional sort function


  // {"map", _map},
  // {"each", _each},
  // {"def", _def},
  // {"has", _has},
  // {"get", _get},
  // {"del", _del},
#ifdef MAX_PINS
  {"on", _on},
  {"off", _off},
#endif
  // {"+", _add},
  // {"-", _sub},
  // {"*", _mul},
  // {"/", _div},
  // {"%", _mod},
  // {"<", _lt},
  // {"<=", _lte},
  // {">", _gt},
  // {">=", _gte},
  // {"=", _eq},
  // {"!=", _neq},
  {0,0},
};

int main() {
  // Initialize symbol system with our builtins.
  symbols_init(functions);
  quoteSym = Symbol("quote");
  listSym = Symbol("list");

  // Initialize repl environment with a version variable and ref to self.
  repl = table_set(Nil, Symbol("env"), Nil);
  table_set(repl, Symbol("env"), repl);
  table_set(repl, Symbol("version"), Symbol(VM_VERSION));

  const char** lines = (const char*[]) {
    "(append! nil 1 2 3)",
    "(append! [1] 2 3)",
    "(append! [1 2] 3)",
    "(sort [1 3 5 7 6 4 2 52 34 64 24 23])",
    0
  };

  prompt = "> ";

  for (int i = 0; lines[i]; i++) {
    parse(lines[i]);
  }


  // Start the repl
  onLine = parse;
  while (editor_step());
}


#define VM_VERSION "MonkeyRocker"
#define SYMBOLS_BLOCK_SIZE 128
#define PAIRS_BLOCK_SIZE 16
#define THEME tim
// #define MAX_PINS 22
#define TRACE
#define API static

#include "src/data.c"
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
  return reverse(cons(SymbolRange(s,end), parts));
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
  value = reverse(value);

  print("\r\x1b[K");
  print(prompt);
  dump_line(value);
  each(repl, value, eval_dump_fn);
}

static value_t _list(value_t env, value_t args) {
  return map(env, args, eval);
}

static value_t call_fn(value_t env_fn, value_t value) {
  value_t env = car(env_fn);
  value_t fn = cdr(env_fn);
  return apply(env, fn, cons(value, Nil));
}

static value_t _map(value_t env, value_t args) {
  var2(env, args, list, fn);
  return map(cons(env, fn), list, call_fn);
}

static value_t _each(value_t env, value_t args) {
  var2(env, args, list, fn);
  return each(cons(env, fn), list, call_fn);
}

// Passed values through unaffected
static value_t _quote(value_t env, value_t args) {
  (void)(env);
  return args;
}

// Define a function
static value_t _def(value_t env, value_t args) {
  value_t key = car(args);
  value_t fn = cdr(args);
  set(env, key, fn);
  return key;
}

static bool notList(value_t list) {
  while (list.type == PairType) {
    list = cdr(list);
    if (isNil(list)) return false;
  }
  return true;
}

// static bool notMap(value_t map) {
//   while (map.type == PairType) {
//     if (car(map).type != PairType) return true;
//     map = cdr(map);
//     if (isNil(map)) return false;
//   }
//   return true;
// }

static value_t _get(value_t env, value_t args) {
  value_t key = car(args);
  return notList(key) ? get(env, key) : aget(env, key);
}

static value_t _has(value_t env, value_t args) {
  bool found = true;
  while (args.type == PairType) {
    pair_t pair = pairs[args.data];
    value_t key = pair.left;
    value_t res = notList(key) ? has(env, key) : ahas(env, key);
    if (eq(res, TypeError)) return res;
    if (eq(res, False)) found = false;
    args = pair.right;
  }
  if (isNil(args)) return Bool(found);
  return TypeError;
}

static value_t _set(value_t env, value_t args) {
  while (args.type == PairType) {
    pair_t pair = pairs[args.data];
    value_t key = pair.left;
    if (pair.right.type != PairType) return TypeError;
    pair = pairs[pair.right.data];
    value_t value = eval(env, pair.left);
    value_t res = notList(key) ?
      set(env, key, value) :
      aset(env, key, value);
    if (eq(res, TypeError)) return res;
    args = pair.right;
  }
  if (isNil(args)) return True;
  return TypeError;
}

static value_t _del(value_t env, value_t args) {
  while (args.type == PairType) {
    pair_t pair = pairs[args.data];
    value_t key = pair.left;
    value_t res = notList(key) ?
      del(env, key) :
      adel(env, key);
    if (eq(res, TypeError)) return res;
    args = pair.right;
  }
  if (isNil(args)) return True;
  return TypeError;
}

static value_t _car(value_t env, value_t args) {
  var1(env, args, a)
  return car(a);
}

static value_t _cdr(value_t env, value_t args) {
  var1(env, args, a)
  return cdr(a);
}

static value_t _add(value_t env, value_t args) {
  int sum = 0;
  while (args.type == PairType) {
    pair_t pair = getPair(args);
    value_t value = eval(env, pair.left);
    if (value.type == IntegerType) {
      sum += value.data;
    }
    args = pair.right;
  }
  return Integer(sum);
}

static value_t _mul(value_t env, value_t args) {
  int sum = 1;
  while (args.type == PairType) {
    pair_t pair = getPair(args);
    value_t value = eval(env, pair.left);
    if (value.type == IntegerType) {
      sum *= value.data;
    }
    args = pair.right;
  }
  return Integer(sum);
}

static value_t _sub(value_t env, value_t args) {
  var2(env, args, a, b)
  if (a.type == IntegerType && b.type == IntegerType) {
    return Integer(a.data - b.data);
  }
  return Undefined;
}

static value_t _div(value_t env, value_t args) {
  var2(env, args, a, b)
  if (a.type == IntegerType && b.type == IntegerType) {
    return Integer(a.data / b.data);
  }
  return Undefined;
}

static value_t _mod(value_t env, value_t args) {
  var2(env, args, a, b)
  if (a.type == IntegerType && b.type == IntegerType) {
    return Integer(a.data % b.data);
  }
  return Undefined;
}

static value_t _lt(value_t env, value_t args) {
  var2(env, args, a, b)
  if (a.type == IntegerType && b.type == IntegerType) {
    return Bool(a.data < b.data);
  }
  return Undefined;
}

static value_t _lte(value_t env, value_t args) {
  var2(env, args, a, b)
  if (a.type == IntegerType && b.type == IntegerType) {
    return Bool(a.data <= b.data);
  }
  return Undefined;
}

static value_t _gt(value_t env, value_t args) {
  var2(env, args, a, b)
  if (a.type == IntegerType && b.type == IntegerType) {
    return Bool(a.data > b.data);
  }
  return Undefined;
}

static value_t _gte(value_t env, value_t args) {
  var2(env, args, a, b)
  if (a.type == IntegerType && b.type == IntegerType) {
    return Bool(a.data >= b.data);
  }
  return Undefined;
}

static value_t _eq(value_t env, value_t args) {
  var2(env, args, a, b)
  return Bool(eq(a,b));
}

static value_t _neq(value_t env, value_t args) {
  var2(env, args, a, b)
  return Bool(!eq(a,b));
}

static value_t _print(value_t env, value_t args) {
  return each(env, args, eval_dump_fn);
}

fn1(_ilen, list, return ilen(list);)
fn2(_iget, list, key, return iget(list, key);)
fn3(_iset, list, key, value, return iset(list, key, value);)

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



static const builtin_t *functions = (const builtin_t[]){
  {"eval", eval},
  {"list", _list},
  {"map", _map},
  {"each", _each},
  {"quote", _quote},
  {"def", _def},
  {"set", _set},
  {"has", _has},
  {"get", _get},
  {"del", _del},
  {"car", _car},
  {"cdr", _cdr},
#ifdef MAX_PINS
  {"on", _on},
  {"off", _off},
#endif
  {"+", _add},
  {"-", _sub},
  {"*", _mul},
  {"/", _div},
  {"%", _mod},
  {"<", _lt},
  {"<=", _lte},
  {">", _gt},
  {">=", _gte},
  {"=", _eq},
  {"!=", _neq},
  {"ilen", _ilen},
  {"iget", _iget},
  {"iset", _iset},
  // {"sadd", _sadd},
  // {"sdel", _sdel},
  // {"shas", _shas},
  {"print", _print},
  {0,0},
};

int main() {
  // Initialize symbol system with our builtins.
  symbols_init(functions);
  quoteSym = Symbol("quote");
  listSym = Symbol("list");

  // Initialize repl environment with a version variable and ref to self.
  repl = set(Nil, Symbol("env"), Nil);
  set(repl, Symbol("env"), repl);
  // set(repl, Symbol("version"), Symbol(VM_VERSION));

  const char** lines = (const char*[]) {
    "(+ 1 2)",
    "(print 'Hello 'world!)",
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

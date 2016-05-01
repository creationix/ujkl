
#define VM_VERSION "MonkeyRocker"
#define SYMBOLS_BLOCK_SIZE 128
#define PAIRS_BLOCK_SIZE 16
#define THEME tim
// #define TRACE
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
  while (value.type == PairType) {
    dump(eval(repl, car(value)));
    value = cdr(value);
  }
}

static value_t _list(value_t env, value_t args) {
  (void)(env);
  value_t node = args;
  value_t res = Nil;
  // TODO: find a way to implement this thas doesn't cause so much GC pressure.
  while (node.type == PairType) {
    pair_t pair = getPair(node);
    res = cons(eval(env, pair.left), res);
    node = pair.right;
  }
  return reverse(res);
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
  value_t values = Nil;
  while (args.type == PairType) {
    pair_t pair = pairs[args.data];
    value_t key = pair.left;
    value_t res = notList(key) ? get(env, key) : aget(env, key);
    if (eq(res, TypeError)) return res;
    values = cons(res, values);
    args = pair.right;
  }
  if (isNil(args)) return reverse(values);
  return TypeError;
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
  args = _list(env, args);
  value_t node = args;
  dump_line(node);
  return Nil;
}

fn1(_ilen, list, return ilen(list);)
fn2(_iget, list, key, return iget(list, key);)
fn3(_iset, list, key, value, return iset(list, key, value);)

static const builtin_t *functions = (const builtin_t[]){
  {"eval", eval},
  {"list", _list},
  {"quote", _quote},
  {"def", _def},
  {"set", _set},
  {"has", _has},
  {"get", _get},
  {"del", _del},
  {"car", _car},
  {"cdr", _cdr},
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
    // "(+ 1 2)",
    // "'(Hello world!)'",
    // "(set jack.stats.age 10 jack.name 'Jack)",
    // "(has jack.stats.age)",
    // "(get jack.name)",
    // "(get jack.other.stuff)",
    // "(has jack.stats.other)",
    // "(get jack.stats.other)",
    // "jack",
    "(set c.b.a 0)",
    "(set a.b.c 10) (set a.b.d 20)",
    "env",
    "(del a.b.c)",
    "env",
    "(del (a))",
    "env",
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

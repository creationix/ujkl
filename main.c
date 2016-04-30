#include <assert.h>

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

#include "test.c"

static value_t repl;

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
        atom = True;
      }
      else {
        atom = SymbolRange(start, data);
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
  while (node.type == PairType) {
    pair_t pair = getPair(node);
    pair.left = eval(env, pair.left);
    setPair(node, pair);
    node = pair.right;
  }
  return args;
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
  mset(env, key, fn);
  return key;
}

static value_t _set(value_t env, value_t args) {
  value_t key = car(args);
  value_t value = eval(env, cdar(args));
  mset(env, key, value);
  return key;
}

static value_t _add(value_t env, value_t args) {
  int sum = 0;
  args = _list(env, args);
  while (args.type == PairType) {
    pair_t pair = getPair(args);
    if (pair.left.type == IntegerType) {
      sum += pair.left.data;
    }
    args = pair.right;
  }
  return Integer(sum);
}

static value_t _sub(value_t env, value_t args) {
  int sum;
  args = _list(env, args);
  bool first = true;
  while (args.type == PairType) {
    pair_t pair = getPair(args);
    if (pair.left.type == IntegerType) {
      if (first) {
        first = false;
        sum = pair.left.data;
      }
      else {
        sum -= pair.left.data;
      }
    }
    args = pair.right;
  }
  return first ? Undefined : Integer(sum);
}

static value_t _mul(value_t env, value_t args) {
  int sum = 1;
  args = _list(env, args);
  while (args.type == PairType) {
    pair_t pair = getPair(args);
    if (pair.left.type == IntegerType) {
      sum *= pair.left.data;
    }
    args = pair.right;
  }
  return Integer(sum);
}

static value_t _div(value_t env, value_t args) {
  int sum;
  args = _list(env, args);
  bool first = true;
  while (args.type == PairType) {
    pair_t pair = getPair(args);
    if (pair.left.type == IntegerType) {
      if (first) {
        first = false;
        sum = pair.left.data;
      }
      else {
        sum /= pair.left.data;
      }
    }
    else {
      return Undefined;
    }
    args = pair.right;
  }
  return first ? Undefined : Integer(sum);
}

static value_t _print(value_t env, value_t args) {
  args = _list(env, args);
  value_t node = args;
  dump_line(node);
  return Nil;
}

fn2(_mget, map, key, return mget(map, key);)
fn3(_mset, map, key, value, return mset(map, key, value);)
fn2(_mhas, map, key, return mhas(map, key);)

static const builtin_t *functions = (const builtin_t[]){
  {"list", _list},
  {"quote", _quote},
  {"def", _def},
  {"set", _set},
  {"+", _add},
  {"-", _sub},
  {"*", _mul},
  {"/", _div},
  {"mget", _mget},
  {"mset", _mset},
  {"mhas", _mhas},
  {"print", _print},
  {0,0},
};

int main() {
  // Initialize symbol system with our builtins.
  symbols_init(functions);
  quoteSym = Symbol("quote");
  listSym = Symbol("list");

  // Initialize repl environment with a version variable and ref to self.
  repl = List(
    Mapping(version,Symbol(VM_VERSION)),
    Mapping(env,Nil)
  );
  mset(repl, Symbol("env"),repl);

  // Initialize the repl prompt and callback
  prompt = "> ";
  onLine = parse;
  // Run the line editor
  while (editor_step());
}

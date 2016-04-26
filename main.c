#include <unistd.h>
#include "line-reader.c"
#include "symbol.c"

pair_t *pairs;
int num_pairs;

void writeValue(value_t value) {
  writeCString(" *");
  writeInt((int)value.num);
  if (value.num == (uint32_t)~0) {
    writeCString(" Undefined");
    return;
  }
  switch (value.type) {
    case Integer:
      writeChar(' ');
      writeInt(value.value);
      break;
    case Atom:
      switch ((atom_t)value.value) {
        case Nil:
          writeCString(" nil");
          break;
        case True:
          writeCString(" true");
          break;
        case False:
          writeCString(" false");
          break;
      }
      break;
    case Symbol:
      writeChar(' ');
      writeInt(value.value);
      writeChar(':');
      writeCString(getSymbol(value.value));
      break;
    case Pair: {
      pair_t pair = pairs[value.value];
      writeCString(" (");
      writeValue(pair.left);
      writeCString(" .");
      writeValue(pair.right);
      writeCString(" )");
      break;
    }
  }
}

value_t makePair(value_t left, value_t right) {
  int index = 0;
  if (!pairs) {
    num_pairs = 1;
    pairs = malloc(sizeof(pair_t));
  }
  else {
    // Look for empty slots
    for (;index < num_pairs; index++) {
      if (!pairs[index].num) break;
    }
    // If none are found, allocate a new slot.
    if (index == num_pairs) {
      num_pairs++;
      pairs = realloc(pairs, (size_t)num_pairs * sizeof(pair_t));
    }
  }
  pairs[index].left = left;
  pairs[index].right = right;
  return (value_t){
    .type = Pair,
    .gc = 0,
    .value = index
  };
}

value_t makeInt(int val) {
  return (value_t){
    .type = Integer,
    .gc = 0,
    .value = val
  };
}

value_t makeNil() {
  return (value_t){
    .type = Atom,
    .gc = 0,
    .value = Nil
  };
}

value_t makeBool(bool value) {
  return (value_t){
    .type = Atom,
    .gc = 0,
    .value = value ? True : False
  };
}

value_t makeSymbol(const char* name, int len) {
  int index = matchSymbol(name, len);
  if (index == 0) return makeNil();
  else if (index == 1 || index == 2) return makeBool(index == 1);
  return (value_t){
    .type = Symbol,
    .gc = 0,
    .value = index
  };
}

static void handleInput(const char *line) {
  while (*line) {
    if (*line == '(') {
      line++;
      writeCString(" (");
      continue;
    }
    if (*line == ')') {
      line++;
      writeCString(" )");
      continue;
    }
    if (*line == '.') {
      line++;
      writeCString(" .");
      continue;
    }
    if (*line == ' ') {
      line++;
      continue;
    }
    if (*line >= '0' && *line <= '9') {
      int n = 0;
      while (*line >= '0' && *line <= '9') {
        n = n * 10 + (*line - '0');
        line++;
      }
      writeValue(makeInt(n));
      continue;
    }
    const char *start = line;
    while(*line && *line != '.' && *line != '(' && *line != ')' && *line != ' ') { line++; }
    writeValue(makeSymbol(start, (int)(line - start)));
  }
  writeCString("\n");
}

int main() {
  onLine = handleInput;
  prompt = "> ";
  writeValue((value_t){
    .type = 3,
    .gc = 1,
    .value = ~0
  });
  writeValue(makePair(makeSymbol("add", 0), makeInt(42)));
  writeChar('\n');
  writeValue(makeInt(42));
  writeChar('\n');
  startEditor();
  while(stepEditor());
  stopEditor();
  return 0;
}

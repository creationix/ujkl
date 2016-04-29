#ifndef PRINT_C
#define PRINT_C

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>

#ifndef API
#define API
#endif

#define MAX_LINE_LENGTH 64
static char write_buffer[MAX_LINE_LENGTH];
static size_t write_index;

static bool needFlush;

API void print_flush() {
  write(1, write_buffer, write_index);
  write_index = 0;
  needFlush = false;
}

static void check() {
  if (write_index == MAX_LINE_LENGTH) print_flush();
}

API bool print_int(int num) {
  int digit = 1;
  if (num < 0) {
    check();
    write_buffer[write_index++] = '-';
    num = -num;
  }
  while (digit <= num) { digit *= 10; }
  if (digit > 1) digit /= 10;
  while (digit > 0) {
    check();
    write_buffer[write_index++] = 48 + ((num / digit) % 10);
    digit /= 10;
  }
  return true;
}

API bool print_char(const char c) {
  check();
  write_buffer[write_index++] = c;
  if (c == '\n') print_flush();
  return true;
}

API bool print(const char* value) {
  while (*value) {
    check();
    write_buffer[write_index++] = *value;
    if (*value++ == '\n') needFlush = true;
  }
  if (needFlush) print_flush();
  return true;
}

#endif

#ifndef WRITE_BUFFER_C
#define WRITE_BUFFER_C
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 80
static char write_buffer[MAX_LINE_LENGTH];
static size_t write_index;

bool writeInt(int num) {
  int digit = 1;
  if (num < 0) {
    if (write_index >= MAX_LINE_LENGTH) { return false; }
    write_buffer[write_index++] = '-';
    num = -num;
  }
  while (digit <= num) { digit *= 10; }
  if (digit > 1) digit /= 10;
  while (digit > 0) {
    if (write_index >= MAX_LINE_LENGTH) { return false; }
    write_buffer[write_index++] = 48 + ((num / digit) % 10);
    digit /= 10;
  }
  return true;
}

bool writeChar(const char c) {
  if (write_index >= MAX_LINE_LENGTH) { return false; }
  write_buffer[write_index++] = c;
  return true;
}

bool writeCString(const char* value) {
  while (*value) {
    if (write_index >= MAX_LINE_LENGTH) { return false; }
    write_buffer[write_index++] = *value++;
  }
  return true;
}

bool writeString(const char *value, int len) {
  while (len--) {
    if (write_index >= MAX_LINE_LENGTH) { return false; }
    write_buffer[write_index++] = *value++;
  }
  return true;
}


void writeFlush() {
  write(1, write_buffer, write_index);
  write_index = 0;
}

#endif

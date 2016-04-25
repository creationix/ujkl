#include <unistd.h>
#include "line-reader.c"

static const char *onLine(uint8_t *line, uint8_t len) {
  writeString(line, len);
  writeChar('\n');
  return 0;
}

int main() {
  startEditor("> ", onLine);
  while(stepEditor());
  stopEditor();
  return 0;
}

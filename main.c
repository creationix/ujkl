#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#define MAX_LINE_LENGTH 128
typedef struct line_s {
  int x;
  int length;
  uint8_t line[MAX_LINE_LENGTH];
} line_t;

typedef struct editor_s {
  enum mode_e {
    NORMAL,
    ESC,
    CSI
  } mode;
  int csi_args[2];
  int csi_num;
  char* prompt;
  line_t current;
  line_t memory;
} editor_t;

uint8_t write_buffer[MAX_LINE_LENGTH];
size_t write_index;

bool writeInt(int num) {
  int digit = 1;
  while (digit <= num) { digit *= 10; }
  while (digit > 0) {
    if (write_index >= MAX_LINE_LENGTH) { return false; }
    write_buffer[write_index++] = 48 + ((num / digit) % 10);
    digit /= 10;
  }
  return true;
}
bool writeCString(const char* value) {
  while (*value) {
    if (write_index >= MAX_LINE_LENGTH) { return false; }
    write_buffer[write_index++] = (uint8_t)*value++;
  }
  return true;
}
bool writeString(const char* value, size_t len) {
  while (len--) {
    if (write_index >= MAX_LINE_LENGTH) { return false; }
    write_buffer[write_index++] = (uint8_t)*value++;
  }
  return true;
}

bool writeChar(const char c) {
  if (write_index >= MAX_LINE_LENGTH) { return false; }
  write_buffer[write_index++] = (uint8_t)c;
  return true;
}

void writeFlush() {
  write(1, write_buffer, write_index);
  write_index = 0;
}

bool moveLeft(editor_t* editor, int n) {
  if (n == 0) return true;
  editor->current.x -= n;
  writeCString("\33[");
  writeInt(n);
  writeChar('D');
  writeFlush();
  return true;
}
bool moveRight(editor_t* editor, int n) {
  if (n == 0) return true;
  editor->current.x += n;
  writeCString("\33[");
  writeInt(n);
  writeChar('C');
  writeFlush();
  return true;
}

bool handleChar(editor_t* editor, int c) {
  if (c == 0) { goto refresh; }
  switch (editor->mode) {
  case NORMAL:
    // Insert printable characters into line
    if (c >= 0x20 && c < 0x7f) {
      goto insert;
    }

    // Handle the start of a CSI sequence
    if (c == 27) {
      editor->mode = ESC;
      return true;
    }

    // Handle Control+D
    if (c == 4) {
      // If there is data, clear it.
      if (editor->current.length) {
        editor->current.length = 0;
        editor->current.x = 0;
        goto refresh;
      }
      // Otherwise, exit the console.
      return false;
    }


    // Handle backspace
    if (c == 127) {
      if (editor->current.x > 0) {
        moveLeft(editor, 1);
        goto delete;
      }
      return true;
    }

    // Handle Enter
    if (c == 10) {
      editor->memory.x = 0;
      editor->memory.length = 0;
      write(1, "\n", 1);
      goto swap;
    }
    if (c == 12) { // Control+L clear screen
      write(1, "\33[2J\33[H", 7);
      goto refresh;
    }
    writeCString("\ncode ");
    writeInt(c);
    writeChar('\n');
    writeFlush();
    return true;
  case ESC:
    if (c == '[') {
      editor->csi_num = 0;
      editor->csi_args[0] = 0;
      editor->mode = CSI;
    }
    else {
      editor->mode = NORMAL;
    }
    return true;
  case CSI:
    if (c >= '0' && c <= '9') {
      editor->csi_args[editor->csi_num] = editor->csi_args[editor->csi_num] * 10 + (c - '0');
      return true;
    }
    if (c == ';') {
      editor->csi_num++;
      if (editor->csi_num >= 8) editor->mode = NORMAL;
      else editor->csi_args[editor->csi_num] = 0;
      return true;
    }
    if (c >= '@' && c <= '~') {
      editor->mode = NORMAL;
      switch (c) {
      case 'H': // Home
        if (editor->current.x == 0) return true;
        return moveLeft(editor, editor->current.x);
      case 'F': // End
        if (editor->current.x == editor->current.length) return true;
        return moveRight(editor, editor->current.length - editor->current.x);
      case 'A': case 'B': // Up or down
        goto swap;
      case 'D': // Left
        // Alt+Left or Control+Left is word-left
        if (editor->csi_num == 1 && (editor->csi_args[1] == 3 || editor->csi_args[1] == 5)) {
          int i = editor->current.x;
          while (i > 0 && editor->current.line[--i - 1] != 0x20);
          return moveLeft(editor, editor->current.x - i);
        }
        // Otherwise do plain left
        if (editor->current.x > 0) {
          return moveLeft(editor, 1);
        }
        return true;
      case 'C': // Right
        // Alt+Right or Control+Right is word-right
        if (editor->csi_num == 1 && (editor->csi_args[1] == 3 || editor->csi_args[1] == 5)) {
          int i = editor->current.x;
          while (i < editor->current.length && editor->current.line[++i] != 0x20);
          return moveRight(editor, i - editor->current.x);
        }
        // Otherwise to plain right
        if (editor->current.x < editor->current.length) {
          return moveRight(editor, 1);
        }
        return true;

      // Handle delete
      case '~':
        if (editor->current.length > editor->current.x) {
          goto delete;
        }
        return true;

      default: {
        writeChar('\n');
        writeInt(c);
        writeChar(':');
        for (int i = 0; i <= editor->csi_num; i++) {
          writeChar(' ');
          writeInt(editor->csi_args[i]);
        }
        writeChar('\n');
        writeFlush();
        break;
      }
      }
    }
  }
  return false;

  insert: {
    int i = editor->current.length;
    int t = editor->current.length + 1;
    if (i > MAX_LINE_LENGTH) {
      i = MAX_LINE_LENGTH;
    }
    while (i > editor->current.x) {
      editor->current.line[i] = editor->current.line[i - 1];
      i--;
    }
    editor->current.line[editor->current.x] = (uint8_t)c;
    if (editor->current.x < MAX_LINE_LENGTH - 1) {
      editor->current.x++;
    }
    if (editor->current.length < MAX_LINE_LENGTH) {
      editor->current.length++;
    }
    if (editor->current.x == t) {
      write(1, &c, 1);
      return true;
    }

  }
  goto refresh;

  swap: {
    line_t temp = editor->current;
    editor->current = editor->memory;
    editor->memory = temp;
  }
  goto refresh;

  delete: {
    int i = editor->current.x;
    while (i < editor->current.length) {
      editor->current.line[i] = editor->current.line[i + 1];
      i++;
    }
    editor->current.length--;
  }
  goto refresh;

  refresh: {
    writeCString("\r\33[K");
    writeCString(editor->prompt);
    writeString((char*)editor->current.line, (size_t)editor->current.length);
    if (editor->current.x < editor->current.length) {
      writeCString("\33[");
      writeInt(editor->current.length - editor->current.x);
      writeChar('D');
    }
    writeFlush();
    return true;
  }

  }

int main() {
  editor_t* editor = calloc(1, sizeof(editor_t));
  editor->prompt = "> ";

  struct termios old_tio, new_tio;
	tcgetattr(STDIN_FILENO, &old_tio);
	new_tio = old_tio;
	/* disable canonical mode (buffered i/o) and local echo */
	new_tio.c_lflag &= (unsigned)(~ICANON & ~ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

  char c;
  handleChar(editor, 0);
  do {
    read(0, &c, 1);
  } while (handleChar(editor, c));

	/* restore the former settings */
	tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
  return 0;
}

#ifndef LINE_READER_C
#define LINE_READER_C
#include <termios.h>
#include <stdlib.h>
#include <signal.h>
#include "write-buffer.c"

typedef struct line_s {
  uint8_t x;
  uint8_t length;
  uint8_t line[MAX_LINE_LENGTH];
} line_t;

typedef const char* (*read_fn)(uint8_t *data, uint8_t len);

typedef struct editor_s {
  const char* prompt;
  read_fn onLine;
  enum mode_e {
    NORMAL,
    ESC,
    CSI
  } mode:4;
  uint8_t csi_num:4;
  uint8_t csi_args[3];
  line_t current;
  line_t memory;
} editor_t;

static editor_t editor;
static struct termios old_tio, new_tio;

static bool moveLeft(uint8_t n) {
  if (n == 0) return true;
  editor.current.x -= n;
  writeCString("\33[");
  writeInt(n);
  writeChar('D');
  writeFlush();
  return true;
}

static bool moveRight(uint8_t n) {
  if (n == 0) return true;
  editor.current.x += n;
  writeCString("\33[");
  writeInt(n);
  writeChar('C');
  writeFlush();
  return true;
}

static bool handleChar(char c) {
  if (c == 0) { goto refresh; }
  switch (editor.mode) {
  case NORMAL:
    // Insert printable characters into line
    if (c >= 0x20 && c < 0x7f) {
      goto insert;
    }

    // Handle the start of a CSI sequence
    if (c == 27) {
      editor.mode = ESC;
      return true;
    }

    // Handle Control+D
    if (c == 4) {
      // If there is data, clear it.
      if (editor.current.length) {
        editor.current.length = 0;
        editor.current.x = 0;
        goto refresh;
      }
      // Otherwise, exit the console.
      return false;
    }


    // Handle backspace
    if (c == 127) {
      if (editor.current.x > 0) {
        moveLeft(1);
        goto delete;
      }
      return true;
    }

    // Handle Enter
    if (c == 10) {
      writeCString("\r\n");
      writeFlush();
      if (editor.current.length) {
        const char* newPrompt = editor.onLine(editor.current.line, editor.current.length);
        if (newPrompt) {
          editor.prompt = newPrompt;
        }
      }
      editor.memory.x = 0;
      editor.memory.length = 0;
      goto swap;
    }
    if (c == 12) { // Control+L clear screen
      write(1, "\33[2J\33[H", 7);
      goto refresh;
    }
    // Uncomment to see unhandled codes
    // writeCString("\ncode ");
    // writeInt(c);
    // writeChar('\n');
    // writeFlush();
    return true;
  case ESC:
    if (c == '[') {
      editor.csi_num = 0;
      editor.csi_args[0] = 0;
      editor.mode = CSI;
    }
    else {
      editor.mode = NORMAL;
    }
    return true;
  case CSI:
    if (c >= '0' && c <= '9') {
      editor.csi_args[editor.csi_num] = (uint8_t)(editor.csi_args[editor.csi_num] * 10 + (c - '0'));
      return true;
    }
    if (c == ';') {
      editor.csi_num++;
      if (editor.csi_num >= 8) editor.mode = NORMAL;
      else editor.csi_args[editor.csi_num] = 0;
      return true;
    }
    if (c >= '@' && c <= '~') {
      editor.mode = NORMAL;
      switch (c) {
      case 'H': // Home
        if (editor.current.x == 0) return true;
        return moveLeft(editor.current.x);
      case 'F': // End
        if (editor.current.x == editor.current.length) return true;
        return moveRight(editor.current.length - editor.current.x);
      case 'A': case 'B': // Up or down
        goto swap;
      case 'D': // Left
        // Alt+Left or Control+Left is word-left
        if (editor.csi_num == 1 && (editor.csi_args[1] == 3 || editor.csi_args[1] == 5)) {
          uint8_t i = editor.current.x;
          while (i > 0 && editor.current.line[--i - 1] != 0x20);
          return moveLeft(editor.current.x - i);
        }
        // Otherwise do plain left
        if (editor.current.x > 0) {
          return moveLeft(1);
        }
        return true;
      case 'C': // Right
        // Alt+Right or Control+Right is word-right
        if (editor.csi_num == 1 && (editor.csi_args[1] == 3 || editor.csi_args[1] == 5)) {
          uint8_t i = editor.current.x;
          while (i < editor.current.length && editor.current.line[++i] != 0x20);
          return moveRight(i - editor.current.x);
        }
        // Otherwise to plain right
        if (editor.current.x < editor.current.length) {
          return moveRight(1);
        }
        return true;

      // Handle delete
      case '~':
        if (editor.current.length > editor.current.x) {
          goto delete;
        }
        return true;

      default:
        writeChar('\n');
        writeInt(c);
        writeChar(':');
        for (uint8_t i = 0; i <= editor.csi_num; i++) {
          writeChar(' ');
          writeInt(editor.csi_args[i]);
        }
        writeChar('\n');
        writeFlush();
        break;
      }
    }
  }
  return false;

  insert: {
    uint8_t i = editor.current.length;
    uint8_t t = editor.current.length + 1;
    if (i > MAX_LINE_LENGTH) {
      i = MAX_LINE_LENGTH;
    }
    while (i > editor.current.x) {
      editor.current.line[i] = editor.current.line[i - 1];
      i--;
    }
    editor.current.line[editor.current.x] = (uint8_t)c;
    if (editor.current.x < MAX_LINE_LENGTH - 1) {
      editor.current.x++;
    }
    if (editor.current.length < MAX_LINE_LENGTH) {
      editor.current.length++;
    }
    if (editor.current.x == t) {
      write(1, &c, 1);
      return true;
    }

  }
  goto refresh;

  swap: {
    line_t temp = editor.current;
    editor.current = editor.memory;
    editor.memory = temp;
  }
  goto refresh;

  delete: {
    uint8_t i = editor.current.x;
    while (i < editor.current.length) {
      editor.current.line[i] = editor.current.line[i + 1];
      i++;
    }
    editor.current.length--;
  }
  goto refresh;

  refresh:
    writeCString("\r\33[K");
    writeCString((const char*)editor.prompt);
    writeString(editor.current.line, editor.current.length);
    if (editor.current.x < editor.current.length) {
      writeCString("\33[");
      writeInt((editor.current.length - editor.current.x));
      writeChar('D');
    }
    writeFlush();
    return true;
}

void stopEditor() {
  write(1, "\n", 1);
  /* restore the former settings */
	tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

}

static void onInt(int sig) {
  if (sig == SIGINT) {
    write(1, "^C", 2);
    stopEditor();
    exit(0);
  }
}

void startEditor(const char* prompt, read_fn onLine) {
  editor.prompt = prompt;
  editor.onLine = onLine;
	tcgetattr(STDIN_FILENO, &old_tio);
	new_tio = old_tio;
	/* disable canonical mode (buffered i/o) and local echo */
	new_tio.c_lflag &= (unsigned)(~ICANON & ~ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
  signal(SIGINT, onInt);
  handleChar(0);
}

bool stepEditor() {
  char c;
  read(0, &c, 1);
  return handleChar(c);
}

#endif

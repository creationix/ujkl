#ifndef LINE_READER_C
#define LINE_READER_C
#include <termios.h>
#include <stdlib.h>
#include <signal.h>
#include "write-buffer.c"

typedef struct line_s {
  int x;
  int length;
  char line[MAX_LINE_LENGTH];
} line_t;

typedef void (*read_fn)(const char *data);

static const char* prompt;
static read_fn onLine;
static enum mode_e {
  NORMAL,
  ESC,
  CSI
} mode;
static int csi_num;
static char csi_args[3];
static line_t current, memory;
static struct termios old_tio, new_tio;

static bool moveLeft(int n) {
  if (n == 0) return true;
  current.x -= n;
  writeCString("\33[");
  writeInt(n);
  writeChar('D');
  writeFlush();
  return true;
}

static bool moveRight(int n) {
  if (n == 0) return true;
  current.x += n;
  writeCString("\33[");
  writeInt(n);
  writeChar('C');
  writeFlush();
  return true;
}

static bool handleChar(char c) {
  if (c == 0) { goto refresh; }
  switch (mode) {
  case NORMAL:
    // Insert printable characters into line
    if (c >= 0x20 && c < 0x7f) {
      goto insert;
    }

    // Handle the start of a CSI sequence
    if (c == 27) {
      mode = ESC;
      return true;
    }

    // Handle Control+D
    if (c == 4) {
      // If there is data, clear it.
      if (current.length) {
        current.length = 0;
        current.x = 0;
        goto refresh;
      }
      // Otherwise, exit the console.
      return false;
    }


    // Handle backspace
    if (c == 127) {
      if (current.x > 0) {
        moveLeft(1);
        goto delete;
      }
      return true;
    }

    // Handle Enter
    if (c == 10) {
      writeCString("\r\n");
      writeFlush();
      if (current.length) {
        current.line[current.length] = 0;
        onLine(current.line);
      }
      memory.x = 0;
      memory.length = 0;
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
      csi_num = 0;
      csi_args[0] = 0;
      mode = CSI;
    }
    else {
      mode = NORMAL;
    }
    return true;
  case CSI:
    if (c >= '0' && c <= '9') {
      csi_args[csi_num] = (csi_args[csi_num] * 10 + (c - '0'));
      return true;
    }
    if (c == ';') {
      csi_num++;
      if (csi_num >= 8) mode = NORMAL;
      else csi_args[csi_num] = 0;
      return true;
    }
    if (c >= '@' && c <= '~') {
      mode = NORMAL;
      switch (c) {
      case 'H': // Home
        if (current.x == 0) return true;
        return moveLeft(current.x);
      case 'F': // End
        if (current.x == current.length) return true;
        return moveRight(current.length - current.x);
      case 'A': case 'B': // Up or down
        goto swap;
      case 'D': // Left
        // Alt+Left or Control+Left is word-left
        if (csi_num == 1 && (csi_args[1] == 3 || csi_args[1] == 5)) {
          int i = current.x;
          while (i > 0 && current.line[--i - 1] != 0x20);
          return moveLeft(current.x - i);
        }
        // Otherwise do plain left
        if (current.x > 0) {
          return moveLeft(1);
        }
        return true;
      case 'C': // Right
        // Alt+Right or Control+Right is word-right
        if (csi_num == 1 && (csi_args[1] == 3 || csi_args[1] == 5)) {
          int i = current.x;
          while (i < current.length && current.line[++i] != 0x20);
          return moveRight(i - current.x);
        }
        // Otherwise to plain right
        if (current.x < current.length) {
          return moveRight(1);
        }
        return true;

      // Handle delete
      case '~':
        if (current.length > current.x) {
          goto delete;
        }
        return true;

      default:
        writeChar('\n');
        writeInt(c);
        writeChar(':');
        for (int i = 0; i <= csi_num; i++) {
          writeChar(' ');
          writeInt(csi_args[i]);
        }
        writeChar('\n');
        writeFlush();
        break;
      }
    }
  }
  return false;

  insert: {
    int i = current.length;
    int t = current.length + 1;
    if (i > MAX_LINE_LENGTH) {
      i = MAX_LINE_LENGTH;
    }
    while (i > current.x) {
      current.line[i] = current.line[i - 1];
      i--;
    }
    current.line[current.x] = c;
    if (current.x < MAX_LINE_LENGTH - 1) {
      current.x++;
    }
    if (current.length < MAX_LINE_LENGTH) {
      current.length++;
    }
    if (current.x == t) {
      write(1, &c, 1);
      return true;
    }

  }
  goto refresh;

  swap: {
    line_t temp = current;
    current = memory;
    memory = temp;
  }
  goto refresh;

  delete: {
    int i = current.x;
    while (i < current.length) {
      current.line[i] = current.line[i + 1];
      i++;
    }
    current.length--;
  }
  goto refresh;

  refresh:
    writeCString("\r\33[K");
    writeCString((const char*)prompt);
    writeString(current.line, current.length);
    if (current.x < current.length) {
      writeCString("\33[");
      writeInt((current.length - current.x));
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

void startEditor() {
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

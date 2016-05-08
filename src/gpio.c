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

#ifdef MAX_PINS
  {"on", _on},
  {"off", _off},
#endif

// 1 2 +
// INT:1 INT:2 SYM:+

/*
0mxxxxxx mxxxxxx... integer literal
00xxxxxx

if expr... then - one time
  code...
elseif expr... then - zero or more times
  code...
else - zero or one times
  code...
end

while expr... do
  code...
  break
  continue
end

*/



typedef enum {
  // Basic arithmetic
  ADD, // (value value -- value) (+)
  SUB, // (value value -- value) (-)
  MUL, // (value value -- value) (*)
  DIV, // (value value -- value) (/)
  MOD, // (value value -- value) (%)
  NEG, // (value -- value)
  // Comparisons
  GT,  // (value value -- value) (>)
  GTE, // (value value -- value) (>=)
  EQ,  // (value value -- value) (=)
  NEQ, // (value value -- value) (!=)
  // Boolean logic
  AND, // (value value -- value) (&&)
  OR,  // (value value -- value) (||)
  XOR, // (value value -- value) (^^)
  NOT, // (value -- value)
  // Local variables
  SET, // (value index --) set slot, written as set-X where X is variable name.
  GET, // (index -- value) get slot, written as X where X is variable name.
  // Stack manipulation
  DRP, // (a --)
  NIP, // (a b -- b)
  DUP, // (a -- a a)
  OVR, // (a b -- a b a)
  TCK, // (a b -- b a b)
  SWP, // (a b -- b a)
  ROT, // (a b c -- b c a)
  REW, // (a b c -- c a b)
  // Conditional logic, generated from structured code in AST
  JMP, // (offset --) unconditional jump
  IF,  // (cond offset --) conditional jump
  // Buffer management
  ESC, // (num -- buffer) consume num bytes and push as buffer
  NEW, // (num -- buffer) alloce num bytes of zero and push as buffer
  LEN, // (buffer -- length) get length of buffer
  IDX, // (buffer offset -- byte) read byte from buffer
  UPD, // (buffer offset byte --) Write to buffer at offset
  // Function calling
  DEF, // (buffer index --) store code (buffer) at index
  CAL, // (index) jump to code at index and return when done
  // Library functions
  PIN_MODE,  // (pin mode --)
  DIGITAL_WRITE,  // (pin value --)
  DIGITAL_READ,  // (pin -- value)
  TONE, // (pin freq dur --) play square wave on pin with frequency and duration
  NEOPIXEL, // (pin data --) neopixel write
  I2C_INIT, // (scl sda --) Initialize I2C bus
  I2C_WRITE, // (byte --) Write byte to I2C Bus
  I2C_READ, // (-- byte) Read byte from I2C Bus
} opcode_t;

int main() {
}

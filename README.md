# UJKL

UJKL (will probably be renamed) is a small lisp interpreter aimed at IoT devices.
My main target will be raspberry PI computers and esp8266 chips.  The purpose is
to enable children to design and program machines in the real world.

While this is a lisp in syntax and semantics, it is not common lisp or scheme,
many conventions here are my own.

Code is data and data is code.  This means that a function is nothing more than
a data list with special structure.

**NOTICE** The code is not yet complete.  It currently has a working repl and
some of the documented APIs here, but I'm doing readme-driven-development and
designing the language/apis before implementing them.  Feel free to comment on
the design or peruse the code patterns in general, but don't be surprised if
the code doesn't match the docs (or doesn't compile at all).

## Value Types

There are very few primitives types in the runtime.  They are:

- Integers - signed integers with 29 bits of capacity (-268435456 to 268435455)
- True, False - useful for boolean operations
- Nil - Represents the empty list usually
- Symbol - An interned, immutable string.  Used for variable names and property
  names mostly.
- Native - Also a symbol, but instead of resolving to something in the local
  environment, this points to a native C function.
- Pair - This is the only data structure in the VM.  It is a container that can
  reference exactly two values.
- TypeError, RangeError, Undefined, etc... - result of a bad operation.

## Value Conventions

- List - A list is a chain of pairs with value on left and pointer to next node
  on right.
- Table - A table is a list where every value is a cons pair containing key and
  value.  This is how local variables in the environments are implemented.
- Function - A function is a list where the first value is a list of symbols
  for the parameters, everything after that is the body.

## Keywords:

These take the environment table as well as unevaluated arguments.  This means
they can alter the environment or choose to not evaluate certain arguments.

### Local Variables / Environment

Here `key` can be a list of keys or a single key. If it's a list, each value
will resolve using the result of the previous as the new table.

- (get key) -> value
- (has key ...) -> bool
- (del key ...)
- (set key value ...)
- (def key (params...) body...) - define a function

### Control Flow

- (if condition block...)
- (else-if condition block...)
- (else block...)
- (while condition block...)
- (do block...)

## Functions

Functions simply take a list of arguments (pre-evaluated) and return a value.

A function can be a builtin or user-defined structure.  A user-defined function
is an parameters list followed by the body.

- (fn args...) - Call a function with args
- (apply fn args...) - same thing, but exposing apply.

### Cons Pair Operations

- (car pair) -> value - return left side of pair
- (cdr pair) -> value - return right side of pair
- (set-car! pair value) -> bool - modify pair in place
- (set-cdr! pair value) -> bool - modify pair in place

The following equivalents are in the C interface.

```c
value_t car(value_t pair);
value_t cdr(value_t pair);
bool set_car(value_t pair, value_t val);
bool set_cdr(value_t pair, value_t val);
```

### List Operations

A list is a singly-linked list of cons pairs where the data is on the left side
and a reference to the next node is on the right.

- (list? list) -> boolean - each node is either a nil or a pair pointing to a node
- (length? list) -> number - length of list
- (reverse list) -> list
- (sort list) -> list - sort with default sorting
- (sort list ((l r)...)->rank) -> list - sort a list with optional sort function

- (concat list list...) -> combined-list
- (append! list value...) -> list - add some values to end of list.

- (iget list index) -> value
- (iset! list index value) -> list

- (has? set value) -> boolean
- (add! set value) -> list
- (remove! set value) -> list

In the C interface for functions these are the equivalents

```c
bool is_list(value_t val);
int list_length(value_t list);
value_t list_reverse(value_t list);
value_t list_sort(value_t list);
value_t list_custom_sort(value_t list, value_t context, api_fn, sorter);
value_t list_append(value_t list, value_t tail);
value_t list_get(value_t set, int index);
value_t list_set(value_t set, int index, value_t value);
bool list_has(value_t list, value_t val);
value_t list_add(value_t list, value_t val);
value_t list_remove(value_t list, value_t val);
```

### Table Operations

- in all these `key` can be a list of keys or a key

- (table? tab) -> boolean - a list of pairs
- (t-get tab key) -> value
- (t-has tab key ...) -> bool
- (t-del! tab key ...) -> tab
- (t-set! tab key value ...) -> tab

The following C functions are also available.

```c
bool is_table(value_t val);
value_t table_get(value_t tab, value_t key);
value_t table_aget(value_t tab, value_t keys);
bool table_has(value_t tab, value_t key);
bool table_ahas(value_t tab, value_t keys);
value_t table_del(value_t tab, value_t key);
value_t table_adel(value_t tab, value_t keys);
value_t table_set(value_t tab, value_t key, value);
```

### Iterators

- If iter is list, loop through each item
- If iter is positive number, loop from 0 to number -1
- if iter is negative number, loop from -number - 1 to 0

- (each iter ((item)...)->value) -> last-value
- (map iter ((item)...)->value) -> list
- (filter iter ((item)...)->boolean) -> list
- (filter-map iter ((item)...)->boolean) -> list

```c
value_t list_each(value_t list, value_t context, api_fn block);
value_t list_map(value_t list, value_t context, api_fn block);
value_t list_filter(value_t list, value_t context, api_fn block);
value_t list_filter_map(value_t list, value_t context, api_fn block);
```


### Integer Math

- (+ number...) -> number - Add numbers
- (* number...) -> number - multiply numbers
- (- a b) -> number - subtract
- (- a) -> number - negate
- (/ num dem) -> number - divide
- (% num mod) -> number - modulus
- (rand) -> random integer using full precision
- (rand mod) -> random integer from 0 to mod - 1
- (rand start stop) -> random integer from start to stop inclusive

```c
void deadbeef_seed(int seed);
int deadbeef_rand();
```

### Comparison

Values must be of same type, numbers can compare with numbers, symbols compare
alphabetically, pairs compare by contents recursively, left first, then right.

If types don't match, != will false, = will true, others type-error.

- (< a b ...) -> bool - check if values on left are less than values on right
- (> a b ...) -> bool - greater than
- (<= a b ...) -> bool - greater than or equal
- (>= a b ...) -> bool - greater than or equal
- (!= a b) -> bool - check if values are different
- (= a b) -> bool check if values are equal

### Logic

Truthyness is defined as:

- true, numbers, symbols, pairs - all truthy
- false, nil, undefined, etc - all falsy

```c
bool to_bool(value_t val);
```

- (and a b ...) -> value - short circuit evaluation, return last value processed
- (or a b ...) -> value - same props as and
- (xor a b) -> value - returns false if values are both truthy, truthy value otherwise
- (not a) -> bool - negates

### Console/Serial I/O

- (print value...) dump values separated by spaces

### PubSub Mesh Communication

Nodes can live within a network and communicate via pubsub system.

- (net-put name data) - send data to all nodes with name
- (net-get name) -> data - Read data from queue, nil if empty
- (net-when name ((data)...)) -> handle - Setup listener callback
- (net-stop handle) - remove listener

### GPIO

Low level access to digital GPOIs.  This is great for controlling LEDs and
reading buttons.

- (gpio-enable pin mode) - mode can be 'output, 'input, or 'input-pullup.
- (gpio-write pin value) - value is boolean
- (gpio-read pin) -> value - value is boolean
- (gpio-when pin ((value)...)) -> handle - Register a callback for when a pin
  changes value.
- (gpio-stop handle) - remove listener

### I2C

This allows writing drivers for I2C devices in the scripting language!

- (i2c-init scl-pin sda-pin) - initialize pins for i2c
- (i2c-write byte) -> ack - low level write command
- (i2c-read ack) -> byte - low level read command
- (i2c-start) - low level start command
- (i2d-stop) - low level stop command
- (i2c-slave-write addr bytes) -> bool - high level write command
- (i2c-slave-read addr data len) -> bytes - high level read command

### WS2812

If we can get away with it, this allows streaming of neopixel values.

If we are too slow then start and stop will need to be baked into just the list
commands.

- (ws2812-seq-start)
- (ws2812-seq-stop)
- (ws2812-byte pin byte)
- (ws2812-rgb pin color)
- (ws2812-rgbw pin color)
- (ws2812-rgb-list pin colors)
- (ws2812-rgbw-list pin colors)

### Tone

Generate a square wave by flipping a digital pin at a frequency.

- (tone pin freq duration) - Play tone at frequency on pin for duration

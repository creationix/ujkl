# UJKL

UJKL (will probably be renamed) is a small lisp interpreter aimed at IoT devices.
My main target will be raspberry PI computers and esp8266 chips.  The purpose is
to enable children to design and program machines in the real world.

While this is a lisp in syntax and semantics, it is not common lisp or scheme,
many conventions here are my own.

Code is data and data is code.  This means that a function is nothing more than
a data list with special structure.

## Value Types

There are very few primitives types in the runtime.  They are:

- Integers - signed integers with 29 bits of capacity (-268435456-268435455)
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

## Environment variables

Here `key` can be a list of keys or a single key. If it's a list, each value
will resolve using the result of the previous as the new table.

- (get key) -> value
- (has key ...) -> bool
- (del key ...)
- (set key value ...)
- (def key (params...) body...) - define a function

## Cons Pair Operations

- (fst pair) -> value - return left side of pair
- (snd pair) -> value - return right side of pair
- (set-fst! pair value) - modify pair in place
- (set-snd! pair value) - modify pair in place

## List Operations

A list is a singly-linked list of cons pairs where the data is on the left side
and a reference to the next node is on the right.

- (list? list) -> boolean - each node is either a nil or a pair pointing to a node
- (length? list) -> number - length of list
- (reverse list) -> list
- (sort list ((l r)...)->rank) -> list - sort a list with optional sort function

- (concat list list...) -> combined-list
- (append! list value...) -> list - append some values to list.

- (has? set value) -> boolean
- (add! set value) -> list
- (remove! set value) -> list

## Table Operations

- in all these `key` can be a list of keys or a key

- (table? tab) -> boolean - a list of pairs
- (t-get tab key) -> value
- (t-has tab key ...) -> bool
- (t-del! tab key ...) -> obj
- (t-set! tab key value ...) -> obj

## Iterators

- If iter is list, loop through each item
- If iter is positive number, loop from 0 to number -1
- if iter is negative number, loop from -number - 1 to 0

- (each iter ((item)...)->value) -> last-value
- (map iter ((item)...)->value) -> list
- (filter iter ((item)...)->boolean) -> list
- (filter-map iter ((item)...)->boolean) -> list

## Control Flow

- (if condition block...)
- (else-if condition block...)
- (else block...)
- (while condition block...)

## Integer Math

- (+ number...) -> number - Add numbers
- (* number...) -> number - multiply numbers
- (- a b) -> number - subtract
- (- a) -> number - negate
- (/ num dem) -> number - divide
- (% num mod) -> number - modulus
- (rand) -> random integer using full precision
- (rand mod) -> random integer from 0 to mod - 1
- (rand start stop) -> random integer from start to stop inclusive

## Comparison

Values must be of same type, numbers can compare with numbers, symbols compare
alphabetically, pairs compare by contents recursively, left first, then right.

If types don't match, != will false, = will true, others type-error.

- (< a b ...) -> bool - check if values on left are less than values on right
- (> a b ...) -> bool - greater than
- (<= a b ...) -> bool - greater than or equal
- (>= a b ...) -> bool - greater than or equal
- (!= a b) -> bool - check if values are different
- (= a b) -> bool check if values are equal

## Logic

Truthyness is defined as:

- true, numbers, symbols, pairs - all truthy
- false, nil, undefined, etc - all falsy

- (and a b ...) -> value - short circuit evaluation, return last value processed
- (or a b ...) -> value - same props as and
- (xor a b) -> value - returns false if values are both truthy, truthy value otherwise
- (not a) -> bool - negates

## Console/Serial I/O

- (print value...) dump values separated by spaces

## PubSub Mesh Communication

Nodes can live within a network and communicate via pubsub system.

- (net-namespace namespace) - prefix all names with namespace
- (net-send name data) - send data to all nodes with name
- (net-read name) -> data - Read data from queue, nil if empty
- (net-when name ((data)...)) -> handle - Setup listener callback
- (net-stop handle) - remove listener

## GPIO

- (gpio-enable pin mode) - mode can be 'output, 'input, or 'input-pullup.
- (gpio-write pin value) - value is boolean
- (gpio-read pin) -> value - value is boolean
- (tone pin freq duration) - Play tone at frequency on pin for duration
- (gpio-when pin (()))

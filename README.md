
`size_t len(void *ptr);`

Returns the number of elements in a dynamic or an associative array.

* `ptr` -  pointer to the dynamic or associative array

**Returns:** Number of elements in the array.

---

`foreach(type *ref, type *ptr, size_t len = len(ptr))`

Iterate over an array.

* `ref` -  array iterator, not necessary to declare before
* `ptr` -  pointer to an array
* `len` -  number of elements to iterate (default is `len(ptr)`)


---

`type *reserve(type **pptr, size len, bool map = false);`

Pre-allocates memory for an array.

* `pptr` -  pointer to the dynamic or associative array,
may be any type
* `len` -  pre-allocated items count
* `map` -  if true preallocate memory for an associative array

**Returns:** Pointer to the pre-allocated array.

---

`ssize_t append(type **pptr, type init);`

Adds an element to the end of a dynamic array, expands memory
usage if necessary.

* `pptr` -  pointer to the dynamic array, may be any type
* `init` -  initializer for a new array element, may be an aggregate
initializer list

**Returns:** Index in the array where the new value is appended or `-1`
if something went wrong, the index is valid until any method on the
dynamic array is called.

---

`mapof(ktype, vtype)`

Associative array element type.

* `ktype` -  associative array index (key) type, can be any non-pointer
type except a pointer to a null terminated string
* `vtype` -  a type of value associated with the key, can be any type

To pass associative array pointers to functions, the associative array type
must be fully qualified using the `typedef` keyword.

---

`ssize_t insert(mapof(ktype, vtype) **pptr, ktype key, vtype value);`

Adds an element to a dynamic associative array, expands memory
usage if necessary.

If an element with the same key exists, a duplicate element will be
added, to prevent this, the `lookup` method should be used.

* `pptr` -  pointer to the associative array, may be declared using
`mapof` macro
* `key` -  associative array index value, maybe any standard type
* `init` -  initializer for a new data element, may be an aggregate
initializer list

**Returns:** Index in the array where the new value is inserted or `-1`
if something went wrong, the index is valid until any method on the
associative array is called.

---

`ssize_t delete(mapof(ktype, vtype) **pptr, vtype *ref);`

Removes an element from an associative array.

* `pptr` -  pointer to the associative array
* `ref` -  reference to a data value associated with a key
in the array, can be returned by `lookup()` method

**Returns:** Index in the array that `ref` belonged to,
the index is valid until any associative array method is called.

---

`vtype *lookup(mapof(ktype, vtype) **pptr, ktype key);`

Searches a data associated with a key.

* `pptr` -  pointer to the associative array
* `key` -  associative array key value

**Returns:** Reference to the data that the `key` is associated with,
the reference is valid until any associative array method is called,
if the key doesn't exist NULL pointer will be returned.

---

`int fprintln(FILE *fp, ...);`

Prints a line to a stream.

* `fp` -  output stream
* `` - ... list of values or constants of standard type to print

**Returns:** The number of bytes printed.

---

`int println(...);`

Prints a line to the standard output stream.

* `` - ... list of values or constants of standard type to print

**Returns:** The number of bytes printed.

---

`int printv(const char *sep, type *ptr, size_t len = len(ptr));`

Print an array to the standard output stream.

* `sep` -  separator between elements of the output array
* `ptr` -  array of values or constants of standard type to print
* `len` -  number of elements to output (default is `len()`)

**Returns:** The number of bytes printed.

---

`char *join(...);`

Concatenates an list of values into a single string.

* `` - ... list of values or constants of standard type to join

**Returns:** The pointer to joined string, should be released by calling `free()`.

---

`char *joinv(const char *sep, type *ptr, size_t len = len(ptr));`

Concatenates an array into a single string.

* `sep` -  substring between the joined elements
* `ptr` -  array of values or constants of standard type to join
* `len` -  number of elements to join (default is `len(ptr)`)

**Returns:** The pointer to joined string, should be released by calling `free()`.

---

`void split(const char *str, const char *sep, ...);`

Splits a string into tokens and assigns the token values
to the specified list of variables.

* `str` -  the string to be parsed
* `sep` -  substring delimits the tokens in the parsed string
* `` - ... list of pointers to variables to assign token values to

Tokens will be converted to the target type before assignment.
For pointers to a string, the necessary amount of memory will be
allocated to store the token. Such memory should be released by
calling `free()`.

---

`void splitv(const char *str, const char *sep, type **pptr);`

Splits a string into tokens and adds the token values
to a dynamic array.

* `str` -  string to be parsed
* `sep` -  substring separates tokens in the parsed string
* `pptr` -  pointer to a list to assign token values to

Tokens will be converted to the target type before assignment.

---

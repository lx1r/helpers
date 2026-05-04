---
## Generic helpers

The library provides C generic helpers for:
* Dynamically growable arrays of any type
* Associative arrays holding unique keys associated with specific values
* Outputing a list of variables of any built-in type to a file
* Converting a list of variables of any built-in type to a string
* Tokenizing string into a list of variables of any built-in type

---
`_ptr`

An automatic pointer that invokes `free()` when leaving the scope.

---
`_vptr`

An automatic pointer to an array that invokes `vfree()` when leaving the scope.

---
`size_t len(type *ptr);`

Returns the number of elements in a static, a variable-length or
dynamically growable array. For associative array the function returns
the number of elements of the underlying dynamic array.

* `ptr` - pointer to the dynamic or associative array

**Returns:** Number of elements in the array.

---
`foreach (type *ref, type *ptr, size_t len = len(ptr))`

Iterate over a static, a variable-length, a dynamic or
an associative array.

* `ref` - array iterator name, not necessary to declare before
* `ptr` - pointer to an array
* `len` - number of elements to iterate, default is `len(ptr)`


---
`boot resize(type **pptr, size cap);`

Changes the capacity of a dynamic array.

* `pptr` - pointer to the dynamic array, may be any type
* `cap` - requested capacity

If the array length is less than the requested capacity,
the length will be truncated.

**Returns:** On success, `true` is returned. If the requested
capacity is not enought `false` is returned and the original
dynamic array is left untouched.

---
`ssize_t append(type **pptr, type init);`

Adds an element to the end of a dynamic array, expands memory
usage if necessary.

* `pptr` - pointer to the dynamic array, may be any type
* `init` - initializer for a new array element, may be an aggregate initializer list

**Returns:** Index in the array where the new value is appended or `-1`
if something went wrong, the index is valid until any method on the
dynamic array is called.

---
`void vfree(type **ptr);`

Releases allocated memory for each element of a dynamic array.

* `ptr` - pointer to the dynamic array, may be any type

---
`entry(ktype, vtype)`

Associative array element type.

* `ktype` - associative array index type
* `vtype` - a type of value associated with the key

The array index can be any built-in scalar type, a structure,
or a pointer to a null value. The array value can be any type.

To pass associative array pointers to functions, the associative array
type must be fully qualified using the `typedef` keyword.

---
`bool rehash(entry(ktype, vtype) **pptr, size cap = 64);`

Changes the capacity of an associative array.

* `pptr` - pointer to the associative array
* `cap` - requested capacity

**Returns:** On success, `true` is returned. If the requested
capacity is not enought `false` is returned and the original
associative array is left untouched.

---
`vtype *insert(entry(ktype, vtype) **pptr, ktype key, vtype init);`

Adds a new element to a dynamic associative array only if it
did not exist, exapands memory usage if necessary.

* `pptr` - pointer to the associative array, may be declared by `entry` macro
* `key` - associative array index value
* `init` - array value initializer, may be an aggregate initializer list

**Returns:** Reference to the inserted data in the associative array or
NULL if something went wrong. The reference is valid until any method
on the associative array is called.

---
`vtype *update(entry(ktype, vtype) **pptr, ktype key, vtype init);`

Adds a new element or update an existing element to a dynamic
associative array, exapands memory usage if necessary.

* `pptr` - pointer to the associative array, may be declared by `entry` macro
* `key` - associative array index value
* `init` - array value initializer, may be an aggregate initializer list

**Returns:** Reference to the updated data in the associative array or
NULL if something went wrong. The reference is valid until any method
on the associative array is called.

---
`bool delete(entry(ktype, vtype) **pptr, vtype *ref);`

Removes an element from an associative array.

* `pptr` - pointer to the associative array
* `ref` - reference to a value associated with a key

Value reference can be returned by `lookup()` method.

**Returns:** On success, `true` is returned. If `ref` is invalid
`false` is returned.

---
`vtype *lookup(entry(ktype, vtype) **pptr, ktype key);`

Searches a data associated with a key.

* `pptr` - pointer to the associative array
* `key` - associative array key value

**Returns:** Reference to the data that the `key` is associated with,
the reference is valid until any associative array method is called,
if the key doesn't exist NULL pointer will be returned.

---
`int fprint(FILE *fp, ...);`

Prints a list of values into a stream.

* `fp` - output stream
* `...` - list of values or constants of standard type to print

**Returns:** The number of characters printed.

---
`int print(...);`

Prints a list of values into the standard output stream.

* `...` - list of values or constants of standard type to print

**Returns:** The number of characters printed.

---
`int fprintln(FILE *fp, ...);`

Prints a line to a stream.

* `fp` - output stream
* `...` - list of values or constants of standard type to print

**Returns:** The number of characters printed.

---
`int println(...);`

Prints a line to the standard output stream.

* `...` - list of values or constants of standard type to print

**Returns:** The number of characters printed.

---
`int fprintv(FILE *fp, const char *sep, type *ptr, size_t len = len(ptr));`

Print an array to a stream.

* `fp` - output stream
* `sep` - separator between elements of the output array
* `ptr` - array of values or constants of standard type to print
* `len` - number of elements to output, default is `len()`

**Returns:** The number of characters printed.

---
`int printv(const char *sep, type *ptr, size_t len = len(ptr));`

Print an array to the standard output stream.

* `sep` - separator between elements of the output array
* `ptr` - array of values or constants of standard type to print
* `len` - number of elements to output, default is `len()`

**Returns:** The number of characters printed.

---
`char *join(...);`

Concatenates a list of values into a single string.

* `...` - list of values or constants of standard type to join

**Returns:** The pointer to joined string, should be released by calling `free()`.

---
`char *joinv(const char *sep, type *ptr, size_t len = len(ptr));`

Concatenates an array into a single string.

* `sep` - substring between the joined elements
* `ptr` - array of values or constants of standard type to join
* `len` - number of elements to join, default is `len(ptr)`

**Returns:** The pointer to joined string, should be released by calling `free()`.

---
`void split(const char *str, const char *sep, ...);`

Splits a string into tokens and assigns the token values
to the specified list of variables.

* `str` - the string to be parsed
* `sep` - substring delimits the tokens in the parsed string
* `...` - list of pointers to variables to assign token values to

Tokens will be converted to the target type before assignment.
For pointers to a string, the necessary amount of memory will be
allocated to store the token. Such memory should be released by
calling `free()`.

---
`void splitv(const char *str, const char *sep, type **pptr);`

Splits a string into tokens and adds the token values
to a dynamic array.

* `str` - string to be parsed
* `sep` - substring separates tokens in the parsed string
* `pptr` - pointer to a list to assign token values to

Tokens will be converted to the target type before assignment.


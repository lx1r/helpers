#ifndef ___HELPERS_H
#define ___HELPERS_H

/*
 * ## Generic helpers
 *
 * The library provides C generic helpers for
 *
 * * [Dynamically growable arrays of arbitrary type](#dynamic-arrays)
 * * [Associative arrays with generic keys and values](#associative-arrays)
 * * [Printing a list of built-in type variables to a file](#output-helpers)
 * * [Converting a list of built-in type variables to a string](#string-conversion)
 * * [Tokenizing a string into a list of variables of built-in types](#string-tokenization)
 */

#ifndef ___concat
#define ___concat(a, b) a ## b
#endif
#ifndef ___apply
#define ___apply(fn, n) ___concat(fn, n)
#endif
#ifndef ___nth
#define ___nth(_, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, N, ...) N
#endif
#ifndef ___narg
#define ___narg(...)\
	___nth(_, ##__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>

/*
 * ### Pointers with scope lifetime
 */

/**
 * @type type ptr(var);
 *
 * @brief An automatic pointer that invokes `free()` when leaving the scope.
 *
 * @param type pointer type
 * @param var variable name
 */
#define ptr(var) *__attribute__((__cleanup__(___pfree))) var

/**
 * @type pptr(var);
 *
 * @brief An automatic pointer to an array of pointers that invokes `vfree()` when leaving the scope.
 *
 * @param type pointer type
 * @param var variable name
 */
#define pptr(var) **__attribute__((__cleanup__(___pvfree))) var

static inline void ___pfree(void *ptr)
{
	void **pptr = (void **)ptr;

	free(*pptr);
	*pptr = NULL;
}

#define ___cap_sz(ptr) malloc_usable_size(ptr) /* assume sizeof(size_t) alignment */

struct ___meta {
	size_t len:__SIZE_WIDTH__ - 1;
	size_t ext:1;
};

static inline struct ___meta *___meta(void *ptr)
{
	return ptr + ___cap_sz(ptr) - sizeof(struct ___meta);
}

#define ___inuse_sz(len) (((((len) - 1) / __LONG_WIDTH__) + 1) * __SIZEOF_LONG__)

static inline unsigned long *___inuse_bits(void *ptr)
{
	struct ___meta *meta = ___meta(ptr);
	if (!meta->ext)
		return NULL;
	return (void *)meta - ___inuse_sz(meta->len);
}

#define ___inuse_test(ptr, slot) \
	(___inuse_bits(ptr)[(slot) / __LONG_WIDTH__] & (1UL << ((slot) % __LONG_WIDTH__)))

#define ___inuse_set(ptr, slot) \
	(___inuse_bits(ptr)[(slot) / __LONG_WIDTH__] |= (1UL << ((slot) % __LONG_WIDTH__)))

#define ___inuse_clear(ptr, slot) \
	(___inuse_bits(ptr)[(slot) / __LONG_WIDTH__] &= ~(1UL << ((slot) % __LONG_WIDTH__)))

#define ___inuse(ptr, slot) \
	_Generic(&(ptr),\
		 typeof(*(ptr)) **: ___inuse_dynamic(ptr, slot),\
		 default: true)

static inline bool ___inuse_dynamic(void *ptr, ssize_t slot)
{
	struct ___meta *meta = ___meta(ptr);

	return !meta->ext || ___inuse_test(ptr, slot);
}

/*
 * ### Array helpers
 */

/**
 * @fn size_t len(type *ptr);
 *
 * @brief Returns the number of elements in a static, a variable-length or
 * dynamically growable array. For associative array the function returns
 * the number of elements of the underlying dynamic array.
 *
 * @param ptr pointer to the dynamic or associative array
 *
 * @return Number of elements in the array.
 */
#define len(ptr) \
	_Generic(&(ptr),\
		 typeof(*(ptr)) **: ___dynamic_len,\
		 default: ___static_len)(ptr, sizeof((ptr)[0]), sizeof(ptr))

static inline __attribute__((pure))
size_t ___static_len(void *ptr __attribute__((__unused__)), size_t entry_sz, size_t sz)
{
	return sz / entry_sz;
}

static inline __attribute__((pure))
size_t ___dynamic_len(void *ptr, size_t entry_sz __attribute__((__unused__)),
		      size_t sz __attribute__((__unused__)))
{
	if (!ptr)
		return 0;
	return ___meta(ptr)->len;
}

#define count(ptr) ({\
	size_t count_ = 0;\
	foreach (ref, ptr) count_++;\
	count_;\
})

/**
 * @fn foreach (type *ref, type *ptr, size_t len = len(ptr))
 *
 * @brief Iterate over a static, a variable-length, a dynamic or
 * an associative array.
 *
 * @param ref array iterator name, not necessary to declare before
 * @param ptr pointer to an array
 * @param len number of elements to iterate, default is `len(ptr)`
 *
 */
#define foreach(ref, ptr, ...) \
	___apply(___foreach, ___narg(__VA_ARGS__))(ref, ptr, ##__VA_ARGS__)

#define ___foreach0(ref, ptr) \
	for (__auto_type ref = (ptr); ref < (ptr) + len(ptr); ref++) \
	if (___inuse(ptr, (ref) - (ptr)))

#define ___foreach1(ref, ptr, n) \
	for (__auto_type ref = (ptr); ref < (ptr) + (n); ref++)

/*
 * ### Dynamic arrays
 */

/**
 * @fn ssize_t append(type **pptr, type init);
 *
 * @brief Adds an element to the end of a dynamic array, expands memory
 * usage if necessary.
 *
 * @param pptr pointer to the dynamic array, may be any type
 * @param init initializer for a new array element, may be an aggregate initializer list
 *
 * @return Index in the array where the new value is appended or `-1`
 * if something went wrong.
 */
#define append(pptr, ...) ({\
	ssize_t slot_ = len(*(pptr));\
	typeof(*(pptr)) ptr_ = ___extend(*(pptr), slot_ + 1, sizeof(**(pptr)));\
	if (ptr_) {\
		ptr_[slot_] = (typeof(*ptr_))__VA_ARGS__;\
		*(pptr) = ptr_;\
	} else {\
		slot_ = -1;\
	}\
	slot_;\
})

/**
 * @fn ssize_t extend(type **pptr, ssize_t len);
 *
 * @brief Changes the size of a dynamic array.
 *
 * @param pptr pointer to the dynamic array, may be any type
 * @param len requested additional number of elements
 *
 * @return Index in the array from which it is extended or `-1`
 * if something went wrong.
 */
#define extend(pptr, ext) ({\
	ssize_t slot_ = len(*(pptr));\
	typeof(*(pptr)) ptr_ = ___extend(*(pptr), slot_ + (ext), sizeof(**(pptr)));\
	if (ptr_) {\
		*(pptr) = ptr_;\
	} else {\
		slot_ = -1;\
	}\
	slot_;\
})

void *___extend(void *old_ptr, size_t new_len, size_t entry_sz);

/**
 * @fn void vfree(type **ptr);
 *
 * @brief Releases allocated memory for each element of a dynamic array.
 *
 * @param ptr pointer to the dynamic array, may be any type
 */
#define vfree(ptr) ___vfree((void **)(ptr))

static inline void ___vfree(void **pptr)
{
	foreach (p, pptr)
		free(*p);
	free(pptr);
}

static inline void ___pvfree(void *ptr)
{
	void ***ppptr = (void ***)ptr;
	vfree(*ppptr);
	*ppptr = NULL;
}

/*
 * ### Associative arrays
 */

/**
 * @type entry(ktype, vtype)
 *
 * @brief Associative array element type.
 *
 * @param ktype associative array index type
 * @param vtype a type of value associated with the key
 *
 * The array index can be any built-in scalar type, a structure,
 * or a pointer to a null value. The array value can be any type.
 *
 * To pass associative array pointers to functions, the associative array
 * type must be fully qualified using the `typedef` keyword.
 */
#define entry(ktype, vtype) struct { ktype key; vtype value; }

#define ___typeof_key(pptr) typeof((*(pptr))->key)
#define ___typeof_value(pptr) typeof((*(pptr))->value)

#define ___typed_key_ptr(pptr, slot) \
	((slot) != -1) ? &(*(pptr))[slot].key : (___typeof_key(pptr) *)NULL;

#define ___typed_value_ptr(pptr, slot) \
	((slot) != -1) ? &(*(pptr))[slot].value : (___typeof_value(pptr) *)NULL;

#define ___key(entry) (entry)
#define ___entry(ptr, slot) ((ptr) + (slot)*meta->entry_sz)

struct ___entry_meta {
	size_t key_sz;
	size_t entry_sz;
	unsigned long (*hash)(const void *, size_t);
	int (*cmp)(const void *, const void *, size_t);
};

#define ___entry_meta(pptr) &(struct ___entry_meta){\
	sizeof((**(pptr)).key),\
	sizeof(**(pptr)),\
	func((const void *key_ptr, size_t key_sz),\
	     _Generic((**(pptr)).key,\
		      char *:		___hnv1az(*(char **)key_ptr),\
		      const char *:	___hnv1az(*(char **)key_ptr),\
		      default:		___hnv1a(key_ptr, key_sz))),\
					\
	func((const void *lhs, const void *rhs, size_t sz),\
	     _Generic((**(pptr)).key,\
		      char *:		__builtin_strcmp(*(char **)lhs, *(char **)rhs),\
		      const char *:	__builtin_strcmp(*(char **)lhs, *(char **)rhs),\
		      default:		__builtin_memcmp(lhs, rhs, sz)))\
}

#define keyof(ref, type) ({\
	const typeof(((type *)0)->value) *ref_ = (ref);\
	&(((type *)((void *)ref_ - __builtin_offsetof(type, value)))->key);\
})

static inline unsigned long ___hnv1a(const void *key, size_t len) {
	unsigned long hash = 14695981039346656037UL;
	for (size_t i = 0; i < len; i++) {
		hash ^= ((unsigned char *)key)[i];
		hash *= 1099511628211UL;
	}
	return hash;
}

static inline unsigned long ___hnv1az(const char *key) {
	unsigned long hash = 14695981039346656037UL;
	for (const char *p = key; *p; p++) {
		hash ^= *(unsigned char *)p;
		hash *= 1099511628211UL;
	}
	return hash;
}

/**
 * @fn bool rehash(entry(ktype, vtype) **pptr, size cap = 64);
 *
 * @brief Changes the capacity of an associative array.
 *
 * @param pptr pointer to the associative array
 * @param cap requested capacity
 *
 * @return On success, `true` is returned. If the requested
 * capacity is not enought `false` is returned and the original
 * associative array is left untouched.
 */
#define rehash(pptr, cap) ({\
	bool ret_ = false;\
	typeof(*(pptr)) ptr_;\
	ptr_ = ___rehash(*(void **)pptr, ___entry_meta(pptr), cap);\
	if (ptr_ && *(pptr) != ptr_) {\
		*(pptr) = ptr_;\
		ret_ = true;\
	}\
	ret_;\
})

void *___rehash(void *old_ptr, struct ___entry_meta *meta, size_t new_cap);

/**
 * @fn vtype *insert(entry(ktype, vtype) **pptr, ktype key, vtype init);
 *
 * @brief Adds a new element to a dynamic associative array only if it
 * did not exist, exapands memory usage if necessary.
 *
 * @param pptr pointer to the associative array, may be declared by `entry` macro
 * @param key associative array index value
 * @param init array value initializer, may be an aggregate initializer list
 *
 * @return Reference to the inserted data in the associative array or
 * NULL if something went wrong. The reference is valid until any method
 * on the associative array is called.
 */
#define insert(pptr, ...) \
	___insert_entry(pptr, false, {__VA_ARGS__})

/**
 * @fn vtype *update(entry(ktype, vtype) **pptr, ktype key, vtype init);
 *
 * @brief Adds a new element or update an existing element to a dynamic
 * associative array, exapands memory usage if necessary.
 *
 * @param pptr pointer to the associative array, may be declared by `entry` macro
 * @param key associative array index value
 * @param init array value initializer, may be an aggregate initializer list
 *
 * @return Reference to the updated data in the associative array or
 * NULL if something went wrong. The reference is valid until any method
 * on the associative array is called.
 */
#define update(pptr, ...) \
	___insert_entry(pptr, true, {__VA_ARGS__})

#define ___insert_entry(pptr, update, ...) ({\
	typeof(**(pptr)) entry_ = __VA_ARGS__;\
	ssize_t slot_ = ___insert((void **)pptr, ___entry_meta(pptr), &entry_,\
				  &entry_.key, update);\
	___typed_value_ptr(pptr, slot_);\
})

ssize_t ___insert(void **pptr, struct ___entry_meta *meta,
		  void *entry, void *key_ptr, bool update);

/**
 * @fn bool delete(entry(ktype, vtype) **pptr, vtype *ref);
 *
 * @brief Removes an element from an associative array.
 *
 * @param pptr pointer to the associative array
 * @param ref reference to a value associated with a key
 *
 * Value reference can be returned by `lookup()` method.
 *
 * @return On success, `true` is returned. If `ref` is invalid
 * `false` is returned.
 */
#define delete(pptr, ref) ({\
	___delete((void **)pptr, ___entry_meta(pptr), ref);\
})

bool ___delete(void **pptr, struct ___entry_meta *meta, void *value_ptr);

/**
 * @fn vtype *lookup(entry(ktype, vtype) **pptr, ktype key);
 *
 * @brief Searches a data associated with a key.
 *
 * @param pptr pointer to the associative array
 * @param key associative array key value
 *
 * @return Reference to the data that the `key` is associated with,
 * the reference is valid until any associative array method is called,
 * if the key doesn't exist NULL pointer will be returned.
 */
#define lookup(pptr, ...) ({\
	___typeof_key(pptr) key_ = __VA_ARGS__;\
	ssize_t slot_ = ___lookup((void **)pptr, ___entry_meta(pptr), &key_);\
	___typed_value_ptr(pptr, slot_);\
})

#ifdef LOOKUP_STAT
#define ___lookup_inc() ___lookups++
#define ___lookup_probe_inc() ___lookup_probes++
#else
#define ___lookup_inc()
#define ___lookup_probe_inc()
#endif
#define lookup_probes() (((double)___lookup_probes) / ___lookups)

extern ssize_t ___lookups;
extern ssize_t ___lookup_probes;

static inline ssize_t ___lookup(void **pptr, struct ___entry_meta *meta, void *key_ptr)
{
	void *ptr = *pptr;
	size_t cap = len(ptr);
	if (!cap)
		return -1;

	ssize_t slot = meta->hash(key_ptr, meta->key_sz) % cap;
	ssize_t end = slot;

	___lookup_inc();
	do {
		___lookup_probe_inc();
		if (!___inuse_test(ptr, slot))
			break;
		else if (meta->cmp(___key(___entry(ptr, slot)), key_ptr, meta->key_sz) == 0)
			return slot;
		slot = (slot + 1) % cap;
	} while (slot != end);

	return -1;
}

/*
 * ### Output helpers
 */

#define ___fill0(ptr, i)
#define ___fill1(ptr, i, x) (ptr)[i] = (x)
#define ___fill2(ptr, i, x, ...) ___fill1(ptr, i, x); ___fill1(ptr, i + 1, __VA_ARGS__)
#define ___fill3(ptr, i, x, ...) ___fill1(ptr, i, x); ___fill2(ptr, i + 1, __VA_ARGS__)
#define ___fill4(ptr, i, x, ...) ___fill1(ptr, i, x); ___fill3(ptr, i + 1, __VA_ARGS__)
#define ___fill5(ptr, i, x, ...) ___fill1(ptr, i, x); ___fill4(ptr, i + 1, __VA_ARGS__)
#define ___fill6(ptr, i, x, ...) ___fill1(ptr, i, x); ___fill5(ptr, i + 1, __VA_ARGS__)
#define ___fill7(ptr, i, x, ...) ___fill1(ptr, i, x); ___fill6(ptr, i + 1, __VA_ARGS__)
#define ___fill8(ptr, i, x, ...) ___fill1(ptr, i, x); ___fill7(ptr, i + 1, __VA_ARGS__)
#define ___fill9(ptr, i, x, ...) ___fill1(ptr, i, x); ___fill8(ptr, i + 1, __VA_ARGS__)
#define ___fill10(ptr, i, x, ...) ___fill1(ptr, i, x); ___fill9(ptr, i + 1, __VA_ARGS__)
#define ___fill11(ptr, i, x, ...) ___fill1(ptr, i, x); ___fill10(ptr, i + 1, __VA_ARGS__)
#define ___fill12(ptr, i, x, ...) ___fill1(ptr, i, x); ___fill11(ptr, i + 1, __VA_ARGS__)

#define ___fill(ptr, ...)\
	___apply(___fill, ___narg(__VA_ARGS__))(ptr, 0, ##__VA_ARGS__)

#define ___fill_shift(ptr, ...) ({\
	___fill(ptr, __VA_ARGS__);\
	(ptr) += ___narg(__VA_ARGS__);\
})

#define ___MAX_PR_FMT 4

#define ___fill_pr_fmt(ptr, x)\
	_Generic(x,\
		 _Bool:			___fill_shift(ptr, '%', 'd'),\
		 char:			___fill_shift(ptr, '%', 'c'),\
		 signed char:		___fill_shift(ptr, '%', 'h', 'h', 'i'),\
		 unsigned char:		___fill_shift(ptr, '%', 'h', 'h', 'u'),\
		 signed short:		___fill_shift(ptr, '%', 'h', 'i'),\
		 unsigned short:	___fill_shift(ptr, '%', 'h', 'u'),\
		 signed int:		___fill_shift(ptr, '%', 'i'),\
		 unsigned int:		___fill_shift(ptr, '%', 'u'),\
		 signed long:		___fill_shift(ptr, '%', 'l', 'i'),\
		 unsigned long:		___fill_shift(ptr, '%', 'l', 'u'),\
		 signed long long:	___fill_shift(ptr, '%', 'l', 'l', 'i'),\
		 unsigned long long:	___fill_shift(ptr, '%', 'l', 'l', 'u'),\
		 float:			___fill_shift(ptr, '%', '.', '2', 'f'),\
		 double:		___fill_shift(ptr, '%', '.', '4', 'f'),\
		 long double:		___fill_shift(ptr, '%', 'L', 'f'),\
		 char *:		___fill_shift(ptr, '%', 's'),\
		 const char *:		___fill_shift(ptr, '%', 's'),\
		 volatile char *:	___fill_shift(ptr, '%', 's'),\
		 volatile const char *:	___fill_shift(ptr, '%', 's'),\
		 default:		___fill_shift(ptr, '%', 'p'))

#define ___fill_fmt0(ptr, x)
#define ___fill_fmt1(ptr, x) ___fill_pr_fmt(ptr, x)
#define ___fill_fmt2(ptr, x, ...) ___fill_pr_fmt(ptr, x); ___fill_fmt1(ptr, __VA_ARGS__)
#define ___fill_fmt3(ptr, x, ...) ___fill_pr_fmt(ptr, x); ___fill_fmt2(ptr, __VA_ARGS__)
#define ___fill_fmt4(ptr, x, ...) ___fill_pr_fmt(ptr, x); ___fill_fmt3(ptr, __VA_ARGS__)
#define ___fill_fmt5(ptr, x, ...) ___fill_pr_fmt(ptr, x); ___fill_fmt4(ptr, __VA_ARGS__)
#define ___fill_fmt6(ptr, x, ...) ___fill_pr_fmt(ptr, x); ___fill_fmt5(ptr, __VA_ARGS__)
#define ___fill_fmt7(ptr, x, ...) ___fill_pr_fmt(ptr, x); ___fill_fmt6(ptr, __VA_ARGS__)
#define ___fill_fmt8(ptr, x, ...) ___fill_pr_fmt(ptr, x); ___fill_fmt7(ptr, __VA_ARGS__)
#define ___fill_fmt9(ptr, x, ...) ___fill_pr_fmt(ptr, x); ___fill_fmt8(ptr, __VA_ARGS__)
#define ___fill_fmt10(ptr, x, ...) ___fill_pr_fmt(ptr, x); ___fill_fmt9(ptr, __VA_ARGS__)
#define ___fill_fmt11(ptr, x, ...) ___fill_pr_fmt(ptr, x); ___fill_fmt10(ptr, __VA_ARGS__)
#define ___fill_fmt12(ptr, x, ...) ___fill_pr_fmt(ptr, x); ___fill_fmt11(ptr, __VA_ARGS__)
#define ___fill_fmt(ptr, ...)\
	___apply(___fill_fmt, ___narg(__VA_ARGS__))(ptr, __VA_ARGS__)

/**
 * @fn int fprint(FILE *fp, ...);
 *
 * @brief Prints a list of values into a stream.
 *
 * @param fp output stream
 * @param ... list of values or constants of standard type to print
 *
 * @return The number of characters printed.
 */
#define fprint(fp, ...) ({\
	char fmt_[___narg(__VA_ARGS__) * ___MAX_PR_FMT + 1];\
	char *dst_ = fmt_;\
	___fill_fmt(dst_, __VA_ARGS__);\
	*dst_++ = '\0';\
	fprintf(fp, fmt_, ##__VA_ARGS__);\
})

/**
 * @fn int print(...);
 *
 * @brief Prints a list of values into the standard output stream.
 *
 * @param ... list of values or constants of standard type to print
 *
 * @return The number of characters printed.
 */
#define print(...) fprint(stdout, ##__VA_ARGS__)

/**
 * @fn int fprintln(FILE *fp, ...);
 *
 * @brief Prints a line to a stream.
 *
 * @param fp output stream
 * @param ... list of values or constants of standard type to print
 *
 * @return The number of characters printed.
 */
#define fprintln(fp, ...) ({\
	char fmt_[___narg(__VA_ARGS__) * ___MAX_PR_FMT + 2];\
	char *dst_ = fmt_;\
	___fill_fmt(dst_, __VA_ARGS__);\
	*dst_++ = '\n';\
	*dst_++ = '\0';\
	fprintf(fp, fmt_, ##__VA_ARGS__);\
})

/**
 * @fn int println(...);
 *
 * @brief Prints a line to the standard output stream.
 *
 * @param ... list of values or constants of standard type to print
 *
 * @return The number of characters printed.
 */
#define println(...) fprintln(stdout, ##__VA_ARGS__)

/**
 * @fn int fprintv(FILE *fp, const char *sep, type *ptr, size_t len = len(ptr));
 *
 * @brief Print an array to a stream.
 *
 * @param fp output stream
 * @param sep separator between elements of the output array
 * @param ptr array of values or constants of standard type to print
 * @param len number of elements to output, default is `len()`
 *
 * @return The number of characters printed.
 */
#define fprintv(fp, sep, ptr, ...)\
	___apply(fprintv, ___narg(__VA_ARGS__))(fp, sep, ptr, ##__VA_ARGS__)

#define fprintv0(fp, sep, ptr) fprintv1(fp, sep, ptr, len(ptr))
#define fprintv1(fp, sep, ptr, len) ___fprintv(fp, sep, ptr, len)

#define ___fprintv(fp, sep, ptr, len) ({\
	int nb_ = 0;\
	size_t len_ = len;\
	char fmt_[2 * ___MAX_PR_FMT + 1];\
	char *dst_ = fmt_;\
	__auto_type ptr_ = ptr;\
	___fill_pr_fmt(dst_, "");\
	___fill_pr_fmt(dst_, *ptr_);\
	*dst_ = '\0';\
	for (size_t i_ = 0; i_ < len_; i_++) {\
		nb_ += fprintf(fp, fmt_, i_ ? sep : "", ptr_[i_]);\
	}\
	nb_;\
})

/**
 * @fn int printv(const char *sep, type *ptr, size_t len = len(ptr));
 *
 * @brief Print an array to the standard output stream.
 *
 * @param sep separator between elements of the output array
 * @param ptr array of values or constants of standard type to print
 * @param len number of elements to output, default is `len()`
 *
 * @return The number of characters printed.
 */
#define printv(sep, ptr, ...) fprintv(stdout, sep, ptr, ##__VA_ARGS__)

/*
 * ### String conversion
 */

/**
 * @fn char *join(...);
 *
 * @brief Concatenates a list of values into a single string.
 *
 * @param ... list of values or constants of standard type to join
 *
 * @return The pointer to joined string, should be released by calling `free()`.
 */
#define join(...) ({\
	char fmt_[___narg(__VA_ARGS__) * ___MAX_PR_FMT + 1];\
	char *dst_ = fmt_;\
	___fill_fmt(dst_, __VA_ARGS__);\
	*dst_++ = '\0';\
	int nb_ = snprintf(NULL, 0, fmt_, __VA_ARGS__);\
	char *buf_ = malloc(nb_ + 1);\
	if (buf_) sprintf(buf_, fmt_, __VA_ARGS__);\
	buf_;\
})

/**
 * @fn char *joinv(const char *sep, type *ptr, size_t len = len(ptr));
 *
 * @brief Concatenates an array into a single string.
 *
 * @param sep substring between the joined elements
 * @param ptr array of values or constants of standard type to join
 * @param len number of elements to join, default is `len(ptr)`
 *
 * @return The pointer to joined string, should be released by calling `free()`.
 */
#define joinv(sep, ptr, ...)\
	___apply(joinv, ___narg(__VA_ARGS__))(sep, ptr, ##__VA_ARGS__)

#define joinv0(sep, ptr) joinv1(sep, ptr, len(ptr))
#define joinv1(sep, ptr, len) ___joinv(sep, ptr, len)

#define ___joinv(sep, ptr, len) ({\
	int nb_ = 0;\
	size_t len_ = len;\
	char fmt_[2 * ___MAX_PR_FMT + 1];\
	char *dst_ = fmt_;\
	__auto_type ptr_ = ptr;\
	___fill_pr_fmt(dst_, "");\
	___fill_pr_fmt(dst_, *ptr_);\
	*dst_ = '\0';\
	for (size_t i_ = 0; i_ < len_; i_++) {\
		nb_ += snprintf(NULL, 0, fmt_, i_ ? sep : "", ptr_[i_]);\
	}\
	char *buf_ = malloc(nb_ + 1);\
	nb_ = 0;\
	for (size_t i_ = 0; i_ < len_; i_++) {\
		nb_ += sprintf(buf_ + nb_, fmt_, i_ ? sep : "", ptr_[i_]);\
	}\
	buf_;\
})

#define ___get_val(str, sep, next, p) ({\
	char *tok_ = ___get_tok(str, sep, &next);\
	*(p) = ___str_to(tok_, *(p));\
	free(tok_);\
})

char *___get_tok(const char *str, const char *sep, const char **next);

#define ___str_to(str, x) ({\
	const char *str_ = str;\
	(str_) ? \
	_Generic(x,\
		 _Bool:			strtoul(str_, NULL, 0),\
		 char:			str_[0],\
		 signed char:		(signed char)strtol(str_, NULL, 0),\
		 unsigned char:		(unsigned char)strtoul(str_, NULL, 0),\
		 signed short:		(signed short)strtol(str_, NULL, 0),\
		 unsigned short:	(unsigned short)strtoul(str_, NULL, 0),\
		 signed int:		(signed int)strtol(str_, NULL, 0),\
		 unsigned int:		(unsigned int)strtoul(str_, NULL, 0),\
		 signed long:		strtol(str_, NULL, 0),\
		 unsigned long:		strtoul(str_, NULL, 0),\
		 signed long long:	strtoll(str_, NULL, 0),\
		 unsigned long long:	strtoull(str_, NULL, 0),\
		 float:			strtof(str_, NULL),\
		 double:		strtod(str_, NULL),\
		 long double:		strtold(str_, NULL),\
		 char *:		__builtin_strdup(str_)) : 0;\
})

/*
 * ### String tokenization
 */

/**
 * @fn void split(const char *str, const char *sep, ...);
 *
 * @brief Splits a string into tokens and assigns the token values
 * to the specified list of variables.
 *
 * @param str the string to be parsed
 * @param sep substring delimits the tokens in the parsed string
 * @param ... list of pointers to variables to assign token values to
 *
 * Tokens will be converted to the target type before assignment.
 * For pointers to a string, the necessary amount of memory will be
 * allocated to store the token. Such memory should be released by
 * calling `free()`.
 */
#define split(str, sep, p, ...) ({\
	const char *next_ = NULL;\
	___get_val(str, sep, next_, p);\
	___apply(___split, ___narg(p, ##__VA_ARGS__))(sep, next_, ##__VA_ARGS__);\
})

#define ___split1(sep, next)
#define ___split2(sep, next, p) ___get_val(NULL, sep, next, p)
#define ___split3(sep, next, p, ...) ___get_val(NULL, sep, next, p); ___split2(sep, next, __VA_ARGS__)
#define ___split4(sep, next, p, ...) ___get_val(NULL, sep, next, p); ___split3(sep, next, __VA_ARGS__)
#define ___split5(sep, next, p, ...) ___get_val(NULL, sep, next, p); ___split4(sep, next, __VA_ARGS__)
#define ___split6(sep, next, p, ...) ___get_val(NULL, sep, next, p); ___split5(sep, next, __VA_ARGS__)
#define ___split7(sep, next, p, ...) ___get_val(NULL, sep, next, p); ___split6(sep, next, __VA_ARGS__)
#define ___split8(sep, next, p, ...) ___get_val(NULL, sep, next, p); ___split7(sep, next, __VA_ARGS__)
#define ___split9(sep, next, p, ...) ___get_val(NULL, sep, next, p); ___split8(sep, next, __VA_ARGS__)
#define ___split10(sep, next, p, ...) ___get_val(NULL, sep, next, p); ___split9(sep, next, __VA_ARGS__)
#define ___split11(sep, next, p, ...) ___get_val(NULL, sep, next, p); ___split10(sep, next, __VA_ARGS__)
#define ___split12(sep, next, p, ...) ___get_val(NULL, sep, next, p); ___split11(sep, next, __VA_ARGS__)

/**
 * @fn void splitv(const char *str, const char *sep, type **pptr);
 *
 * @brief Splits a string into tokens and adds the token values
 * to a dynamic array.
 *
 * @param str string to be parsed
 * @param sep substring separates tokens in the parsed string
 * @param pptr pointer to a list to assign token values to
 *
 * Tokens will be converted to the target type before assignment.
 */
#define splitv(str, sep, pptr) ({\
	const char *next_ = NULL;\
	for (char *tok_ = ___get_tok(str, sep, &next_); tok_; tok_ = ___get_tok(NULL, sep, &next_)) {\
		append(pptr, ___str_to(tok_, **(pptr)));\
		free(tok_);\
	}\
})

#define func(args, expr)\
	({ typeof(({ ___apply(___decl, ___narg args) args expr; })) ___func args { return expr; } ___func; })

#define ___decl1(x) x;
#define ___decl2(x, ...) x; ___decl1(__VA_ARGS__)
#define ___decl3(x, ...) x; ___decl2(__VA_ARGS__)
#define ___decl4(x, ...) x; ___decl3(__VA_ARGS__)
#define ___decl5(x, ...) x; ___decl4(__VA_ARGS__)
#define ___decl6(x, ...) x; ___decl5(__VA_ARGS__)
#define ___decl7(x, ...) x; ___decl6(__VA_ARGS__)
#define ___decl8(x, ...) x; ___decl7(__VA_ARGS__)
#define ___decl9(x, ...) x; ___decl8(__VA_ARGS__)
#define ___decl10(x, ...) x; ___decl9(__VA_ARGS__)
#define ___decl11(x, ...) x; ___decl10(__VA_ARGS__)
#define ___decl12(x, ...) x; ___decl11(__VA_ARGS__)

#endif

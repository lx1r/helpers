#ifndef ___HELPERS_H
#define ___HELPERS_H

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

#define ___fill0(ptr, p)
#define ___fill1(ptr, p, x) (ptr)[p] = (x)
#define ___fill2(ptr, p, x, ...) ___fill1(ptr, p, x); ___fill1(ptr, p + 1, __VA_ARGS__)
#define ___fill3(ptr, p, x, ...) ___fill1(ptr, p, x); ___fill2(ptr, p + 1, __VA_ARGS__)
#define ___fill4(ptr, p, x, ...) ___fill1(ptr, p, x); ___fill3(ptr, p + 1, __VA_ARGS__)
#define ___fill5(ptr, p, x, ...) ___fill1(ptr, p, x); ___fill4(ptr, p + 1, __VA_ARGS__)
#define ___fill6(ptr, p, x, ...) ___fill1(ptr, p, x); ___fill5(ptr, p + 1, __VA_ARGS__)
#define ___fill7(ptr, p, x, ...) ___fill1(ptr, p, x); ___fill6(ptr, p + 1, __VA_ARGS__)
#define ___fill8(ptr, p, x, ...) ___fill1(ptr, p, x); ___fill7(ptr, p + 1, __VA_ARGS__)
#define ___fill9(ptr, p, x, ...) ___fill1(ptr, p, x); ___fill8(ptr, p + 1, __VA_ARGS__)
#define ___fill10(ptr, p, x, ...) ___fill1(ptr, p, x); ___fill9(ptr, p + 1, __VA_ARGS__)
#define ___fill11(ptr, p, x, ...) ___fill1(ptr, p, x); ___fill10(ptr, p + 1, __VA_ARGS__)
#define ___fill12(ptr, p, x, ...) ___fill1(ptr, p, x); ___fill11(ptr, p + 1, __VA_ARGS__)
#define ___fill(ptr, ...)\
	___apply(___fill, ___narg(__VA_ARGS__))(ptr, 0, ##__VA_ARGS__)

#define ___fill_shift(ptr, ...) ({\
	___fill(ptr, __VA_ARGS__);\
	(ptr) += ___narg(__VA_ARGS__);\
})

#define static_len(ptr) (sizeof(ptr)/sizeof(ptr[0]))

#undef __always_inline
#define __always_inline inline __attribute__((always_inline))

#define ALIGN(x, a)		ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN_DOWN(x, a)	ALIGN((x) - ((a) - 1), (a))
#define ALIGN_MASK(x, mask)	(((x) + (mask)) & ~(mask))

#define BITS_PER_LONG	64
#define BIT_MASK(nr)	(1ULL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)	((nr) / BITS_PER_LONG)

static __always_inline bool test_bit(unsigned long nr, unsigned long *bits)
{
	if (bits[BIT_WORD(nr)] & BIT_MASK(nr))
		return true;
	return false;
}

static __always_inline void set_bit(unsigned long nr, unsigned long *bits)
{
	bits[BIT_WORD(nr)] |= BIT_MASK(nr);
}

static __always_inline void clear_bit(unsigned long nr, unsigned long *bits)
{
	bits[BIT_WORD(nr)] &= ~BIT_MASK(nr);
}

#ifndef NO_LIBC

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

static inline void ___pclose(int *pfd) { close(*pfd); }
static inline void ___pfclose(FILE **pfp) { fclose(*pfp); }
static inline void ___pfree(void *pptr) { free(*(void **)pptr); }
#define __defer(func) __attribute__((__cleanup__(___p##func)))

static inline void *zalloc(size_t size) { return calloc(1, size); }
static void inline ___zfree(void **ptr) { free(*ptr); *ptr = NULL; }
#define zfree(ptr) ___zfree((void **)(ptr))

#define ___HAS_USED_MASK	(1UL << (BITS_PER_LONG - 1))
#define ___cap(ptr)		(ALIGN_DOWN(malloc_usable_size(ptr), sizeof(size_t)))
#define ___user_sz(ptr, len)	(ALIGN((len) * sizeof(*(ptr)), sizeof(size_t)))
#define ___meta_len_sz()	sizeof(size_t)
#define ___meta_used_sz(len)	((BIT_WORD((len) - 1) + 1) * sizeof(unsigned long))

static inline size_t *___meta_len_ptr(void *ptr)
{
	return ptr + ___cap(ptr) - ___meta_len_sz();
}

static inline size_t ___meta_len(size_t *len_ptr)
{
	return *len_ptr & ~___HAS_USED_MASK;
}

static inline bool ___meta_has_used(size_t *len_ptr)
{
	return !!(*len_ptr & ___HAS_USED_MASK);
}

static inline void ___meta_set_len(size_t *len_ptr, size_t len, bool has_used)
{
	*len_ptr = len;
	if (has_used) *len_ptr |= ___HAS_USED_MASK;
}

static inline unsigned long *___meta_used_ptr(void *len_ptr)
{
	if (!___meta_has_used(len_ptr))
		return NULL;
	size_t len = ___meta_len(len_ptr);
	return (void *)len_ptr - ___meta_used_sz(len);
}

/**
 * @brief **len()** returns the number of items in a dynamic or
 * an associative array
 * @param pptr pointer to the dynamic or associative array
 * @return number of items in the array
 */
static inline size_t len(void *ptr)
{
	if (!ptr)
		return 0;
	size_t *len_ptr = ___meta_len_ptr(ptr);
	return ___meta_len(len_ptr);
}

static inline void *___extend(void *ptr, size_t len, size_t sz, bool has_used)
{
	unsigned long *prev_used_ptr = NULL;
	size_t prev_used_sz = 0;

	if (ptr) {
		size_t *len_ptr = ___meta_len_ptr(ptr);
		prev_used_ptr = ___meta_used_ptr(len_ptr);
		prev_used_sz = ___meta_used_sz(___meta_len(len_ptr));
	}
	if (!ptr && !len)
		len = 32;

	ptr = realloc(ptr, sz*len + ___meta_used_sz(len) + ___meta_len_sz());
	if (!ptr)
		return NULL;

	size_t *len_ptr = ___meta_len_ptr(ptr);
	___meta_set_len(len_ptr, len, has_used || prev_used_ptr);
	unsigned long *used_ptr = ___meta_used_ptr(len_ptr);

	if (prev_used_ptr)
		memmove(used_ptr, prev_used_ptr, prev_used_sz);
	if (has_used || prev_used_ptr)
		memset(used_ptr + prev_used_sz, 0, ___meta_used_sz(len) - prev_used_sz);

	return ptr;
}

#define ___meta_used_test(ptr, slot) ({\
	test_bit(slot, ___meta_used_ptr(___meta_len_ptr(ptr)));\
})

#define ___meta_used_set(ptr, slot) ({\
	set_bit(slot, ___meta_used_ptr(___meta_len_ptr(ptr)));\
})

#define ___meta_used_clear(ptr, slot) ({\
	clear_bit(slot, ___meta_used_ptr(___meta_len_ptr(ptr)));\
})

static inline bool used(void *ptr, ssize_t slot)
{
	if (!ptr)
		return false;

	size_t *len_ptr = ___meta_len_ptr(ptr);

	return !___meta_has_used(len_ptr) ||
		test_bit(slot, ___meta_used_ptr(len_ptr));
}

#define reserve(pptr, len) ({\
	*(pptr) = ___extend(NULL, len, sizeof(*(*(pptr))), true);\
})

static inline size_t ___align_sz(size_t nb)
{
	if (!nb) return 0;
	/* initial size */
	if (!(nb & ~0xf))
		return 64;
	/* align to page */
	if (nb & ~0xfff)
		return (nb + 0xfff) & ~0xfff;
	/* align to power of 2 */
	nb = nb - 1;
	nb |= (nb >> 1);
	nb |= (nb >> 2);
	nb |= (nb >> 4);
	return nb + 1;
}

/**
 * @brief **append()** adds an element to the end of a dynamic array,
 * expands memory usage if necessary
 * @param pptr pointer to the dynamic array, may be any type
 * @param init initializer for a new array element, may be an aggregate
 * initializer list
 * @return index in the array where the new value is appended
 */
#define append(pptr, ...) ({\
	ssize_t len_ = len(*(pptr)) + 1;\
	size_t sz_ = ___align_sz(___user_sz(*(pptr), len_) + ___meta_len_sz());\
	typeof(*(pptr)) ptr_ = realloc(*(pptr), sz_);\
	if (ptr_) {\
		ptr_[len_ - 1] = (typeof(*ptr_))__VA_ARGS__;\
		*___meta_len_ptr(ptr_) = len_;\
		*(pptr) = ptr_;\
	} else {\
		len_ = 0;\
	}\
	len_ - 1;\
})

static inline unsigned long ___hnv1a(const void *key, size_t len) {
	unsigned long hash = 14695981039346656037UL;
	for (int i = 0; i < len; i++) {
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

#define ___PROBE_STEP 4

static inline ssize_t ___try_insert(void **pptr, void *data, size_t data_sz, size_t key_sz,
				    unsigned long (*hashfn)(const void *, size_t))
{
	void *ptr = *pptr;
	size_t cap = len(ptr);
	unsigned long hash = hashfn(data, key_sz);

	for (size_t i = 0; i < cap; i += ___PROBE_STEP) {
		size_t slot = (hash + i) % cap;
		if (!___meta_used_test(ptr, slot)) {
			memcpy(ptr + slot*data_sz, data, data_sz);
			___meta_used_set(ptr, slot);
			//printf("insert: slot=%zu\n", slot);
			return slot;
		}
	}
	return -1;
}

static inline void *___rehash(void *ptr, size_t ext_cap, size_t data_sz, size_t key_sz,
			      unsigned long (*hashfn)(const void *, size_t))
{
	size_t cap = len(ptr);
	if (!ext_cap) ext_cap = 32;
	printf("rehash: cap=%zd\n", ext_cap);

	void *ext_ptr = ___extend(NULL, ext_cap, data_sz, true);
	if (!ext_ptr)
		return NULL;

	for (size_t slot = 0; slot < cap; slot++) {
		if (___meta_used_test(ptr, slot)) {
			ssize_t rc = ___try_insert(&ext_ptr, ptr + slot*data_sz, data_sz, key_sz, hashfn);
			if (rc == -1) {
				free(ext_ptr);
				return NULL;
			}
		}
	}
	free(ptr);
	return ext_ptr;
}

static inline ssize_t ___insert(void **pptr, void *data, size_t data_sz, size_t key_sz,
				unsigned long (*hashfn)(const void *, size_t))
{
	void *ptr = *pptr;
	//printf("insert: data_sz=%zu key_sz=%zu\n", data_sz, key_sz);
	do {
		size_t slot = ___try_insert(pptr, data, data_sz, key_sz, hashfn);
		if (slot != -1)
			return slot;
		ptr = ___rehash(ptr, 2*len(ptr), data_sz, key_sz, hashfn);
		if (ptr) *pptr = ptr;
	} while (ptr);

	return -1;
}

static inline void *___lookup(void **pptr, void *key_ptr, size_t data_sz, size_t key_sz, size_t val_off,
			      unsigned long (*hashfn)(const void *, size_t),
			      int (*cmprfn)(const void *, const void *, size_t))
{
	void *ptr = *pptr;
	//printf("lookup: data_sz=%zu key_sz=%zu\n", data_sz, key_sz);
	size_t cap = len(ptr);
	unsigned long hash = hashfn(key_ptr, key_sz);
	//printf("lookup: key=%d, hash=%lx\n", *(int*)key_ptr, hash);

	for (size_t i = 0; i < cap; i += ___PROBE_STEP) {
		size_t slot = (hash + i) % cap;
		//printf("lookup: slot=%zu\n", slot);
		if (___meta_used_test(ptr, slot) && cmprfn(ptr + slot*data_sz, key_ptr, key_sz) == 0) {
			return ptr + slot*data_sz + val_off;
		}
	}
	return NULL;
}

#define ___decl_hashfn(key, func_name) \
	unsigned long func_name(const void *key_ptr, size_t key_sz) {\
		return _Generic(key,\
				char *:		___hnv1az(*(char **)key_ptr),\
				const char *:	___hnv1az(*(char **)key_ptr),\
				default:	___hnv1a(key_ptr, key_sz));\
	}

#define ___decl_cmprfn(key, func_name) \
	int func_name(const void *lhs, const void *rhs, size_t sz) {\
		return _Generic(key,\
				char *:		strcmp(*(char **)lhs, *(char **)rhs),\
				const char *:	strcmp(*(char **)lhs, *(char **)rhs),\
				default:	memcmp(lhs, rhs, sz));\
	}

#define ___value_offset(ptr) ((void *)&(ptr)->value - (void *)(ptr))

/**
 * @brief **mapof()** defines associative array type
 * @param key_type associative array index (key) type, can be any non-pointer
 * type except a pointer to a string
 * @param val_type a type of value associated with the key, can be any type
 *
 * To pass associative array pointers to functions, the associative array type
 * must be fully qualified using the `typedef` keyword.
 */
#define mapof(key_type, val_type) struct { key_type key; val_type value; }

/**
 * @brief **insert()** adds an element to a dynamic associative array,
 * expands memory usage if necessary
 * @param pptr pointer to the associative array, may be declared using
 * `mapof` macro
 * @param key associative array index value, maybe any standard type
 * @param init initializer for a new data element, may be an aggregate
 * initializer list
 * @return index in the array where the new value is inserted,
 * the index is valid until any method of the associative array is called
 */
#define insert(pptr, k, ...) ({\
	typeof(**(pptr)) data_ = {k, (typeof((*(pptr))->value))__VA_ARGS__};\
	___decl_hashfn(data_.key, hashfn_);\
	___insert((void **)pptr, &data_, sizeof(data_), sizeof((*(pptr))->key), hashfn_);\
})

/**
 * @brief **delete()** removes an element from an associative array
 * @param pptr pointer to the associative array
 * @param val_ref reference to a data value associated with a key
 * in the array, can be returned by `lookup()` method
 * @return index in the array that `val_ref` belonged to,
 * the index is valid until any associative array method is called
 */
#define delete(pptr, val_ref) ({\
	typeof(*(pptr)) ptr_ = *(pptr);\
	typeof(*(pptr)) slot_ptr_ = (void *)(val_ref) - ___value_offset(ptr_);\
	ssize_t slot_ = slot_ptr_ - ptr_;\
	___meta_used_clear(ptr_, slot_);\
	slot_;\
})

/**
 * @brief **lookup()** searches a data associated with a key
 * @param pptr pointer to the associative array
 * @param key associative array key value
 * @return reference to the data that the `key` is associated with,
 * the reference is valid until any associative array method is called
 */
#define lookup(pptr, k) ({\
	typeof((*(pptr))->key) key_ = k;\
	___decl_hashfn(key_, hashfn_);\
	___decl_cmprfn(key_, cmprfn_);\
	___lookup((void **)pptr, &key_, sizeof(**(pptr)), sizeof((*(pptr))->key), \
		  ___value_offset(*(pptr)), hashfn_, cmprfn_);\
})

#define ___fill_pr_fmt(ptr, x)\
	_Generic(x,\
		 _Bool:                 ___fill_shift(ptr, '%', 'd'),\
		 char:                  ___fill_shift(ptr, '%', 'c'),\
		 signed char:           ___fill_shift(ptr, '%', 'h', 'h', 'i'),\
		 unsigned char:         ___fill_shift(ptr, '%', 'h', 'h', 'u'),\
		 signed short:          ___fill_shift(ptr, '%', 'h', 'i'),\
		 unsigned short:        ___fill_shift(ptr, '%', 'h', 'u'),\
		 signed int:            ___fill_shift(ptr, '%', 'i'),\
		 unsigned int:          ___fill_shift(ptr, '%', 'u'),\
		 signed long:           ___fill_shift(ptr, '%', 'l', 'i'),\
		 unsigned long:         ___fill_shift(ptr, '%', 'l', 'u'),\
		 signed long long:      ___fill_shift(ptr, '%', 'l', 'l', 'i'),\
		 unsigned long long:    ___fill_shift(ptr, '%', 'l', 'l', 'u'),\
		 float:                 ___fill_shift(ptr, '%', 'f'),\
		 double:                ___fill_shift(ptr, '%', 'f'),\
		 long double:           ___fill_shift(ptr, '%', 'L', 'f'),\
		 char *:                ___fill_shift(ptr, '%', 's'),\
		 const char *:          ___fill_shift(ptr, '%', 's'),\
		 default:               ___fill_shift(ptr, '%', 'p'))

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

#define fprint(fp, ...) ({\
	char fmt_[___narg(__VA_ARGS__)*4 + 1];\
	char *dst_ = fmt_;\
	___fill_fmt(dst_, __VA_ARGS__);\
	*dst_++ = '\0';\
	fprintf(fp, fmt_, ##__VA_ARGS__);\
})

#define print(...) fprint(stdout, ##__VA_ARGS__)

/**
 * @brief **fprintln()** print a line to a stream
 * @param fp output stream
 * @param ... list of values or constants of standard type to print
 * @return the number of bytes printed
 */
#define fprintln(fp, ...) ({\
	char fmt_[___narg(__VA_ARGS__)*4 + 2];\
	char *dst_ = fmt_;\
	___fill_fmt(dst_, __VA_ARGS__);\
	*dst_++ = '\n';\
	*dst_++ = '\0';\
	fprintf(fp, fmt_, ##__VA_ARGS__);\
})

/**
 * @brief **println()** print a line to the standard output stream
 * @param ... list of values or constants of standard type to print
 * @return the number of bytes printed
 */
#define println(...) fprintln(stdout, ##__VA_ARGS__)

#define ___printv(fp, tokens, nr_tokens, delim) ({\
	int nb = 0;\
	size_t len_ = (nr_tokens);\
	const char *delim_ = "";\
	char fmt_[4 + 2 + 1];\
	char *dst_ = fmt_;\
	typeof(tokens) *tokens_ptr_ = &(tokens);\
	___fill_pr_fmt(dst_, delim_);\
	___fill_pr_fmt(dst_, **tokens_ptr_);\
	*dst_ = '\0';\
	for (size_t i_ = 0; i_ < len_; i_++) {\
		nb += fprintf(fp, fmt_, delim_, (*tokens_ptr_)[i_]);\
		if (i_ == 0) delim_ = (delim);\
	}\
	nb;\
})

#define fprintv2(fp, tokens, nr_tokens, delim) ___printv(fp, tokens, nr_tokens, delim)
#define fprintv1(fp, tokens, nr_tokens) fprintv2(fp, tokens, nr_tokens, ",")
#define fprintv0(fp, tokens) fprintv1(fp, tokens, len(tokens))
#define fprintv(fp, tokens, ...)\
	___apply(fprintv, ___narg(__VA_ARGS__))(fp, tokens, ##__VA_ARGS__)

/**
 * @brief **printv()** print an array to the standard output stream
 * @param tokens array of values or constants of standard type to print
 * @param nr_tokens number of tokens to output (default is len())
 * @param delim delimiter output between the tokens (default ",")
 * @return the number of bytes printed
 */
#define printv(tokens, ...) fprintv(stdout, tokens, ##__VA_ARGS__)

static inline void ___fprint_seq(FILE *fp, unsigned long start, unsigned long end,
				 const char **period_ptr, const char *comma, const char *dash)
{
	if (start == -1)
		return;

	unsigned long diff = end - start;

	if (diff == 0)
		fprintf(fp, "%s%ld", *period_ptr, start);
	else if (diff == 1)
		fprintf(fp, "%s%ld%s%ld", *period_ptr, start, comma, end);
	else
		fprintf(fp, "%s%ld%s%ld", *period_ptr, start, dash, end);

	*period_ptr = comma;
}

static inline void fprintb2(FILE *fp, unsigned long *bits, unsigned long nr_bits,
			    const char *comma, const char *dash)
{
	const char *period = "";
	unsigned long start = -1, end;

	for (unsigned long i = 0; i < nr_bits; i++) {
		if (test_bit(i, bits)) {
			if (start == -1)
				start = i;
			end = i;
		} else {
			___fprint_seq(fp, start, end, &period, comma, dash);
			start = -1;
		}
	}
	___fprint_seq(fp, start, end, &period, comma, dash);
}

#define fprintb1(fp, bits, nr_bits, comma) fprintb2(fp, bits, nr_bits, comma, "-")
#define fprintb0(fp, bits, nr_bits) fprintb1(fp, bits, nr_bits, ",")
#define fprintb(fp, bits, nr_bits, ...)\
	___apply(fprintb, ___narg(__VA_ARGS__))(fp, bits, nr_bits, ##__VA_ARGS__)

#define printb(bits, nr_bits, ...) fprintb(stdout, bits, nr_bits, ##__VA_ARGS__)

/**
 * @brief **join()** concatenates an list of values into a single string
 * @param ... list of values or constants of standard type to join
 * @return the pointer to joined string, should be released by calling `free()`
 */
#define join(...) ({\
	char fmt_[___narg(__VA_ARGS__)*4 + 1];\
	char *dst_ = fmt_;\
	___fill_fmt(dst_, __VA_ARGS__);\
	*dst_++ = '\0';\
	size_t nb = snprintf(NULL, 0, fmt_, __VA_ARGS__);\
	char *buf = malloc(nb + 1);\
	if (buf) sprintf(buf, fmt_, __VA_ARGS__);\
	buf;\
})

/**
 * @brief **joinv()** concatenates an array into a single string
 * @param tokens array of values or constants of standard type to join
 * @param nr_tokens number of elements to join (default is len(tokens))
 * @param delim substring between the joined elements (default ",")
 * @return the pointer to joined string, should be released by calling `free()`
 */

#define joinv(tokens, nr_tokens, delim) //TODO

#define ___strto(x, s) ({\
	const char *str_ = s;\
	(str_) ? \
	_Generic(x,\
		 _Bool:                 strcasecmp(str_, "true") == 0 || strtoul(str_, NULL, 0),\
		 char:                  str_[0],\
		 signed char:           (signed char)strtol(str_, NULL, 0),\
		 unsigned char:         (unsigned char)strtoul(str_, NULL, 0),\
		 signed short:          (signed short)strtol(str_, NULL, 0),\
		 unsigned short:        (unsigned short)strtoul(str_, NULL, 0),\
		 signed int:            (signed int)strtol(str_, NULL, 0),\
		 unsigned int:          (unsigned int)strtoul(str_, NULL, 0),\
		 signed long:           strtol(str_, NULL, 0),\
		 unsigned long:         strtoul(str_, NULL, 0),\
		 signed long long:      strtoll(str_, NULL, 0),\
		 unsigned long long:    strtoull(str_, NULL, 0),\
		 float:                 strtof(str_, NULL),\
		 double:                strtod(str_, NULL),\
		 long double:           strtold(str_, NULL),\
		 char *:                strdup(str_)) : 0;\
})

#define ___splitn(str, delim, p) *(p) = ___strto(*(p), strtok(NULL, delim))
#define ___split1(str, delim)
#define ___split2(str, delim, p) ___splitn(str, delim, p)
#define ___split3(str, delim, p, ...) ___splitn(str, delim, p); ___split2(str, delim, __VA_ARGS__)
#define ___split4(str, delim, p, ...) ___splitn(str, delim, p); ___split3(str, delim, __VA_ARGS__)
#define ___split5(str, delim, p, ...) ___splitn(str, delim, p); ___split4(str, delim, __VA_ARGS__)
#define ___split6(str, delim, p, ...) ___splitn(str, delim, p); ___split5(str, delim, __VA_ARGS__)
#define ___split7(str, delim, p, ...) ___splitn(str, delim, p); ___split6(str, delim, __VA_ARGS__)
#define ___split8(str, delim, p, ...) ___splitn(str, delim, p); ___split7(str, delim, __VA_ARGS__)
#define ___split9(str, delim, p, ...) ___splitn(str, delim, p); ___split8(str, delim, __VA_ARGS__)
#define ___split10(str, delim, p, ...) ___splitn(str, delim, p); ___split9(str, delim, __VA_ARGS__)
#define ___split11(str, delim, p, ...) ___splitn(str, delim, p); ___split10(str, delim, __VA_ARGS__)
#define ___split12(str, delim, p, ...) ___splitn(str, delim, p); ___split11(str, delim, __VA_ARGS__)

/**
 * @brief **split()** splits a string into tokens and assigns
 * the token values to the specified list of variables
 * @param str the string to be parsed
 * @param delim substring delimits the tokens in the parsed string
 * @param ... list of pointers to variables to assign token values to,
 * tokens will be converted to target type before assignment
 */
#define split(str, delim, p, ...) ({\
	char *__defer(free) dup_ = strdup(str);\
	*(p) = ___strto(*(p), strtok(dup_, delim));\
	___apply(___split, ___narg(p, __VA_ARGS__))(dup_, delim, ##__VA_ARGS__);\
})

/**
 * @brief **splitv()** splits a string into tokens and assigns the token values to a list
 * @param str the string to be parsed
 * @param delim substring delimits the tokens in the parsed string
 * @param pptr pointer to a list to assign token values to
 * tokens will be converted to target type before assignment
 */
#define splitv(str, delim, pptr) ({\
	char *__defer(free) dup_ = strdup(str);\
	for (char *token_ = strtok(dup_, delim); token_; token_ = strtok(NULL, delim)) {\
		append(pptr, ___strto(**(pptr), token_));\
	}\
})

static inline int splitb(const char *str, const char *delim, const char **tokens, int nr_tokens, unsigned long *bits)
{
	char __defer(free) *dup = strdup(str);
	if (!dup)
		return -1;

	for (char *token = strtok(dup, delim); token; token = strtok(NULL, delim)) {
		for (int i = 0; i < nr_tokens; i++)
			if (strcmp(token, tokens[i]) == 0)
				set_bit(i, bits);
	}
	return 0;
}

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

#define func(args, expr)\
	({ typeof(({ ___apply(___decl, ___narg args) args expr; })) ___func args { return expr; } ___func; })

#endif /* NO_LIBC */
#endif

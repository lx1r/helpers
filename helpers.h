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

#define static_len(va) (sizeof(va)/sizeof(va[0]))

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

static inline size_t *___meta_len_ptr(void *ptr, size_t cap)
{
	return ptr + cap - ___meta_len_sz();
}

static inline unsigned long *___meta_used_ptr(void *ptr, size_t cap)
{
	size_t *len_ptr = ___meta_len_ptr(ptr, cap);
	size_t len = *len_ptr & ~___HAS_USED_MASK;
	return (void *)len_ptr - ___meta_used_sz(len);
}

static inline size_t len(void *ptr)
{
	if (!ptr)
		return 0;
	size_t *len_ptr = ___meta_len_ptr(ptr, ___cap(ptr));
	return *len_ptr & ~___HAS_USED_MASK;
}

#define reserve(pptr, len) ({\
	size_t new_len = len;\
	*(pptr) = malloc(___user_sz(*(pptr), new_len) + ___meta_used_sz(new_len) + ___meta_len_sz());\
	size_t cap = ___cap(*(pptr));\
	size_t *len_ptr = ___meta_len_ptr(*(pptr), cap);\
	*len_ptr = new_len | ___HAS_USED_MASK;\
	unsigned long *used_ptr = ___meta_used_ptr(*(pptr), cap);\
	memset(used_ptr, 0, ___meta_used_sz(new_len));\
})

#define ___meta_used_test(ptr, slot) ({\
	unsigned long *used_ptr = ___meta_used_ptr(ptr, ___cap(ptr));\
	test_bit(slot, used_ptr);\
})

#define ___meta_used_set(ptr, slot) ({\
	unsigned long *used_ptr = ___meta_used_ptr(ptr, ___cap(ptr));\
	set_bit(slot, used_ptr);\
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

#define append(pptr, ...) ({\
	ssize_t old_len = len(*(pptr));\
	ssize_t new_len = old_len + 1;\
	size_t sz = ___align_sz(___user_sz(*(pptr), new_len) + ___meta_len_sz());\
	typeof(*(pptr)) ptr = realloc(*(pptr), sz);\
	if (ptr) {\
		*(ptr + old_len) = (typeof(*ptr))__VA_ARGS__;\
		*___meta_len_ptr(ptr, ___cap(ptr)) = new_len;\
		*(pptr) = ptr;\
	} else {\
		new_len = 0;\
	}\
	new_len - 1;\
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
	char fmt[___narg(__VA_ARGS__)*4 + 1];\
	char *dst = fmt;\
	___fill_fmt(dst, __VA_ARGS__);\
	*dst++ = '\0';\
	fprintf(fp, fmt, ##__VA_ARGS__);\
})

#define print(...) fprint(stdout, ##__VA_ARGS__)

#define fprintln(fp, ...) ({\
	char fmt[___narg(__VA_ARGS__)*4 + 2];\
	char *dst = fmt;\
	___fill_fmt(dst, __VA_ARGS__);\
	*dst++ = '\n';\
	*dst++ = '\0';\
	fprintf(fp, fmt, ##__VA_ARGS__);\
})

#define println(...) fprintln(stdout, ##__VA_ARGS__)

#define fprintv(fp, slots, len) ({ do {\
	size_t n = (len);\
	if (n == 0) break;\
	char fmt[4 + 2 + 1];\
	char *dst = fmt;\
	___fill_pr_fmt(dst, (slots)[0]);\
	*dst = '\0';\
	fprintf(fp, fmt, (slots)[0]);\
	if (n == 1) break;\
	dst = fmt;\
	*dst++ = ',';\
	*dst++ = ' ';\
	___fill_pr_fmt(dst, (slots)[0]);\
	*dst = '\0';\
	for (size_t i = 1; i < n; i++) {\
		fprintf(fp, fmt, (slots)[i]);\
	}\
} while(0); })

#define printv(tokens, n) fprintv(stdout, tokens, n)

#define join(...) ({\
	char fmt[___narg(__VA_ARGS__)*4 + 1];\
	char *dst = fmt;\
	___fill_fmt(dst, __VA_ARGS__);\
	*dst++ = '\0';\
	size_t nb = snprintf(NULL, 0, fmt, __VA_ARGS__);\
	char *buf = malloc(nb + 1);\
	if (buf) sprintf(buf, fmt, __VA_ARGS__);\
	buf;\
})

#define ___strto(x, s) ({\
	const char *str = s;\
	(str) ? \
	_Generic(x,\
		 _Bool:                 strcasecmp(str, "true") == 0 || strtoul(str, NULL, 0),\
		 char:                  str[0],\
		 signed char:           (signed char)strtol(str, NULL, 0),\
		 unsigned char:         (unsigned char)strtoul(str, NULL, 0),\
		 signed short:          (signed short)strtol(str, NULL, 0),\
		 unsigned short:        (unsigned short)strtoul(str, NULL, 0),\
		 signed int:            (signed int)strtol(str, NULL, 0),\
		 unsigned int:          (unsigned int)strtoul(str, NULL, 0),\
		 signed long:           strtol(str, NULL, 0),\
		 unsigned long:         strtoul(str, NULL, 0),\
		 signed long long:      strtoll(str, NULL, 0),\
		 unsigned long long:    strtoull(str, NULL, 0),\
		 float:                 strtof(str, NULL),\
		 double:                strtod(str, NULL),\
		 long double:           strtold(str, NULL),\
		 char *:                strdup(str)) : 0;\
})

#define splitv(str, delim, pptr) ({\
	char *dup = strdup(str);\
	for (char *token = strtok(dup, delim); token; token = strtok(NULL, delim)) {\
		append(pptr, ___strto(**(pptr), token));\
	}\
	free(dup);\
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

#define split(str, delim, p, ...) ({\
	char __defer(free) *dup = strdup(str);\
	*(p) = ___strto(*(p), strtok(dup, delim));\
	___apply(___split, ___narg(p, __VA_ARGS__))(dup, delim, ##__VA_ARGS__);\
})

static inline int splitb(const char *str, const char *delim, const char **tokens, int n, unsigned long *bits)
{
	char __defer(free) *dup = strdup(str);
	if (!dup)
		return -1;

	for (char *token = strtok(dup, delim); token; token = strtok(NULL, delim)) {
		for (int i = 0; i < n; i++)
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

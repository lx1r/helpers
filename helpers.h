#ifndef ___HELPERS_H
#define ___HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

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

#define ___fill0(arr, p)
#define ___fill1(arr, p, x) arr[p] = x
#define ___fill2(arr, p, x, ...) arr[p] = x; ___fill1(arr, p + 1, __VA_ARGS__)
#define ___fill3(arr, p, x, ...) arr[p] = x; ___fill2(arr, p + 1, __VA_ARGS__)
#define ___fill4(arr, p, x, ...) arr[p] = x; ___fill3(arr, p + 1, __VA_ARGS__)
#define ___fill5(arr, p, x, ...) arr[p] = x; ___fill4(arr, p + 1, __VA_ARGS__)
#define ___fill6(arr, p, x, ...) arr[p] = x; ___fill5(arr, p + 1, __VA_ARGS__)
#define ___fill7(arr, p, x, ...) arr[p] = x; ___fill6(arr, p + 1, __VA_ARGS__)
#define ___fill8(arr, p, x, ...) arr[p] = x; ___fill7(arr, p + 1, __VA_ARGS__)
#define ___fill9(arr, p, x, ...) arr[p] = x; ___fill8(arr, p + 1, __VA_ARGS__)
#define ___fill10(arr, p, x, ...) arr[p] = x; ___fill9(arr, p + 1, __VA_ARGS__)
#define ___fill11(arr, p, x, ...) arr[p] = x; ___fill10(arr, p + 1, __VA_ARGS__)
#define ___fill12(arr, p, x, ...) arr[p] = x; ___fill11(arr, p + 1, __VA_ARGS__)

#define ___fill(arr, ...)\
	___apply(___fill, ___narg(__VA_ARGS__))(arr, 0, ##__VA_ARGS__)

#define ___fill_shift(arr, ...) ({\
	___fill(arr, __VA_ARGS__);\
	(arr) += ___narg(__VA_ARGS__);\
})

static inline void ___pfree(void *pptr) { free(*(void **)pptr); }
#define ___defer_free __attribute__((__cleanup__(___pfree)))

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

struct ___meta_len {
	size_t len;
};

struct ___meta_used {
	unsigned long bits[0];
};

#define ___HAS_USED_MASK	(1UL << (BITS_PER_LONG - 1))
#define ___cap(ptr)		(ALIGN_DOWN(malloc_usable_size(ptr), sizeof(size_t)))
#define ___user_sz(ptr, len)	(ALIGN((len) * sizeof(*(ptr)), sizeof(size_t)))
#define ___meta_len_sz()	sizeof(size_t)
#define ___meta_used_sz(len)	((BIT_WORD((len) - 1) + 1) * sizeof(unsigned long))

//#define ___meta_len_ptr(ptr, cap)   ((struct ___meta_len *)((void *)(ptr) + (cap) - ___meta_len_sz()))
//#define ___meta_used_ptr(ptr, cap) ((struct ___meta_used *)((void *)(ptr) + (cap) - ___meta_len_sz() - ___meta_used_sz(___meta_len_ptr(ptr, cap)->len & ~___HAS_USED_MASK)))

//#define ___meta_sz(ptr)	___meta_len_sz() + (___meta_has_used(ptr) ? ___meta_used_sz(___len(ptr)) : 0)

//#define ___meta_has_used(ptr)	(___meta_len_ptr(ptr)->len & ___HAS_USED_MASK)

//#define cap(ptr) ((ptr) ? ((___cap(ptr) - ___meta_len_sz(ptr)) / sizeof(*(ptr))) : 0)
//#define len(ptr) ((ptr) ? ___meta_len_ptr(ptr, ___cap(ptr))->len & ~___HAS_USED_MASK : 0)


static inline size_t ___get_meta(void *ptr, size_t **len_pptr, unsigned long **used_pptr)
{
	if (!ptr)
		return 0;
	size_t *len_ptr = ptr + ___cap(ptr) - ___meta_len_sz();
	size_t len = *len_ptr;

	if (len_pptr)
		*len_pptr = len_ptr;

	if (len & ___HAS_USED_MASK) {
		len &= ~___HAS_USED_MASK;
		if (used_pptr)
			*used_pptr = (void *)*len_ptr - ___meta_used_sz(len);
	}
	return len;
}

//set_meta_len
//set_meta_used

#define len(ptr) ___get_meta(ptr, NULL, NULL)

#define ___alloc_tbl(ptr, len) ({\
	size_t ___len = len;\
	ptr = malloc(___user_sz(ptr, ___len) + ___meta_used_sz(___len) + ___meta_len_sz());\
	size_t *___len_ptr; unsigned long *___used_ptr;\
	___get_meta(ptr, &___len_ptr, &___used_ptr);\
	*___len_ptr = ___len | ___HAS_USED_MASK;\
	memset(___used_ptr, 0, ___meta_used_sz(___len));\
})

#define ___used(ptr, slot) ({\
	unsigned long *___used_ptr;\
	___get_meta(ptr, NULL, &___used_ptr);\
	test_bit(slot, ptr);\
})

#define ___extend(ptr, len) ptr = realloc(ptr, ___align_sz(___user_sz(ptr, len) + ___meta_len_sz()))

#define append(pptr, ...) ({\
	size_t ___len = len(*(pptr));\
	___extend(*(pptr), ___len + ___narg(__VA_ARGS__));\
	___fill((*(pptr) + ___len), ##__VA_ARGS__);\
	size_t *___len_ptr;\
	___get_meta(*(pptr), &___len_ptr, NULL);\
	*___len_ptr = ___len + ___narg(__VA_ARGS__);\
})

#define ___fill_pr_fmt(arr, x)\
	_Generic(x,\
		 _Bool:                 ___fill_shift(arr, '%', 'd'),\
		 char:                  ___fill_shift(arr, '%', 'c'),\
		 signed char:           ___fill_shift(arr, '%', 'h', 'h', 'i'),\
		 unsigned char:         ___fill_shift(arr, '%', 'h', 'h', 'u'),\
		 signed short:          ___fill_shift(arr, '%', 'h', 'i'),\
		 unsigned short:        ___fill_shift(arr, '%', 'h', 'u'),\
		 signed int:            ___fill_shift(arr, '%', 'i'),\
		 unsigned int:          ___fill_shift(arr, '%', 'u'),\
		 signed long:           ___fill_shift(arr, '%', 'l', 'i'),\
		 unsigned long:         ___fill_shift(arr, '%', 'l', 'u'),\
		 signed long long:      ___fill_shift(arr, '%', 'l', 'l', 'i'),\
		 unsigned long long:    ___fill_shift(arr, '%', 'l', 'l', 'u'),\
		 float:                 ___fill_shift(arr, '%', 'f'),\
		 double:                ___fill_shift(arr, '%', 'f'),\
		 long double:           ___fill_shift(arr, '%', 'L', 'f'),\
		 char *:                ___fill_shift(arr, '%', 's'),\
		 const char *:          ___fill_shift(arr, '%', 's'),\
		 default:               ___fill_shift(arr, '%', 'p'))

#define ___fill_fmt0(arr, x)
#define ___fill_fmt1(arr, x) ___fill_pr_fmt(arr, x)
#define ___fill_fmt2(arr, x, ...) ___fill_pr_fmt(arr, x); ___fill_fmt1(arr, __VA_ARGS__)
#define ___fill_fmt3(arr, x, ...) ___fill_pr_fmt(arr, x); ___fill_fmt2(arr, __VA_ARGS__)
#define ___fill_fmt4(arr, x, ...) ___fill_pr_fmt(arr, x); ___fill_fmt3(arr, __VA_ARGS__)
#define ___fill_fmt5(arr, x, ...) ___fill_pr_fmt(arr, x); ___fill_fmt4(arr, __VA_ARGS__)
#define ___fill_fmt6(arr, x, ...) ___fill_pr_fmt(arr, x); ___fill_fmt5(arr, __VA_ARGS__)
#define ___fill_fmt7(arr, x, ...) ___fill_pr_fmt(arr, x); ___fill_fmt6(arr, __VA_ARGS__)
#define ___fill_fmt8(arr, x, ...) ___fill_pr_fmt(arr, x); ___fill_fmt7(arr, __VA_ARGS__)
#define ___fill_fmt9(arr, x, ...) ___fill_pr_fmt(arr, x); ___fill_fmt8(arr, __VA_ARGS__)
#define ___fill_fmt10(arr, x, ...) ___fill_pr_fmt(arr, x); ___fill_fmt9(arr, __VA_ARGS__)
#define ___fill_fmt11(arr, x, ...) ___fill_pr_fmt(arr, x); ___fill_fmt10(arr, __VA_ARGS__)
#define ___fill_fmt12(arr, x, ...) ___fill_pr_fmt(arr, x); ___fill_fmt11(arr, __VA_ARGS__)

#define ___fill_fmt(arr, ...)\
	___apply(___fill_fmt, ___narg(__VA_ARGS__))(arr, __VA_ARGS__)

#define fprint(fp, ...) ({\
	char ___fmt[___narg(__VA_ARGS__)*4 + 1];\
	char *___dst = ___fmt;\
	___fill_fmt(___dst, __VA_ARGS__);\
	*___dst++ = '\0';\
	fprintf(fp, ___fmt, ##__VA_ARGS__);\
})

#define print(...) fprint(stdout, ##__VA_ARGS__)

#define fprintln(fp, ...) ({\
	char ___fmt[___narg(__VA_ARGS__)*4 + 2];\
	char *___dst = ___fmt;\
	___fill_fmt(___dst, __VA_ARGS__);\
	*___dst++ = '\n';\
	*___dst++ = '\0';\
	fprintf(fp, ___fmt, ##__VA_ARGS__);\
})

#define println(...) fprintln(stdout, ##__VA_ARGS__)

#define join(...) ({\
	char ___fmt[___narg(__VA_ARGS__)*4 + 1];\
	char *___dst = ___fmt;\
	___fill_fmt(___dst, __VA_ARGS__);\
	*___dst++ = '\0';\
	size_t nb = snprintf(NULL, 0, ___fmt, __VA_ARGS__);\
	char *buf = malloc(nb + 1);\
	if (buf) sprintf(buf, ___fmt, __VA_ARGS__);\
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
	char *dup = strdup(str);\
	*(p) = ___strto(*(p), strtok(dup, delim));\
	___apply(___split, ___narg(p, __VA_ARGS__))(dup, delim, ##__VA_ARGS__);\
	free(dup);\
})

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

#endif

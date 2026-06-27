#include "helpers.h"

#define INITIAL_LEN	___nr_bits(unsigned long)

void *___reserve(void *old_ptr, size_t new_cap, size_t entry_sz)
{
	size_t old_len = len(old_ptr);
	if (!new_cap)
		new_cap = INITIAL_LEN;

	void *new_ptr = realloc(old_ptr, new_cap*entry_sz + sizeof(struct ___meta));
	if (!new_ptr)
		return NULL; /* ENOMEM */

	/* truncation requested */
	if (old_len > new_cap)
		old_len = new_cap;

	struct ___meta *meta = ___meta(new_ptr);
	meta->len = old_len;
	meta->ext = 0;

	return new_ptr;
}

void *___try_extend(void *old_ptr, size_t new_len, size_t entry_sz)
{
	size_t old_cap = old_ptr ? (___cap_sz(old_ptr) - sizeof(struct ___meta)) / entry_sz : 0;
	void *new_ptr = old_ptr;

	if (new_len > old_cap) {
		size_t new_cap = old_cap ? old_cap + old_cap/4 : 0;
		new_ptr = ___reserve(old_ptr, new_cap, entry_sz);
	}
	if (new_ptr)
		___meta(new_ptr)->len = new_len;

	return new_ptr;
}

static inline void *___reserve_ext(size_t cap, size_t entry_sz)
{
	void *ptr = malloc(entry_sz*cap + sizeof(struct ___meta) + ___inuse_sz(cap));
	if (!ptr)
		return NULL;
	struct ___meta *meta = ___meta(ptr);
	meta->len = cap;
	meta->ext = 1;
	__builtin_memset(___inuse_bits(meta), 0, ___inuse_sz(cap));
	return ptr;
}

static ssize_t ___try_insert(void *ptr, struct ___entry_meta *meta, void *entry,
			     void *key_ptr, bool update)
{
	size_t cap = len(ptr);
	if (!cap)
		return -1; /* ENOSPC */;

	ssize_t slot = meta->hash(___key(entry), meta->key_sz) % cap;
	ssize_t end = (slot + cap/2) % cap; /* force rehash */

	do {
		if (!___inuse_test(ptr, slot)) {
			__builtin_memcpy(___entry(ptr, slot), entry, meta->entry_sz);
			___inuse_set(ptr, slot);
			return slot;
		} else if (key_ptr && meta->cmp(___key(___entry(ptr, slot)),
						key_ptr, meta->key_sz) == 0) {
			if (update)
				__builtin_memcpy(___entry(ptr, slot), entry, meta->entry_sz);
			/* EEXIST */
			return slot;
		}
		slot = (slot + 1) % cap;
	} while (slot != end);

	return -1; /* ENOSPC */
}

void *___rehash(void *old_ptr, struct ___entry_meta *meta, size_t new_cap)
{
	size_t old_len = len(old_ptr);
	if (!new_cap)
		new_cap = INITIAL_LEN;

	void *new_ptr = ___reserve_ext(new_cap, meta->entry_sz);
	if (!new_ptr)
		return NULL; /* ENOMEM */

	for (ssize_t slot = 0; slot < old_len; slot++) {
		if (___inuse_test(old_ptr, slot)) {
			ssize_t ret = ___try_insert(new_ptr, meta, ___entry(old_ptr, slot), NULL, false);
			if (ret == -1) {
				free(new_ptr);
				return old_ptr; /* ENOSPC */
			}
		}
	}
	free(old_ptr);
	return new_ptr;
}

ssize_t ___insert(void **pptr, struct ___entry_meta *meta,
		  void *entry, void *key_ptr, bool update)
{
	void *ptr = *pptr;
	do {
		ssize_t slot = ___try_insert(ptr, meta, entry, key_ptr, update);
		if (slot >= 0)
			return slot;

		int factor = 2;
		do {
			ptr = ___rehash(ptr, meta, factor * len(ptr));
			factor++;
		} while (ptr == *pptr);

		if (ptr)
			*pptr = ptr;
	} while (ptr);

	return -1; /* ENOMEM */
}

static void ___shift_cluster(void *ptr, struct ___entry_meta *meta, ssize_t empty)
{
	size_t cap = len(ptr);
	ssize_t slot = empty;
	ssize_t end = empty;

	do {
		slot = (slot + 1) % cap;
		if (!___inuse_test(ptr, slot))
			break;

		ssize_t pos = meta->hash(___key(___entry(ptr, slot)), meta->key_sz) % cap;
		if (empty <= slot) {
			if (empty < pos && pos <= slot)
				continue;
		} else {
			if (pos <= slot || empty < pos)
				continue;
		}

		___inuse_set(ptr, empty);
		__builtin_memcpy(___entry(ptr, empty), ___entry(ptr, slot), meta->entry_sz);
		___inuse_clear(ptr, slot);
		empty = slot;

	} while (slot != end);
}

static ssize_t ___get_slot(void *ptr, size_t entry_sz, void *value_ptr)
{
	size_t cap = len(ptr);
	ssize_t slot = ((size_t)value_ptr - (size_t)ptr) / entry_sz;

	if (slot < 0 || slot >= cap)
		return -1; /* EINVAL */
	if (!___inuse_test(ptr, slot))
		return -1; /* EFAULT */

	return slot;
}

bool ___delete(void **pptr, struct ___entry_meta *meta, void *value_ptr)
{
	void *ptr = *pptr;
	ssize_t slot = ___get_slot(ptr, meta->entry_sz, value_ptr);
	if (slot == -1)
		return false;

	___inuse_clear(ptr, slot);
	___shift_cluster(ptr, meta, slot);

	return true;
}

ssize_t ___lookups = 0;
ssize_t ___lookup_probes = 0;

char *___get_tok(const char *str, const char *sep, const char **next)
{
	size_t tok_len;
	size_t sep_len = __builtin_strlen(sep);

	if (!str)
		str = *next;
	if (!str)
		return NULL;

	char *next_sep = __builtin_strstr(str, sep);
	if (next_sep) {
		tok_len = next_sep - str;
		*next = next_sep + sep_len;
	} else {
		tok_len = __builtin_strlen(str);
		*next = NULL;
	}
	char *tok = malloc(tok_len + 1);
	__builtin_memcpy(tok, str, tok_len);
	tok[tok_len] = '\0';
	return tok;
}

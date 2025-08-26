#include "helpers.h"


static inline unsigned xrand()
{
	static unsigned seed = 31421;
	seed = 16807*(seed%127773) - 2836*(seed/127773);
	return seed;
}

int *keys = NULL;
mapof(int, int) *ptr = NULL, *prev;

void add(mapof(int, int) **pptr)
{
	int key = xrand() & 0xffff;
	int data = xrand() & 0xfff;
	void *prev = *pptr;

	println("insert: key=", key, " data=", data);
	insert(pptr, key, data) ;
	if (prev != *pptr)
		println("rehash: ", len(*pptr));

	append(&keys, key);
}

void find(int key, mapof(int, int) **pptr)
{
	int *data = lookup(pptr, key);
	if (data)
		println("lookup: key=", key, " data=", *data);
	else
		println("lookup: key=", key, " not found");
}

int xxx_pair(mapof(int, char *) *xpair)
{
	return xpair->key;
}


void del(int key)
{
	int *data = lookup(&ptr, key);
	if (!data) {
		println("delete: key=", key, " not found");
		return;
	}
	delete(&ptr, data);
	find(key, &ptr);
}

#define foreach1(ref, ptr, len) \
	for (typeof(ptr) (ref) = (ptr); (ref) < (ptr) + len; (ref)++) \

#define foreach(ref, ptr) \
	for (auto (ref) = (ptr); (ref) < (ptr) + len(ptr); (ref)++) \
	if (used(ptr, (ref) - (ptr)))

void walk()
{
	println("len=", len(ptr));
	foreach(ref, ptr)
		println("slot=", ref - ptr, " key=", ref->key, " data=", ref->value);
}

int main()
{
	println("ALIGN(1)=", ALIGN(1, sizeof(unsigned long)));
	println("ALIGN_DOWN(1)=", ALIGN_DOWN(1, sizeof(unsigned long)));
	println("ALIGN_DOWN(", sizeof(unsigned long), ")=", ALIGN_DOWN(sizeof(unsigned long), sizeof(unsigned long)));
	println("ALIGN_DOWN(40)=", ALIGN_DOWN(40, sizeof(unsigned long)));

	find(7, &ptr);

	insert(&ptr, 1, 11);
	insert(&ptr, 8, 18);
	insert(&ptr, 7, 17);
	insert(&ptr, 4, 14);
	insert(&ptr, 9, 19);
	insert(&ptr, 11, 111);
	walk();

	reserve(&keys, 64);
	reserve(&ptr, 64, true);

	insert(&ptr, 7, 17);

	for (int i = 0; i < 1000; i++) add(&ptr);
	walk();

	___lookup_probes = 0;
	foreach(it, keys) find(*it, &ptr);
	println("probes per lookup: ", ___lookup_probes/(double)len(keys));

	del(7);

	return 0;
}

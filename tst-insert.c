#include "helpers.h"

mapof(int, int) *ptr = NULL, *prev;
int *keys = NULL;

static inline unsigned xrand()
{
	static unsigned seed = 31421;
	seed = 16807*(seed%127773) - 2836*(seed/127773);
	return seed;
}

void add()
{
	int key = xrand() & 0xffff;
	int data = xrand() & 0xfff;
	void *prev = ptr;

	println("insert: key=", key, " data=", data);
	insert(&ptr, key, data) ;
	if (prev != ptr)
		println("rehash: ", len(ptr));

	append(&keys, key);
}

void find(int key)
{
	int *data = lookup(&ptr, key);
	if (data)
		println("lookup: key=", key, " data=", *data);
	else
		println("lookup: key=", key, " not found");
}

void del(int key)
{
	int *data = lookup(&ptr, key);
	if (!data) {
		println("delete: key=", key, " not found");
		return;
	}
	delete(&ptr, data);
	find(key);
}

void walk()
{
	println("len=", len(ptr));
	foreach (ref, ptr)
		println("slot=", ref - ptr, " key=", ref->key, " data=", ref->value);
}

int main()
{
	println("ALIGN(1)=", ___align(1, sizeof(unsigned long)));
	println("ALIGN_DOWN(1)=", ___align_down(1, sizeof(unsigned long)));
	println("ALIGN_DOWN(", sizeof(unsigned long), ")=", ___align_down(sizeof(unsigned long), sizeof(unsigned long)));
	println("ALIGN_DOWN(40)=", ___align_down(40, sizeof(unsigned long)));

	find(7);

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

	for (int i = 0; i < 1000; i++) add();
	walk();

	___lookup_probes = 0;
	foreach (it, keys) find(*it);
	println("probes per lookup: ", ___lookup_probes/(double)len(keys));

	del(7);

	return 0;
}

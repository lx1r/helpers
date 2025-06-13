#include "helpers.h"

mapof(int, int) *ptr = NULL, *prev;
int *keys = NULL;

void add()
{
	int key = rand() & 0xffff;
	int data = rand() & 0xfff;
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

#define foreach1(ref, ptr, len) \
	for (typeof(ptr) (ref) = (ptr); (ref) < (ptr) + len; (ref)++) \

#define foreach(ref, ptr) \
	for (typeof(ptr) (ref) = (ptr); (ref) < (ptr) + len(ptr); (ref)++) \
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

	foreach(it, keys) find(*it);

	del(7);

	return 0;
}

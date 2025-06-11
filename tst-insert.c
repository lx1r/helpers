#include "helpers.h"

mapof(int, int) *pd = NULL;

void walk()
{
	println("len=", len(pd));
	for (int i = 0; i < len(pd); i++)
		if (used(pd, i))
			println("slot=", i, " key=", pd[i].key, " data=", pd[i].value);
}

void find(int key)
{
	int *data = lookup(&pd, key);
	if (data)
		println("found key=", key, " data=", *data);
	else
		println("key=", key, " not found");
}

int main()
{
	find(7);

	insert(&pd, 1, 1);
	insert(&pd, 8, 8);
	insert(&pd, 7, 7);
	insert(&pd, 4, 4);
	insert(&pd, 9, 9);
	insert(&pd, 19, 19);

	walk();
	find(7);

	insert(&pd, 12, 12);
	insert(&pd, 83, 83);
	insert(&pd, 74, 74);
	insert(&pd, 34, 34);
	insert(&pd, 29, 29);
	insert(&pd, 219, 219);
	insert(&pd, 432, 432);
	insert(&pd, 56, 56);
	insert(&pd, 233, 233);
	insert(&pd, 76, 76);
	insert(&pd, 32, 32);
	insert(&pd, 412, 412);
	insert(&pd, 453, 453);
	insert(&pd, 432, 432);
	insert(&pd, 342, 342);
	insert(&pd, 324, 324);

	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	insert(&pd, 23, 99);
	walk();

	find(1);
	find(8);
	find(7);
	find(4);
	find(9);
	find(19);

	find(12);
        find(83);
        find(74);
        find(34);
        find(29);
        find(219);
        find(432);
        find(56);
        find(233);
        find(76);
        find(32);
        find(412);
        find(453);
        find(432);
        find(342);
        find(324);
        find(11111);

	int *data = lookup(&pd, 7);
	delete(&pd, data);
	find(7);

	return 0;
}

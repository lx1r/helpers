#include "helpers.h"

mapof(int, int) *pd = NULL;

void walk()
{
	println("len=", len(pd));
	for (int i = 0; i < len(pd); i++)
		if (used(pd, i))
			println("slot=", i, " key=", pd[i].key, " data=", pd[i].data);
}

void find(int key)
{
	ssize_t slot = lookup(&pd, key);
	println("found key=", key, " slot=", slot);
}

int main()
{
	insert(&pd, 1, 11);
	insert(&pd, 8, 18);
	insert(&pd, 7, 17);
	insert(&pd, 4, 14);
	insert(&pd, 9, 19);
	insert(&pd, 19, 99);

	walk();
	find(7);

	insert(&pd, 12, 11);
	insert(&pd, 83, 18);
	insert(&pd, 74, 17);
	insert(&pd, 34, 14);
	insert(&pd, 29, 19);
	insert(&pd, 219, 99);
	insert(&pd, 432, 99);
	insert(&pd, 56, 99);
	insert(&pd, 233, 99);
	insert(&pd, 76, 99);
	insert(&pd, 23, 99);
	insert(&pd, 32, 99);
	insert(&pd, 412, 99);
	insert(&pd, 4534532, 99);
	insert(&pd, 432342, 99);
	insert(&pd, 34265, 99);
	insert(&pd, 32442, 99);

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
        find(23);
        find(32);
        find(412);
        find(4534532);
        find(432342);
        find(34265);
        find(32442);
        find(11111);

	return 0;
}

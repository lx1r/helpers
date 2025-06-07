#include "helpers.h"


int main()
{
	mapof(int, int) *pd = NULL;

	insert(&pd, 1, 11);
	insert(&pd, 8, 18);
	insert(&pd, 7, 17);
	insert(&pd, 4, 14);
	insert(&pd, 9, 19);
	insert(&pd, 19, 99);

	println("len=", len(pd));
	for (int i = 0; i < len(pd); i++)
		if (used(pd, i))
			println(i, ": ", pd[i].key, ":", pd[i].data);

	println("find");
	ssize_t slot = lookup(&pd, 7);
	println("7: slot=", slot);

	return 0;
}

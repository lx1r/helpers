#include "helpers.h"

typedef entry(const char *, int *) multi_map_t;

int *insert_multi(multi_map_t **mp, const char *key, int val)
{
	int **lt = insert(mp, key, NULL);
	append(lt, val);
	return *lt;
}

int main(void)
{
	multi_map_t *map = NULL;

	insert_multi(&map, "hi", 1);
	insert_multi(&map, "hi", 4);
	insert_multi(&map, "hi", 7);
	insert_multi(&map, "low", 20);
	insert_multi(&map, "low", 25);
	insert_multi(&map, "medium", 11);
	insert_multi(&map, "medium", 14);

	foreach (m, map)
		foreach (v, m->value)
			println("key=", m->key, " val=", *v);

	return 0;
}

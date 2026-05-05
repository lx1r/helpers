#include "helpers.h"

typedef entry(const char * /* inner key */, int  /* inner val */) inner_map_t;
typedef entry(const char * /* inner map name */, inner_map_t *) map_t;

int *insert_inner(map_t **map, const char *name, const char *key, int val)
{
	inner_map_t **inner_map = insert(map, name, NULL);
	return insert(inner_map, key, val);
}

unsigned long lookup_inner(map_t **map, const char *name, const char *key)
{
	inner_map_t **inner_map = lookup(map, name);
	if (inner_map) {
		int *ref = lookup(inner_map, key);
		if (ref) {
			println("found ", name, ":", key, ":", *ref);
			return *ref;
		}
	}
	println("no ", name, ":", key);
	return -1;
}

int main(void)
{
	map_t *map = NULL;

	insert_inner(&map, "hi", "timer", 1);
	insert_inner(&map, "hi", "net", 4);
	insert_inner(&map, "hi", "sched", 7);
	insert_inner(&map, "low", "timer", 20);
	insert_inner(&map, "low", "sched", 25);
	insert_inner(&map, "medium", "net", 11);
	insert_inner(&map, "medium", "rcu", 14);

	println("main:", count(map));
	foreach (mp, map) {
		println(mp->key, ":", count(mp->value));
		foreach (in, mp->value)
			println(mp->key, ":", in->key, ":", in->value);
	}

	lookup_inner(&map, "none", "none");

	foreach (mp, map)
		lookup_inner(&map, mp->key, "none");

	lookup_inner(&map, "hi", "timer");
	lookup_inner(&map, "low", "sched");
	lookup_inner(&map, "medium", "net");

	return 0;
}

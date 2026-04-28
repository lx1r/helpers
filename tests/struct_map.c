#include "helpers.h"

struct key {
	int id;
	int type;
};

struct val {
	int data;
	int config;
};

int main()
{
	entry(struct key, struct val) *map = NULL;

	for (int i = 0; i < 20; i++)
		insert(&map, {i, 10 + i}, {10 + i, 100 + i});

	println("count=", count(map));
	foreach (ref, map)
		println("slot=", ref - map,
			" key=", ref->key.id, ":", ref->key.type,
			" val=", ref->value.data, ":", ref->value.config);

	struct val *val = lookup(&map, {1, 11});
	if (val) {
		struct key *key = keyof(map, val);
		println("found key=", key->id, ":", key->type,
			" val=", val->data, ":", val->config);
	}

	return 0;
}

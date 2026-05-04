#include "helpers.h"

struct key {
	short id;
	int type;
};

struct val {
	int conf;
	float data;
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
			" val=", ref->value.conf, ":", ref->value.data);

	struct val *val = lookup(&map, {1, 11});
	if (val) {
		struct key *key = keyof(map, val);
		println("found key=", key->id, ":", key->type,
			" val=", val->conf, ":", val->data);
	}

	return 0;
}

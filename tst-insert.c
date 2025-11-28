#include "helpers.h"

pair(int, int) *ptr = NULL, *prev;
int *keys = NULL;

static inline unsigned xrand()
{
	static unsigned seed = 31421;
	seed = 16807*(seed%127773) - 2836*(seed/127773);
	return seed;
}

void add_rand()
{
	int key = xrand() & 0xffff;
	int data = xrand() & 0xfff;
	void *prev = ptr;
	int *data_ref;

	println("insert: key=", key, " data=", data);
	data_ref = insert(&ptr, key, data);
	if (*data_ref != data)
		println("add fail: ", *data_ref, "!=", data);
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

void del_rand(int key)
{
	int *data = lookup(&ptr, key);
	if (!data) {
		println("delete: key=", key, " not found");
		return;
	}
	delete(&ptr, data);
	find(key);
}

void walk_rand()
{
	println("len=", len(ptr));
	foreach (ref, ptr)
		println("slot=", ref - ptr, " key=", ref->key, " data=", ref->value);
}

pair(char *, int) *months = NULL;

void walk_months()
{
	println("months: len=", count(months), " cap=", len(months));
	foreach (month, months)
		println(" ", month->key, ": ", month->value);
}

int main()
{
	find(7);

	insert(&ptr, 1, 11);
	insert(&ptr, 8, 18);
	insert(&ptr, 7, 17);
	insert(&ptr, 4, 14);
	insert(&ptr, 9, 19);
	insert(&ptr, 11, 111);
	walk_rand();

	size_t meta = *(size_t *)___meta(ptr);
	println("meta=", (void*)meta);
	println("len=", (void*)len(ptr));

	reserve(&keys, 64);
	reserve(&ptr, 64, true);

	insert(&ptr, 7, 17);

	for (int i = 0; i < 1000; i++) add_rand();
	walk_rand();

	___lookup_probes = 0;
	foreach (it, keys) find(*it);
	println("probes per lookup: ", ___lookup_probes/(double)___lookups);

	del_rand(7);
	foreach (it, keys) if ((xrand() & 0xf) == 1) del_rand(*it);
	walk_rand();


	println("initial months");
	insert(&months, "january", 1);
	insert(&months, "february", 2);
	insert(&months, "march", 3);
	insert(&months, "april", 4);
	insert(&months, "may", 5);
	insert(&months, "june", 6);
	insert(&months, "july", 7);
	insert(&months, "august", 8);
	insert(&months, "september", 9);
	insert(&months, "october", 10);
	insert(&months, "november", 11);
	insert(&months, "december", 12);
#if 0
	insert(&months, "1dember", 12);
	insert(&months, "3dsasember", 12);
	insert(&months, "2dsasember", 12);
	insert(&months, "1dsasember", 12);
	insert(&months, "2dsaseamber", 12);
	insert(&months, "as2dsasember", 12);
	insert(&months, "wdsasember", 12);
	insert(&months, "321dsasember", 12);
	insert(&months, "sadssasember", 12);
	insert(&months, "sdadsasember", 12);
	insert(&months, "5trdasasember", 12);
	insert(&months, "4354dsasember", 12);
	insert(&months, "redre453sasember", 12);
	insert(&months, "61de3mbuer", 12);
	insert(&months, "51de2mbeyur", 12);
	insert(&months, "f1de1mber", 12);
	insert(&months, "gf1dember", 12);
	insert(&months, "h1hde2mber", 12);
	insert(&months, "51de2muyber", 12);
	insert(&months, "7h1desmber", 12);
	insert(&months, "yy1deumsber", 12);
	insert(&months, "j1dsember", 12);
	insert(&months, "gv1desmber", 12);
	insert(&months, "fsgdasfdasdfa", 12);
	insert(&months, "afsddfsa", 12);
	insert(&months, "rqrqew", 12);
	insert(&months, "yuj", 12);
	insert(&months, "rewrew", 12);
	insert(&months, "e4r", 12);
	insert(&months, "fgeer", 12);
	insert(&months, "453gf", 12);
	insert(&months, "jbhhvgfchg", 12);
	insert(&months, "786fghcgv", 12);
	insert(&months, "hfcgfxgc", 12);
	insert(&months, "jghhgj", 12);
#endif
	walk_months();

	int *month;
	if ((month = lookup(&months, "august"))) println("found: ", *month);
	else println("node not found");
	if ((month = lookup(&months, "trillium"))) println("found: ", *month);
	else println("node not found");

	println("del except: october, november, december");
	delete(&months, lookup(&months, "january"));
	delete(&months, lookup(&months, "february"));
	delete(&months, lookup(&months, "march"));
	delete(&months, lookup(&months, "april"));
	delete(&months, lookup(&months, "may"));
	delete(&months, lookup(&months, "june"));
	delete(&months, lookup(&months, "july"));
	delete(&months, lookup(&months, "august"));
	delete(&months, lookup(&months, "september"));
	walk_months();
	println("add trillium");
	insert(&months, "trillium", 14);
	walk_months();

	return 0;
}

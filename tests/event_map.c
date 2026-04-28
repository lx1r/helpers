#include "helpers.h"

int *event_list = NULL;
entry(int, int) *event_map = NULL;

static inline int event(void)
{
	static unsigned seed = 31421;
	seed = 16807*(seed%127773) - 2836*(seed/127773);
	return seed & 0xffff;
}

void dump_events(void)
{
	println("len=", len(event_map));
	foreach (ref, event_map)
		println("slot=", ref - event_map, " ev=", ref->key, " count=", ref->value);
}

void register_event(void)
{
	int ev = event();
	static int count = 0;

	int *data = insert(&event_map, ev, count);
	if (!data) {
		println("event ", ev, " already registered");
		return;
	}
	println("registered event ", ev, " count=", *data);
	append(&event_list, ev);
	count++;
}

void unregister_event(int ev)
{
	int *data = lookup(&event_map, ev);
	if (!data) {
		println("event ", ev, " not registered");
		dump_events();
		return;
	}
	int count = *data;
	delete(&event_map, data);
	println("unregistered event ", ev, " count=", count);
}

void find_event(int ev)
{
	int *data = lookup(&event_map, ev);
	if (!data) {
		println("event ", ev, " not found");
		dump_events();
		return;
	}
	println("found event ", ev, " count=", *data);
}

void sanity_test(void)
{
	void *lookup_ret = lookup(&event_map, 11);
	println("lookup ret: ", lookup_ret);
	ssize_t delete_ret = delete(&event_map, event_map + 11);
	println("delete ret: ", delete_ret);
}

int main()
{
	sanity_test();
	resize(&event_list, 400);
	rehash(&event_map, 400);
	sanity_test();

	for (int i = 0; i < 10000; i++)
		register_event();

	for (int i = 0; i < len(event_list); i++)
		find_event(event_list[i]);

	for (int i = 0; i < len(event_list)/8; i++)
		unregister_event(event_list[i]);

	for (int i = len(event_list)/8; i < len(event_list); i++)
		find_event(event_list[i]);

	println("len(event_map)=", len(event_map));
	println("count(event_map)=", count(event_map));
	println("len(event_list)=", len(event_list));

	return 0;
}

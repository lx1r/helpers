#include "helpers.h"

typedef entry(const char * /* event name */, unsigned long /* event id */) event_map;
entry(const char * /* provider name */, event_map *) *providers = NULL;

int add_event_list(const char *provider, size_t len, const char *list[len])
{
	event_map **evs = insert(&providers, provider, NULL);
	if (!evs)
		return -1;

	for (size_t i = 0 ; i < len; i++)
		insert(evs, list[i], i);

	return 0;
}

unsigned long get_event_id(const char *provider, const char *event)
{
	event_map **evs = lookup(&providers, provider);
	if (evs) {
		unsigned long *cf = lookup(evs, event);
		if (cf) {
			println("found ", provider, ":", event, ":", *cf);
			return *cf;
		}
	}
	println("no ", provider, ":", event);
	return -1;
}

static const char *hardirq_list[] = {
	"reschedule",
	"call_func",
	"cpu_stop",
	"cpu_crash_stop",
	"timer",
	"irq_work",
	"wakeup",
};

static const char *softirq_list[] = {
	"hi",
	"timer",
	"net_tx",
	"net_rx",
	"block",
	"irq_poll",
	"tasklet",
	"sched",
	"hrtimer",
	"rcu",
};

int main(void)
{
	add_event_list("hardirq", len(hardirq_list), hardirq_list);
	add_event_list("softirq", len(softirq_list), softirq_list);

	get_event_id("none", "none");

	foreach (ev, providers)
		get_event_id(ev->key, "none");

	foreach (np, hardirq_list)
		get_event_id("hardirq", *np);

	foreach (np, softirq_list)
		get_event_id("softirq", *np);

	return 0;
}

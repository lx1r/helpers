#include "helpers.h"

typedef entry(const char * /*event*/, unsigned long /*config*/) events;
entry(const char * /*provider*/, events *) *providers = NULL;

static int add_event_list(const char *probe, size_t len, const char *list[len])
{
    events **evs = insert(&providers, strdup(probe), NULL);
    if (!evs)
        return -1;

    for (size_t i = 0 ; i < len; i++)
        if (list[i])
            insert(evs, list[i], i);
    return 0;
}

unsigned long get_event_conf(const char *provider, const char *event)
{
        events **evs = lookup(&providers, provider);
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

static const char *syscall_list[] = {
        "open",
        "link",
        "unlink",
        "mknod",
        "chmod",
        "chown",
        "mkdir",
        "rmdir",
        "chown",
        "access",
        "rename",
};

int main(void)
{
    add_event_list("hardirq", len(hardirq_list), hardirq_list);
    add_event_list("softirq", len(softirq_list), softirq_list);
    add_event_list("syscall", len(syscall_list), syscall_list);

    get_event_conf("none", "none");

    foreach (ev, providers)
        get_event_conf(ev->key, "none");
    foreach (np, softirq_list)
        get_event_conf("softirq", *np);
    foreach (np, hardirq_list)
        get_event_conf("hardirq", *np);
    foreach (np, hardirq_list)
        get_event_conf("syscall", *np);
    foreach (np, syscall_list)
        get_event_conf("syscall", *np);

    return 0;
}

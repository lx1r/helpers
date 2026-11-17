#include "helpers.h"

struct person {
	char *name;
	int born;
};

void persons_cleanup(struct person **persons_ptr)
{
	foreach (p, *persons_ptr) {
		free(p->name);
	free(*persons_ptr);
	*persons_ptr = NULL;
}

int main()
{
	struct person ptr(swp, persons_cleanup) = NULL;

	append(&swp, {join("Mark", " ", "Hamill"), 1951});
	append(&swp, {join("Harrison", " ", "Ford"), 1942});
	append(&swp, {join("Carrie", " ", "Fisher"), 1956});
	append(&swp, {join("Peter", " ", "Cushing"), 1913});
	append(&swp, {join("Alec", " ", "Guinness"), 1914});

	foreach (p, swp)
		println(p->name, ", ", p->born);
	println("Total: ", len(swp));

	return 0;
}

#include "helpers.h"

struct person {
	char *name;
	char *surname;
	int born;
};

int main()
{
	struct person *swp = NULL;

	append(&swp, {"Mark", "Hamill", 1951});
	append(&swp, {"Harrison", "Ford", 1942});
	append(&swp, {"Carrie", "Fisher", 1956});
	append(&swp, {"Peter", "Cushing", 1913});
	append(&swp, {"Alec", "Guinness", 1914});

	foreach (p, swp)
		println(p->name, " ", p->surname, ", ", p->born);
	println("Total: ", len(swp));

	return 0;
}

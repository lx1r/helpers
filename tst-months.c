#include "helpers.h"

typedef pair(char *, int) months;

void dump_months(months *months)
{
	foreach (month, months)
		println(month->key, ": ", month->value);
}

int main()
{
	months *months = NULL;

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
	dump_months(months);

	int *month;
	if ((month = lookup(&months, "august")))
		println("found: ", *month);
	else
		println("node not found");

	delete(&months, lookup(&months, "january"));
	delete(&months, lookup(&months, "february"));
	delete(&months, lookup(&months, "march"));
	delete(&months, lookup(&months, "april"));
	delete(&months, lookup(&months, "may"));
	delete(&months, lookup(&months, "june"));
	delete(&months, lookup(&months, "july"));
	delete(&months, lookup(&months, "august"));
	delete(&months, lookup(&months, "september"));
	insert(&months, "trillium", 14);
	dump_months(months);

	if ((month = lookup(&months, "trillium")))
		println("found: ", *month);
	else
		println("node not found");

	return 0;
}

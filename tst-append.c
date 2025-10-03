#include "helpers.h"

struct unit {
	int id;
	int data1;
	int data2;
	int data3;
};

int main()
{
	unsigned long *nums = NULL;

	append(&nums, 1);
	append(&nums, 2);
	append(&nums, 3);
	append(&nums, 4);
	append(&nums, 5);
	append(&nums, 6);
	append(&nums, 7);
	append(&nums, 8);
	ssize_t ret = append(&nums, 11);
	println("ret=", ret);

	print("len=", len(nums), ": ");
	printv(",", nums);
	println();

	char *str = joinv(",", nums);
	println("str=", str);
	println("str=", joinv(",", nums, len(nums) - 1));
	println("str=", joinv("--", nums, len(nums) - 2));

	struct unit *units = NULL;
	append(&units, {0});
	append(&units, {1, 11});
	append(&units, {2, 22, 222});
	append(&units, {3, 33, 333, 3333});

	print("len=", len(units), ":");
	foreach (unit, units)
		print(" ", unit->data1);
	println();

	return 0;
}

#include "helpers.h"

static void walk(int *nums)
{
	print("len=", len(nums), ":");
	for (int i = 0; i < len(nums); i++)
		print(" ", nums[i]);
	println();
}

int main()
{
	int *nums = NULL;

	append(&nums, 1);
	append(&nums, 2, 3);
	append(&nums, 4, 5, 6);
	append(&nums, 7, 8, 9, 10);
	append(&nums, 11, 12, 13, 14, 15);
	append(&nums, 16, 17, 18, 19, 20, 21);
	append(&nums, 22, 23, 24, 25, 26, 27, 28);
	append(&nums, 29, 30, 31, 32, 33, 34, 35, 36);

	walk(nums);

	return 0;
}

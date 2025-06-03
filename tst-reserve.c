#include "helpers.h"

int main()
{
	println("malloc(1): ", malloc_usable_size(malloc(1)));
	println("malloc(32): ", malloc_usable_size(malloc(32)));
	println("ALIGN(1)=", ALIGN(1, sizeof(unsigned long)));
	println("ALIGN_DOWN(1)=", ALIGN_DOWN(1, sizeof(unsigned long)));
	println("ALIGN_DOWN(", sizeof(unsigned long), ")=", ALIGN_DOWN(sizeof(unsigned long), sizeof(unsigned long)));
	println("ALIGN_DOWN(40)=", ALIGN_DOWN(40, sizeof(unsigned long)));

	int *nums;
	reserve(&nums, 20);
	println("len(nums)=", len(nums));

	for (int i = 0; i < len(nums); i++) {
		if (i % 2) {
			print(i, " ");
			___meta_used_set(nums, i);
		}
		nums[i] = i;
	}
	println();
	for (int i = 0; i < len(nums); i++) {
		if (___meta_used_test(nums, i))
			print(i, " ");
	}
	println();

	return 0;
}

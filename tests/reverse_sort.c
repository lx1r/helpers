#include "helpers.h"

int main()
{
	int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

	foreach (a, arr)
		print(*a, " ");
	println();

	/* reverse sort */
	qsort(arr, len(arr), sizeof(*arr),
	      func((const void *lhs, const void *rhs),
		   *(int*)rhs - *(int*)lhs));

	foreach (a, arr)
		print(*a, " ");
	println();

	return 0;
}

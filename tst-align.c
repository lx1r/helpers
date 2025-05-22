#include "helpers.h"

int main()
{
	println("malloc(1): ", malloc_usable_size(malloc(1)));
	println("malloc(32): ", malloc_usable_size(malloc(32)));
	println("ALIGN(1)=", ALIGN(1, sizeof(unsigned long)));
	println("ALIGN_DOWN(1)=", ALIGN_DOWN(1, sizeof(unsigned long)));
	println("ALIGN_DOWN(", sizeof(unsigned long), ")=", ALIGN_DOWN(sizeof(unsigned long), sizeof(unsigned long)));
	println("ALIGN_DOWN(40)=", ALIGN_DOWN(40, sizeof(unsigned long)));

	return 0;
}

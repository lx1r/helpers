#include "helpers.h"

int main()
{
	int lx[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        foreach (e, lx) println(*e);
        size_t res = reduce(func((size_t a, int b), a + b), lx);
        println("res=", res);

        int *ly = map(func((int a), a + 10), lx);
        foreach (e, ly) println(*e);
        println("cnt=", count(ly));

        int *lz = filter(func((int a), a % 2 == 0), ly);
        reduce(func((size_t a, int b), a + println(b)), lz);
        println("cnt=", count(lz));

	return 0;
}

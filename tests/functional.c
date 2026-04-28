#include "helpers.h"

#define map(fn, lt) ({\
	___typeof_ref(lt) ret_ = NULL;\
	foreach (ref, lt) append(&ret_, fn(*ref));\
	ret_;\
})

#define filter(fn, lt) ({\
	typeof(lt) ret_ = NULL;\
	foreach (ref, lt) if (fn(*ref)) append(&ret_, *ref);\
	ret_;\
})

#define reduce(fn, lt) ({\
	typeof(fn(0, lt[0])) ret_ = 0;\
	foreach (ref, lt) ret_ = fn(ret_, *ref);\
	ret_;\
})

int main()
{
	int lx[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

	foreach (e, lx)
		print(*e, " ");
	println();

	size_t res = reduce(func((size_t a, int b), a + b), lx);
	println("sum=", res);

	int *ly = map(func((int a), a + 10), lx);
	foreach (e, ly)
		print(*e, " ");
	println();

	int *lz = filter(func((int a), (a % 2) == 0), ly);
	foreach (e, lz)
		print(*e, " ");
	println();

	return 0;
}

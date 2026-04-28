#include "helpers.h"

int main()
{
	char *sep = "//";

	char _ptr path = join("path", sep, "to", sep, "dir");
	println(path);

	char *dirs[3];
	split(path, sep, &dirs[0], &dirs[1], &dirs[2]);

	foreach (dir, dirs)
		print(*dir, "  ");
	println("");

	foreach (dir, dirs)
		free(*dir);

	char _ptr path2 = joinv(sep, dirs);
	println(path2);

	char _vptr ddir = NULL;
	splitv(path2, sep, &ddir);

	foreach (dir, ddir)
		print(*dir, "  ");
	println("");

	return 0;
}

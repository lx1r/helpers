#include "helpers.h"

int main()
{
	char *sep = "//";

	char *path = join("path", sep, "to", sep, "dir");
	println(path);

	char *dirs[3];
	split(path, sep, &dirs[0], &dirs[1], &dirs[2]);

	foreach (dir, dirs)
		print(*dir, "  ");
	println("");

	foreach (dir, dirs)
		free(*dir);
	free(path);

	path = joinv(sep, dirs);
	println(path);

	char **ddir = NULL;
	splitv(path, sep, &ddir);

	foreach (dir, ddir)
		print(*dir, "  ");
	println("");

	vfree(ddir);

	return 0;
}

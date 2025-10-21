#include "helpers.h"

void test_vstr(char *str, char *sep)
{
	char **tokens = NULL;

	splitv(str, sep, &tokens);
	for (int i = 0; i < len(tokens); i++)
		println("tokens[", i, "]=", tokens[i]);
	char *res = joinv(":_-_:", tokens);
	println("sep=", sep);
	println("str=", str);
	println("res=", res);

	const char *next;
	for (char *s = ___get_tok(str, sep, &next); s; s = ___get_tok(NULL, sep, &next))
		printf("sub=%s\n", s);
	println();
}

void test_split3(char *str, char *sep)
{
	char *dir[3];

	println("sep=", sep);
	println("str=", str);
	split(str, sep, &dir[0], &dir[1], &dir[2]);

	foreach (d, dir)
		println("dir=", *d);
	println();
}

int main()
{
	char *sep;

	sep = "$token-comma$";
	test_vstr("first$token-comma$middle$token-comma$last", sep);
	test_vstr("$token-comma$middle$token-comma$last", sep);
	test_vstr("first$token-comma$middle$token-comma$", sep);
	test_vstr("$token-comma$middle$token-comma$", sep);
	test_vstr("$token-comma$$token-comma$middle$token-comma$$token-comma$", sep);
	test_vstr("$token-comma$$token-comma$middle1$token-comma$$token-comma$middle2$token-comma$$token-comma$", sep);
	test_vstr("$notokens$", sep);
	test_vstr("", sep);

	test_split3("dir1/dir2/dir3", "/");
	test_split3("di/r1/_/di/r2/_/di/r3", "/_/");
	test_split3("$tod$dir1$tod$dir2$tod$dir3$tod$", "$tod$");
	test_split3("dir1$tod$dir2$tod$dir3", "$tod$");

	return 0;
}

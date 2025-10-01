#include "helpers.h"

void test_vstr(char *str, char *delim)
{
	char **tokens = NULL;

	splitv(str, delim, &tokens);
	for (int i = 0; i < len(tokens); i++)
		println("tokens[", i, "]=", tokens[i]);
	char *res = joinv(tokens, len(tokens), ":_-_:");
	println("delim=", delim);
	println("str=", str);
	println("res=", res);

	const char *next;
	for (char *s = ___subtok(str, delim, &next); s; s = ___subtok(NULL, delim, &next))
		printf("sub=%s\n", s);
	println();
}

void test_split3(char *str, char *delim)
{
	char *dir[3];

	println("delim=", delim);
	println("str=", str);
	split(str, delim, &dir[0], &dir[1], &dir[2]);

	for (int i = 0; i < static_len(dir); i++)
		println("dir", i, "=", dir[i]);
	println();
}

int main()
{
	char *delim;

	delim = "$token-comma$";
	test_vstr("first$token-comma$middle$token-comma$last", delim);
	test_vstr("$token-comma$middle$token-comma$last", delim);
	test_vstr("first$token-comma$middle$token-comma$", delim);
	test_vstr("$token-comma$middle$token-comma$", delim);
	test_vstr("$token-comma$$token-comma$middle$token-comma$$token-comma$", delim);
	test_vstr("$token-comma$$token-comma$middle1$token-comma$$token-comma$middle2$token-comma$$token-comma$", delim);
	test_vstr("$notokens$", delim);
	test_vstr("", delim);

	test_split3("dir1/dir2/dir3", "/");
	test_split3("di/r1/_/di/r2/_/di/r3", "/_/");
	test_split3("$tod$dir1$tod$dir2$tod$dir3$tod$", "$tod$");
	test_split3("dir1$tod$dir2$tod$dir3", "$tod$");

	return 0;
}

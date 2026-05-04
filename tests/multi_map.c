#include "helpers.h"

typedef entry(const char *, int *) multi_map;

int *insert_multi(multi_map **mp, const char *key, int val)
{
	int **lt = lookup(mp, key);
	if (!lt)
		lt = insert(mp, key, NULL);
	if (!lt)
		return NULL;

	append(lt, val);
	return *lt;
}

int main(void)
{
	multi_map *multi_map = NULL;

	insert_multi(&multi_map, "hi", 1);
	insert_multi(&multi_map, "hi", 4);
	insert_multi(&multi_map, "hi", 7);
	insert_multi(&multi_map, "low", 20);
	insert_multi(&multi_map, "low", 25);
	insert_multi(&multi_map, "medium", 11);
	insert_multi(&multi_map, "medium", 14);

	foreach (m, multi_map)
		foreach (v, m->value)
			println("key=", m->key, " val=", *v);

	return 0;
}

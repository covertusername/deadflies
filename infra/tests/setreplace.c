#include <u.h>
#include <libc.h>
#include <thread.h>
#include "../primitives.h"
#include "../conv.h"
#include "../struct.h"

Typeinfo rune;

/* 
 * just a simple mathematical comparison, i don't feel like implementing
 * unicode alphabetical sorting for a goddamn test program
 */

int
runesort(Rune *a, Rune *b)
{
	if(*a >= *b)
		return *a - *b;
	else
		return *b - *a;
}

void
eappend(List *l, void *item, Jstring *type)
{
	if(append(l, item, type) != 0)
		sysfatal("%r");
}

void
threadmain(int, char **)
{
	List *test;
	Rune *a, *b, *c;

	rune = Typeinfo {
		.typename = jstring(L"rune");
		.isref = 1;
		.freefunc = free;
		.compare = runesort;
	};
	test = newlist(Set, rune);
	a = malloc(sizeof(Rune));
	b = malloc(sizeof(Rune));
	c = malloc(sizeof(Rune));
	*a = L'a';
	*b = L'b';
	*c = L'c';
	eappend(test, a, rune.typename);
	eappend(test, b, rune.typename);
	eappend(test, c, rune.typename);
	print("%C%C%C\n", *(test->items[0]), *(test->items[1]), *(test->items[2]));
	
}

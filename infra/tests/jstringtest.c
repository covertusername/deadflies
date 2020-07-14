#include <u.h>
#include <libc.h>
#include <thread.h>
#include "primitives.h"

void
threadmain(int, char **)
{
	Jstring *j;

	j = newjstring(20);
	print("didn't segfault!\n");
	freejstring(j);
	threadexitsall(nil);
}
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "primitives.h"

void
threadmain(int, char**)
{
	Byteseq *r;
	Byteseq *c;
	long cnt;

	r = newbyteseq(20);
	if(r == nil)
		sysfatal("%r");
	fprint(2, "lcasebyteseq test\n");
	fprint(2, "please feed me %d bytes of garbage: ", r->length);
	cnt = readn(0, r->sequence, r->length);
	if(cnt < 0)
		sysfatal("%r");
	fprint(2, "got %d bytes of garbage\n", cnt);
	fprint(2, "raw garbage is ");
	write(0, r->sequence, cnt);
	c = lcasebyreseq(r);
	if(c == nil)
		sysfatal("%r");
	fprint(2, "\ncooked garbage is ");
	write(0, c->sequence, cnt);
	freebyteseq(c);
	fprint(2, "ucasebyteseq test\n");
	fprint(2, "please feed me %d bytes of garbage: ", r->length);
}
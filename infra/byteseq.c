#include <u.h>
#include <libc.h>
#include <thread.h>
#include "primitives.h"

Byteseq *
newbyteseq(ulong length)
{
	Byteseq *b;

	b = mallocz(sizeof(Byteseq), 1);
	if(b == nil){
		return nil;
	}
	b->sequence = mallocz(length, 1);
	if(b->sequence == nil){
		return nil;
	}
	b->length = length;
	incref(b);
	return b;
}

void
freebyteseq(Byteseq *seq)
{
	if(decref(seq) == 0){
		free(seq->sequence);
		free(seq);
	}
}

int
resizebyteseq(Byteseq *seq, ulong length)
{
	void *new;

	new = realloc(seq->sequence, length);
	if(new == nil){
		return -1;
	}
	seq->sequence = new;
	seq->length = length;
	return 0;
}

/* 
 * i could probably consolidate these next two functions somehow but i think that would make it unnecessarily
 * more complex, especially considering that they are already very simple.
 */

int
writebyteseq(Byteseq *seq, ulong off, void *ptr, ulong cnt)
{
	if(off > seq->length){
		werrstr("writebyteseq: cannot write past end of sequence");
		return -1;
	}
	if(cnt + off > seq->length)				/* keep this in sync with readbyteseq */
		cnt = seq->length - off;
	if(cnt == 0)
		return 0;
	memcpy(seq->sequence + off, ptr, cnt);
	return cnt;
}

int
readbyteseq(Byteseq *seq, ulong off, void *ptr, ulong cnt)
{
	if(off > seq->length){
		werrstr("readbyteseq: cannot read past end of sequence");
		return -1;
	}
	if(cnt + off > seq->length)				/* keep this in sync with writebyteseq */
		cnt = seq->length - off;
	if(cnt == 0)
		return 0;
	memcpy(ptr, seq->sequence + off, cnt);
	return cnt;
}

/* 
 * exceedingly gross/useless functions defined in the infra standard, referenced by the html standard
 * the comment above the read/write functions applies here too, but these functions are even smaller.
 * the standard defines an arbitrary byte sequence, but these functions are obviously intended for use with
 * strings. (ascii strings, as well. what even is unicode?)
 */

Byteseq *
lcasebyteseq(Byteseq *seq)
{
	ulong i;
	Byteseq *b;

	b = newbyteseq(seq->length);
	if(b == nil)
		return nil;
	for(i = 0; i <= seq->length - 1; i++)
		b->sequence[i] = tolower(seq->sequence[i]);
	return b;
}

Byteseq *
ucasebyteseq(Byteseq *seq)
{
	ulong i;
	Byteseq *b;

	b = newbyteseq(seq->length);
	if(b == nil)
		return nil;
	for(i = 0; i <= seq->length - 1; i++)
		b->sequence[i] = toupper(seq->sequence[i]);
	return b;
}

/*
 * there is an algorithm in the standard for determining whether or not one sequence starts with
 * another. i have not implemented it here because the same result can be acheived if you check
 * the result of memcmp(2) correctly.
 * the same goes for seeing if one sequence is less than another.
 */



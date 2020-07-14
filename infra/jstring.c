#include <u.h>
#include <libc.h>
#include <thread.h>
#include "primitives.h"
#include "conv.h"
#include "rune.h"

/*
 * oseq should have been allocated by malloc, usually from within newjstring(). if there was an
 * error, oseq remains valid and this function returns nil. if there wasn't an error, this function
 * returns a pointer to the resulting sequence. oseq is no longer valid after this, so make a copy
 * of it if you need it.
 * oseql does not need to be allocated via malloc. the only reason it's a pointer is to provide a mechanism
 * to allow updating the length value in the caller.
 */

codeunit *
appendcuseq(codeunit *oseq, ulong *oseql, codeunit *aseq, ulong aseql)
{
	codeunit *cu;

	if(aseql == 0)
		return oseq;

	cu = realloc(oseq, (*oseql + aseql) * sizeof(codeunit));
	if(cu == nil)
		return nil;
	memcpy(cu + (*oseql * sizeof(codeunit)), aseq, aseql * sizeof(codeunit));
	*oseql = *oseql + aseql;
	return cu;
}

Jstring *
newjstring(ulong length)
{
	Jstring *j;

	j = mallocz(sizeof(Jstring), 1);
	if(j == nil)
		return nil;
	j->sequence = mallocz(length * sizeof(codeunit), 1);
	if(j->sequence == nil){
		free(j);
		return nil;
	}
	j->length = length;
	incref(j);
	return j;
}

ulong
countrunes(Jstring *j)
{
	ulong i;
	ulong cnt;

	cnt = 0;
	for(i = 0; i < j->length; i++){
		if(issurrogate(j->sequence[i]) && i < j->length - 1)
			if(issurrogate(j->sequence[i+1])){
				i++;
				cnt++;
				continue;
			}
		cnt++;
	}
	return cnt;
}

/*
 * there is an algorithm in the standard for determining whether or not one string starts with
 * another. i have not implemented it here because the same result can be acheived if you check
 * the result of memcmp(2) correctly.
 * the same goes for seeing if one string is less than another.
 */

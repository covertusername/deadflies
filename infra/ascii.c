#include <u.h>
#include <libc.h>
#include <ctype.h>
#include <thread.h>
#include "primitives.h"
#include "rune.h"

int (*opfunc)(Rune);

enum {
	Upper = -1,
	Lower = 1,
};

int
isasciistring(Jstring *j)
{
	ulong i;

	for(i = 0; i < j->length; i++)
		if(isascii(j->sequence[i]) == 0)
			return 0;
	return 1;
}

static Jstring *
ulasciistring(Jstring *string, int op)
{
	Jstring *j;
	ulong i;

	if(op == Upper)
		opfunc = isasciiupper;
	if(op == Lower)
		opfunc = isasciilower;

	j = newjstring(string->length);
	if(j == nil)
		return nil;

	for(i = 0; i < string->length; i++){
		if(opfunc((Rune)(string->sequence[i])) == 1)
			j->sequence[i] = string->sequence[i] + (op * 0x20);
		else
			j->sequence[i] = string->sequence[i];
	}
	j->isscalar = string->isscalar;
	return j;
}

Jstring *
tolowerasciistring(Jstring *string)
{
	return ulasciistring(string, Lower);
}

Jstring *
toupperasciistring(Jstring *string)
{
	return ulasciistring(string, Upper);
}

int
ciasciimatch(Jstring *a, Jstring *b)
{
	Jstring *la;
	Jstring *lb;
	ulong i;

	if(a->length != b->length)
		return 0;

	la = tolowerasciistring(a);
	if(la == nil)
		return -1;
	lb = tolowerasciistring(b);
	if(lb == nil){
		freejstring(la);
		return -1;
	}

	for(i = 0; i < a->length; i++)
		if(la->sequence[i] != lb->sequence[i]){
			freejstring(la);
			freejstring(lb);
			return 0;
		}
	freejstring(la);
	freejstring(lb);
	return 1;
}

Jstring *
stripnl(Jstring *string)
{
	Jstring *j;
	codeunit *c;
	codeunit *temp;
	ulong templen;
	ulong i;

	c = nil;
	templen = 0;
	for(i = 0; i < string->length; i++){
		if((string->sequence[i] == 0x000A) || (string->sequence[i] == 0x000D))
			continue;
		else{
			temp = appendcuseq(c, &templen, &(string->sequence[i]), 1);
			if(temp == nil){
				free(c);
				return nil;
			}
			c = temp;
		}
	}
	j = mallocz(sizeof(Jstring), 1);
	if(j == nil){
		free(c);
		return nil;
	}
	j->sequence = c;
	j->length = templen;
	j->isscalar = string->isscalar;
	incref(j);
	return j;
}

Jstring *
normalizenl(Jstring *string)
{
	Jstring *j;
	codeunit *c;
	codeunit *temp;
	ulong templen;
	ulong i;
	codeunit newline;

	c = nil;
	templen = 0;
	newline = 0x000A;
	for(i = 0; i < string->length; i++){
		if(string->sequence[i] == 0x000D){
			if(string->sequence[i+1] == 0x000A){
				temp = appendcuseq(c, &templen, &newline, 1);
				if(temp == nil){
					free(c);
					return nil;
				}
				c = temp;
				i++;
			}
			else{
				temp = appendcuseq(c, &templen, &newline, 1);
				if(temp == nil){
					free(c);
					return nil;
				}
				c = temp;
			}
		}
		else{
			temp = appendcuseq(c, &templen, &(string->sequence[i]), 1);
			if(temp == nil){
				free(c);
				return nil;
			}
			c = temp;
		}
	}
	j = mallocz(sizeof(Jstring), 1);
	if(j == nil){
		free(c);
		return nil;
	}
	j->sequence = c;
	j->length = templen;
	j->isscalar = string->isscalar;
	incref(j);
	return j;
}

Jstring *
stripltspace(Jstring *string)
{
	Jstring *j;
	ulong starti;
	ulong endi;
	ulong newlen;

	for(starti = 0; starti < string->length; starti++){
		if(iswhitespace((Rune)(string->sequence[starti])))
			continue;
		else
			break;
	}
	for(endi = string->length; endi > 0; endi--){
		if(iswhitespace((Rune)(string->sequence[endi - 1])))
			continue;
		else
			break;
	}
	if(endi < starti){
		/* 
		 * we got handed a string containing nothing but whitespace. bleh.
		 * just return the empty string.
		 */
		return newjstring(0);
	}
	newlen = endi - starti;
	j = newjstring(newlen);
	if(j == nil)
		return nil;
	memcpy(j->sequence, &(string->sequence[starti]), newlen*sizeof(codeunit));
	j->isscalar = string->isscalar;
	return j;
}

Jstring *
collapsespace(Jstring *string)
{
	Jstring *stripped;
	Jstring *j;
	ulong i;
	codeunit *c;
	codeunit *temp;
	ulong templen;
	codeunit space;

	/*
	 * the standard says we are supposed to collapse whitespace and THEN strip leading/trailing
	 * whitespace. we strip the leading/trailing whitespace here first because 1) the end result
	 * is the same, and 2) it keeps us from having to deal with the same sections of the string
	 * twice.
	 */

	stripped = stripltspace(string);
	if(stripped == nil)
		return nil;
	c = nil;
	templen = 0;
	space = 0x0020;
	for(i = 0; i < stripped->length; i++){
		if(iswhitespace((Rune)(stripped->sequence[i]))){
			temp = appendcuseq(c, &templen, &space, 1);
			if(temp == nil){
				free(c);
				freejstring(stripped);
				return nil;
			}
			c = temp;
			i++;
			/*
			 * we don't have to check if we fall off the end of the string here because we
			 * stripped trailing whitespace already. the last character in the string should be
			 * something else.
			 */
			for(; iswhitespace((Rune)(stripped->sequence[i])); i++)
				continue;
		}
		else{
			temp = appendcuseq(c, &templen, &(stripped->sequence[i]), 1);
			if(temp == nil){
				free(c);
				freejstring(stripped);
				return nil;
			}
			c = temp;
		}
	}
	freejstring(stripped);
	j = mallocz(sizeof(Jstring), 1);
	if(j == nil){
		free(c);
		return nil;
	}
	j->sequence = c;
	j->length = templen;
	j->isscalar = string->isscalar;
	incref(j);
	return j;
}

/*
 * there is a term "skip ASCII whitespace", but the way to do that is to simply call
 * collectrunes() using isspace(), and discard the result. the passed position variable will still
 * be updated.
 */

#include <u.h>
#include <libc.h>
#include <thread.h>
#include "primitives.h"
#include "conv.h"
#include "rune.h"

Jstring *
collectrunes(int (*cond)(Rune), Jstring *rawin, ulong *pos)
{
	Jstring *res;
	Jstring *input;
	codeunit *c;
	codeunit *temp;
	ulong templen;

	/* we scalarize the input to make sure we don't feed utf162rune bad runes. */
	input = scalarize(rawin);
	if(input == nil)
		return nil;
	c = nil;
	templen = 0;
	while(*pos < input->length && cond(utf162rune(&(input->sequence[*pos])))){
		if(issurrogate(input->sequence[*pos])){
			/* by this point all surrogates in the string are paired, so this is fine */
			temp = appendcuseq(c, &templen, &(input->sequence[*pos]), 2);
			if(temp == nil){
				free(c);
				freejstring(input);
				return nil;
			}
			c = temp;
			*pos += 2;
		}
		else{
			temp = appendcuseq(c, &templen, &(input->sequence[*pos]), 1);
			if(temp == nil){
				free(c);
				freejstring(input);
				return nil;
			}
			c = temp;
			*pos += 1;
		}
	}
	freejstring(input);
	res = mallocz(sizeof(Jstring), 1);
	if(res == nil){
		free(c);
		return nil;
	}
	res->sequence = c;
	res->length = templen;
	res->isscalar++;
	incref(res);
	return res;
}

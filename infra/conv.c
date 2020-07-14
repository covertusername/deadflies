#include <u.h>
#include <libc.h>
#include <ctype.h>
#include <thread.h>
#include "primitives.h"
#include "conv.h"
#include "rune.h"
#include "ascii.h"

/*
 * due to the nature of the decoding algorithm here, the resulting JString is guaranteed to be
 * scalar. specifically, all we are doing is casting chars to codeunits, and chars cannot be large
 * enough to fit within the range of surrogates.
 */

Jstring *
isodecode(Byteseq *input)
{
	Jstring *j;
	ulong i;

	j = newjstring(input->length);
	if(j == nil)
		return nil;
	for(i = 0; i < input->length; i++)
		j->sequence[i] = (codeunit)(input->sequence[i]);
	j->isscalar++;
	return j;
}

Byteseq *
isoencode(Jstring *input)
{
	Byteseq *b;
	ulong i;

	b = newbyteseq(input->length);
	if(b == nil)
		return nil;
	for(i = 0; i < input->length; i++){
		if(input->sequence[i] > 0x00FF){
			freebyteseq(b);
			werrstr("isoencode: can't encode string with codeunits greater than 0x00FF");
			return nil;
		}
		b->sequence[i] = (u8int)(input->sequence[i]);
	}
	return b;
}

Byteseq *
asciiencode(Jstring *j)
{
	if(isasciistring(j))
		return isoencode(j);
	werrstr("asciiencode: cannot encode non-ascii string");
	return nil;
}

Jstring *
asciidecode(Byteseq *b)
{
	ulong i;

	for(i = 0; i < b->length; i++)
		if(isascii(b->sequence[i]) != 1){
			werrstr("asciidecode: cannot encode non-ascii byte sequence");
			return nil;
		}
	return isodecode(b);
}

/*
 * this function was modified from unicode.readthedocs.io section 7.5 on Sun Sep 29 23:01:05 CDT 2019.
 * it was then edited to handle characters that either needed no conversion or were out of Unicode
 * range. (the original version aborted on either case, via an assert(2).)
 * the function's name there was 'encode_utf16_pair'.
 * this function returns 0 if no conversion was done, 1 if it was, -1 on error.
 */

int
rune2utf16(Rune r, codeunit *part)
{
	uint code;

	if(r < 0x10000){
		return 0;
	}
	if(r > 0x10FFFF){
		werrstr("rune2utf16: %d is not a valid rune\n", r);
		return -1;
	}
	code = r - 0x10000;
	part[0] = 0xD800 | (code >> 10);
	part[1] = 0xDC00 | (code & 0x3FF);
	return 1;
}

/*
 * this function was modified from unicode.readthedocs.io section 7.5 on Tue Oct 1 07:26:12 CDT 2019.
 * it was then edited to handle invalid utf-16 surrogate pairs. (the original version aborted when
 * given non-surrogates using an assert(2).)
 * the function's name there was 'decode_utf16_pair'.
 * if given a non-surrogate in part[0], it returns that value.
 * if part[0] is a surrogate but part[1] isn't, it returnes Runeerror.
 */

Rune
utf162rune(codeunit *part)
{
	Rune code;

	if(part[0] < 0xD800 || part[0] > 0xDBFF)
		return (Rune)part[0];
	if(part[1] < 0xDC00 || part[1] > 0xDFFF)
		return Runeerror;
	code = 0x10000;
	code += (part[0] & 0x03FF) << 10;
	code += (part[1] & 0x03FF);
	return code;
}

Jstring *
scalarize(Jstring *string)
{
	Jstring *j;
	ulong i;
	codeunit replace;

	if(string->isscalar){
		incref(string);
		return string;
	}
	if(string->length == 0){
		string->isscalar++;
		incref(string);
		return string;
	}
	j = newjstring(string->length);
	if(j == nil)
		return nil;
	memcpy(j->sequence, string->sequence, j->length * sizeof(codeunit));
	replace = 0xFFFD;
	for(i = 0; i < j->length; i++){
		if(issurrogate(j->sequence[i])){
			if(i < j->length - 1 && issurrogate(j->sequence[i + 1])){
				i++;
				continue;
			}
			j->sequence[i] = replace;
			continue;
		}
	}
	return j;
}

/* 
 * this function converts between a nil-terminated sequence of Runes and
 * jstrings.  it's easy to make string literals into jstrings now, using
 * the syntax L"literal" provided by the C compiler.  (don't run this on
 * unix.)
 * the returned pointer is malloc()ed like a normal Jstring, so you
 * should probably assign it to a variable first so you can free it
 * later, rather than passing the result of this function directly.
 */

Jstring *
jstring(Rune *string)
{
	Jstring *j;
	uint stringlen;
	ulong i;
	ulong tempsize;
	codeunit part[2];
	codeunit *temp;
	codeunit *temp2;
	codeunit casted;

	stringlen = runecnt(string);
	temp = nil;
	tempsize = 0;
	for(i = 0; i <= stringlen; i++){
		switch(rune2utf16(string[i], part)){
		case 1:
			temp2 = appendcuseq(temp, &tempsize, part, 2);
			if(temp2 == nil){
				free(temp);
				return nil;
			}
			temp = temp2;
			break;
		case 0:
			casted = (codeunit)(string[i]);
			temp2 = appendcuseq(temp, &tempsize, &casted, 1);
			if(temp2 == nil){
				free(temp);
				return nil;
			}
			temp = temp2;
			break;
		default:
			free(temp);
			return nil;
		}
	}
	j = mallocz(sizeof(Jstring), 1);
	if(j == nil){
		free(temp);
		return nil;
	}
	j->sequence = temp;
	j->length = tempsize;
	incref(j);
	return j;
}

#include <u.h>
/* no libc.h because this doesn't actually call the c library */

/* 
 * a scalar value is defined as a code point (Rune in plan 9) that is not a surrogate.
 * frankly, i don't feel like making an entire function just to flip the polarity of this one, so just
 * invert the output of issurrogate() in your if statements.
 */

int
issurrogate(Rune r)
{
	return r >= 0xD800 && r <= 0xDFFF;
}

int
isnotchar(Rune r)
{
	switch(r){
	case 0xFFFE: case 0xFFFF: case 0x1FFFE: case 0x1FFFF: case 0x2FFFE: case 0x2FFFF:
	case 0x3FFFE: case 0x3FFFF: case 0x4FFFE: case 0x4FFFF: case 0x5FFFE: case 0x5FFFF:
	case 0x6FFFE: case 0x6FFFF: case 0x7FFFE: case 0x7FFFF: case 0x8FFFE: case 0x8FFFF:
	case 0x9FFFE: case 0x9FFFF: case 0xAFFFE: case 0xAFFFF: case 0xBFFFE: case 0xBFFFF:
	case 0xCFFFE: case 0xCFFFF: case 0xDFFFE: case 0xDFFFF: case 0xEFFFE: case 0xEFFFF:
	case 0xFFFFE: case 0xFFFFF: case 0x10FFFE: case 0x10FFFF:
		return 1;
	default:
		return r >= 0xFDD0 && r <= 0xFDEF;
	}
}

/*
 * isascii is defined in ctype(2), and works on runes.
 * i am not re-implementing it here, use that.
 * the other macros there don't work on runes, however, so those are re-implemented here, and may not be
 * exactly the same, implementation-wise - i am trying to strictly conform to the infra/html standard.
 */

int
istabornewline(Rune r)
{
	switch(r){
	case 0x0009: case 0x000A: case 0x000D:
		return 1;
	default:
		return 0;
	}
}

/* 
 * this is one of the functions that diverges from ctype(2), specifically isspace().
 * it does not include vertical tabs, but everything else is identical.
 */

int
iswhitespace(Rune r)
{
	switch(r){
	case 0x0009: case 0x000A: case 0x000D: case 0x000C: case 0x0020:
		return 1;
	default:
		return 0;
	}
}

int
isc0(Rune r)
{
	return r <= 0x001F;
}

int
isc0orspace(Rune r)
{
	return isc0(r) || r == 0x0020;
}

/*
 * this function is basically iscntrl() in ctype(2) but
 * with additional characters outside the ascii range.
 * (specifically, the characters in the range 0x0080 to 0x009F, inclusive.)
 */

int
iscontrol(Rune r)
{
	return isc0(r) || (r >= 0x007F && r <= 0x009F);
}

/* this function is identical in specification to isdigit() in ctype(2). grrr. */

int
isasciidigit(Rune r)
{
	return r >= 0x0030 && r <= 0x0039;
}

/* these next three functions do the job of the single isxdigit() in ctype(2). */

int
isasciiuxdigit(Rune r)
{
	return isasciidigit(r) || (r >= 0x0041 && r <= 0x0046);
}

int
isasciilxdigit(Rune r)
{
	return isasciidigit(r) || (r >= 0x0061 && r <= 0x0066);
}

int
isasciixdigit(Rune r)
{
	return isasciiuxdigit(r) || isasciilxdigit(r);
}

/* isupper(), islower(), isalpha(), isalnum(). */

int
isasciiupper(Rune r)
{
	return r >= 0x0041 && r <= 0x005A;
}

int
isasciilower(Rune r)
{
	return r >= 0x0061 && r <= 0x007A;
}

int
isasciialpha(Rune r)
{
	return isasciiupper(r) || isasciilower(r);
}

int
isasciialnum(Rune r)
{
	return isasciidigit(r) || isasciialpha(r);
}

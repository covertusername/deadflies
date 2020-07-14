#include <u.h>
#include <libc.h>
#include <thread.h>
#include "primitives.h"
#include "struct.h"

/* base list type functions */

List *
newlist(int listtype, Typeinfo info)
{
	List *l;

	l = mallocz(sizeof(List), 1);
	if(l == nil)
		return nil;
	l->items = malloc(0); /* just so we have something to realloc() later */
	if(l->items == nil){
		free(l);
		return nil;
	}
	l->info = info;
	incref(l->info.typename);
	l->listtype = listtype;
	incref(l);
	return l;
}

void
freelist(List *l)
{
	ulong i;

	if(decref(l) == 0){
		for(i = 0; i < l->size; i++)
			l->info.freefunc(l->items[i]);
		free(l->items);
		freejstring(l->info.typename);
		free(l);
	}
}

/*
 * contrary to the standard, append, extend and prepend all use this
 * function internally because my implementation renders all these
 * actions essentially identical in execution.  the standard actually
 * says to call prepend() if index is 0; but here, calling a dedicated
 * prepend would do the same thing as an insert.
 */

static int
_insert(List *l, void *item, Jstring *type, ulong index)
{
	void *temp;

	if(l->info.typename->length != type->length || memcmp(l->info.typename->sequence, type->sequence, type->length) != 0){
		werrstr("insert: list type mismatch");
		return -1;
	}
	if(index > l->size){
		werrstr("insert: index out of range");
		return -1;
	}
	temp = realloc(l->items, sizeof(void *) * (l->size + 1));
	if(temp == nil)
		return -1;
	l->items = temp;
	memcpy(&(l->items[index + 1]), &(l->items[index]), (l->size - index) * sizeof(void *));
	if(l->info.isref)
		incref(item);
	l->items[index] = item;
	l->size++;
	return 0;
}

/* 
 * to preserve operational simplicity, 'contains' is only partially
 * implemented here.  the rest of the implementation depends on the type
 * itself, but essentially just needs be be a comparison operation.
 * returns the index of the item if the list contains it, otherwise
 * returns -1.
 */

vlong
contains(List *l, void *item, Jstring *type)
{
	ulong i;

	if(l->info.typename->length != type->length || memcmp(l->info.typename->sequence, type->sequence, type->length) != 0)
		return -1; /* not really an error because a list of a type can't contain objects of other types */
	for(i = 0; i < l->size; i++)
		if(l->info.compare(l->items[i], item) == 0)
			return i;
	return -1;
}

/*
 * technically, the specification does not say that you can't perform
 * arbitrary insertion (potentially creating duplicates) on sets.
 * however, such an action makes no sense, and as opposed to refusing to
 * allow this to work on sets, i will simply make it perform the same
 * checks as the other functions.
 */

int
insert(List *l, void *item, Jstring *type, ulong index)
{
	if((l->listtype == Set) && (contains(l, item, type) <= 0))
		return 0;
	return _insert(l, item, type, index);
}

int
append(List *l, void *item, Jstring *type)
{
	int e;

	if((l->listtype == Set) && (contains(l, item, type) <= 0))
		return 0;
	e = _insert(l, item, type, l->size);
	if(e == -1)
		werrstr("append: %r");
	return e;
}

void
extend(List *a, List *b)
{
	ulong i;

	/* sysfatal here because list a might be modified when the failure occurs */
	for(i = 0; i < b->size; i++)
		if(append(a, b->items[i], b->info.typename) != 0)
			sysfatal("extend: %r");
}

int
prepend(List *l, void *item, Jstring *type)
{
	int e;

	if((l->listtype == Set) && (contains(l, item, type) <= 0))
		return 0;
	e = _insert(l, item, type, 0);
	if(e == -1)
		werrstr("prepend: %r");
	return e;
}

/*
 * these two functions were originally huge and complicated, in an
 * attempt to match the standard.  i eventually needed to do too much
 * with them.  instead of accepting a condition, they just take an item
 * to be replaced/removed (in the form of an index), and for replace, its
 * replacement.  the condition should be evaluated in the calling
 * function if necessary.
 */

static int
_replace(List *l, ulong index, void *replacement, Jstring *type)
{
	if(l->info.typename->length != type->length || memcmp(l->info.typename->sequence, type->sequence, type->length) != 0){
		werrstr("replace: list type mismatch");
		return -1;
	}
	l->info.freefunc(l->items[index]);
	if(l->info.isref != 0)
		incref(replacement);
	l->items[index] = replacement;
	return 0;
}

int
replace(List *l, ulong index, void *replacement, Jstring *type)
{
	vlong repindex;
	int e;

	if(l->listtype == Set){
		repindex = contains(l, replacement, type)
		if(repindex < 0)
			e = _replace(l, index, replacement, type);
		if(repindex == index)
			return 0; /* no point */
		if(repindex < index)
			lremove(l, index)
		if(repindex > index){
			e = _replace(l, index, replacement, type);
			if(e < 0)
				return e;
			lremove(l, repindex);
		}
		return e;
	}
	return _replace(l, index, replacement, type);
}

/* 
 * the standard specifies explicitly removing each item, but it's faster
 * to just iterate through them directly.
 */

int
empty(List *l)
{
	ulong i;
	void *temp;

	temp = malloc(0);
	if(temp == nil)
		return -1;
	for(i = 0; i < l->size; i++)
		l->info.freefunc(l->items[i]);
	free(l->items);
	l->items = temp;
}

void
lremove(List *l, ulong index)
{
	void *temp;

	l->info.freefunc(l->items[index]);
	l->size--;
	memmove(l->items+index, l->items+index+1, (l->size - index) * sizeof(void *));
	temp = realloc(l->items, l->size);
	if(temp == nil)
		sysfatal("lremove: %r");
	l->items = temp;
}

/*
 * the standard says that making a clone should pretty much be identical
 * to extend()ing an empty list.  i don't do that here, it is easier to
 * just copy all the pointers over since they will be in the same order
 * anyway.  note that this is a shallow clone, and doesn't clone the
 * items themselves.
 */

List *
clone(List *donor)
{
	List *_clone;
	void *temp;
	int i;

	_clone = newlist(donor->listtype, donor->info);
	if(_clone == nil)
		return nil;
	temp = realloc(_clone->items, donor->size * sizeof(void *));
	if(temp == nil){
		freelist(_clone);
		return nil;
	}
	_clone->items = temp;
	memcpy(_clone->items, donor->items, donor->size * sizeof(void *));
	if(_clone->info.isref)
		for(i = 0; i < _clone->size; i++)
			incref(_clone->items[i]);
	_clone->size = donor->size;
	return _clone;
}

/*
 * the standard defines ascending/descending sorting.  nothing needs to
 * be implemented here thanks to the system qsort(2) function.
 * unfortunately you will have to pass it a custom function for each item
 * type.
 */

/* stack functions */

int
push(List *l, void *item, Jstring *type)
{
	return append(l, item, type);
}

void *
pop(List *l)
{
	void *ret;

	if(l->size == 0){
		werrstr("empty");
		return nil;
	}
	ret = l->items[l->size - 1];
	if(l->info.isref)
		incref(ret);
	lremove(l, l->size - 1);
	return ret;
}

/* queue functions */

int
enqueue(List *l, void *item, Jstring *type)
{
	return append(l, item, type);
}

void *
dequeue(List *l)
{
	void *ret;

	if(l->size == 0){
		werrstr("empty");
		return nil;
	}
	ret = l->items[0];
	if(l->info.isref)
		incref(ret);
	lremove(l, l->items[0]);
	return ret;
}

/* set functions */

/* 
 * returns 0 if a is a subset of b, 1 if b is a subset of a, -1 if
 * neither case is true. will return 0 if a and b are the same set.
 * to test for supersets, negate this function.
 */

int
subset(List *a, List *b)
{
	ulong i;
	int ret;

	if(a->size <= b->size)
		for(i = 0; i < a->size; i++){
			if(contains(b, a->items[i], a->info.typename) < 0)
				return -1;
			return 0;
		}
	else {
		ret = subset(b, a);
		if(ret == -1)
			return -1;
		return !ret;
	}
}

List *
intersection(List *a, List *b)
{
	List *ret;
	ulong i;

	ret = newlist(Set, a->info);
	if(ret == nil)
		return nil;
	for(i = 0; i < a->size; i++){
		if(contains(b, a->items[i], a->info.typename) >= 0)
			if(append(ret, a->items[i], a->info.typename) == -1){
				freelist(ret);
				return nil;
			}				
	}
	return ret;
}
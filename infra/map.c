#include <u.h>
#include <libc.h>
#include <thread.h>
#include "primitives.h"
#include "struct.h"

Map *
newmap(Typeinfo keyinfo, Typeinfo valinfo)
{
	Map *new;

	new = malloc(sizeof(Map));
	if(new == nil)
		return nil;
	new->set = newlist(Set, keyinfo);
	if(new->set == nil){
		free(new);
		return nil;
	}
	new->values = malloc(0);
	if(new->values == nil){
		freelist(new->set);
		free(new);
		return nil;
	}
	new->info = valinfo;
	incref(new);
	incref(new->info.typename);
	incref(new->set);
	return new;
}

void
freemap(Map *map)
{
	ulong i;

	if(decref(map) == 0){
		for(i = 0; i < map->set->size; i++)
			map->info.freefunc(map->values[i]);
		free(map->values);
		freelist(map->set);
		freejstring(map->info.typename);
		free(map);
	}
}

/* get a value from the map based on its key, if any */

void *
getval(Map *map, void *key, Jstring *type)
{
	int i;

	i = contains(map->set, key, type);
	if(i == -1)
		return nil;
	if(map->info.isref)
		incref(map->values[i]);
	return map->values[i];
}

/* 
 * given a key, set the corresponding value to value, creating a new
 * entry if it doesn't exist
*/

int
setval(Map *map, void *key, Jstring *keytype, void *value, Jstring *valtype)
{
	vlong i;
	void *temp;

	i = contains(map->set, key, keytype);
	if(i == -1){
		if(append(map->set, key, keytype) == -1){
			werrstr("setval: %r");
			return -1;
		}
		i = map->set->size - 1;
		temp = realloc(map->values, map->set->size * sizeof(void *);
		if(temp == nil){
			lremove(map->set, i);
			return -1;
		}
	}
	if(map->info.typename->length != valtype->length || memcmp(map->info.typename->sequence, valtype->sequence, valtype->length) != 0){
		werrstr("setval: map value type mismatch");
		return -1;
	}
	map->values[i] = value;
	return 0;
}

void
mremove(Map *map, ulong index)
{
	void *temp;

	lremove(map->set, index);
	map->info.freefunc(map->values[index]);
	memmove(&(map->values[index]), &(map->values[index+1]), (map->set->size - index) * sizeof(void *));
	temp = realloc(map->values, map->set->size);
	if(temp == nil)
		sysfatal("mremove: %r");
	map->values = temp;
}

int
clear(Map *map)
{
	ulong i;
	void *temp;
	ulong size;

	size = map->set->size;
	temp = malloc(0);
	if(temp == nil)
		return -1;
	if(empty(map->set) == -1)
		return -1;
	for(i = 0; i < size; i++)
		map->info.freefunc(map->values[i]);
	free(map->values);
	map->values = temp;
	return 0;
}

/* contains() from the list functions can be re-used here via Map->set. we don't need to re-implement it. */

/* we return a clone of Map->set here so we don't corrupt the map */

List *
getkeys(Map *map)
{
	return lclone(map->set);
}

List *
getvals(Map *map)
{
	List *ret;
	void *temp;

	ret = newlist(Plain, map->info);
	if(ret == nil)
		return nil;
	temp = realloc(ret->items, map->set->size * sizeof(void *));
	if(temp == nil)
		return nil;
	ret->size = map->set->size;
	ret->items = temp;
	memcpy(ret->items, map->values, ret->size * sizeof(void *));
	if(ret->info.isref)
		for(i = 0; i < ret->size; i++)
			incref(ret->items[i]);
	return ret;
}

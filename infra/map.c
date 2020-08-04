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

/* get a value from the map based on its key */

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
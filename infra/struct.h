typedef struct List List;
typedef struct Typeinfo Typeinfo;
typedef struct Map Map;

/* list types */

enum{
	Plain,
	Stack,
	Queue,
	Set
};

struct Typeinfo
{
	Jstring *typename; /* the type of the items in the structure */
	int isref; /* 0 if not reference-counted, 1 otherwise */
	void (*freefunc)(void *); /* a function to call on each item in a struct when said item needs to be freed */
	int (*compare)(void *, void *); /* comparison, should be a function as described in qsort(2) */
};

struct List
{
	QLock;
	Ref;
	Typeinfo info;
	void **items;
	ulong size;
	int listtype; /* the type of list */
};

struct Map
{
	QLock;
	Ref;
	List *set; /* the key part of the map. always an ordered set. should not be used directly, unless returned when getting the keys. the items are the keys. */
	Typeinfo info;
	void **values;
}

/* generic list operations */
List *newlist(int listtype, Typeinfo info);
vlong contains(List *l, void *item, Jstring *type);
void freelist(List *l);
int append(List *l, void *item, Jstring *type);
void extend(List *a, List *b);
int prepend(List *l, void *item, Jstring *type);
int replace(List *l, ulong index, void *replacement, Jstring *type);
int empty(List *l);
int insert(List *l, void *item, Jstring *type, ulong index);
void lremove(List *l, ulong index);
List *clone(List *donor);

/* stacks */

int push(List *l, void *item, Jstring *type);
void *pop(List *l);

/* queues */

int enqueue(List *l, void *item, Jstring *type);
void *dequeue(List *l);

/* sets */

int subset(List *a, List *b);
List *intersection(List *a, List *b);
List *union(List *a, List *b);

/* map operations */

Map *newmap(Typeinfo keyinfo, Typeinfo valinfo);
void freemap(Map *map);
void *getval(Map *map, void *key, Jstring *type);
int setval(Map *map, void *key, Jstring *keytype, void *value, Jstring *valtype);
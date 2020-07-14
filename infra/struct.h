typedef struct List List;
typedef struct Typeinfo Typeinfo;

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

/* stacks */

int push(List *l, void *item, Jstring *type);
void *pop(List *l);

/* queues */

int enqueue(List *l, void *item, Jstring *type);
void *dequeue(List *l);

/* sets */

int subset(List *a, List *b);
int intersect(List *a, List *b);

/* ascii tests */

int isasciistring(Jstring *j);
Jstring *tolowerstring(Jstring *string);
Jstring *toupperstring(Jstring *string);
int ciasciimatch(Jstring *a, Jstring *b);
Jstring *stripnl(Jstring *string);
Jstring *normalizenl(Jstring *string);
Jstring *stripltspace(Jstring *string);
Jstring *collapsespace(Jstring *string);

/* type conversion */

Jstring *isodecode(Byteseq *input);
int rune2utf16(Rune r, codeunit *part);
Rune utf162rune(codeunit *part);
Jstring *scalarize(Jstring *j);
Byteseq *isoencode(Jstring *input);
Jstring *jstring(Rune *string);
Byteseq *asciiencode(Jstring *j);
Jstring *asciidecode(Byteseq *b);

/* one of a kind */

Jstring *collectrunes(int (*cond)(Rune), Jstring *input, ulong *pos);

/* primitive types */

typedef struct Byteseq Byteseq;

struct Byteseq
{
	RWLock;
	Ref;
	u8int *sequence;
	ulong length;
};

Byteseq *newbyteseq(ulong length);
int resizebyteseq(Byteseq *seq, ulong length);
void freebyteseq(Byteseq *seq);
int writebyteseq(Byteseq *seq, ulong off, void *ptr, ulong cnt);
int readbyteseq(Byteseq *seq, ulong off, void *ptr, ulong cnt);
Byteseq *lcasebyteseq(Byteseq *seq);
Byteseq *ucasebyteseq(Byteseq *seq);

typedef struct Jstring Jstring;
typedef u16int codeunit;

struct Jstring
{
	RWLock;
	Ref;
	codeunit *sequence;
	ulong length;
	int isscalar;
};

codeunit *appendcuseq(codeunit *oseq, ulong *oseql, codeunit *aseq, ulong aseql);
Jstring *newjstring(ulong length);
void freejstring(Jstring *string);
Jstring *dupjstring(Jstring *string);
ulong countrunes(Jstring *string);

char Egreg[] = "the flies are alive!";

/* like ctype.h but works on Runes, not chars */

int issurrogate(Rune r);
int isnotchar(Rune r);
int istabornewline(Rune r);
int iswhitespace(Rune r);
int isc0(Rune r);
int isc0orspace(Rune r);
int iscontrol(Rune r);
int isasciidigit(Rune r);
int isasciiuxdigit(Rune r);
int isasciilxdigit(Rune r);
int isasciixdigit(Rune r);
int isasciiupper(Rune r);
int isasciilower(Rune r);
int isasciialpha(Rune r);
int isasciialnum(Rune r);
uint runecnt(Rune *r);

/* data structures */

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
List *lclone(List *donor);

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
void mremove(Map *map, ulong index);
int clear(Map *map);
List *getkeys(Map *map);
List *getvals(Map *map);
Map *mclone(Map *map);

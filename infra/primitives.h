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

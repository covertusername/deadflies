Jstring *isodecode(Byteseq *input);
int rune2utf16(Rune r, codeunit *part);
Rune utf162rune(codeunit *part);
Jstring *scalarize(Jstring *j);
Byteseq *isoencode(Jstring *input);
Jstring *jstring(Rune *string);
Byteseq *asciiencode(Jstring *j);
Jstring *asciidecode(Byteseq *b);

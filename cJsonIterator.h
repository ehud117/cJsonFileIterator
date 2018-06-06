#define USE_STDIO
#include <stdbool.h>

#ifdef USE_STDIO
#include <stdio.h>
typedef FILE *Json;
#define jsonGetc(json) getc(json)
#define jsonUngetc(c,json) ungetc(c, json)
#endif



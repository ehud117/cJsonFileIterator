#include <stdbool.h>

#define JSON_USE_STDIO

enum {
	JSON_OK,
	JSON_ENDED,
	JSON_ERROR
};

#ifdef JSON_USE_STDIO
#include <stdio.h>
typedef FILE *Json;
#endif
extern bool jsonOpen(Json *json, char *fileName);
extern int jsonGetFirstObjectKey(Json *json, char *dst);
extern int jsonGetFirstArrayValue(Json *json, char *dst);
extern int jsonGetNextArrayValue(Json *json, char *dst);
extern int jsonGetObjectValue(Json *json, char *dst);
extern int jsonGetNextObjectKey(Json *json, char *dst);
extern void jsonClose(Json *json);

inline bool jsonOpen(Json *json, char *fileName) {
#ifdef JSON_USE_STDIO
	*json = fopen(fileName, "r");
	return (*json) != NULL;
#else
	return false;
#endif
}

void jsonClose(Json *json) {
#ifdef JSON_USE_STDIO
	fclose(*json);
#endif
}

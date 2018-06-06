#include "cJsonIterator.h"
#include <string.h>
#include <ctype.h>
#include <assert.h>

bool jsonOpen(Json *json, char *fileName) {
#ifdef USE_STDIO
	*json = fopen(fileName, "r");
	return (*json) != NULL;
#endif
}

void jsonClose(Json *json) {
#ifdef USE_STDIO
	fclose(*json);
#endif
}
static bool readString(Json *json, char *dst) {
	int c;
	while (true) {
		c = jsonGetc(*json);
		if (c == -1)
			return false;
		if (c == '\\') {
			c = jsonGetc(*json);
			if (c == -1)
				return false;
			switch (c) {
				case 'b': 
					*dst++ = '\b'; break;
				case 'f': 
					*dst++ = '\f'; break;
				case 'n':
					*dst++ = '\n'; break;
				case 'r':
					*dst++ = '\r'; break;
				case 't':
					*dst++ = '\t'; break;
				// TODO: \u followed by 4 hex digits
				default: 
					*dst++ = c; 
					break;
			}
		} else if (c != '"') {
			*dst++ = c;
		} else { // Closing double quatation reached.
			*dst = '\0';
			return true;
		}
	}
}

static bool checkLiteralToken(Json *json, const char *tokEnd, char *dst) {
	int c;
	for (; *tokEnd != '\0'; tokEnd++) {
		c = jsonGetc(*json);
		if (c == -1)
			return false;
		if (*tokEnd != c)
			return false;
		*dst++ = c;
	}
	*dst = '\0';
	return true;
	
}

bool _jsonGetNextToken(Json *json, char *dst) {
	int c;
	do {
		c = jsonGetc(*json);
	} while (c == ' ' || c == '\t' || c == '\n' || c == '\r');
	
	*dst++ = c;
	if (strchr("{}[],:", c) != NULL) {
		*dst = '\0';
		return true;
	}
	
	if (c == '"') { // Read string from json.
		dst--; // Skip the opening double quatation
		return readString(json, dst);
	}
	if (c == 't')  // Possibly the 'true' token
		return checkLiteralToken(json, "rue", dst);
	if (c == 'f') // Possibliy the 'false' token
		return checkLiteralToken(json, "alse", dst);
	if (c == 'n') // Possibliy the 'null' token
		return checkLiteralToken(json, "ull", dst);
	if (isdigit(c)) {
		while (true) {
			c = jsonGetc(*json);
			if (! isdigit(c)) 
				break;
			*dst++ = c;
		}	
		jsonUngetc(c, *json);
		*dst++ = '\0';
		return true;
	}
	return false;
}

bool jsonGetNextToken(Json *json, char *dst) {
	bool res = _jsonGetNextToken(json, dst);
	/* printf("jsonGetNextToken returned %d, %s\n", res, dst); */
	return res;
}

#define JSON_FILENAME "example.json"
int main() {
	Json json;
	char tmp[50];
	char colon[10];
	
	printf("reading file %s\n", JSON_FILENAME);
	
	if (! jsonOpen(&json, JSON_FILENAME)) {
		printf("Couldn't open file %s for reading\n", JSON_FILENAME);
		return 0;
	}
	assert(jsonGetNextToken(&json, tmp));
	assert(strcmp(tmp, "{") == 0);
	assert(jsonGetNextToken(&json, tmp));
	if (strcmp(tmp, "}") == 0) {
		jsonClose(&json);
		return 0;
	}
	while (true) {
		printf("Value(s) for key \"%s\" - ", tmp);
		
		assert(jsonGetNextToken(&json, colon));
		assert(strcmp(colon, ":") == 0);
		
		if (strcmp(tmp, "intakeTimes") == 0 || strcmp(tmp, "pillsPerIntake") == 0) {
			assert(jsonGetNextToken(&json, tmp));
			assert(strcmp(tmp, "[") == 0);
			assert(jsonGetNextToken(&json, tmp));
			if (strcmp(tmp, "]") != 0) {
				printf("%s", tmp);
				while (true) {
					assert(jsonGetNextToken(&json, tmp));
					if (strcmp(tmp, "]") == 0)
						break;
					assert(strcmp(tmp, ",") == 0);
					assert(jsonGetNextToken(&json, tmp));
					printf("  %s", tmp);
				}
			}
			printf("\n");
		} else {
			assert(jsonGetNextToken(&json, tmp));
			printf("%s\n", tmp);
		}
		assert(jsonGetNextToken(&json, tmp));
		if (strcmp(tmp, "}") == 0) {
			jsonClose(&json);
			return 0;
		}
		assert(strcmp(tmp, ",") == 0);
		assert(jsonGetNextToken(&json, tmp));
	}
}


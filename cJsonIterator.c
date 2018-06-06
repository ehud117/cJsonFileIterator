#include "cJsonIterator.h"
#include <string.h>
#include <ctype.h>
#include <assert.h>

#ifdef JSON_USE_STDIO
#define jsonGetc(json) getc(json)
#define jsonUngetc(c,json) ungetc(c, json)
#endif

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

int jsonGetObjectValue(Json *json, char *dst) {
	if (! jsonGetNextToken(json, dst))
		return false;
	if (strcmp(dst, ":") != 0)
		return false;
	return jsonGetNextToken(json, dst);
}

int jsonGetFirstObjectKey(Json *json, char *dst) {
	if (! jsonGetNextToken(json, dst))
		return JSON_ERROR;
	if (strcmp(dst, "{") != 0)
		return JSON_ERROR;
	if (! jsonGetNextToken(json, dst))
		return JSON_ERROR;
	if (strcmp(dst, "}") == 0)
		return JSON_ENDED;
	else
		return JSON_OK;
}

int jsonGetNextObjectKey(Json *json, char *dst) {
	// Get the next token
	if (! jsonGetNextToken(json, dst))
		return JSON_ERROR;
	// Is it the end of json
	if (strcmp(dst, "}") == 0)
		return JSON_ENDED;
	// If not it should be ','
	if (strcmp(dst, ",") != 0)
		return JSON_ERROR;
	if (jsonGetNextToken(json, dst))
		return JSON_OK;
	else
		return JSON_ERROR;
}

int jsonGetFirstArrayValue(Json *json, char *dst) {
	if (! jsonGetObjectValue(json, dst))
		return JSON_ERROR;
	if (strcmp(dst, "[") != 0)
		return JSON_ERROR;
	if (! jsonGetNextToken(json, dst))
		return JSON_ERROR;
	if (strcmp(dst, "]") == 0)
		return JSON_ENDED;
	else
		return JSON_OK;
}

int jsonGetNextArrayValue(Json *json, char *dst) {
	if (! jsonGetNextToken(json, dst))
		return JSON_ERROR;
	// Is it the end of array
	if (strcmp(dst, "]") == 0)
		return JSON_ENDED;
	// If not it should be ','
	if (strcmp(dst, ",") != 0)
		return JSON_ERROR;
	if (jsonGetNextToken(json, dst))
		return JSON_OK;
	else
		return JSON_ERROR;
}
	
#define JSON_FILENAME "example.json"
int main() {
	Json json;
	int res;	
	char tmp[50];
	
	if (! jsonOpen(&json, JSON_FILENAME)) {
		printf("Couldn't open file %s for reading\n", JSON_FILENAME);
		return 0;
	}
	
	res = jsonGetFirstObjectKey(&json, tmp);
	while (res != JSON_ENDED) {
		printf("Value(s) for key \"%s\" - ", tmp);
		
		if (strcmp(tmp, "intakeTimes") == 0 || strcmp(tmp, "pillsPerIntake") == 0) {
			res = jsonGetFirstArrayValue(&json, tmp);
			assert (res != JSON_ERROR);
			while (res != JSON_ENDED) {
				printf("%s   ", tmp);
				res = jsonGetNextArrayValue(&json, tmp);
				assert (res != JSON_ERROR);
			}
			printf("\n");
		} else {
			assert(jsonGetObjectValue(&json, tmp));
			printf("%s\n", tmp);
		}
		
		res = jsonGetNextObjectKey(&json, tmp);
		assert(res != JSON_ERROR);
	}
	jsonClose(&json);
	return 0;
}


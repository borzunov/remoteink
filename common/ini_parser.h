#ifndef INI_PARSER_H
#define INI_PARSER_H

#include "../common/exceptions.h"


/* Supported INI file syntax:
	[Section]
	Key = Value
	; Comment 1
	# Comment 2
*/

struct IniParam {
	const char *key;
	ExcCode (*setter)(const char *key, const char *value);
	ExcCode (*getter)(const char *key, char *buffer, int buffer_size);
	int required;
};
#define INI_ANY_KEY "*"

#define INI_VALUE_FALSE "False"
#define INI_VALUE_TRUE "True"

struct IniSection {
	const char *title;
	struct IniParam *params;
};

ExcCode ini_load(const char *filename, const struct IniSection sections[]);
ExcCode ini_save(const char *filename, const struct IniSection sections[]);


#endif

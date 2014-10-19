#ifndef INI_PARSER_H
#define INI_PARSER_H


/* Supported INI file syntax:
    [Section]
    Key = Value
    ; Comment 1
    # Comment 2
*/

struct IniParam {
    const char *key;
    void (*setter)(const char *key, const char *value);
    const char *(*getter)(const char *key);
};
#define INI_ANY_KEY "*"

#define INI_VALUE_FALSE "False"
#define INI_VALUE_TRUE "True"

struct IniSection {
    const char *title;
    struct IniParam *params;
};

int ini_load(const char *filename, const struct IniSection sections[]);
int ini_save(const char *filename, const struct IniSection sections[]);


#endif

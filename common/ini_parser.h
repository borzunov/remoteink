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

#define INI_VALUE_TRUE "True"

struct IniSection {
    const char *title;
    struct IniParam *params;
};

int load_params(const char *filename, const struct IniSection sections[],
        int ignore_if_not_exists, void (*error_handler)(const char *message));
int save_params(const char *filename, const struct IniSection sections[],
        void (*error_handler)(const char *message));


#endif

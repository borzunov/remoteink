#include "exceptions.h"
#include "ini_parser.h"
#include "messages.h"

#include <stdio.h>
#include <string.h>

#define ERR_INI_PARAM_BEFORE_SECTION "Can't define parameter \"%s\" \
before section definition at \"%s\""
#define ERR_INI_UNKNOWN_SECTION "Unknown section \"%s\" at \"%s\""
#define ERR_INI_UNKNOWN_KEY "Unknown key \"%s\" in section \"%s\" at \"%s\""
#define ERR_INI_AMBIGOUS_LINE "Ambiguous line \"%s\" in \"%s\""
#define ERR_INI_SAVE_ANY_KEY "Can't save parameters declared with INI_ANY_KEY"

#define LINE_BUFFER_SIZE 256
char line_buffer[LINE_BUFFER_SIZE];

FILE *f;

void ini_close_file() {
    fclose(f);
}

int ini_load(const char *filename, const struct IniSection sections[]) {
    f = fopen(filename, "r");
    if (f == NULL) {
        throw_exc(ERR_FILE_OPEN_FOR_READING, filename);
        return 0;
    }
    push_finally(ini_close_file);
    const struct IniSection *cur_section = NULL;
    while (fgets(line_buffer, LINE_BUFFER_SIZE, f)) {
        if (ferror(f)) {
            throw_exc(ERR_FILE_READ, filename);
            return 0;
        }
        
        int len = strlen(line_buffer);
        if (line_buffer[len - 1] == '\n')
            len--;
        line_buffer[len] = 0;
        
        // Handle empty lines and comments
        if (!len || line_buffer[0] == ';' || line_buffer[0] == '#')
            continue;
        
        // Handle sections
        if (line_buffer[0] == '[' && line_buffer[len - 1] == ']') {
            line_buffer[len - 1] = 0;
            cur_section = NULL;
            int i;
            for (i = 0; sections[i].title != NULL; i++)
                if (!strcmp(line_buffer + 1, sections[i].title)) {
                    cur_section = &sections[i];
                    break;
                }
            if (cur_section == NULL) {
                throw_exc(ERR_INI_UNKNOWN_SECTION, line_buffer + 1, filename);
                return 0;
            }
            continue;
        }
        
        // Handle keys
        int i;
        for (i = 0; i < len - 2; i++)
            if (
                line_buffer[i] == ' ' && line_buffer[i + 1] == '=' &&
                line_buffer[i + 2] == ' '
            ) {
                line_buffer[i] = 0;
                
                if (cur_section == NULL) {
                    throw_exc(ERR_INI_PARAM_BEFORE_SECTION,
                            line_buffer, filename);
                    return 0;
                }
                
                int j;
                for (j = 0; cur_section->params[j].key != NULL; j++) {
                    if (
                        !strcmp(INI_ANY_KEY, cur_section->params[j].key) ||
                        !strcmp(line_buffer, cur_section->params[j].key)
                    ) {
                        cur_section->params[j].setter(
                                line_buffer, line_buffer + i + 3);
                        break;
                    }
                }
                if (cur_section->params[j].key == NULL) {
                    throw_exc(ERR_INI_UNKNOWN_KEY,
                            line_buffer, cur_section->title, filename);
                    return 0;
                }
                break;
            }
            
        if (i == len - 2) {
            throw_exc(ERR_INI_AMBIGOUS_LINE, line_buffer, filename);
            return 0;
        }
    }
    pop_finally();
    return 1;
}

int ini_save(const char *filename, const struct IniSection sections[]) {
    f = fopen(filename, "w");
    if (f == NULL) {
        throw_exc(ERR_FILE_OPEN_FOR_WRITING, filename);
        return 0;
    }
    push_finally(ini_close_file);
    int i;
    for (i = 0; sections[i].title != NULL; i++) {
        const struct IniSection *cur_section = &sections[i];
        if (fprintf(f, "[%s]\n", cur_section->title) < 0) {
            throw_exc(ERR_FILE_WRITE, filename);
            return 0;
        }
        int j;
        for (j = 0; sections[i].params[j].key != NULL; j++) {
            if (!strcmp(INI_ANY_KEY, cur_section->params[j].key)) {
                throw_exc(ERR_INI_SAVE_ANY_KEY);
                return 0;
            }
            if (fprintf(
                f, "%s = %s\n",
                cur_section->params[j].key,
                cur_section->params[j].getter(cur_section->params[j].key)
            ) < 0) {
                throw_exc(ERR_FILE_WRITE, filename);
                return 0;
            }
        }
    }
    pop_finally();
    return 1;
}

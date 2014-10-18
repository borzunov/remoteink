#include "ini_parser.h"

#include <stdio.h>
#include <string.h>

#define MESSAGE_BUFFER_SIZE 256
char message_buffer[MESSAGE_BUFFER_SIZE];

#define THROW_ERROR(...) {\
    snprintf(message_buffer, MESSAGE_BUFFER_SIZE, __VA_ARGS__);\
    error_handler(message_buffer);\
    return 0;\
}

#define LINE_BUFFER_SIZE 256
char line_buffer[LINE_BUFFER_SIZE];

int load_params(const char *filename, const struct IniSection sections[],
        int ignore_if_not_exists, void (*error_handler)(const char *message)) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        if (ignore_if_not_exists)
            return 0;
        THROW_ERROR("Can't open file \"%s\" for reading", filename);
    }
    const struct IniSection *cur_section = NULL;
    while (fgets(line_buffer, LINE_BUFFER_SIZE, f)) {
        if (ferror(f))
            THROW_ERROR("Can't read from file \"%s\"", filename);
        
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
            if (cur_section == NULL)
                THROW_ERROR("Unknown section \"%s\" at \"%s\"",
                        line_buffer + 1, filename);
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
                
                if (cur_section == NULL)
                    THROW_ERROR("Can't define parameter \"%s\" "
                            "before section definition at \"%s\"",
                            line_buffer, filename);
                
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
                if (cur_section->params[j].key == NULL)
                    THROW_ERROR(
                            "Unknown key \"%s\" in section \"%s\" at \"%s\"",
                            line_buffer, cur_section->title, filename);
                break;
            }
            
        if (i == len - 2)
            THROW_ERROR("Ambiguous line \"%s\" in \"%s\"",
                    line_buffer, filename);
    }
    if (fclose(f))
        THROW_ERROR("Can't close file \"%s\" after reading", filename);
    return 1;
}

const char *write_error = "Can't write to file \"%s\"";

int save_params(const char *filename, const struct IniSection sections[],
        void (*error_handler)(const char *message)) {
    FILE *f = fopen(filename, "r");
    if (f == NULL)
        THROW_ERROR("Can't open file \"%s\" for writing", filename);
    int i;
    for (i = 0; sections[i].title != NULL; i++) {
        const struct IniSection *cur_section = &sections[i];
        if (fprintf(f, "[%s]\n", cur_section->title) < 0)
            THROW_ERROR(write_error, filename);
        int j;
        for (j = 0; sections[i].params[j].key != NULL; j++) {
            if (!strcmp(INI_ANY_KEY, cur_section->params[j].key))
                THROW_ERROR("Can't save parameters declared with INI_ANY_KEY");
            if (fprintf(
                f, "%s = %s\n",
                cur_section->params[j].key,
                cur_section->params[j].getter(cur_section->params[j].key)
            ) < 0)
                THROW_ERROR(write_error, filename);
        }
    }
    if (fclose(f))
        THROW_ERROR("Can't close file \"%s\" after writing", filename);
    return 1;
}

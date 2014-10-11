#include "options.h"

#include <stdio.h>
#include <string.h>

char server_host[OPTION_SIZE];
int server_port = 9312;

void load_options(const char *filename) {
    // Set defaults
    strcpy(server_host, "192.168.0.101");
    
    FILE *f = fopen(filename, "r");
    if (f == NULL)
        return;
    
    enum {SECTION_UNKNOWN, SECTION_SERVER} section = SECTION_UNKNOWN;
    static char buffer[OPTION_SIZE];
    while (fgets(buffer, OPTION_SIZE, f)) {
        int buffer_len = strlen(buffer);
        while (buffer_len && (
            buffer[buffer_len - 1] == ' ' ||
            buffer[buffer_len - 1] == '\n'
        ))
            buffer_len--;
        if (!buffer_len)
            continue;
        buffer[buffer_len] = 0;
        
        static char key[OPTION_SIZE], value[OPTION_SIZE];
        if (buffer[0] == '[' && buffer[buffer_len - 1] == ']') {
            buffer[buffer_len - 1] = 0;
            
            if (!strcmp(buffer + 1, "Server"))
                section = SECTION_SERVER;
            else
                section = SECTION_UNKNOWN;
        } else
        if (sscanf(buffer, "%s = %s", key, value) == 2) {
            if (section == SECTION_SERVER) {
                if (!strcmp(key, "Host"))
                    strcpy(server_host, value);
                else
                if (!strcmp(key, "Port"))
                    sscanf(value, "%d", &server_port);
            }
        }
    }
    
    fclose(f);
}

void save_options(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (f == NULL)
        return;

    fprintf(f,
        "[Server]\n"
        "Host = %s\n"
        "Port = %d\n",
    server_host, server_port);
    fclose(f);
}

#include "../common/exceptions.h"
#include "../common/ini_parser.h"
#include "../common/messages.h"
#include "options.h"

#include <stdio.h>
#include <string.h>


#define BUFFER_SIZE 256
char server_host[BUFFER_SIZE] = "0.0.0.0";
int server_port = 9312;

void load_server_host(const char *key, const char *value) {
    strncpy(server_host, value, BUFFER_SIZE);
}

void load_server_port(const char *key, const char *value) {
    if (!(
        sscanf(value, "%d", &server_port) == 1 &&
        PORT_MIN <= server_port && server_port <= PORT_MAX
    ))
        throw_exc(ERR_INVALID_PORT, PORT_MIN, PORT_MAX);
}


int stats_enabled = 0;
char stats_file[BUFFER_SIZE] = "stats.log";

void load_stats_enabled(const char *key, const char *value) {
    if (!strcasecmp(value, INI_VALUE_FALSE))
        stats_enabled = 0;
    else
    if (!strcasecmp(value, INI_VALUE_TRUE))
        stats_enabled = 1;
    else
        throw_exc(ERR_INVALID_BOOL, key);
}

void load_stats_file(const char *key, const char *value) {
    strncpy(stats_file, value, BUFFER_SIZE);
}


const struct IniSection sections[] = {
    {"Server", (struct IniParam []) {
        {"Host", load_server_host, NULL},
        {"Port", load_server_port, NULL},
        {NULL, NULL, NULL}
    }},
    {"Stats", (struct IniParam []) {
        {"Enabled", load_stats_enabled, NULL},
        {"File", load_stats_file, NULL},
        {NULL, NULL, NULL}
    }},
    {"Shortcuts", (struct IniParam []) {
        //{"*", load_shortcut, NULL},
        {NULL, NULL}
    }},
    {NULL, NULL}
};


void load_config(const char *filename) {
    ini_load(filename, sections);
}

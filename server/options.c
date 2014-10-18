#include "../common/ini_parser.h"
#include "options.h"
#include "main.h"

#include <stdio.h>
#include <string.h>


#define BUFFER_SIZE 256
char message_buffer[BUFFER_SIZE];


char server_host[BUFFER_SIZE] = "0.0.0.0";
int server_port = 9312;

void load_server_host(const char *key, const char *value) {
    strncpy(server_host, value, BUFFER_SIZE);
}

#define PORT_MIN 1
#define PORT_MAX 49151

void load_server_port(const char *key, const char *value) {
    if (!(
        sscanf(value, "%d", &server_port) == 1 &&
        PORT_MIN <= server_port && server_port <= PORT_MAX
    )) {
        snprintf(
            message_buffer, BUFFER_SIZE,
            "Port number should be in the interval from %d to %d",
            PORT_MIN, PORT_MAX
        );
        show_error(message_buffer);
    }
}


int stats_enabled = 0;
char stats_file[BUFFER_SIZE] = "stats.log";

void load_stats_enabled(const char *key, const char *value) {
    stats_enabled = !strcasecmp(value, INI_VALUE_TRUE);
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
    load_params(filename, sections, 1, show_error);
}

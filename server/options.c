#include "../common/exceptions.h"
#include "../common/ini_parser.h"
#include "../common/messages.h"
#include "control.h"
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


struct HandlerRecord {
    const char *key;
    void (*handler)();
};

const struct HandlerRecord handlers[] = {
    {"MoveUp", move_up_handler},
    {"MoveDown", move_down_handler},
    {"MoveLeft", move_left_handler},
    {"MoveRight", move_right_handler},
    {"ResetPosition", reset_position},
    {NULL, NULL}
};

struct Shortcut shortcuts[BUFFER_SIZE];
int shortcuts_count = 0;

#define SHORTCUT_VALUE_DISABLED "None"

#define ERR_SHORTCUT_UNKNOWN_ACTION "Unknown action \"%s\" in shortcut \"%s\""
#define ERR_SHORTCUT_OVERFLOW "You can't define more than %d shortcuts"

void load_shortcut(const char *key, const char *value) {
    if (!strcmp(value, SHORTCUT_VALUE_DISABLED))
        return;
    if (shortcuts_count >= BUFFER_SIZE - 1)
        throw_exc(ERR_SHORTCUT_OVERFLOW, BUFFER_SIZE - 1);
    
    void (*handler)() = NULL;
    for (int i = 0; handlers[i].key != NULL; i++)
        if (!strcmp(key, handlers[i].key)) {
            handler = handlers[i].handler;
            break;
        }
    if (handler == NULL)
        throw_exc(ERR_SHORTCUT_UNKNOWN_ACTION, key, value);
    
    struct Hotkey hotkey = parse_hotkey(value);
    shortcuts[shortcuts_count].hotkey = hotkey;
    shortcuts[shortcuts_count].handler = handler;
    shortcuts_count++;
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
        {"*", load_shortcut, NULL},
        {NULL, NULL}
    }},
    {NULL, NULL}
};


void load_config(const char *filename) {
    ini_load(filename, sections);
    shortcuts[shortcuts_count].handler = NULL;
}

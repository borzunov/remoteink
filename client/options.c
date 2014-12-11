#include "../common/exceptions.h"
#include "../common/ini_parser.h"
#include "../common/messages.h"
#include "options.h"

#include <stdio.h>
#include <string.h>


#define BUFFER_SIZE 256
char server_host[BUFFER_SIZE] = "192.168.0.101";
int server_port = 9312;

void load_server_host(const char *key, const char *value) {
    strncpy(server_host, value, BUFFER_SIZE);
}

const char *save_server_host(const char *key) {
	return server_host;
}

void load_server_port(const char *key, const char *value) {
    if (!(
        sscanf(value, "%d", &server_port) == 1 &&
        PORT_MIN <= server_port && server_port <= PORT_MAX
    ))
        throw_exc(ERR_INVALID_PORT, PORT_MIN, PORT_MAX);
}

#define BUFFER_SIZE 256
char server_port_buffer[BUFFER_SIZE];

const char *save_server_port(const char *key) {
	snprintf(server_port_buffer, BUFFER_SIZE, "%d", server_port);
	return server_port_buffer;
}


const struct IniSection sections[] = {
    {"Server", (struct IniParam []) {
        {"Host", load_server_host, save_server_host},
        {"Port", load_server_port, save_server_port},
        {NULL, NULL, NULL}
    }},
    {NULL, NULL}
};


void load_config(const char *filename) {
    ini_load(filename, sections);
}

void save_config(const char *filename) {
	ini_save(filename, sections);
}

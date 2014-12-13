#include "../common/ini_parser.h"
#include "../common/messages.h"
#include "../common/utils.h"
#include "options.h"

#include <stdio.h>
#include <string.h>


#define BUFFER_SIZE 256
char server_host[BUFFER_SIZE] = "192.168.0.101";
int server_port = 9312;

ExcCode load_server_host(const char *key, const char *value) {
	strncpy(server_host, value, BUFFER_SIZE);
	return 0;
}

ExcCode save_server_host(const char *key, char *buffer, int buffer_size) {
	strncpy(buffer, server_host, buffer_size);
	return 0;
}

ExcCode load_server_port(const char *key, const char *value) {
	TRY(parse_port(value, &server_port));
	return 0;
}

ExcCode save_server_port(const char *key, char *buffer, int buffer_size) {
	snprintf(buffer, buffer_size, "%d", server_port);
	return 0;
}


int orientation = 0;

const char *orientation_captions[] = {"Portrait", "Landscape"};

#define ERR_INCORRECT_ORIENTATION "Incorrect orientation type"

ExcCode load_orientation(const char *key, const char *value) {
	for (int i = 0; i < 2; i++)
		if (!strcmp(value, orientation_captions[i])) {
			orientation = i;
			return 0;
		}
	THROW(ERR_INCORRECT_ORIENTATION);
}

ExcCode save_orientation(const char *key, char *buffer, int buffer_size) {
	strncpy(buffer, orientation_captions[orientation], buffer_size);
	return 0;
}


const struct IniSection sections[] = {
	{"Server", (struct IniParam []) {
		{"Host", load_server_host, save_server_host},
		{"Port", load_server_port, save_server_port},
		{NULL, NULL, NULL}
	}},
	{"Client", (struct IniParam []) {
		{"Orientation", load_orientation, save_orientation},
		{NULL, NULL, NULL}
	}},
	{NULL, NULL}
};


ExcCode load_config(const char *filename) {
	TRY(ini_load(filename, sections));
	return 0;
}

ExcCode save_config(const char *filename) {
	TRY(ini_save(filename, sections));
	return 0;
}

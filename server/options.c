#include "main.h"
#include "options.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *server_host = "0.0.0.0";
int server_port = 9312;
int screen_left = 0;
int screen_top = 0;
const char *stats_file = 0;

char message_buffer[256];

void show_version() {
    printf(
        "Server for tool for using E-Ink reader as computer monitor\n"
        "Copyright (c) 2013-2014 Alexander Borzunov\n"
    );
}

void show_help() {
    show_version();
    printf(
        "\n"
        "Standard options:\n"
        "    -h, --help          Display this help and exit.\n"
        "    --version           Output version info and exit.\n"
        "\n"
        "Network options:\n"
        "    -H, --host HOST     Set host for server.\n"
        "                        Default: %s\n"
        "    -P, --port PORT     Set post for server.\n"
        "                        Default: %d\n"
        "\n"
        "Image options:\n"
        "    -x, --position X,Y  Set position of left top corner of grabbed part\n"
        "                        of the screen. If reader's screen greater than monitor,\n"
        "                        you can set positions with negative X and Y\n"
        "                        to move image to the center of reader's screen.\n"
        "                        Default: %d,%d\n"
        "\n"
        "Debug options:\n"
        "    --stats FILE        Collect statistics about perfomance and data transfer\n"
        "                        and save it to FILE. Some warnings may be displayed to\n"
        "                        standard output.\n",
        server_host, server_port, screen_left, screen_top
    );
}

const char *get_key_value(int argc, char *argv[], int *index, const char *key) {
    int key_len = strlen(key);
    if (!strncmp(argv[*index], key, key_len)) {
        if (!(key[0] == '-' && key[1] != '-') && argv[*index][key_len] == '=')
            return argv[*index] + key_len;
        if (!argv[*index][key_len]) {
            if (*index != argc - 1) {
                (*index)++;
                return argv[*index];
            }
                        
            sprintf(message_buffer, "Argument \"%s\" requires value", key);
            show_error(message_buffer);
        }
    }
    return 0;
}

void parse_options(int argc, char *argv[]) {
    int i;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            show_help();
            exit(EXIT_SUCCESS);
        }
        if (!strcmp(argv[i], "--version")) {
            show_version();
            exit(EXIT_SUCCESS);
        }

        const char *value;
        if ((value = get_key_value(argc, argv, &i, "-H")) ||
                (value = get_key_value(argc, argv, &i, "--host")))
            server_host = value;
        else
        if ((value = get_key_value(argc, argv, &i, "-P")) ||
                (value = get_key_value(argc, argv, &i, "--port"))) {
            if (sscanf(value, "%d", &server_port) != 1)
                show_error("Incorrect port number");
        } else
        if ((value = get_key_value(argc, argv, &i, "-x")) ||
                (value = get_key_value(argc, argv, &i, "--position"))) {
            if (sscanf(value, "%d,%d", &screen_left, &screen_top) != 2)
                show_error("Incorrect position. The position should be "
                           "a string with format \"X,Y\" e.g. \"100,100\".");
        } else
        if ((value = get_key_value(argc, argv, &i, "--stats")))
            stats_file = value;
        else {
            sprintf(message_buffer, "Incorrect argument \"%s\"", argv[i]);
            show_error(message_buffer);
        }
    }
}

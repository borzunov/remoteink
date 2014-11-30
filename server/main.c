#include "../common/exceptions.h"
#include "../common/messages.h"
#include "../common/protocol.h"
#include "control.h"
#include "options.h"
#include "profiler.h"
#include "screenshot.h"
#include "shortcuts.h"
#include "transfer.h"

#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define ERR_SIGPIPE "Connection closed"

#define ERR_ARG_EXCESS "Excess argument is present"

#define ERR_CLIENT_VERSION "Incompatible client version"


#define MAX_FPS 20

// Screen regions may be processed separately. This is useful,
// for example, in the word processors, when at typing we have only two small
// updates in the middle of the screen (text) and in the bottom of the screen
// (the counters of lines and characters). Then it will be updated not one big
// rectangle of the screen, but two small ones.
#define WIDTH_DIV 1
#define HEIGHT_DIV 3

int has_stats = 0;


int serv_fd, conn_fd;


void exit_handler() {
    // Handling errors here is redundant because we already have an exception
    close(conn_fd);
    close(serv_fd);
    
    if (has_stats && stats_enabled) {
        profiler_save(stats_file);
        printf("[*] Stats saved to \"%s\"\n", stats_file);
    }
}

void sigint_handler(int code) {
    exit_handler();
    exit(EXIT_SUCCESS);
}

void sigpipe_handler(int code) {
    throw_exc(ERR_SIGPIPE);
}


int in_show_error = 0;

void show_error(const char *error) {
    if (!in_show_error) {
        in_show_error = 1;
        exit_handler();
    }
    fprintf(stderr, "[-] Error: %s\n", error);
    exit(EXIT_FAILURE);
}


const char *config_filename = "inkmonitor-server.ini";


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
        "Usage:\n"
        "    inkmonitor-server [OPTION] [CONFIG]\n"
        "\n"
        "Standard options:\n"
        "    -h, --help    Display this help and exit.\n"
        "    --version     Output version info and exit.\n"
        "\n"
        "Parameters:\n"
        "    CONFIG        INI configuration file.\n"
        "                  Default: %s\n",
        config_filename
    );
}


void parse_arguments(int argc, char *argv[]) {
    int pos = 0;
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
        
        if (pos == 0)
            config_filename = argv[i];
        else
            throw_exc(ERR_ARG_EXCESS);
        pos++;
    }
}


Display *display;

void *start_handle_shortcuts(void *arg) {
    handle_shortcuts(shortcuts);
    return NULL;
}


void send_first_frame(Imlib_Image *image, Window window, DATA32 **data) {
    *image = screenshot_get(window,
            frame_left, frame_top, client_width, client_height);
    imlib_context_set_image(*image);
    *data = imlib_image_get_data_for_reading_only();
    
    image_send_all(conn_fd, *data, client_width, client_height);
}

char conn_check_char = CONN_CHECK;

void send_next_frame(Imlib_Image *image, Window window, DATA32 **data,
        int region_width, int region_height) {
    // Check whether connection is dead. If so, SIGPIPE will be sent.
    if (write(conn_fd, &conn_check_char, sizeof (char)) < 0)
        throw_exc(ERR_SOCK_SEND);
    
    profiler_start(STAGE_SHOT);    
    Imlib_Image next_image = screenshot_get(window,
            frame_left, frame_top, client_width, client_height);
    imlib_context_set_image(next_image);
    DATA32 *next_data = imlib_image_get_data_for_reading_only();
    profiler_finish(STAGE_SHOT);
    
    unsigned i, j;
    for (i = 0; i < HEIGHT_DIV; i++)
        for (j = 0; j < WIDTH_DIV; j++)
            image_send_diff(
                conn_fd, *data, next_data,
                client_width, client_height,
                j * region_width, i * region_height,
                region_width, region_height
            );
    gib_imlib_free_image_and_decache(*image);
    *image = next_image;
    *data = next_data;

    traffic_uncompressed += client_width * client_height * 4;
    has_stats = 1;
}

void setup_server() {    
    serv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_fd < 0)
        throw_exc(ERR_SOCK_CREATE);

    struct hostent *serv = gethostbyname(server_host);
    if (serv == NULL)
        throw_exc(ERR_SOCK_RESOLVE);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, serv->h_addr, serv->h_length);
    serv_addr.sin_port = htons(server_port);
    
    if (bind(serv_fd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0)
        throw_exc(ERR_SOCK_BIND, server_host, server_port);

    listen(serv_fd, 0);
}

void accept_client() {
    struct sockaddr_in *client_addr;
    socklen_t client_len = sizeof (struct sockaddr_in);
    conn_fd = accept(serv_fd, (struct sockaddr *) &client_addr, &client_len);
    if (conn_fd < 0)
        throw_exc(ERR_SOCK_ACCEPT);

    int header_len = strlen(HEADER);
    if (read(conn_fd, buffer, header_len + 1) != header_len + 1 ||
            buffer[header_len] != '\n' || strncmp(buffer, HEADER, header_len))
        throw_exc(ERR_CLIENT_VERSION);
}

void perform_handshake() {
    if (read(conn_fd, buffer, COORD_SIZE * 2) != COORD_SIZE * 2)
        throw_exc(ERR_SOCK_RECV);
    int i = -1;
    READ_COORD(client_width, buffer, i);
    READ_COORD(client_height, buffer, i);
}

#define MIN_FRAME_DURATION ((NSECS_PER_SEC) / (MAX_FPS))

void perform_mainloop(Window window) {
    Imlib_Image image;
    DATA32 *data;
    send_first_frame(&image, window, &data);
    
    unsigned region_width = client_width / WIDTH_DIV;
    unsigned region_height = client_height / HEIGHT_DIV;
    while (1) {
        long long frame_start_time = get_time_nsec();
        
        send_next_frame(&image, window, &data,
                region_width, region_height);

        long long frame_duration = get_time_nsec() - frame_start_time;
        long long sleep_time = MIN_FRAME_DURATION - frame_duration;
        if (sleep_time > 0)
            usleep(sleep_time / NSECS_PER_MSEC);
        else
        if (stats_enabled && sleep_time < 0)
            printf("[~] Too slow! Frame duration is %lld ms.\n",
                    frame_duration / NSECS_PER_MSEC);
    }
}

int main(int argc, char *argv[]) {
    set_except(show_error);
    signal(SIGINT, sigint_handler);
    signal(SIGPIPE, sigpipe_handler);
    
    printf("InkMonitor v0.01 Alpha 4 - Server\n");
    parse_arguments(argc, argv);
    
    Screen *screen;
    Window window;
    screenshot_init(&screen, &window);
    
    screen_width = screen->width;
    screen_height = screen->height;
    window_left = 0;
    window_top = 0;
    window_width = screen_width;
    window_height = screen_height;
    reset_position();
    
    shortcuts_init();
    
    load_config(config_filename);
    printf(
        "    Configuration: %s\n"
        "    Monitor resolution: %dx%d\n",
        config_filename, screen_width, screen_height
    );
    
    pthread_t shortcuts_thread;
    int shortcuts_res;
    if (pthread_create(&shortcuts_thread, NULL,
            start_handle_shortcuts, &shortcuts_res))
        throw_exc(ERR_THREAD_CREATE);
    
    setup_server();
    printf("[+] Listen on %s:%d\n", server_host, server_port);
    accept_client();
    printf("[+] Accepted client connection\n");
    perform_handshake();
    printf("    Reader resolution: %ux%u\n",
            client_width, client_height);
    perform_mainloop(window);
    return 0;
}

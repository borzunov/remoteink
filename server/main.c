#include "../common/exceptions.h"
#include "../common/messages.h"
#include "../common/protocol.h"
#include "../common/utils.h"
#include "control.h"
#include "options.h"
#include "profiler.h"
#include "screen.h"
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


void show_error(const char *error, int is_fatal) {
	if (is_fatal) {
		fprintf(stderr, "[-] Fatal Error: %s\n", error);
		exit(EXIT_FAILURE);
	} else
		fprintf(stderr, "[-] Error: %s\n", error);
}


#define CONFIG_FILENAME_SIZE 256
char config_filename[CONFIG_FILENAME_SIZE];


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


ExcCode parse_arguments(int argc, char *argv[]) {
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
		
		if (pos == 0) {
			strncpy(config_filename, argv[i], CONFIG_FILENAME_SIZE - 1);
			config_filename[CONFIG_FILENAME_SIZE - 1] = '\0';
		} else
			THROW(ERR_ARG_EXCESS);
		pos++;
	}
	return 0;
}


Display *display;

void *start_handle_shortcuts(void *arg) {
	//handle_shortcuts(shortcuts);
	return NULL;
}


ExcCode track_focused_window() {
	xcb_window_t window;
	if (window_tracking_enabled) {
		TRY(window_get_focused(&window));
	} else {
		TRY(window_get_root(&window));
	}
	TRY(activate_window_context(window));
	return 0;
}


int serv_fd, conn_fd;


int has_stats = 0;

ExcCode send_first_frame(unsigned **image_data) {
	TRY(track_focused_window());
	TRY(screenshot_get(
			active_context->frame_left, active_context->frame_top,
			client_width, client_height, image_data));
	TRY(image_send_all(conn_fd, *image_data, client_width, client_height));
	return 0;
}

char conn_check_char = CONN_CHECK;

ExcCode send_next_frame(unsigned **image_data,
		int region_width, int region_height) {
	// Check whether connection is dead. If so, SIGPIPE will be sent.
	if (write(conn_fd, &conn_check_char, sizeof (char)) < 0)
		THROW(ERR_SOCK_WRITE);
	
	profiler_start(STAGE_SHOT);   
	TRY(track_focused_window());
	unsigned *next_image_data;
	TRY(screenshot_get(
			active_context->frame_left, active_context->frame_top,
			client_width, client_height, &next_image_data));
	profiler_finish(STAGE_SHOT);
	
	unsigned i, j;
	for (i = 0; i < HEIGHT_DIV; i++)
		for (j = 0; j < WIDTH_DIV; j++)
			TRY(image_send_diff(
				conn_fd, *image_data, next_image_data,
				client_width, client_height,
				j * region_width, i * region_height,
				region_width, region_height
			));
	free(*image_data);
	*image_data = next_image_data;

	traffic_uncompressed += client_width * client_height * 4;
	has_stats = 1;
	return 0;
}


int handle_client_flag = 0;

ExcCode client_accept() {
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof (struct sockaddr_in);
	conn_fd = accept(serv_fd, (struct sockaddr *) &client_addr, &client_len);
	if (conn_fd < 0)
		THROW(ERR_SOCK_ACCEPT);
		
	handle_client_flag = 1;
	return 0;
}

ExcCode client_handshake() {
	int header_len = strlen(HEADER);
	if (read(conn_fd, buffer, header_len + 1) != header_len + 1 ||
			buffer[header_len] != '\n' || strncmp(buffer, HEADER, header_len))
		THROW(ERR_CLIENT_VERSION);
		
	if (read(conn_fd, buffer, COORD_SIZE * 2) != COORD_SIZE * 2)
		THROW(ERR_SOCK_READ);
	int i = -1;
	READ_COORD(client_width, buffer, i);
	READ_COORD(client_height, buffer, i);
	return 0;
}

#define MIN_FRAME_DURATION ((NSECS_PER_SEC) / (MAX_FPS))

ExcCode client_mainloop() {
	unsigned *image_data;
	TRY(send_first_frame(&image_data));
	
	unsigned region_width = client_width / WIDTH_DIV;
	unsigned region_height = client_height / HEIGHT_DIV;
	while (handle_client_flag) {
		long long frame_start_time = get_time_nsec();
		
		TRY(send_next_frame(&image_data, region_width, region_height));

		long long frame_duration = get_time_nsec() - frame_start_time;
		long long sleep_time = MIN_FRAME_DURATION - frame_duration;
		if (sleep_time > 0)
			usleep(sleep_time / NSECS_PER_MSEC);
		else
		if (stats_enabled && sleep_time < 0)
			printf("[~] Too slow! Frame duration is %lld ms.\n",
					frame_duration / NSECS_PER_MSEC);
	}
	return 0;
}

void client_disconnect() {
	handle_client_flag = 0;
}


ExcCode server_create() {
	serv_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (serv_fd < 0)
		THROW(ERR_SOCK_CREATE);
	return 0;
}

ExcCode server_setup() {
	struct hostent *serv = gethostbyname(server_host);
	if (serv == NULL)
		THROW(ERR_SOCK_RESOLVE, server_host);
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr.s_addr, serv->h_addr, serv->h_length);
	serv_addr.sin_port = htons(server_port);
	
	int optval = 1;
	size_t optlen = sizeof(int);
	setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &optval, optlen);
	
	if (bind(serv_fd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0)
		THROW(ERR_SOCK_BIND, server_host, server_port);

	listen(serv_fd, 0);
	return 0;
}

ExcCode server_handle_client() {
	TRY(client_accept());
	
	#undef FINALLY
	#define FINALLY {\
		close(conn_fd);\
		handle_client_flag = 0;\
	}
	
	printf("[+] Accepted client connection\n");
	TRY(client_handshake());
	printf("    Reader resolution: %ux%u\n", client_width, client_height);
	TRY(client_mainloop());
	
	FINALLY;
	#undef FINALLY
	#define FINALLY
	return 0;
}

void server_shutdown() {
	close(serv_fd);
	if (has_stats && stats_enabled && !profiler_save(stats_filename))
		printf("[*] Stats saved to \"%s\"\n", stats_filename);
}


ExcCode serve(int argc, char *argv[]) {
	printf("InkMonitor v0.01 Alpha 4 - Server\n");
	
	get_default_config_path("inkmonitor-server.ini",
			config_filename, CONFIG_FILENAME_SIZE);
	TRY(parse_arguments(argc, argv));
	
	TRY(screenshot_init(&screen_width, &screen_height));
	TRY(shortcuts_init());
	
	TRY(load_config(config_filename));
	printf(
		"    Configuration: %s\n"
		"    Monitor resolution: %dx%d\n",
		config_filename, screen_width, screen_height
	);
	
	pthread_t shortcuts_thread;
	int shortcuts_res;
	if (pthread_create(&shortcuts_thread, NULL,
			start_handle_shortcuts, &shortcuts_res))
		THROW(ERR_THREAD_CREATE);
	
	TRY(server_create());
	
	#undef FINALLY
	#define FINALLY server_shutdown();
	
	TRY(server_setup());
	printf("[+] Listen on %s:%d\n", server_host, server_port);
	while (1)
		if (server_handle_client())
			show_error(exc_message, 0);
	
	FINALLY;
	#undef FINALLY
	#define FINALLY
	return 0;
}

void sigint_handler(int code) {
	if (handle_client_flag) {
		client_disconnect();
		printf("[*] Client disconnected\n");
	} else {
		server_shutdown();
		exit(EXIT_SUCCESS);
	}
}

void sigpipe_handler(int code) {
	if (handle_client_flag) {
		client_disconnect();
		show_error(ERR_SIGPIPE, 0);
	} else {
		server_shutdown();
		show_error(ERR_SIGPIPE, 1);
	}
}

int main(int argc, char *argv[]) {
	signal(SIGINT, sigint_handler);
	signal(SIGPIPE, sigpipe_handler);
	
	if (serve(argc, argv))
		show_error(exc_message, 1);
	return 0;
}

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

#include <crypt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


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
		"Copyright (c) 2013-2015 Alexander Borzunov\n"
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


#define ERR_ARG_EXCESS "Excess argument is present"

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


void *handle_shortcuts(void *arg) {
	if (shortcuts_handle_start(shortcuts))
		show_error(exc_message, 0);
	return NULL;
}


void track_focused_window() {
	if (control_context_get() != NULL)
		control_update_frame();
	
	xcb_window_t window;
	if (!window_tracking_enabled || screen_get_focused(&window))
		screen_get_root(&window);
	control_context_select(window);
}


int serv_fd, conn_fd;


ExcCode return_cursor() {
	int x, y, same_screen;
	if (screen_cursor_get_position(&x, &y, &same_screen))
		return 0;
	const struct WindowContext *context = control_context_get();
	int new_x = MAX(x, context->frame_left);
	new_x = MIN(new_x, context->frame_left + context->frame_width);
	int new_y = MAX(y, context->frame_top);
	new_y = MIN(new_y, context->frame_top + context->frame_height);
	if (!same_screen || x != new_x || y != new_y)
		screen_cursor_set_position(new_x, new_y);
	return 0;
}

ExcCode image_expand(Imlib_Image *image, int dest_width, int dest_height) {
	imlib_context_set_image(*image);
	int source_width = imlib_image_get_width();
	int source_height = imlib_image_get_height();
	if (source_width == dest_width && source_height == dest_height)
		return 0;
	Imlib_Image dest_image = imlib_create_image(dest_width, dest_height);
	if (dest_image == NULL)
		THROW(ERR_IMAGE);
	
	imlib_context_set_image(dest_image);
	imlib_context_set_color(0, 0, 0, 255);
	imlib_image_fill_rectangle(0, 0, dest_width, dest_height);
	imlib_blend_image_onto_image(*image, 0, 0, 0, source_width, source_height,
			(dest_width - source_width) / 2, (dest_height - source_height) / 2,
			source_width, source_height);
	
	imlib_context_set_image(*image);
	imlib_free_image_and_decache();
	*image = dest_image;
	return 0;
}

ExcCode image_resize(Imlib_Image *image, int dest_width, int dest_height) {
	imlib_context_set_image(*image);
	int source_width = imlib_image_get_width();
	int source_height = imlib_image_get_height();
	if (source_width == dest_width && source_height == dest_height)
		return 0;
	Imlib_Image dest_image = imlib_create_cropped_scaled_image(
			0, 0, source_width, source_height, dest_width, dest_height);
	imlib_free_image_and_decache();
	if (dest_image == NULL)
		THROW(ERR_IMAGE);
	*image = dest_image;
	return 0;
}

#define LABEL_SHOW_TIME_NSEC (2 * (NSECS_PER_SEC))

#define LABEL_BOX_MARGIN 25
#define LABEL_BOX_PADDING 5

ExcCode draw_label(Imlib_Image image) {
	const char *text;
	long long creation_time_nsec;
	control_label_get(&text, &creation_time_nsec);
	if (text == NULL)
		return 0;
	if (get_time_nsec() - creation_time_nsec > LABEL_SHOW_TIME_NSEC) {
		text = NULL;
		return 0;
	}
	
	int label_width, label_height;
	imlib_get_text_size(text, &label_width, &label_height);
	imlib_context_set_image(image);
	int box_left = LABEL_BOX_MARGIN;
	int box_top = imlib_image_get_height() -
			LABEL_BOX_MARGIN - 2 * LABEL_BOX_PADDING - label_height;
	int box_width = 2 * LABEL_BOX_PADDING + label_width;
	int box_height = 2 * LABEL_BOX_PADDING + label_height;
	imlib_context_set_color(255, 255, 255, 255);
	imlib_image_fill_rectangle(box_left, box_top, box_width, box_height);
	imlib_context_set_color(0, 0, 0, 255);
	imlib_image_draw_rectangle(box_left, box_top, box_width, box_height);
	imlib_text_draw(box_left + LABEL_BOX_PADDING, box_top + LABEL_BOX_PADDING,
			text);
	return 0;
}

ExcCode image_turn_to_data(Imlib_Image image, unsigned **res) {
	// Note: *res will be set to pointer to a buffer that contains image
	//       pixels encoded as RGBX. This pointer should be freed by caller.
	imlib_context_set_image(image);
	int data_size = imlib_image_get_width() * imlib_image_get_height() *
			sizeof (unsigned);
	*res = malloc(data_size);
	if (*res == NULL)
		THROW(ERR_MALLOC);
	unsigned *data = imlib_image_get_data_for_reading_only();
	memcpy(*res, data, data_size);
	imlib_free_image_and_decache();
	return 0;
}

ExcCode get_screenshot_data(unsigned **image_data,
		int client_width, int client_height) {
	track_focused_window();
	if (cursor_capturing_enabled)
		TRY(return_cursor());
	const struct WindowContext *context = control_context_get();
	Imlib_Image image;
	TRY(screen_shot(
			context->frame_left, context->frame_top,
			context->frame_width, context->frame_height,
			&image));
	TRY(image_expand(&image, context->frame_width, context->frame_height));
	TRY(image_resize(&image, client_width, client_height));
	TRY(draw_label(image));
	TRY(image_turn_to_data(image, image_data));
	return 0;
}


int has_stats = 0;

ExcCode send_first_frame(unsigned **image_data,
		int client_width, int client_height) {
	pthread_mutex_lock(&control_lock);
	#undef FINALLY
	#define FINALLY pthread_mutex_unlock(&control_lock);
	
	TRY(get_screenshot_data(image_data, client_width, client_height));
	TRY(transfer_image_send_all(conn_fd,
			*image_data, client_width, client_height));
	
	FINALLY;
	#undef FINALLY
	#define FINALLY
	return 0;
}

char conn_check_char = CONN_CHECK;

ExcCode send_next_frame(unsigned **image_data,
		int client_width, int client_height,
		int region_width, int region_height) {
	// Check whether connection is dead. If so, SIGPIPE will be sent.
	if (write(conn_fd, &conn_check_char, sizeof (char)) < 0)
		THROW(ERR_SOCK_WRITE);
	
	pthread_mutex_lock(&control_lock);
	#undef FINALLY
	#define FINALLY pthread_mutex_unlock(&control_lock);
	
	profiler_start(STAGE_SHOT);
	unsigned *next_image_data;
	TRY(get_screenshot_data(&next_image_data, client_width, client_height));
	profiler_finish(STAGE_SHOT);
	
	unsigned i, j;
	for (i = 0; i < height_divisor; i++)
		for (j = 0; j < width_divisor; j++)
			TRY(transfer_image_send_diff(
				conn_fd, *image_data, next_image_data,
				client_width, client_height,
				j * region_width, i * region_height,
				region_width, region_height
			));
	free(*image_data);
	*image_data = next_image_data;

	profiler_traffic_count_uncompressed(client_width * client_height * 4);
	has_stats = 1;
	
	FINALLY;
	#undef FINALLY
	#define FINALLY
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

int are_strings_equal_stable(const char *a, const char *b) {
	// Compare strings "a" and "b" in constant time to prevent timing attacks
	int res = 1;
	int i;
	for (i = 0; a[i] & b[i]; i++)
		res &= (a[i] == b[i]);
	if (a[i] | b[i])
		return 0;
	return res;
}

#define ERR_CRYPT "Failed to verify password's hash"

ExcCode check_password(const char *suggested_password, int *is_correct) {
	if (expected_password[0] == '$') {
		const char *expected_hash = expected_password;
		const char *suggested_hash = crypt(suggested_password, expected_hash);
		if (suggested_hash == NULL)
			THROW(ERR_CRYPT);
		*is_correct = are_strings_equal_stable(suggested_hash, expected_hash);
	} else
		*is_correct = are_strings_equal_stable(
				suggested_password, expected_password);
	return 0;
}

#define ERR_CLIENT_VERSION "Incompatible client version"

#define TRANSFER_BUFFER_SIZE 256
char transfer_buffer[TRANSFER_BUFFER_SIZE];

ExcCode client_handshake(int *client_width, int *client_height) {
	const char *line;
	TRY(transfer_recv_string(conn_fd, &line));
	if (strcmp(line, HEADER))
		THROW(ERR_CLIENT_VERSION);
		
	TRY(transfer_recv_string(conn_fd, &line));
	int is_correct;
	TRY(check_password(line, &is_correct));
	transfer_buffer[0] = is_correct ? PASSWORD_CORRECT : PASSWORD_WRONG;
	if (write(conn_fd, transfer_buffer, 1) < 0)
		THROW(ERR_SOCK_WRITE);
	if (!is_correct)
		THROW(ERR_WRONG_PASSWORD);
		
	if (read(conn_fd, transfer_buffer, COORD_SIZE * 2) != COORD_SIZE * 2)
		THROW(ERR_SOCK_READ);
	int i = -1;
	READ_COORD(*client_width, transfer_buffer, i);
	READ_COORD(*client_height, transfer_buffer, i);
	return 0;
}

ExcCode client_mainloop(int client_width, int client_height) {
	control_label_set("Connected");
	
	unsigned *image_data;
	TRY(send_first_frame(&image_data, client_width, client_height));
	
	unsigned region_width = client_width / width_divisor;
	unsigned region_height = client_height / height_divisor;
	while (handle_client_flag) {
		long long frame_start_time = get_time_nsec();
		
		TRY(send_next_frame(&image_data, client_width, client_height,
				region_width, region_height));

		long long frame_duration = get_time_nsec() - frame_start_time;
		long long sleep_time = NSECS_PER_SEC / max_fps - frame_duration;
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
	profiler_traffic_init();
	
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
	int client_width, client_height;
	TRY(client_handshake(&client_width, &client_height));
	control_client_dimensions_set(client_width, client_height);
	printf("    Reader resolution: %ux%u\n", client_width, client_height);
	TRY(client_mainloop(client_width, client_height));
	
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


ExcCode screen_of_display(xcb_connection_t *c, int screen,
		xcb_screen_t **res) {
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(c));
	for (int i = 0; iter.rem; i++) {
		if (i == screen) {
			*res = iter.data;
			return 0;
		}
		xcb_screen_next(&iter);
	}
	THROW(ERR_X_REQUEST);
}

ExcCode serve(int argc, char *argv[]) {
	printf("InkMonitor v0.01 Alpha 4 - Server\n");
	
	get_default_config_path("inkmonitor-server.ini",
			config_filename, CONFIG_FILENAME_SIZE);
	TRY(parse_arguments(argc, argv));
	
	int default_screen_no;
	xcb_connection_t *display = xcb_connect(NULL, &default_screen_no);
	if (xcb_connection_has_error(display))
		THROW(ERR_X_CONNECT);
	xcb_screen_t *screen;
	TRY(screen_of_display(display, default_screen_no, &screen));
	int screen_width = screen->width_in_pixels;
	int screen_height = screen->height_in_pixels;
	xcb_window_t root = screen->root;
	control_screen_dimensions_set(screen_width, screen_height);
	
	TRY(screen_init(display, screen, root));
	TRY(shortcuts_init(display, screen, root));
	
	TRY(options_config_load(config_filename));
	printf(
		"    Configuration: %s\n"
		"    Monitor resolution: %dx%d\n",
		config_filename, screen_width, screen_height
	);
	
	Imlib_Font label_font = imlib_load_font(label_font_name);
	if (label_font == NULL)
		THROW(ERR_FONT, label_font_name);
	imlib_context_set_font(label_font);
	
	pthread_t shortcuts_thread;
	int shortcuts_res;
	if (pthread_mutex_init(&control_lock, NULL))
		THROW(ERR_MUTEX_INIT);
	if (pthread_create(&shortcuts_thread, NULL,
			handle_shortcuts, &shortcuts_res))
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

#define ERR_SIGPIPE "Connection closed"

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

#include "../common/ini_parser.h"
#include "../common/messages.h"
#include "../common/utils.h"
#include "control.h"
#include "options.h"

#include <stdio.h>
#include <string.h>


#define SERVER_HOST_SIZE 256
char server_host[SERVER_HOST_SIZE];
int server_port;

ExcCode load_server_host(const char *key, const char *value) {
	strncpy(server_host, value, SERVER_HOST_SIZE - 1);
	server_host[SERVER_HOST_SIZE - 1] = '\0';
	return 0;
}

ExcCode load_server_port(const char *key, const char *value) {
	TRY(parse_int(key, value, PORT_MIN, PORT_MAX, &server_port));
	return 0;
}


int max_fps;

ExcCode load_max_fps(const char *key, const char *value) {
	TRY(parse_int(key, value, 1, 100, &max_fps));
	return 0;
}

#define MAX_DIM_DIVISOR 5
int width_divisor, height_divisor;

ExcCode load_width_divisor(const char *key, const char *value) {
	TRY(parse_int(key, value, 1, MAX_DIM_DIVISOR, &width_divisor));
	return 0;
}

ExcCode load_height_divisor(const char *key, const char *value) {
	TRY(parse_int(key, value, 1, MAX_DIM_DIVISOR, &height_divisor));
	return 0;
}


int window_tracking_enabled;

ExcCode load_window_tracking_enabled(const char *key, const char *value) {
	TRY(parse_bool(key, value, &window_tracking_enabled));
	return 0;
}

double default_scale;

ExcCode load_default_scale(const char *key, const char *value) {
	TRY(parse_double(key, value, MIN_SCALE, MAX_SCALE, &default_scale));
	return 0;
}


int stats_enabled;
#define STATS_FILENAME_SIZE 256
char stats_filename[STATS_FILENAME_SIZE];

ExcCode load_stats_enabled(const char *key, const char *value) {
	TRY(parse_bool(key, value, &stats_enabled));
	return 0;
}

ExcCode load_stats_filename(const char *key, const char *value) {
	strncpy(stats_filename, value, STATS_FILENAME_SIZE - 1);
	stats_filename[STATS_FILENAME_SIZE - 1] = '\0';
	return 0;
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
	{"ZoomIn", zoom_in_handler},
	{"ZoomOut", zoom_out_handler},
	{"ResetScale", reset_scale},
	{"ToggleWindowTracking", toogle_window_tracking_handler},
	{"AdjustWindowSize", adjust_window_size_handler},
	{NULL, NULL}
};

#define SHORTCUTS_MAX_COUNT 256
struct Shortcut shortcuts[SHORTCUTS_MAX_COUNT];
int shortcuts_count = 0;

#define SHORTCUT_VALUE_DISABLED "None"

#define ERR_SHORTCUT_UNKNOWN_ACTION "Unknown action \"%s\" in shortcut \"%s\""
#define ERR_SHORTCUT_OVERFLOW "You can't define more than %d shortcuts"

ExcCode load_shortcut(const char *key, const char *value) {
	if (!strcmp(value, SHORTCUT_VALUE_DISABLED))
		return 0;
	if (shortcuts_count >= SHORTCUTS_MAX_COUNT - 1)
		THROW(ERR_SHORTCUT_OVERFLOW, SHORTCUTS_MAX_COUNT - 1);
	
	void (*handler)() = NULL;
	for (int i = 0; handlers[i].key != NULL; i++)
		if (!strcmp(key, handlers[i].key)) {
			handler = handlers[i].handler;
			break;
		}
	if (handler == NULL)
		THROW(ERR_SHORTCUT_UNKNOWN_ACTION, key, value);
	
	struct Hotkey hotkey;
	TRY(parse_hotkey(value, &hotkey));
	shortcuts[shortcuts_count].hotkey = hotkey;
	shortcuts[shortcuts_count].handler = handler;
	shortcuts_count++;
	return 0;
}


const struct IniSection sections[] = {
	{"Server", (struct IniParam []) {
		{"Host", load_server_host, NULL, 1},
		{"Port", load_server_port, NULL, 1},
		{NULL}
	}},
	{"Monitor", (struct IniParam []) {
		{"MaxFPS", load_max_fps, NULL, 1},
		{"WidthDivisor", load_width_divisor, NULL, 1},
		{"HeightDivisor", load_height_divisor, NULL, 1},
		{NULL}
	}},
	{"Defaults", (struct IniParam []) {
		{"WindowTrackingEnabled", load_window_tracking_enabled, NULL, 1},
		{"Scale", load_default_scale, NULL, 1},
		{NULL}
	}},
	{"Stats", (struct IniParam []) {
		{"Enabled", load_stats_enabled, NULL, 1},
		{"File", load_stats_filename, NULL, 1},
		{NULL}
	}},
	{"Shortcuts", (struct IniParam []) {
		{"*", load_shortcut, NULL, 0},
		{NULL}
	}},
	{NULL}
};


ExcCode load_config(const char *filename) {
	TRY(ini_load(filename, sections));
	shortcuts[shortcuts_count].handler = NULL;
	return 0;
}

#include "control.h"
#include "screen.h"

#include <stdlib.h>

int screen_width, screen_height, client_width, client_height,
		window_left, window_top, window_width, window_height;

struct WindowContext *contexts_list = NULL;

struct WindowContext *active_context = NULL;
int window_tracking_enabled = 1;

ExcCode activate_window_context(xcb_window_t window) {
	TRY(window_get_geometry(window,
			&window_left, &window_top, &window_width, &window_height));
	
	if (active_context != NULL && active_context->window == window)
		return 0;
	struct WindowContext *cur = contexts_list;
	while (cur != NULL) {
		if (cur->window == window) {
			active_context = cur;
			return 0;
		}
		cur = cur->next;
	}
	
	cur = (struct WindowContext *) malloc(sizeof (struct WindowContext));
	cur->window = window;
	cur->next = contexts_list;
	contexts_list = cur;
	active_context = cur;
	
	reset_position();
	return 0;
}


int min(int a, int b) {
	return (a < b) ? a : b;
}

int max(int a, int b) {
	return (a > b) ? a : b;
}


int reset_coord(int screen_side, int client_side,
		int window_pos, int window_side) {
	if (client_side > screen_side)
		return -(client_side - screen_side) / 2;
	int frame_pos;
	if (client_side > window_side)
		frame_pos = window_pos - (client_side - window_side) / 2;
	else
		frame_pos = window_pos;
	frame_pos = max(frame_pos, 0);
	frame_pos = min(frame_pos, screen_side - client_side);
	return frame_pos;
}

void reset_position() {
	active_context->frame_top = reset_coord(screen_height, client_height,
			window_top, window_height);
	active_context->frame_left = reset_coord(screen_width, client_width,
			window_left, window_width);
}

#define MOVE_STEP 10

void decrease_coord(int *frame_pos) {
	*frame_pos = max(*frame_pos - MOVE_STEP, 0);
}

void increase_coord(int *frame_pos, int screen_side, int client_side) {
	*frame_pos = min(*frame_pos + MOVE_STEP, screen_side - client_side);
}

void move_up_handler() {
	decrease_coord(&active_context->frame_top);
}

void move_down_handler() {
	increase_coord(&active_context->frame_top, screen_height, client_height);
}

void move_left_handler() {
	decrease_coord(&active_context->frame_left);
}

void move_right_handler() {
	increase_coord(&active_context->frame_left, screen_width, client_width);
}

void toogle_window_tracking_handler() {
	window_tracking_enabled = !window_tracking_enabled;
}

void adjust_window_size_handler() {
	window_resize(active_context->window, client_width, client_height);
	activate_window_context(active_context->window);
	reset_position();
}

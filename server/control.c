#include "../common/messages.h"
#include "../common/utils.h"
#include "control.h"
#include "options.h"
#include "screen.h"

#include <math.h>
#include <stdlib.h>

int screen_width, screen_height, client_width, client_height,
		window_left, window_top, window_width, window_height;

struct WindowContext *contexts_list = NULL;

struct WindowContext *active_context = NULL;

void activate_window_context(xcb_window_t window) {
	if (window_get_geometry(window,
			&window_left, &window_top, &window_width, &window_height))
		window_get_root(&window);
	
	if (active_context != NULL && active_context->window == window)
		return;
	struct WindowContext *cur = contexts_list;
	while (cur != NULL) {
		if (cur->window == window) {
			active_context = cur;
			return;
		}
		cur = cur->next;
	}
	
	cur = (struct WindowContext *) malloc(sizeof (struct WindowContext));
	if (cur == NULL)
		return;
	cur->window = window;
	cur->next = contexts_list;
	contexts_list = cur;
	active_context = cur;
	
	reset_scale();
	reset_position();
}

void normalize_coord(int *coord, int frame_side, int screen_side) {
	int margin = frame_side - screen_side;
	if (margin > 0)
		*coord = -margin / 2;
	else {
		*coord = MAX(*coord, 0);
		*coord = MIN(*coord, screen_side - frame_side);
	}
}

void update_frame_dims() {
	active_context->frame_width = round(
			client_width / active_context->scale);
	active_context->frame_height = round(
			client_height / active_context->scale);
	
	normalize_coord(&active_context->frame_left,
			active_context->frame_width, screen_width);
	normalize_coord(&active_context->frame_top,
			active_context->frame_height, screen_height);
}


int reset_coord(int screen_side, int frame_side,
		int window_pos, int window_side) {
	if (frame_side > screen_side)
		return -(frame_side - screen_side) / 2;
	int frame_pos;
	if (frame_side > window_side)
		frame_pos = window_pos - (frame_side - window_side) / 2;
	else
		frame_pos = window_pos;
	frame_pos = MAX(frame_pos, 0);
	frame_pos = MIN(frame_pos, screen_side - frame_side);
	return frame_pos;
}

void reset_position() {
	active_context->frame_top = reset_coord(screen_height,
			active_context->frame_height, window_top, window_height);
	active_context->frame_left = reset_coord(screen_width,
			active_context->frame_width, window_left, window_width);
}

#define MOVE_STEP 10

void decrease_coord(int *frame_pos) {
	*frame_pos = MAX(*frame_pos - MOVE_STEP, 0);
}

void increase_coord(int *frame_pos, int screen_side, int frame_side) {
	*frame_pos = MIN(*frame_pos + MOVE_STEP, screen_side - frame_side);
}

void move_up_handler() {
	decrease_coord(&active_context->frame_top);
}

void move_down_handler() {
	increase_coord(&active_context->frame_top,
			screen_height, active_context->frame_height);
}

void move_left_handler() {
	decrease_coord(&active_context->frame_left);
}

void move_right_handler() {
	increase_coord(&active_context->frame_left,
			screen_width, active_context->frame_width);
}

void toogle_window_tracking_handler() {
	window_tracking_enabled = !window_tracking_enabled;
}

void adjust_window_size_handler() {
	window_resize(active_context->window,
			active_context->frame_width, active_context->frame_height);
	activate_window_context(active_context->window);
	reset_position();
}

void reset_scale() {
	active_context->scale = default_scale;
	update_frame_dims();
}

void center_zoomed_position(double new_scale) {	
	int prev_width = round(client_width / active_context->scale);
	int prev_height = round(client_height / active_context->scale);
	int cur_width = round(client_width / new_scale);
	int cur_height = round(client_height / new_scale);
	active_context->frame_left += (prev_width - cur_width) / 2;
	active_context->frame_top += (prev_height - cur_height) / 2;
}

#define SCALE_COEFF 1.2

void zoom_in_handler() {
	double new_scale = active_context->scale * SCALE_COEFF;
	if (new_scale < MAX_SCALE + EPS) {
		center_zoomed_position(new_scale);
		active_context->scale = new_scale;
	}
}

void zoom_out_handler() {
	int prev_width = round(client_width / active_context->scale);
	int prev_height = round(client_height / active_context->scale);
	if (prev_width >= screen_width && prev_height >= screen_height)
		return;
	double new_scale = active_context->scale / SCALE_COEFF;
	if (new_scale > MIN_SCALE - EPS) {
		center_zoomed_position(new_scale);
		active_context->scale = new_scale;
	}
}

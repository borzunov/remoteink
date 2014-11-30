#include "control.h"

int screen_width, screen_height, client_width, client_height,
		window_left, window_top, window_width, window_height;
int frame_left = 0;
int frame_top = 0;


inline int min(int a, int b) {
	return (a < b) ? a : b;
}

inline int max(int a, int b) {
	return (a > b) ? a : b;
}


inline int reset_coord(int screen_side, int client_side,
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
	frame_top = reset_coord(screen_height, client_height,
			window_top, window_height);
	frame_left = reset_coord(screen_width, client_width,
			window_left, window_width);
}

#define MOVE_STEP 10

inline void decrease_coord(int *frame_pos) {
	*frame_pos = max(*frame_pos - MOVE_STEP, 0);
}

inline void increase_coord(int *frame_pos, int screen_side, int client_side) {
	*frame_pos = min(*frame_pos + MOVE_STEP, screen_side - client_side);
}

void move_up_handler() {
	decrease_coord(&frame_top);
}

void move_down_handler() {
	increase_coord(&frame_top, screen_height, client_height);
}

void move_left_handler() {
	decrease_coord(&frame_left);
}

void move_right_handler() {
	increase_coord(&frame_left, screen_width, client_width);
}

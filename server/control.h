#ifndef CONTROL_H
#define CONTROL_H


extern int screen_width, screen_height, client_width, client_height,
		window_left, window_top, window_width, window_height,
		frame_left, frame_top;

void reset_position();
void move_up_handler();
void move_down_handler();
void move_left_handler();
void move_right_handler();


#endif


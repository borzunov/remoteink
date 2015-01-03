#ifndef CONTROL_H
#define CONTROL_H

#include <xcb/xcb.h>

#include "../common/exceptions.h"


extern int screen_width, screen_height, client_width, client_height;

struct WindowContext {
	xcb_window_t window;
	struct WindowContext *next;
	
	int frame_left;
	int frame_top;
};

extern struct WindowContext *active_context;
extern int window_tracking_enabled;

extern ExcCode activate_window_context(xcb_window_t window);


void reset_position();
void move_up_handler();
void move_down_handler();
void move_left_handler();
void move_right_handler();
void toogle_window_tracking_handler();


#endif


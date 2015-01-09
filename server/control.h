#ifndef CONTROL_H
#define CONTROL_H

#include <xcb/xcb.h>

#include "../common/exceptions.h"


extern int screen_width, screen_height, client_width, client_height;

struct WindowContext {
	xcb_window_t window;
	struct WindowContext *next;
	
	int frame_left, frame_top;
	double scale;
	int frame_width, frame_height;
};

extern pthread_mutex_t active_context_lock;
extern struct WindowContext *active_context;
extern int window_tracking_enabled;

extern void activate_window_context(xcb_window_t window);
extern void update_frame_dims();


extern void reset_position();
extern void move_up_handler();
extern void move_down_handler();
extern void move_left_handler();
extern void move_right_handler();
extern void toogle_window_tracking_handler();
extern void adjust_window_size_handler();
extern void reset_scale();
extern void zoom_in_handler();
extern void zoom_out_handler();


#endif


#ifndef SCREEN_H
#define SCREEN_H

#include "../common/exceptions.h"

#include <Imlib2.h>
#include <xcb/xcb.h>


extern ExcCode screen_init(xcb_connection_t *cur_display,
		xcb_screen_t *cur_screen, xcb_window_t cur_root);
extern void screen_free();

extern ExcCode screen_shot(int x, int y, int width, int height,
		Imlib_Image *res);

extern ExcCode screen_cursor_get_position(int *x, int *y, int *same_screen);
extern ExcCode screen_cursor_set_position(int x, int y);

extern void screen_get_root(xcb_window_t *res);
extern ExcCode screen_get_focused(xcb_window_t *res);
extern ExcCode screen_window_get_geometry(xcb_window_t window,
		int *left, int *top, int *width, int *height);
extern ExcCode screen_window_resize(
		xcb_window_t window, int width, int height);


#endif

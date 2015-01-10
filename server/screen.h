#ifndef SCREEN_H
#define SCREEN_H

#include "../common/exceptions.h"

#include <Imlib2.h>
#include <xcb/xcb.h>


extern xcb_connection_t *display;
extern xcb_screen_t *screen;
extern xcb_window_t root;

extern ExcCode screenshot_init();
extern ExcCode screenshot_get(int x, int y, int width, int height,
		Imlib_Image *res);

extern void window_get_root(xcb_window_t *res);
extern ExcCode window_get_focused(xcb_window_t *res);
extern ExcCode window_get_geometry(xcb_window_t window,
		int *left, int *top, int *width, int *height);
extern ExcCode window_resize(xcb_window_t window, int width, int height);


#endif

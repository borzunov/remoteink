#ifndef SCREEN_H
#define SCREEN_H

#include "../common/exceptions.h"

#include <xcb/xcb.h>


extern ExcCode screenshot_init(int *screen_width, int *screen_height);
extern ExcCode screenshot_get(int x, int y, int width, int height,
		unsigned **res_data);

extern ExcCode window_get_root(xcb_window_t *res);
extern ExcCode window_get_focused(xcb_window_t *res);
extern ExcCode window_get_geometry(xcb_window_t window,
		int *left, int *top, int *width, int *height);


#endif

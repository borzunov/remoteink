#ifndef SCREEN_H
#define SCREEN_H

#include "../common/exceptions.h"

#include <giblib/giblib.h>
#include <X11/Xutil.h>


ExcCode screenshot_init(int *screen_width, int *screen_height);
Imlib_Image screenshot_get(int x, int y, int width, int height);

Window window_get_root();
Window window_get_focused();
void window_get_geometry(Window window,
		int *left, int *top, int *width, int *height);


#endif

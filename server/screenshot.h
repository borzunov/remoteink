#ifndef SCREEN_H
#define SCREEN_H

#include <giblib/giblib.h>
#include <X11/Xutil.h>


void screenshot_init(Screen **screen, Window *root);
Imlib_Image screenshot_get(Window root, int x, int y, int width, int height);


#endif

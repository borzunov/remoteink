#ifndef SCREEN_H
#define SCREEN_H

#include <giblib/giblib.h>
#include <X11/Xutil.h>


void screenshot_init(Screen **screen, Window *root);
Imlib_Image screenshot_get(Window root, int x, int y, int width, int height);


extern int screen_left, screen_top;

void move_up_handler();
void move_down_handler();
void move_left_handler();
void move_right_handler();


#endif

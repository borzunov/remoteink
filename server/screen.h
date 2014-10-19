#ifndef SCREEN_H
#define SCREEN_H

#include <giblib/giblib.h>


extern void screenshot_init();
extern Imlib_Image screenshot_get(int x, int y, int width, int height);

extern Screen *screen;


#endif

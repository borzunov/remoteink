#ifndef OPTIONS_H
#define OPTIONS_H

#include "../common/exceptions.h"
#include "shortcuts.h"


extern char server_host[];
extern int server_port;

extern int max_fps, width_divisor, height_divisor;

extern int window_tracking_enabled;
#define MIN_SCALE 0.4
#define MAX_SCALE 5
extern double default_windows_scale, default_desktop_scale;
extern int cursor_capturing_enabled;

extern int move_step;
extern double scale_factor;

extern char label_font_name[];

extern int stats_enabled;
extern char stats_filename[];

extern struct Shortcut shortcuts[];


ExcCode load_config(const char *filename);


#endif

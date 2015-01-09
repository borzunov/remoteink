#ifndef OPTIONS_H
#define OPTIONS_H

#include "../common/exceptions.h"
#include "shortcuts.h"


extern char server_host[];
extern int server_port;

extern int max_fps, width_divisor, height_divisor;

extern int window_tracking_enabled;
#define EPS 1e-7
#define MIN_SCALE 0.4
#define MAX_SCALE 5
extern double default_scale;

extern int stats_enabled;
extern char stats_filename[];

extern struct Shortcut shortcuts[];


ExcCode load_config(const char *filename);


#endif

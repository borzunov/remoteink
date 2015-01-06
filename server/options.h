#ifndef OPTIONS_H
#define OPTIONS_H

#include "../common/exceptions.h"
#include "shortcuts.h"


extern char server_host[];
extern int server_port;

extern int max_fps, width_divisor, height_divisor;

extern int window_tracking_enabled;

extern int stats_enabled;
extern char stats_filename[];

extern struct Shortcut shortcuts[];


ExcCode load_config(const char *filename);


#endif

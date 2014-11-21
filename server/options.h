#ifndef OPTIONS_H
#define OPTIONS_H

#include "shortcuts.h"


extern char server_host[];
extern int server_port;

extern int stats_enabled;
extern char stats_file[];

extern struct Shortcut shortcuts[];


void load_config(const char *filename);


#endif

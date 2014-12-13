#ifndef OPTIONS_H
#define OPTIONS_H

#include "../common/exceptions.h"
#include "shortcuts.h"


extern char server_host[];
extern int server_port;

extern int stats_enabled;
extern char stats_filename[];

extern struct Shortcut shortcuts[];


ExcCode load_config(const char *filename);


#endif

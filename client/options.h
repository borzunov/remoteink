#ifndef OPTIONS_H
#define OPTIONS_H

#include "../common/exceptions.h"


ExcCode load_config(const char *filename);
ExcCode save_config(const char *filename);


extern char server_host[];
extern int server_port;

extern int orientation;
extern const char* orientation_captions[];


#endif

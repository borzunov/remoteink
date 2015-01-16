#ifndef OPTIONS_H
#define OPTIONS_H

#include "../common/exceptions.h"


extern ExcCode options_config_load(const char *filename);
extern ExcCode options_config_save(const char *filename);


extern char server_host[];
extern int server_port;

#define PASSWORD_SIZE 256
extern char password[];

extern int orientation;
extern const char* orientation_captions[];

extern int agreement_accepted;


#endif

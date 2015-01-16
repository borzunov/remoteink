#ifndef OPTIONS_H
#define OPTIONS_H

#include "../common/exceptions.h"


extern ExcCode load_config(const char *filename);
extern ExcCode save_config(const char *filename);


extern char server_host[];
extern int server_port;

#define PASSWORD_SIZE 256
extern char password[];

extern int orientation;
extern const char* orientation_captions[];

extern int agreement_accepted;


#endif

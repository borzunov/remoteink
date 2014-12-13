#ifndef CLIENT_H
#define CLIENT_H

#include "../common/exceptions.h"


ExcCode client_connect();
void client_shutdown();


extern unsigned screen_width, screen_height;

extern int client_process;


#endif

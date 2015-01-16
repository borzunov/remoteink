#ifndef CLIENT_H
#define CLIENT_H

#include "../common/exceptions.h"


extern ExcCode client_connect(int client_width, int client_height);
extern void client_shutdown();


#endif

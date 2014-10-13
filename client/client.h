#ifndef CLIENT_H
#define CLIENT_H


void *client_connect(void *arg);
void client_shutdown();


extern unsigned screen_width, screen_height;

extern int client_process;


#endif

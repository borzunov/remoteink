#ifndef OPTIONS_H
#define OPTIONS_H


extern void read_options(const char *filename);


#define OPTION_SIZE 1024
extern char server_host[OPTION_SIZE];
extern int server_port;


#endif

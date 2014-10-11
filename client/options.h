#ifndef OPTIONS_H
#define OPTIONS_H


void load_options(const char *filename);
void save_options(const char *filename);


#define OPTION_SIZE 1024
extern char server_host[OPTION_SIZE];
extern int server_port;


#endif

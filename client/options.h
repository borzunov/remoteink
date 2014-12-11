#ifndef OPTIONS_H
#define OPTIONS_H


void load_config(const char *filename);
void save_config(const char *filename);


extern char server_host[];
extern int server_port;


#endif

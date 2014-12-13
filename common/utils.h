#ifndef UTILS_H
#define UTILS_H


void get_default_config_path(const char *name, char *buffer, int buffer_size);


ExcCode parse_port(const char *str, int *res);


#endif

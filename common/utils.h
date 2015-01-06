#ifndef UTILS_H
#define UTILS_H


extern void get_default_config_path(const char *name,
		char *buffer, int buffer_size);


extern ExcCode parse_int(const char *key, const char *value, int min, int max,
		int *res);
extern ExcCode parse_bool(const char *key, const char *value, int *res);


#endif

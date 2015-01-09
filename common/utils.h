#ifndef UTILS_H
#define UTILS_H

#include "../common/exceptions.h"


#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))


extern void get_default_config_path(const char *name,
		char *buffer, int buffer_size);


extern ExcCode parse_int(const char *key, const char *value, int min, int max,
		int *res);
extern ExcCode parse_double(const char *key, const char *value,
		double min, double max,
		double *res);
extern ExcCode parse_bool(const char *key, const char *value, int *res);


#endif

#include "exceptions.h"
#include "ini_parser.h"
#include "messages.h"
#include "utils.h"

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define EXECUTABLE_SYMLINK "/proc/self/exe"

ExcCode get_executable_dirname(char *buffer, int buffer_size) {
	int path_len = readlink(EXECUTABLE_SYMLINK, buffer, buffer_size - 1);
	if (path_len == -1)
		PANIC("Failed to retreive the application directory: "
				"Can't resolve \"" EXECUTABLE_SYMLINK "\"");
	buffer[path_len] = '\0';
	
	const char *res = dirname(buffer);
	int dirname_len = strlen(res);
	if (dirname_len > buffer_size - 1)
		dirname_len = buffer_size - 1;
	memmove(buffer, res, dirname_len);
	buffer[dirname_len] = '\0';
	return 0;
}

void get_default_config_path(const char *name, char *buffer, int buffer_size) {
	if (get_executable_dirname(buffer, buffer_size)) {
		strncpy(buffer, name, buffer_size);
		buffer[buffer_size - 1] = '\0';
		return;
	}
	int directory_len = strlen(buffer);
	snprintf(buffer + directory_len, buffer_size - directory_len, "/%s", name);
}


ExcCode parse_int(const char *key, const char *value, int min, int max,
		int *res) {
	int number;
	if (!(
		sscanf(value, "%d", &number) == 1 &&
		min <= number && number <= max
	))
		PANIC(ERR_INVALID_INT, key, min, max);
	*res = number;
	return 0;
}

ExcCode parse_double(const char *key, const char *value,
		double min, double max,
		double *res) {
	double number;
	if (!(
		sscanf(value, "%lf", &number) == 1 &&
		min <= number && number <= max
	))
		PANIC(ERR_INVALID_DOUBLE, key, min, max);
	*res = number;
	return 0;
}

ExcCode parse_bool(const char *key, const char *value, int *res) {
	if (!strcasecmp(value, INI_VALUE_FALSE))
		*res = 0;
	else
	if (!strcasecmp(value, INI_VALUE_TRUE))
		*res = 1;
	else
		PANIC(ERR_INVALID_BOOL, key);
	return 0;
}

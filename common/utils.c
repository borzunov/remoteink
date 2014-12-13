#include "exceptions.h"
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
		THROW("Failed to retreive the application directory: "
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


ExcCode parse_port(const char *str, int *res) {
	int number;
	if (!(
		sscanf(str, "%d", &number) == 1 &&
		PORT_MIN <= number && number <= PORT_MAX
	))
		THROW(ERR_INVALID_PORT, PORT_MIN, PORT_MAX);
	*res = number;
	return 0;
}

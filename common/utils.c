#include "exceptions.h"
#include "utils.h"

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define BUFFER_SIZE 256
char executable_dirname_buffer[BUFFER_SIZE];

#define EXECUTABLE_SYMLINK "/proc/self/exe"

const char *get_executable_dirname() {
	int len = readlink(EXECUTABLE_SYMLINK, executable_dirname_buffer,
			BUFFER_SIZE - 1);
	if (len == -1) {
		throw_exc("Failed to retreive the application directory: "
				"Can't resolve \"" EXECUTABLE_SYMLINK "\"");
		return NULL;
	}
	executable_dirname_buffer[len] = '\0';
	return dirname(executable_dirname_buffer);
}

#define BUFFER_SIZE 256
char config_path_buffer[BUFFER_SIZE];

const char *get_default_config_path(const char *name) {
	const char *directory = get_executable_dirname();
	if (directory == NULL)
		return name;
	snprintf(config_path_buffer, BUFFER_SIZE, "%s/%s", directory, name);
	return config_path_buffer;
}

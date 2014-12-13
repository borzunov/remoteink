#ifndef TRANSFER_H
#define TRANSFER_H

#include <giblib/giblib.h>


ExcCode image_send_all(
	int conn_fd, const DATA32 *image, unsigned width, unsigned height
);
ExcCode image_send_diff(
	int conn_fd, const DATA32 *prev_image, const DATA32 *next_image,
	unsigned client_width, unsigned client_height,
	unsigned region_left, unsigned region_top,
	unsigned region_width, unsigned region_height
);


#define TRANSFER_BUFFER_SIZE 4 * 1024 * 1024
extern char buffer[TRANSFER_BUFFER_SIZE];


#endif

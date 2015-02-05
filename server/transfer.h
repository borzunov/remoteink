#ifndef TRANSFER_H
#define TRANSFER_H


extern ExcCode transfer_send_error(int conn_fd, const char *message);

extern ExcCode transfer_recv_string(int conn_fd, const char **res);

extern ExcCode transfer_image_send_all(
	int conn_fd, const unsigned *image, unsigned width, unsigned height
);
extern ExcCode transfer_image_send_diff(
	int conn_fd, const unsigned *prev_image, const unsigned *next_image,
	unsigned client_width, unsigned client_height,
	unsigned region_left, unsigned region_top,
	unsigned region_width, unsigned region_height
);


#endif

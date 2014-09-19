#ifndef TRANSFER_H
#define TRANSFER_H

#include <giblib/giblib.h>


void image_send_all(int conn_fd, const DATA32 *image,
        unsigned width, unsigned height);
extern void image_send_diff(int conn_fd,
        const DATA32 *prev_image, const DATA32 *next_image,
        unsigned y_begin, unsigned width, unsigned height);


#define BUFFER_SIZE 4 * 1024 * 1024
extern char buffer[BUFFER_SIZE];

extern int stat_XM_compressed;
extern int stat_CRN_compressed;


#endif

#include "../common/exceptions.h"
#include "../common/messages.h"
#include "../common/protocol.h"
#include "options.h"
#include "profiler.h"
#include "transfer.h"

#include <unistd.h>

char buffer[TRANSFER_BUFFER_SIZE];

ExcCode string_read(int conn_fd, const char **res) {
	if (read(conn_fd, buffer, LENGTH_SIZE) != LENGTH_SIZE)
		THROW(ERR_SOCK_READ);
	int i = -1;
	int len;
	READ_LENGTH(len, buffer, i);
	if (read(conn_fd, buffer, len) != len)
		THROW(ERR_SOCK_READ);
	*res = buffer;
	buffer[len] = 0;
	return 0;
}

ExcCode send_buffer(int conn_fd, const char *buffer, int len) {
	profiler_start(STAGE_TRANSFER);
	
	if (write(conn_fd, buffer, len) < 0)
		THROW(ERR_SOCK_WRITE);
		
	profiler_finish(STAGE_TRANSFER);
	return 0;
}

ExcCode wait_confirm(int conn_fd) {
	profiler_start(STAGE_DRAW);
	
	if (read(conn_fd, buffer, 1) != 1 || buffer[0] != RES_CONFIRM)
		THROW(ERR_SOCK_READ);
	
	profiler_finish(STAGE_DRAW);
	return 0;
}

ExcCode image_send_all(
	int conn_fd, const unsigned *image, unsigned width, unsigned height
) {
	unsigned x, y;
	int i = 0;
	
	buffer[i++] = CMD_RESET_POSITION;
	WRITE_COORD(0, buffer, i);
	WRITE_COORD(0, buffer, i);
	
	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++) {
			unsigned pixel = image[y * width + x];
			buffer[i++] = CMD_PUT_COLOR;
			WRITE_COLOR(pixel, buffer, i);
		}

	buffer[i++] = CMD_SOFT_UPDATE;
	
	TRY(send_buffer(conn_fd, buffer, i));
	TRY(wait_confirm(conn_fd));
	return 0;
}

unsigned color = 0;

ExcCode image_send_diff(
	int conn_fd, const unsigned *prev_image, const unsigned *next_image,
	unsigned client_width, unsigned client_height,
	unsigned region_left, unsigned region_top,
	unsigned region_width, unsigned region_height
) {
	profiler_start(STAGE_DIFF);
	
	unsigned region_right = region_left + region_width;
	unsigned region_bottom = region_top + region_height;
	
	int i = 0;
	
	int skipped_before = 0;
	int is_repeat_before = 0;
	
	buffer[i++] = CMD_RESET_POSITION;
	WRITE_COORD(region_left, buffer, i);
	WRITE_COORD(region_top, buffer, i);
	
	traffic_diffs += 5;
	
	unsigned x1 = region_right - 1;
	unsigned y1 = region_bottom - 1;
	unsigned x2 = region_left;
	unsigned y2 = region_top;
	
	// If the client screen divided horizontally into more than one
	// region, we need to skip pixels from another regions
	unsigned line_skip = client_width - region_width;
	
	unsigned y, x;
	for (y = region_top; y < region_bottom; y++) {
		for (x = region_left; x < region_right; x++) {
			unsigned prev_pixel = prev_image[y * client_width + x];
			unsigned next_pixel = next_image[y * client_width + x];
			if (prev_pixel != next_pixel) {
				if (skipped_before) {
					buffer[i++] = CMD_SKIP;
					WRITE_COUNT(skipped_before, buffer, i);
					skipped_before = 0;
					
					traffic_diffs += 4;
				}
				if (next_pixel != color) {
					buffer[i++] = CMD_PUT_COLOR;
					WRITE_COLOR(next_pixel, buffer, i);
					color = next_pixel;
					is_repeat_before = 0;
					
					traffic_diffs += 4;
				} else
				if (is_repeat_before) {
					// count++ in previos CMD_PUT_REPEAT
					(*(unsigned *) (buffer + i - COUNT_SIZE))++;
					is_repeat_before = 1;
				} else {
					buffer[i++] = CMD_PUT_REPEAT;
					WRITE_COUNT(1, buffer, i);
					is_repeat_before = 1;
					
					traffic_diffs += 4;
				}
				
				if (x < x1)
					x1 = x;
				if (x > x2)
					x2 = x;
				if (y < y1)
					y1 = y;
				if (y > y2)
					y2 = y;
			} else {
				skipped_before++;
				is_repeat_before = 0;
			}
		}
		
		if (line_skip) {
			skipped_before += line_skip;
			is_repeat_before = 0;
		}
	}
		
	int need_send = (x1 <= x2 && y1 <= y2);
	if (need_send) {
		buffer[i++] = CMD_PARTIAL_UPDATE;
		WRITE_COORD(x1, buffer, i);
		WRITE_COORD(y1, buffer, i);
		WRITE_COORD(x2 - x1 + 1, buffer, i);
		WRITE_COORD(y2 - y1 + 1, buffer, i);
		
		traffic_diffs += 9;
	}
	
	profiler_finish(STAGE_DIFF);
		
	if (need_send) {
		TRY(send_buffer(conn_fd, buffer, i));
		TRY(wait_confirm(conn_fd));
	}
	return 0;
}

#include "../common/protocol.h"
#include "main.h"
#include "options.h"
#include "profiler.h"
#include "transfer.h"

char buffer[BUFFER_SIZE];

int stat_XM_compressed;
int stat_CRN_compressed;

void send_buffer(int conn_fd, const char *buffer, int len) {
    PROFILER_BEGIN(STAGE_TRANSFER);
    
    if (write(conn_fd, buffer, len) < 0)
        show_client_error();
        
    PROFILER_END(STAGE_TRANSFER);
}

void wait_confirm(int conn_fd) {
    PROFILER_BEGIN(STAGE_DRAW);
    
    if (read(conn_fd, buffer, 1) != 1 || buffer[0] != RES_CONFIRM)
        show_client_error();
    
    PROFILER_END(STAGE_DRAW);
}

void image_send_all(int conn_fd, const DATA32 *image,
        unsigned width, unsigned height) {
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
    
    send_buffer(conn_fd, buffer, i);
    wait_confirm(conn_fd);
}

unsigned color = 0;

void image_send_diff(int conn_fd,
        const DATA32 *prev_image, const DATA32 *next_image,
        unsigned y_begin, unsigned width, unsigned height) {
    PROFILER_BEGIN(STAGE_DIFF);
    
    unsigned x, y;
    int i = 0;
    
    int skipped_before = 0;
    int is_repeat_before = 0;
    
    buffer[i++] = CMD_RESET_POSITION;
    WRITE_COORD(0, buffer, i);
    WRITE_COORD(y_begin, buffer, i);
    
    unsigned x1 = width - 1;
    unsigned y1 = y_begin + height - 1;
    unsigned x2 = 0;
    unsigned y2 = y_begin;
    
    unsigned y_end = y_begin + height;
    for (y = y_begin; y < y_end; y++)
        for (x = 0; x < width; x++) {
            unsigned prev_pixel = prev_image[y * width + x];
            unsigned next_pixel = next_image[y * width + x];
            if (prev_pixel != next_pixel) {
                if (skipped_before) {
                    buffer[i++] = CMD_SKIP;
                    WRITE_COUNT(skipped_before, buffer, i);
                    skipped_before = 0;
                    
                    stat_XM_compressed += 5;
                    stat_CRN_compressed += 4;
                }
                if (next_pixel != color) {
                    buffer[i++] = CMD_PUT_COLOR;
                    WRITE_COLOR(next_pixel, buffer, i);
                    color = next_pixel;
                    is_repeat_before = 0;
                    
                    stat_CRN_compressed += 4;
                } else
                if (is_repeat_before) {
                    // count++ in previos CMD_PUT_REPEAT
                    (*(unsigned *) (buffer + i - COUNT_SIZE))++;
                    is_repeat_before = 1;
                } else {
                    buffer[i++] = CMD_PUT_REPEAT;
                    WRITE_COUNT(1, buffer, i);
                    is_repeat_before = 1;
                    
                    stat_CRN_compressed += 4;
                }
                
                stat_XM_compressed += 4;
                
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
        
    int need_send = (x1 <= x2 && y1 <= y2);
    if (need_send) {
        buffer[i++] = CMD_PARTIAL_UPDATE;
        WRITE_COORD(x1, buffer, i);
        WRITE_COORD(y1, buffer, i);
        WRITE_COORD(x2 - x1 + 1, buffer, i);
        WRITE_COORD(y2 - y1 + 1, buffer, i);
        
        stat_XM_compressed += 9;
        stat_CRN_compressed += 14; // 5 for CMD_RESET_POSITION at the beginning
                                   // and 9 for CMD_PARTIAL_UPDATE now
    }
    
    PROFILER_END(STAGE_DIFF);
        
    if (need_send) {
        send_buffer(conn_fd, buffer, i);
        wait_confirm(conn_fd);
    }
}

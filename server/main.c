#include "../common/protocol.h"
#include "main.h"
#include "options.h"
#include "profiler.h"
#include "screen.h"
#include "transfer.h"

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_FPS 20

// The top and bottom of the screen will be treated separately. This is useful,
// for example, in a word processor, when there are small updates in the middle
// of the screen (text) and in the bottom of the screen (the counters of lines
// or characters). Then it will be updated not one big rectangle of the screen,
// and two small ones. By default the screen will be divided into three areas
// with height of 1 / 5, 3 / 5 and 1 / 5.
#define HEIGHT_DIV_NOM 1
#define HEIGHT_DIV_DENOM 5

void show_error(const char *error) {
    fprintf(stderr, "[-] Error: %s\n", error);
    exit(EXIT_FAILURE);
}

void show_conn_error(const char *message) {
    show_error(message == NULL ? "Failed to start server" : message);
}

void show_client_error() {
#ifdef ENABLE_PROFILER
    profiler_save("profiler.log"); //
#endif
    
    show_error("Connection closed");
}

#define MIN_FRAME_DURATION ((NSECS_PER_SEC) / (MAX_FPS))

int main(int argc, char *argv[]) {
    printf("InkMonitor v0.01 Alpha 4 - Server\n");
    
    parse_options(argc, argv);
    
    screenshot_init();
    printf("    Monitor resolution: %dx%d\n", screen->width, screen->height);
        
    int serv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_fd < 0)
        show_conn_error(NULL);
    
    struct hostent *serv = gethostbyname(server_host);
    if (serv == NULL)
        show_conn_error("No such host");
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, serv->h_addr, serv->h_length);
    serv_addr.sin_port = htons(server_port);
    
    if (bind(serv_fd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0)
        show_conn_error("Failed to bind this port on this host");

    listen(serv_fd, 1);
    
    struct sockaddr_in *client_addr;
    socklen_t client_len = sizeof (client_addr);
    printf("[+] Listen on %s:%d\n", server_host, server_port);
    
    int conn_fd = accept(
            serv_fd, (struct sockaddr *) &client_addr, &client_len);
    if (conn_fd < 0)
        show_client_error();

    int header_len = strlen(HEADER);
    if (read(conn_fd, buffer, header_len + 1) != header_len + 1 ||
            buffer[header_len] != '\n' || strncmp(buffer, HEADER, header_len))
        show_error("Incompatible client version");
    printf("[+] Accepted client connection\n");
    
    if (read(conn_fd, buffer, COORD_SIZE * 2) != COORD_SIZE * 2)
        show_client_error();
    unsigned client_width, client_height;
    int i = -1;
    READ_COORD(client_width, buffer, i);
    READ_COORD(client_height, buffer, i);
    unsigned height_div1 = client_height * HEIGHT_DIV_NOM / HEIGHT_DIV_DENOM;
    unsigned height_div2 = client_height *
            (HEIGHT_DIV_DENOM - HEIGHT_DIV_NOM) / HEIGHT_DIV_DENOM;
    printf("    Reader resolution: %ux%u\n",
            client_width, client_height);
    
    if (start_x <= -screen->width || start_x >= screen->width ||
            start_y <= -screen->height || start_y >= screen->height)
        show_error("Incorrect position of left top corner of grabbed part "
                   "of the screen. Failed to fill reader screen.");
    
    Imlib_Image image = screenshot_get(start_x, start_y,
            client_width, client_height);
    imlib_context_set_image(image);
    DATA32 *data = imlib_image_get_data_for_reading_only();
    
    image_send_all(conn_fd, data, client_width, client_height);
    
    int frames = 0;
    while (1) {
        long long frame_start_time = get_time_nsec();
    
        stat_XM_compressed = 0;
        stat_CRN_compressed = 0;
        
        PROFILER_BEGIN(STAGE_SHOT);
        
        Imlib_Image next_image = screenshot_get(start_x, start_y,
                client_width, client_height);
        imlib_context_set_image(next_image);
        DATA32 *next_data = imlib_image_get_data_for_reading_only();
        
        PROFILER_END(STAGE_SHOT);
        
        image_send_diff(conn_fd, data, next_data,
                0, client_width, height_div1);
        image_send_diff(conn_fd, data, next_data,
                height_div1, client_width, height_div2 - height_div1);
        image_send_diff(conn_fd, data, next_data,
                height_div2, client_width, client_height - height_div2);
                
        gib_imlib_free_image_and_decache(image);
        image = next_image;
        data = next_data;
    
        frames++;
        if (show_stat) {
            printf("    Compression: XM / original: %.2lf%%\n",
                    (stat_XM_compressed /
                    (double) (client_width * client_height * 4)) * 100.);
            if (stat_XM_compressed)
                printf("    Compression: CRN / XM: %.2lf%%\n",
                        (stat_CRN_compressed /
                        (double) stat_XM_compressed) * 100.);
            printf("[*] Total traffic: %.2lf KB\n",
                    stat_CRN_compressed / 1024.);
        }
        
        long long frame_duration = get_time_nsec() - frame_start_time;
        long long sleep_time = MIN_FRAME_DURATION - frame_duration;
        if (sleep_time > 0)
            usleep(sleep_time / NSECS_PER_MSEC);
#ifdef ENABLE_PROFILER
        else
        if (sleep_time < 0)
            printf("Too slow! Frame duration is %lld ms.\n",
                    frame_duration / NSECS_PER_MSEC);
#endif
    }

    close(conn_fd);
    close(serv_fd);
    return 0; 
}

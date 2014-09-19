#include "../common/protocol.h"
#include "client.h"
#include "main.h"
#include "options.h"

#include <inkview.h>
#include <netdb.h>

unsigned screen_width, screen_height;

#define BUFFER_SIZE 4 * 1024 * 1024
char buffer[BUFFER_SIZE];

unsigned x, y, color;

int conn_fd;

inline void client_send_confirm() {
    buffer[0] = RES_CONFIRM;
    if (write(conn_fd, buffer, 1) < 0)
        show_client_error();
}

int client_exec(const char *commands, int len) {
    int i;
    unsigned j, count, w, h;
    for (i = 0; i < len; i++) {
        switch (commands[i]) {
        case CMD_RESET_POSITION:
            if (i + COORD_SIZE * 2 + 1 > len)
                return i;
            READ_COORD(x, commands, i);
            READ_COORD(y, commands, i);
            break;
        case CMD_SKIP:
            if (i + COUNT_SIZE + 1 > len)
                return i;
            READ_COUNT(count, commands, i);
            
            count += x;
            y += count / screen_width;
            x = count % screen_width;
            break;
        case CMD_PUT_REPEAT:
            if (i + COUNT_SIZE + 1 > len)
                return i;
            READ_COUNT(count, commands, i);
            
            for (j = 0; j < count; j++) {
                DrawPixel(x, y, color);
                if (++x == screen_width) {
                    x = 0;
                    if (++y == screen_height)
                        y = 0;
                }
            }
            break;
        case CMD_PUT_COLOR:
            if (i + COUNT_SIZE + 1 > len)
                return i;
            READ_COLOR(color, commands, i);
            
            DrawPixel(x, y, color);
            if (++x == screen_width) {
                x = 0;
                if (++y == screen_height)
                    y = 0;
            }
            break;
            
        #ifdef CMD_EXTENTED
        case CMD_SET_COLOR:
            if (i + COLOR_SIZE + 1 > len)
                return i;
            READ_COLOR(color, commands, i);
            break;
        case CMD_DRAW_PIXEL:
            if (i + COORD_SIZE * 2 + 1 > len)
                return i;
            READ_COORD(x, commands, i);
            READ_COORD(y, commands, i);
            
            DrawPixel(x, y, color);
            break;
        case CMD_DRAW_LINE:
            if (i + COORD_SIZE * 4 + 1 > len)
                return i;
            READ_COORD(x1, commands, i);
            READ_COORD(y1, commands, i);
            READ_COORD(x2, commands, i);
            READ_COORD(y2, commands, i);
            
            DrawLine(x1, y1, x2, y2, color);
            break;
        case CMD_FILL_AREA:
            if (i + COORD_SIZE * 4 + 1 > len)
                return i;
            READ_COORD(x, commands, i);
            READ_COORD(y, commands, i);
            READ_COORD(w, commands, i);
            READ_COORD(h, commands, i);
            
            FillArea(x, y, w, h, color);
            break;
            
        case CMD_FULL_UPDATE:
            FullUpdate();
            client_send_confirm();
            break;
        #endif
        case CMD_SOFT_UPDATE:
            SoftUpdate();
            client_send_confirm();
            break;
        case CMD_PARTIAL_UPDATE:
            if (i + COORD_SIZE * 4 + 1 > len)
                return i;
            READ_COORD(x, commands, i);
            READ_COORD(y, commands, i);
            READ_COORD(w, commands, i);
            READ_COORD(h, commands, i);
            
            PartialUpdate(x, y, w, h);
            client_send_confirm();
            break;
        #ifdef CMD_EXTENTED
        case CMD_PARTIAL_UPDATE_BW:
            if (i + COORD_SIZE * 4 + 1 > len)
                return i;
            READ_COORD(x, commands, i);
            READ_COORD(y, commands, i);
            READ_COORD(w, commands, i);
            READ_COORD(h, commands, i);
            
            PartialUpdateBW(x, y, w, h);
            client_send_confirm();
            break;
        case CMD_DYNAMIC_UPDATE:
            if (i + COORD_SIZE * 4 + 1 > len)
                return i;
            READ_COORD(x, commands, i);
            READ_COORD(y, commands, i);
            READ_COORD(w, commands, i);
            READ_COORD(h, commands, i);
            
            DynamicUpdate(x, y, w, h);
            client_send_confirm();
            break;
        case CMD_DYNAMIC_UPDATE_BW:
            if (i + COORD_SIZE * 4 + 1 > len)
                return i;
            READ_COORD(x, commands, i);
            READ_COORD(y, commands, i);
            READ_COORD(w, commands, i);
            READ_COORD(h, commands, i);
            
            DynamicUpdateBW(x, y, w, h);
            client_send_confirm();
            break;
        #endif
        }
    }
    return len;
}

void client_mainloop() {
    ClearScreen();
    x = 0;
    y = 0;
    color = 0;
    
    int prefix_size = 0;
    while (1) {
        int read_size = read(conn_fd,
                buffer + prefix_size, BUFFER_SIZE - prefix_size);
        if (read_size <= 0)
            show_client_error();
        
        int executed_size = client_exec(buffer, prefix_size + read_size);
        prefix_size += read_size - executed_size;
        memmove(buffer, buffer + executed_size, prefix_size);
    }
}

void *client_connect(void *arg) {
    clear_labels();
    add_label("Connecting...");
    SoftUpdate();
    
    query_network();
    
    conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_fd < 0)
        show_conn_error(NULL);
    
    struct hostent *serv = gethostbyname(server_host);
    if (serv == NULL)
        show_conn_error("No such host");
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, serv->h_addr, serv->h_length);
    serv_addr.sin_port = htons(server_port);
    
    if (connect(conn_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        show_conn_error(NULL);
        
    add_label("Starting...");
    SoftUpdate();
    
    if (write(conn_fd, HEADER "\n", strlen(HEADER) + 1) < 0)
        show_client_error();

    int i = 0;
    WRITE_COORD(screen_width, buffer, i);
    WRITE_COORD(screen_height, buffer, i);
    if (write(conn_fd, buffer, COORD_SIZE * 2) < 0)
        show_client_error();
        
    client_mainloop();
    
    return NULL;
}

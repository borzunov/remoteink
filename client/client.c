#include "../common/messages.h"
#include "../common/protocol.h"
#include "client.h"
#include "options.h"
#include "ui.h"

#include <inkview.h>
#include <netdb.h>

#define ERR_UNSUPPORTED_COMMAND "Unsupported command is received"

#define TRANSFER_BUFFER_SIZE 4 * 1024 * 1024
char buffer[TRANSFER_BUFFER_SIZE];

unsigned x, y, color;

int conn_fd;

int client_process;

ExcCode client_send_confirm() {
	buffer[0] = RES_CONFIRM;
	if (write(conn_fd, buffer, 1) < 0 && client_process)
		PANIC(ERR_SOCK_TRANSFER);
	return 0;
}

ExcCode client_exec(const char *commands, int len, int *processed,
		int client_width, int client_height) {
	int i;
	unsigned j, count, w, h;
	for (i = 0; i < len; i++) {
		switch (commands[i]) {
			case CMD_SHOW_ERROR:
				for (j = i + 1; j < len && commands[j]; j++) ;
				if (j == len) {
					*processed = i;
					return 0;
				}
				Message(ICON_ERROR, "Server Error",
						commands + i + 1, MESSAGE_MSECS);
				client_process = 0;
				*processed = j;
				return 0;
			case CMD_RESET_POSITION:
				if (i + COORD_SIZE * 2 + 1 > len) {
					*processed = i;
					return 0;
				}
				READ_COORD(x, commands, i);
				READ_COORD(y, commands, i);
				break;
			case CMD_SKIP:
				if (i + COUNT_SIZE + 1 > len) {
					*processed = i;
					return 0;
				}
				READ_COUNT(count, commands, i);
				
				count += x;
				y += count / client_width;
				x = count % client_width;
				break;
			case CMD_PUT_REPEAT:
				if (i + COUNT_SIZE + 1 > len) {
					*processed = i;
					return 0;
				}
				READ_COUNT(count, commands, i);
				
				for (j = 0; j < count; j++) {
					DrawPixel(x, y, color);
					if (++x == client_width) {
						x = 0;
						if (++y == client_height)
							y = 0;
					}
				}
				break;
			case CMD_PUT_COLOR:
				if (i + COUNT_SIZE + 1 > len) {
					*processed = i;
					return 0;
				}
				READ_COLOR(color, commands, i);
				
				DrawPixel(x, y, color);
				if (++x == client_width) {
					x = 0;
					if (++y == client_height)
						y = 0;
				}
				break;
				
			#ifdef CMD_EXTENTED
			case CMD_SET_COLOR:
				if (i + COLOR_SIZE + 1 > len) {
					*processed = i;
					return 0;
				}
				READ_COLOR(color, commands, i);
				break;
			case CMD_DRAW_PIXEL:
				if (i + COORD_SIZE * 2 + 1 > len) {
					*processed = i;
					return 0;
				}
				READ_COORD(x, commands, i);
				READ_COORD(y, commands, i);
				
				DrawPixel(x, y, color);
				break;
			case CMD_DRAW_LINE:
				if (i + COORD_SIZE * 4 + 1 > len) {
					*processed = i;
					return 0;
				}
				READ_COORD(x1, commands, i);
				READ_COORD(y1, commands, i);
				READ_COORD(x2, commands, i);
				READ_COORD(y2, commands, i);
				
				DrawLine(x1, y1, x2, y2, color);
				break;
			case CMD_FILL_AREA:
				if (i + COORD_SIZE * 4 + 1 > len) {
					*processed = i;
					return 0;
				}
				READ_COORD(x, commands, i);
				READ_COORD(y, commands, i);
				READ_COORD(w, commands, i);
				READ_COORD(h, commands, i);
				
				FillArea(x, y, w, h, color);
				break;
				
			case CMD_FULL_UPDATE:
				FullUpdate();
				TRY(client_send_confirm());
				break;
			#endif
			case CMD_SOFT_UPDATE:
				SoftUpdate();
				TRY(client_send_confirm());
				break;
			case CMD_PARTIAL_UPDATE:
				if (i + COORD_SIZE * 4 + 1 > len) {
					*processed = i;
					return 0;
				}
				READ_COORD(x, commands, i);
				READ_COORD(y, commands, i);
				READ_COORD(w, commands, i);
				READ_COORD(h, commands, i);
				
				PartialUpdate(x, y, w, h);
				TRY(client_send_confirm());
				break;
			#ifdef CMD_EXTENTED
			case CMD_PARTIAL_UPDATE_BW:
				if (i + COORD_SIZE * 4 + 1 > len) {
					*processed = i;
					return 0;
				}
				READ_COORD(x, commands, i);
				READ_COORD(y, commands, i);
				READ_COORD(w, commands, i);
				READ_COORD(h, commands, i);
				
				PartialUpdateBW(x, y, w, h);
				TRY(client_send_confirm());
				break;
			case CMD_DYNAMIC_UPDATE:
				if (i + COORD_SIZE * 4 + 1 > len) {
					*processed = i;
					return 0;
				}
				READ_COORD(x, commands, i);
				READ_COORD(y, commands, i);
				READ_COORD(w, commands, i);
				READ_COORD(h, commands, i);
				
				DynamicUpdate(x, y, w, h);
				TRY(client_send_confirm());
				break;
			case CMD_DYNAMIC_UPDATE_BW:
				if (i + COORD_SIZE * 4 + 1 > len) {
					*processed = i;
					return 0;
				}
				READ_COORD(x, commands, i);
				READ_COORD(y, commands, i);
				READ_COORD(w, commands, i);
				READ_COORD(h, commands, i);
				
				DynamicUpdateBW(x, y, w, h);
				TRY(client_send_confirm());
				break;
			#endif
			
			case CONN_CHECK:
				break;
				
			default:
				printf("[%d;%s]\n", buffer[i], buffer + i); //
				PANIC(ERR_UNSUPPORTED_COMMAND);
		}
	}
	*processed = len;
	return 0;
}

void client_shutdown() {
	client_process = 0;
	shutdown(conn_fd, SHUT_RDWR);
}

ExcCode client_mainloop(int client_width, int client_height) {
	x = 0;
	y = 0;
	color = 0;
	
	int prefix_size = 0;
	client_process = 1;
	while (client_process) {
		int read_size = read(conn_fd,
				buffer + prefix_size, TRANSFER_BUFFER_SIZE - prefix_size);
		if (!client_process || !read_size)
			break;
		if (read_size < 0)
			PANIC(ERR_SOCK_TRANSFER);
		
		int executed_size;
		TRY(client_exec(buffer, prefix_size + read_size, &executed_size,
				client_width, client_height));
		prefix_size += read_size - executed_size;
		memmove(buffer, buffer + executed_size, prefix_size);
	}
	return 0;
}

void query_network() {
	if (!(QueryNetwork() & NET_CONNECTED)) {
		NetConnect(NULL);
		
		// Network selection dialog can ruin the image on the screen. Let's
		// wait until it disappears.
		sleep(1);
	}
}

ExcCode client_string_send(const char *str) {
	int i = 0;
	int len = strlen(str);
	WRITE_LENGTH(len, buffer, i);
	memcpy(buffer + i, str, len);
	i += len;
	if (write(conn_fd, buffer, i) < 0)
		PANIC(ERR_SOCK_TRANSFER);
	return 0;
}

ExcCode client_handshake(int client_width, int client_height) {
	TRY(client_string_send(HEADER));
	
	TRY(client_string_send(password));
	
	int i = 0;
	WRITE_COORD(client_width, buffer, i);
	WRITE_COORD(client_height, buffer, i);
	if (write(conn_fd, buffer, i) < 0)
		PANIC(ERR_SOCK_TRANSFER);
	return 0;
}

ExcCode client_connect(int client_width, int client_height) {
	query_network();
	ClearScreen();
	ShowHourglass();
	
	conn_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (conn_fd < 0)
		PANIC(ERR_SOCK_CREATE);
	
	struct hostent *serv = gethostbyname(server_host);
	if (serv == NULL)
		PANIC(ERR_SOCK_RESOLVE, server_host);
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr.s_addr, serv->h_addr, serv->h_length);
	serv_addr.sin_port = htons(server_port);
	
	if (connect(
		conn_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)
	) < 0)
		PANIC(ERR_SOCK_CONNECT, server_host, server_port);
	
	TRY(client_handshake(client_width, client_height));
	HideHourglass();
	
	TRY(client_mainloop(client_width, client_height));
	
	close(conn_fd);
	return 0;
}

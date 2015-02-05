#ifndef PROTOCOL_H
#define PROTOCOL_H


/* Protocol description
 * --------------------
 * 
 * General:
 *   - All data is sent as little-endian numbers
 * 
 * Protocol:
 *   1. Client sends length of HEADER string (2-byte unsigned integer),
 *      then its content.
 *   2. Client sends length of password (2-byte unsigned integer),
 *      then its content.
 *   3. Client sends width and height (two 2-byte unsigned integers).
 *   4. Server sends sequence of commands described below. First byte of each
 *      command is a character describes its type. The following bytes are
 *      command arguments if they exist:
 *        - Error message is a null-byte terminated string
 *          (if CMD_SHOW_ERROR sent, it should be a last command)
 *        - Coordinates are 2-byte unsigned integers
 *        - Color is 3-byte integer interpreted as RGB
 *          (in fact it will be sent as BGR)
 *        - Numbers describes count of command repetitions are
 *          3-byte unsigned integers
 *        - Values of arguments "color", "x" and "y" are saved between executed
 *          commands. At the beginning they set to zeros.
 *      Note: You can define macros CMD_EXTENTED to enable extented command
 *            set, which is implemented in the client but not used in the
 *            current version of the server.
 *   5. After execution of screen update commands client sends one byte that
 *      contains character RES_CONFIRM.
 *   6. Also server can send one byte contains character CONN_CHECK instead of
 *      the command to check the connection. This character should be ignored
 *      by the client.
 */

#define HEADER "InkMonitor-0.02"

#define CMD_SHOW_ERROR        'E' // SHOW_ERROR(message)

#define CMD_RESET_POSITION    'X' // RESET_POSITION(x, y)
#define CMD_SKIP              'N' // SKIP(count)
#define CMD_PUT_REPEAT        'R' // PUT_REPEAT(count)
#define CMD_PUT_COLOR         'T' // PUT_COLOR(color)

#ifdef CMD_EXTENTED
#define CMD_SET_COLOR         'C' // SET_COLOR(color)
#define CMD_DRAW_PIXEL        'O' // DRAW_PIXEL(x, y)
#define CMD_DRAW_LINE         'L' // DRAW_LINE(x1, y1, x2, y2)
#define CMD_FILL_AREA         'A' // FILL_AREA(x, y, w, h)

#define CMD_FULL_UPDATE       'F' // FULL_UPDATE()
#endif
#define CMD_SOFT_UPDATE       'S' // SOFT_UPDATE()
#define CMD_PARTIAL_UPDATE    'P' // PARTIAL_UPDATE(x, y, w, h)
#ifdef CMD_EXTENTED
#define CMD_PARTIAL_UPDATE_BW 'p' // PARTIAL_UPDATE_BW(x, y, w, h)
#define CMD_DYNAMIC_UPDATE    'D' // DYNAMIC_UPDATE(x, y, w, h)
#define CMD_DYNAMIC_UPDATE_BW 'd' // DYNAMIC_UPDATE_BW(x, y, w, h)
#endif

#define RES_CONFIRM '+'

#define CONN_CHECK '?'


#define LENGTH_SIZE 2
#define READ_LENGTH READ_2B
#define WRITE_LENGTH WRITE_2B

#define COORD_SIZE 2
#define READ_COORD READ_2B
#define WRITE_COORD WRITE_2B

#define COLOR_SIZE 3
#define READ_COLOR READ_3B
#define WRITE_COLOR WRITE_3B

#define COUNT_SIZE 3
#define READ_COUNT READ_3B
#define WRITE_COUNT WRITE_3B


#define READ_2B(dest, buffer, i) {\
	(dest) = *(short unsigned *) ((buffer) + (i) + 1);\
	(i) += 2;\
}
#define READ_3B(dest, buffer, i) {\
	(dest) = (*(unsigned *) ((buffer) + (i))) >> 8;\
	(i) += 3;\
}
#define READ_4B(dest, buffer, i) {\
	(dest) = *(unsigned *) ((buffer) + (i) + 1);\
	(i) += 4;\
}

#define WRITE_2B(src, buffer, i) {\
	*(short unsigned *) ((buffer) + (i)) = (src);\
	(i) += 2;\
}
#define WRITE_3B(src, buffer, i) {\
	*(unsigned *) ((buffer) + (i)) = (src);\
	(i) += 3;\
}
#define WRITE_4B(src, buffer, i) {\
	*(unsigned *) ((buffer) + (i)) = (src);\
	(i) += 4;\
}


#endif

#ifndef PROTOCOL_H
#define PROTOCOL_H


/* [ Protocol description ]
 * Types:
 * - All data sent as little-endian numbers.
 * - Numbers describes coordinates, width or height are 2-byte (short unsigned).
 * - Numbers describes RGB color are 3-byte (in fact they sent as BGR).
 * - Numbers describes count of repeats or absences are 3-byte too.
 * 
 * Protocol:
 * 1). Client sent HEADER string describes it's version and char '\n'.
 * 2). Client sent two shorts describes reader's screen width and height.
 * 3). Server sent commands described below. First sent char with type of
 *     command, then sent command arguments, if they exist. Also you can
 *     define macros CMD_EXTENTED to enable extented command set, which is
 *     implemented in the client, but not used in the current version
 *     of the server.
 * 4). "color", "x" and "y" arguments saved between different commands.
 *     At the beginning of command execution these values set to zeros.
 * 5). For update commands client sent char RES_CONFIRM after its execution.
 */

#define HEADER "InkMonitor v0.01"

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


#define COORD_SIZE 2
#define READ_COORD READ_2B
#define WRITE_COORD WRITE_2B

#define COLOR_SIZE 3
#define READ_COLOR READ_3B
#define WRITE_COLOR WRITE_3B

#define COUNT_SIZE 3
#define READ_COUNT READ_3B
#define WRITE_COUNT WRITE_3B


#define READ_2B(dest, buffer, i) (dest) = *(short unsigned *)\
                                         ((buffer) + (i) + 1);\
                                 (i) += 2;
#define READ_3B(dest, buffer, i) (dest) = (*(unsigned *)\
                                         ((buffer) + (i))) >> 8;\
                                 (i) += 3;
#define READ_4B(dest, buffer, i) (dest) = *(unsigned *) ((buffer) + (i) + 1);\
                                 (i) += 4;

#define WRITE_2B(src, buffer, i) *(short unsigned *) ((buffer) + (i)) = (src);\
                                    i += 2;
#define WRITE_3B(src, buffer, i) *(unsigned *) ((buffer) + (i)) = (src);\
                                    i += 3;
#define WRITE_4B(src, buffer, i) *(unsigned *) ((buffer) + (i)) = (src);\
                                    i += 4;


#endif

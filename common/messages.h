#ifndef MESSAGES_H
#define MESSAGES_H

#include "ini_parser.h"


#define ERR_FILE_OPEN_FOR_READING "Can't open file \"%s\" for reading"
#define ERR_FILE_OPEN_FOR_WRITING "Can't open file \"%s\" for writing"
#define ERR_FILE_READ "Can't read from file \"%s\""
#define ERR_FILE_WRITE "Can't write to file \"%s\""

#define ERR_SOCK_CREATE "Failed to create socket"
#define ERR_SOCK_RESOLVE "No such host"
#define ERR_SOCK_BIND "Failed to bind %s:%d"
#define ERR_SOCK_ACCEPT "Failed to accept a connection"
#define ERR_SOCK_RECV "Failed to receive data from socket"
#define ERR_SOCK_SEND "Failed to send data to socket"

#define PORT_MIN 1
#define PORT_MAX 49151
#define ERR_INVALID_PORT "Port number should be in the interval from %d to %d"
#define ERR_INVALID_BOOL "Boolean parameter \"%s\" should be set to \
\"True\" or \"False\""

#define ERR_DISPLAY "Can't open X display"


#endif
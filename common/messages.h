#ifndef MESSAGES_H
#define MESSAGES_H


#define ERR_THREAD_CREATE "Failed to create thread"

#define ERR_FILE_OPEN_FOR_READING "Can't open file \"%s\" for reading"
#define ERR_FILE_OPEN_FOR_WRITING "Can't open file \"%s\" for writing"
#define ERR_FILE_READ "Can't read from file \"%s\""
#define ERR_FILE_WRITE "Can't write to file \"%s\""

#define ERR_SOCK_CREATE "Failed to create socket"
#define ERR_SOCK_RESOLVE "No such host: %s"
#define ERR_SOCK_BIND "Failed to bind %s:%d"
#define ERR_SOCK_ACCEPT "Failed to accept a connection"
#define ERR_SOCK_CONNECT "Failed to connect to %s:%d"
#define ERR_SOCK_READ "Failed to receive data from socket"
#define ERR_SOCK_WRITE "Failed to send data to socket"
#define ERR_SOCK_CLOSE "Failed to close socket"

#define PORT_MIN 1
#define PORT_MAX 49151
#define ERR_INVALID_PORT "Port number should be in range from %d to %d"
#define ERR_INVALID_BOOL "Boolean parameter \"%s\" should be set to "\
		"\"True\" or \"False\""

#define ERR_X_CONNECT "Failed to connect to X server"
#define ERR_X_REQUEST "Error on request to X server"


#endif

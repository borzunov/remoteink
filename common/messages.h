#ifndef MESSAGES_H
#define MESSAGES_H


#define ERR_MALLOC "Failed to allocate memory"
#define ERR_THREAD_CREATE "Failed to create thread"
#define ERR_MUTEX_INIT "Failed to initialize mutex"

#define ERR_FILE_CREATE "Can't create file \"%s\""
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

#define ERR_PROTOCOL "Protocol mismatch"

#define PORT_MIN 1
#define PORT_MAX 49151
#define ERR_INVALID_INT "Parameter \"%s\" should be an integer number "\
		"in range from %d to %d"
#define ERR_INVALID_DOUBLE "Parameter \"%s\" should be a real number "\
		"in range from %.1lf to %.1lf"
#define ERR_INVALID_BOOL "Boolean parameter \"%s\" should be set to "\
		"\"True\" or \"False\""

#define ERR_IMAGE "Error on operations with frame"

#define ERR_FONT "Failed to load font \"%s\""

#define ERR_X_CONNECT "Failed to connect to X server"
#define ERR_X_REQUEST "Error on request to X server"

#define ERR_WRONG_PASSWORD "Wrong password"


#endif

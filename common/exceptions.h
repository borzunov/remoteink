#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdio.h>


#define EXC_MESSAGE_SIZE 256
extern char exc_message[EXC_MESSAGE_SIZE];

typedef int ExcCode;

#define TRY(expr) {\
	if (expr) {\
		FINALLY;\
		return -1;\
	}\
}

#define THROW(...) {\
	snprintf(exc_message, EXC_MESSAGE_SIZE, __VA_ARGS__);\
	FINALLY;\
	return -1;\
}

#define FINALLY


#endif

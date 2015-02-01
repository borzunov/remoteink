#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdio.h>


#define EXC_MESSAGE_SIZE 256
extern char exc_message[EXC_MESSAGE_SIZE];

typedef int ExcCode;

#define TRY(expr) {\
	if (expr)\
		return -1;\
}
#define TRY_WITH_DEFER(expr) {\
	if (expr) {\
		pop_defer(NULL);\
		return -1;\
	}\
}

#define PANIC(...) {\
	snprintf(exc_message, EXC_MESSAGE_SIZE, __VA_ARGS__);\
	return -1;\
}
#define PANIC_WITH_DEFER(...) {\
	snprintf(exc_message, EXC_MESSAGE_SIZE, __VA_ARGS__);\
	pop_defer(NULL);\
	return -1;\
}

typedef void (*DeferFunc)();

extern void push_defer(DeferFunc func);
extern void pop_defer(DeferFunc func);
extern void pop_all_defers();


#endif

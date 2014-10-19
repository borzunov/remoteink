#include "exceptions.h"

#include <stdio.h>
#include <stdarg.h>

void (*except_handler)(const char *message);

void set_except(void (*handler)(const char *message)) {
    except_handler = handler;
}

#define MAX_FINALLY_HANDLERS 16
void (*finally_handlers[MAX_FINALLY_HANDLERS])();
int finally_handlers_top = 0;

void push_finally(void (*handler)()) {
    finally_handlers[finally_handlers_top] = handler;
    finally_handlers_top++;
}

void pop_finally() {
    finally_handlers_top--;
    finally_handlers[finally_handlers_top]();
}

#define MESSAGE_BUFFER_SIZE 256
char message_buffer[MESSAGE_BUFFER_SIZE];

void throw_exc(const char* format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vsnprintf(message_buffer, MESSAGE_BUFFER_SIZE, format, argptr);
    va_end(argptr);
    
    while (finally_handlers_top)
        pop_finally();
    except_handler(message_buffer);
}

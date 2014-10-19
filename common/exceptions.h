#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H


void set_except(void (*handler)(const char *message));

void push_finally(void (*handler)());
void pop_finally();

void throw_exc(const char* format, ...);


#endif

#include "exceptions.h"

#include <assert.h>

char exc_message[EXC_MESSAGE_SIZE];

#define DEFERS_STACK_SIZE 16
DeferFunc defers_stack[DEFERS_STACK_SIZE];
int defers_stack_top = 0;

void push_defer(DeferFunc func) {
	assert(defers_stack_top < DEFERS_STACK_SIZE);
	defers_stack[defers_stack_top++] = func;
}

void pop_defer(DeferFunc func) {
	assert(defers_stack_top > 0 &&
			(func == NULL || func == defers_stack[defers_stack_top - 1]));
	defers_stack[--defers_stack_top]();
}

void pop_all_defers() {
	while (defers_stack_top > 0)
		defers_stack[--defers_stack_top]();
}

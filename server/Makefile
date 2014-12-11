CC=gcc
CFLAGS=-std=gnu99 -pedantic -Wall -O3
LDFLAGS=-lgiblib -lImlib2 -lpthread -lrt -lX11

SOURCES=control.c ../common/exceptions.c ../common/ini_parser.c main.c options.c profiler.c screen.c shortcuts.c transfer.c
DEPS=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=inkmonitor-server

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@

-include $(DEPS)

%.o: %.c
	$(CC) -MM $(CFLAGS) $< -MF $(patsubst %.o,%.d,$@)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(DEPS) $(OBJECTS) $(EXECUTABLE)

CC=gcc
CFLAGS=-std=gnu99 -pedantic -Wall -O3
LDFLAGS=-lgiblib -lImlib2 -lpthread -lrt -lX11

SOURCES=control.c ../common/exceptions.c ../common/ini_parser.c main.c options.c profiler.c screen.c shortcuts.c transfer.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=inkmonitor-server

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $? $(LDFLAGS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

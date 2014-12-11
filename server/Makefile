SOURCES=control.c ../common/exceptions.c ../common/ini_parser.c main.c options.c profiler.c screen.c shortcuts.c transfer.c
OBJDIR=obj
OBJECTS=$(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(SOURCES))))
CONFIG=$(OBJDIR)/inkmonitor-server.ini
EXECUTABLE=$(OBJDIR)/inkmonitor-server

CC=gcc
CFLAGS=-std=gnu99 -pedantic -Wall -O3
LDFLAGS=
LIBS=-lgiblib -lImlib2 -lpthread -lrt -lX11

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJDIR) $(CONFIG) $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)

$(OBJDIR):
	mkdir $(OBJDIR)

$(CONFIG):
	cp default.ini $(CONFIG)

-include $(OBJDIR)/*.d

CDEPS = -MT$@ -MF$(addsuffix .d, $(basename $@)) -MMD -MP

$(OBJDIR)/%.c.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $(CDEPS) $<

$(OBJDIR)/%.c.o: ../common/%.c
	$(CC) -c $(CFLAGS) -o $@ $(CDEPS) $<

clean:
	rm -rf $(OBJDIR)

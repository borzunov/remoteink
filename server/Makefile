SOURCES=../common/exceptions.c ../common/ini_parser.c ../common/utils.c control.c main.c options.c profiler.c screen.c security.c shortcuts.c transfer.c
OBJDIR=obj
OBJECTS=$(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(SOURCES))))
EXECUTABLE=$(OBJDIR)/inkmonitord

CC=gcc
CFLAGS=-std=gnu99 -pedantic -Wall -Wshadow -O3
LDFLAGS=
LIBS=-lcrypt -lImlib2 -lm -lpthread -lrt -lX11 -lxcb -lxcb-keysyms -lxcb-xfixes

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJDIR) $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)

$(OBJDIR):
	mkdir $(OBJDIR)

-include $(OBJDIR)/*.d

CDEPS = -MT$@ -MF$(addsuffix .d, $(basename $@)) -MMD -MP

$(OBJDIR)/%.c.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $(CDEPS) $<

$(OBJDIR)/%.c.o: ../common/%.c
	$(CC) -c $(CFLAGS) -o $@ $(CDEPS) $<

clean:
	rm -rf $(OBJDIR)

SOURCES=../common/exceptions.c ../common/ini_parser.c ../common/utils.c control.c main.c options.c profiler.c screen.c security.c shortcuts.c transfer.c
OBJDIR=obj
OBJECTS=$(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(SOURCES))))
BINDIR=bin
EXECUTABLE=$(BINDIR)/inkmonitord

EXECUTABLE_PASSWD=inkmonitord-passwd
CONFIG=config.ini

CC=gcc
CFLAGS=-std=gnu99 -pedantic -Wall -Wshadow -O3
LDFLAGS=
LIBS=-lcrypt -lImlib2 -lm -lpthread -lrt -lX11 -lxcb -lxcb-keysyms -lxcb-xfixes

.PHONY: all
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJDIR) $(OBJECTS) $(BINDIR)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)

$(OBJDIR):
	mkdir $(OBJDIR)

$(BINDIR):
	mkdir $(BINDIR)

-include $(OBJDIR)/*.d

CDEPS = -MT$@ -MF$(addsuffix .d, $(basename $@)) -MMD -MP

$(OBJDIR)/%.c.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $(CDEPS) $<

$(OBJDIR)/%.c.o: ../common/%.c
	$(CC) -c $(CFLAGS) -o $@ $(CDEPS) $<

.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(BINDIR)


INSTALL_DIR?=/usr

INSTALL=install
INSTALLDATA=install -m 644

.PHONY: install
install: all inkmonitord-passwd config.ini
	$(INSTALL) $(EXECUTABLE) $(INSTALL_DIR)/bin/inkmonitord
	$(INSTALL) $(EXECUTABLE_PASSWD) $(INSTALL_DIR)/bin/inkmonitord-passwd
	mkdir -p /etc/inkmonitord
	$(INSTALLDATA) $(CONFIG) /etc/inkmonitord/config.ini

.PHONY: uninstall
uninstall:
	rm -f $(INSTALL_DIR)/bin/inkmonitord
	rm -f $(INSTALL_DIR)/bin/inkmonitord-passwd
	rm -rf /etc/inkmonitord

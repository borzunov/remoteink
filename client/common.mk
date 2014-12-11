SDKDIR = /usr/local/pocketbook
OBJDIR = obj_$(BUILD)
SYSTEM_LINK = $(OBJDIR)/system

-include $(OUT).mk

ifeq ($(BUILD), arm)
PROJECT = $(OBJDIR)/$(OUT).app
LIBS += -linkview
INCLUDES +=
CC  = arm-linux-gcc
CFLAGS += -Os -D__ARM__ -Wall -fomit-frame-pointer
CXX = arm-linux-g++
CXXFLAGS += -Os -D__ARM__ -Wall -fomit-frame-pointer
LD = arm-linux-g++
LDFLAGS += -Wl,-s
endif

ifeq ($(BUILD), arm_gnueabi)
PROJECT = $(OBJDIR)/$(OUT).app
LIBS += -linkview
CC  = arm-none-linux-gnueabi-gcc
CFLAGS += -Os -D__ARM__ -Wall -fomit-frame-pointer
CXX = arm-none-linux-gnueabi-g++
CXXFLAGS += -Os -D__ARM__ -Wall -fomit-frame-pointer
LD = arm-none-linux-gnueabi-g++
LDFLAGS += -Wl,-s
endif

ifeq ($(BUILD), emu)
PROJECT = $(OBJDIR)/$(OUT)
LIBS += -linkview
INCLUDES +=
CC  = gcc
CFLAGS += -D__EMU__ -DIVSAPP -Wall -g -m32
CXX = g++
CXXFLAGS += -D__EMU__ -DIVSAPP -Wall -g -m32
LD = g++
LDFLAGS += -m32
endif

ifeq ($(BUILD), std)
PROJECT = $(OBJDIR)/$(OUT)
LIBS += -lpthread
CC  = gcc
CFLAGS += -Wall -g
CXX = g++
CXXFLAGS += -Wall -g
LD = g++
endif

all: $(PROJECT)

clean:
ifeq ($(BUILD), emu)
	rm -f $(SYSTEM_LINK)
endif
	rm -rf $(OBJDIR)*

CDEPS = -MT$@ -MF`echo $@ | sed -e 's,\.o$$,.d,'` -MD -MP

OBJS		= $(addprefix $(OBJDIR)/, $(addsuffix .o, $(notdir $(SOURCES))))
BITMAP_OBJS = $(addsuffix .c.o, $(addprefix $(OBJDIR)/, $(notdir $(BITMAPS))))

# don't delete the intermediate files after make success
.PRECIOUS: $(OBJDIR)/%.bmp.c

$(OBJDIR):
	mkdir $(OBJDIR)

$(SYSTEM_LINK):
ifeq ($(BUILD), emu)
	ln -s $(SDKDIR)/system $(SYSTEM_LINK)
endif

$(OBJDIR)/%.bmp.c.o: $(OBJDIR)/%.bmp.c
	$(CC) -c -o $@ $(CFLAGS) $(INCLUDES) $(CDEPS) $<

$(OBJDIR)/%.c.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $(INCLUDES) $(CDEPS) $<

$(OBJDIR)/%.c.o: ../common/%.c
	$(CC) -c -o $@ $(CFLAGS) $(INCLUDES) $(CDEPS) $<

$(OBJDIR)/%.cpp.o: %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $(INCLUDES) $(CDEPS) $<

$(OBJDIR)/%.bmp.c: %.bmp
	pbres -c $@ $<

$(PROJECT) : $(OBJDIR) $(SYSTEM_LINK) $(OBJS) $(BITMAP_OBJS)
	$(LD) -o $@ $(OBJS) $(BITMAP_OBJS) $(LDFLAGS) $(LIBS)

# Dependencies tracking:
-include $(OBJDIR)/*.d

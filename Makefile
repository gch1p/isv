CC := gcc

PROGRAM = isv

OS = $(shell uname -s)
ifeq ($(OS),Linux)
	HIDAPI = hidapi-hidraw
endif
ifeq ($(OS),Darwin)
	HIDAPI = hidapi
endif

CFLAGS  = -O2 -std=c99
CFLAGS += -Wall -W
CFLAGS += `pkg-config --cflags $(HIDAPI)`
LDFLAGS  = -lm
LDFLAGS += `pkg-config --libs $(HIDAPI)`

INSTALL = /usr/bin/env install
PREFIX	= /usr/local

OBJS = isv.o util.o p18.o print.o variant.o
OBJS += libvoltronic/voltronic_dev_usb_hidapi.o
OBJS += libvoltronic/voltronic_crc.o
OBJS += libvoltronic/voltronic_dev.o

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

install: $(PROGRAM)
	$(INSTALL) $(PROGRAM) $(PREFIX)/bin

clean:
	rm -f $(OBJS) $(PROGRAM)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -I. -o $@

.PHONY: all install clean distclean
CC=gcc
CFLAGS = -Wall
LDFLAGS =

.DEFAULT_GOAL := all

SRC = $(wildcard src/*.c)
OBJ = $(patsubst %.c,%.o,$(wildcard src/*.c))
TARGET = lidar

.PHONY: all clean clobber

depend: .depend
.depend: $(SRC)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^ -MF ./.depend

include .depend

debug ?= 1 
ifeq ($(debug),1) 
	CFLAGS += -g
else
	CFLAGS += -O3
endif

all: $(OBJ)
	$(CC) $^ $(CFLAGS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

clean:
	rm -f $(OBJ)

clobber: clean
	rm -f $(TARGET)
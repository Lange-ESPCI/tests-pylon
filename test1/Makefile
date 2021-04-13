.PHONY: all clean

PYLON_ROOT ?= /opt/pylon32

CC = gcc
CPPFLAGS   := $(shell $(PYLON_ROOT)/bin/pylon-config --cflags pylonc)
CFLAGS := -W -Wall -std=c17 -pedantic -g -c -O0 #debug
LDFLAGS := $(shell $(PYLON_ROOT)/bin/pylon-config --libs-rpath pylonc) -lpng -lm -lpthread
LDLIBS     := $(shell $(PYLON_ROOT)/bin/pylon-config --libs pylonc)
EXEC := test1
BUILD_DIR ?= build
SRC_DIR ?= src
SRC = $(shell find $(SRC_DIR) -name *.c)
OBJ=$(SRC:%.c=$(BUILD_DIR)/%.o)

create_dir = @mkdir -p $(@D)

$(BUILD_DIR)/$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	
$(BUILD_DIR)/%.o: %.c
	$(create_dir)
	$(CC) -o $@ -c $< $(CFLAGS) $(CPPFLAGS)
	
clean:
	rm -rf $(BUILD_DIR) 

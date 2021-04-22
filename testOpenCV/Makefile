.PHONY: all clean

EXEC := testOpenCV

PYLON_ROOT ?= /opt/pylon32

CC = g++
CPPFLAGS := $(shell $(PYLON_ROOT)/bin/pylon-config --cflags) -I/usr/local/include/opencv4
CXXFLAGS := -W -Wall -std=c++17 -pedantic -g -c -O0
LDFLAGS := $(shell $(PYLON_ROOT)/bin/pylon-config --libs-rpath)
LDLIBS := $(shell $(PYLON_ROOT)/bin/pylon-config --libs) -lopencv_core -lopencv_highgui -lopencv_videoio

BUILD_DIR := build
SRC_DIR := src

SRC := $(shell find $(SRC_DIR) -name *.cpp)
OBJ := $(SRC:%.cpp=$(BUILD_DIR)/%.o)

create_dir = @mkdir -p $(@D)

$(BUILD_DIR)/$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	
$(BUILD_DIR)/%.o: %.cpp
	$(create_dir)
	$(CC) -o $@ -c $< $(CXXFLAGS) $(CPPFLAGS)
	
clean:
	rm -rf $(BUILD_DIR) 
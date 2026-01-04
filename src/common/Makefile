# CC=gcc
# SOURCES=$(wildcard *.c)
# #SOURCES+=../../testFrame/src/testFrame.c
# CFLAGS+=-O -g -Wall -Wextra
# OBJECTS+=$(patsubst %.c, %.o, $(SOURCES))
# TARGET=test.o

# $(TARGET):$(OBJECTS)
# 	#$(CC) $(OBJECTS) $(CFLAGS) -o $@
# 	$(LD) -r -o $@ $(OBJECTS)

# %.o:%.c
# 	$(CC) $(CFLAGS) -c -o $@ $<

# clean:
# 	-rm -f $(OBJECTS) $(TARGET)

# File paths
SRC_DIR := ./
OBJ_DIR := ./

# Compilation flags
CC := gcc
CFLAGS := -Wall -Wextra #-DTESTCASE_FROM_FILE

# Files to be compiled
TARGET := builtin.o
SRCS := $(wildcard $(SRC_DIR)/*.c) #SRCS set to all *.c files
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)%.o) #OBJS set to all *.o files

# Complie each *.c file as *.o file
$(TARGET): $(OBJS)
	$(LD) -r -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	-rm -f $(OBJS) $(TARGET)
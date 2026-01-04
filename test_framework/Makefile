# CC=gcc
# SOURCES=$(wildcard *.c)
# CFLAGS+=-O -g -Wall -Wextra -I../common
# OBJECTS+=$(patsubst %.c, %.o, $(SOURCES))
# LIST_OBJ=../common/test.o
# OBJECTS+=$(LIST_OBJ)
# TARGET=yamm

# $(TARGET):$(OBJECTS)
# 	$(CC) $(OBJECTS) $(CFLAGS) -o $@

# $(LIST_OBJ):
# 	make -C ../common/

# %.o:%.c
# 	$(CC) $(CFLAGS) -c -o $@ $<

# clean:
# 	-rm -f $(OBJECTS) $(TARGET)
# 	make -C ../common/ clean

SRC_DIR := ./
OBJ_DIR := ./

# Compilation flags
CC := gcc
CFLAGS := -Wall -Wextra -I../src/common -I../src/yamm -I./ #-DTESTCASE_FROM_FILE

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
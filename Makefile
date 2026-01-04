CC=gcc
SOURCES=$(wildcard *.c)
CFLAGS+=-O -g -Wall -Wextra -I./test_framework
OBJECTS+=$(patsubst %.c, %.o, $(SOURCES))
COMMON_OBJ=./src/common/builtin.o
YAMM_OBJ=./src/yamm/builtin.o
YAMM_TEST_OBJ=./test/builtin.o
TEST_FRAME_OBJ=./test_framework/builtin.o
OBJECTS+=$(COMMON_OBJ)
OBJECTS+=$(YAMM_OBJ)
OBJECTS+=$(YAMM_TEST_OBJ)
OBJECTS+=$(TEST_FRAME_OBJ)
TARGET=main

$(TARGET):$(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $@

$(COMMON_OBJ):
	make -C ./src/common/

$(YAMM_OBJ):
	make -C ./src/yamm/

$(YAMM_TEST_OBJ):
	make -C ./test/

$(TEST_FRAME_OBJ):
	make -C ./test_framework/

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<


clean:
	-rm -f $(OBJECTS) $(TARGET)
	make -C ./src/common/ clean
	make -C ./src/yamm/ clean
	make -C ./test/ clean
	make -C ./test_framework/ clean
CC=gcc
CC_FLAGS= -g -O0 -Wall -m32

HYPERVISOR_DIR=../../src
TEST_DIR=${HYPERVISOR_DIR}/../test
APP_NAME=frameAllocatorTest

CC_FLAGS+=-I. -I${HYPERVISOR_DIR}/common\
              -I${HYPERVISOR_DIR}/hardware\
	      -I${HYPERVISOR_DIR}/memoryManager  #Pull the .h files we need

SOURCE_FILES=frameAllocatorTest.c \
             ${TEST_DIR}/common/serial.c \
             ${HYPERVISOR_DIR}/memoryManager/frameAllocator.c

all:  build-test

build-test: $(APP_NAME)
frameAllocatorTest: ${SOURCE_FILES}
	${CC} ${CC_FLAGS} -o ${APP_NAME} ${SOURCE_FILES} -D FRAME_TABLE_ALLOC_TEST

run-test: $(APP_NAME)
	./$(APP_NAME)

.PHONY: clean
clean:
	@rm -f $(APP_NAME) *~
	@rm -f *.o
	@clear

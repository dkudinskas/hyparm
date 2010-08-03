CC=gcc
CC_FLAGS= -g -O0 -Wall -m32

HYPERVISOR_DIR=../../src
TEST_DIR=${HYPERVISOR_DIR}/../test
APP_NAME=pageTableTest

CC_FLAGS+=-I. -I${HYPERVISOR_DIR}/common\
              -I${HYPERVISOR_DIR}/hardware\
	      -I${HYPERVISOR_DIR}/memoryManager  #Pull the .h files we need

SOURCE_FILES=pageTableTest.c \
             ${TEST_DIR}/common/serial.c \
             ${TEST_DIR}/common/frameAllocator.c \
             ${HYPERVISOR_DIR}/memoryManager/pageTable.c

all:  build-test

build-test: $(APP_NAME)
pageTableTest: ${SOURCE_FILES}
	${CC} ${CC_FLAGS} -o ${APP_NAME} ${SOURCE_FILES} -D PAGE_TABLE_TEST

run-test: $(APP_NAME)
	./$(APP_NAME)

.PHONY: clean
clean:
	@rm -f $(APP_NAME) *~
	@rm -f *.o
	@clear

TEST_NAME     = sscanf

SRC_PATH      = ../../src
SRC_FILES     = common/stdio.c
SRC_FILES_FP  = $(foreach SRC_FILE, $(SRC_FILES), $(SRC_PATH)/$(SRC_FILE))

TST_PATH      = ../include
TST_FILES     =
TST_FILES_FP  = $(foreach TST_FILE, $(TST_FILES), $(TST_PATH)/$(TST_FILE))

CFLAGS        = -m32 -Wall -Wextra -DTEST=1 -iquote $(TST_PATH) -iquote $(SRC_PATH)
LDFLAGS       =

.PHONY: clean run

$(TEST_NAME): $(TST_FILES_FP) $(SRC_FILES_FP) $(TEST_NAME).c
	gcc -o $@ $(CFLAGS) $(LDFLAGS) $^

clean:
	rm $(TEST_NAME)

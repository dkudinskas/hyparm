TEST_NAME     = cli
CFLAGS        = -Wall -Wextra -DTEST_CLI=1 -iquote ../include -iquote $(SRC_PATH)
LDFLAGS       = -lncurses

SRC_PATH      = ../../src
SRC_FILES     = cli.c common/stringFunctions.c
SRC_FILES_FP  = $(foreach SRC_FILE, $(SRC_FILES), $(SRC_PATH)/$(SRC_FILE))


.PHONY: clean run

$(TEST_NAME): $(SRC_FILES_FP) $(TEST_NAME).c
	gcc -o $@ $(CFLAGS) $(LDFLAGS) $^

clean:
	rm $(TEST_NAME)

CC := gcc
SRCD := src
TSTD := tests
BLDD := build
BIND := bin
INCD := include

ALL_SRCF := $(shell find $(SRCD) -type f -name *.c)
ALL_OBJF := $(patsubst $(SRCD)/%,$(BLDD)/%,$(ALL_SRCF:.c=.o))
FUNC_FILES := $(filter-out build/main.o, $(ALL_OBJF))
TEST_SRC := $(shell find $(TSTD) -type f -name *.c)
INC := -I $(INCD)

EXEC := hi
TEST_EXEC := $(EXEC)_test

CFLAGS := -Wall -Werror
DFLAGS := -g -DDEBUG
STD := -std=gnu11
TEST_LIB := -lcriterion

LIBS := -lreadline -pthread -ljansson

CFLAGS += $(STD)

.PHONY: clean all

debug: CFLAGS += -g -DDEBUG
debug: all

all: setup $(EXEC) $(TEST_EXEC)

setup:
	mkdir -p bin build

$(EXEC): $(ALL_OBJF)
	$(CC) $^ -o $(BIND)/$@


$(TEST_EXEC): $(ALL_OBJF)
	$(CC) $(CFLAGS) $(INC) $(FUNC_FILES) $(TEST_SRC) $(TEST_LIB) -o $(BIND)/$@

$(BLDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	$(RM) -r $(BLDD) $(BIND)

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

CFLAGS := -Wall -Werror -Wextra -Wno-variadic-macros
DFLAGS := -g -DDEBUG -DCOLOR

STD := -std=c99
TEST_LIB := -lcriterion
ifeq ($(shell uname), SunOS)
	LIBS := -lsendfile
else
	LIBS :=
endif

EXEC := utf
TEST_EXEC := $(EXEC)_tests

.PHONY: clean all setup format

all: setup $(EXEC) $(TEST_EXEC)

debug: CFLAGS += $(DFLAGS)
debug: all

setup:
	mkdir -p bin build

$(EXEC): $(ALL_OBJF)
	$(CC) $(CFLAGS) $(STD) $^ -o $(BIND)/$@ $(LIBS)

$(TEST_EXEC): $(FUNC_FILES)
	$(CC) $(CFLAGS) -std=gnu11 $(INC) $(FUNC_FILES) $(TEST_SRC) $(TEST_LIB) -o $(BIND)/$@

$(BLDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	rm -rf $(BLDD) $(BIND)

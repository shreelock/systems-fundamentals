include Makefile.config

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

CFLAGS := -Wall -Werror
COLORF := -DCOLOR
DFLAGS := -g -DDEBUG
PRINT_STAMENTS := -DERROR -DSUCCESS -DWARN -DINFO

STD := -std=gnu11
TEST_LIB := -lcriterion -lreadline
LIBS := -lreadline

CFLAGS += $(STD)

EXEC := sfish
TEST_EXEC := $(EXEC)_tests

.PHONY: clean all

all: setup $(EXEC) $(TEST_EXEC)

debug: CFLAGS += $(DFLAGS) $(PRINT_STAMENTS) $(COLORF)
debug: all

setup:
	@echo $(EC)
	mkdir -p bin build

$(EXEC): $(ALL_OBJF)
	$(CC) $^ $(LIBS) -o $(BIND)/$@ $(LIBS)

$(TEST_EXEC): $(FUNC_FILES)
	$(CC) $(CFLAGS) $(EC) $(INC) $(FUNC_FILES) $(TEST_SRC) -o $(BIND)/$(TEST_EXEC) $(TEST_LIB)

$(BLDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) $(EC) $(INC) -c -o $@ $<

clean:
	rm -rf $(BLDD) $(BIND)

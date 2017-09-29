CC := gcc
SRCD := src
TSTD := tests
BLDD := build
BIND := bin
INCD := include
LIBD := lib

SRCF := $(shell find $(SRCD) -type f -name *.c)
TSTF := $(shell find $(TSTD) -type f -name *.c)

SRC_OBJF := $(patsubst $(SRCD)/%,$(BLDD)/%,$(SRCF:.c=.o))
TST_OBJF := $(patsubst $(TSTD)/%,$(BLDD)/%,$(TSTF:.c=.o))

ALL_OBJF := $(SRC_OBJF) $(TST_OBJF)

INC := -I$(INCD)

CFLAGS := -Wall -Werror
DFLAGS := -g -DDEBUG -DCOLOR

STD := -std=gnu11
TEST_LIB := -lcriterion

EXEC := sfmm
EXEC_TEST := $(EXEC)_tests

.PHONY: clean all setup format

all: setup $(EXEC) $(EXEC_TEST)

debug: CFLAGS += $(DFLAGS)
debug: all

setup:
	mkdir -p bin build

$(EXEC): $(SRC_OBJF)
	$(CC) $(CFLAGS) $(STD) $(INC) $(LIBD)/sfutil.o $^ -o $(BIND)/$@

$(EXEC_TEST): $(filter-out build/main.o, $(ALL_OBJF))
	$(CC) $(CFLAGS) $(STD) $(INC) $(LIBD)/sfutil.o $^ -o $(BIND)/$@ $(TEST_LIB)

$(BLDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) $(STD) $(INC) -c $< -o $@

$(BLDD)/%.o: $(TSTD)/%.c
	$(CC) $(CFLAGS) $(STD) $(INC) -c $< -o $@

clean:
	rm -rf $(BIND) $(BLDD)

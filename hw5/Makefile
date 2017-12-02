CC := gcc
SRCD := src
TSTD := tests
BLDD := build
BIND := bin
INCD := include

MAP_SRCF := $(SRCD)/hashmap.c
EC_MAP_SRCF := $(SRCD)/extracredit.c

MAP_OBJF := $(BLDD)/hashmap.o
EC_MAP_OBJF := $(BLDD)/extracredit.o

MAP_TESTF := $(TSTD)/hashmap_tests.c
EC_TESTF := $(TSTD)/extracredit_tests.c

MAIN  := build/cream.o

ALL_SRCF := $(filter-out $(MAP_SRCF) $(EC_MAP_SRCF), $(wildcard $(SRCD)/*.c))
ALL_OBJF := $(patsubst $(SRCD)/%, $(BLDD)/%, $(ALL_SRCF:.c=.o))
ALL_FUNCF := $(filter-out $(MAIN), $(ALL_OBJF))
ALL_TESTF := $(filter-out $(MAP_TESTF) $(EC_TESTF), $(wildcard $(TSTD)/*.c))

INC := -I $(INCD)

CFLAGS := -Wall -Werror
DFLAGS := -g -DDEBUG
ECFLAGS := -DEC

STD := -std=gnu11
TEST_LIB := -lcriterion

CFLAGS += $(STD)

EXEC := cream
TEST_EXEC := $(EXEC)_tests
LIBS := -lpthread

.PHONY: clean all
.DEFAULT: clean all

all: TEST_SRC = $(ALL_TESTF) $(MAP_TESTF)
all: setup all_exec all_test_exec

ec: CFLAGS += $(ECFLAGS)
ec: TEST_SRC = $(ALL_TESTF) $(EC_TESTF)
ec: setup ec_exec ec_test_exec

debug: CFLAGS += $(DFLAGS)
debug: all

debug_ec: CFLAGS += $(DFLAGS)
debug_ec: ec

setup:
	mkdir -p bin build

all_exec: $(ALL_OBJF) $(MAP_OBJF)
	$(CC) $^ -o $(BIND)/$(EXEC) $(LIBS)

all_test_exec: $(ALL_FUNCF) $(MAP_OBJF)
	$(CC) $(CFLAGS) $(INC) $^ $(TEST_SRC) -o $(BIND)/$(TEST_EXEC) $(TEST_LIB) $(LIBS)

ec_exec: $(ALL_OBJF) $(EC_MAP_OBJF)
	$(CC) $^ -o $(BIND)/$(EXEC) $(LIBS)

ec_test_exec: $(ALL_FUNCF) $(EC_MAP_OBJF)
	$(CC) $(CFLAGS) $(INC) $^ $(TEST_SRC) -o $(BIND)/$(TEST_EXEC) $(TEST_LIB) $(LIBS)

$(BLDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -rf $(BLDD) $(BIND)

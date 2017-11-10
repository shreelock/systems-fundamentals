CC := gcc
SRCD := src
TSTD := tests
BLDD := build
BIND := bin
INCD := include

DEPS = ${BLDD}/hashmap.o
EC_DEPS = ${BLDD}/extracredit.o

TESTS = ${TSTD}/hashmap_tests.c
EC_TESTS = ${TSTD}/extracredit_tests.c

ALL_SRCF := $(shell find $(SRCD) -type f -name *.c)
ALL_OBJF := $(patsubst $(SRCD)/%, $(BLDD)/%, $(ALL_SRCF:.c=.o))
ALL_FUNCF := $(filter-out build/cream.o, $(ALL_OBJF))
ALL_TESTF := $(shell find $(TSTD) -type f -name *.c)

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

all: DEP_OBJS := $(filter-out $(EC_DEPS), $(ALL_OBJF))
all: DEP_FUNCS := $(filter-out $(EC_DEPS), $(ALL_FUNCF))
all: TEST_SRC := $(filter-out $(EC_TESTS), $(ALL_TESTF))
all: compile

ec: DEP_OBJS := $(filter-out $(DEPS), $(ALL_OBJF))
ec: DEP_FUNCS := $(filter-out $(DEPS), $(ALL_FUNCF))
ec: TEST_SRC := $(filter-out $(TESTS), $(ALL_TESTF))
ec: CFLAGS += $(ECFLAGS)
ec: compile

compile: setup $(EXEC) $(TEST_EXEC)

debug: CFLAGS += $(DFLAGS)
debug: all

setup:
	mkdir -p bin build

$(EXEC): $(ALL_OBJF)
	$(CC) $(DEP_OBJS) -o ${BIND}/$@ $(LIBS)

$(TEST_EXEC): $(ALL_FUNCF)
	$(CC) $(CFLAGS) $(INC) $(DEP_FUNCS) $(TEST_SRC) -o $(BIND)/$@ $(TEST_LIB) $(LIBS)

$(BLDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -rf $(BLDD) $(BIND)

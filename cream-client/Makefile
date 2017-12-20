CC := gcc
BLDD := build
BIND := bin
INCD := include
LIBD := lib
SRCD := src

INC := -I $(INCD)

CFLAGS := -std=gnu11
CFLAGS += -Wall -Werror

LIBS := -lpthread

DFLAGS := -g -DDEBUG

LIB_OBJF := $(patsubst $(LIBD)/%.c,$(BLDD)/%.o,$(wildcard $(LIBD)/*.c))

.PHONY: clean all
.DEFAULT: clean all

all: setup ${BIND}/cream_client

debug: CFLAGS += $(DFLAGS)
debug: all

setup:
	mkdir -p bin build

${BIND}/cream_client: $(LIB_OBJF) ${BLDD}/cream_client.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

${BLDD}/cream_client.o:
	$(CC) $(CFLAGS) $(INC) -c ${SRCD}/cream_client.c -o $@ 

${BLDD}/%.o: $(LIBD)/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	rm -rf $(BLDD) $(BIND)

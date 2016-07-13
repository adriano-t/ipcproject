# Sources:
SRCS = ipc_calculator.c utils.c

# Objects:
OBJS = $(SRCS:.c=.o)

# Config:
CC:=gcc
CFLAGS:=-c -g -Wall 
LD:=gcc
LDFLAGS:=-g  -pthread

all: ipc_calculator.x

clean:
	@echo Cleaning..
	@rm -f *.o
	@rm -f ipc_calculator.x
	@echo done.

ipc_calculator.x: $(OBJS)
	@echo Linking $@
	@$(LD) $(LDFLAGS) -o $@ $^

utils.o: utils.c utils.h
	@echo $@
	@$(CC) $(CFLAGS) -o $@ $<

ipc_calculator.o: ipc_calculator.c  ipc_calculator.h utils.h
	@echo $@
	@$(CC) $(CFLAGS) -o $@ $<

utils.h:
	@echo $@
	@touch $@

# Collects as dependencies targets which do not match
# any generated file name (e.g. all, clean).
.PHONY: all clean




SRCS = shell.cpp
CFLAGS = -O2
PROT =
OUT = shell
CC = g++

all:
	$(CC) $(CFLAGS) $(PROT) $(SRCS) -o $(OUT)

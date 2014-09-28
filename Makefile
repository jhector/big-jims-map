SRCS = server.cpp
CFLAGS = -O2
PROT =
OUT = server
CC = g++

all:
	$(CC) $(CFLAGS) $(PROT) $(SRCS) -o $(OUT)

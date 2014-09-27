SRCS = server.cpp
CFLAGS = -O2 -L ./lib -Xlinker "-rpath=./lib/"
PROT =
OUT = server
CC = g++

all:
	$(CC) $(CFLAGS) $(PROT) $(SRCS) -o $(OUT)

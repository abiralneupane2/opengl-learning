CC=gcc
SRCDIR=src
SRC=main.c
OBJ=main.o
PKGS=`pkg-config --cflags --libs gtk4`
LIBS = -lepoxy -lm

build:
	$(CC) -o $(OBJ) $(SRCDIR)/$(SRC) $(PKGS) $(LIBS)

clean:
	rm $(OBJ)

run: clean build
	./main.o

debug: clean
	$(CC) -g -o $(OBJ) $(SRCDIR)/$(SRC) $(PKGS) $(LIBS)
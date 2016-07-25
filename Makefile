CC = gcc
CFLAG = -g -I ./include -O -o

CLFLAG = -c $(CFLAG)

objects = io/io.o fs/build.o unbuffered/fs.o unbuffered/directory.o fs/inode.o journal/journal.o unbuffered/file.o

all: test $(objects)

%.o:%.c
	$(CC) $(CLFLAG) $@ $<
test: $(objects)
	$(CC) $(CFLAG) test test.c $(objects)

clean:
	rm $(objects)

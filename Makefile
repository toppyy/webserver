IDIR =./include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=./obj
SDIR=./src
LDIR =./lib
LIBS=-lm

_DEPS = server.h respond.h parse.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = server.o respond.o parse.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))



$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
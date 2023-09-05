IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=../obj
LDIR =../lib
LIBS=-lm

_DEPS = server.h client.h factory.h respond.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = server.o client.o factory.o respond.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
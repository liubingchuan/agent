INCDIR = ./include
LIBDIR = ./lib
INCLUDE = -I$(INCDIR)
CC = cc
all: commserv

OBJ=commserv.o commtools.o

commserv.o:commserv.h

commserv:$(OBJ)
	cc $(OBJ) -Insl -o $@
	
$(OBJ):$(HEADERS)
.c.o:
	cc -c -O -g $(INCLUDE) $*.c
	
clean:
	rm commserv *.o
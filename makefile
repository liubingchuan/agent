INCDIR = ./include
LIBDIR = ./lib
INCLUDE = -l$(INCDIR)
CC = cc
all: commserv

OBJ=commserv.o commtools.o

commserv.o:commserv.h

commserv:$(OBJ)
	cc $(OBJ) -lnsl -o $@
	
$(OBJ):$(HEADERS)
.c.o:
	cc -c -O $(INCLUDE) $*.c
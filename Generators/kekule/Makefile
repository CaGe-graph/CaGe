CC = cc
CFLAGS = -O4 -w

TARG = fusgen$(SUFFIX)
OBJS = fusgen.o kekule.o kekule_benz.o match.o modes.o label.o label_benz.o horloge.o

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS)

clear:
	rm -f $(TARG)*.log *core

clean: clear
	rm -f *.o $(TARG)

$(OBJS): fusgen.h
fusgen.o horloge.o: horloge.h

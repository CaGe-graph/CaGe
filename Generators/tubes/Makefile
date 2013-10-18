CFLAGS = -w -O4

TARG = tube$(SUFFIX)
OBJS = tube.o

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS)

clean:
	rm -f *.o $(TARG)


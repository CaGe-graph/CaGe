CFLAGS = -Wall -g

TARG = cone$(SUFFIX)
OBJS = cone.o util.o twopentagons.o

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS)

clean:
	rm -f *.o $(TARG)


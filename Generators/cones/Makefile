CFLAGS = -w -O4

TARG = cone$(SUFFIX)
OBJS = cone.o pseudoconvex.o twopentagons.o oldspiral2planar.o

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS)

clean:
	rm -f *.o $(TARG)

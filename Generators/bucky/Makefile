CFLAGS = -w -O4

TARG = buckygen$(SUFFIX)
OBJS = buckygen.c

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS)

clean:
	rm -f *.o $(TARG)

CFLAGS=-O4 -w
# Under Windows, set SUFFIX to .exe
SUFFIX=

all: makelist$(SUFFIX)

clean:
	rm -f *.o bindata_boundaries_length_*

reset: clean
	rm -f makelist$(SUFFIX)

makelist$(SUFFIX): makelist.c ../Generators/vul_in.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -DMAINFUNCTION="int getyesno(int argc, char *argv[])" $^

compute: makelist$(SUFFIX)
	nice -n 19 ./makelist$(SUFFIX) 26 2>/dev/null || true
	nice -n 19 ./makelist$(SUFFIX) 30 2>/dev/null || true

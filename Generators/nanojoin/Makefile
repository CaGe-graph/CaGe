CFLAGS=-w -O4
OBJECTS = bottomup/bu_extra.o bottomup/bu_mergepaths.o bottomup/bu_nanojoin.o bottomup/bu_operations.o \
	topdown/td_graphutils.o topdown/td_nanojoin.o topdown/td_operations.o topdown/td_patchAdjacency.o topdown/td_bitvector.o topdown/td_isomorphismcheck.o\
	tree/tree.o topdown/td_isomorphismcheck.o\
	nanojoin.o

bottomup/%.o: bottomup/%.c
	mkdir -p bottomup
	$(CC) $(CFLAGS) -c -o $@ $<

topdown/%.o: topdown/%.c
	mkdir -p topdown
	$(CC) $(CFLAGS) -c -o $@ $<

tree/%.o: tree/%.c
	mkdir -p tree
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

join: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -fr bottomup/*.o
	rm -fr topdown/*.o
	rm -fr tree/*.o
	rm -f *.o

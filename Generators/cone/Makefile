all : cone conetest non_iso_pl

cone : cone.c util.c twopentagons.c
	cc cone.c util.c twopentagons.c -o cone -Wall -g

conetest : conetest.c
	cc conetest.c -o conetest -Wall

non_iso_pl : non_iso_pl.c hashfunction.c splay.c
	cc non_iso_pl.c -o non_iso_pl -Wall

clean:
	rm cone

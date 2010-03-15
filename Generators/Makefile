
# Makefile for CaGe generators & embedders
#
# Sebastian Lisken, Gunnar Brinkmann, Olaf Delgado, Thomas Harmuth


CFLAGS=-O4
# Under Windows, set SUFFIX to .exe
SUFFIX=


all: generators embedders others clean


generators: fullgen$(SUFFIX) cgf$(SUFFIX) hcgen$(SUFFIX) \
	    plantri$(SUFFIX) plantri_md6$(SUFFIX) plantri_ad$(SUFFIX) \
	    cat$(SUFFIX) tee$(SUFFIX) vul_in$(SUFFIX) catacondensed$(SUFFIX) \
	    kekule_all cone_all tube_all quad_restrict$(SUFFIX) leapfrog$(SUFFIX)

embedders: embed$(SUFFIX) cat$(SUFFIX) tee$(SUFFIX)

others: mkfoldnet

clean: kekule_clean cone_clean tube_clean
	rm -f *.o

clear: clean kekule_clear cone_clear tube_clear
	rm cat cgf embed fullgen hcgen plantri plantri_ad tee tubetype plantri_md6 vul_in catacondensed fusgen quad_restrict

reset: clean
	rm -f fullgen$(SUFFIX) cgf$(SUFFIX) tubetype$(SUFFIX) hcgen$(SUFFIX) plantri$(SUFFIX) plantri_md6$(SUFFIX) plantri_ad$(SUFFIX) embed$(SUFFIX) cat$(SUFFIX) vul_in$(SUFFIX) catacondensed$(SUFFIX) fusgen$(SUFFIX) quad_restrict$(SUFFIX)


fullgen$(SUFFIX) : fullgen.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

cpf$(SUFFIX) : CPF.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ -lmalloc

cgf$(SUFFIX) : CGF.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

hcgen$(SUFFIX) : HCgen.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

# Old plantri plugin "min4.c" now deleted.
# Functionality integrated since v3.1, use plantri -m4.
# More info: see the plantri/fullgen page, http://cs.anu.edu.au/~bdm/plantri/
# plantri_min4$(SUFFIX) : plantri.c min4.c
# 	$(CC) $(CFLAGS) $(CPPFLAGS) -o plantri_min4 -O '-DPLUGIN="min4.c"' plantri.c

plantri$(SUFFIX) : plantri.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

plantri_md6$(SUFFIX) : plantri_md6.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ -lm

plantri_ad$(SUFFIX) : plantri.c allowed_deg.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -O '-DPLUGIN="allowed_deg.c"' plantri.c

embed$(SUFFIX) : embed.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ -lm

cat$(SUFFIX) : cat.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

tee$(SUFFIX) : tee.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

mkfoldnet$(SUFFIX) : mkfoldnet.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ -lm

vul_in$(SUFFIX) : vul_in.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

catacondensed$(SUFFIX) : catacondensed.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

kekule_all:
	SUFFIX=$(SUFFIX) $(MAKE) -C kekule
	cp kekule/fusgen$(SUFFIX) .

kekule_clean:
	SUFFIX=$(SUFFIX) $(MAKE) -C kekule clean

kekule_clear:
	rm -f fusgen$(SUFFIX)

cone_all:
	SUFFIX=$(SUFFIX) $(MAKE) -C cones
	cp cones/cone$(SUFFIX) .

cone_clean:
	SUFFIX=$(SUFFIX) $(MAKE) -C cones clean

cone_clear:
	rm -f cone$(SUFFIX)

tube_all:
	SUFFIX=$(SUFFIX) $(MAKE) -C tubes
	cp tubes/tube$(SUFFIX) .

tube_clean:
	SUFFIX=$(SUFFIX) $(MAKE) -C tubes clean

tube_clear:
	rm -f tube$(SUFFIX)

quad_restrict$(SUFFIX) : plantri.c quad_restrict.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -O '-DPLUGIN="quad_restrict.c"' '-DALLTOGETHER' plantri.c -lm

leapfrog$(SUFFIX) : leapfrog.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

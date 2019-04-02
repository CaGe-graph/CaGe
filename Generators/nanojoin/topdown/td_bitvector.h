#ifndef TD_BITVECTOR_H
#define TD_BITVECTOR_H

struct bitvector {
	unsigned long long bits[10];
	unsigned long long maxbit;
	int lastused;
};

static const unsigned long long lastbit = ((unsigned long long) 1) << 63;
void bit_add(struct bitvector *vector, unsigned char bit);
void bit_rotate(struct bitvector *vector);
char bit_compare(struct bitvector *vector1, struct bitvector *vector2);

#endif /* TD_BITVECTOR_H */
#include<stdio.h>
#include<stdlib.h>

#include "bu_extra.h"

/*
 * Tells us if we have a repetitive border
 */
int find_repetition(char* input) {
	int parts, length, offset, index;
	length = 1;
	unsigned char found, ok;
	found = 0;
	while (!found && length < input[0]) {
		if (input[0] % length == 0) {
			parts = ((int) input[0]) / length;
			ok = 1;
			offset = 0;
			while (ok && offset < parts) {
				index = 0;
				while (ok && index < length) {
					if (input[offset * length + index + 1] != input[index + 1]) {
						ok = 0;
					}
					index++;
				}
				offset++;
			}
			if (ok) {
				found = 1;
				length--;
			}
		}
		length++;
	}
	return length;
}

/**
 * checks if bordercode is canonical
 */
int lexicographical_biggest(char* bordercode) {
	int found_bigger = 0;
	int i = 1;
	int j;
	int size = bordercode[0];

	while (found_bigger == 0 && i < size) {
		j = 0;
		while (j < size && bordercode[(i + j) % size + 1] == bordercode[j + 1]) {
			j++;
		}
		//make sure -1 is bigger than everything else
		if (j != size && (bordercode[j + 1] != -1 && (bordercode[(i + j) % size + 1] > bordercode[j + 1] || bordercode[(i + j) % size + 1] == -1))) {
			found_bigger = 1;
		}
		i++;
	}
	return found_bigger;
}

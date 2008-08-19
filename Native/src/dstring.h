
# ifndef DSTRING_INCLUDED
# define DSTRING_INCLUDED

/*
   These functions define a dynamic string that can also hold NUL bytes.
   Allocated memory increases and decreases in chunks of 1K.
   Length changes are cheap as long as no chunk boundary is crossed.
   dstrings are initialized like a simple C type, using "new_dstring"
   as an empty value.  Any functions that want to make changes to a
   dstring (like the ones defined here) will want a dstring pointer
   as an argument.  dstring has two members, base and length, that
   denote the memory area used by the string.
   After using the "add*" functions, "clear_dstring" must be called
   before discarding a dstring to avoid memory leaks.
*/


struct dstring
{
  char  *base;
  int   length;
  int   rest;
};
typedef struct dstring dstring;
extern const dstring new_dstring;


extern void set_length (dstring *s, int new_length);
/*
   The length is the number of usable bytes starting from "base".
*/

extern void add_char (dstring *s, int c);
/*
   Increases the length by 1 and writes c into the last byte.
*/

extern void add_bytes (dstring *s, char *bytes, int byteslength);
/*
   Adds the given number of bytes and sets them from "bytes".
*/

extern void add_string (dstring *s, char *string);
/*
   Adds a string, without the terminating NUL character.
*/

extern void clear_dstring (dstring *s);
/*
   Releases any memory allocated for s and resets s to the empty dstring.
*/


# endif /* DSTRING_INCLUDED */


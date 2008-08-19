
# include <stdlib.h>
# include <string.h>
# include <stdio.h>

# include "malloc.h"

# include "dstring.h"

/*
   The following types and functions define a "dynamic string"
   as a char array.  An int stores the length (the string does
   not need to be null-terminated).  Memory allocation is done
   in "chunks" of 1K (or some other size, ideally a power of 2).
   Length changes are cheap as long as the number of "chunks"
   does not change.

   Detailed function comments are in dstring.h.
*/


extern int debug_dstring;


const dstring new_dstring = { NULL, 0, 0 };


# define  STRING_CHUNK  1024

void
set_length (dstring *s, int new_length)
{
  int  new_rest, new_size;
  char *new_base;
  if (new_length < 0) new_length = 0;
  if (debug_dstring) fprintf (stderr, "new length: %d\n", new_length);
  new_rest = s->rest - (new_length - s->length);
  if (debug_dstring) fprintf (stderr, "new rest: %d\n", new_rest);
  if (new_rest < 0 || new_rest >= STRING_CHUNK) {
      new_size = new_length + (STRING_CHUNK - 1);
      new_size -= new_size % STRING_CHUNK;
      if (debug_dstring) fprintf (stderr, "new size: %d\n", new_size);
      new_rest %= STRING_CHUNK;
      new_rest += new_rest >= 0 ? 0 : STRING_CHUNK;
      if (debug_dstring) fprintf (stderr, "new rest: %d\n", new_rest);
      if (new_size == 0) {
          if (s->base != NULL) free (s->base);
	  new_base = NULL;
      } else {
	  new_base = realloc (s->base, new_size);
      }
      s->base = new_base;
  }
  s->length = new_length;
  s->rest = new_rest;
}

void
add_char (dstring *s, int c)
{
  int n = s->length;
  set_length (s, n+1);
  s->base [n] = (char) c;
}

void
add_bytes (dstring *s, char *bytes, int byteslength)
{
  int n = s->length;
  set_length (s, n + byteslength);
  memcpy (s->base + n, bytes, byteslength);
}

void
add_string (dstring *s, char *string)
{
  add_bytes (s, string, strlen (string));
}

void
clear_dstring (dstring *s)
{
  set_length (s, 0);
}



# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include "error_exit.h"


extern int debug_malloc;


void *xmalloc (size_t size)
{
  void *result = malloc (size);
  if (debug_malloc) fprintf (stderr, "+ malloc (%d) = %p\n", (int)size, result);
  if (result == NULL) {
      error_exit ("malloc failure", "lang/OutOfMemoryException");
  }
  return result;
}

void *xrealloc (void *ptr, size_t size)
{
  void *result = realloc (ptr, size);
  if (debug_malloc) fprintf (stderr, "o realloc (%p, %d) = %p\n", ptr, (int)size, result);
  if (size > 0) if (result == NULL) error_exit ("realloc failure", "lang/OutOfMemoryException");
  return result;
}

void xfree (void *ptr)
{
  if (debug_malloc) fprintf (stderr, "- free (%p)\n", ptr);
  free (ptr);
}

char *xstrdup (const char *string)
{
  char *result;
  int length;
  length = strlen (string) + 1;
  result = xmalloc (length);
  memcpy (result, string, length);
  return result;
}


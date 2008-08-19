
# include <stdio.h>

# include "error_exit.h"

# include "dstring.h"

# include "malloc.h"


char *cmd_name = "t_dstring";

int debug_dstring = 1, debug_malloc = 0;


int main ()
{
# define  X(p, base)  ((p) ? (p) - (base) : 0)
  dstring t = new_dstring;
  char *base = malloc (1), *b1, *b2, *b3;
  printf ("0x%08x %d %d\n", X (t.base, base), t.length, t.rest);
  set_length (&t, 1);
  printf ("0x%08x %d %d\n", X (t.base, base), t.length, t.rest);
  b1 = malloc (1024);
  printf ("0x%08x\n", X (b1, base));
  set_length (&t, 1023);
  printf ("0x%08x %d %d\n", X (t.base, base), t.length, t.rest);
  set_length (&t, 1024);
  printf ("0x%08x %d %d\n", X (t.base, base), t.length, t.rest);
  set_length (&t, 1025);
  printf ("0x%08x %d %d\n", X (t.base, base), t.length, t.rest);
  set_length (&t, 1023);
  printf ("0x%08x %d %d\n", X (t.base, base), t.length, t.rest);
  set_length (&t, 0);
  printf ("0x%08x %d %d\n", X (t.base, base), t.length, t.rest);
  b2 = malloc (1024);
  printf ("0x%08x\n", X (b2, base));
  free (b1);
  b1 = malloc (1024);
  printf ("0x%08x\n", X (b1, base));
  b3 = malloc (1024);
  printf ("0x%08x\n", X (b3, base));
  free (b1);
  free (b2);
  free (b3);
  return 0;
}


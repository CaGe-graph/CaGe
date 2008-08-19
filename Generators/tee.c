
# include <stdlib.h>
# include <stdio.h>


int main (int argc, char *argv[])
{
  FILE **outfile;
  int c, i;
  int value = 0;
  outfile = (FILE **) malloc (argc * sizeof (FILE *));
  outfile [0] = stdout;
  for (i = 1; i < argc; ++i)
  {
    if ((outfile [i] = fopen (argv [i], "wb")) == NULL) {
      fprintf (stderr, "%s: ", argv [0]);
      perror (argv [i]);
      value = 1;
    }
  }
  while ((c = getchar ()) != EOF)
  {
    for (i = 0; i < argc; ++i)
    {
      if (outfile [i] != NULL) {
        if (fputc (c, outfile [i]) == EOF) value = 1;
      }
    }
  }
  for (i = 0; i < argc; ++i)
  {
    fclose (outfile [i]);
  }
  return value;
}


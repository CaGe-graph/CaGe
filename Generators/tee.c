
# include <stdlib.h>
# include <stdio.h>


int main (int argc, char *argv[])
{
  FILE **outfile;
  int c, i;
  int value = 0;
  int append = 0;
  char option[2] = "wb";
  /* first check for the option a */
  if (argv[0][0] == '-' && argv[0][1] == 'a') {
    option[0] = 'a';
    append = 1;
  }
  outfile = (FILE **) malloc ((argc - append) * sizeof (FILE *));
  outfile [0] = stdout;
  for (i = 1 + append; i < argc; ++i)
  {
    if ((outfile [i] = fopen (argv [i], option)) == NULL) {
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


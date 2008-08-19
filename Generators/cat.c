
# include <stdio.h>


void cat (FILE *input)
{
  int c;
  while ((c = getc (input)) != EOF)
  {
    putchar (c);
  }
}


int main (int argc, char *argv[])
{
  int i, value;
  FILE *input;

  value = 0;
  if (argc <= 1) {
    cat (stdin);
  } else {
    for (i = 1; i < argc; ++i)
    {
      if ((input = fopen (argv [i], "rb")) != NULL) {
        cat (input);
      } else {
	fprintf (stderr, "%s: ", argv [0]);
        perror (argv [i]);
	value = 1;
      }
    }
  }
  return value;
}


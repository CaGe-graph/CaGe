
# include <stdio.h>
# include <unistd.h>
# include <fcntl.h>


int main (int argc, char *argv[])
{
  int c;
  printf ("fnctl: %d\n", fcntl (0, F_SETFL, O_NONBLOCK));
  printf ("getchar: %d\n", getchar ());
  printf ("fnctl: %d\n", fcntl (0, F_SETFL, 0));
  printf ("getchar: %d\n", getchar ());
  return 0;
}


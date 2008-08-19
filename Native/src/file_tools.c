
# include <unistd.h>
# include <fcntl.h>
# include <stdio.h>

# include "file_tools.h"


/*
   See "file_tools.h" for comment.
*/
int
next_byte (FILE *file)
{
  int fd, flags, c;
  fd = fileno (file);
  flags = fcntl (fd, F_GETFL) & (O_NONBLOCK | O_APPEND);
  if (fcntl (fd, F_SETFL, flags | O_NONBLOCK) == -1) return -2;
  c = getc (file);
  fcntl (fd, F_SETFL, flags);
  if (c == EOF) {
      c = -1;
  } else {
      ungetc (c, file);
  }
  return c;
}


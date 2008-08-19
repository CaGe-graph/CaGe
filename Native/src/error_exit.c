
# include <stdlib.h>
# include <stdio.h>
# include <errno.h>


extern char *cmd_name;


void
error_exit (char *msg, char *error_name)
{
  fprintf (stderr, "%s: fatal error (%s): %s\n", cmd_name, error_name, msg);
  if (errno > 0) {
      fprintf (stderr, "errno is %d: %s\n", errno, strerror (errno));
  }
  exit (2);
}

void
error_exit_subproc (char *msg, char *error_name)
{
  error_exit (msg, error_name);
}

void setJNIEnv (void *x)
{
}


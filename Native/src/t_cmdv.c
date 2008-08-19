
# include <stdio.h>

# include "error_exit.h"

# include "cmd_vector.h"


char *cmd_name = "t_cmdv";

int debug_malloc = 1;


void print_cmd_vector (struct cmd_vector *cmdv)
{
  char **argv;
  int  i;
  if (cmdv != NULL && cmdv->length > 0) {
      if (cmdv->rundir != NULL) printf ("RunDir: '%s'\n", cmdv->rundir);
      if (cmdv->path != NULL) printf ("Path: '%s'\n", cmdv->path);
      for (i = 0; ; )
      {
	for (argv = cmdv->cmd [i]; *argv; ++argv)
	{
	  printf (" '%s'", *argv);
	}
	if (++i < cmdv->length) {
	    printf (" | ");
	} else {
	    break;
	}
      }
      printf ("\n");
  }
  free_cmd_vector (cmdv);
}

main ()
{
  char response [80];
  fputs ("Enter commands to parse.  Empty line exits.\n", stderr);
  for ( ; ; )
  {
    fputs ("> ", stderr);
    fflush (stderr);
    if (! fgets (response, sizeof (response), stdin)) break;
    if (*response == '\n' || *response == '\0') break;
    print_cmd_vector (make_cmd_vector_string (response, NULL, NULL));
  }
  return 0;
}


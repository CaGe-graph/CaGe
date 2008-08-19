
# include <stdlib.h>
# include <string.h>

# include "jni_patches.h"

# include <jni.h>

# include "jbytes.h"

# include "dstring.h"

# include "malloc.h"


struct cmd_vector
{
  int  length;
  char ***cmd;
  char *rundir;
  char *path;
};


/*
   strxpbrk does the same as strpbrk(3), only considers the
   trailing null byte part of the "accept" string
*/
static char *strxpbrk (char *string, char *accept)
{
  char found [256], *c;
  memset (found, 0, sizeof (found));
  found [0] = (char) 1;
  for (c = accept; *c; ++c)
    found [*c] = (char) 1;
  for (c = string; ; ++c)
    if (found [*c]) return c;
}

void *
make_cmd_vector_string (char *string, char *rundir, char *path)
{
  int cmdi, argi, status;
  struct cmd_vector *cmdv;
  char *s, *cp, c;
  cmdv = malloc (sizeof (*cmdv));
  cmdv->length = 0;
  cmdv->cmd = NULL;
  cmdv->rundir = rundir;
  cmdv->path = path;
  cmdi = status = 0;
  for (s = string; (cp = strxpbrk (s, " |\n")) != NULL; s = cp+1)
  {
    if (*cp == '\n') *cp = '\0';
    switch (status)
    {
      case 0:
	if (cp == s) {
	    if (*cp == '\0') { cp = NULL; break; }
	    continue;
	}
	cmdv->cmd = realloc (cmdv->cmd, ++cmdv->length * sizeof (char **));
	argi = -1;
	cmdv->cmd [cmdi] = NULL;
	status = 1;
      case 1:
	c = *cp;
	*cp = '\0';
	if (cp > s) {
	    ++argi;
	    cmdv->cmd [cmdi] = realloc (cmdv->cmd [cmdi],
	     (argi + 2) * sizeof (char *));
	    cmdv->cmd [cmdi][argi] = strdup (s);
	}
        if (c == '\0' || c == '|') {
	    cmdv->cmd [cmdi][argi+1] = NULL;
	    ++cmdi;
	    status = 0;
	    if (c == '\0') { cp = NULL; break; }
	    continue;
	}
	break;
    }
    if (cp == NULL) break;
  }
  if (cmdv->length == 0) {
      free (cmdv);
      cmdv = NULL;
  }
  return cmdv;
}

void *
make_cmd_vector (JNIEnv *env, jobjectArray cmds,
 jbyteArray rundir, jbyteArray path)
{
  int cmdlen, i, j;
  struct cmd_vector *cmdv;
  char **argv;
  jobject command, argument;

  if (cmds == NULL) return NULL;
  cmdv = malloc (sizeof (*cmdv));
  cmdv->rundir = jbytes2string (env, rundir);
  cmdv->path = jbytes2string (env, path);
  cmdv->length = (*env)->GetArrayLength (env, cmds);
  cmdv->cmd = malloc (cmdv->length * sizeof (char **));
  for (i = 0; i < cmdv->length; ++i)
  {
    command = (*env)->GetObjectArrayElement (env, cmds, (jsize) i);
    cmdlen = (*env)->GetArrayLength (env, command);
    argv = malloc ((cmdlen + 1) * sizeof (char *));
    for (j = 0; j < cmdlen; ++j)
    {
      argument = (*env)->GetObjectArrayElement (env, command, (jsize) j);
      argv [j] = jbytes2string (env, argument);
    }
    argv [j] = 0;
    cmdv->cmd [i] = argv;
  }
  return cmdv;
}

void
free_cmd_vector (void *cmd_vector)
{
  int i;
  char **argv;
  struct cmd_vector *cmdv;
  if ((cmdv = cmd_vector) == NULL) return;
  for (i = cmdv->length - 1; i >= 0; --i)
  {
    for (argv = cmdv->cmd [i]; *argv; ++argv)
    {
      free (*argv);
    }
    free (cmdv->cmd [i]);
  }
  free (cmdv->cmd);
  free (cmdv->rundir);
  free (cmdv->path);
  free (cmdv);
}


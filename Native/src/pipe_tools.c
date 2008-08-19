
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <sys/wait.h>
# include <signal.h>
# include <string.h>

# include "malloc.h"

# include "cmd_vector.h"

# include "error_exit.h"


extern int debug_pipe;


void
redir (int fd1, int fd2)
{
  if (fd1 != fd2) {
      dup2(fd1, fd2);
      close (fd1);
  }
}


void
establish_pipe (void *cmd_vector,
 int i_fd, int o_fd,
 char *infilename, char *outfilename, int out_append, char *errfilename,
 pid_t *pipe_pid, int *writer_fd, int *reader_fd)
{
  struct cmd_vector *cmdv;
  int pipefd [2];
  int pipe_i_fd, pipe_o_fd, fd;
  int i, j;
  int use_rundir, use_path;
  pid_t pid;

  cmdv = cmd_vector;
# define  check_use(string,var)  { (var)=0; if((string)!=NULL) if(*(string)!='\0') (var)=1; }
  check_use (cmdv->rundir, use_rundir);
  check_use (cmdv->path, use_path);

  *writer_fd = -1;
  *reader_fd = -1;
  if (infilename != NULL) {
      if ((fd = open (infilename, O_RDONLY)) < 0) {
          error_exit ("can't open input file", "io/IOException");
	  return;
      }
      if (debug_pipe) fprintf (stderr, "Opening file '%s' for reading as %d\n", infilename, fd);
      pipe_i_fd = fd;
  } else if (i_fd < 0) {
      if (pipe (pipefd)) {
          error_exit ("can't create pipe", "io/IOException");
	  return;
      }
      if (debug_pipe)
	  fprintf (stderr, "Process %d creating pipe: %d -> %d\n", getpid (), pipefd [1], pipefd [0]);
      pipe_i_fd = pipefd [0];
      *writer_fd = pipefd [1];
  } else {
      pipe_i_fd = i_fd;
  }

  if (outfilename != NULL) {
      int flags = O_CREAT | O_WRONLY | (out_append ? O_APPEND : O_TRUNC);
      if ((fd = open (outfilename, flags, 00644)) < 0) {
          error_exit ("can't open output file", "io/IOException");
	  return;
      }
      if (debug_pipe) fprintf (stderr, "Opening file '%s' for writing as %d\n", outfilename, fd);
      pipe_o_fd = fd;
  } else if (o_fd < 0) {
      if (pipe (pipefd)) {
          error_exit ("can't create pipe", "io/IOException");
	  return;
      }
      if (debug_pipe)
	  fprintf (stderr, "Process %d creating pipe: %d -> %d\n", getpid (), pipefd [1], pipefd [0]);
      pipe_o_fd = pipefd [1];
      *reader_fd = pipefd [0];
  } else {
      pipe_o_fd = o_fd;
  }

  if (errfilename != NULL) {
      if ((fd = creat (errfilename, 00644)) < 0) {
          error_exit ("can't open error file", "io/IOException");
	  return;
      }
      if (debug_pipe) fprintf (stderr, "Opening file '%s' for stderr as %d\n", errfilename, fd);
  } else {
      fd = -1;
  }

  if (debug_pipe) {
      fprintf (stderr, "Creating pipeline under process %d, reading from %d, writing into %d\n", getpid (), pipe_i_fd, pipe_o_fd);
  }
  switch (pid = fork ())
  {
    case -1:
      error_exit ("fork failure", "io/IOException");
      return;
    case 0:
      if (i_fd < 0) {
	  close (*writer_fd);
	  if (debug_pipe)
	     fprintf (stderr, "Process %d closing %d\n", getpid (), *writer_fd);
      }
      if (o_fd < 0) {
	  close (*reader_fd);
	  if (debug_pipe)
	     fprintf (stderr, "Process %d closing %d\n", getpid (), *reader_fd);
      }
      if (fd >= 0) {
	  redir (fd, STDERR_FILENO);
	  if (debug_pipe && fd != STDERR_FILENO) fprintf (stderr, "Process %d reopening %d from %d\n", getpid (), STDERR_FILENO, fd);
      }
      break;
    default:
      *pipe_pid = pid;
      if (debug_pipe) fprintf (stderr, "Process %d forking into process %d\n", getpid (), pid);
      if (i_fd < 0 || infilename != NULL) {
          close (pipe_i_fd);
	  if (debug_pipe) fprintf (stderr, "Process %d closing %d\n", getpid (), pipe_i_fd);
      }
      if (o_fd < 0 || outfilename != NULL) {
          close (pipe_o_fd);
	  if (debug_pipe) fprintf (stderr, "Process %d closing %d\n", getpid (), pipe_o_fd);
      }
      if (fd >= 0) {
          close (fd);
	  if (debug_pipe) fprintf (stderr, "Process %d closing %d\n", getpid (), fd);
      }
      return;
  }

  if (use_rundir) {
      if (chdir (cmdv->rundir)) {
          error_exit_subproc ("can't change into rundir", "io/IOException");
	  return;
      }
  }
  if (use_path) {
      char *pathenv;
      pathenv = malloc (strlen (cmdv->path) + 6);
      sprintf (pathenv, "PATH=%s", cmdv->path);
      putenv (pathenv);
  }

  for (i = cmdv->length - 1; i >= 0; i -= 1)
  {
    if (i > 0) {
        if (pipe (pipefd)) {
	    error_exit_subproc ("can't create pipe", "io/IOException");
	    return;
	}
	if (debug_pipe)
	    fprintf (stderr, "Process %d creating pipe: %d -> %d\n", getpid (), pipefd [1], pipefd [0]);
	switch (pid = fork ())
	{
	  case -1:
	    error_exit_subproc ("fork failure", "io/IOException");
	    break;
	  case 0:
	    pipe_o_fd = pipefd [1];
	    close (pipefd [0]);
	    if (debug_pipe) fprintf (stderr, "Process %d closing %d\n", getpid (), pipefd [0]);
	    continue;
	  default:
	    if (debug_pipe) fprintf (stderr, "Process %d forking into process %d\n", getpid (), pid);
	    pipe_i_fd = pipefd [0];
	    close (pipefd [1]);
	    if (debug_pipe) fprintf (stderr, "Process %d closing %d\n", getpid (), pipefd [1]);
	    break;
	}
    }
    if (debug_pipe) fprintf (stderr, "Process %d starting '%s' (in = %d, out = %d)\n", getpid (), cmdv->cmd [i][0], pipe_i_fd, pipe_o_fd);
    redir (pipe_i_fd, STDIN_FILENO);
    redir (pipe_o_fd, STDOUT_FILENO);
    if (use_path) {
        if (strpbrk (cmdv->cmd [i][0], "/\\")) {
	    error_exit_subproc (
	     "pipe restricted - won't run commands in other directories",
	     "io/IOException");
	}
    }
    execvp (cmdv->cmd [i][0], cmdv->cmd [i]);
    error_exit_subproc (
     use_path ?
      "exec failure - possibly caused by restricted search path" :
      "exec failure - probably caused by inexistent command",
     "io/IOException");
    return;
  }
}


/*
   Waits for child process "pid" to exit and returns its exit value.
   If the process has ended due to a signal, (128+signal) is returned.
   If the process ended in another non-normal way, 256 is returned.
   -1 is returned if waiting for the process did not succeed (it doesn't
   exist, or is not a child process).
*/
int
wait_for_exit (pid_t pipe_pid)
{
  int status;
  pid_t pid;
  if (debug_pipe) fprintf (stderr, "waiting for process %d ...\n", pipe_pid);
  if ((pid = waitpid (pipe_pid, &status, 0)) <= 0) return -1;
  if (pid != pipe_pid) return -1;
  if (debug_pipe) fprintf (stderr, "finished.\n");
  return
   WIFEXITED(status) ? WEXITSTATUS(status) :
   WIFSIGNALED(status) ? 128 + WTERMSIG(status) :
   256;
}


/*
   Same as "wait_for_exit", only doesn't wait.
   Return values are the same as in "wait_for_exit", only
   -2 is returned if "pid" is a "waitable" process that is still running.
*/
int
check_for_exit (pid_t pipe_pid)
{
  int status;
  pid_t pid;
  if ((pid = waitpid (pipe_pid, &status, WNOHANG)) < 0) return -1;
  if (pid == 0) return -2;
  if (pid != pipe_pid) return -1;
  return
   WIFEXITED(status) ? WEXITSTATUS(status) :
   WIFSIGNALED(status) ? 128 + WTERMSIG(status) :
   256;
}


void
collect_any_children ()
{
  int status;
  pid_t pid;
  do {
    pid = waitpid (-1, &status, WNOHANG);
  }
  while (pid > 0 && (WIFEXITED (status) || WIFSIGNALED (status)));
}


/*

void
establish_pipe_1 (void *cmd_vector, pid_t *pipe_pid)
{
  struct cmd_vector *cmdv;
  int pipefd [2];
  int pipeinfd, pipeoutfd, cmdinfd, cmdoutfd;
  int i, j;
  pid_t pid;

  if (debug_pipe)
      fprintf (stderr, "Creating pipe under process %d\n", getpid ());

  cmdv = cmd_vector;
  pipeinfd = STDIN_FILENO;
  pipeoutfd = STDOUT_FILENO;
  for (i = 0; i < cmdv->length; i = j)
  {
    if (i == 0) {
        cmdinfd = pipeinfd;
    } else {
        cmdinfd = pipefd [0];
    }
    j = i+1;
    if (j < cmdv->length) {
        if (pipe (pipefd)) {
	    error_exit ("can't create pipe", "io/IOException");
	    return;
	}
	cmdoutfd = pipefd [1];
    } else {
        cmdoutfd = pipeoutfd;
    }
    switch (pid = fork ())
    {
      case -1:
        error_exit ("fork failure", "io/IOException");
	break;
      case 0:
	redir (cmdinfd, STDIN_FILENO);
	redir (cmdoutfd, STDOUT_FILENO);
	if (debug_pipe) fprintf (stderr, "Starting '%s' as pipe stage %d under process %d (in = %d, out = %d)\n", cmdv->cmd [i][0], i, getpid (), cmdinfd, cmdoutfd);
        execvp (cmdv->cmd [i][0], cmdv->cmd [i]);
	error_exit ("exec failure", "io/IOException");
	break;
      default:
	*pipe_pid = pid;
	break;
    }
    if (i > 0) close (cmdinfd);
    if (j < cmdv->length) close (cmdoutfd);
  }
}

*/


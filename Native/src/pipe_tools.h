
/*
   Functions that support creating pipes from Java.
*/

# include <sys/types.h>


extern void establish_pipe (void *cmd_vector,
 int infd, int outfd,
 char *infilename, char *outfilename, int out_append, char *errfilename,
 pid_t *pipe_pid, int *writer_fd, int *reader_fd);
/*
   Starts a pipe, using a command vector as obtained by make_cmd_vector,
   that reads from file descriptor "infd" and writes to descriptor "outfd".
   If "infilename" and "outfilename" are non-NULL, attempt to read and/or
   write to files of that name instead.
   "outfilename" will be overwritten if "out_append" is zero.
   If "infd" is negative and "infilename" is NULL, a pipe is created,
   the pipe reads from the reading end of the pipe and returns the writing
   end of the pipe in *writer_fd.
   Likewise, if "outfd" is negative and "outfilename" is NULL,
   the pipe writes to the writing end of a new pipe and returns the reading
   end of that pipe in *reader_fd.
   If no reading and/or writing pipe is established,
   *reader_fd and/or *writer_fd are set to -1 respectively.
   A process ID is returned in *pipe_pid.
   The cmd_vector can be released by free_cmd_vector after this.
*/

extern int wait_for_exit (pid_t pipe_pid);
/*
   Wait for a pipe previously started by "establish_pipe" to finish.
   Returns the exit status of the pipe (i.e. of its last process).
   The status is lost if wait_for_exit or collect_any_children has
   been called since the process finished.  In that case, -1 is returned.
   If the process has been stopped due a signal, 128+signal is returned.
   If the process has not exited, but otherwise stopped, 256 is returned.
*/

extern int check_for_exit (pid_t pipe_pid);
/*
   See if a pipe previously started by "establish_pipe" has finished.
   Returns the exit status of the pipe (i.e. of its last process).
   The status is lost if check_for_exit or collect_any_children has
   been called since the process finished.
   See wait_for_exit for an interpretation of exit values.
*/

extern void collect_any_children ();
/*
   Calls wait(2) to clean up resources left by any dead child processes.
   It will not be possible to query those processes' exit values
   via wait_for_exit or check_for_exit.
*/


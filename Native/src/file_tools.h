
/*
   Some basic file operations.
*/

# include <stdio.h>


extern int next_byte (FILE *file);
/*
   returns the next byte from "file", if there is one available
   without blocking, -1 if there isn't, and -2 if we can't find out
   (if the call to fcntl used to set non-blocking input failed).
   If a non-negative value is returned, the byte has been ungetc'ed
   so it is still left to be read later (or, in fact, returned by
   another call to next_byte).
*/


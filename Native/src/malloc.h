
# ifndef MALLOC_INCLUDED
# define MALLOC_INCLUDED


# include <stdlib.h>

extern void *xmalloc (size_t size);
extern void *xrealloc (void *ptr, size_t size);
extern void xfree (void *ptr);
extern char *xstrdup (const char *string);

# define  malloc(size)        xmalloc (size)
# define  realloc(ptr, size)  xrealloc (ptr, size)
# define  free(ptr)           xfree (ptr)
# define  strdup(string)      xstrdup (string)


# endif /* MALLOC_INCLUDED */


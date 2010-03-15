#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#define outside USHRT_MAX

typedef unsigned short VERTEX;
typedef unsigned short CODETYPE;

typedef struct e/* The data type used for edges */
{ 
	VERTEX start;         /* vertex where the edge starts */
	VERTEX end;           /* vertex where the edge ends */ 
	struct e *prev;    /* previous edge in clockwise direction */
	struct e *next;    /* next edge in clockwise direction */
	struct e *invers;  /* the edge that is inverse to this one */
        unsigned char pentagon_right; /* Is there a pentagon right of this edge */
	int dummy;     /*  int for temporary use */
        unsigned short mark__;    
                       /* used for marking. May only be accessed via the MARK_E macros */
} EDGE;

/* The cap we are dealing with as a global variable: */

EDGE **firstedge=NULL;   /* pointer to arbitrary edge out of vertex i. 
			    This pointer may change during the run, so 
			    all one can rely on is that at any point it 
			    is "some" edge out of i  */
int nv=0;             /* number of vertices */
int l=0, m=0; /* The boundary encoding of the cap is (23)^l(32)^m. 
                 We always have l >= m. */



#define new_path(a,b) (new__path[((a)<<2)+((b)-1)])
#define new_path_last(a,b) (new__path_last[((a)<<2)+((b)-1)])
/* entry (a,b) has the first/last edge of a path of length b starting at vertex a */


#define MARK_LIMIT (USHRT_MAX-3)

static unsigned short markvalue_v = MARK_LIMIT;
static unsigned short *marks__v;
#define RESETMARKS_V {int mki; if ((++markvalue_v) > MARK_LIMIT) \
       { markvalue_v = 1; for (mki=0;mki<=maxlabel;++mki) marks__v[mki]=0;}}
#define UNMARK_V(x) (marks__v[x] = 0)
#define ISMARKED_V(x) (marks__v[x] == markvalue_v)
#define UNMARKED_V(x) (marks__v[x] != markvalue_v)
#define MARK_V(x) (marks__v[x] = markvalue_v)


/* Every time new edges are allocated, their mark__ values must be set to 0 or 
   RESETMARKS_E must be called after they have been added to the edge_array_start
   list */

static unsigned short markvalue_e = MARK_LIMIT;
#define MAX_EDGE_ARRAYS 10
EDGE *edge_array_start[MAX_EDGE_ARRAYS]; /* to remember the various arrays where edges 
					    were allocated. ALL EDGES THAT ARE ALLOCATED 
					    AND MUST BE MARKED MUST BE CONTAINED IN ONE 
					    OF THE ARRAYS LISTED HERE !!! */
int how_many_edges[MAX_EDGE_ARRAYS]; /* how many edges are after that vector */
#define RESETMARKS_E {if ((++markvalue_e) > MARK_LIMIT) \
  { markvalue_e = 1; init_edgemarks();}}
#define UNMARK_E(x) ((x)->mark__ = 0)
#define ISMARKED_E(x) ((x)->mark__ == markvalue_e)
#define UNMARKED_E(x) ((x)->mark__ != markvalue_e)
#define MARK_E(x) ((x)->mark__ = markvalue_e)


EDGE **new__path; /* Contains pre-initialised paths of length 1,2,3 or 4.
		    A path starting at new_path(n,.) has n as the first vertex
		    number. new_path(.,m) is the first edge to be glued to the start */
EDGE **new__path_last; /* new_path_last(.,.) is the last edge to be glued  */

EDGE *pentagons[30]; /* A list of directed edges with a pentagon on the right hand
			side. So at most 6*5=30 edges are possible. */
int pentagonedges=0; /* The number of edges in the list so far (5 times number of
			pentagons in the patch) */
int pentagoncounter=0;

int maxnv_cap=0;  /* The maximum number of vertices that can occur in the cap */
int maxnv=0;      /* The maximum number of vertices that can occur in a cap+tube */
int maxlabel=0;   /* The maximum label of a vertex that can occur in a cap+tube 
                     This value must be computed right at the beginning of the program
		     and may never be changed. It is e.g. used in the marking makros. */

int construct_ipr=0; /* shall only IPR caps be constructed ? */
int is_ipr=1;        /* Is the patch IPR ? The contents of this variable is only
			modified in case construct_ipr==1. It is set to 0 in
			add_n_gon and reset to 1 in the calling routine after
                        undoing the modification that led to being non-ipr */

int auts_statistic[7]={0}, auts_statistic_mirror[7]={0};
                     /* In these variables the number of caps with a given number
                        of AUTOMORPHISM PRESERVING automorphisms and with/without 
			a reflection among them is stored. This information is evaluated 
			to determine the number of caps with a given group. */


EDGE *isomorphism_tube; /* an edge returned by make_tube and used as a link to the tube that
			   is glued to the cap for isomorphism testing */


/* variables used in check_representation and get_representation; */
static EDGE **startedge_cgr=NULL;/* startedge[i] is the starting edge for 
				    exploring the vertex with the number i+1 */
static VERTEX *number_cgr=NULL;  /* The new numbers of the vertices, starting 
				    at 0 */ 




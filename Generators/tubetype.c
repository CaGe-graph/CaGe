/* OPTIONS:
   ipr : Only caps with isolated pentagons are generated
   log : A logfile is generated
   Ci, Civ : Only caps with a special symmetry are written to stdout 
             and counted
   split x y : The generation tree is "split" at level y. The x-th part
               is generated 
   tube x  : A tube of length x is glued to the generated caps
   verbose : More information to stderr 
   noout : Structures are only counted, not written to stdout.
*/
int graph_counter=0;
/*
   the macro 'my_endianness' is a char value of 'b' for big-endian
   and 'l' for little-endian machines
*/
static unsigned short word = (((unsigned short) 'b') << 8) | 'l';
# define  my_endianness  (* (char *) &word)
/* === fullerene_caps.h == start ====================================== */
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

int glue(), intersect(), unglue2(), intersect_v(), unglue(), ungluev();
int output_glue(),output_glue_zigzag(),output_unglue(),output_unglue_zigzag();
int finish_patch(),last_patch(),add_tail();
void reglue(), regluev();

/* === fullerene_caps.h == end ======================================== */
/* === caps.c == start ================================================ */

/**************************INIT_EDGEMARKS************************************/

void init_edgemarks()
{
int i;
EDGE *limit, *run;

for (i=0; (run=edge_array_start[i]) != 0; i++)
  { 
    limit=run+how_many_edges[i];
    while (run!=limit) { run->mark__ = 0; run++; }
  }


}

/*************************WRITE_PLANAR_CODE_LABEL*****************************/

void write_planar_code_label(FILE *fil, EDGE *start)

/* relabels the graph starting at start and writes the resulting code to fil .
   A header is written only once, so in case this routine is called for
   different files, a header is written only into the first one.
   The routine assumes a maximum average valency of 3. To adapt it for richer
   graphs, the amount of memory allocated in case of the first call must be changed.
*/

{

static int first_call=1;
EDGE *temp, *remember, *checkstart;  
static unsigned short next_number, actual_number, vertex;
static unsigned short *code, *runcode, *runcode2;

if (first_call) { fprintf(fil,">>planar_code %ce<<", my_endianness); first_call=0; 
                  code=(unsigned short *)malloc(4*maxlabel*sizeof(unsigned short));
		  if (code==NULL) { fprintf(stderr,"Cannot allocate memory for the code\n");
		                    exit(0); }
                }

runcode=code;

RESETMARKS_V;

if (start->end==outside) 
  { checkstart=start;
    start=start->next;
    while ((start->end==outside) && (start!=checkstart)) start=start->next;
    if (start==checkstart) { if (putc(1,fil)==EOF) 
                                 { fprintf(stderr,"Cannot write to the file...\n");
				   exit(0); } 
                             if (putc(0,fil)==EOF) 
                                 { fprintf(stderr,"Cannot write to the file...\n");
				   exit(0); }
			     return; }
  }


vertex=start->start;
number_cgr[vertex] = 1; MARK_V(vertex);
startedge_cgr[1]=start;
vertex=start->end;
number_cgr[vertex] = 2; MARK_V(vertex);
startedge_cgr[2]=start->invers;
actual_number = 1;
next_number=3;

    while (actual_number<next_number)
    {   temp = remember= startedge_cgr[actual_number];
	
        do
	  {
	    vertex = temp->end;
	    if (vertex != outside) /* number of outside is outside */
	      { if (UNMARKED_V(vertex))
		{ startedge_cgr[next_number] = temp->invers->next;
		number_cgr[vertex] = next_number;
		MARK_V(vertex);
		vertex=next_number;
		next_number++;
		}
	      else vertex = number_cgr[vertex];
	      (*runcode)=vertex;
	      runcode++;
	      }
	    temp=temp->next;
	  } while (temp != remember);

	/* Next vertex to explore: */
	(*runcode)=0;
	runcode++;
        actual_number++; 
    }

actual_number--;

 if (actual_number < UCHAR_MAX) /* still fits into unsigned char */
   {
     if (putc(actual_number,fil)==EOF) { fprintf(stderr,"Cannot write to the file...\n");
                                         exit(0); }
     for (runcode2=code; runcode2<runcode; runcode2++)  
       if (putc(*runcode2,fil)==EOF) { fprintf(stderr,"Cannot write to the file...\n");
                                       exit(0); }
   }
 else 
   { if (putc(0,fil)==EOF) { fprintf(stderr,"Cannot write to the file...\n");
                             exit(0); }
   if (fwrite(&actual_number,sizeof(unsigned short),1,fil)!=1)
     { fprintf(stderr,"Cannot write to the file...\n");
       exit(0); }
   if (fwrite(code,sizeof(unsigned short),runcode-code,fil)!=runcode-code)
     { fprintf(stderr,"Cannot write to the file...\n");
       exit(0); }

 }
}


/*************************WRITE_PLANAR_CODE_LABEL_fix*****************************/

void write_planar_code_label_fix(FILE *fil, int fix)

/* relabels the graph and writes the resulting code to fil .
   The first fix vertices are not relabelled -- they must have starts in the 
   firstedge vector !!

   A header is written only once, so in case this routine is called for
   different files, a header is written only into the first one.
   The routine assumes a maximum average valency of 3. To adapt it for richer
   graphs, the amount of memory allocated in case of the first call must be changed.
*/

{

static int first_call=1;
EDGE *temp, *remember, *checkstart, *start;  
static unsigned short next_number, actual_number, vertex, i;
static unsigned short *code, *runcode, *runcode2;

if (first_call) { fprintf(fil,">>planar_code %ce<<", my_endianness); first_call=0; 
                  code=(unsigned short *)malloc(4*maxlabel*sizeof(unsigned short));
		  if (code==NULL) { fprintf(stderr,"Cannot allocate memory for the code\n");
		                    exit(0); }
                }

runcode=code;

RESETMARKS_V;

for (i=1; i<=fix; i++) { MARK_V(i);
                         startedge_cgr[i]=firstedge[i];
			 number_cgr[i]=i;
                        }


start=firstedge[1];

if (fix==0)
  {
    vertex=start->start;
    number_cgr[vertex] = 1; MARK_V(vertex);
    startedge_cgr[1]=start;
  }

if (fix<2)
  {
    if (start->end==outside) 
      { checkstart=start;
      start=start->next;
      while ((start->end==outside) && (start!=checkstart)) start=start->next;
      if (start==checkstart) { if (putc(1,fil)==EOF) 
	{ fprintf(stderr,"Cannot write to the file...\n");
	exit(0); } 
      if (putc(0,fil)==EOF) 
	{ fprintf(stderr,"Cannot write to the file...\n");
	exit(0); }
      return; }
      }

    vertex=start->end;
    number_cgr[vertex] = 2; MARK_V(vertex);
    startedge_cgr[2]=start->invers;
  }

actual_number = 1;
if (fix>=2) next_number=fix+1; else next_number=3;

    while (actual_number<next_number)
    {   temp = remember= startedge_cgr[actual_number];
	
        do
	  {
	    vertex = temp->end;
	    if (vertex != outside) /* number of outside is outside */
	      { if (UNMARKED_V(vertex))
		{ startedge_cgr[next_number] = temp->invers->next;
		number_cgr[vertex] = next_number;
		MARK_V(vertex);
		vertex=next_number;
		next_number++;
		}
	      else vertex = number_cgr[vertex];
	      (*runcode)=vertex;
	      runcode++;
	      }
	    temp=temp->next;
	  } while (temp != remember);

	/* Next vertex to explore: */
	(*runcode)=0;
	runcode++;
        actual_number++; 
    }

actual_number--;

 if (actual_number < UCHAR_MAX) /* still fits into unsigned char */
   {
     if (putc(actual_number,fil)==EOF) { fprintf(stderr,"Cannot write to the file...\n");
                                         exit(0); }
     for (runcode2=code; runcode2<runcode; runcode2++)  
       if (putc(*runcode2,fil)==EOF) { fprintf(stderr,"Cannot write to the file...\n");
                                       exit(0); }
   }
 else 
   { if (putc(0,fil)==EOF) { fprintf(stderr,"Cannot write to the file...\n");
                             exit(0); }
   if (fwrite(&actual_number,sizeof(unsigned short),1,fil)!=1)
     { fprintf(stderr,"Cannot write to the file...\n");
       exit(0); }
   if (fwrite(code,sizeof(unsigned short),runcode-code,fil)!=runcode-code)
     { fprintf(stderr,"Cannot write to the file...\n");
       exit(0); }

 }
}




/***************************INIT_MARKS****************************************/

void init_marks()
{
if (maxlabel==0) { fprintf(stderr,"maxlabel not yet set (still 0)... error\n");
                   exit(0);
                  }
if ((marks__v=(unsigned short *)malloc((maxlabel+1)*sizeof(unsigned short)))==NULL)
  { fprintf(stderr,"Cannot allocate space for the marks !\n");
    exit(0); }

if ((startedge_cgr=(EDGE **)malloc((maxlabel+1)*sizeof(EDGE *)))==NULL)
  { fprintf(stderr,"Cannot allocate memory for startedges.\n"); exit(0); }
if ((number_cgr=(VERTEX *)malloc((maxlabel+1)*sizeof(VERTEX)))==NULL)
   { fprintf(stderr,"Cannot allocate memory for numbers.\n"); exit(0); }
}




/******************************CHECK_REPRESENTATION******************************/

int check_representation(EDGE *givenedge, VERTEX representation[], int clockwise)

/* Tests whether starting from a given edge and constructing the representation 
   in clockwise or counterclockwise direction, an automorphism or even a 
   better representation can be found. Returns 0 for failure (this start 
   is worse (larger representation)), 1 for an automorphism and 2 for a 
   better (smaller) representation. 

   If clockwise is 0, counterclockwise orientation is tested, otherwise
   clockwise orientation.

   representation[] must be like the one computed in get_representation().

   givenedge must lead to the outside and givenedge->next and 
   givenedge->prev must lead to internal vertices !!!

*/

{
static EDGE *temp;  
static int next_number, actual_number, vertex;

RESETMARKS_V;

if (clockwise)
  {
    temp = givenedge->next;
    vertex=temp->start;
    number_cgr[vertex] = 0; MARK_V(vertex);
    vertex=temp->end;
    number_cgr[vertex] = 1; MARK_V(vertex);
    startedge_cgr[1]=temp->invers->next;
    temp=temp->next;
    vertex=temp->end;
    number_cgr[vertex] = 2; MARK_V(vertex);
    startedge_cgr[2]=temp->invers->next;
    actual_number = 1;
    next_number=3;

    while (actual_number<next_number)
      {   temp = startedge_cgr[actual_number]; 
      /* Check the next two edges around temp->origin. */
      vertex = temp->end;
      if (vertex != outside) /* number of outside is outside */
	{ if (UNMARKED_V(vertex))
	  { startedge_cgr[next_number] = temp->invers->next;
	  number_cgr[vertex] = next_number;
	  MARK_V(vertex);
	  vertex=next_number;
	  next_number++;
	  }
	else vertex = number_cgr[vertex];
	}
      if (vertex > (*representation)) return(0);
      if (vertex < (*representation)) return(2);
      representation++; 
      /* next vertex: */
      temp=temp->next;
      vertex = temp->end;
      if (vertex != outside)
	{ if (UNMARKED_V(vertex))
	  { startedge_cgr[next_number] = temp->invers->next;
	  number_cgr[vertex] = next_number;
	  MARK_V(vertex);
	  vertex=next_number;
	  next_number++;
	  }
	else vertex = number_cgr[vertex];
	}
      if (vertex > (*representation)) return(0);
      if (vertex < (*representation)) return(2);
      representation++; 
      /* Next vertex to explore: */
      actual_number++; 
      }
  } /* end of clockwise orientation */
 else /* that is: counterclockwise orientation -- no comments -- the same as above
	 just in the other direction */
   {
     temp = givenedge->prev;
     vertex=temp->start;
     number_cgr[vertex] = 0; MARK_V(vertex);
     vertex=temp->end;
     number_cgr[vertex] = 1; MARK_V(vertex);
     startedge_cgr[1]=temp->invers->prev;
     temp=temp->prev;
     vertex=temp->end;
     number_cgr[vertex] = 2; MARK_V(vertex);
     startedge_cgr[2]=temp->invers->prev;
     actual_number = 1;
     next_number=3;

     while (actual_number<next_number)
       {   temp = startedge_cgr[actual_number];
       vertex = temp->end;
       if (vertex != outside) 
	 { if (UNMARKED_V(vertex))
	   { startedge_cgr[next_number] = temp->invers->prev;
	   number_cgr[vertex] = next_number;
	   MARK_V(vertex);
	   vertex=next_number;
	   next_number++;
	   }
	 else vertex = number_cgr[vertex];
	 }
       if (vertex > (*representation)) return(0);
       if (vertex < (*representation)) return(2);
       representation++; 
       /* next vertex: */
       temp=temp->prev;
       vertex = temp->end;
       if (vertex != outside)
	 { if (UNMARKED_V(vertex))
	   { startedge_cgr[next_number] = temp->invers->prev;
	   number_cgr[vertex] = next_number;
	   MARK_V(vertex);
	   vertex=next_number;
	   next_number++;
	   }
	 else vertex = number_cgr[vertex];
	 }
       if (vertex > (*representation)) return(0);
       if (vertex < (*representation)) return(2);
       representation++; 
       actual_number++; 
       }
   } /* end of counterclockwise */

/* If no difference occured, then the representation is the same -- it also
   can't be smaller */

return(1);

}

/******************************GET_REPRESENTATION************************************/

void get_representation(EDGE *givenedge, VERTEX representation[])

/* Computes a representation in clockwise direction from givenedge that
   is an invariant of the edge and the orientation clockwise. 

   representation[] must have at least twice as many entries as the maximum
   cap to which this routine is applied.

   givenedge must lead to the outside and givenedge->next and 
   givenedge->prev must lead to an internal vertex !!!

*/

{
EDGE *temp;  
static int next_number, actual_number, vertex;

RESETMARKS_V;

temp = givenedge->next;
vertex=temp->start;
number_cgr[vertex] = 0; MARK_V(vertex);
vertex=temp->end;
number_cgr[vertex] = 1; MARK_V(vertex);
startedge_cgr[1]=temp->invers->next;
temp=temp->next;
vertex=temp->end;
number_cgr[vertex] = 2; MARK_V(vertex);
startedge_cgr[2]=temp->invers->next;
actual_number = 1;
next_number=3;

    while (actual_number<next_number)
    {   temp = startedge_cgr[actual_number];
	/* Check the next two edges around temp->origin. */
        vertex = temp->end;
	if (vertex != outside) /* number of outside is outside */
	  { if (UNMARKED_V(vertex))
	    { startedge_cgr[next_number] = temp->invers->next;
	    number_cgr[vertex] = next_number;
	    MARK_V(vertex);
	    vertex=next_number;
	    next_number++;
	    }
	  else vertex = number_cgr[vertex];
	  }
	(*representation)=vertex;
	representation++; 
	/* next vertex: */
	temp=temp->next;
        vertex = temp->end;
	if (vertex != outside)
	  { if (UNMARKED_V(vertex))
	    { startedge_cgr[next_number] = temp->invers->next;
	    number_cgr[vertex] = next_number;
	    MARK_V(vertex);
	    vertex=next_number;
	    next_number++;
	    }
	  else vertex = number_cgr[vertex];
	  }
	(*representation)=vertex;
	representation++; 
	/* since the graph is cubic, there is no need to put end signs,
	   because for every vertex 2 items are listed */
	/* Next vertex to explore: */
        actual_number++; 
    }

}


/************************ l_patch_is_canonical ***************************/
/* 
Parameters:
int *numstarts;  Number of possible starting points; i.e. canonical in clockwise
		   direction  
int *numstarts_mirror;  The same; anticlockwise direction 
EDGE *starts[];  Possible starting edges, i.e. edges pointing to "outside" where 
		   the start vertex is part of a (canonical in clockwise direction) 
		   pentagon 
EDGE *starts_mirror[];  analogously; anticlockwise 
*/ 

int l_patch_is_canonical(EDGE *mark, EDGE *starts[], int *numstarts, 
			 EDGE *starts_mirror[], int *numstarts_mirror)

/* Checks whether starting at mark and developing the code in clockwise direction
   the "best possible" code is obtained. Returns 1 if yes, 0 if not. 
   The code is compared against all *numstarts codes obtained from edges in starts 
   in clockwise direction and against all *numstarts_mirror codes obtained from
   edges in starts_mirror in counterclockwise direction. 

   As soon as an orientation inversing automorphism is found, the mirror images
   are not tested any more, since it is assumed that the mirror-starts are just
   mirror-images of the normal starts.

*/

{
  int i,test;
  int auts=1, mirror=0;
  static VERTEX *representation=NULL;

  graph_counter++;
if (representation==NULL)
  {  
    if ((representation=(VERTEX *)calloc(maxlabel*2,sizeof(VERTEX)))==NULL)
    { fprintf(stderr,"Cannot allocate memory for the representation.\n"); exit(0); }
  }

  get_representation(mark,representation);


  for (i=0;i<*numstarts; i++) 
    if (starts[i] != mark)
    { test=check_representation(starts[i],representation,1);
      if (test==2) return 0;
      if (test==1) auts++;
    }

  for (i=0;i<*numstarts_mirror; i++) 
    { test=check_representation(starts_mirror[i],representation,0);
      if (test==2) return 0;
      if (test==1) { mirror=1; break; }
    }

if (mirror) (auts_statistic_mirror[auts])++; else (auts_statistic[auts])++;

  return(1);
}

/****************************COMPUTE_SEQUENCE**********************************/

int compute_sequence(EDGE *mark, int sequence[], EDGE *starts[], int *lengthp)

/* Assumes that mark is the edge on the right vertex of a convex edge 
   leading to "outside" and computes the cyclic sequence of numbers
   of 3-vertices between two convex edges starting at mark. 
   The sequence is written to (sequence+1)[]. In sequence[0], the number
   of convex edges until the next canonical possibility for a mark is stored
   (including the actual mark) in case the mark is at a canonical position --
   otherwise the value is undefined.
   In starts[i] the edge pointing to "outside" and corresponding to sequence[i]
   is stored.
   In *lengthp the length of the sequence is returned.
   The return value is 1 in case the mark is on a canonical position (that is:
   the cyclic sequence starting there is lexicographically minimal), 0 else. 
   The sequence may not exceed 6 entries, because of the local variable "buffer".
*/

{

EDGE *run;
int i,j,length,counter,first;
int buffer[13];



run=mark;
length=0;
do { length++;
     starts[length]=run;
     run=run->next->invers->next;
     counter=0;
     while (run->end != outside)
       { counter++; 
	 run=run->invers->prev->invers->next; }
     sequence[length]=counter;
   } while (run != mark);

sequence[0]=*lengthp=length;
for (i=1; i<=length; i++) buffer[i]=buffer[i+length]=sequence[i];

for (i=2, first=buffer[1]; i<=length; i++)
  { if (buffer[i]<first) return 0;
    if (buffer[i]==first)
      { for (j=1;(j<length) && (buffer[i+j]==buffer[1+j]);j++);
	if (j==length) { sequence[0]=i-1; return 1; } /* found symmetry */
	if (buffer[i+j]<buffer[1+j]) return 0;
      }
  }

return 1;
}

/****************************COMPUTE_SEQUENCE_PENTAGON**********************************/

int compute_sequence_pentagon(EDGE *mark, EDGE *starts[], int *numstarts,
			                  EDGE *starts_mirror[], int *numstarts_mirror)

/* Assumes that mark is the edge on a pentagon leading to "outside" of a complete
   zigzag sequence (that is: neither convex nor concave edges exist). The function
   computes the cyclic sequence of numbers of 3-vertices between two 
   "outside"-edges and returns 1, if this sequence is lexicographically minimal
   when starting at mark __AND__ going in clockwise direction and 0 otherwise --
   so 0 is also returned when you starting at mark, and going counterclockwise
   is minimal. 

   The prerequisites are not checked, so applying this function for other patches
   or with another edge as mark, will most likely lead to problems (e.g. an 
   infinite loop or a segmentation fault).

   In starts[], the pointers to the edges where a mark would be canonical
   are listed and in *numstarts their number is stored. In starts_mirror[]
   and *numstarts_mirror the same information for counterclockwise rotation
   is stored. 
   This information is only valid in case 1 is returned ! (So *numstarts_mirror=0 
   or *numstarts_mirror=*numstarts.)
   
*/


{

EDGE *run;
int i,j,length,counter,first;
int buffer[12];
EDGE *edges[7];

starts[0]=mark; *numstarts=1; *numstarts_mirror=0;
run=mark=mark->next;
length=0;
do { counter=0;
     edges[length]=run->prev; 
     do
       {
	 counter++; 
	 run=run->invers->next->invers->prev;
       }
     while (!run->pentagon_right);
     buffer[length]=counter; length++;
   } while (run != mark);

if (length==1) { starts_mirror[0]=starts[0]; *numstarts_mirror=1;
                 return 1; }

edges[length]=edges[0]; /* one overlap for the mirror image checking */

for (i=0; i<length; i++) buffer[i+length]=buffer[i];

for (i=1, first=buffer[0]; i<length; i++)
  { if (buffer[i]<first) return 0;
    if (buffer[i]==first)
      { for (j=1;(j<length) && (buffer[i+j]==buffer[j]);j++);
      if (j==length) /* automorphism of the boundary sequence */
	{ starts[*numstarts]=edges[i]; (*numstarts)++; }
      else
	if ((j<length) && (buffer[i+j]<buffer[j])) return 0;
      }
  }



for (i=2*length-1; i>=length; i--)
  { /* if (buffer[i]<first) return 0; cannot happen after the previous loop */
    if (buffer[i]==first)
      { for (j=1;(j<length) && (buffer[i-j]==buffer[j]);j++);
      if (j==length) /* automorphism of the boundary sequence */
	{ starts_mirror[*numstarts_mirror]=edges[i-length+1]; /* +1 since it is the other
								 side of the segment */
	  (*numstarts_mirror)++; }
      else
	if ((j<length) && (buffer[i-j]<buffer[j])) return 0;
      }
  }

return 1;
}


/********************************INIT_NEW_PATHS**********************************/

void init_new_paths(int maxnv_cap)

/* Initialises the new_path entries. Must be called before the construction starts.
   The parameter maxnv_cap must be the maximum number of vertices a cap might have */

{
int nv,length,i;
EDGE *edges;

edges=(EDGE *) malloc(3*(1+2+3+4)*maxnv_cap*sizeof(EDGE));
if (edges==NULL) { fprintf(stderr,"Can not get enough space for the cap edges \n");
		   exit(1); }
 { int marki;
   EDGE *markrun;
   
   for (marki=0; (marki<MAX_EDGE_ARRAYS) && (edge_array_start[marki]!=0); marki++);
   if (marki==MAX_EDGE_ARRAYS) 
        { fprintf(stderr,"Constant MAX_EDGE_ARRAYS too small (1) -- increase.\n");
	  exit(1); }
   edge_array_start[marki]=edges;
   how_many_edges[marki]=3*(1+2+3+4)*maxnv_cap;
   for (marki=3*(1+2+3+4)*maxnv_cap, markrun=edges; marki>0; marki--, markrun++) markrun->mark__=0;
 }

new__path=(EDGE **)malloc(sizeof(EDGE *)*(maxnv_cap+1)*4);
if (new__path==NULL) { fprintf(stderr,"Can not get enough space for the new__path entries \n");
		       exit(1); }
new__path_last=(EDGE **)malloc(sizeof(EDGE *)*(maxnv_cap+1)*4);
if (new__path_last==NULL) 
  { fprintf(stderr,"Can not get enough space for the new__path_last entries \n");
    exit(1); }


for (nv=1; nv<=maxnv_cap; nv++)
  for (length=1; length<=4; length++,edges++)
    { /* first the edge that is glued to the existing patch: */
	 new_path(nv,length)=edges;
	 edges->start=nv; 
	 edges->pentagon_right=0;
	 edges->prev=edges+2; edges->next=edges+1;
	 edges++; 
	 edges->start=nv; edges->end=outside; 
	 edges->pentagon_right=0;
	 edges->prev=edges-1; edges->next=edges+1;
	 edges++;
	 edges->start=nv; edges->pentagon_right=0;
	 edges->prev=edges-1; edges->next=edges-2;

      for (i=1; i<length; i++)
	{
	 edges++;
	 edges->start=nv+i; edges->end=nv+(i-1);
	 edges->pentagon_right=0;
	 edges->prev=edges+2; edges->next=edges+1;
	 edges->invers=edges-1; 
	 (edges-1)->invers=edges; (edges-1)->end=nv+i;
	 edges++; 
	 edges->start=nv+i; edges->end=outside; 
	 edges->pentagon_right=0;
	 edges->prev=edges-1; edges->next=edges+1;
	 edges++;
	 edges->start=nv+i;
	 edges->pentagon_right=0;
	 edges->prev=edges-1; edges->next=edges-2;
       }
	 new_path_last(nv,length)=edges;
       }
}


/**************************MAKE_N_GON******************************/

EDGE *make_n_gon(int n, int maxnv)
/* builds an n-gon and returns an edge leading to the outside. The firstedge
   vector is initialized to be this n-gon. In case the firstedge vector does
   not yet exist as a vector, it is allocated for maxnv+1 entries.*/
{

static EDGE *edges=NULL;
EDGE *run;
int i;

if (edges != NULL) {
                    for (i=0; edge_array_start[i] != edges; i++);
		    for ( ; (i<MAX_EDGE_ARRAYS-1) && (edge_array_start[i+1]!=0); i++)
		      { edge_array_start[i]=edge_array_start[i+1];
		        how_many_edges[i]=how_many_edges[i+1]; }
		    edge_array_start[i]=0;  
		    how_many_edges[i]=0;
                    free(edges);
                   }


edges=(EDGE *)malloc(sizeof(EDGE)*3*n);

 { int marki;
   EDGE *markrun;
   
   for (marki=0; (marki<MAX_EDGE_ARRAYS) && (edge_array_start[marki]!=0); marki++);
   if (marki==MAX_EDGE_ARRAYS) 
        { fprintf(stderr,"Constant MAX_EDGE_ARRAYS too small (2) -- increase.\n");
	  exit(1); }
   edge_array_start[marki]=edges;
   how_many_edges[marki]=3*n;
   for (marki=3*n, markrun=edges; marki>0; marki--, markrun++) markrun->mark__=0;
 }



if (edges==NULL) { fprintf(stderr,"Do not get space for edges in make_n_gon.\n");
		   exit(1); }


if (firstedge==NULL) { if (n>maxnv) { fprintf(stderr,"Cannot construct n-gon for ");
				      fprintf(stderr,"n>maxnv"); exit(1); }
                       firstedge=(EDGE **)malloc(sizeof(EDGE *)*(maxnv+1));
		       if (firstedge==NULL) 
			 { fprintf(stderr,"Do not get space for firstedge[].\n");
			   exit(1); }
		     }



run=edges;

if (n<3) 
  { fprintf(stderr,"The smallest possible face is a 3-gon.\n");
    exit(1); }

firstedge[1]=run;
run->start=1; run->end=n; firstedge[1]=run;
run->pentagon_right=0;
run->prev=run+2; run->next=run+1; run->invers=run+(3*n-1);
run++;
run->start=1; run->end=outside; firstedge[0]=run;
run->pentagon_right=0;
run->prev=run-1; run->next=run+1; run->invers=NULL;
run++;
run->start=1; run->end=2;
run->pentagon_right=0;
run->prev=run-1; run->next=run-2; run->invers=run+1;
run++;

for (i=2;i<n; i++)
  { firstedge[i]=run;
    run->start=i; run->end=i-1; firstedge[i]=run;
    run->pentagon_right=0;
    run->prev=run+2; run->next=run+1; run->invers=run-1;
    run++;
    run->start=i; run->end=outside; firstedge[i]=run;
    run->pentagon_right=0;
    run->prev=run-1; run->next=run+1; run->invers=NULL;
    run++;
    run->start=i; run->end=i+1;
    run->pentagon_right=0;
    run->prev=run-1; run->next=run-2; run->invers=run+1;
    run++;
  }

firstedge[n]=run;
run->start=n; run->end=n-1; firstedge[n]=run;
run->pentagon_right=0;
run->prev=run+2; run->next=run+1; run->invers=run-1;
run++;
run->start=n; run->end=outside; firstedge[n]=run;
run->pentagon_right=0;
run->prev=run-1; run->next=run+1; run->invers=NULL;
run++;
run->start=n; run->end=1;
run->pentagon_right=0;
run->prev=run-1; run->next=run-2; run->invers=edges;

nv=n;

if (n==5) { pentagoncounter=1; 
            for (pentagonedges=0; pentagonedges<5; pentagonedges++) 
	      { pentagons[pentagonedges]=run; 
	        run->pentagon_right=1;
	        run=run->invers->prev; }
           }
else pentagoncounter=pentagonedges=0;

return(edges+1);

}



/*******************************ADD_N_GON*************************************/

EDGE *add_n_gon(int n, EDGE *start)

/* Adds an n-gon starting at "start" in clockwise direction. Returns the last
   edge of this polygon pointing to the outside, if such an edge exists.
   Otherwise (that is: if no new vertices are inserted, but just a gap
   closed) the routine exits, since this is not expected to be necessary in this
   program.

   The firstedge, nv entries are updated.

   In case construct_ipr==1 and n==5, it is checked whether this penatgon neighbours
   another. In case it does not, the global variable is_ipr is set to 0;

*/


{

EDGE *run, *last;
int length, i;

if (construct_ipr && (n==5))
    {
      last=start->next;
      if (last->pentagon_right) is_ipr=0;
      last=last->invers->next;
      for (length=n-2; last->end!=outside; last=last->invers->next) 
	{ if (last->pentagon_right) is_ipr=0;
          length--;
	}
    }
else
   {
     last=start->next->invers->next;
     for (length=n-2; last->end!=outside; last=last->invers->next) length--;
   }
/* length counts, how many new vertices are needed. */
   

if (length<=0) 
  { if (length<0) 
      { fprintf(stderr,"Error -- %d-gon does not fit in %d-hole \n",n,n-length);
	exit(4); }
    if (length==0)
      { fprintf(stderr,"Didn't expect to add a polygon without a new vertex.\n");
	fprintf(stderr,"small changes necessary.\n");
	exit(5); }
  }


run=new_path(nv+1,length);
start->invers=run; run->invers=start;
start->end=nv+1; run->end=start->start;

for (i=1; i<=length; i++, run=run->prev->invers) firstedge[nv+i]=run;

run=new_path_last(nv+1,length);
last->invers=run; run->invers=last;
last->end=nv+length; run->end=last->start;

nv += length;

if (n==5) { for ( ; n ; n--, start=start->invers->prev) 
              {  pentagons[pentagonedges]=start; pentagonedges++;
		 start->pentagon_right=1; }
            pentagoncounter++; } 

return(run->prev);

}


/*******************************DELETE_N_GON*************************************/

EDGE *delete_n_gon(EDGE *start)

/* Deletes the n-gon of which start is the last edge to the outside. Returns the
   inverse of the last edge in counterclockwise direction that was deleted.
   This function relies on start being the edge returned when building this
   n-gon... */

{

EDGE *run, *last;
int length;

if (start->next->pentagon_right)
  { for (run=start->next, length=5; length ; length--, run=run->invers->prev)
      run->pentagon_right=0;
    pentagonedges -= 5;
    pentagoncounter--;
  }

start->next->invers->end=outside;

last=start->prev->invers->prev;

for (length=1; last->end==outside; last=last->prev->invers->prev) length++;
/* length is the number of vertices that are removed. last is the edge in prev
   direction of that to point to the outside */
   
last=last->next;
last->end=outside;

nv-=length;

return(last);

}


/*******************************ADD_N_GON_INVERSE********************************/

EDGE *add_n_gon_inverse(int n, EDGE *start)

/* Like add_n_gon, but in counterclockwise direction */

{

EDGE *run, *last;
int length, i;

last=start->prev->invers->prev;


for (length=n-2; last->end!=outside; last=last->invers->prev) length--;

if (length<=0) 
  { if (length<0) 
      { fprintf(stderr,"Error -- %d-gon does not fit in %d-hole \n",n,n-length);
	exit(42); }
    if (length==0)
      { fprintf(stderr,"Didn't expect to add a polygon without a new vertex.\n");
	fprintf(stderr,"small changes necessary.\n");
	exit(52); }
  }

/* The path must be glued in the opposite direction: */

run=new_path_last(nv+1,length);
start->invers=run; run->invers=start;
start->end=nv+length; run->end=start->start;

for (i=length; i>=1; i--, run=run->next->invers) firstedge[nv+i]=run;

run=new_path(nv+1,length);
last->invers=run; run->invers=last;
last->end=nv+1; run->end=last->start;

nv += length;

if (n==5) { for ( start=start->invers ; n ; n--, start=start->invers->prev) 
              {  pentagons[pentagonedges]=start; pentagonedges++;
		 start->pentagon_right=1; }
            pentagoncounter++; } 

return(run->next);

}


/*******************************DELETE_N_GON_INVERSE******************************/

EDGE *delete_n_gon_inverse(EDGE *start)

/* like delete_n_gon, but in the other direction */

{

EDGE *run, *last;
int length;

if (start->next->pentagon_right)
  { for (run=start->next, length=5; length ; length--, run=run->invers->prev)
      run->pentagon_right=0;
    pentagonedges -= 5;
    pentagoncounter--;
  }

start->prev->invers->end=outside;

last=start->next->invers->next;

for (length=1; last->end==outside; last=last->next->invers->next) length++;
   
last=last->prev;
last->end=outside;

nv-=length;

return(last);

}

/**********************************WRITEMAP**************************************/

void writemap()
{

int i,j;
EDGE *run;

fprintf(stderr,"\n\n");
for (i=1; i<=nv; i++)
  { run=firstedge[i];
    fprintf(stderr,"%d:",i);
    for (j=0; j<3; j++, run=run->next)
      { if (run->end != outside) fprintf(stderr," %d",run->end);
	else fprintf(stderr," a");
      }
    fprintf(stderr,"\n");
  }
fprintf(stderr,"\n");

 if (pentagoncounter) { fprintf(stderr,"%d Pentagons right of:\n",pentagoncounter);
                        for (i=0; i<pentagonedges; i++)
			  fprintf(stderr,"%d->%d  ",pentagons[i]->start,pentagons[i]->end);
                      }
fprintf(stderr,"\n\n");
}


/****************************WRITE_TUBE********************************/

/* writes a tube, but relies A LOT on the special method used to build it,
   so it only works for tube bodies built by make_tube */

void write_tube(EDGE *ref_edge, int vertices)
{
fprintf(stderr,"\n\n");
for ( ; vertices; vertices--, ref_edge+=3)
  { fprintf(stderr,"%d:",ref_edge->start);
    if (ref_edge->end==outside) fprintf(stderr," A"); 
    else fprintf(stderr," %d",ref_edge->end);
    if (ref_edge->next->end==outside) fprintf(stderr," A"); 
    else fprintf(stderr," %d",ref_edge->next->end);
    if (ref_edge->prev->end==outside) fprintf(stderr," A"); 
    else fprintf(stderr," %d",ref_edge->prev->end);
    fprintf(stderr,"\n");
  }
}



/*****************************MAKE_PATH********************************/

EDGE *makepath(EDGE edgelist[])

/* combines the next 3*2*(l+m) preinitialized edges of edgelist to form a path.
   An edge located at a vertex of a double-2-edge and pointing to the
   outside, so that in counterclockwise (prev-) direction the first edge of 
   the m-part follows is returned in case m!=0 and an arbitrary edge to the 
   outside otherwise.
*/

{
int i, vertices;
EDGE *run;

vertices=2*(l+m);

if (m != 0)
{
/* The first edge of the triple is always taken to point to the outside */
/* first initialize the first 2,3 pair: */
run=edgelist;
run->end=outside; run->invers=NULL;
run++;
run->invers=edgelist+3*vertices-1; run->end=run->invers->start;
run++;
run->invers=run+3; run->end=run->invers->start;
run++;

/* Now the next l-1 pairs */
for (i=1; i<l; i++)
  { 
    run->end=outside; run->invers=NULL;
    run++;
    run->invers=run+3; run->end=run->invers->start;
    run++;
    run->invers=run-3; run->end=run->invers->start;
    run++;
    run->end=outside; run->invers=NULL;
    run++;
    run->invers=run-3; run->end=run->invers->start;
    run++;
    run->invers=run+3; run->end=run->invers->start;
    run++;
  }

/* Now the last vertex of the l-path */
run->end=outside; run->invers=NULL;
run++;
run->invers=run+4; run->end=run->invers->start;
run++;
run->invers=run-3; run->end=run->invers->start;
run++;



if (m==1)
  { 
    run->end=outside; run->invers=NULL;
    run++;
    run->invers=run+3; run->end=run->invers->start;
    run++;
    run->invers=run-4; run->end=run->invers->start;
    run++;
    run->end=outside; run->invers=NULL;
    run++;
    run->invers=run-3; run->end=run->invers->start;
    run++;
    run->invers=edgelist+1; run->end=run->invers->start;
    run++;
  }
else /* that is m>=2 */
  { run->end=outside; run->invers=NULL;
    run++;
    run->invers=run+3; run->end=run->invers->start;
    run++;
    run->invers=run-4; run->end=run->invers->start;
    run++;

    for (i=1; i<m; i++)
      { run->end=outside; run->invers=NULL;
	run++;
	run->invers=run-3; run->end=run->invers->start;
	run++;
	run->invers=run+3; run->end=run->invers->start;
	run++;
	run->end=outside; run->invers=NULL;
	run++;
	run->invers=run+3; run->end=run->invers->start;
	run++;
	run->invers=run-3; run->end=run->invers->start;
	run++;
      }
    run->end=outside; run->invers=NULL;
    run++;
    run->invers=run-3; run->end=run->invers->start;
    run++;
    run->invers=edgelist+1; run->end=run->invers->start;
    run++;
  }
} /* end of m!=0 */
else /* that is m==0 */
{
run=edgelist;
run->end=outside; run->invers=NULL;
run++;
run->invers=run+3*(vertices-1); run->end=run->invers->start;
run++;
run->invers=run+3; run->end=run->invers->start;
run++;

/* Now the next l-1 pairs */
for (i=1; i<l; i++)
  { run->end=outside; run->invers=NULL;
    run++;
    run->invers=run+3; run->end=run->invers->start;
    run++;
    run->invers=run-3; run->end=run->invers->start;
    run++;
    run->end=outside; run->invers=NULL;
    run++;
    run->invers=run-3; run->end=run->invers->start;
    run++;
    run->invers=run+3; run->end=run->invers->start;
    run++;
  }

/* Now the last vertex  */
run->end=outside; run->invers=NULL;
run++;
run->invers=edgelist+1; run->end=run->invers->start;
run++;
run->invers=run-3; run->end=run->invers->start;
run++;
}
return edgelist;
}


/*******************************MAKE_TUBE***********************************/

EDGE *make_tube(int first_label, int rows)

/* constructs a tube with boundary structure given by the global variables l and
   m and "rows" rows of hexagons. An edge pointing to the outside and located at 
   a vertex neighbouring the vertices of the concave edge of the tube on the l-part
   is returned in case m!=0 and an arbitrary edge to the outside otherwise. 
   Identifying this edge with the corresponding edge from the cap (the marked one),
   a total of (rows+1) rows is added to the initial cap. The labelling of the
   vertices starts with the smallest label first_label. */

{

EDGE *edges, *run, *run2, *returnedge, *lastedge;
int numvertices, i,j;

if (l<3) { fprintf(stderr,"Error -- value of l too small \n"); exit(1); }

numvertices=2*(l+m)*(rows+1);
if ((edges=(EDGE *)malloc(sizeof(EDGE)*3*numvertices)) == NULL)
  { fprintf(stderr,"Do not get enough space for the edges of the tube.\n");
    exit(1); }

 { int marki;
   EDGE *markrun;
   
   for (marki=0; (marki<MAX_EDGE_ARRAYS) && (edge_array_start[marki]!=0); marki++);
   if (marki==MAX_EDGE_ARRAYS) 
        { fprintf(stderr,"Constant MAX_EDGE_ARRAYS too small (3) -- increase.\n");
	  exit(1); }
   edge_array_start[marki]=edges;
   how_many_edges[marki]=3*numvertices;
   for (marki=3*numvertices, markrun=edges; marki>0; marki--, markrun++) markrun->mark__=0;
 }



/* initialize things around a vertex: */
for (i=0, run=edges; i<numvertices; i++)
  { run->start=first_label+i;
    run->next=run+2; run->prev=run+1; run++;
    run->start=first_label+i;
    run->next=run-1; run->prev=run+1; run++;
    run->start=first_label+i;
    run->next=run-1; run->prev=run-2; run++; }

returnedge=lastedge=makepath(edges);
edges += 6*(l+m);

for (i=1; i<=rows;i++, edges += 6*(l+m) )
  { 
    run=lastedge->next->invers->prev;
    lastedge=run2=makepath(edges);

    run->invers=run2; run->end=run2->start;
    run2->invers=run; run2->end=run->start;
    for (j=l+m-1; j>0; j--)
      { 
	do { run=run->next; 
	     if (run->end != outside) run=run->invers;} while (run->end != outside);
	do { run2=run2->prev;
	     if (run2->end != outside) run2=run2->invers;} while (run2->end != outside);
	run->invers=run2; run->end=run2->start;
	run2->invers=run; run2->end=run->start;
      }
  }

if (m != 0)
  {
    for (i=l-1; i ; i--) returnedge=returnedge->next->invers->next->invers->next;
  }

return returnedge;
}

/********************************USAGE*************************************/

void usage(char name[])
{
fprintf(stderr,"Usage: %s l m with (l,m) the encoding of the cap \n",name);
exit(0);
}


/********************************IS_NUMBER*********************************/

int is_number(char string[])
{
while (*string != 0) { if (!isdigit(*string)) return 0; string++; }
return 1;
}




/* === caps.c == end ================================================== */
int group;
/* === iso.c == start ================================================= */
int lm_patch_is_canonical (EDGE *mark)

/* This is the main routine which organises the isomorphism testing
   of a newly generated patch.

   *mark is a pointer to the marked edge of the patch. */

{

   int i, j, k, result, dist_to_convex=0, pentagon_found, auts=0, mirror=0, eight;
   int dist_to_convex2=0, zero;
   EDGE *run, *starting_point, *original_pentagon_edge;
   static VERTEX *representation=NULL;

   if (representation==NULL)
     {
       if ((representation=(VERTEX *)calloc(maxlabel*2,sizeof(VERTEX)))==NULL)
       { fprintf(stderr,"Cannot allocate memory for the representation.\n"); exit(0); }
     }

/* First, find the shortest distance between a pentagon and
   the convex edge in the `m' part (= dist_to_convex). */
   graph_counter++;
   zero=1;
   run = mark;
   run = run->prev;
   while (run->invers->pentagon_right == 0)
   {
      if (dist_to_convex == (m-1)) {return(0);}
      run = run->invers->next->invers->prev;
      dist_to_convex++;
   }

/* If dist_to_convex == 0 then the convex edge is itself
   a pentagon edge, and we can accept this patch
   immediately.   */

/* Count automorphisms for the group counts */
   if (mirror > 0) {(auts_statistic_mirror[mirror])++;}
   else {(auts_statistic[auts])++;}

   if (dist_to_convex == 0) {
      if (l != m) {auts_statistic[1]++; return(1);
      }
      else{
         get_representation(mark, representation);
         starting_point = mark->prev->invers->prev;
         result = check_representation(starting_point, representation,0);
         if (result == 0) { auts_statistic[1]++; return (1); }
         if (result == 1) { auts_statistic_mirror[1]++; return (1); }
         if (result == 1) { return (0); }
      }
   }
   original_pentagon_edge = run->invers;

   /* Now find the shortest distance between a pentagon and
      the convex edge in the `l' part (= dist_to_convex2). */
   run = mark;
   run = run->prev->invers;
   while (run->pentagon_right == 0)
   {
      if (dist_to_convex2 == (l-1)) {return(0);}
      run = run->invers->prev->invers->next;
      dist_to_convex2++;
   }
   if (l == m){
      if (dist_to_convex2 < dist_to_convex) {return(0);}
   }

/* Get the initial representation (move this part later!) */

   get_representation(mark, representation);

/* Now, glue the new cap patch into the (global) tube patch */

   glue(mark);

/* Loop over pentagon edges: */

   for (i = 0; i < pentagonedges; i++)
   {
      for (j = 0; j < dist_to_convex; j++) /* NB Not dist_to_convex*/
      {
         run = pentagons[i];
         pentagon_found = 0;
         if (run->invers->pentagon_right==1) {goto one;}
         for (k = 0; k < j; k++)
         {
            run = run->invers->next->invers->prev;
            if (run->invers->pentagon_right==1) {goto one;} 
         }
         for (k = 0; k < l; k++)
         {   if (run->pentagon_right == 1) {pentagon_found=1;} 
          run = run->invers->prev->invers->next; 
            if (run->invers->pentagon_right==1) {goto one;} 
         }
         for (k = 0; k < (m-j); k++)
         { run = run->invers->next->invers->prev; 
            if (run->invers->pentagon_right==1) {goto one;} 
         }
         if (run == pentagons[i] && pentagon_found == 1)
         {
            eight = intersect(run, j);
            if (eight == 0){
            /* Shorter distance -> unglue and discard */
               unglue2(mark);
               return(0); 
            }
         }
      one: if (l == m)
         {
            run = pentagons[i];
            pentagon_found = 0;
            run = run->invers;
            if (run->pentagon_right==1) {goto two;} 
            for (k = 0; k < j; k++)
            {
               run = run->invers->prev->invers->next;
               if (run->pentagon_right==1) {goto two;} 
            }
            for (k = 0; k < l; k++)
            {   if (run->invers->pentagon_right == 1) {pentagon_found=1;}
             run = run->invers->next->invers->prev; 
             if (run->pentagon_right==1) {goto two;} 
            }
            for (k = 0; k < (m-j); k++)
            { run = run->invers->prev->invers->next; 
             if (run->pentagon_right==1) {goto two;} 
            }
            run = run->invers;
            if (run == pentagons[i] && pentagon_found == 1)
            {
            run = run->invers;
            eight = intersect_v(run, j);
            if (eight == 0){
               /* Shorter distance -> unglue and discard */
                  unglue2(mark);
                  return(0);
               }
            }
         }
two : continue;
      }

   /* Now do dist_to_convex. */

      run = pentagons[i];
      if (run == original_pentagon_edge) {auts++; goto three;}
      pentagon_found = 0;
      if (run->invers->pentagon_right == 1) {goto three;}
      for (j = 0; j < dist_to_convex; j++)
      { run = run->invers->next->invers->prev; 
        if (run->invers->pentagon_right == 1) {goto three;}
      }

   /* Assign the mark to `starting_point' for use in 
      evaluating the planar code below */

      starting_point = run;
      starting_point = starting_point->invers->next;

      for (j = 0; j < l; j++)
      {  if (run->pentagon_right == 1) {pentagon_found=1;}
         run = run->invers->prev->invers->next; 
         if (run->invers->pentagon_right == 1) {goto three;}
      }
      for (j = 0; j < (m-dist_to_convex); j++)
      {  run = run->invers->next->invers->prev; 
         if (run->invers->pentagon_right == 1) {goto three;}
      }
      if (run == pentagons[i] && pentagon_found == 1)
      {
      eight = intersect(run, dist_to_convex);
      if (eight == 0){
      /* Unglue the newly discovered patch */

         if (zero==1) { unglue2(mark);
            get_representation(mark, representation);
            glue(mark);
            unglue(run, dist_to_convex); 
            result = check_representation(starting_point, representation,1);
            if (result == 2) { reglue(run, dist_to_convex);
               unglue2(mark);
               return (0);
            }
            else if (result == 1) { auts++; } 
            zero=0;
         }
         else{
            unglue(run, dist_to_convex); 
            result = check_representation(starting_point, representation,1);
            if (result == 2) 
            {  reglue(run, dist_to_convex);
               unglue2(mark);
               return (0);
            }
            else if (result == 1) { auts++; } 
         }

      /* The original patch survives. Reglue it and continue */

         reglue(run, dist_to_convex); 
      }
      }
three:if (l == m)
      {
         run = pentagons[i];
         pentagon_found = 0;
         run = run->invers;
         if (run->pentagon_right==1) {goto four;}
         for (j = 0; j < dist_to_convex; j++)
         { run = run->invers->prev->invers->next; 
           if (run->pentagon_right==1) {goto four;}
         }
         starting_point = run;
         starting_point = starting_point->invers->prev;
         for (j = 0; j < l; j++)
         {  if (run->invers->pentagon_right == 1) {pentagon_found=1;}
            run = run->invers->next->invers->prev; 
            if (run->pentagon_right==1) {goto four;}
         }
         for (j = 0; j < (m-dist_to_convex); j++)
         {  run = run->invers->prev->invers->next; 
            if (run->pentagon_right==1) {goto four;}
         }
         run = run->invers;
         if (run == pentagons[i] && pentagon_found == 1)
         {
            run = run->invers;
         eight = intersect_v(run, dist_to_convex);
         if (eight == 0){
            if (zero==1) { unglue2(mark);
               get_representation(mark, representation);
               glue(mark);
               ungluev(run, dist_to_convex);
               result = check_representation(starting_point, representation,0);
               if (result == 2) 
               {  regluev(run, dist_to_convex);
                  unglue2(mark);
                  return (0);
               }
               else if (result == 1) { mirror++;}
               zero=0;
            }
            else{
               ungluev(run, dist_to_convex);
               result = check_representation(starting_point, representation,0);
               if (result == 2) 
               {  regluev(run, dist_to_convex);
                  unglue2(mark);
                  return (0);
               }
               else if (result == 1) { mirror++;}
            }
            regluev(run, dist_to_convex); 
         } 
         }
      }
four: continue;
   }
  
/* If we are here then our original patch has survived. Unglue it
   and return */

   unglue2(mark);

/* Count automorphisms for the group counts */ 
   if (mirror > 0) {(auts_statistic_mirror[mirror])++;} 
   else {(auts_statistic[auts])++;}

   return (1);
}


int glue(EDGE *mark)

/* A routine to glue a newly created patch into a previously generated 
   tube graph. The mark of the cap patch is the first aussen edge 
   attached to the convex edge in a clockwise direction. The tube
   patch has an `interior' boundary where the aussen edges point
   inwards. Its marked edge is the first aussen (pointing inwards)
   in a clockwise direcion from the convex edge from the cap
   patch point iof view (= the concave edge of the interior
   boundary of the tube patch). 

   `mark' is a pointer to the cap patch marked edge and `isomorphism_tube'
   is a pointer to the tube patch marked edge (which should be
   global. */

{

  EDGE *edge1, *edge2; 
  int i;

  edge1 = mark;
  edge2 = isomorphism_tube;

/* At this point, it is assumed that edge1 points to the marked
   aussen of the cap patch, and that edge2 points to the corresponding
   (inward pointing) aussen of the tube patch. */

  for (i = 1; i < l; i++)
  {
    edge1->end = edge2->start;  
    edge1->invers = edge2;
    edge2->end = edge1->start;
    edge2->invers = edge1;
    edge1 = edge1->next->invers->next->invers->next;
    edge2 = edge2->prev->invers->prev->invers->prev;
  }
  edge1->end = edge2->start;
  edge1->invers = edge2;
  edge2->end = edge1->start;
  edge2->invers = edge1;
  edge1 = edge1->next->invers->next->invers->next->invers->next;
  edge2 = edge2->prev->invers->prev;
  for (i = 1; i < m; i++)
  {
    edge1->end = edge2->start;
    edge1->invers = edge2;
    edge2->end = edge1->start;
    edge2->invers = edge1;
    edge1 = edge1->next->invers->next->invers->next;
    edge2 = edge2->prev->invers->prev->invers->prev; 
  }
  edge1->end = edge2->start;
  edge1->invers = edge2;
  edge2->end = edge1->start;
  edge2->invers = edge1;

  return(1); 
}

int unglue2 (EDGE *mark)

/* A routine to perform the final ungluing before we return
   from the isomorphism routine.*/

{
  EDGE *edge1, *edge2; 
  int i;

  edge1 = mark;
  edge2 = isomorphism_tube;

  for (i = 1; i < l; i++)
  {
    edge1->end = outside;
    edge1->invers = NULL;
    edge2->end = outside;
    edge2->invers = NULL;
    edge1 = edge1->next->invers->next->invers->next;
    edge2 = edge2->prev->invers->prev->invers->prev;
  }
  edge1->end = outside;
  edge1->invers = NULL;
  edge2->end = outside;
  edge2->invers = NULL;
  edge1 = edge1->next->invers->next->invers->next->invers->next;
  edge2 = edge2->prev->invers->prev;
  for (i = 1; i < m; i++)
  {
    edge1->end = outside;
    edge1->invers = NULL;
    edge2->end = outside;
    edge2->invers = NULL;
    edge1 = edge1->next->invers->next->invers->next;
    edge2 = edge2->prev->invers->prev->invers->prev; 
  }
  edge1->end = outside;
  edge1->invers = NULL;
  edge2->end = outside;
  edge2->invers = NULL;
  return(1);
}



int unglue(EDGE *mark, int dist_to_convex)

/* A routine to unglue a patch from the surrounding graph. 
   `dist_to_convex' is the number of (1,0) vertices before the convex edge,
   `mark' is a pointer to the starting vertex of the path. `outside' is taken
   to be a global variable used to mark `aussen' bonds. At the end of
   the routine, the pointer `run' is set to the starting edge.
   This routine is only used when checking a new patch for the planar code.*/

{

  EDGE *run; 
  int i;
  run = mark;
  for (i=0; i<dist_to_convex; i++)
  {
    run = run->invers->next->invers->next;
    run->end = outside; 
    run = run->next;
  }
  for (i=0; i<l; i++)
  {
    run = run->invers->next;
    run->end = outside; 
    run = run->next->invers->next;
  }
  for (i=0; i<(m-dist_to_convex); i++)
  {
    run = run->invers->next->invers->next;
    run->end = outside; 
    run = run->next;
  }
  return(1);
}

int ungluev(EDGE *mark, int dist_to_convex)

/* A routine to unglue a patch from the surrounding graph.
   FOR MIRROR IMAGES ONLY */

{

  EDGE *run;
  int i;
  run = mark;
  for (i=0; i<dist_to_convex; i++)
  {
    run = run->invers->prev->invers->prev;
    run->end = outside;
    run = run->prev;
  }
  for (i=0; i<l; i++)
  {
    run = run->invers->prev;
    run->end = outside;
    run = run->prev->invers->prev;
  }
  for (i=0; i<(m-dist_to_convex); i++)
  {
    run = run->invers->prev->invers->prev;
    run->end = outside;
    run = run->prev;
  }
  return(1);
}


void reglue(EDGE *mark, int dist_to_convex)

/* A routine to glue an unglued patch back into the surrounding graph. 
   `dist_to_convex' is the number of (1,0) vertices before the convex edge,
   `mark' is a pointer to the starting edge of the path. At the end of
   the routine, the pointer `edge' is set to the starting edge.*/

{

  EDGE *edge; 
  int i;

  edge = mark;
  for (i=0; i<dist_to_convex; i++)
  {
    edge = edge->invers->next->invers->next;
    edge->end = edge->invers->start;
    edge = edge->next;
  }
  for (i=0; i<l; i++)
  {
    edge = edge->invers->next;
    edge->end = edge->invers->start;
    edge = edge->next->invers->next;
  }
  for (i=0; i<(m-dist_to_convex); i++)
  {
    edge = edge->invers->next->invers->next;
    edge->end = edge->invers->start;
    edge = edge->next;
  }
}

void regluev(EDGE *mark, int dist_to_convex)

/* A routine to glue an unglued patch back into the surrounding graph.
   `dist_to_convex' is the number of (1,0) vertices before the convex edge,
   `mark' is a pointer to the starting edge of the path. At the end of
   the routine, the pointer `edge' is set to the starting edge.
   FOR MIRROR IMAGES ONLY */

{

  EDGE *edge;
  int i;

  edge = mark;
  for (i=0; i<dist_to_convex; i++)
  {
    edge = edge->invers->prev->invers->prev;
    edge->end = edge->invers->start;
    edge = edge->prev;
  }
  for (i=0; i<l; i++)
  {
    edge = edge->invers->prev;
    edge->end = edge->invers->start;
    edge = edge->prev->invers->prev;
  }
  for (i=0; i<(m-dist_to_convex); i++)
  {
    edge = edge->invers->prev->invers->prev;
    edge->end = edge->invers->start;
    edge = edge->prev;
  }
}

int intersect(EDGE *mark, int dist_to_convex) 

/* This routine checks whether a newly found path intersects with itself.
   It returns 1 if it does intersect and 0 otherwise. */

{
   EDGE *run;
   int i;

   RESETMARKS_E;
   run = mark;
   MARK_E(run);
   for (i=0; i<dist_to_convex; i++){
      run = run->invers->next;
      if (ISMARKED_E(run->invers)){return(1);}
      MARK_E(run);
      run = run->invers->prev;
      if (ISMARKED_E(run->invers)){return(1);}
      MARK_E(run);
   }
   for (i=0; i<l; i++){
      run = run->invers->prev;
      if (ISMARKED_E(run->invers)){return(1);}
      MARK_E(run);
      run = run->invers->next;
      if (ISMARKED_E(run->invers)){return(1);}
      MARK_E(run); 
   }
   for (i=0; i<(m-dist_to_convex-1); i++){
      run = run->invers->next;
      if (ISMARKED_E(run->invers)){return(1);}
      MARK_E(run);
      run = run->invers->prev;
      if (ISMARKED_E(run->invers)){return(1);}
      MARK_E(run);
   }
   return(0);
}

int intersect_v(EDGE *mark, int dist_to_convex) 

/* Same as `intersect', except for mirror images. */
   

{
   EDGE *run;
   int i;

   RESETMARKS_E;
   run = mark;
   MARK_E(run);
   for (i=0; i<dist_to_convex; i++){
      run = run->invers->prev;
      MARK_E(run);
      if (ISMARKED_E(run->invers)){return(1);}
      run=run->invers->next;
      MARK_E(run);
      if (ISMARKED_E(run->invers)){return(1);}
   }
   for (i=0; i<l; i++){
      run = run->invers->next;
      MARK_E(run); 
      if (ISMARKED_E(run->invers)) {return(1);}
      run = run->invers->prev;
      MARK_E(run); 
      if (ISMARKED_E(run->invers)) {return(1);}
   }
   for (i=0; i<(m-dist_to_convex-1); i++){
      run = run->invers->prev;
      MARK_E(run);
      if (ISMARKED_E(run->invers)) {return(1);}
      run = run->invers->next;
      MARK_E(run);
      if (ISMARKED_E(run->invers)) {return(1);}
   }
   return(0);
}

/* === iso.c == end =================================================== */
#include <time.h>
#include <sys/times.h>
#define NEXT_ENTRY(i) (((i+1)>number_of_rows) ? 1:(i+1))
#define ENTRY_SUM(i,k) ((x=((i+k)%number_of_rows)) ? x : (i+k))
#define XMOD2(x) ((x%2)?((x+1)/2) : (x/2))
#define MAX(a,b) ((a>b)?(a):(b))

#define C1 1
#define C2 2
#define C3 3
#define C5 4
#define C6 5
#define Cs 6
#define C2v 7
#define C3v 8
#define C5v 9
#define C6v 10

#include <sys/times.h>
#if !defined(CLK_TCK) && !defined(_SC_CLK_TCK)
#include <time.h>
#endif
#if !defined(CLK_TCK) && !defined(_SC_CLK_TCK)
#include <unistd.h>
#endif
#if !defined(CLK_TCK) && defined(_SC_CLK_TCK)
#define CLK_TCK sysconf(_SC_CLK_TCK)
#endif
#ifndef CLK_TCK
#define CLK_TCK 60     /* If the CPU time stated by the program appears
		      to be out by a constant ratio, the most likely
		      explanation is that the code got to this point but
		      60 is the wrong guess.  Another common value is 100. */
#endif

#define time_factor CLK_TCK


int cap_counter=0;
int outputtube_length=0;
EDGE *outputtube_edge;
int no_output = 0;
char gruppe[11] = {1,1,1,1,1,1,1,1,1,1,1};
int split = 0;
int level=0; /*depth in the generation tree, defined by the number 
	       of added lines*/
int split_level; 
int rest=0,mod=1; /* Parameters for splitting the generation tree*/
int modcounter=0;
int pentagon_m_part = 0;
/* The following constants are used only for debugging and testing; 
   not valid for a final version */
#define MAXLEVEL 100
#define OUTPUT 1

int maxlevel = 0;
void output_map();
int vertex_bound;
int header=0;
int sign=0;
int knoten[MAXLEVEL] = {0};

/************************ lm_patch_is_canonical ***************************/
/* This routine is a dummy for the isomorphism test*/

int xlm_patch_is_canonical(EDGE *mark)
{
  return(1);
}
/************************** output_patch **********************************/
int output_patch(EDGE *mark)
{
  FILE *fil;
  EDGE *edge;
  fil = stdout;
  edge = mark;

  
  if (split && level<split_level){
    if (rest%mod)return(0);
  }

  if (gruppe[group]){
#ifdef DEBUG
    if (vertex_bound < nv)fprintf(stderr,"Nr: %d; Grenze: %d, Eckenzahl: %d\n",cap_counter+1, vertex_bound, nv);
#endif
  if(!no_output){ 
    if (outputtube_length){
      if (m) output_glue(mark);
      else output_glue_zigzag(mark);
      /*write_planar_code_label_fix(fil,nv);*/
      write_planar_code_label(fil,edge);
      if (m) output_unglue(mark);
      else output_unglue_zigzag(mark);
    }
    else output_map();
  }
  }
  cap_counter++;
  return(cap_counter);
} 

/************************** output_unglue **********************************/
int output_unglue (EDGE *mark)

/* A routine to perform the final ungluing before we return
   from the isomorphism routine.*/

{
  EDGE *edge1, *edge2; 
  int i;

  edge1 = mark;
  edge2 = outputtube_edge;

  for (i = 1; i < l; i++)
  {
    edge1->end = outside;
    edge1->invers = NULL;
    edge2->end = outside;
    edge2->invers = NULL;
    edge1 = edge1->next->invers->next->invers->next;
    edge2 = edge2->prev->invers->prev->invers->prev;
  }
  edge1->end = outside;
  edge1->invers = NULL;
  edge2->end = outside;
  edge2->invers = NULL;
  edge1 = edge1->next->invers->next->invers->next->invers->next;
  edge2 = edge2->prev->invers->prev;
  for (i = 1; i < m; i++)
  {
    edge1->end = outside;
    edge1->invers = NULL;
    edge2->end = outside;
    edge2->invers = NULL;
    edge1 = edge1->next->invers->next->invers->next;
    edge2 = edge2->prev->invers->prev->invers->prev; 
  }
  edge1->end = outside;
  edge1->invers = NULL;
  edge2->end = outside;
  edge2->invers = NULL;
  return(1);
}

/************************** output_glue_zigzag **********************************/
int output_glue_zigzag(EDGE *mark)
{
  EDGE *edge1, *edge2;
  int i;
  
  edge1 = mark;
  edge2 = outputtube_edge;

  for (i=0; i< l; i++){
    edge1->end = edge2->start;
    edge2->end = edge1->start;
    edge1->invers = edge2;
    edge2->invers = edge1;

    edge1 = edge1->prev->invers->prev->invers->prev;
    edge2 = edge2->next->invers->next->invers->next;
  }

return(1);
}
/************************** output_unglue_zigzag **********************************/
int output_unglue_zigzag (EDGE *mark)
{
  EDGE *edge1, *edge2;
  int i;
  
  edge1 = mark;
  edge2 = outputtube_edge;
  for (i = 0; i < l; i++)
    {
      edge1->end = outside;
      edge1->invers = NULL;
      edge2->end = outside;
      edge2->invers = NULL;
      edge1 = edge1->next->invers->next->invers->next;
      edge2 = edge2->prev->invers->prev->invers->prev;
    }
return(1);
}

/************************** output_glue **********************************/
int output_glue(EDGE *mark)

/* A routine to glue a newly created patch into a previously generated 
   tube graph. The mark of the cap patch is the first aussen edge 
   attached to the convex edge in a clockwise direction. The tube
   patch has an `interior' boundary where the aussen edges point
   inwards. Its marked edge is the first aussen (pointing inwards)
   in a clockwise direcion from the convex edge from the cap
   patch point iof view (= the concave edge of the interior
   boundary of the tube patch). 

   `mark' is a pointer to the cap patch marked edge and `isomorphism_tube'
   is a pointer to the tube patch marked edge (which should be
   global. */

{

  EDGE *edge1, *edge2; 
  VERTEX setpoint;
  int i;

  edge1 = mark;
  edge2 = outputtube_edge;

/* At this point, it is assumed that edge1 points to the marked
   aussen of the cap patch, and that edge2 points to the corresponding
   (inward pointing) aussen of the tube patch. */
  for (i = 1; i < l; i++)
  {
    setpoint = edge2->start; 
    edge1->end = setpoint;
    edge1->invers = edge2;
    setpoint = edge1->start;
    edge2->end = setpoint;
    edge2->invers = edge1;
    edge1 = edge1->next->invers->next->invers->next;
    edge2 = edge2->prev->invers->prev->invers->prev;
  }
  setpoint = edge2->start;
  edge1->end = setpoint;
  edge1->invers = edge2;
  setpoint = edge1->start;
  edge2->end = setpoint;
  edge2->invers = edge1;
  edge1 = edge1->next->invers->next->invers->next->invers->next;
  edge2 = edge2->prev->invers->prev;
  for (i = 1; i < m; i++)
  {
    setpoint = edge2->start;
    edge1->end = setpoint;
    edge1->invers = edge2;
    setpoint = edge1->start;
    edge2->end = setpoint;
    edge2->invers = edge1;
    edge1 = edge1->next->invers->next->invers->next;
    edge2 = edge2->prev->invers->prev->invers->prev; 
  }
  setpoint = edge2->start;
  edge1->end = setpoint;
  edge1->invers = edge2;
  setpoint = edge1->start;
  edge2->end = setpoint;
  edge2->invers = edge1;
  return(1); 
}
/************************** output_map **********************************/
/* im Wesentlichen eine Kopie von "writemap", die den Graphen nach 
   stdout schreibt. Geht davon aus, dass unsigned char reicht.  
   OUTPUT = 0: stderror
   OUTPUT != 0: stdout */
void output_map()
{
  int i,j;
  EDGE *run;
  unsigned short zeichen;
  unsigned short null=0;
  FILE *file;

  if (OUTPUT) file = stdout;
  else file = stderr;

  if (!header){fprintf(file,">>planar_code %ce<<", my_endianness);header=1;}
  putchar(0);
  zeichen = nv; 
  fwrite(&zeichen,sizeof(unsigned short),1,file);    
  for (i=1; i<=nv; i++){
    run=firstedge[i];
    for (j=0; j<3; j++, run=run->prev){
      if (run->end != outside) {
	zeichen = run->end;
	fwrite(&zeichen,sizeof(unsigned short),1,file);}      
    }
    fwrite(&null,sizeof(unsigned short),1,file);
  }
  fflush(file);
}
/******************************* reduce *********************************/
/* Deletes a row of "face_number" faces; beginning with the last face added. 
   Returns the (dangling) edge of the patch, at which a new row can be added. 
*/

EDGE *reduce(EDGE *start, int face_number)
{
  int i;

  level--;
  for (i=0; i<face_number; i++) start = delete_n_gon(start);
  is_ipr = 1;
  return(start);
}
/******************************* operation_notpc ******************************/
/* int hex_number: number of hexagons to be added
   int pent_end:  1 if pentagon at the end
   EDGE **pmark: pointer to the marked edge; also the first edge where a new face 
                 will be added
        
  Adds a row of hex_number hexagons and pent_end pentagons. The mark will be the 
  dangling edge which starts at the end of the first added convex edge 

  It has to be ensured that the operation is well-defined, i.e. the
  number of hexagons must be adapted to the place.

*/
EDGE *operation_notpc(int hex_number, int pent_end,  EDGE **pmark)
{

  int i,not_marked=0;
  EDGE *mark;
  EDGE *start;

  level++;
  mark = start = *pmark;
  if (hex_number){    /* The "normal" case, when the number of hexagons is 
			 not 0. Otherwise the mark must be changed to 
			 the _last_ pentagon, since it is the only face 
			 to be added.*/
    start = add_n_gon(6, start);
    mark = start->prev->invers->prev;
    hex_number--;}
  else not_marked = 1;

  for (i=0; i<hex_number; i++) start = add_n_gon(6, start);
  if (pent_end)  start = add_n_gon(5, start);
  if (not_marked) mark = start;
  *pmark = mark;
  return(start);
}
/******************************* operation ******************************/
/*int pent_start: 1, if pentagon at the beginning
  int hex_number: number of hexagons to be added
  int pent_end:  1 if pentagon at the end
  EDGE *start: first edge where a new face will be added

  Adds a row of pent_start pentagons, hex_number hexagons, pent_end
  pentagons. The mark will be the dangling edge which starts at the
  end of the first added convex edge or in case that the first face is a pentagon

  It has to be ensured that the operation is well-defined, i.e. the
  number of hexagons must be adapted to the place.

*/
EDGE *operation(int pent_start, int hex_number, int pent_end, EDGE *start, EDGE **pmark)
{

  int i,not_marked=0;
  EDGE *mark;

  level++;
  mark = *pmark;

  if (pent_start) {
    start = add_n_gon(5, start);
    mark = start->prev->invers->prev;
  }
  else { 
    if (hex_number){ 
      start = add_n_gon(6, start);
      mark = start->prev->invers->prev;
      hex_number--;}
    else not_marked = 1;
  }
   /* The "normal" case, if the number of hexagons is 
      not 0. Otherwise the mark must be changed to 
      the _last_ pentagon, since it is the only face 
      to be added.*/
  for (i=0; i<hex_number; i++){ 
    start = add_n_gon(6, start);
  }
  if (pent_end)   {
    start = add_n_gon(5, start);}
  if (not_marked) mark = start;
  *pmark = mark;

  return(start);
}
/************************ construct_notpc_patch *************************/
/* Set of constructions which can be applied only for _almost_ pseudoconvex patches.

   m_bound    Distance between concave and first convex edge,
   defined by (10)^m_bound; lower bound for "m"
*/
int construct_notpc_patch(EDGE *mark, int sequence[], int m_bound, int length)
{
  int i;
  EDGE *edge;
  int number_of_faces;
  int new_sequence[7];
  EDGE *new_mark;
  int new_length;
  int new_m_bound; 

knoten[level]++;
maxlevel = MAX(level, maxlevel);
  if (split){
    if (level == split_level){
      modcounter++;
      if ((modcounter%mod) != rest )  return(0); }
  }
  new_mark = mark;
  number_of_faces = sequence[0];
  new_m_bound = m_bound+1;
  if (!(number_of_faces) ||(new_m_bound > m) || (m<(m_bound+length-1)))return(0);
  if (length == 1){
    if ( (!construct_ipr||is_ipr) &&(m_bound == m) && (number_of_faces==l)&&lm_patch_is_canonical(new_mark)){
      output_patch(mark);  return(1);}
    else return(0); 
  }

  if (length == 2) { 
    if ((number_of_faces+sequence[1]==l) && (number_of_faces >= (m-m_bound)))
      finish_patch(mark, sequence, m_bound);
    return(0);
  }
  if ((nv+2*number_of_faces)<=maxnv_cap){
    edge = operation_notpc(number_of_faces-1,1,&new_mark);
    new_length = length-1;
    new_sequence[0] = sequence[0]+sequence[1]; 
    for (i=1;i<new_length;i++) new_sequence[i] = sequence[i+1]; 
    if (!construct_ipr||is_ipr)
      construct_notpc_patch(new_mark, new_sequence, new_m_bound,new_length);
    if ((nv+1) <= maxnv_cap){
      edge = delete_n_gon(edge); is_ipr=1; /* Deleting of the last pentagon */
      edge = add_n_gon(6,edge);  /* Adding a hexagon instead */
      if (number_of_faces==1) new_mark = edge->prev->invers->prev;
      /* Otherwise the new_mark is still valid*/
      new_length = length;
      new_sequence[0] = sequence[0]-1;
      new_sequence[1] = sequence[1] + 1;
      for(i=2;i<new_length; i++)new_sequence[i] = sequence[i];
      if (!construct_ipr||is_ipr)
	construct_notpc_patch(new_mark, new_sequence, new_m_bound,new_length);
      if ( !sequence[1]){ /* Additional face possible */
	if ((nv+2) <= maxnv_cap) {
	  edge =  add_n_gon(5,edge); 
	  number_of_faces++;
	  new_length = length-1;
	  new_sequence[0] = sequence[0];
	  new_sequence[1] = sequence[2]+1;
	  for (i=2;i<new_length;i++) new_sequence[i] = sequence[i+1];
	  if (!construct_ipr||is_ipr)
	    construct_notpc_patch(new_mark, new_sequence, new_m_bound,new_length);
	  if ((nv+1) <= maxnv_cap){
	    edge = delete_n_gon(edge);is_ipr=1;  /* Deleting of the last pentagon */
	    edge = add_n_gon(6,edge);  /* Adding a hexagon instead */
	    new_length = length;
	    new_sequence[0] = sequence[0];
	    new_sequence[1] = 0; /* (=sequence[1])*/
	    new_sequence[2] = sequence[2]+1;
	    for (i=3;i<new_length;i++) new_sequence[i] = sequence[i];
	    if (!construct_ipr||is_ipr)
	      construct_notpc_patch(new_mark, new_sequence, new_m_bound,new_length);
	  }
	}
      }
    }
    edge = reduce(edge, number_of_faces);is_ipr=1;
    return(0);
  }
  else return(0); 
}

/************************ construct_patch *************************/
/* Set of constructions which can be applied only for pseudoconvex patches.*/
int construct_patch(EDGE *mark, int sequence[], EDGE *starts[], int length)
{
  int i,j,k,x;
  int canonical,sum;
  EDGE *edge;
  int number_of_rows;
  int number_of_faces,max_number;
  int new_sequence[7];
  EDGE *new_starts[7];
  EDGE *new_starts_mirror[7];
  EDGE *new_mark;
  EDGE *startedge;
  int new_length;
  int new_numstarts;
  int new_numstarts_mirror;
  int boundary;

knoten[level]++;
maxlevel = MAX(level, maxlevel);
  if (split){
    if (level == split_level) {
      modcounter++;
      if ((modcounter%mod) != rest ) return(0); 
    }
  }
/********* output of the patches **************/
  /* Computation of the boundary length: */
  for (i=1,boundary=0; i<=sequence[0]; i++){ boundary+=sequence[i];}

  if ((boundary>(l+m-length+1)) || (nv > maxnv_cap)) return(0);

  /******* Construction **************/
   new_mark = mark;
 /* Default operations: */
  if (pentagoncounter<5 ) {
    number_of_rows = sequence[0];
    for (i=1; i<=number_of_rows; i++){   
      number_of_faces = sequence[i];
      if (number_of_faces){
	/* "Default operation" with pentagon at the end: */ 
	if ((2*number_of_faces + nv )<=maxnv_cap) {/* Number of vertices after 
						     applying this operation*/
	  edge = operation(0,number_of_faces-1,1, starts[i], &new_mark);
	  /* "mark" is on the second (clockwise) "outside-edge" 
	     of the first face.*/
	  new_length = length-1;
	  canonical=compute_sequence(new_mark, new_sequence, new_starts,&new_length);
	  if (canonical && (!construct_ipr||is_ipr)){
	    construct_patch(new_mark, new_sequence, new_starts, new_length);}
	  /* Possible since the next operations would induce an even 
	     bigger increase in the number of vertices. So this row is finished. */
	  /* "Default operation" without pentagon: */
	  if ((1+nv) <= maxnv_cap) {/* one vertex more than in the first case*/
	    edge = delete_n_gon(edge);is_ipr=1;
	    edge = add_n_gon(6,edge);
	    if (number_of_faces == 1)new_mark = edge->prev->invers->prev;
	    /* The only case where the _first_ face is deleted and so a new mark 
	       has to be set. */
	    new_length = length;
	    canonical=compute_sequence(new_mark, new_sequence, new_starts,&new_length);
	    if(canonical){ /* condition is_ipr=1 always fulfilled*/
	    construct_patch(new_mark, new_sequence, new_starts, new_length);}
	    /* Deletion of the remaining faces. Well-defined, 
	       since number_of_faces>0 */
	    /* Additional Pentagon, if there is a 0-edge at the end */ 
	    /* new_mark remains the same as before */  
	    if  ((sequence[NEXT_ENTRY(i)] == 0) && (length>2)&&((2+nv)<=maxnv_cap)){
	      edge = add_n_gon(5,edge);
	      number_of_faces++;
	      new_length = length-1;
	      canonical=compute_sequence(new_mark, new_sequence, new_starts,&new_length);
	      if (canonical && ((!construct_ipr)||is_ipr)){
		construct_patch(new_mark, new_sequence, new_starts, new_length);}
	    }
	  }
	  edge = reduce(edge, number_of_faces); is_ipr=1;/* Deletion of the remaining hexagons */
	}
      }
    }
  }

  if (length == 2) {
    if ((number_of_faces = sequence[1])){/* convex edges not neighbours (otherwise 
					  this operation won't lead to a canonical 
					  patch) */
      if ((number_of_faces+sequence[NEXT_ENTRY(1)]+1==l) && !m){/* Optimisation */
	/*the result patch will satisfy (l,0) */
	if ((2*(number_of_faces-1) + nv)<=maxnv_cap){ 
	  /* if this condition is not fulfilled the 
	     second row can neither be added, since it 
	     is even longer than this one */     
	  if (number_of_faces>1)  { /*Otherwise not enough places for two pentagons*/
	    edge = operation(1,number_of_faces-2,1, starts[1], &new_mark);
	    new_length = 0;
	    canonical = compute_sequence_pentagon(new_mark,new_starts,&new_numstarts,new_starts_mirror,&new_numstarts_mirror);	  
	    if (canonical &&(!construct_ipr||is_ipr) && l_patch_is_canonical(new_mark,new_starts,&new_numstarts,new_starts_mirror,&new_numstarts_mirror))output_patch(new_mark);
	    edge = reduce(edge, number_of_faces);is_ipr=1;
	  }
	  if ((sequence[0]>1) && ((number_of_faces = sequence[NEXT_ENTRY(1)])>1) &&  ((2*(number_of_faces-1) + nv)<=maxnv_cap)){
	    /* Second row only if it won't lead to mirror images */
	    number_of_faces = sequence[2];
	    edge = operation(1,number_of_faces-2,1, starts[2], &new_mark);
	    new_length = 0;
	    canonical = compute_sequence_pentagon(new_mark,new_starts,&new_numstarts,new_starts_mirror,&new_numstarts_mirror);
	    if (canonical &&(!construct_ipr||is_ipr) && l_patch_is_canonical(new_mark,new_starts,&new_numstarts,new_starts_mirror,&new_numstarts_mirror))output_patch(new_mark);
	    edge = reduce(edge, number_of_faces);is_ipr=1;
	  }
	}
      }
    }
    else { /* Convex edges are neighbours */
      number_of_faces = sequence[2]+1;
      if (((2*number_of_faces+3) + nv) <= maxnv_cap){ 
	edge = operation(0,number_of_faces,1, starts[1], &new_mark);
	/* Adding of a complete layer of faces; hexagon at the end would 
	   never be canonical;*/
	new_length = 1;
	canonical=compute_sequence(new_mark, new_sequence, new_starts,&new_length);	
	if (canonical &&(!construct_ipr||is_ipr)){
	construct_patch(new_mark, new_sequence, new_starts, new_length);}
	edge = reduce(edge, number_of_faces+1);is_ipr=1;
      }
    }
  }
  /* Test for canonicity not necessary since there is only one convex edge resp. 
     one pentagon */
  if (length==1) {
    /* Ring of hexagons */
    number_of_faces = sequence[1];
    if ((2*(number_of_faces+1) + nv)<=maxnv_cap) {
      edge = operation(0,number_of_faces,0, starts[1], &new_mark);
      /*Last face still missing*/      
      /* Second operation for finishing a pseudoconvex patch: */
      if (!m && (l==number_of_faces+1)){
	edge = add_n_gon(5,edge); /* Last face is pentagon*/
	new_length = 0;
	new_mark = edge;
	compute_sequence_pentagon(new_mark,new_starts,&new_numstarts,new_starts_mirror,&new_numstarts_mirror);
	if ((!construct_ipr||is_ipr) && l_patch_is_canonical(new_mark,new_starts,&new_numstarts,new_starts_mirror,&new_numstarts_mirror)) output_patch(new_mark);
	edge = delete_n_gon(edge);is_ipr=1;
      }
      if((nv+1)<=maxnv_cap) {
        edge = add_n_gon(6,edge);/* Last face is hexagon*/
	new_mark = edge;
	new_length = length;
	compute_sequence(new_mark, new_sequence, new_starts,&new_length);
	/*Always canonical; routine only used to get the actual parameters*/
	construct_patch(new_mark, new_sequence, new_starts, new_length);
	edge = delete_n_gon(edge);
      }
      edge = reduce(edge, number_of_faces);is_ipr=1;/* Deletion of hexagons*/
    }
  }
  /* Step pseudoconvex --> almost pseudoconvex; "normal" case */
  if (m){
    if (length>1){
      number_of_rows = sequence[0];
      sum = 0;
      for (i=1; i<=number_of_rows; i++)    sum+=sequence[i];
      sum = (sum*(length/number_of_rows)); 
      if (sum <= l){ /*Otherwise boundary would never satisfy l */
	for (i=1; i<=number_of_rows; i++){
	  max_number = sequence[i];
	  if (max_number>1){
	    startedge = starts[i];
	    for(j=1; j<max_number; j++){
	      /* new starting point for the "incomplete rows" will be computed*/
	      startedge = startedge->next->invers->next->invers->next;
	      number_of_faces = max_number -j; /* Number of faces which will be added */
	      if (((nv+number_of_faces*2)<=maxnv_cap) && (((m>=(number_of_faces+length-1))))){		  
		/* This condition uses the property of the caps that they 
		   must have a pentagon in each part of the boundary */

		edge = operation(0,number_of_faces-1,1,startedge,&new_mark);
		new_length = length;
		new_sequence[0] =  sequence[NEXT_ENTRY(i)]+number_of_faces;
		new_sequence[new_length-1] = j;
		if (new_length == 1)new_sequence[0] = number_of_faces + j;
		for(k=1; k<new_length-1; k++) new_sequence[k] = sequence[ENTRY_SUM((i),(k+1))];
		if (!construct_ipr||is_ipr)
		  construct_notpc_patch(new_mark, new_sequence, 1, new_length);
		if (((nv+1) <=maxnv_cap) && ((number_of_faces+length-1)<=m ) ){
		  /* This condition uses the property of the caps that they 
		     must have a pentagon in each part of the boundary */
		  edge = delete_n_gon(edge);is_ipr=1;
		  edge = add_n_gon(6,edge);
		  new_length = length+1; /* new number of convex edges */
		  if (number_of_faces == 1)new_mark = edge->prev->invers->prev;
		  /* The only case where the _first_ face is deleted and so a new mark 
		     has to be set. */
		  
		  new_sequence[0] = number_of_faces-1;/* between the two new convex edges*/
		  new_sequence[1] = sequence[NEXT_ENTRY(i)] + 1;
		  for(k=2; k<new_length-1; k++) new_sequence[k] = sequence[ENTRY_SUM((i),(k))];
		  new_sequence[new_length-1] = j;
		  if (length == 1) new_sequence[new_length-1] = j+1;
		  if (!construct_ipr||is_ipr)/* Not necessary; is_ipr=1*/
		    construct_notpc_patch(new_mark, new_sequence, 1, new_length);
		  /* Additional face can be added: */
		  if (!sequence[NEXT_ENTRY(i)]) {
		    /* Additional pentagon */
		    if ((nv+1 <=maxnv_cap) ){ 
		      edge = add_n_gon(5,edge);
		      number_of_faces++;
		      new_length = length;
		      new_sequence[0] = number_of_faces-1;
		      new_sequence[1] = sequence[ENTRY_SUM((i),(2))]+1;
		      for(k=2; k<new_length-1; k++) new_sequence[k] = sequence[ENTRY_SUM((i),(k+1))];
		      if (length==2) new_sequence[new_length-1] = j+1;
		      else new_sequence[new_length-1] = j;
		      if (!construct_ipr||is_ipr)
			construct_notpc_patch(new_mark, new_sequence, 1, new_length);
		      /* Additional hexagon: */
		      if((nv+1 <= maxnv_cap)) {
			edge = delete_n_gon(edge);is_ipr = 1;
			edge = add_n_gon(6,edge);
			new_length = length+1; 
			new_sequence[0] = number_of_faces-1;
			new_sequence[1] = 0;
			new_sequence[2] = sequence[ENTRY_SUM((i),(2))]+1;
			for(k=3; k<new_length-1; k++) new_sequence[k] = sequence[ENTRY_SUM((i),(k))];
			if (length==2) new_sequence[new_length-1] = j+1;
			else new_sequence[new_length-1] = j;
			construct_notpc_patch(new_mark, new_sequence, 1, new_length);
		      }
		    }
		  }
		}
		edge = reduce(edge, number_of_faces);is_ipr=1;
	      }
	    }
	  }
	}
      }
    }

    /* Step pseudoconvex --> almost pseudoconvex; special cases */
    if ((length == 1) && (!construct_ipr||is_ipr)){
      last_patch(mark, sequence[1]);
    }
    if (length == 2){ 
      /* A complete row of hexagons + additional pentagon*/
      number_of_rows = sequence[0]; /* Necessary for the macro NEXT_ENTRY(i) */
      for (i=1;i<=2;i++){
	number_of_faces = sequence[i];
	if ((l>=(sequence[i]+1))&&(m>=sequence[NEXT_ENTRY(i)])){/* Optimisation*/
	  if ((2*number_of_faces+3+nv)<=maxnv_cap){
	    /* Information about pentagon in the m_part: */
#ifdef debug
	    edge = starts[NEXT_ENTRY(i)]->next;
	    for (i=0; i<sequence[NEXT_ENTRY(i)]; i++){
	      if (edge->pentagon_right){
		pentagon_m_part = 1;
		break;}
	      else edge = edge->invers->next->invers->next->next;
	    }
#endif
	    startedge = starts[i];
	    edge = operation(0,number_of_faces,1,startedge,&new_mark);
	    new_length = length;
	    new_sequence[0] = number_of_faces;
	    new_sequence[1] = 1;
	    if (!construct_ipr||is_ipr)
	      construct_notpc_patch(new_mark, new_sequence, sequence[NEXT_ENTRY(i)]+1, new_length);
	    if ((nv+1) <= maxnv_cap){
	      edge = delete_n_gon(edge); is_ipr=1;
	      edge = add_n_gon(6,edge);
	      new_sequence[1] = 0;
	      new_sequence[2] = 1;
	      new_length = length+1;
	      construct_notpc_patch(new_mark, new_sequence, sequence[NEXT_ENTRY(i)]+1, new_length);
	    }
	  edge = reduce(edge, number_of_faces+1);is_ipr=1;

	  }
	}
      }
    }
    if ((length == 3) && !sequence[1] && (m>=sequence[2]+4) && (l+sequence[3]>=2) && (nv + 2*number_of_faces + 2 <= maxnv_cap) ){
      /* add several faces!! */
      number_of_faces = sequence[3]+1;
      edge = operation(0,number_of_faces,0,starts[3], &new_mark);
      add_tail(edge, 1, new_mark, number_of_faces, sequence[2]+2);
      add_tail(edge, 0, new_mark, number_of_faces, sequence[2]+2);
      edge = reduce(edge, number_of_faces); 
    }
  }

  return(1);
}

/**************************** finish_patch ******************************/
/* Applied to almost pseudoconvex patches with exactly two convex edges */
int finish_patch(EDGE *mark, int sequence[], int m_bound  )
{
  EDGE *edge, *start, *end, *new_mark;
  int i,j;
  int number_of_faces, number_of_rows;
  if (split && level<split_level){
    if (rest%mod)return(0);
  }
  
  number_of_faces = sequence[0];
  number_of_rows = m - m_bound;
    if ((number_of_rows > 1) && !pentagon_m_part) return(0);
  if ((number_of_rows*(2*number_of_faces-number_of_rows+2)-1) + nv <= maxnv_cap ){
    start = new_mark = mark;  
    for (i=0; i<number_of_rows-1; i++){
      level++;
      edge = add_n_gon(6, start);
      start = edge->prev->invers->prev;
      for (j=0; j<number_of_faces-1; j++) edge = add_n_gon(6, edge);
      number_of_faces--;
    }
    new_mark = start;
    edge = operation_notpc(number_of_faces-1, 1, &new_mark);  
    if ((!construct_ipr||is_ipr)&&lm_patch_is_canonical(new_mark)) output_patch(new_mark);
    /* Reduction: */
    for (i=number_of_rows; i>=1; i--){
      end = edge->next->invers->next->invers->next;
      edge = reduce(edge,number_of_faces);
      number_of_faces++;
      edge = end;
    }
    is_ipr=1;
  }
  return(1);
}

/******************** add_tail *****************************/
/* Function which adds several additional faces in case that there is a 0--edge
   at the end of the row */
int add_tail(EDGE *last, int pentagon, EDGE *mark, int number_of_faces, int m_bound)
{
  EDGE *edge;
  int i,j;
  int additional_faces;
  int max_add;
  int new_sequence[7];
  EDGE *new_mark;
  EDGE *startedge;
  int new_length;

  new_mark = mark;
  max_add = (int)((l-number_of_faces-1)/2)+5; /* More additional faces would not 
				       lead to a valid patch */
  if (max_add > 0){
    startedge = last->prev->invers->prev;
    edge = startedge;
    additional_faces = 1;
    while((additional_faces <= max_add)&&( (pentagon && ((nv+additional_faces*4-1)<=maxnv_cap))||(!pentagon && ((nv+additional_faces*4)<=maxnv_cap)))){
      for(i=0; i<additional_faces-1; i++){
	edge = add_n_gon(6, edge);
	edge = edge->prev->invers->prev->prev->invers->prev;
      }
      if (pentagon) {
	edge = add_n_gon(5, edge);
	new_length = 3;
      }
      else {
	edge = add_n_gon(6, edge);   
	new_length = 4;
	new_sequence[2] = 0;
      }
      new_sequence[0] = number_of_faces + additional_faces-1;
      new_sequence[1] = 0;
      new_sequence[new_length-1] = additional_faces;
      if (!construct_ipr||is_ipr){
      construct_notpc_patch(new_mark, new_sequence, m_bound, new_length);}
      for(j=additional_faces; j>0; j--){ /* "reduce(  )" */
	edge = delete_n_gon(edge);
	edge = edge->next->invers->next->next->invers->next;
      }
      edge = edge->prev->invers->prev->prev->invers->prev;
      is_ipr = 1;
      additional_faces++;
    }
  }
  return(0);
}
/******************** last_patch ***************************/
/* This routine is used for pseudoconvex patches with exactly one convex edge. 
   In this case the remaining steps of construction are uniquely determined.
   The parameter x is the number of degree 3 vertices of the boundary.*/ 
int last_patch(EDGE *mark, int x)
{
  int i,j,number_of_rows,max_vertices,number_of_faces;
  EDGE *edge, *start;
  EDGE *new_mark;
  EDGE *end_edge;

  if (split && level<split_level){
    if (rest%mod)return(0);
  }

  new_mark = mark;
  /* normal case; i.e. the incomplete row starts in the middle of a row 
     ((x-m) steps forward) */
  if ((l==x) && (m!=x)){
    number_of_rows = m;
    if ((m*(m+2)-1)+nv <= maxnv_cap){
      start = mark;
      for (i=0; i<x-m; i++) start = start->next->invers->next->invers->next;
      number_of_faces = m-1; /* since the first face is added separately*/
      for (i=0; i<number_of_rows-1; i++) {
	edge = add_n_gon(6,start);
	start = edge->prev->invers->prev;
	for (j=0; j<number_of_faces; j++) edge = add_n_gon(6,edge);
	level++;
	number_of_faces--;
      }
      edge  = add_n_gon(5, start);
      level++;
      new_mark = edge;
      if ((!construct_ipr||is_ipr)&&lm_patch_is_canonical(new_mark)) 
	output_patch(new_mark);
      for(i=0; i<number_of_rows; i++){
	end_edge = edge->next->invers->next->invers->next;
	reduce(edge, i+1);
	edge = end_edge;
      }
    }
  }
  is_ipr=1;

  /* special case, where the incomplete row starts at the first convex edge 
     and ends somewhere in the middle */
  number_of_rows = m + l - x;
  max_vertices = 0;
  max_vertices=(number_of_rows*(l+x-m+2)-1);
  if  ((l<=x) && (number_of_rows  > 0) && (number_of_rows <= l) && (max_vertices+nv <= maxnv_cap)){
    edge = new_mark = mark;
    for(i=1;i<number_of_rows;i++){ 
      edge = operation_notpc(l-i+1,0,&new_mark);
    }
    edge = operation_notpc(l-number_of_rows,1,&new_mark); 
    if ((!construct_ipr||is_ipr)&&lm_patch_is_canonical(new_mark)) 
      output_patch(new_mark);
    for (i=number_of_rows; i>=1; i--){
      end_edge = edge->next->invers->next->invers->next;
      edge = reduce(edge,l-i+1);
      edge = end_edge;
    }
  }
  is_ipr=1;
  
  return(0);
}
/***************** initial_patch *************************/
/* Construction of the patches for the beginning. First a "first_face
   -gon" (5 or 6) is constructed, then "number_of_hexagons" many
   hexagons and a "last_face -gon" (5 or 6; without loss of generality
   5 only in case that first_face is also 5). The return-value is the
   marked edge. */

EDGE *initial_patch(int first_face, int number_of_hexagons, int last_face)
{
  EDGE *start;
  EDGE *mark;
  int i;
  
  start = make_n_gon(first_face,maxnv);
  for (i=0; i<number_of_hexagons; i++){
    start = add_n_gon(6, start);
    start = start->prev->invers->prev->prev->invers->prev;
  }
  start = add_n_gon(last_face,start);  
  if (last_face == 5)  mark = start->prev->invers->prev;
  else  mark = start->prev->invers->prev->prev->invers->prev;

  return(mark);
}
/************************ cap *****************************/
/*  Starting  */
int cap(EDGE *mark, int sequence[], EDGE *starts[])
{
  int i,j;
  int maxstart;
  int length = 6;

  if (m==0) maxstart = 1;/* Other starting patches could not be canonical */
  else maxstart = XMOD2(l+m); 
  
  for (i=0; i<maxstart; i++){  
    mark = initial_patch(6,i,6);
    compute_sequence(mark, sequence, starts,&length); 
    construct_patch(mark,sequence,starts,length); 
    mark = mark->next->invers->next->next->invers->next;
    for (j=0; j<=i; j++){ 
      mark = delete_n_gon(mark);
      mark = mark->next->invers->next->next->invers->next;
    }
  }

  for (i=0; i<maxstart; i++){
    mark = initial_patch(5,i,6); 
    compute_sequence(mark, sequence, starts, &length);
    construct_patch(mark,sequence,starts,length);
    mark = mark->next->invers->next->next->invers->next;
    for (j=0; j<=i; j++){ 
      mark = delete_n_gon(mark);
      mark = mark->next->invers->next->next->invers->next;
    }
  }
  for (i=0; i<maxstart; i++){
    if (i || !construct_ipr){
      mark = initial_patch(5,i,5);
      compute_sequence(mark, sequence, starts, &length);
      construct_patch(mark,sequence,starts,length);
      mark = mark->next->invers->next;
      for (j=0; j<=i; j++){ 
	mark = delete_n_gon(mark);
	mark = mark->next->invers->next->next->invers->next;
      }
    }
  }
  
  return(0);
}
/************** limit_vertices ********************/
int limit_vertices( )
{
  int i,limit=5;
  int max_vertices;

  for(i=5; i<= (l+m); i++) limit += (2*i+1);
  /* Upper bound for the number of vertices; due to the result of the diploma thesis
     of J. Greinus*/
#ifdef schranke
 {
   vertex_bound = l+m + m*(m+1) + l*(l+1) -20;
   /* A better bound for the vertices, but nor yet proven. */
   if (l<7)  max_vertices = (limit-1);
   max_vertices = (vertex_bound);
#else
   max_vertices = (limit-1);
#endif
   return(max_vertices);
}

/********************* init *********************************/
void init()
{
if (maxlabel==0) { fprintf(stderr,"maxlabel not yet set (still 0)... error\n");
                   exit(0);
                  }
if ((marks__v=(unsigned short *)malloc((maxlabel+1)*sizeof(unsigned short)))==NULL)
  { fprintf(stderr,"Cannot allocate space for the marks !\n");
    exit(0); }

if ((startedge_cgr=(EDGE **)malloc(maxlabel*sizeof(EDGE *)))==NULL)
  { fprintf(stderr,"Cannot allocate memory for startedges.\n"); exit(0); }
if ((number_cgr=(VERTEX *)malloc(maxlabel*sizeof(VERTEX)))==NULL)
   { fprintf(stderr,"Cannot allocate memory for numbers.\n"); exit(0); }

  init_new_paths(maxnv_cap);
  if (outputtube_length) outputtube_edge =  make_tube(maxnv_cap+1,outputtube_length-1);
  isomorphism_tube = make_tube(maxnv_cap+1, l+m); /* Tube which is necessary for the
						     isomorphism checking*/
}
/********************* main *********************************/
int main(int argc, char *argv[])
{
  EDGE *merke=NULL;
  struct tms TMS;
  int i,j,max_tubelength,savetime,log=0,verbose=0;
  int sequence[7];
  EDGE *starts[7];
  FILE *fil;
  char logfilename[25];
  char reset_symmetry_filter=0;

  if (argc<3) usage(argv[0]);
  /* The boundary parameters of the caps are necessary: */
  if (!isdigit(argv[1][0])) usage(argv[0]); l=atoi(argv[1]);
  if (!isdigit(argv[2][0])) usage(argv[0]); m=atoi(argv[2]);
  if ((l<0) || (m<0)) usage(argv[0]);
  if (l<m) { i=l; l=m; m=i; }

  /* List of further options : */
    for (i=3; i< argc; i++){
      switch( argv[i][0])
	{
	  /* Option "noout" : No output of graphs in planarcode, only the 
	     number of accepted structures */
	case 'n': { 
	  if (!(strcmp(argv[i], "noout"))){no_output = 1;}
	  else { fprintf(stderr,"Nonidentified option: %s \n",argv[i]); exit(0); }
	  break;
	}
	case 'i': { /* Option "ipr" : Only caps with isolated pentagons will be
		       constructed */
	  if (!(strcmp(argv[i], "ipr"))) {construct_ipr=1;}
	  else {fprintf(stderr,"Nonidentified option: %s \n",argv[i]); exit(0);}
	  break;
	}
	case 'C': { /* Group filter */
	  if (!reset_symmetry_filter){for (j=0; j<11; j++) gruppe[j]=0; 
	  reset_symmetry_filter=1;}
	  if (!(strcmp(argv[i], "C1"))){gruppe[C1] = 1;}
	  else if  (!(strcmp(argv[i], "C2"))){gruppe[C2] = 1;}
	  else if  (!(strcmp(argv[i], "C3"))){gruppe[C3] = 1;}
	  else if  (!(strcmp(argv[i], "C5"))){gruppe[C5] = 1;}
	  else if  (!(strcmp(argv[i], "C6"))){gruppe[C6] = 1;}
	  else if  (!(strcmp(argv[i], "Cs"))){gruppe[Cs] = 1;}
	  else if  (!(strcmp(argv[i], "C2v"))){gruppe[C2v] = 1;}
	  else if  (!(strcmp(argv[i], "C3v"))){gruppe[C3v] = 1;}
	  else if  (!(strcmp(argv[i], "C5v"))){gruppe[C5v] = 1;}
	  else if  (!(strcmp(argv[i], "C6v"))){gruppe[C6v] = 1;}
	  else {fprintf(stderr,"Nonidentified option: %s \n",argv[i]); exit(0);}
	  break;
	}	  
	case 'l': { /* Writing of a logfile */
	  if (!(strcmp(argv[i], "log"))){ 
	    log = 1;  
	    sprintf(logfilename,"tube_%d_%d.log",l,m);
	  }
	  else  { fprintf(stderr,"Nonidentified option: %s \n",argv[i]); exit(0); }
	  break;
	}
	case 's': { /* Option "split" : Splitting of the generation tree in depth 
		       split_level; only a part will be generated*/
	  if (!(strcmp(argv[i], "split"))){ 
	    if (!isdigit(argv[i+1][0]) ||(!isdigit(argv[i+2][0])) )usage(argv[0]);
	    else {
	      rest = atoi(argv[++i]); 
	      mod = atoi(argv[++i]);
	      if (rest >= mod ){fprintf(stderr,"Second parameter of 'split' has to be > than the first. \n"); exit(1);}
	      split = 1;
	    }
	  }
	  else  { fprintf(stderr,"Nonidentified option: %s \n",argv[i]); exit(0); }
	  break;
	}
	case 'v': { /* Option "verbose" : Information about groups, number of 
		       generated structures etc. will be given */
	  if (!(strcmp(argv[i], "verbose"))){ verbose = 1;}
	  else  { fprintf(stderr,"Nonidentified option: %s \n",argv[i]); exit(0); }
	  break;
	}
	case 't': { /* Option "tube" : a tube of length outputtube_length
		       will be glued to the generated caps*/
	  if (!(strcmp(argv[i++], "tube")))
	    {
	      if (!isdigit(argv[i][0]))usage(argv[0]);
	      else outputtube_length = atoi(argv[i]); 
	    }
	  else {fprintf(stderr,"Nonidentified option: %s \n",argv[i]); exit(0);}
	  break;
	}
	default :{usage(argv[0]);}
	}
    }
    if (log){ /* Name of the logfile, respecting ipr and split option */
      if (construct_ipr){
	if (!split) sprintf(logfilename,"tube_i.%d_%d.log",l,m);
	else sprintf(logfilename,"tube_i.%d_%d.%d_%d.log",l,m,rest,mod);
      }
      else {
	if (split) sprintf(logfilename,"tube_%d_%d.%d_%d.log",l,m,rest,mod);
	else sprintf(logfilename,"tube.%d_%d.log",l,m);
      }
    }
    split_level = (l+m)/3; /* Heuristic value which is reasonable for the depth 
			      of splitting */
    maxnv_cap = limit_vertices();  /* maximum number of vertices in a cap */
    max_tubelength = MAX((l+m),outputtube_length);
    maxnv = maxlabel = (2*max_tubelength*max_tubelength + maxnv_cap);  
                            /* 2*(l+m) boundary length; l+m tube length */

    if (log) {fil = fopen(logfilename,"w");
    
    }
    else fil = stderr;

  fprintf(fil,"Command:\n");
  for (i=0; i< argc; i++) fprintf(fil,"%s ",argv[i]); fprintf(fil,"\n");
  
  fprintf(fil,"Will generate caps with at most %d vertices.\n",maxnv_cap);
    init(); /* Needs the correct value of maxnv_cap */
    cap(merke, sequence, starts);

    times(&TMS);
    savetime=(unsigned int) TMS.tms_utime;
if (verbose) fprintf(fil,"Number of generated structures: %d\n",graph_counter);
if (split)fprintf(fil,"Part %d mod %d\n",rest,mod);
fprintf(fil,"Number of accepted caps: %d\n",cap_counter);
if (verbose) {fprintf(fil,"Ratio: %f\n",(float) graph_counter/cap_counter);
if (gruppe[C1])
  if (auts_statistic[1]) fprintf(fil,"With group C1: %d \n",auts_statistic[1]);
if (gruppe[C2])
  if (auts_statistic[2]) fprintf(fil,"With group C2: %d \n",auts_statistic[2]);
if (gruppe[C3])
  if (auts_statistic[3]) fprintf(fil,"With group C3: %d \n",auts_statistic[3]);
if (gruppe[C5])
  if (auts_statistic[5]) fprintf(fil,"With group C5: %d \n",auts_statistic[5]);
if (gruppe[C6])
  if (auts_statistic[6]) fprintf(fil,"With group C6: %d \n",auts_statistic[6]);
if (gruppe[Cs])
  if (auts_statistic_mirror[1]) 
    fprintf(fil,"With group Cs: %d \n",auts_statistic_mirror[1]);
if (gruppe[C2v])
  if (auts_statistic_mirror[2]) 
    fprintf(fil,"With group C2v: %d \n",auts_statistic_mirror[2]);
if (gruppe[C3v])
  if (auts_statistic_mirror[3]) 
    fprintf(fil,"With group C3v: %d \n",auts_statistic_mirror[3]);
if (gruppe[C5v]) 
  if (auts_statistic_mirror[5]) 
    fprintf(fil,"With group C5v: %d \n",auts_statistic_mirror[5]);
if (gruppe[C6v])
  if (auts_statistic_mirror[6]) 
    fprintf(fil,"With group C6v: %d \n",auts_statistic_mirror[6]);
}
#ifdef time_factor
fprintf(fil,"User CPU-time for cap generation: %.1f sec. \n",(double)savetime/time_factor);
#endif

#ifdef DEBUG
for (i=0; i< maxlevel+2; i++)fprintf(stderr,"knoten[%2d] = %d\n", i, knoten[i]);
#endif
/* fclose(fil); noch einfuegen? */

return 0;
}




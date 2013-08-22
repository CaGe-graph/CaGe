/* plantri_md6.c :  generate 3-connected planar triangulations with 
                maximum degree at most 6.
    
    It is necessary that either fullgen or buckygen be on the path,
    as it is started in a subprocess. fullgen is available from
    Gunnar Brinkmann, gunnar@mathematik.uni-bielefeld.de. buckygen
    is available from Jan Goedgebeur <Jan.Goedgebeur@UGent.be>.
    The variable FULLERENES determines which one is used; with
    buckgen as the default.

    Usage:  plantri_md6 [-aAghodufic#s#e#r#m#/#] n [outfile]

      n is the number of vertices (4-MAXN)
        
           This parameter can also be given as a range min-max for
           greater total efficiency.

      outfile is the output file  (missing or "-" means stdout)

      -o : Without this option, mirror images are treated as the same.  
	   With this option they are different unless there is a 
	   reflecting automorphism.  Implies -g.  In the case of -A
           (graph6 output), only one graph is written because graph6
           format loses the imbedding.  Therefore it is usually a waste
           of time to use -o and -A together.

      -u : Just count, don't write any graphs.

      -d : Write the dual graph instead (a 3-connected cubic planar graph).

      -a : Write in a human-readable ascii text format, instead
	   of the default planar_code format
	   
      -A : Write in graph6 format, instead of the default planar_code format.
	   -a -A -S are incompatible.

      -S : Write in sparse6 format, instead of the default planar_code format.
	   -a -A -S are incompatible.

      -h : Without -a or -A, suppress writing of header ">>planar_code<<".
	   With -A, force writing of header ">>graph6<<".
	   With -S, force writing of header ">>sparse6<<".

      -g : Provide correct group in all cases.  This is only important
	   if you use a plug-in that needs the group.  It causes a 
	   significant performance penalty.

      -f : Restrict to minimum degree at least 4.  This option is not
           very efficient, but better than filtering at the end.
           The minimum number of vertices becomes 6.
	   
      -x : Restrict to only degrees 4 and 6.  This option is not
           very efficient, but better than filtering at the end.
           The minimum number of vertices becomes 6.
	   
      -c# : Restrict to case # of fullerenes, where #=1,2,3.  The value
           -c0 (default) gives all cases together.  The single
           non-fullerene case (K4) goes in case 2. (fullgen only)

      -i : Make the fullerene generator write some information about the
           generation process -- once to a logfile and once to stderr.    

      -s# : Start from fullerenes with at least # faces (=vertices of the 
           triangulation).  The default is 12 -- the smallest number of
           faces in a fullerene.

      -e# : Start from fullerenes with at most # faces.  Default is n. 
           # must be a positive number.

        The generation is performed by adding edges to the duals of
        fullerenes, and also to the basic trangulation K4.  -s and -e
        can be used to split the starting graphs into classes.

	If neither -s nor -e are chosen, the complete generation is 
        performed -- including the generation of structures starting 
        from the K4.  In case at least one of the options is chosen, 
        the structures starting at K4 are generated if the e-value is 
        smaller than 12 and otherwise the part between (including) 
        the s-value and the e-value.

      -m#/# : The two integer arguments are res and mod, with
        0 <= res < mod.  The fullerene generation is divided into
        mod pieces and the res-th piece only is constructed.
	The case of starting at K4 belongs to piece 0.

      Switches can be given separately or concatenated.

   Authors:  Gunnar Brinkmann   gunnar@mathematik.uni-bielefeld.de
	     Brendan McKay      bdm@cs.anu.edu.au

---------

   Output formats: 

      The output for each graph consists of the number of vertices,
      then for each vertex a list of all its neighbours in clockwise
      order.  These neighbour lists are separated by a "separator",
      and terminated by a "terminator".

		             planar_code          ascii format
		              (default)            (with -a)

      file type               binary               text
      number of vertices      one byte          decimal + blank
      vertex names          bytes 1,2,3...        letters 'a','b',...
      separators              zero byte           comma ','
      terminator              zero byte           newline '\n'

      For example, the planar dual of the cube appears like this:

      planar_code: 
	 6 4 3 2 5 0 3 6 5 1 0 1 4 6 2 0 5 6 3 1 0 4 1 2 6 0 4 5 2 3 0
	 (One number per byte, no newlines).

      ascii format: 
	 6 dcbe,cfea,adfb,efca,dabf,debc
	 (Including the space after "6".  Followed by newline.)

      For planar_code, the standard header ">>planar_code<<"
      (without null or newline) is written at the start of the
      output unless the -h switch is given.

      Using -A, the graph6 format can be selected instead.  This is
      an ASCII format for general undirected graphs, and does not
      encode the imbedding.  It is described elsewhere (contact bdm).
      Similarly, -S selects sparse6 format (smaller for the dual).

---------

   plantri has a "plug-in" facility that enables you to examine
   triangulations as they are made, without the need for expensive i/o
   operations.  This can save a factor of two in some cases.

   If the preprocessor variable PLUGIN is defined at compile time, it
   is taken to be a file name enclosed in double quotes.  For example,
   in the Unix shell, this can be achieved with the switch 
   '-DPLUGIN="plugin.c"' (including all quotes) on the cc command.
   The contents of that file are included into this one.

   As each graph is made, a call is made like this:

      int FILTER(int nbtot, int nbop, int doflip)  

	nbtot = total number of canonical labellings
	nbop  = number of those which are orientation-preserving
	doflip = nonzero iff there is no orientation-reversing
		 automorphism and -o is given
	
	nbtot and nbop are only guaranteed correct if -g or -o is given.

   The procedure you provide can do anything except modify plantri data.  
   You can write the graph (to outfile, or to a file you open yourself),
   and/or you can collect statistics in global variables you declare
   yourself.  PLUGIN must return an int value.  A value of 0 means
   "don't write" and other values mean "write unless -u".

   In addition, procedure SUMMARY() is called at the end of execution.  
   This is to enable you to write information collected by FILTER().
   Usually summary information is written to msgfile, which is equal
   to stdout if an output file is specified, and to stderr if not.
   The normal plantri summary is written if SUMMARY() is called, but
   can be suppressed by setting the global variable dosummary = 0.

   Names other than FILTER and SUMMARY can be used just by defining
   those names as macros on the command line.

   Note that the PLUGIN file is included right into the text of this
   program, so all the information here is available.  For example,
    outfile = open file for writing graphs (unless -u)
    msgfile = open file for writing informational messages

   In addition, the plug-in code can define several optional features.
   (1) If the preprocessor variable PLUGIN_INIT is defined, it is 
       included after the input parameters are parsed but before any
       triangulations are generated.
   (2) If the preprocessor variable PRE_FILTER is defined, it is taken
       to be an expression which is evaluated for each intermediate
       triangulation (smaller than output size).  If the expression
       evaluates to 0, that branch of the tree is cut off (i.e. no
       descendants of the tested triangulation are generated).  
       Otherwise, computation proceeds as normal.  Use of this facility
       requires considerable knowledge of the working of the program.
   (3) If the preprocess variable PLUGIN_SWITCHES is defined, it is
       expanded at the place where command-line switches are parsed.
       This allows you to add additional switches.  For example

#define PLUGIN_SWITCHES else if (arg[j] == 'x') xswitch = TRUE;

       will add a -x option.  You must allocate space for xswitch yourself.

   The file maxdeg.c contains a detailed example of a plug-in.

---------

Counts (without -o):

		     -- mindegree --
 nv   ne   nf       3       4       5      total

  4    6    4       1       0       0         1
  5    9    6       1       0       0         1
  6   12    8       1       1       0         2
  7   15   10       4       1       0         5
  8   18   12       8       2       0        10
  9   21   14      11       4       0        15
 10   24   16      23       7       0        30
 11   27   18      34      10       0        44
 12   30   20      54      22       1        77
 13   33   22      83      32       0       115
 14   36   24     125      58       1       184
 15   39   26     174      92       1       267
 16   42   28     267     151       2       420
 17   45   30     365     227       3       595
 18   48   32     509     368       6       883
 19   51   34     706     530       6      1242
 20   54   36     963     805      15      1783
 21   57   38    1270    1158      17      2445
 22   60   40    1708    1695      40      3443
 23   63   42    2204    2373      45      4622
 24   66   44    2876    3354      89      6319
 25   69   46    3695    4595     116      8406
 26   72   48    4708    6340     199     11247
 27   75   50    5925    8480     271     14676
 28   78   52    7491   11417     437     19345
 29   81   54    9255   15049     580     24884
 30   84   56   11463   19832     924     32219
 31   87   58   14083   25719    1205     41007
 32   90   60   17223   33258    1812     52293
 33   93   62   20857   42482    2385     65724
 34   96   64   25304   54184    3465     82953
 35   99   66   30273   68271    4478    103022
 36  102   68   36347   85664    6332    128343
 37  105   70   43225  106817    8149    158191
 38  108   72   51229  132535   11190    194954
 39  111   74   60426  163194   14246    237866
 40  114   76   71326  200251   19151    290728
 41  117   78   83182  244387   24109    351678
 42  120   80   97426  296648   31924    425998
 43  123   82  113239  358860   39718    511817
 44  126   84  131425  431578   51592    614595
 45  129   86  151826  517533   63761    733120
 46  132   88  175302  617832   81738    874872
 47  135   90  200829  735257   99918   1036004
 48  138   92  231042  870060  126409   1227511
 49  141   94  263553 1029114  153493   1446160
 50  144   96  300602 1209783  191839   1702224

From here on, totals don't include mindegree 3.

 51  147   98     -   1420472  231017   1651489
 52  150  100     -   1659473  285914   1945387
 53  153  102     -   1937509  341658   2279167
 54  156  104     -   2249285  419013   2668298
 55  139  106     -   2612410  303174   2915584
 56  142  108     -   2677727  604217   3281944

--------------------------------------------------------

Size limits:

The space used by plantri is O(n^2).  Apart from that, the only
practical limits on MAXN (the maximum permitted number of vertices)
is determined by the limits imposed by the output syntax.

The following table gives the largest legal MAXN value.
  
  Switches:     none    -d     -a    -ad      -A    -Ad     -u
  Output:       planar_code      ascii         graph6      none
               primal  dual   primal dual   primal dual    
  MAXN limit:   255    129      99     51    4094  2049   no limit

The limits for ascii code could be raised to 114,59 easily.

Change History:

       19-Nov-1996 : created from plantri 1.0.5
       12-Mar-1997 : trivial changes
       13-Sep-2011 : allow buckgen as well as fullgen
 
**************************************************************************/

#include <stdio.h> 

#if __STDC__
#include <stdlib.h>
#endif

#define CPUTIME 1          /* Whether to measure the cpu time or not */

#if CPUTIME
#include <sys/times.h>
#include <time.h>
#if !defined(CLK_TCK) && !defined(_SC_CLK_TCK)
#include <unistd.h>
#endif
#if !defined(CLK_TCK) && defined(_SC_CLK_TCK)
#define CLK_TCK sysconf(_SC_CLK_TCK)
#endif
#if !defined(CLK_TCK) && defined(CLOCKS_PER_SEC)
#define CLK_TCK CLOCKS_PER_SEC
#endif
#if !defined(CLK_TCK)
#define CLK_TCK 60     /* If the CPU time stated by the program appears
                       to be out by a constant ratio, the most likely
                       explanation is that the code got to this point but
                       60 is the wrong guess.  Another common value is 100. */
#endif
#endif

#ifndef FULLERENES
#define FULLERENES 1  /* 0 = use fullgen; 1 = use buckygen */
#endif
#ifndef STATS
#define STATS 1  /* nonzero = write some statistics */
#endif

#ifndef MAXN
#define MAXN (400/2+2)     /* the maximum number of vertices; see above */
#endif
#define MAXE (6*MAXN-12)   /* the maximum number of oriented edges */
#define MAXF (2*MAXN-4)    /* the maximum number of faces */

typedef struct e /* The data type used for edges */
{ 
	int start;         /* vertex where the edge starts */
	int end;           /* vertex where the edge ends */ 
	int rightface;     /* face on the right side of the edge
			      note: only valid if find_dual() called */
	struct e *prev;    /* previous edge in clockwise direction */
	struct e *next;    /* next edge in clockwise direction */
	struct e *invers;  /* the edge that is inverse to this one */
	int mark,index;    /* two ints for temporary use */
} EDGE;

#define FALSE 0
#define TRUE  1

/* Global variables */

static FILE *outfile;      /* output file for graphs */
static FILE *msgfile;      /* file for informational messages */

static int minnv,maxnv;    /* orders of output graphs */

static int aswitch,
	   Aswitch,
	   Sswitch,
	   hswitch,
	   dswitch,
	   gswitch,
	   oswitch,
	   uswitch,
	   cswitch,
           iswitch,
           sswitch,
           eswitch,
           esswitch,
           mswitch,
	   fswitch,        /* presence of command-line switches */
           xswitch;
extern int errno;
static int dosummary;      /* used by plugin */
static char *cmdname;      /* points to arg[0] */

/* The variables below are used at each level of the iteration,
   updating and restoring as we move up and down the search tree */

static int nv;             /* number of vertices; they are 0..nv-1 */
static int ne;             /* number of directed edges (6*nv-12) */

static EDGE edges[6+32*MAXN];
#define init_edge edges
#define STAR3(n) (edges + 6 + ((n)<<3))
#define STAR4(n) (edges + 6 + 8*MAXN + ((n)<<3))
#define STAR5(n) (edges + 6 + 16*MAXN + ((n)<<4))

static int degree[MAXN];   /* the degrees of the vertices */
static EDGE *firstedge[MAXN]; /* pointer to arbitrary edge out of vertex i. */
  /* This pointer may change during the run, so all one can rely on is that
     at any point it is "some" edge out of i */

static EDGE *facestart[MAXF]; /* an edge in the clockwise orientation of
			         each face.  Only valid when computed. */

static EDGE *numbering[2*MAXE][MAXE]; 
  /* holds numberings produced by canon() */

#define PCODE ">>planar_code<<"
#define PCODELEN (sizeof(PCODE)-1)    /* "-1" to avoid the null */
#define G6CODE ">>graph6<<"
#define G6CODELEN (sizeof(G6CODE)-1)    /* "-1" to avoid the null */
#define S6CODE ">>sparse6<<"
#define S6CODELEN (sizeof(S6CODE)-1)    /* "-1" to avoid the null */

#if FULLERENES==0
#define SWITCHES "[-aASghodufic#e#s#m#/#]"
#else
#define SWITCHES "[-aASghodufie#s#m#/#]"
#endif

/* The program is so fast that the count of output graphs can quickly
   overflow a 32-bit integer.  Therefore, we use two long values
   for each count, with a ratio of 10^9 between them.  The macro
   ADDBIG adds a small number to one of these big ones.  The macro
   PRINTBIG prints one in decimal. */

typedef struct
{
    long hi,lo;
} bigint;

#define ZEROBIG(big) big.hi = big.lo = 0L
#define ISZEROBIG(big) (big.lo == 0 && big.hi == 0)
#define ADDBIG(big,extra) if ((big.lo += (extra)) >= 1000000000L) \
	{ ++big.hi; big.lo -= 1000000000L;}
#define PRINTBIG(file,big) if (big.hi == 0) \
 fprintf(file,"%ld",big.lo); else fprintf(file,"%ld%09ld",big.hi,big.lo)
#define BIGTODOUBLE(big) (big.hi * 1000000000.0 + big.lo)

static bigint nout;          /* counter of output graphs */

#if STATS    /* optional statistics collection */
static bigint numrooted;  /* rooted maps */
static bigint ntriv;      /* counter of those with trivial groups 
				   (an upper bound only without -o) */
static unsigned long nummindeg[4][MAXN+1]; /* count according to min degree */
#endif

/**************************************************************************/
/* Routines for extending and reducing a triangulation.
   General principle:  extendx(e); reducex(e)  will extend by one 
   vertex of degree x (x=3,4,5) then reduce it to the original graph.  
   The final graph is exactly the as the original (including pointer values) 
   except that possibly the values of firstedge[] might be different.
*/

static void 
extend3(EDGE *e)

/* inserts a vertex with valence 3 in the triangle on the right hand
   side (->next direction) of the edge e */
{
	register EDGE *work1, *work2, *work3;

	work1 = STAR3(nv);
	work2 = work1+1;
	work3 = work2+1;
	firstedge[nv] = work3+1;

/* work1 starts at the beginning of e: */

	work1->start = work1->invers->end = e->start;
	e->next->prev = work1;
	work1->next = e->next;
	work1->prev = e;
	e->next = work1;
	(degree[e->start])++;

/* Now go one edge further around the triangle and the same once more */

	e = e->invers->prev;
	
	work2->start = work2->invers->end = e->start;
	e->next->prev = work2;
	work2->next = e->next;
	work2->prev = e;
	e->next = work2;
	(degree[e->start])++;

	e = e->invers->prev;

	work3->start = work3->invers->end = e->start;
	e->next->prev = work3;
	work3->next = e->next;
	work3->prev = e;
	e->next = work3;
	(degree[e->start])++;

	degree[nv] = 3;

/* Now I have 6 edges and one vertex more */

	ne += 6; 
	nv++;
}

/****************************************/

static void 
reduce3(EDGE *e)

/* deletes a vertex with valence 3 in the triangle on the right hand side
 (->next-direction) of the edge e. It is not checked whether the vertex
 really has valence 3 -- this has to be made sure in advance      */
{
/* It might be that one of the edges leading to the new vertex now is
   an entry of firstedge[] */

/*if (firstedge[e->start]==e->next) would take too much time, so just*/ 

	firstedge[e->start] = e;
	e->next = e->next->next; e->next->prev = e; 
	(degree[e->start])--;
	e = e->invers;

	firstedge[e->start] = e;
/* Now delete on the ->prev side */
	e->prev = e->prev->prev; e->prev->next = e; 
	(degree[e->start])--;
	e = e->prev->invers;

	firstedge[e->start] = e;
/* Again on the ->prev side: */
	e->prev = e->prev->prev; e->prev->next = e; 
	(degree[e->start])--;

	nv--;  
	ne -= 6;
}

/************************************************************************/

static void 
extend4(EDGE *e, EDGE *list[])

/* Deletes e->next and its inverse and puts a valence 4 vertex into the
   resulting square.
   In list[0..1] the deleted edges are stored. This list must be handed 
   to reduce4() */
{
	register EDGE *work1, *work2, *work3, *work4;

	list[0] = e->next; list[1] = e->next->invers;

	work1 = STAR4(nv);
	work2 = work1+1;
	work3 = work2+1;
	work4 = work3+1;
	firstedge[nv] = work4+1;;

	firstedge[e->start] = e;
/* make sure firstedge points at a valid edge afterwards */

/* work1 starts at the beginning of e: */

	work1->start = work1->invers->end = e->start;
	work1->next = e->next->next;
	work1->next->prev = work1;
	work1->prev = e;
	e->next = work1;
/* the degree of e->start doesn't change */

/* Now go one edge further around the square: */

	e = e->invers->prev;

	work2->start = work2->invers->end = e->start;
	e->next->prev = work2;
	work2->next = e->next;
	work2->prev = e;
	e->next = work2;
	(degree[e->start])++;

/* Now we have one edge to jump about again: */
	e = e->invers->prev->prev;

	firstedge[e->start] = e;
/* Again an edge is deleted... */

	work3->start = work3->invers->end = e->start;
	work3->next = e->next->next;
	work3->next->prev = work3;
	work3->prev = e;
	e->next = work3;
/* the degree of e->start doesn't change */

/* Now go again one edge further around the square: */

	e = e->invers->prev;

	work4->start = work4->invers->end = e->start;
	e->next->prev = work4;
	work4->next = e->next;
	work4->prev = e;
	e->next = work4;
	(degree[e->start])++;

	degree[nv] = 4;

/* Now I have 6 edges and one vertex more */

	ne += 6; 
	nv++;
}

/**************************/

static void 
reduce4(EDGE *e, EDGE *list[])

/* The inverse operation to extend4().
   Deletes the vertex with valence 4 in the triangle on the right hand side
   (->next-direction) of the edge e that is not contained in e. It is not 
   checked whether the vertex really has valence 4 -- this has to be made sure 
   in advance. The vector list[] must contain the edges deleted before. 
   It might be that one of the edges leading to the new vertex now is
   an entry of firstedge[] */
{

	firstedge[e->start] = e;

	list[0]->next->prev = list[0];
	e->next = list[0];
	e = e->invers;

	firstedge[e->start] = e;
/* Now delete on the ->prev side */
	e->prev = e->prev->prev; e->prev->next = e; 
	(degree[e->start])--;
	e = e->prev->invers;

	firstedge[e->start] = e;
/* Again on the ->prev side: */
	list[1]->prev->next = list[1];
	e->prev = list[1];

	e = list[1]->prev->invers;
	firstedge[e->start] = e;
	e->prev = e->prev->prev; e->prev->next = e; 
	(degree[e->start])--;

	nv--;  
	ne -= 6;
}

/**********************************************************************/

static int 
testcanon(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->next" direction, an automorphism or even a better representation 
   can be found. Returns 0 for failure, 1 for an automorphism and 2 for 
   a better representation.  This function exits as soon as a better 
   representation is found. A function that computes and returns the 
   complete better representation can work pretty similar.*/
{
	EDGE *temp, *run;  
	EDGE *startedge[MAXN]; /* startedge[i] is the starting edge for 
				exploring the vertex with the number i+1 */
	int number[MAXN], i;   /* The new numbers of the vertices, starting 
				at 1 in order to have "0" as a possibility to
				mark ends of lists and not yet given numbers */
	int last_number, actual_number, vertex, col;

	for (i = 0; i < nv; i++) number[i] = 0;

	number[givenedge->start] = 1; 
	number[givenedge->end] = 2;
	actual_number = 1;
	last_number = 2;
	temp = givenedge;
	startedge[1] = givenedge->invers;

/* A representation is a clockwise ordering of all neighbours ended with a 0.
   The neighbours are numbered in the order that they are reached by the BFS 
   procedure. Every number is preceded by the colour of the vertex.
   Since every representation starts with "2" and the same colour, we do not 
   have to note that. Every first entry in a new clockwise ordering (and its 
   colour) is determined by the entries before (the first time it occurs in 
   the list to be exact), so this is not given either. 
   The K4 could be denoted  c0 3 c1 4 0 c1 4 c0 3 0 c3 2 c1 4 0 c0 3 c3 2 0 
   if c0 is the colour of vertex 3, c1 that of vertex 4 and c3 that of 
   vertex 2. Note that the colour of vertex 1 is -- by definition -- always 
   the smallest one */

	while (last_number < nv)
	{  
   	    for (run = temp->next; run != temp; run = run->next)
	/* this loop marks all edges around temp->origin. */
	      { vertex = run->end;
	  	col = colour[vertex];
	  	if (col > (*representation)) return(0);
	  	if (col < (*representation)) return(2);
	  	representation++;
	  	if (!number[vertex]) { startedge[last_number] = run->invers;
				last_number++; number[vertex] = last_number; }
	  	vertex = number[vertex];
	  	if (vertex > (*representation)) return(0);
	  	if (vertex < (*representation)) return(2);
	  	representation++;
	      }
   /* check whether representation[] is also at the end of a cyclic list */
   	    if ((*representation) != 0) return(2); 
   	    representation++;
   /* Next vertex to explore: */
   	    temp = startedge[actual_number];  actual_number++; 
	}

	while (actual_number <= nv) 
			/* Now we know that all numbers have been given */
	{  
   	    for (run = temp->next; run != temp; run = run->next)
	/* this loop marks all edges around temp->origin. */
	      { vertex = run->end;
	    	col = colour[vertex];
	    	if (col > (*representation)) return(0);
	    	if (col < (*representation)) return(2);
	    	representation++;
	    	vertex = number[vertex];
	    	if (vertex > (*representation)) return(0);
	    	if (vertex < (*representation)) return(2);
	    	representation++;
	      }
   /* check whether representation[] is also at the end of a cyclic list */
     	    if ((*representation) != 0) return(2); 
   	    representation++;
   /* Next vertex to explore: */
   	    temp = startedge[actual_number];  actual_number++; 
 	}

	return(1);
}

/*****************************************************************************/

static int 
testcanon_mirror(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->prev" direction, an automorphism or even a better representation can 
   be found. Comments see testcanon -- it is exactly the same except for 
   the orientation */
{
	EDGE *temp, *run;  
	EDGE *startedge[MAXN]; /* startedge[i] is the starting edge for 
			       exploring the vertex with the number i+1 */
	int number[MAXN], i; /* The new numbers of the vertices, starting 
				at 1 in order to have "0" as a possibility to
		                mark ends of lists and not yet given numbers */
	int last_number, actual_number, vertex, col;

	for (i = 0; i < nv; i++) number[i] = 0;

	number[givenedge->start] = 1; 
	number[givenedge->end] = 2;
	actual_number = 1;
	last_number = 2;
	temp = givenedge;
	startedge[1] = givenedge->invers;

/* A representation is a clockwise ordering of all neighbours ended with a 0.
   The neighbours are numbered in the order that they are reached by the BFS 
   procedure. Every number is preceded by the colour of the vertex.
   Since every representation starts with "2" and the same colour, we do not 
   have to note that. Every first entry in a new clockwise ordering (and its 
   colour) is determined by the entries before (the first time it occurs in 
   the list to be exact), so this is not given either. 
   The K4 could be denoted  c0 3 c1 4 0 c1 4 c0 3 0 c3 2 c1 4 0 c0 3 c3 2 if
   c0 is the colour of vertex 3, c1 that of vertex 4 and c3 that of vertex 2. 
   Note that the colour of vertex 1 is -- by definition -- always the 
   smallest one */

	while (last_number < nv)
	{  
   	    for (run = temp->prev; run != temp; run = run->prev)
	/* this loop marks all edges around temp->origin. */
	      { vertex = run->end;
	  	col = colour[vertex];
	  	if (col > (*representation)) return(0);
	  	if (col < (*representation)) return(2);
	  	representation++;
	  	if (!number[vertex]) { startedge[last_number] = run->invers;
				 last_number++; number[vertex] = last_number; }
	  	vertex = number[vertex];
	  	if (vertex > (*representation)) return(0);
	  	if (vertex < (*representation)) return(2);
	  	representation++;
	      }
	      
   /* check whether representation[] is also at the end of a cyclic list */
   	    if ((*representation) != 0) return(2); 
   	    representation++;
   /* Next vertex to explore: */
   	    temp = startedge[actual_number];  actual_number++; 
	}

	while (actual_number <= nv) 
			  /* Now we know that all numbers have been given */
	{  
	    for (run = temp->prev; run != temp; run = run->prev)
	/* this loop marks all edges around temp->origin. */
	      { vertex = run->end;
	  	col = colour[vertex];
	  	if (col > (*representation)) return(0);
	  	if (col < (*representation)) return(2);
	  	representation++;
	  	vertex = number[vertex];
	  	if (vertex > (*representation)) return(0);
	  	if (vertex < (*representation)) return(2);
	  	representation++;
	      }
   /* check whether representation[] is also at the end of a cyclic list */
   	    if ((*representation) != 0) return(2); 
   	    representation++;
   /* Next vertex to explore: */
   	    temp = startedge[actual_number];  actual_number++; 
	}

	return(1);
}

/****************************************************************************/
 
static void
testcanon_first_init(EDGE *givenedge, int representation[], int colour[])
 
/* Tests whether starting from a given edge and constructing the code in
   "->next" direction, an automorphism or even a better representation can 
   be found. A better representation will be completely constructed and 
   returned in "representation".  It works pretty similar to testcanon except 
   for obviously necessary changes, so for extensive comments see testcanon */
{
	register EDGE *run;
	register vertex;
	EDGE *temp;  
	EDGE *startedge[MAXN]; 
	int number[MAXN], i; 
	int last_number, actual_number;
 
	for (i = 0; i < nv; i++) number[i] = 0;
 
	number[givenedge->start] = 1; 
	number[givenedge->end] = 2;
	actual_number = 1;
	last_number = 2;
	temp = givenedge;
	startedge[1] = givenedge->invers;
 
	while (last_number < nv)
	{  
	    for (run = temp->next; run != temp; run = run->next)
	      { vertex = run->end;
		if (!number[vertex]) { startedge[last_number] = run->invers;
		                 last_number++; number[vertex] = last_number; }
		*representation = colour[vertex]; representation++;
		*representation = number[vertex]; representation++;
	      }
	    *representation = 0;
	    representation++;
	    temp = startedge[actual_number];  actual_number++;
	}

	while (actual_number <= nv) 
	{  
	    for (run = temp->next; run != temp; run = run->next)
	      { vertex = run->end; 
		*representation = colour[vertex]; representation++;
		*representation = number[vertex]; representation++;
	      }
	    *representation = 0;
	    representation++;
	    temp = startedge[actual_number];  actual_number++;
	}

	return;
}

/****************************************************************************/

static int 
testcanon_init(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->next" direction, an automorphism or even a better representation can 
   be found. A better representation will be completely constructed and 
   returned in "representation".  It works pretty similar to testcanon except 
   for obviously necessary changes, so for extensive comments see testcanon */
{
	register EDGE *run;
	register int col, vertex;
	EDGE *temp;  
	EDGE *startedge[MAXN]; 
	int number[MAXN], i; 
	int better = 0; /* is the representation already better ? */
	int last_number, actual_number;

	for (i = 0; i < nv; i++) number[i] = 0;

	number[givenedge->start] = 1; 
	number[givenedge->end] = 2;
	actual_number = 1;
	last_number = 2;
	temp = givenedge;
	startedge[1] = givenedge->invers;

	while (last_number < nv)
	{  
   	    for (run = temp->next; run != temp; run = run->next)
	      { vertex = run->end;
	  	col = colour[vertex];
	  	if (!number[vertex]) { startedge[last_number] = run->invers;
				 last_number++; number[vertex] = last_number; }
	  	if (better)
	    	{ *representation = col; representation++;
	      	  *representation = number[vertex]; representation++; }
	  	else
	    	{
	      	    if (col > (*representation)) return(0);
	            if (col < (*representation)) 
		    { better = 1; *representation = col;
			     	   representation++; 
			          *representation = number[vertex]; 
				   representation++; }
	            else
		    {
		        representation++;
		        vertex = number[vertex];
		        if (vertex > (*representation)) return(0);
		        if (vertex < (*representation)) { better = 1;  
						   *representation = vertex; }
		        representation++;
		    }
	        }
	      }
   	    if ((*representation) != 0) { better = 1; *representation = 0; }
   	    representation++;
   	    temp = startedge[actual_number];  actual_number++;
 	}

	while (actual_number <= nv) 
	{  
	    for (run = temp->next; run != temp; run = run->next)
	      { vertex = run->end; 
	  	col = colour[vertex];
	  	if (better)
	    	{ *representation = col; representation++;
	    	  *representation = number[vertex]; representation++; }
	        else
	        {
	            if (col > (*representation)) return(0);
	            if (col < (*representation)) 
		    { better = 1; *representation = col;
				   representation++; 
			          *representation = number[vertex]; 
				   representation++; }
	      	    else
		    {
		  	representation++;
		  	vertex = number[vertex];
		  	if (vertex > (*representation)) return(0);
		  	if (vertex < (*representation)) { better = 1;  
						   *representation = vertex; }
		  	representation++;
		    }
	        }
	      }
   	    if ((*representation) != 0) { better = 1; *representation = 0; }
   	    representation++;
   	    temp = startedge[actual_number];  actual_number++;
	}

	if (better) return(2);
	return(1);
}

/****************************************************************************/

static int 
testcanon_mirror_init(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->prev" direction, an automorphism or even a better representation can 
   be found. A better representation will be completely constructed and 
   returned in "representation".  It works pretty similar to testcanon except 
   for obviously necessary changes, so for extensive comments see testcanon */
{
	EDGE *temp, *run;  
	EDGE *startedge[MAXN]; 
	int number[MAXN], i; 
	int better = 0; /* is the representation already better ? */
	int last_number, actual_number, vertex, col;


	for (i = 0; i < nv; i++) number[i] = 0;

	number[givenedge->start] = 1; 
	number[givenedge->end] = 2;
	actual_number = 1;
	last_number = 2;
	temp = givenedge;
	startedge[1] = givenedge->invers;

	while (last_number < nv)
	{  
	    for (run = temp->prev; run != temp; run = run->prev)
	      { vertex = run->end;
	  	col = colour[vertex];
	  	if (!number[vertex]) { startedge[last_number] = run->invers;
				 last_number++; number[vertex] = last_number; }
	  	if (better)
	        { *representation = col; representation++;
	          *representation = number[vertex]; representation++; }
	  	else
	        {
	      	    if (col > (*representation)) return(0);
	      	    if (col < (*representation)) 
		    { better = 1; *representation = col;
				   representation++; 
				  *representation = number[vertex]; 
				   representation++; }
	            else
		    {
		        representation++;
		  	vertex = number[vertex];
		  	if (vertex > (*representation)) return(0);
		  	if (vertex < (*representation)) { better = 1;  
						   *representation = vertex; }
		  	representation++;
		    }
	        }
	      }
   	    if ((*representation) != 0) { better = 1; *representation = 0; }
   	    representation++;
   	    temp = startedge[actual_number];  actual_number++;
	}

	while (actual_number <= nv) 
	{  
   	    for (run = temp->prev; run != temp; run = run->prev)
	      { vertex = run->end; 
	  	col = colour[vertex];
	  	if (better)
	    	{ *representation = col; representation++;
	          *representation = number[vertex]; representation++; }
	        else
	        {
	            if (col > (*representation)) return(0);
	            if (col < (*representation)) 
		    { better = 1; *representation = col;
				   representation++; 
				  *representation = number[vertex]; 
				   representation++; }
	      	    else
		    {
		        representation++;
		        vertex = number[vertex];
		        if (vertex > (*representation)) return(0);
		        if (vertex < (*representation)) { better = 1;  
						   *representation = vertex; }
		        representation++;
		    }
	        }
	      }
	   if ((*representation) != 0) { better = 1; *representation = 0; }
	   representation++;
	   temp = startedge[actual_number];  actual_number++;
	}

	if (better) return(2);
	return(1);
}

/****************************************************************************/

static void
construct_numb(EDGE *givenedge, EDGE *numbering[])

/* Starts at givenedge and writes the edges in the well defined order 
   into the list.  Works like testcanon. Look there for comments. */
{
	EDGE *temp, **tail, **limit, *run;  
	EDGE *startedge[MAXN]; /* startedge[i] is the starting edge for 
  				 exploring the vertex with the number i+1 */
	int number[MAXN], i; /* The new numbers of the vertices, starting 
				at 1 in order to have "0" as a possibility to
    				mark ends of lists and not yet given numbers */
	int last_number, actual_number, vertex;


	for (i = 0; i < nv; i++) number[i] = 0;

	tail = numbering; /* The first entry of the numbering list */
	limit = numbering+ne-1;  /* Last valid entry of the numbering list */

	number[givenedge->start] = number[givenedge->end] = 1;
	actual_number = 1;
	last_number = 2;
	temp = *tail = givenedge;
	startedge[1] = givenedge->invers;

	while (last_number < nv)
	{  
   	    for (run = temp->next; run != temp; run = run->next)
	/* this loop marks all edges around temp->origin. */
	      { vertex = run->end;
	  	if (!number[vertex]) { startedge[last_number] = run->invers;
				 last_number++; number[vertex] = 1; }
	  	tail++; *tail = run; 
	      }
   	    if (tail != limit)
     	    {
       		tail++;
       		*tail = temp = startedge[actual_number];  actual_number++; }
 	}

	while (tail != limit) 
			/* Now we know that all numbers have been given */
	{  
   	    for (run = temp->next; run != temp; run = run->next)
	/* this loop marks all edges around temp->origin. */
	      { tail++; *tail = run; }
   	    if (tail != limit)
     	    { 
       /* Next vertex to explore: */
       	        tail++;
	       *tail = temp = startedge[actual_number];  actual_number++; }
 	}

	return;
}

/****************************************************************************/

static void 
construct_numb_mirror(EDGE *givenedge, EDGE *numbering[])

/* Starts at givenedge and writes the edges in the well defined order 
   into the list.  Works like testcanon. Look there for comments.  */
{
	EDGE *temp, **tail, **limit, *run;  
	EDGE *startedge[MAXN]; /* startedge[i] is the starting edge for 
				 exploring the vertex with the number i+1 */
	int number[MAXN], i; /* The new numbers of the vertices, starting 
			       at 1 in order to have "0" as a possibility to 
			       mark ends of lists and not yet given numbers */
	int last_number, actual_number, vertex;

	for (i = 0; i < nv; i++) number[i] = 0;

	tail = numbering; /* The first entry of the numbering list */
	limit = numbering+ne-1;  /* Last valid entry of the numbering list */

	number[givenedge->start] = number[givenedge->end] = 1;
	actual_number = 1;
	last_number = 2;
	temp = *tail = givenedge;
	startedge[1] = givenedge->invers;

	while (last_number < nv)
	{  
   	    for (run = temp->prev; run != temp; run = run->prev)
	/* this loop marks all edges around temp->origin. */
	      { vertex = run->end;
	        if (!number[vertex]) { startedge[last_number] = run->invers;
				 last_number++; number[vertex] = 1; }
	  	tail++; *tail = run; 
	      }
	    if (tail != limit)
     	    {
       	        tail++;
       		*tail = temp = startedge[actual_number];  actual_number++; }
 	}

	while (tail != limit) 
			/* Now we know that all numbers have been given */
	{  
   	    for (run = temp->prev; run != temp; run = run->prev)
	/* this loop marks all edges around temp->origin. */
	      { tail++; *tail = run; }
   	    if (tail != limit)
     	    { 
       /* Next vertex to explore: */
       	        tail++;
       		*tail = temp = startedge[actual_number];  actual_number++; }
 	}

	return;
}

/****************************************************************************/

static int 
canon(int colour[], EDGE *can_numberings[][MAXE], 
      int *num_can_numberings, int *num_can_numberings_or_pres)

/* Checks whether the last vertex (number: nv-1) is canonical or not. 
   Returns 1 if yes, 0 if not. One of the criterions a canonical vertex 
   must fulfill, is that its colour is minimal. If the vertex with minimal
   colour is not one with minimal valence, startlist and startlist_last
   may be too small.
   A possible starting edge for the construction of a representation is 
   one with lexicographically minimal colour pair (start,INT_MAX-end).
   In can_numberings[][] the canonical numberings are stored as sequences 
   of oriented edges.  For every 0 <= i,j < *num_can_numberings and every 
   0 <= k < ne the edges can_numberings[i][k] and can_numberings[j][k] can 
   be mapped onto each other by an automorphism. The first 
   *num_can_numberings_or_pres numberings are orientation preserving while 
   the rest is orientation reversing.

   In case of only 1 automorphism, in can_numberings[0][0] the "canonical" 
   edge is given.  It is one edge emanating at the canonical vertex. The 
   rest of the numbering is not given. 

   In case of nontrivial automorphisms, can[0] starts with a list of edges 
   adjacent to nv-1. In case of an orientation preserving numbering the deges 
   are listed in ->next direction, otherwise in ->prev direction.

   Works OK if at least one vertex has valence >= 3. Otherwise some numberings 
   are computed twice, since changing the orientation (the cyclic order around 
   each vertex) doesn't change anything */
{
	int i, last_vertex, test;
	int minstart, maxend; /* (minstart,maxend) will be the chosen colour 
				pair of an edge */
	EDGE *startlist_last[5], *startlist[5*MAXN], *run, *end;
	int list_length_last, list_length;
	int representation[2*MAXE+MAXN];
	EDGE *numblist[MAXE], *numblist_mirror[MAXE]; /* lists of edges where 
				starting gives a canonical representation */
	int numbs = 1, numbs_mirror = 0;

	last_vertex = nv-1;
	minstart = colour[last_vertex];

/* determine the smallest possible end for the vertex "last_vertex" */

	list_length_last = 1; startlist_last[0] = end = firstedge[last_vertex];
	maxend = colour[end->end];

	for (run = end->next; run != end; run = run->next)
  	  { if (colour[run->end] > maxend)
	    { startlist_last[0] = run; 
	      list_length_last = 1; maxend = colour[run->end];}
	    else if (colour[run->end] == maxend)
	    { startlist_last[list_length_last] = run; list_length_last++; }
  	  }

/* Now we know the pair that SHOULD be minimal and we can determine a list 
   of all edges with this colour pair. If a new pair is found that is even 
   smaller, we can return 0 at once */

	list_length = 0;

	for (i = 0; i < last_vertex; i++) 
  	  { if (colour[i] < minstart) return(0);
    	    if (colour[i] == minstart)
      	    { run = end = firstedge[i];
	      do
	      {
	          if (colour[run->end] > maxend) return(0);
	          if (colour[run->end] == maxend)
	          { startlist[list_length] = run; list_length++; }
	          run = run->next;
	      } while (run != end);
	    }
  	  }

/* OK -- so there is no smaller pair and now we have to determine the 
   smallest representation around vertex "last_vertex": */

	testcanon_first_init(startlist_last[0], representation, colour);
	numblist[0] = startlist_last[0];
	test = testcanon_mirror_init(startlist_last[0], representation, colour);
	if (test == 1) 
	{ numbs_mirror = 1; numblist_mirror[0] = startlist_last[0]; }
	else if (test == 2)  
	{ numbs_mirror = 1; numbs = 0; 
	  numblist_mirror[0] = startlist_last[0]; };

	for (i = 1; i < list_length_last; i++)
	  { test = testcanon_init(startlist_last[i], representation, colour);
    	    if (test == 1) { numblist[numbs] = startlist_last[i]; numbs++; }
    	    else if (test == 2) 
	    { numbs_mirror = 0; numbs = 1; numblist[0] = startlist_last[i]; };
	    test = testcanon_mirror_init(startlist_last[i], 
					 representation, colour);
    	    if (test == 1)  
	    { numblist_mirror[numbs_mirror] = startlist_last[i]; 
	      numbs_mirror++; }
    	    else if (test == 2) 
	    { numbs_mirror = 1; numbs = 0; 
	      numblist_mirror[0] = startlist_last[i]; };
  	  }

/* Now we know the best representation we can obtain starting at last_vertex. 
   Now we will check all the others. We can return 0 at once if we find a 
   better one */

	for (i = 0; i < list_length; i++)
  	  { test = testcanon(startlist[i], representation, colour);
	    if (test == 1) { numblist[numbs] = startlist[i]; numbs++; }
	    else if (test == 2) return(0);
	    test = testcanon_mirror(startlist[i], representation, colour);
	    if (test == 1) 
	    { numblist_mirror[numbs_mirror] = startlist[i]; numbs_mirror++; }
    	    else if (test == 2) return(0);
  	  }

	*num_can_numberings_or_pres = numbs;
	*num_can_numberings = numbs+numbs_mirror;

	if (*num_can_numberings>1)
  	{ for (i = 0; i < numbs; i++) 
	      construct_numb(numblist[i], can_numberings[i]); 
    	  for (i = 0; i < numbs_mirror; i++, numbs++) 
	      construct_numb_mirror(numblist_mirror[i], 
				     can_numberings[numbs]);
  	}
	else 
	{ if (numbs) can_numberings[0][0] = numblist[0];
	  else can_numberings[0][0] = numblist_mirror[0]; }

	return(1);
}

/****************************************************************************/

static void
compute_autom_fullerenes(int colour[], EDGE *can_numberings[][MAXE], 
	      int *num_can_numberings, int *num_can_numberings_or_pres)

/* Works pretty similar to canon() -- the only difference is that it
   always computes the automorphism group -- not only in case the last
   vertex is canonical. The restriction to fullerenes has its cause in
   the restriction to maximum degree 6, which is used for the size of
   startlist and startlist_last.*/

{
	int i, last_vertex, test;
	int minstart, maxend; /* (minstart,maxend) will be the chosen colour 
				pair of an edge */
	EDGE *startlist_last[6], *startlist[6*MAXN], *run, *end;
	       /* startlist and startlist_last are sufficient for fullerenes 
		      -- not in general */
	int list_length_last, list_length;
	int representation[2*MAXE+MAXN];
	EDGE *numblist[MAXE], *numblist_mirror[MAXE]; /* lists of edges where 
				starting gives a canonical representation */
	int numbs = 1, numbs_mirror = 0;

	last_vertex = nv-1;
	minstart = colour[last_vertex];

/* determine the smallest possible end for the vertex "last_vertex" */

	list_length_last = 1; 
	startlist_last[0] = end = firstedge[last_vertex];
	maxend = colour[end->end];

	for (run = end->next; run != end; run = run->next)
  	{ 
	    if (colour[run->end] > maxend)
	    { 	
		startlist_last[0] = run; 
	        list_length_last = 1; 
                maxend = colour[run->end];
            }
	    else if (colour[run->end] == maxend)
	    { 
		startlist_last[list_length_last] = run; 
                list_length_last++; 
            }
  	}

/* Now we know the pair that SHOULD be minimal and we can determine a list 
   of all edges with this colour pair. If a new pair is found that is even 
   smaller, we can NOT return 0 at once */

	list_length = 0;

	for (i = 0; i < last_vertex; i++) 
  	{
    	    if (colour[i] == minstart)
      	    { 
		run = end = firstedge[i];
	        do
	        {
	            if (colour[run->end] == maxend)
	            { 
		        startlist[list_length] = run; 
			list_length++; 
		    }
	            run = run->next;
	        } while (run != end);
	    }
  	}

/* OK -- now we have to determine the 
   smallest representation around vertex "last_vertex": */

	testcanon_first_init(startlist_last[0], representation, colour);
	numblist[0] = startlist_last[0];
	test = testcanon_mirror_init(startlist_last[0],representation,colour);
	if (test == 1) 
	{ 
 	    numbs_mirror = 1; 
	    numblist_mirror[0] = startlist_last[0]; 
 	}
	else if (test == 2)  
	{ 
	    numbs_mirror = 1; 
	    numbs = 0; 
	    numblist_mirror[0] = startlist_last[0]; 
	}

	for (i = 1; i < list_length_last; i++)
	{ 
	    test = testcanon_init(startlist_last[i], representation, colour);
    	    if (test == 1) 
	    { 
		numblist[numbs] = startlist_last[i]; 
		numbs++; 
	    }
    	    else if (test == 2) 
	    { 
		numbs_mirror = 0; 
		numbs = 1; 
		numblist[0] = startlist_last[i]; 
	    }

	    test = testcanon_mirror_init(startlist_last[i], 
					 representation, colour);
    	    if (test == 1)  
	    { 
		numblist_mirror[numbs_mirror] = startlist_last[i]; 
	        numbs_mirror++; 
	    }
    	    else if (test == 2) 
	    { 
		numbs_mirror = 1; 
		numbs = 0; 
	        numblist_mirror[0] = startlist_last[i]; 
	    }
  	}

/* Now we know the best representation we can obtain starting at last_vertex. 
   Now we will check all the others. */ 

	for (i = 0; i < list_length; i++)
  	{ 
	    test = testcanon(startlist[i], representation, colour);
	    if (test == 1) 
	    { 
		numblist[numbs] = startlist[i]; 
		numbs++; 
	    }
	    test = testcanon_mirror(startlist[i], representation, colour);
	    if (test == 1) 
	    { 
		numblist_mirror[numbs_mirror] = startlist[i]; 
		numbs_mirror++; 
	    }
  	}

	*num_can_numberings_or_pres = numbs;
	*num_can_numberings = numbs + numbs_mirror;


	if (*num_can_numberings>1)
  	{ 
	    for (i = 0; i < numbs; i++) 
	        construct_numb(numblist[i], can_numberings[i]); 
    	    for (i = 0; i < numbs_mirror; i++, numbs++) 
	        construct_numb_mirror(numblist_mirror[i], 
				             can_numberings[numbs]);
  	}
	else 
	{
	    if (numbs) can_numberings[0][0] = numblist[0];
	    else can_numberings[0][0] = numblist_mirror[0]; 
	}
}


/**************************************************************************/

static int
numedgeorbits(int nbtot, int nbop)

/* return number of orbits of directed edges, under the
   orientation-preserving automorphism group (assumed computed) */
{
	register EDGE **nb0,**nblim,**nb;
	register int i,j;

	if (nbtot == 1)
	    return ne;
	else
	{
	    nb0 = (EDGE**) numbering[0];
	    if (nbop == 0) nblim = (EDGE**) numbering[nbtot];
	    else           nblim = (EDGE**) numbering[nbop];

	    for (i = 0; i < ne; ++i)
		nb0[i]->mark = 0;

	    j = 0;
	    for (i = 0; i < ne; ++i, ++nb0)
		if (!(*nb0)->mark)
	        {
		    ++j;
		    for (nb = nb0; nb < nblim; nb += MAXE)
			(*nb)->mark = 1;
		}
	    return j;
	}
}

/**************************************************************************/

/* Include optional file for special processing. */

#ifdef PLUGIN
#include PLUGIN
#endif

/**************************************************************************/

static void
initialize() 

/* initialize stars */
{
	register int i,j;
	register EDGE *si;

	for (i = 0; i < MAXN; ++i)
	{
	    si = STAR3(i);

	    for (j = 0; j < 3; ++j)
	    {
	  	si[j].end = si[j+3].start = i;
		si[j].invers = si + 3 + j;
		si[j+3].invers = si + j;
	        si[j+3].next = si + 3 + (j+1) % 3;
		si[j+3].prev = si + 3 + (j+2) % 3;
	    }

	    si = STAR4(i);
 
	    for (j = 0; j < 4; ++j) 
	    { 
		si[j].end = si[j+4].start = i;
		si[j].invers = si + 4 + j; 
		si[j+4].invers = si + j; 
		si[j+4].next = si + 4 + (j+1) % 4; 
		si[j+4].prev = si + 4 + (j+3) % 4;
	    }

	    si = STAR5(i);
  
	    for (j = 0; j < 5; ++j)  
	    {  
		si[j].end = si[j+5].start = i; 
		si[j].invers = si + 5 + j;  
		si[j+5].invers = si + j;  
		si[j+5].next = si + 5 + (j+1) % 5;  
		si[j+5].prev = si + 5 + (j+4) % 5; 
	    }
	}
}

/**************************************************************************/

static void
initialK4()

/* create initial K4 */
{
	init_edge[0].start = 0; init_edge[0].end = 1;
	init_edge[0].next = init_edge[0].prev = init_edge+2;
	init_edge[0].invers = init_edge+1;

	init_edge[1].start = 1; init_edge[1].end = 0;
	init_edge[1].next = init_edge[1].prev = init_edge+4;
	init_edge[1].invers = init_edge+0;

	init_edge[2].start = 0; init_edge[2].end = 2;
	init_edge[2].next = init_edge[2].prev = init_edge+0;
	init_edge[2].invers = init_edge+3;

	init_edge[3].start = 2; init_edge[3].end = 0;
	init_edge[3].next = init_edge[3].prev = init_edge+5;
	init_edge[3].invers = init_edge+2;

	init_edge[4].start = 1; init_edge[4].end = 2;
	init_edge[4].next = init_edge[4].prev = init_edge+1;
	init_edge[4].invers = init_edge+5;

	init_edge[5].start = 2; init_edge[5].end = 1;
	init_edge[5].next = init_edge[5].prev = init_edge+3;
	init_edge[5].invers = init_edge+4;

	nv = 3;
	ne = 6;

	degree[0] = degree[1] = degree[2] = 2;
	firstedge[0] = init_edge;
	firstedge[1] = init_edge+1;
	firstedge[2] = init_edge+3;

	extend3(init_edge);
}

/**************************************************************************/

static void
find_extensions(int numb_total, int numb_pres,
                EDGE *ext3[], int *numext3, 
                EDGE *ext4[], int *numext4,
                EDGE *ext5[], int *numext5)

/* Find all nonequivalent places for extension.
   These are listed in ext3/4/5 according to the degree of the 
   future new vertex.  The number of cases is returned in numext3/4/5. */
{
        register int i,k;
        int deg3,deg4;
        register EDGE *e,*e1,*e2,*ex;
        EDGE **nb0,**nb1,**nbop,**nblim;

        deg3 = deg4 = 0;
        for (i = 0; i < nv; ++i)
            if      (degree[i] == 3) ++deg3;
            else if (degree[i] == 4) ++deg4;
        
     /* code for trivial group */

        if (numb_total == 1)
        {
	    if (nv == maxnv-1 && fswitch)
	       *numext3 = 0;
	    else
	    {
                k = 0;
                for (i = 0; i < nv; ++i)
                {
		    if (degree[i] == 6) continue;
                    e = ex = firstedge[i];
                    do
                    {
                        e1 = e->invers->prev;
                        if (e1 > e)
                        {
                            e1 = e1->invers->prev;
                            if (e1 > e) 
			    {
				if (degree[e->end] < 6 &&
					degree[e1->start] < 6)
				    ext3[k++] = e;
			    }
                        }
                        e = e->next;
                    }
                    while (e != ex);
		}
                *numext3 = k;
	    }

            if (deg3 <= 2)
            {
                k = 0;
                for (i = 0; i < nv; ++i)
                {
                    if (degree[i] == 3) continue;
                    e = ex = firstedge[i];
                    do
                    {
                        e1 = e->next;
                        if (e1->invers > e1)
                        {
                            e2 = e1->invers->prev;
                            if (degree[e->end] < 6 &&
				degree[e2->end] < 6 &&
				(degree[e->end] == 3)
                                        + (degree[e2->end] == 3) == deg3)
                            ext4[k++] = e;
                        }
                        e = e->next;
                    }
                    while (e != ex);
                }
                *numext4 = k;
            }
            else
                *numext4 = 0;
                        
            *numext5 = 0;
        }

     /* code for nontrivial group */
        else
        {
            nb0 = (EDGE**)numbering[0];
            nbop = (EDGE**)numbering[numb_pres == 0 ? numb_total : numb_pres];
            nblim = (EDGE**)numbering[numb_total];

            for (i = 0; i < ne; ++i)
            {
                nb0[i]->mark = 0;
                nb0[i]->index = i;
            }

	    if (nv == maxnv-1 && fswitch)
		*numext3 = 0;
	    else
	    {
                k = 0;
                for (i = 0; i < ne; ++i)
                {
                    e = nb0[i];
                    if (e->mark) continue;
		    if (degree[e->start] == 6 || degree[e->end] == 6
                        || degree[e->next->end] == 6) continue;

                    ext3[k++] = e;
                    
                    for (nb1 = nb0 + i; nb1 < nbop; nb1 += MAXE)
                        (*nb1)->mark = 1;
    
                    for (; nb1 < nblim; nb1 += MAXE)
                            (*nb1)->invers->mark = 1;
    
                    e1 = e->invers->prev;
                    for (nb1 = nb0 + e1->index; nb1 < nbop; nb1 += MAXE)
                        (*nb1)->mark = 1;
    
                    for (; nb1 < nblim; nb1 += MAXE)
                        (*nb1)->invers->mark = 1;
    
                    e1 = e1->invers->prev;
                    for (nb1 = nb0 + e1->index; nb1 < nbop; nb1 += MAXE)
                        (*nb1)->mark = 1;
    
                    for (; nb1 < nblim; nb1 += MAXE)
                        (*nb1)->invers->mark = 1;
                }
                *numext3 = k;
	    }
        
            if (deg3 <= 2)
            {
                for (i = 0; i < ne; ++i)
                    nb0[i]->mark = 0;
                k = 0;
                for (i = 0; i < ne; ++i)
                {
                    e = nb0[i];
                    if (e->mark) continue;
                    e1 = e->next->invers->prev;
		    if (degree[e->end] == 6 || degree[e1->end] == 6) continue;
                    if ((degree[e->end] == 3) + (degree[e1->end] == 3) != deg3)
                        continue;
                    ext4[k++] = e;

                    for (nb1 = nb0 + i; nb1 < nbop; nb1 += MAXE)
                        (*nb1)->mark = 1;

                    for (; nb1 < nblim; nb1 += MAXE)
                        (*nb1)->prev->prev->mark = 1;

                    for (nb1 = nb0 + e1->index; nb1 < nbop; nb1 += MAXE)
                        (*nb1)->mark = 1;
 
                    for (; nb1 < nblim; nb1 += MAXE)
                        (*nb1)->prev->prev->mark = 1;
                }
                *numext4 = k;
            }
            else
                *numext4 = 0;

            *numext5 = 0;
        }
}

/**************************************************************************/

static void
find_dual()

/* Store in the rightface field of each edge the number of the face on 
   the right hand side of that edge.  Faces are numbered 0,1,....  Also
   store in facestart[i] an example of an edge in the clockwise
   orientation of the face boundary, for each i. */
{
	register int i,nf;
	register EDGE *e,*e1,*ex;

	for (i = 0; i < nv; ++i)
	{
	    e = ex = firstedge[i];
	    do
	    {
		e->rightface = -1;
		e = e->next;
	    }
	    while (e != ex);
	}

	nf = 0;
	for (i = 0; i < nv; ++i)
	{
	    e = ex = firstedge[i];
	    do
	    {
		if (e->rightface < 0)
		{
		    e->rightface = nf;
		    e1 = e->invers->prev;
		    e1->invers->prev->rightface = e1->rightface = nf;
		    facestart[nf] = e;
		    ++nf;
		}
		e = e->next;
	    }
	    while (e != ex);
	}
}

/**************************************************************************/

static void
write_dual_planar_code(FILE *f, int doflip)   

/* Write the dual in planar_code format.  Always write in next direction,
   and if doflip != 0 also write in prev direction.  find_dual() must
   have been called first */
{
	register int i,k,k1;
	register EDGE *e;
	unsigned char code[2*MAXF+2*MAXE+5];
	int nf;

	nf = 2*nv - 4;
	code[0] = nf;
	k = 1;

	for (i = 0; i < nf; ++i)
	{
	    e = facestart[i]->invers;
	    code[k] = 1 + e->rightface;
	    e = e->prev->invers;
	    code[k+1] = 1 + e->rightface;
	    e = e->prev->invers;
	    code[k+2] = 1 + e->rightface;
	    code[k+3] = 0;
	    k += 4;
	}

	if (doflip)
	{
	    k1 = 1;
	    code[k++] = nf;
	    for (i = 0; i < nf; ++i)
	    {
		code[k] = code[k1+2];
		code[k+1] = code[k1+1];
		code[k+2] = code[k1];
		code[k+3] = 0;
		k += 4;
		k1 += 4;
	    }
	}

	if (fwrite(code,(size_t)1,(size_t)k,f) != k)
	{
	    fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
	    perror(">E ");
	    exit(1);
	}
}

/**************************************************************************/

static void
write_dual_alpha(FILE *f, int doflip)   

/* Write the dual in planar_code format.  Always write in next direction,
   and if doflip != 0 also write in prev direction.  find_dual must have
   been called first. */
{
	register int i,k,k1;
	register EDGE *e;
	unsigned char code[2*MAXF+2*MAXE+10];
	int nf,nfsize;

	nf = 2*nv - 4;
	if (nf >= 10)
	{
	    code[0] = '0' + nf/10;
	    code[1] = '0' + nf%10;
	    code[2] = ' ';
	    nfsize = k = 3;
	}
	else
	{
	    code[0] = '0' + nf;
	    code[1] = ' ';
	    nfsize = k = 2;
	}

	for (i = 0; i < nf; ++i)
	{
	    e = facestart[i]->invers;
	    code[k] = 'a' + e->rightface;
	    e = e->prev->invers;
	    code[k+1] = 'a' + e->rightface;
	    e = e->prev->invers;
	    code[k+2] = 'a' + e->rightface;
	    code[k+3] = ',';
	    k += 4;
	}
	code[k-1] = '\n';

	if (doflip)
	{
	    for (i = 0; i < nfsize; ++i)
		code[k++] = code[i];

	    k1 = nfsize;
	    for (i = 0; i < nf; ++i)
	    {
		code[k] = code[k1+2];
		code[k+1] = code[k1+1];
		code[k+2] = code[k1];
		code[k+3] = ',';
		k += 4;
		k1 += 4;
	    }
	}
	code[k-1] = '\n';

	if (fwrite(code,(size_t)1,(size_t)k,f) != k)
	{
	    fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
	    perror(">E ");
	    exit(1);
	}
}

/**************************************************************************/

static void
write_planar_code(FILE *f, int doflip)   

/* Write in planar_code format.  Always write in next direction,
   and if doflip != 0 also write in prev direction. */
{
	register int i,k;
	register EDGE *ex,*e;
	unsigned char code[2*MAXN+2*MAXE+5];

	code[0] = nv;
	k = 1;

	for (i = 0; i < nv; ++i)
	{
	    e = ex = firstedge[i];
	    do
	    {
		code[k++] = e->end + 1;
		e = e->next;
	    }
	    while (e != ex);
	    code[k++] = 0;
	}

	if (doflip)
	{
	    code[k++] = nv;
	    for (i = 0; i < nv; ++i)
	    {
		e = ex = firstedge[i];
		do
		{
		    code[k++] = e->end + 1;
		    e = e->prev;
		}
		while (e != ex);
		code[k++] = 0;
	    }
	}

	if (fwrite(code,(size_t)1,(size_t)k,f) != k)
	{
	    fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
	    perror(">E ");
	    exit(1);
	}
}

/**************************************************************************/

static void
write_sparse6(FILE *f, int doflip)

/* Write in sparse6 format.  doflip is ignored. */
{
    unsigned char code[20+2*MAXE+2*MAXN];
    register unsigned char *p;
    int nb,i,j,lastj,x,k,r,rr,topbit;
    EDGE *e,*ex;

    p = code;
    *p++ = ':';

    if (nv <= 62)
        *p++ = 63 + nv;
    else
    {
        *p++ = 63 + 63;
        *p++ = 63 + 0;
        *p++ = 63 + (nv >> 6);
        *p++ = 63 + (nv & 0x3F);
    }

    for (i = nv-1, nb = 0; i != 0 ; i >>= 1, ++nb) {}
    topbit = 1 << (nb-1);
    k = 6;
    x = 0;

    lastj = 0;
    for (j = 0; j < nv; ++j)
    {
        e = ex = firstedge[j];
        do
        {
            i = e->end;
            if (i <= j)
            {
                if (j == lastj)
                {
                    x <<= 1;
                    if (--k == 0)
                    {
                        *p++ = 63 + x;
                        k = 6;
                        x = 0;
                    }
                }
                else
                {
                    x = (x << 1) | 1;
                    if (--k == 0)
                    {
                        *p++ = 63 + x;
                        k = 6;
                        x = 0;
                    }
                    if (j > lastj+1)
                    {
                        for (r = 0, rr = j; r < nb; ++r, rr <<= 1)
                        {
                            if (rr & topbit) x = (x << 1) | 1;
                            else             x <<= 1;
                            if (--k == 0)
                            {
                                *p++ = 63 + x;
                                k = 6;
                                x = 0;
                            }
                        }
                        x <<= 1;
                        if (--k == 0)
                        {
                            *p++ = 63 + x;
                            k = 6;
                            x = 0;
                        }
                    }
                    lastj = j;
                }
                for (r = 0, rr = i; r < nb; ++r, rr <<= 1)
                {
                    if (rr & topbit) x = (x << 1) | 1;
                    else             x <<= 1;
                    if (--k == 0)
                    {
                        *p++ = 63 + x;
                        k = 6;
                        x = 0;
                    }
                }
            }
            e = e->next;
        } while (e != ex);
    }

    if (k != 6) *p++ = 63 + ((x << k) | ((1 << k) - 1));

    *p++ = '\n';
    k = p - code;

    if (fwrite(code,sizeof(unsigned char),(size_t)k,f) != k)
    {
        fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
        perror(">E ");
        exit(1);
    }
}

/**************************************************************************/

static void
write_dual_sparse6(FILE *f, int doflip)

/* Write dual cubic graph in sparse6 format.  doflip is ignored.
   find_dual() must have been called first. */
{
    unsigned char code[20+2*MAXE+2*MAXF];
    register unsigned char *p;
    int nb,nf,i,j,lastj,x,k,r,rr,topbit;
    EDGE *e,*ex;

    p = code;
    *p++ = ':';

    nf = 2*nv - 4;

    if (nf <= 62)
        *p++ = 63 + nf;
    else
    {
        *p++ = 63 + 63;
        *p++ = 63 + 0;
        *p++ = 63 + (nf >> 6);
        *p++ = 63 + (nf & 0x3F);
    }

    for (i = nf-1, nb = 0; i != 0 ; i >>= 1, ++nb) {}
    topbit = 1 << (nb-1);
    k = 6;
    x = 0;

    lastj = 0;
    for (j = 0; j < nf; ++j)
    {
        e = ex = facestart[j]->invers;
        do
        {
            i = e->rightface;
            if (i <= j)
            {
                if (j == lastj)
                {
                    x <<= 1;
                    if (--k == 0)
                    {
                        *p++ = 63 + x;
                        k = 6;
                        x = 0;
                    }
                }
                else
                {
                    x = (x << 1) | 1;
                    if (--k == 0)
                    {
                        *p++ = 63 + x;
                        k = 6;
                        x = 0;
                    }
                    if (j > lastj+1)
                    {
                        for (r = 0, rr = j; r < nb; ++r, rr <<= 1)
                        {
                            if (rr & topbit) x = (x << 1) | 1;
                            else             x <<= 1;
                            if (--k == 0)
                            {
                                *p++ = 63 + x;
                                k = 6;
                                x = 0;
                            }
                        }
                        x <<= 1;
                        if (--k == 0)
                        {
                            *p++ = 63 + x;
                            k = 6;
                            x = 0;
                        }
                    }
                    lastj = j;
                }
                for (r = 0, rr = i; r < nb; ++r, rr <<= 1)
                {
                    if (rr & topbit) x = (x << 1) | 1;
                    else             x <<= 1;
                    if (--k == 0)
                    {
                        *p++ = 63 + x;
                        k = 6;
                        x = 0;
                    }
                }
            }
            e = e->prev->invers;
        } while (e != ex);
    }

    if (k != 6) *p++ = 63 + ((x << k) | ((1 << k) - 1));

    *p++ = '\n';
    k = p - code;

    if (fwrite(code,sizeof(unsigned char),(size_t)k,f) != k)
    {
        fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
        perror(">E ");
        exit(1);
    }
}

/**************************************************************************/

static void
write_graph6(FILE *f)

/* Write in graph6 format. */
{
	unsigned char code[20+MAXN*(MAXN-1)/12];
	register unsigned char *body;
	int nlen,bodylen,i,j,org;
	static unsigned char g6bit[] = {32,16,8,4,2,1};
	EDGE *e,*ex;

	if (nv <= 62)
	{
	    code[0] = 63 + nv;
	    nlen = 1;
	}
	else
	{
	    code[0] = 63 + 63;
	    code[1] = 63 + 0;
	    code[2] = 63 + (nv >> 6);
	    code[3] = 63 + (nv & 0x3F);
	    nlen = 4;
	}

	body = code + nlen;
	bodylen = ((nv * (nv-1)) / 2 + 5) / 6;
	for (i = 0; i < bodylen; ++i)
	    body[i] = 63;
	body[bodylen] = '\n';

	for (i = org = 0; i < nv; org += i, ++i)
        {
            e = ex = firstedge[i];
            do
            {
                if (e->end < i)
		{
		    j = org + e->end;
		    body[j/6] += g6bit[j%6];
		}
                e = e->next;
            }
            while (e != ex);
        }

	j = nlen + bodylen + 1;
        if (fwrite(code,(size_t)1,(size_t)j,f) != j)
        {
            fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
            perror(">E ");
            exit(1);
        }
}	

/**************************************************************************/

static void
write_dual_graph6(FILE *f)

/* Write dual in graph6 format. */
{
	unsigned char code[20+MAXF*(MAXF-1)/12];
	register unsigned char *body;
	int nf,nlen,bodylen,i,j,org;
	static unsigned char g6bit[] = {32,16,8,4,2,1};
	EDGE *e;

	nf = 2*nv - 4;
	if (nf <= 62)
	{
	    code[0] = 63 + nf;
	    nlen = 1;
	}
	else
	{
	    code[0] = 63 + 63;
	    code[1] = 63 + 0;
	    code[2] = 63 + (nf >> 6);
	    code[3] = 63 + (nf & 0x3F);
	    nlen = 4;
	}

	body = code + nlen;
	bodylen = ((nf * (nf-1)) / 2 + 5) / 6;
	for (i = 0; i < bodylen; ++i)
	    body[i] = 63;
	body[bodylen] = '\n';

	for (i = org = 0; i < nf; org += i, ++i)
        {
            e = facestart[i]->invers;
            if (e->rightface < i)
	    { 
		j = org + e->rightface;
		body[j/6] += g6bit[j%6];
	    }
	    e = e->prev->invers;
	    if (e->rightface < i)
            {
                j = org + e->rightface;
                body[j/6] += g6bit[j%6];
            }
            e = e->prev->invers;
            if (e->rightface < i)
            {
                j = org + e->rightface;
                body[j/6] += g6bit[j%6];
            }
        }

	j = nlen + bodylen + 1;
        if (fwrite(code,(size_t)1,(size_t)j,f) != j)
        {
            fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
            perror(">E ");
            exit(1);
        }
}	

/**************************************************************************/

static void
write_alpha(FILE *f, int doflip)

/* Write in alphabetic format.  Always write in next direction,
   and if doflip != 0 also write in prev direction. */
{
	register int i,k;
	register EDGE *ex,*e;
	unsigned char code[2*MAXN+2*MAXE+9];
	int nvsize;

	if (nv >= 10) 
	{
	    code[0] = '0' + nv/10;
	    code[1] = '0' + nv%10;
	    code[2] = ' ';
	    nvsize = k = 3;
	}
	else
	{
	    code[0] = '0' + nv;
	    code[1] = ' ';
	    nvsize = k = 2;
	}

	for (i = 0; i < nv; ++i)
	{
	    e = ex = firstedge[i];
	    do
	    {
		code[k++] = e->end + 'a';
		e = e->next;
	    }
	    while (e != ex);
	    code[k++] = ',';
	}
	code[k-1] = '\n';

	if (doflip)
	{
	    for (i = 0; i < nvsize; ++i)
	        code[k++] = code[i];
	
	    for (i = 0; i < nv; ++i)
	    {
		e = ex = firstedge[i];
		do
		{
		    code[k++] = e->end + 'a';
		    e = e->prev;
		}
		while (e != ex);
		code[k++] = ',';
	    }
	}
	code[k-1] = '\n';

	if (fwrite(code,(size_t)1,(size_t)k,f) != k)
	{
	    fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
	    perror(">E ");
	    exit(1);
	}
}

/**************************************************************************/

static void
got_one(int nbtot, int nbop)

/* This is called when a complete triangulation is formed.  The main 
   purpose is to write the triangulation and to collect some stats. */
{
	register int doflip,wt,maxdeg;

	doflip = oswitch && (nbop == nbtot || nbop == 0);

#ifdef PLUGIN
	if (FILTER(nbtot,nbop,doflip))
#endif
	{
	    ADDBIG(nout,doflip + 1);
#if STATS
	    wt = doflip ? 2 : 1;
	    ADDBIG(numrooted,wt * numedgeorbits(nbtot,nbop));
	    maxdeg = degree[nv-1];
	    if (maxdeg > 5) maxdeg = 5;
	    nummindeg[maxdeg-3][nv] += wt;
	    if (gswitch && nbtot == 1) ADDBIG(ntriv,wt);
#endif

	    if (!uswitch)
	    {
	        if (dswitch)
	        {
	            find_dual();
		    if (aswitch)       write_dual_alpha(outfile,doflip);
		    else if (Sswitch)  write_dual_sparse6(outfile,doflip);
		    else if (Aswitch)  write_dual_graph6(outfile);
		    else               write_dual_planar_code(outfile,doflip);
	        }
	        else if (aswitch) write_alpha(outfile,doflip);
		else if (Sswitch) write_sparse6(outfile,doflip);
		else if (Aswitch) write_graph6(outfile);
	        else              write_planar_code(outfile,doflip);
	    }
	}
}

/**************************************************************************/

static int
make_colours(int col[])  

/* make better colours for maxdeg=3.
   If nv-1 is not best, return 0.
   Otherwise, return the number of vertices with the same
   colour as nv-1 (including itself).  Note that for correct
   operation, col[nv-1] must have the smallest value in col[]. */
{
	register int i,c,c0,nc;
	register EDGE *e;

	e = firstedge[nv-1];
	c0 = degree[e->end] + degree[e->next->end] + degree[e->next->next->end];

	col[nv-1] = 2;
	nc = 1;

	for (i = nv-1; --i >= 0;)
	{
	    if (degree[i] != 3)
		col[i] = degree[i];
	    else
	    {
		e = firstedge[i]; 
		c = degree[e->end] 
			+ degree[e->next->end] + degree[e->next->next->end]; 
		if (c < c0)  
		    return 0;
		else if (c == c0) 
		{
		    col[i] = 2;
		    ++nc;
		}
		else 
		    col[i] = 3;
	    }
	}

	return nc;
}

/**************************************************************************/

static void
scantree(int nbtot, int nbop)

/* The main node of the recursion.  As this procedure is entered,
   nv,ne,degree etc are set for some graph, and nbtot/nbop are the
   values returned by canon() for that graph. */
{
	EDGE *ext3[MAXE/3],*ext4[MAXE/2],*ext5[1];
	int next3,next4,next5;
	EDGE *save_list[4];
	register int i;
	register EDGE *e1,*e2,**nb,**nblim;
	EDGE *e,*ex;
	int nc,xnbtot,xnbop,v;
	int colour[MAXN],nodd;

	nodd = 0;
	if (xswitch)
	{
	    for (i = 0; i < nv; ++i) if ((degree[i]&1)) ++ nodd;
	    if (nodd > 2*(maxnv-nv)) return;
	}

	if (nv == maxnv)
	{
	    got_one(nbtot,nbop);
	    return;
	}
	else if (nodd == 0 && 
                  nv >= minnv && (!fswitch || degree[nv-1] > 3))
	    got_one(nbtot,nbop);

#ifdef PRE_FILTER
	if (!(PRE_FILTER)) return;
#endif

#ifndef FIND_EXTENSIONS
#define FIND_EXTENSIONS find_extensions
#endif

	FIND_EXTENSIONS(nbtot,nbop,ext3,&next3,ext4,&next4,ext5,&next5);

	for (i = 0; i < next3; ++i)
	{
	    extend3(ext3[i]);
	    nc = make_colours(colour);
	    if (nc)
	    {
	        if (nc == 1 && nv == maxnv && !gswitch)
		    got_one(1,1);
		else if (canon(colour,numbering,&xnbtot,&xnbop))
		    scantree(xnbtot,xnbop);
	    }
	    reduce3(ext3[i]);
	}

	for (i = 0; i < next4; ++i)
	{
	    extend4(ext4[i],save_list);
	    if (canon(degree,numbering,&xnbtot,&xnbop))   
	    {
		e = numbering[0][0];
		v = e->next->next->end;
		ex = e->invers;
		for (e = ex->next; e != ex; e = e->next)
		    if (e->end == v) break;

		e1 = ext4[i]->next->invers;
		if (e != ex) e1 = e1->next;
	
		e2 = e1->next->next;
		nblim = (EDGE**)numbering[xnbtot];
		for (nb = (EDGE**)numbering[0]; nb < nblim; nb += MAXE)
		    if (*nb == e1 || *nb == e2) break;

		if (nb < nblim) scantree(xnbtot,xnbop); 
	    }
	    reduce4(ext4[i],save_list); 
	}
}

/****************************************************************************/

static int 
getswitchvalue(char *arg, int *pj)

/* Find integer value for switch. 
   arg is a pointer to command-line argument.
   pj is an index into arg, which is updated.
   The value of the switch is the function return value.
   For example, if arg="-xyz1432q" and *pj=3 (pointing at "z"),
       the value 1432 is returned and *pj=8 (pointing at "q"). */
{
        int j,ans;

        ans = 0;
        for (j = *pj; arg[j+1] >= '0' && arg[j+1] <= '9'; ++j)
            ans = ans * 10 + (arg[j+1] - '0');
        
        *pj = j;
        return ans;
}


/***************************************************************************/

static void 
init_circular_lists(EDGE *edge, int length)

{
	int j;

	length--; /* largest index instead of length of circular list */

	edge->prev=edge+length;
	edge->next=edge+1;

	for (j = 1; j < length; ++j)  
    	{  
      	    edge[j].next = edge+j+1;
      	    edge[j].prev = edge+j-1;
    	}

	edge[length].prev=edge+length-1;
	edge[length].next=edge;
}


/*****************************************************************************/

int read_next_fullerene(int which_case, int res, int mod)

/* This function reads the next fullerene from a Unix pipe and writes the 
   dual of it in a way that it can be accessed via the global variable

   static EDGE *firstedge[MAXN];

   It relies on that for call n+1 the data structure is in the same shape 
   as at the end of call n (fully restored). It returns 1 if a fullerene 
   is read, 0 if no more fullerene is read. 

   The program fullgen or buckygen must be in the path */

{ 
	static int first_call=1;
	static EDGE fives[12][5]; 
	    /* circular lists that are just once initialised */
	static EDGE sixes[MAXN-12][6];
	static FILE *fullpipe;

	int code[MAXN*7];
	int nlt, ch, i, j, nulls, counter, count5, count6, end;
	char resmod[30],command[100];
	EDGE *search;

	
	if (first_call)
  	  { if (maxnv < 12) return 0;
    	    first_call=0;
    	    for (i = 0; i < 12; i++) 
		init_circular_lists(fives[i],5);
    	    for (i = 0; i < MAXN-12; i++) 
		init_circular_lists(sixes[i],6);

#if FULLERENES==0
	    if (mod > 1) sprintf(resmod," mod %d %d",res,mod);
	    else         sprintf(resmod,"");
	    
	    if (which_case==0) 
                sprintf(command,
	               "fullgen %d start %d stdout code 7%s%s",
                       2*eswitch-4,2*sswitch-4,resmod,
		       iswitch ? " spiralcheck" : " quiet");
            else 
	        sprintf(command,
		       "fullgen %d start %d stdout code 7 case %d%s%s",
                       2*eswitch-4,2*sswitch-4,which_case,resmod,
		       iswitch ? " spiralcheck" : " quiet"); 
#else
	    if (mod > 1) sprintf(resmod," %d/%d",res,mod);
	    else         sprintf(resmod,"");

	    sprintf(command,"buckygen -S%d%s %d%s",sswitch,
		    iswitch ? "" : " -q",eswitch,resmod);
#endif

    	    fullpipe=popen(command,"r");
    	    if (fullpipe==NULL) 
	      { fprintf(stderr,"Can't open fullerene pipe for reading !\n");
	        exit(1); 
	      }

	    nlt = 0;         /* Skip the header */
	    while (nlt < 2)
	    {
		if ((ch = getc(fullpipe)) == EOF)
		    { pclose(fullpipe); return(0); }
		if (ch == '<') ++nlt;
		else           nlt = 0;
	    }
	}

	if ((nv = getc(fullpipe)) == EOF) { pclose(fullpipe); return(0); }
	
	ne= 6*nv - 12;

	for (nulls = counter = 0; nulls < nv; counter++)
  	  { code[counter] = getc(fullpipe)-1;
    	    if (code[counter] < 0) nulls++;
    	  }
	
	count5 = count6 = 0;
	for (j = 0, counter = 0; j < nv; j++)
  	  { if (code[counter+5] < 0) /* a vertex with valence 5 following */
      	      { for (i = 0; i < 5; i++)
	  	  { end = code[counter]; counter++;
	    	    fives[count5][i].start = j; 
		    fives[count5][i].end = end;
	    	    if (j > end)
	      	      { for (search = firstedge[end]; 
			     search->end != j; search=search->next) {}
			fives[count5][i].invers = search;
			search->invers = fives[count5]+i; 
		      }
	  	  }
		degree[j] = 5;
		firstedge[j] = fives[count5];
		counter++; /* throw away the endmark */
		count5++;
       	      }
    	    else    /* a vertex with valence 6 following */
              { for (i = 0; i < 6; i++)
	  	  { end = code[counter]; counter++;
	    	    sixes[count6][i].start = j; 
		    sixes[count6][i].end = end;
	            if (j > end)
	      	      { for (search=firstedge[end]; 
			     search->end != j; search=search->next) {}
			sixes[count6][i].invers = search;
		 	search->invers = sixes[count6] + i; 
		      }
	  	  }
		degree[j] = 6;
		firstedge[j] = sixes[count6];
		counter++; /* throw away the endmark */
		count6++;
      	      }
  	  }

	return 1;
}


/****************************************************************************/

main(int argc, char *argv[])
{
	char *arg,*outfilename;
	int badargs,argsgot;
	int i,j;
	int nbtot,nbop,nvf;
	int res,mod;
	unsigned long total;
	bigint numfull;
	int K4used;
#if !__STDC__
	char *malloc();
#endif
#if CPUTIME
        struct tms timestruct0,timestruct1;

        times(&timestruct0);
#endif

	cmdname = argv[0];

	argsgot = 0;
	badargs = FALSE;
	outfilename = NULL;
	aswitch = FALSE;
	Aswitch = FALSE;
	Sswitch = FALSE;
	gswitch = FALSE;
	hswitch = FALSE;
	oswitch = FALSE;
	dswitch = FALSE;
	uswitch = FALSE;
	fswitch = FALSE;
	xswitch = FALSE;
	cswitch = 0;
	iswitch = FALSE;
	sswitch = 12;
	eswitch = -1;
	esswitch = FALSE;
	mswitch = FALSE;
	res = 0;
	mod = 1;

	for (i = 1; !badargs && i < argc; ++i)
	{
	    arg = argv[i];
	    if (arg[0] == '-' && arg[1] != '\0')
	    {
		for (j = 1; arg[j] != '\0'; ++j)
		    if      (arg[j] == 'o') oswitch = TRUE;
		    else if (arg[j] == 'd') dswitch = TRUE;
		    else if (arg[j] == 'g') gswitch = TRUE;
		    else if (arg[j] == 'h') hswitch = TRUE;
		    else if (arg[j] == 'a') aswitch = TRUE;
		    else if (arg[j] == 'A') Aswitch = TRUE;
		    else if (arg[j] == 'S') Sswitch = TRUE;
		    else if (arg[j] == 'u') uswitch = TRUE;
		    else if (arg[j] == 'f') fswitch = TRUE;
		    else if (arg[j] == 'x') xswitch = fswitch = TRUE;
		    else if (arg[j] == 'i') iswitch = TRUE;
		    else if (arg[j] == 'c')
			cswitch = getswitchvalue(arg,&j);
		    else if (arg[j] == 'e')
		    {
                        eswitch = getswitchvalue(arg,&j); 
			esswitch = TRUE; 
		    }
		    else if (arg[j] == 's')
		    { 
			sswitch = getswitchvalue(arg,&j);
 			esswitch = TRUE; 
		    }
		    else if (arg[j] == 'm')
                    {
                        res = getswitchvalue(arg,&j);
                        mswitch = TRUE;
                    }
                    else if (arg[j] == '/')
                    {
                        mod = getswitchvalue(arg,&j);
                        mswitch = TRUE;
                    }

#ifdef PLUGIN_SWITCHES
		    PLUGIN_SWITCHES
#endif
		    else  
	                badargs = TRUE;
	    }
	    else if (argsgot >= 3)
		badargs = TRUE;
	    else if (argsgot == 0)
	    {
		if (sscanf(arg,"%d-%d",&minnv,&maxnv) != 2 &&
                    sscanf(arg,"%d:%d",&minnv,&maxnv) != 2)
		{
		    if (sscanf(arg,"%d",&maxnv) == 1) 
		        minnv = maxnv;
                    else
		 	badargs = TRUE;
		}
		++argsgot;
	    }
	    else
	    {
		if (arg[0] == '-')
		{
		    if (argsgot == 0) badargs = TRUE;
		}
		else
		    outfilename = arg;
		++argsgot;
	    }
	}

	if (oswitch) gswitch = TRUE;

	if (argsgot == 0) badargs = TRUE;

	if (badargs)
	{
	    fprintf(stderr,
              ">E Usage: %s %s [n | min-max] [outfile]\n",cmdname,SWITCHES);
	    exit(2);
	}
	
	if (maxnv < 4 || fswitch && maxnv < 6 || maxnv > MAXN)
	{
	    fprintf(stderr,">E %s: n must be 4..%d (6..%d with -f)\n",
                    cmdname,MAXN,MAXN);
	    exit(2);
	}

	if (minnv > maxnv)
	{
	    fprintf(stderr,">E %s: maxnv < minnv not allowed\n",cmdname);
	    exit(2);
	}

        if (cswitch < 0 || cswitch > 3)
        {
            fprintf(stderr,">E %s: value of -c must be 0..3\n",cmdname);
            exit(2);
        }
#if FULLERENES>0
        if (cswitch != 0)
        {
            fprintf(stderr,">E %s: -c is not available with buckgen\n",cmdname);
            exit(2);
        }
#endif

	if (aswitch + Aswitch + Sswitch > 1)
	{
	    fprintf(stderr,">E %s: -a -A -S are incompatible\n",cmdname);
	    exit(2);
	}

        if (!uswitch)
	{
	    nvf = dswitch ? 2*maxnv-4 : maxnv;
            if (aswitch && nvf > 99 ||
	        Aswitch && nvf > 4094 ||
               !aswitch && !Aswitch && !Sswitch && nvf > 255)
	    {
	        fprintf(stderr,
                    ">E %s: n is too large for that output format\n",
		    cmdname);
		exit(2);
	    }
	}

	if (eswitch == -1 || eswitch > maxnv) eswitch = maxnv;
	if (eswitch < 12) eswitch = sswitch = 0;
	else if (sswitch > eswitch)
	{
            fprintf(stderr,">E %s: -e# < -s# not allowed\n",cmdname);
            exit(2);
        }

	if (mswitch && (res < 0 || res >= mod))
	{
	    fprintf(stderr,">E %s: -m#/# needs 0 <= res < mod\n",cmdname);
	    exit(2);
	}

     /* open output file */

	msgfile = stdout;
	if (outfilename == NULL)
	{
	    outfilename = "stdout";
	    outfile = stdout;
	    msgfile = stderr;
	}
	else if ((outfile = fopen(outfilename,
			aswitch || Aswitch || Sswitch ? "w" : "wb")) == NULL)
	{
	    fprintf(stderr,
		  ">E %s: can't open %s for writing\n",cmdname,outfilename);
	    perror(">E ");
	    exit(2);
	}

	ZEROBIG(nout);
#if STATS
	ZEROBIG(ntriv);
	ZEROBIG(numrooted);
#endif

#ifdef PLUGIN_INIT
	PLUGIN_INIT;
#endif

	initialize();   /* initialize stars */

	ZEROBIG(numfull);
	K4used = FALSE;

	if ((cswitch == 0 || cswitch == 2) && (!esswitch || eswitch == 0)
				           && res == 0)
	{
	    initialK4();   /* make initial K4 */
	    K4used = TRUE;

	    canon(degree,numbering,&nbtot,&nbop);

	    if (!uswitch && !aswitch)
	    {
		if (!hswitch && !Aswitch && !Sswitch &&
	            fwrite(PCODE,(size_t)1,PCODELEN,outfile) != PCODELEN
		  || hswitch && Sswitch &&
                    fwrite(S6CODE,(size_t)1,S6CODELEN,outfile) != S6CODELEN
		  || hswitch && Aswitch &&
                    fwrite(G6CODE,(size_t)1,G6CODELEN,outfile) != G6CODELEN)
                {
                    fprintf(stderr,">E %s: error writing header\n",cmdname);
                    perror(">E ");
                    exit(2);
                }
	    }

	    scantree(nbtot,nbop);
	}

	if (!esswitch || eswitch > 0)
	    while (read_next_fullerene(cswitch,res,mod))
	    {
		compute_autom_fullerenes(degree,numbering,&nbtot,&nbop);
	        ADDBIG(numfull,1);
		scantree(nbtot,nbop);
	    }

#if CPUTIME
        times(&timestruct1);
#endif

	dosummary = 1;
#ifdef PLUGIN
	SUMMARY();
#endif

	if (!dosummary) return 0;

	PRINTBIG(msgfile,numfull);
	fprintf(msgfile," fullerenes used");
	if (K4used) fprintf(msgfile," plus K4");
#if CPUTIME
        fprintf(msgfile,"; cpu=%.2f sec\n",
            (double)(timestruct1.tms_cutime+timestruct1.tms_cstime
              -timestruct0.tms_cutime-timestruct0.tms_cstime)/(double)CLK_TCK);
#else
        fprintf(msgfile,"\n");
#endif

	PRINTBIG(msgfile,nout);
	if (dswitch) fprintf(msgfile," cubic graphs");
	else         fprintf(msgfile," triangulations");
	if (uswitch) fprintf(msgfile," generated");
	else         fprintf(msgfile," written to %s",outfilename);
#if CPUTIME
        fprintf(msgfile,"; cpu=%.2f sec\n",
            (double)(timestruct1.tms_utime+timestruct1.tms_stime
              -timestruct0.tms_utime-timestruct0.tms_stime)/(double)CLK_TCK);
#else
        fprintf(msgfile,"\n");
#endif

#if STATS
	if (gswitch)
	{
	    PRINTBIG(msgfile,numrooted);
	    fprintf(msgfile," rooted triangulations\n");
	    fprintf(msgfile,"Number with trivial group: ");
	    PRINTBIG(msgfile,ntriv);
	    fprintf(msgfile,"\n");
	}
	fprintf(msgfile,"Counts by min degree: ");
	if (minnv < maxnv) fprintf(msgfile,"\n");
	for (nv = minnv; nv <= maxnv; ++nv)
	{
	    total = nummindeg[0][nv]+nummindeg[1][nv]+nummindeg[2][nv];
	    if (total != 0)
	        fprintf(msgfile,"%3d: %7lu %7lu %7lu  %8lu\n",
		    nv,nummindeg[0][nv],nummindeg[1][nv],
		    nummindeg[2][nv],total);
	}
#endif

	return 0;
}


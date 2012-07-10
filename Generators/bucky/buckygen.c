#define VERSION "1.0 - May 31 2012"
#define SWITCHES "[-uagsh -IS#rq -odV -v]"

/* buckygen.c :  generate fullerenes.

  This program generates fullerenes: triangulations where all vertices
  have degree 5 or 6. Or if the dual representation is used: cubic plane 
  graphs where all faces are pentagons or hexagons. Euler's formula 
  implies that a fullerene contains exactly 12 degree 5 vertices.

  Buckygen can also generate IPR fullerenes efficiently, these 
  are fullerenes which have no adjacent degree 5 vertices. 

  Exact specifications and instructions for use can be found in the
  separate manual buckygen-guide.txt.
  
  The latest version of buckygen can be found here:
  http://caagt.ugent.be/buckygen/
  
  Author: Jan Goedgebeur (jan.goedgebeur@ugent.be)
  In collaboration with: Gunnar Brinkmann (gunnar.brinkmann@ugent.be) 
                         Brendan McKay (bdm@cs.anu.edu.au)

  Buckygen uses the isomorphism rejection routines and switches from 
  plantri 4.1. More information about plantri can be found here:
  http://cs.anu.edu.au/~bdm/plantri/

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

      Using -g or -s, the graph6 or sparse6 formats can be selected instead.
      These are ASCII formats for general undirected graphs, and do not
      encode the imbedding.  graph6 does not represent loops or edge
      multiplicities either.  They are described in buckygen-guide.txt.

---------

 Size limits:

  The space used by buckygen is O(n^2).  Apart from that, the only
  practical limits on MAXN (the maximum permitted number of vertices)
  is determined by the limits imposed by the output syntax.

  The following table gives the largest legal MAXN value.

   Switches:    none    -d    -a    -ad     -g    -gd  -s    -sd      -u
   Output:      planar_code     ascii        graph6     sparse6      none
               primal  dual  primal dual  primal dual  primal dual
   MAXN limit:  255    129     99     51   255   129    255   129    1023

  The limits for ascii code could be raised to 114 and 59 easily.
  For connectivity < 3, there is also a limit on n of the number of
  bits in a long int (usually 32 or 64).
 
---------

  Copyright (c) 2012 Ghent University

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.  
 
 ---------

  Change History:

        31-May-2012 : Initial release of version 1.0.

**************************************************************************/

#include <stdio.h>

#if __STDC__
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#else
extern int errno;
#endif

/******************************Configuration***********************************/

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


//Uncomment to turn debugmode on
//#define _DEBUG

#ifndef MAXN
#define MAXN 152            /* the maximum number of vertices; see above */
#endif

 
/*
 * For modulo when splitting the generation into several parts. 
 * All fullerenes below this level are generated by all parts.
 * Increasing this level will make it more likely that the parts
 * have a more or less equal execution time.
 * Normally the default values should be good enough.
 */
//#define MAX_SPLITLEVEL 70 //Common part takes about 3 minutes
//#define MAX_SPLITLEVEL_IPR 80 //Common part takes about 2 minutes
#define MAX_SPLITLEVEL 65 //Common part takes about 1.5 minutes
#define MAX_SPLITLEVEL_IPR 75 //Common part takes about 0.5 minutes


/***************You should not change anything below this line*****************/

//Macros for debugging
#ifdef _DEBUG
#define DEBUGASSERT(assertion) if(!(assertion)) {fprintf(stderr, "%s:%u Assertion failed: %s\n", __FILE__, __LINE__, #assertion); exit(1);}
#else
#define DEBUGASSERT(assertion)
#endif

#define MAXE (6*MAXN-12)   /* the maximum number of oriented edges */
#define MAXF (2*MAXN-4)    /* the maximum number of faces */

typedef struct e /* The data type used for edges */
{
    int start;         /* vertex where the edge starts */
    int end;           /* vertex where the edge ends */
    int rightface;     /* face on the right side of the edge
                          note: only valid if make_dual() called */
    struct e *prev;    /* previous edge in clockwise direction */
    struct e *next;    /* next edge in clockwise direction */
    struct e *invers;  /* the edge that is inverse to this one */
    //struct e *min;     /* the least of e and e->invers */
                       /* Important: this field is not set when generating fullerenes. */
    int mark,index;    /* two ints for temporary use;
                          Only access mark via the MARK macros. */
    int label;         /* the label of the edge (0 <= label < ne)
                          Used in ISMARKED_DOUBLE_NEXT/PREV */
} EDGE;


#undef FALSE
#undef TRUE
#define FALSE 0
#define TRUE  1


/**********Some defines which should normally not be modified**************/

/*
 * The max nv investigated by canon_edge_oriented_short. Experimental results
 * showed that 40 is more or less optimal.
 */
#define NV_CANON_SHORT 40


/*
 * Extra bounding criterion for max pathlength in case of IPR.
 * 
 * If nv > LEVEL_THREE_INDEPENDENT, it first checks if there are 3 independent
 * short reductions. If that's the case, no long operations can destroy all of them.
 * 
 * If nv <= 70, only 2.48% has > 2 indep short reductions
 * If nv <= 130, 84% of them have > 2 such reductions
 * 
 * The default value (60) seems to be more or less optimal.
 */
#define LEVEL_THREE_INDEPENDENT 60


/***********************Methods for splay-tree*****************************/

/* 
 * Methods for the splay-tree (see splay.c for more information). 
 * Is used to make sure that no isomorphic irreducible ipr fullerenes
 * are generated.
 */

//Temp, for debugging only
#define IRRED_CAP_5_5 1
#define IRRED_CAP_2_8 2
#define IRRED_CAP_8_2 3
#define IRRED_CAP_9_0 4
#define IRRED_CAP_10_0 5

typedef struct sp {
    unsigned char *graph;
    int length;
    int type; //Cap type, is for debugging purposes only
    struct sp *left, *right, *parent;
} SPLAYNODE;


SPLAYNODE *worklist = NULL;

/* extra arguments to pass to scan procedure */
#define SCAN_ARGS

/* what scan procedure should do for each node p */
#define ACTION(p) outputnode(p)

/* extra arguments for the insertion procedure */
#define INSERT_ARGS , unsigned char *canong, int codelength, int type, int *is_new_node

/* how to compare the key of INSERT_ARGS to the key of node p */
#define COMPARE(p) comparenodes(canong, codelength, type, p)

/* what to do for a new node p */
#define NOT_PRESENT(p) new_splaynode(p, canong, codelength, type, is_new_node)

/* what to do for an old code p */
#define PRESENT(p) old_splaynode(p, is_new_node)


void new_splaynode();
void old_splaynode();

#include "splay.c"


/**************************Global variables********************************/

static char *outfilename;  /* name of output file (NULL for stdout) */
static FILE *outfile;      /* output file for graphs */
static FILE *msgfile;      /* file for informational messages */

/* Filenames for files where fullerenes without spirals are written to */
#define MAX_FILENAME_LENGTH 200 
static char no_penta_spiral_filename[MAX_FILENAME_LENGTH];
static char no_spiral_filename[MAX_FILENAME_LENGTH];

static int write_no_penta_spiral_header = 1;
static int write_no_spiral_header = 1;

static int maxnv;          /* order of output graphs */
static int res,mod;        /* res/mod from command line (default 0/1) */
static int splitlevel,
           splitcount;     /* used for res/mod splitting */

#ifdef PLUGIN
    static int splithint = -1; /* used by plugins to set splitting level */
#endif

static int aswitch,        /* presence of command-line switches */
           gswitch,
           sswitch,
           hswitch,
           dswitch,
           oswitch,
           qswitch,
           uswitch,
           vswitch,
           Vswitch,
           fulleriprswitch,  /* Only generate ipr fullerenes */
           startswitch,      /* Also output fullerenes with start_output =< n < maxnv vertices */
           spiralcheck;      /* Test if the fullerenes have a spiral */

static int zeroswitch;     /* Undocumented option -0 for debugging */

static int start_output = 0; /* If startswitch = 1, also fullerenes with start_output =< n < maxnv vertices
                                will be output, instead of only fullerenes with maxnv vertices */

static int dosummary;      /* used by plugin */
static char *cmdname;      /* points to arg[0] */

/* The variables below are used at each level of the iteration,
   updating and restoring as we move up and down the search tree */

static int nv;             /* number of vertices; they are 0..nv-1 */
static int ne;             /* number of directed edges (6*nv-12) */

//#define NUMEDGES (24+70*MAXN)
//Can use a smaller upper bound for fullerenes, eg:
#define NUMEDGES (MAXE+20)

static EDGE edges[NUMEDGES];

/* Used by the generator for fullerenes */
/* stack-like allocation of edges */
static EDGE *edge_top = edges;
#define NEWEDGE(p) if ((p = edge_top++) >= edges+NUMEDGES) \
   {fprintf(stderr,">E Ran out of edges\n"); exit(1);}
#define FREEEDGES(k) edge_top -= (k)
#define SAVE_EDGE_STATE(var) var = edge_top
#define RESTORE_EDGE_STATE(var) edge_top = var
#define FREE_EDGES edge_top = edges

static EDGE *edge_list[MAXN][MAXN]; /* pointer to the edge i->j;
                                       this array is only valid for edges which are currently part of the graph! */

static int degree[MAXN];   /* the degrees of the vertices */
static EDGE *firstedge[MAXN]; /* pointer to arbitrary edge out of vertex i. */
  /* This pointer may change during the run, so all one can rely on is that
     at any point it is "some" edge out of i */

static EDGE *facestart[MAXF]; /* an edge in the clockwise orientation of
                                 each face.  Only valid when computed. */
static int facesize[MAXF]; /* size of each face.  Only valid when computed. */

static EDGE *numbering[2*MAXE][MAXE];
  /* holds numberings produced by canon() or canon_edge() */


#define PCODE ">>planar_code<<"
#define PCODELEN (sizeof(PCODE)-1)    /* "-1" to avoid the null */
#define G6CODE ">>graph6<<"
#define G6CODELEN (sizeof(G6CODE)-1)    /* "-1" to avoid the null */
#define S6CODE ">>sparse6<<"
#define S6CODELEN (sizeof(S6CODE)-1)    /* "-1" to avoid the null */

static EDGE *code_edge = NULL;
/* if code_edge is not NULL, it is taken as the start for coding for
   ASCII or planar_code. Otherwise firstedge[0] is the start. This
   method implies comparatively few changes due to outputting
   triangulations of disks.

   In case of triangulations of disks, *code_edge should be an edge
   with the "outer" face on the left for the non-mirror case and on
   the right for the mirror case to have the outer face left of 1->2.

   In case of dual output (mirror image or not), the face on the left
   of *code_edge gets the number 1. So for duals of triangulations of
   disks, handing in an edge with the disk on the right outputs the
   "marked" vertex as 1.
*/


/* The program is so fast that the count of output graphs can quickly
   overflow a 32-bit integer.  Therefore, we use two long values
   for each count, with a ratio of 10^9 between them.  The macro
   ADDBIG adds a small number to one of these big numbers.
   BIGTODOUBLE converts a big number to a double (approximately).
   SUMBIGS adds a second big number into a first big number.
   SUBBIGS subtracts a second big number from a first big number.
   PRINTBIG prints a big number in decimal.
   ZEROBIG sets the value of a big number to 0.
   ISZEROBIG tests if the value is 0.
   SETBIG sets a big number to a value at most 10^9-1.
   ISEQBIG tests if two big numbers are equal.
*/

typedef struct
{
    long hi,lo;
} bigint;

#define ZEROBIG(big) big.hi = big.lo = 0L
#define ISZEROBIG(big) (big.lo == 0 && big.hi == 0)
#define SETBIG(big,value) {big.hi = 0L; big.lo = (value);}
#define ADDBIG(big,extra) if ((big.lo += (extra)) >= 1000000000L) \
    { ++big.hi; big.lo -= 1000000000L;}
#define PRINTBIG(file,big) if (big.hi == 0) \
 fprintf(file,"%ld",big.lo); else fprintf(file,"%ld%09ld",big.hi,big.lo)
#define BIGTODOUBLE(big) (big.hi * 1000000000.0 + big.lo)
#define SUMBIGS(big1,big2) {if ((big1.lo += big2.lo) >= 1000000000L) \
    {big1.lo -= 1000000000L; big1.hi += big2.hi + 1L;} \
    else big1.hi += big2.hi;}
#define SUBBIGS(big1,big2) {if ((big1.lo -= big2.lo) < 0L) \
    {big1.lo += 1000000000L; big1.hi -= big2.hi + 1L;} \
    else big1.hi -= big2.hi;}
/* Note: SUBBIGS must not allow the value to go negative.
   SUMBIGS and SUBBIGS both permit big1 and big2 to be the same bigint. */
#define ISEQBIG(big1,big2) (big1.lo == big2.lo && big1.hi == big2.hi)

static bigint nout[MAXN+1];   /* counts of output graphs, per nv */
static bigint totalout;       /* counts of output graphs */
static bigint totalout_op;    /* counts of output graphs, OP */
static bigint nout_V;         /* Deletions due to -V */

static char outtypename[50];  /* How to describe output objects */

#ifdef STATS    /* optional statistics collection */
static bigint numrooted[MAXN+1]; /* rooted maps (fails for connectivity 1) */
static bigint total_numrooted;

static bigint ntriv[MAXN+1]; /* counter of those with trivial groups
                                   (an upper bound only without -o) */
static bigint total_triv;
#endif

#ifdef SPLITTEST
static bigint splitcases;
#endif

/* Some more statistics */
//static unsigned long long int number_of_graphs_generated[MAXN+1] = {0};

//No need to use bigint since there are only very few fullerenes without spiral
static unsigned long long int number_without_pentagon_spiral[MAXN+1] = {0};
static unsigned long long int number_without_spiral[MAXN+1] = {0};


/**********************Macros for various marks****************************/

#define MAXVAL INT_MAX - 2
static int markvalue = MAXVAL;
#define RESETMARKS {int mki; if ((markvalue += 2) > MAXVAL) \
       { markvalue = 2; for (mki=0;mki<NUMEDGES;++mki) edges[mki].mark=0;}}
#define MARKLO(e) (e)->mark = markvalue
#define MARKHI(e) (e)->mark = markvalue+1
#define UNMARK(e) (e)->mark = markvalue-1
#define ISMARKED(e) ((e)->mark >= markvalue)
#define ISMARKEDLO(e) ((e)->mark == markvalue)
#define ISMARKEDHI(e) ((e)->mark > markvalue)

/* and the same for vertices */

static int markvalue_v = MAXVAL;
static int marks__v[MAXN];
#define RESETMARKS_V {int mki; if ((markvalue_v += 2) > MAXVAL) \
       { markvalue_v = 2; for (mki=0;mki<MAXN;++mki) marks__v[mki]=0;}}
#define UNMARK_V(x) (marks__v[x] = 0)
#define ISMARKED_V(x) (marks__v[x] >= markvalue_v)
#define ISMARKEDHI_V(x) (marks__v[x] > markvalue_v)
#define MARK_V(x) (marks__v[x] = markvalue_v)
#define MARKHI_V(x) (marks__v[x] = markvalue_v+1)

static int markvalue_v2 = MAXVAL;
static int marks__v2[MAXN];
#define RESETMARKS_V2 {int mki; if ((markvalue_v2 += 2) > MAXVAL) \
       { markvalue_v2 = 2; for (mki=0;mki<MAXN;++mki) marks__v2[mki]=0;}}
#define UNMARK_V2(x) (marks__v2[x] = 0)
#define ISMARKED_V2(x) (marks__v2[x] >= markvalue_v2)
#define MARK_V2(x) (marks__v2[x] = markvalue_v2)

static int markvalue_v3 = MAXVAL;
static int marks__v3[MAXN];
#define RESETMARKS_V3 {int mki; if ((markvalue_v3 += 2) > MAXVAL) \
       { markvalue_v3 = 2; for (mki=0;mki<MAXN;++mki) marks__v3[mki]=0;}}
#define UNMARK_V3(x) (marks__v3[x] = 0)
#define ISMARKED_V3(x) (marks__v3[x] >= markvalue_v3)
#define MARK_V3(x) (marks__v3[x] = markvalue_v3)

static int markvalue_v4 = MAXVAL;
static int marks__v4[MAXN];
#define RESETMARKS_V4 {int mki; if ((markvalue_v4 += 2) > MAXVAL) \
       { markvalue_v4 = 2; for (mki=0;mki<MAXN;++mki) marks__v4[mki]=0;}}
#define UNMARK_V4(x) (marks__v4[x] = 0)
#define ISMARKED_V4(x) (marks__v4[x] >= markvalue_v4)
#define MARK_V4(x) (marks__v4[x] = markvalue_v4)

static int markvalue_v5 = MAXVAL;
static int marks__v5[MAXN];
#define RESETMARKS_V5 {int mki; if ((markvalue_v5 += 2) > MAXVAL) \
       { markvalue_v5 = 2; for (mki=0;mki<MAXN;++mki) marks__v5[mki]=0;}}
#define UNMARK_V5(x) (marks__v5[x] = 0)
#define ISMARKED_V5(x) (marks__v5[x] >= markvalue_v5)
#define MARK_V5(x) (marks__v5[x] = markvalue_v5)

/* Max length of a straight expansion */
#define MAX_STRAIGHT_LENGTH (MAXN/3)

/* Marks for straight expansions */
//Access through edge->label
static int markvalue_double_next = MAXVAL;
static int marks__double_next[MAXE][MAX_STRAIGHT_LENGTH + 1];
#define RESETMARKS_DOUBLE_NEXT {int mki, mkj; if ((++markvalue_double_next) > MAXVAL) \
       { markvalue_double_next = 1; for (mki=0;mki<MAXE;++mki) for (mkj=0;mkj<=MAX_STRAIGHT_LENGTH;++mkj) marks__double_next[mki][mkj]=0;}}
#define UNMARK_DOUBLE_NEXT(x, y) (marks__double_next[x][y] = 0)
#define ISMARKED_DOUBLE_NEXT(x, y) (marks__double_next[x][y] == markvalue_double_next)
#define MARK_DOUBLE_NEXT(x, y) (marks__double_next[x][y] = markvalue_double_next)

static int markvalue_double_prev = MAXVAL;
static int marks__double_prev[MAXE][MAX_STRAIGHT_LENGTH + 1];
#define RESETMARKS_DOUBLE_PREV {int mki, mkj; if ((++markvalue_double_prev) > MAXVAL) \
       { markvalue_double_prev = 1; for (mki=0;mki<MAXE;++mki) for (mkj=0;mkj<=MAX_STRAIGHT_LENGTH;++mkj) marks__double_prev[mki][mkj]=0;}}
#define UNMARK_DOUBLE_PREV(x, y) (marks__double_prev[x][y] = 0)
#define ISMARKED_DOUBLE_PREV(x, y) (marks__double_prev[x][y] == markvalue_double_prev)
#define MARK_DOUBLE_PREV(x, y) (marks__double_prev[x][y] = markvalue_double_prev)

/* Marks for L0 expansions */
//Access through edge->label
static int markvalue_L0_next = MAXVAL;
static int marks_L0_next[MAXE];
#define RESETMARKS_L0_NEXT {int mki; if ((markvalue_L0_next += 2) > MAXVAL) \
       { markvalue_L0_next = 2; for(mki=0;mki<MAXE;++mki) marks_L0_next[mki]=0;}}
#define MARK_L0_NEXT(x) (marks_L0_next[x] = markvalue_L0_next)
#define UNMARK_L0_NEXT(x) (marks_L0_next[x] = 0)
#define ISMARKED_L0_NEXT(x) (marks_L0_next[x] == markvalue_L0_next)

static int markvalue_L0_prev = MAXVAL;
static int marks_L0_prev[MAXE];
#define RESETMARKS_L0_PREV {int mki; if ((markvalue_L0_prev += 2) > MAXVAL) \
       { markvalue_L0_prev = 2; for(mki=0;mki<MAXE;++mki) marks_L0_prev[mki]=0;}}
#define MARK_L0_PREV(x) (marks_L0_prev[x] = markvalue_L0_prev)
#define UNMARK_L0_PREV(x) (marks_L0_prev[x] = 0)
#define ISMARKED_L0_PREV(x) (marks_L0_prev[x] == markvalue_L0_prev)

/* Marks for B00 expansions */
//Access through edge->label
static int markvalue_b00_next = MAXVAL;
static int marks_b00_next[MAXE];
#define RESETMARKS_B00_NEXT {int mki; if ((markvalue_b00_next += 2) > MAXVAL) \
       { markvalue_b00_next = 2; for(mki=0;mki<MAXE;++mki) marks_b00_next[mki]=0;}}
#define MARK_B00_NEXT(x) (marks_b00_next[x] = markvalue_b00_next)
#define UNMARK_B00_NEXT(x) (marks_b00_next[x] = 0)
#define ISMARKED_B00_NEXT(x) (marks_b00_next[x] == markvalue_b00_next)

static int markvalue_b00_prev = MAXVAL;
static int marks_b00_prev[MAXE];
#define RESETMARKS_B00_PREV {int mki; if ((markvalue_b00_prev += 2) > MAXVAL) \
       { markvalue_b00_prev = 2; for(mki=0;mki<MAXE;++mki) marks_b00_prev[mki]=0;}}
#define MARK_B00_PREV(x) (marks_b00_prev[x] = markvalue_b00_prev)
#define UNMARK_B00_PREV(x) (marks_b00_prev[x] = 0)
#define ISMARKED_B00_PREV(x) (marks_b00_prev[x] == markvalue_b00_prev)


/* Marks for bent expansions */
//Access through edge->label
/* max_bent == max_pathlength_straight - 3 */
#define MAX_BENT_LENGTH (MAX_STRAIGHT_LENGTH - 3)

static int markvalue_bent_next = MAXVAL;
static int marks__bent_next[MAXE][MAX_BENT_LENGTH + 1][MAX_BENT_LENGTH + 1];
#define RESETMARKS_BENT_NEXT {int mki, mkj, mkk; if ((++markvalue_bent_next) > MAXVAL) \
       { markvalue_bent_next = 1; for (mki=0;mki<MAXE;++mki) for (mkj=0;mkj<=MAX_BENT_LENGTH;++mkj) for (mkk=0;mkk<=MAX_BENT_LENGTH;++mkk) marks__bent_next[mki][mkj][mkk]=0;}}
#define UNMARK_BENT_NEXT(x, y, z) (marks__bent_next[x][y][z] = 0)
#define ISMARKED_BENT_NEXT(x, y, z) (marks__bent_next[x][y][z] == markvalue_bent_next)
#define MARK_BENT_NEXT(x, y, z) (marks__bent_next[x][y][z] = markvalue_bent_next)

static int markvalue_bent_prev = MAXVAL;
static int marks__bent_prev[MAXE][MAX_BENT_LENGTH + 1][MAX_BENT_LENGTH + 1];
#define RESETMARKS_BENT_PREV {int mki, mkj, mkk; if ((++markvalue_bent_prev) > MAXVAL) \
       { markvalue_bent_prev = 1; for (mki=0;mki<MAXE;++mki) for (mkj=0;mkj<=MAX_BENT_LENGTH;++mkj) for (mkk=0;mkk<=MAX_BENT_LENGTH;++mkk) marks__bent_prev[mki][mkj][mkk]=0;}}
#define UNMARK_BENT_PREV(x, y, z) (marks__bent_prev[x][y][z] = 0)
#define ISMARKED_BENT_PREV(x, y, z) (marks__bent_prev[x][y][z] == markvalue_bent_prev)
#define MARK_BENT_PREV(x, y, z) (marks__bent_prev[x][y][z] = markvalue_bent_prev)

/* Marks for bent expansions with crossing paths */

#define MAX_DISTANCE MAXN

static int min_distance_next[MAXE];

static int markvalue_bent_crossing_next = MAXVAL;
static int marks_bent_crossing_next[MAXE];

#define RESETMARKS_BENT_CROSSING_NEXT {int mki; if ((markvalue_bent_crossing_next += 2) > MAXVAL) \
       { markvalue_bent_crossing_next = 2; for(mki=0;mki<MAXE;++mki) marks_bent_crossing_next[mki]=0;}}
#define MARK_BENT_CROSSING_NEXT(x) {marks_bent_crossing_next[x] = markvalue_bent_crossing_next; min_distance_next[x] = MAX_DISTANCE + 1;}
#define UNMARK_BENT_CROSSING_NEXT(x) (marks_bent_crossing_next[x] = 0)
#define ISMARKED_BENT_CROSSING_NEXT(x) (marks_bent_crossing_next[x] == markvalue_bent_crossing_next)

#define UPDATE_MIN_DISTANCE_NEXT(x,d) {if(min_distance_next[x] > d) min_distance_next[x] = d;}
#define MIN_DISTANCE_NEXT(x) min_distance_next[x]

static int min_distance_prev[MAXE];

static int markvalue_bent_crossing_prev = MAXVAL;
static int marks_bent_crossing_prev[MAXE];

#define RESETMARKS_BENT_CROSSING_PREV {int mki; if ((markvalue_bent_crossing_prev += 2) > MAXVAL) \
       { markvalue_bent_crossing_prev = 2; for(mki=0;mki<MAXE;++mki) marks_bent_crossing_prev[mki]=0;}}
#define MARK_BENT_CROSSING_PREV(x) {marks_bent_crossing_prev[x] = markvalue_bent_crossing_prev; min_distance_prev[x] = MAX_DISTANCE + 1;}
#define UNMARK_BENT_CROSSING_PREV(x) (marks_bent_crossing_prev[x] = 0)
#define ISMARKED_BENT_CROSSING_PREV(x) (marks_bent_crossing_prev[x] == markvalue_bent_crossing_prev)

#define UPDATE_MIN_DISTANCE_PREV(x,d) {if(min_distance_prev[x] > d) min_distance_prev[x] = d;}
#define MIN_DISTANCE_PREV(x) min_distance_prev[x]


/***********************Some other useful macros***************************/

#define CHECKSWITCH(name) check_switch(name,ok_switches)
#define INCOMPAT(cond,x,y) if (cond) \
  {fprintf(stderr,">E %s: %s and %s are incompatible\n",cmdname,x,y); exit(1);}
#define CHECKRANGE(var,varname,lo,hi) if ((var)<(lo)||(var)>(hi)) \
  {fprintf(stderr,">E %s: the value of %s must be ",cmdname,varname); \
  if ((lo)==(hi)) \
    fprintf(stderr,"%d\n",lo); else fprintf(stderr,"%d..%d\n",lo,hi); \
    exit(1);}
#define PERROR(cond,msg) if (cond) \
   {fprintf(stderr,">E %s: %s\n",cmdname,msg); exit(1);}

#define BOOLSWITCH(name,var)  \
    else if (arg[j]==name) {CHECKSWITCH(name); var = TRUE;}
#define INTSWITCH(name,var)  \
    else if (arg[j]==name) {CHECKSWITCH(name); var = getswitchvalue(arg,&j);}

#define SECRET_SWITCHES "0"

#define MAX(x,y) ((x)<(y) ? (y) : (x))
#define MIN(x,y) ((x)>(y) ? (y) : (x))


/*************Global variables specifically for fullerenes*****************/

/* The irreducible graphs for the fullerene generator (in planar code) */
//length = 12 + 60 + 1
#define CODELENGTH_C20 73
unsigned char code_c20[CODELENGTH_C20] = {12, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 3, 0, 1, 2, 8, 9, 4, 0, 1, 3, 9, 10, 5, 0, 1, 4, 10, 11, 6, 0, 1, 5, 11, 7, 2, 0, 2, 6, 11, 12, 8, 0, 2, 7, 12, 9, 3, 0, 3, 8, 12, 10, 4, 0, 4, 9, 12, 11, 5, 0, 5, 10, 12, 7, 6, 0, 7, 11, 10, 9, 8, 0};

//length = 16 + 84 + 1
#define CODELENGTH_C28 101
unsigned char code_c28[CODELENGTH_C28] = {16, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 3, 0, 1, 2, 8, 9, 4, 0, 1, 3, 9, 10, 11, 5, 0, 1, 4, 11, 12, 6, 0, 1, 5, 12, 13, 7, 2, 0, 2, 6, 13, 14, 8, 0, 2, 7, 14, 16, 9, 3, 0, 3, 8, 16, 10, 4, 0, 4, 9, 16, 15, 11, 0, 4, 10, 15, 12, 5, 0, 5, 11, 15, 13, 6, 0, 6, 12, 15, 14, 7, 0, 7, 13, 15, 16, 8, 0, 10, 16, 14, 13, 12, 11, 0, 8, 14, 15, 10, 9, 0};

/* (5,0)-type nanotube with 30 vertices */
//length = 17 + 90 + 1
#define CODELENGTH_C30 108
unsigned char code_c30[CODELENGTH_C30] = {17, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 3, 0, 1, 2, 9, 10, 4, 0, 1, 3, 10, 11, 12, 5, 0, 1, 4, 12, 13, 6, 0, 1, 5, 13, 14, 7, 0, 1, 6, 14, 15, 8, 2, 0, 2, 7, 15, 16, 9, 0, 2, 8, 16, 10, 3, 0, 3, 9, 16, 11, 4, 0, 4, 10, 16, 15, 17, 12, 0, 4, 11, 17, 13, 5, 0, 5, 12, 17, 14, 6, 0, 6, 13, 17, 15, 7, 0, 7, 14, 17, 11, 16, 8, 0, 8, 15, 11, 10, 9, 0, 11, 15, 14, 13, 12, 0};


/* The codes of irreducible ipr fullerenes with a valid bound and no reducible hexagon rings */
#define NUM_IRRED_IPR_NO_RING 10
#define MAX_CODELENGTH_IRRED_NO_RING 318

int code_irred_ipr_lengths[NUM_IRRED_IPR_NO_RING] = {311, 297, 255, 283, 283, 318, 269, 213, 283, 283};

unsigned char code_irred_ipr[NUM_IRRED_IPR_NO_RING][MAX_CODELENGTH_IRRED_NO_RING] = {
    {46, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 10, 3, 0, 1, 2, 10, 11, 12, 4, 0, 1, 3, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 17, 7, 0, 1, 6, 17, 18, 8, 2, 0, 2, 7, 18, 19, 20, 9, 0, 2, 8, 20, 21, 10, 0, 2, 9, 21, 22, 11, 3, 0, 3, 10, 22, 23, 12, 0, 3, 11, 23, 24, 13, 4, 0, 4, 12, 24, 25, 14, 5, 0, 5, 13, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 29, 17, 0, 6, 16, 29, 30, 18, 7, 0, 7, 17, 30, 31, 19, 8, 0, 8, 18, 31, 32, 20, 0, 8, 19, 32, 33, 21, 9, 0, 9, 20, 33, 34, 22, 10, 0, 10, 21, 34, 35, 23, 11, 0, 11, 22, 35, 36, 24, 12, 0, 12, 23, 36, 25, 13, 0, 13, 24, 36, 37, 26, 14, 0, 14, 25, 37, 38, 27, 15, 0, 15, 26, 38, 28, 16, 0, 16, 27, 38, 39, 40, 29, 0, 16, 28, 40, 41, 30, 17, 0, 17, 29, 41, 31, 18, 0, 18, 30, 41, 42, 32, 19, 0, 19, 31, 42, 43, 33, 20, 0, 20, 32, 43, 34, 21, 0, 21, 33, 43, 44, 35, 22, 0, 22, 34, 44, 45, 36, 23, 0, 23, 35, 45, 37, 25, 24, 0, 25, 36, 45, 38, 26, 0, 26, 37, 45, 39, 28, 27, 0, 28, 38, 45, 44, 46, 40, 0, 28, 39, 46, 41, 29, 0, 29, 40, 46, 42, 31, 30, 0, 31, 41, 46, 43, 32, 0, 32, 42, 46, 44, 34, 33, 0, 34, 43, 46, 39, 45, 35, 0, 35, 44, 39, 38, 37, 36, 0, 39, 44, 43, 42, 41, 40, 0},
    {44, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 19, 0, 8, 18, 30, 31, 20, 9, 0, 9, 19, 31, 32, 21, 10, 0, 10, 20, 32, 33, 22, 0, 10, 21, 33, 23, 12, 11, 0, 12, 22, 33, 34, 24, 0, 12, 23, 34, 25, 14, 13, 0, 14, 24, 34, 35, 36, 26, 0, 14, 25, 36, 37, 27, 15, 0, 15, 26, 37, 38, 28, 16, 0, 16, 27, 38, 29, 17, 0, 17, 28, 38, 39, 30, 18, 0, 18, 29, 39, 40, 31, 19, 0, 19, 30, 40, 41, 32, 20, 0, 20, 31, 41, 42, 33, 21, 0, 21, 32, 42, 34, 23, 22, 0, 23, 33, 42, 35, 25, 24, 0, 25, 34, 42, 41, 43, 36, 0, 25, 35, 43, 44, 37, 26, 0, 26, 36, 44, 38, 27, 0, 27, 37, 44, 39, 29, 28, 0, 29, 38, 44, 40, 30, 0, 30, 39, 44, 43, 41, 31, 0, 31, 40, 43, 35, 42, 32, 0, 32, 41, 35, 34, 33, 0, 35, 41, 40, 44, 36, 0, 36, 43, 40, 39, 38, 37, 0},
    {38, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 10, 3, 0, 1, 2, 10, 11, 12, 4, 0, 1, 3, 12, 13, 14, 5, 0, 1, 4, 14, 15, 16, 6, 0, 1, 5, 16, 17, 18, 7, 0, 1, 6, 18, 19, 8, 2, 0, 2, 7, 19, 20, 9, 0, 2, 8, 20, 21, 22, 10, 0, 2, 9, 22, 11, 3, 0, 3, 10, 22, 23, 24, 12, 0, 3, 11, 24, 13, 4, 0, 4, 12, 24, 25, 26, 14, 0, 4, 13, 26, 15, 5, 0, 5, 14, 26, 27, 28, 16, 0, 5, 15, 28, 17, 6, 0, 6, 16, 28, 29, 30, 18, 0, 6, 17, 30, 19, 7, 0, 7, 18, 30, 31, 20, 8, 0, 8, 19, 31, 32, 21, 9, 0, 9, 20, 32, 33, 22, 0, 9, 21, 33, 23, 11, 10, 0, 11, 22, 33, 34, 24, 0, 11, 23, 34, 25, 13, 12, 0, 13, 24, 34, 35, 26, 0, 13, 25, 35, 27, 15, 14, 0, 15, 26, 35, 36, 28, 0, 15, 27, 36, 29, 17, 16, 0, 17, 28, 36, 37, 30, 0, 17, 29, 37, 31, 19, 18, 0, 19, 30, 37, 32, 20, 0, 20, 31, 37, 38, 33, 21, 0, 21, 32, 38, 34, 23, 22, 0, 23, 33, 38, 35, 25, 24, 0, 25, 34, 38, 36, 27, 26, 0, 27, 35, 38, 37, 29, 28, 0, 29, 36, 38, 32, 31, 30, 0, 32, 37, 36, 35, 34, 33, 0},
    {42, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 18, 8, 0, 2, 7, 18, 19, 20, 9, 0, 2, 8, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 0, 7, 17, 29, 30, 19, 8, 0, 8, 18, 30, 31, 32, 20, 0, 8, 19, 32, 21, 10, 9, 0, 10, 20, 32, 33, 22, 0, 10, 21, 33, 23, 12, 11, 0, 12, 22, 33, 34, 24, 0, 12, 23, 34, 25, 14, 13, 0, 14, 24, 34, 35, 36, 26, 0, 14, 25, 36, 37, 27, 15, 0, 15, 26, 37, 28, 16, 0, 16, 27, 37, 38, 29, 17, 0, 17, 28, 38, 39, 30, 18, 0, 18, 29, 39, 31, 19, 0, 19, 30, 39, 40, 41, 32, 0, 19, 31, 41, 33, 21, 20, 0, 21, 32, 41, 34, 23, 22, 0, 23, 33, 41, 35, 25, 24, 0, 25, 34, 41, 40, 42, 36, 0, 25, 35, 42, 37, 26, 0, 26, 36, 42, 38, 28, 27, 0, 28, 37, 42, 39, 29, 0, 29, 38, 42, 40, 31, 30, 0, 31, 39, 42, 35, 41, 0, 31, 40, 35, 34, 33, 32, 0, 35, 40, 39, 38, 37, 36, 0},
    {42, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 18, 8, 0, 2, 7, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 0, 7, 17, 29, 30, 19, 8, 0, 8, 18, 30, 31, 20, 9, 0, 9, 19, 31, 32, 21, 10, 0, 10, 20, 32, 33, 22, 0, 10, 21, 33, 23, 12, 11, 0, 12, 22, 33, 34, 24, 0, 12, 23, 34, 25, 14, 13, 0, 14, 24, 34, 35, 36, 26, 0, 14, 25, 36, 37, 27, 15, 0, 15, 26, 37, 28, 16, 0, 16, 27, 37, 38, 29, 17, 0, 17, 28, 38, 39, 30, 18, 0, 18, 29, 39, 31, 19, 0, 19, 30, 39, 40, 32, 20, 0, 20, 31, 40, 41, 33, 21, 0, 21, 32, 41, 34, 23, 22, 0, 23, 33, 41, 35, 25, 24, 0, 25, 34, 41, 40, 42, 36, 0, 25, 35, 42, 37, 26, 0, 26, 36, 42, 38, 28, 27, 0, 28, 37, 42, 39, 29, 0, 29, 38, 42, 40, 31, 30, 0, 31, 39, 42, 35, 41, 32, 0, 32, 40, 35, 34, 33, 0, 35, 40, 39, 38, 37, 36, 0},
    {47, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 10, 3, 0, 3, 9, 19, 20, 21, 11, 0, 3, 10, 21, 12, 4, 0, 4, 11, 21, 22, 23, 13, 0, 4, 12, 23, 14, 5, 0, 5, 13, 23, 24, 25, 15, 0, 5, 14, 25, 16, 6, 0, 6, 15, 25, 26, 17, 7, 0, 7, 16, 26, 27, 18, 8, 0, 8, 17, 27, 28, 29, 19, 0, 8, 18, 29, 20, 10, 9, 0, 10, 19, 29, 30, 31, 21, 0, 10, 20, 31, 22, 12, 11, 0, 12, 21, 31, 32, 33, 23, 0, 12, 22, 33, 24, 14, 13, 0, 14, 23, 33, 34, 35, 25, 0, 14, 24, 35, 26, 16, 15, 0, 16, 25, 35, 36, 27, 17, 0, 17, 26, 36, 37, 28, 18, 0, 18, 27, 37, 38, 39, 29, 0, 18, 28, 39, 30, 20, 19, 0, 20, 29, 39, 40, 41, 31, 0, 20, 30, 41, 32, 22, 21, 0, 22, 31, 41, 42, 43, 33, 0, 22, 32, 43, 34, 24, 23, 0, 24, 33, 43, 44, 45, 35, 0, 24, 34, 45, 36, 26, 25, 0, 26, 35, 45, 46, 37, 27, 0, 27, 36, 46, 38, 28, 0, 28, 37, 46, 47, 40, 39, 0, 28, 38, 40, 30, 29, 0, 30, 39, 38, 47, 42, 41, 0, 30, 40, 42, 32, 31, 0, 32, 41, 40, 47, 44, 43, 0, 32, 42, 44, 34, 33, 0, 34, 43, 42, 47, 46, 45, 0, 34, 44, 46, 36, 35, 0, 36, 45, 44, 47, 38, 37, 0, 38, 46, 44, 42, 40, 0},
    {40, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 10, 3, 0, 3, 9, 19, 20, 21, 11, 0, 3, 10, 21, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 16, 6, 0, 6, 15, 26, 27, 17, 7, 0, 7, 16, 27, 28, 18, 8, 0, 8, 17, 28, 29, 19, 0, 8, 18, 29, 20, 10, 9, 0, 10, 19, 29, 30, 31, 21, 0, 10, 20, 31, 22, 11, 0, 11, 21, 31, 32, 23, 12, 0, 12, 22, 32, 33, 34, 24, 0, 12, 23, 34, 25, 14, 13, 0, 14, 24, 34, 35, 36, 26, 0, 14, 25, 36, 27, 16, 15, 0, 16, 26, 36, 37, 28, 17, 0, 17, 27, 37, 38, 29, 18, 0, 18, 28, 38, 30, 20, 19, 0, 20, 29, 38, 39, 31, 0, 20, 30, 39, 32, 22, 21, 0, 22, 31, 39, 33, 23, 0, 23, 32, 39, 40, 35, 34, 0, 23, 33, 35, 25, 24, 0, 25, 34, 33, 40, 37, 36, 0, 25, 35, 37, 27, 26, 0, 27, 36, 35, 40, 38, 28, 0, 28, 37, 40, 39, 30, 29, 0, 30, 38, 40, 33, 32, 31, 0, 33, 39, 38, 37, 35, 0},
    {32, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 10, 3, 0, 3, 9, 19, 20, 21, 11, 0, 3, 10, 21, 12, 4, 0, 4, 11, 21, 22, 23, 13, 0, 4, 12, 23, 14, 5, 0, 5, 13, 23, 24, 25, 15, 0, 5, 14, 25, 16, 6, 0, 6, 15, 25, 26, 17, 7, 0, 7, 16, 26, 27, 18, 8, 0, 8, 17, 27, 28, 19, 0, 8, 18, 28, 20, 10, 9, 0, 10, 19, 28, 29, 21, 0, 10, 20, 29, 22, 12, 11, 0, 12, 21, 29, 30, 23, 0, 12, 22, 30, 24, 14, 13, 0, 14, 23, 30, 31, 25, 0, 14, 24, 31, 26, 16, 15, 0, 16, 25, 31, 27, 17, 0, 17, 26, 31, 32, 28, 18, 0, 18, 27, 32, 29, 20, 19, 0, 20, 28, 32, 30, 22, 21, 0, 22, 29, 32, 31, 24, 23, 0, 24, 30, 32, 27, 26, 25, 0, 27, 31, 30, 29, 28, 0},
    {42, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 10, 3, 0, 1, 2, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 0, 1, 6, 16, 17, 8, 2, 0, 2, 7, 17, 18, 9, 0, 2, 8, 18, 19, 20, 10, 0, 2, 9, 20, 21, 11, 3, 0, 3, 10, 21, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 19, 9, 0, 9, 18, 30, 31, 32, 20, 0, 9, 19, 32, 21, 10, 0, 10, 20, 32, 33, 22, 11, 0, 11, 21, 33, 23, 12, 0, 12, 22, 33, 34, 35, 24, 0, 12, 23, 35, 25, 14, 13, 0, 14, 24, 35, 36, 37, 26, 0, 14, 25, 37, 27, 15, 0, 15, 26, 37, 38, 28, 16, 0, 16, 27, 38, 29, 17, 0, 17, 28, 38, 39, 30, 18, 0, 18, 29, 39, 31, 19, 0, 19, 30, 39, 40, 41, 32, 0, 19, 31, 41, 33, 21, 20, 0, 21, 32, 41, 34, 23, 22, 0, 23, 33, 41, 40, 36, 35, 0, 23, 34, 36, 25, 24, 0, 25, 35, 34, 40, 42, 37, 0, 25, 36, 42, 38, 27, 26, 0, 27, 37, 42, 39, 29, 28, 0, 29, 38, 42, 40, 31, 30, 0, 31, 39, 42, 36, 34, 41, 0, 31, 40, 34, 33, 32, 0, 36, 40, 39, 38, 37, 0},
    {42, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 0, 1, 6, 16, 17, 8, 2, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 19, 0, 8, 18, 30, 31, 20, 9, 0, 9, 19, 31, 21, 10, 0, 10, 20, 31, 32, 33, 22, 0, 10, 21, 33, 23, 12, 11, 0, 12, 22, 33, 34, 35, 24, 0, 12, 23, 35, 25, 14, 13, 0, 14, 24, 35, 36, 37, 26, 0, 14, 25, 37, 27, 15, 0, 15, 26, 37, 38, 28, 16, 0, 16, 27, 38, 29, 17, 0, 17, 28, 38, 39, 30, 18, 0, 18, 29, 39, 40, 31, 19, 0, 19, 30, 40, 32, 21, 20, 0, 21, 31, 40, 41, 34, 33, 0, 21, 32, 34, 23, 22, 0, 23, 33, 32, 41, 36, 35, 0, 23, 34, 36, 25, 24, 0, 25, 35, 34, 41, 42, 37, 0, 25, 36, 42, 38, 27, 26, 0, 27, 37, 42, 39, 29, 28, 0, 29, 38, 42, 41, 40, 30, 0, 30, 39, 41, 32, 31, 0, 32, 40, 39, 42, 36, 34, 0, 36, 41, 39, 38, 37, 0}
};


/* The codes of irreducible ipr fullerenes without a valid boundary */
#define NUM_IRRED_IPR_INVALID_BOUNDARY 36
#define MAX_CODELENGTH_IRRED_INVALID_BOUNDARY 395

int code_irred_ipr_invalid_boundary_lengths[NUM_IRRED_IPR_INVALID_BOUNDARY] = {346, 290, 381, 297, 311, 339, 304, 395, 297, 290, 290, 318, 325, 325, 297, 297, 297, 297, 297, 297, 297, 297, 290, 290, 290, 290, 290, 283, 283, 276, 269, 262, 248, 283, 276, 311};

unsigned char code_irred_ipr_invalid_boundary[NUM_IRRED_IPR_INVALID_BOUNDARY][MAX_CODELENGTH_IRRED_INVALID_BOUNDARY] = {
    {51, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 0, 1, 6, 16, 17, 8, 2, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 25, 14, 5, 0, 5, 13, 25, 26, 27, 15, 0, 5, 14, 27, 28, 16, 6, 0, 6, 15, 28, 29, 17, 7, 0, 7, 16, 29, 30, 18, 8, 0, 8, 17, 30, 31, 32, 19, 0, 8, 18, 32, 20, 9, 0, 9, 19, 32, 33, 21, 10, 0, 10, 20, 33, 34, 35, 22, 0, 10, 21, 35, 23, 12, 11, 0, 12, 22, 35, 36, 37, 24, 0, 12, 23, 37, 25, 13, 0, 13, 24, 37, 38, 26, 14, 0, 14, 25, 38, 39, 40, 27, 0, 14, 26, 40, 41, 28, 15, 0, 15, 27, 41, 29, 16, 0, 16, 28, 41, 42, 30, 17, 0, 17, 29, 42, 43, 31, 18, 0, 18, 30, 43, 44, 32, 0, 18, 31, 44, 33, 20, 19, 0, 20, 32, 44, 45, 34, 21, 0, 21, 33, 45, 46, 36, 35, 0, 21, 34, 36, 23, 22, 0, 23, 35, 34, 46, 47, 37, 0, 23, 36, 47, 38, 25, 24, 0, 25, 37, 47, 39, 26, 0, 26, 38, 47, 48, 49, 40, 0, 26, 39, 49, 41, 27, 0, 27, 40, 49, 42, 29, 28, 0, 29, 41, 49, 50, 43, 30, 0, 30, 42, 50, 51, 44, 31, 0, 31, 43, 51, 45, 33, 32, 0, 33, 44, 51, 46, 34, 0, 34, 45, 51, 48, 47, 36, 0, 36, 46, 48, 39, 38, 37, 0, 39, 47, 46, 51, 50, 49, 0, 39, 48, 50, 42, 41, 40, 0, 42, 49, 48, 51, 43, 0, 43, 50, 48, 46, 45, 44, 0},
    {43, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 10, 3, 0, 1, 2, 10, 11, 12, 4, 0, 1, 3, 12, 13, 14, 5, 0, 1, 4, 14, 15, 16, 6, 0, 1, 5, 16, 17, 18, 7, 0, 1, 6, 18, 19, 8, 2, 0, 2, 7, 19, 20, 9, 0, 2, 8, 20, 21, 22, 10, 0, 2, 9, 22, 11, 3, 0, 3, 10, 22, 23, 24, 12, 0, 3, 11, 24, 13, 4, 0, 4, 12, 24, 25, 26, 14, 0, 4, 13, 26, 15, 5, 0, 5, 14, 26, 27, 28, 16, 0, 5, 15, 28, 17, 6, 0, 6, 16, 28, 29, 30, 18, 0, 6, 17, 30, 19, 7, 0, 7, 18, 30, 31, 20, 8, 0, 8, 19, 31, 32, 21, 9, 0, 9, 20, 32, 33, 22, 0, 9, 21, 33, 23, 11, 10, 0, 11, 22, 33, 34, 35, 24, 0, 11, 23, 35, 25, 13, 12, 0, 13, 24, 35, 36, 26, 0, 13, 25, 36, 27, 15, 14, 0, 15, 26, 36, 37, 38, 28, 0, 15, 27, 38, 29, 17, 16, 0, 17, 28, 38, 39, 30, 0, 17, 29, 39, 31, 19, 18, 0, 19, 30, 39, 40, 32, 20, 0, 20, 31, 40, 41, 33, 21, 0, 21, 32, 41, 34, 23, 22, 0, 23, 33, 41, 42, 35, 0, 23, 34, 42, 36, 25, 24, 0, 25, 35, 42, 37, 27, 26, 0, 27, 36, 42, 43, 38, 0, 27, 37, 43, 39, 29, 28, 0, 29, 38, 43, 40, 31, 30, 0, 31, 39, 43, 41, 32, 0, 32, 40, 43, 42, 34, 33, 0, 34, 41, 43, 37, 36, 35, 0, 37, 42, 41, 40, 39, 38, 0},
    {56, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 25, 14, 5, 0, 5, 13, 25, 26, 27, 15, 0, 5, 14, 27, 28, 16, 6, 0, 6, 15, 28, 29, 17, 7, 0, 7, 16, 29, 30, 18, 8, 0, 8, 17, 30, 31, 19, 0, 8, 18, 31, 32, 20, 9, 0, 9, 19, 32, 33, 21, 10, 0, 10, 20, 33, 34, 22, 0, 10, 21, 34, 23, 12, 11, 0, 12, 22, 34, 35, 36, 24, 0, 12, 23, 36, 37, 25, 13, 0, 13, 24, 37, 38, 26, 14, 0, 14, 25, 38, 39, 27, 0, 14, 26, 39, 40, 28, 15, 0, 15, 27, 40, 41, 29, 16, 0, 16, 28, 41, 42, 30, 17, 0, 17, 29, 42, 43, 31, 18, 0, 18, 30, 43, 44, 32, 19, 0, 19, 31, 44, 45, 33, 20, 0, 20, 32, 45, 46, 34, 21, 0, 21, 33, 46, 35, 23, 22, 0, 23, 34, 46, 47, 48, 36, 0, 23, 35, 48, 49, 37, 24, 0, 24, 36, 49, 38, 25, 0, 25, 37, 49, 50, 39, 26, 0, 26, 38, 50, 51, 40, 27, 0, 27, 39, 51, 41, 28, 0, 28, 40, 51, 52, 42, 29, 0, 29, 41, 52, 53, 43, 30, 0, 30, 42, 53, 44, 31, 0, 31, 43, 53, 54, 45, 32, 0, 32, 44, 54, 47, 46, 33, 0, 33, 45, 47, 35, 34, 0, 35, 46, 45, 54, 55, 48, 0, 35, 47, 55, 49, 36, 0, 36, 48, 55, 50, 38, 37, 0, 38, 49, 55, 56, 51, 39, 0, 39, 50, 56, 52, 41, 40, 0, 41, 51, 56, 53, 42, 0, 42, 52, 56, 54, 44, 43, 0, 44, 53, 56, 55, 47, 45, 0, 47, 54, 56, 50, 49, 48, 0, 50, 55, 54, 53, 52, 51, 0},
    {44, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 5, 0, 1, 4, 12, 13, 14, 6, 0, 1, 5, 14, 15, 16, 7, 0, 1, 6, 16, 17, 8, 2, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 23, 12, 4, 0, 4, 11, 23, 24, 13, 5, 0, 5, 12, 24, 25, 14, 0, 5, 13, 25, 26, 15, 6, 0, 6, 14, 26, 27, 28, 16, 0, 6, 15, 28, 29, 17, 7, 0, 7, 16, 29, 18, 8, 0, 8, 17, 29, 30, 31, 19, 0, 8, 18, 31, 20, 9, 0, 9, 19, 31, 32, 21, 10, 0, 10, 20, 32, 33, 34, 22, 0, 10, 21, 34, 35, 23, 11, 0, 11, 22, 35, 24, 12, 0, 12, 23, 35, 36, 25, 13, 0, 13, 24, 36, 37, 26, 14, 0, 14, 25, 37, 27, 15, 0, 15, 26, 37, 38, 39, 28, 0, 15, 27, 39, 29, 16, 0, 16, 28, 39, 30, 18, 17, 0, 18, 29, 39, 40, 41, 31, 0, 18, 30, 41, 32, 20, 19, 0, 20, 31, 41, 33, 21, 0, 21, 32, 41, 42, 43, 34, 0, 21, 33, 43, 35, 22, 0, 22, 34, 43, 36, 24, 23, 0, 24, 35, 43, 44, 37, 25, 0, 25, 36, 44, 38, 27, 26, 0, 27, 37, 44, 40, 39, 0, 27, 38, 40, 30, 29, 28, 0, 30, 39, 38, 44, 42, 41, 0, 30, 40, 42, 33, 32, 31, 0, 33, 41, 40, 44, 43, 0, 33, 42, 44, 36, 35, 34, 0, 36, 43, 42, 40, 38, 37, 0},
    {46, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 18, 8, 0, 2, 7, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 0, 7, 17, 29, 30, 19, 8, 0, 8, 18, 30, 31, 20, 9, 0, 9, 19, 31, 32, 21, 10, 0, 10, 20, 32, 33, 34, 22, 0, 10, 21, 34, 23, 12, 11, 0, 12, 22, 34, 35, 36, 24, 0, 12, 23, 36, 25, 14, 13, 0, 14, 24, 36, 37, 38, 26, 0, 14, 25, 38, 27, 15, 0, 15, 26, 38, 39, 28, 16, 0, 16, 27, 39, 40, 29, 17, 0, 17, 28, 40, 41, 30, 18, 0, 18, 29, 41, 31, 19, 0, 19, 30, 41, 42, 32, 20, 0, 20, 31, 42, 33, 21, 0, 21, 32, 42, 43, 35, 34, 0, 21, 33, 35, 23, 22, 0, 23, 34, 33, 43, 44, 36, 0, 23, 35, 44, 37, 25, 24, 0, 25, 36, 44, 45, 38, 0, 25, 37, 45, 39, 27, 26, 0, 27, 38, 45, 40, 28, 0, 28, 39, 45, 46, 41, 29, 0, 29, 40, 46, 42, 31, 30, 0, 31, 41, 46, 43, 33, 32, 0, 33, 42, 46, 44, 35, 0, 35, 43, 46, 45, 37, 36, 0, 37, 44, 46, 40, 39, 38, 0, 40, 45, 44, 43, 42, 41, 0},
    {50, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 10, 3, 0, 3, 9, 19, 20, 21, 11, 0, 3, 10, 21, 12, 4, 0, 4, 11, 21, 22, 23, 13, 0, 4, 12, 23, 14, 5, 0, 5, 13, 23, 24, 25, 15, 0, 5, 14, 25, 26, 16, 6, 0, 6, 15, 26, 27, 17, 7, 0, 7, 16, 27, 28, 18, 8, 0, 8, 17, 28, 29, 30, 19, 0, 8, 18, 30, 20, 10, 9, 0, 10, 19, 30, 31, 32, 21, 0, 10, 20, 32, 22, 12, 11, 0, 12, 21, 32, 33, 34, 23, 0, 12, 22, 34, 24, 14, 13, 0, 14, 23, 34, 35, 25, 0, 14, 24, 35, 36, 26, 15, 0, 15, 25, 36, 37, 27, 16, 0, 16, 26, 37, 38, 28, 17, 0, 17, 27, 38, 39, 29, 18, 0, 18, 28, 39, 40, 41, 30, 0, 18, 29, 41, 31, 20, 19, 0, 20, 30, 41, 42, 43, 32, 0, 20, 31, 43, 33, 22, 21, 0, 22, 32, 43, 44, 45, 34, 0, 22, 33, 45, 35, 24, 23, 0, 24, 34, 45, 46, 36, 25, 0, 25, 35, 46, 37, 26, 0, 26, 36, 46, 47, 38, 27, 0, 27, 37, 47, 48, 39, 28, 0, 28, 38, 48, 40, 29, 0, 29, 39, 48, 49, 42, 41, 0, 29, 40, 42, 31, 30, 0, 31, 41, 40, 49, 44, 43, 0, 31, 42, 44, 33, 32, 0, 33, 43, 42, 49, 50, 45, 0, 33, 44, 50, 46, 35, 34, 0, 35, 45, 50, 47, 37, 36, 0, 37, 46, 50, 48, 38, 0, 38, 47, 50, 49, 40, 39, 0, 40, 48, 50, 44, 42, 0, 44, 49, 48, 47, 46, 45, 0},
    {45, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 18, 8, 0, 2, 7, 18, 19, 20, 9, 0, 2, 8, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 25, 14, 5, 0, 5, 13, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 0, 7, 17, 29, 30, 19, 8, 0, 8, 18, 30, 31, 20, 0, 8, 19, 31, 21, 10, 9, 0, 10, 20, 31, 32, 33, 22, 0, 10, 21, 33, 23, 12, 11, 0, 12, 22, 33, 34, 35, 24, 0, 12, 23, 35, 36, 25, 13, 0, 13, 24, 36, 37, 26, 14, 0, 14, 25, 37, 38, 27, 15, 0, 15, 26, 38, 28, 16, 0, 16, 27, 38, 39, 29, 17, 0, 17, 28, 39, 40, 30, 18, 0, 18, 29, 40, 41, 31, 19, 0, 19, 30, 41, 32, 21, 20, 0, 21, 31, 41, 42, 34, 33, 0, 21, 32, 34, 23, 22, 0, 23, 33, 32, 42, 43, 35, 0, 23, 34, 43, 36, 24, 0, 24, 35, 43, 44, 37, 25, 0, 25, 36, 44, 38, 26, 0, 26, 37, 44, 39, 28, 27, 0, 28, 38, 44, 45, 40, 29, 0, 29, 39, 45, 41, 30, 0, 30, 40, 45, 42, 32, 31, 0, 32, 41, 45, 43, 34, 0, 34, 42, 45, 44, 36, 35, 0, 36, 43, 45, 39, 38, 37, 0, 39, 44, 43, 42, 41, 40, 0},
    {58, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 10, 3, 0, 1, 2, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 0, 1, 6, 16, 17, 8, 2, 0, 2, 7, 17, 18, 9, 0, 2, 8, 18, 19, 20, 10, 0, 2, 9, 20, 21, 11, 3, 0, 3, 10, 21, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 25, 14, 5, 0, 5, 13, 25, 26, 27, 15, 0, 5, 14, 27, 28, 16, 6, 0, 6, 15, 28, 29, 17, 7, 0, 7, 16, 29, 30, 18, 8, 0, 8, 17, 30, 31, 19, 9, 0, 9, 18, 31, 32, 33, 20, 0, 9, 19, 33, 34, 21, 10, 0, 10, 20, 34, 35, 22, 11, 0, 11, 21, 35, 23, 12, 0, 12, 22, 35, 36, 37, 24, 0, 12, 23, 37, 38, 25, 13, 0, 13, 24, 38, 39, 26, 14, 0, 14, 25, 39, 40, 41, 27, 0, 14, 26, 41, 28, 15, 0, 15, 27, 41, 42, 29, 16, 0, 16, 28, 42, 43, 30, 17, 0, 17, 29, 43, 44, 31, 18, 0, 18, 30, 44, 32, 19, 0, 19, 31, 44, 45, 46, 33, 0, 19, 32, 46, 47, 34, 20, 0, 20, 33, 47, 48, 35, 21, 0, 21, 34, 48, 36, 23, 22, 0, 23, 35, 48, 49, 50, 37, 0, 23, 36, 50, 38, 24, 0, 24, 37, 50, 51, 39, 25, 0, 25, 38, 51, 40, 26, 0, 26, 39, 51, 52, 53, 41, 0, 26, 40, 53, 42, 28, 27, 0, 28, 41, 53, 54, 43, 29, 0, 29, 42, 54, 55, 44, 30, 0, 30, 43, 55, 45, 32, 31, 0, 32, 44, 55, 56, 57, 46, 0, 32, 45, 57, 47, 33, 0, 33, 46, 57, 49, 48, 34, 0, 34, 47, 49, 36, 35, 0, 36, 48, 47, 57, 58, 50, 0, 36, 49, 58, 51, 38, 37, 0, 38, 50, 58, 52, 40, 39, 0, 40, 51, 58, 56, 54, 53, 0, 40, 52, 54, 42, 41, 0, 42, 53, 52, 56, 55, 43, 0, 43, 54, 56, 45, 44, 0, 45, 55, 54, 52, 58, 57, 0, 45, 56, 58, 49, 47, 46, 0, 49, 57, 56, 52, 51, 50, 0},
    {44, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 18, 8, 0, 2, 7, 18, 19, 20, 9, 0, 2, 8, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 0, 7, 17, 29, 30, 19, 8, 0, 8, 18, 30, 31, 20, 0, 8, 19, 31, 21, 10, 9, 0, 10, 20, 31, 32, 33, 22, 0, 10, 21, 33, 23, 12, 11, 0, 12, 22, 33, 34, 35, 24, 0, 12, 23, 35, 25, 14, 13, 0, 14, 24, 35, 36, 26, 0, 14, 25, 36, 37, 27, 15, 0, 15, 26, 37, 28, 16, 0, 16, 27, 37, 38, 29, 17, 0, 17, 28, 38, 39, 30, 18, 0, 18, 29, 39, 40, 31, 19, 0, 19, 30, 40, 32, 21, 20, 0, 21, 31, 40, 41, 34, 33, 0, 21, 32, 34, 23, 22, 0, 23, 33, 32, 41, 42, 35, 0, 23, 34, 42, 36, 25, 24, 0, 25, 35, 42, 43, 37, 26, 0, 26, 36, 43, 38, 28, 27, 0, 28, 37, 43, 44, 39, 29, 0, 29, 38, 44, 40, 30, 0, 30, 39, 44, 41, 32, 31, 0, 32, 40, 44, 42, 34, 0, 34, 41, 44, 43, 36, 35, 0, 36, 42, 44, 38, 37, 0, 38, 43, 42, 41, 40, 39, 0},
    {43, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 10, 3, 0, 3, 9, 19, 20, 21, 11, 0, 3, 10, 21, 12, 4, 0, 4, 11, 21, 22, 23, 13, 0, 4, 12, 23, 24, 14, 5, 0, 5, 13, 24, 25, 15, 0, 5, 14, 25, 26, 16, 6, 0, 6, 15, 26, 27, 17, 7, 0, 7, 16, 27, 28, 18, 8, 0, 8, 17, 28, 29, 30, 19, 0, 8, 18, 30, 20, 10, 9, 0, 10, 19, 30, 31, 32, 21, 0, 10, 20, 32, 22, 12, 11, 0, 12, 21, 32, 33, 23, 0, 12, 22, 33, 34, 24, 13, 0, 13, 23, 34, 35, 25, 14, 0, 14, 24, 35, 36, 26, 15, 0, 15, 25, 36, 27, 16, 0, 16, 26, 36, 37, 28, 17, 0, 17, 27, 37, 38, 29, 18, 0, 18, 28, 38, 39, 30, 0, 18, 29, 39, 31, 20, 19, 0, 20, 30, 39, 40, 32, 0, 20, 31, 40, 33, 22, 21, 0, 22, 32, 40, 41, 34, 23, 0, 23, 33, 41, 35, 24, 0, 24, 34, 41, 42, 36, 25, 0, 25, 35, 42, 37, 27, 26, 0, 27, 36, 42, 38, 28, 0, 28, 37, 42, 43, 39, 29, 0, 29, 38, 43, 40, 31, 30, 0, 31, 39, 43, 41, 33, 32, 0, 33, 40, 43, 42, 35, 34, 0, 35, 41, 43, 38, 37, 36, 0, 38, 42, 41, 40, 39, 0},
    {43, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 25, 14, 5, 0, 5, 13, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 31, 19, 0, 8, 18, 31, 20, 9, 0, 9, 19, 31, 32, 21, 10, 0, 10, 20, 32, 33, 22, 0, 10, 21, 33, 23, 12, 11, 0, 12, 22, 33, 34, 24, 0, 12, 23, 34, 35, 25, 13, 0, 13, 24, 35, 36, 26, 14, 0, 14, 25, 36, 37, 27, 15, 0, 15, 26, 37, 28, 16, 0, 16, 27, 37, 38, 29, 17, 0, 17, 28, 38, 30, 18, 0, 18, 29, 38, 39, 40, 31, 0, 18, 30, 40, 32, 20, 19, 0, 20, 31, 40, 41, 33, 21, 0, 21, 32, 41, 34, 23, 22, 0, 23, 33, 41, 42, 35, 24, 0, 24, 34, 42, 43, 36, 25, 0, 25, 35, 43, 37, 26, 0, 26, 36, 43, 38, 28, 27, 0, 28, 37, 43, 39, 30, 29, 0, 30, 38, 43, 42, 40, 0, 30, 39, 42, 41, 32, 31, 0, 32, 40, 42, 34, 33, 0, 34, 41, 40, 39, 43, 35, 0, 35, 42, 39, 38, 37, 36, 0},
    {47, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 10, 3, 0, 3, 9, 19, 20, 21, 11, 0, 3, 10, 21, 12, 4, 0, 4, 11, 21, 22, 23, 13, 0, 4, 12, 23, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 16, 6, 0, 6, 15, 26, 27, 17, 7, 0, 7, 16, 27, 28, 18, 8, 0, 8, 17, 28, 29, 30, 19, 0, 8, 18, 30, 20, 10, 9, 0, 10, 19, 30, 31, 32, 21, 0, 10, 20, 32, 22, 12, 11, 0, 12, 21, 32, 33, 23, 0, 12, 22, 33, 34, 24, 13, 0, 13, 23, 34, 35, 25, 14, 0, 14, 24, 35, 36, 26, 0, 14, 25, 36, 27, 16, 15, 0, 16, 26, 36, 37, 28, 17, 0, 17, 27, 37, 38, 29, 18, 0, 18, 28, 38, 39, 40, 30, 0, 18, 29, 40, 31, 20, 19, 0, 20, 30, 40, 41, 42, 32, 0, 20, 31, 42, 33, 22, 21, 0, 22, 32, 42, 43, 34, 23, 0, 23, 33, 43, 44, 35, 24, 0, 24, 34, 44, 45, 36, 25, 0, 25, 35, 45, 37, 27, 26, 0, 27, 36, 45, 46, 38, 28, 0, 28, 37, 46, 39, 29, 0, 29, 38, 46, 47, 41, 40, 0, 29, 39, 41, 31, 30, 0, 31, 40, 39, 47, 43, 42, 0, 31, 41, 43, 33, 32, 0, 33, 42, 41, 47, 44, 34, 0, 34, 43, 47, 46, 45, 35, 0, 35, 44, 46, 37, 36, 0, 37, 45, 44, 47, 39, 38, 0, 39, 46, 44, 43, 41, 0},
    {48, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 10, 3, 0, 1, 2, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 0, 1, 6, 16, 17, 8, 2, 0, 2, 7, 17, 18, 9, 0, 2, 8, 18, 19, 20, 10, 0, 2, 9, 20, 21, 11, 3, 0, 3, 10, 21, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 25, 14, 5, 0, 5, 13, 25, 26, 27, 15, 0, 5, 14, 27, 28, 16, 6, 0, 6, 15, 28, 29, 17, 7, 0, 7, 16, 29, 30, 18, 8, 0, 8, 17, 30, 31, 19, 9, 0, 9, 18, 31, 32, 33, 20, 0, 9, 19, 33, 21, 10, 0, 10, 20, 33, 34, 22, 11, 0, 11, 21, 34, 23, 12, 0, 12, 22, 34, 35, 36, 24, 0, 12, 23, 36, 37, 25, 13, 0, 13, 24, 37, 26, 14, 0, 14, 25, 37, 38, 39, 27, 0, 14, 26, 39, 28, 15, 0, 15, 27, 39, 40, 29, 16, 0, 16, 28, 40, 41, 30, 17, 0, 17, 29, 41, 42, 31, 18, 0, 18, 30, 42, 32, 19, 0, 19, 31, 42, 43, 44, 33, 0, 19, 32, 44, 34, 21, 20, 0, 21, 33, 44, 35, 23, 22, 0, 23, 34, 44, 43, 45, 36, 0, 23, 35, 45, 46, 37, 24, 0, 24, 36, 46, 38, 26, 25, 0, 26, 37, 46, 47, 39, 0, 26, 38, 47, 40, 28, 27, 0, 28, 39, 47, 41, 29, 0, 29, 40, 47, 48, 42, 30, 0, 30, 41, 48, 43, 32, 31, 0, 32, 42, 48, 45, 35, 44, 0, 32, 43, 35, 34, 33, 0, 35, 43, 48, 46, 36, 0, 36, 45, 48, 47, 38, 37, 0, 38, 46, 48, 41, 40, 39, 0, 41, 47, 46, 45, 43, 42, 0},
    {48, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 0, 1, 6, 16, 17, 8, 2, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 23, 12, 4, 0, 4, 11, 23, 24, 25, 13, 0, 4, 12, 25, 14, 5, 0, 5, 13, 25, 26, 27, 15, 0, 5, 14, 27, 28, 16, 6, 0, 6, 15, 28, 29, 17, 7, 0, 7, 16, 29, 30, 18, 8, 0, 8, 17, 30, 31, 19, 0, 8, 18, 31, 32, 20, 9, 0, 9, 19, 32, 21, 10, 0, 10, 20, 32, 33, 34, 22, 0, 10, 21, 34, 23, 11, 0, 11, 22, 34, 35, 24, 12, 0, 12, 23, 35, 36, 25, 0, 12, 24, 36, 26, 14, 13, 0, 14, 25, 36, 37, 38, 27, 0, 14, 26, 38, 28, 15, 0, 15, 27, 38, 39, 29, 16, 0, 16, 28, 39, 40, 30, 17, 0, 17, 29, 40, 41, 31, 18, 0, 18, 30, 41, 42, 32, 19, 0, 19, 31, 42, 33, 21, 20, 0, 21, 32, 42, 43, 44, 34, 0, 21, 33, 44, 35, 23, 22, 0, 23, 34, 44, 45, 36, 24, 0, 24, 35, 45, 37, 26, 25, 0, 26, 36, 45, 46, 47, 38, 0, 26, 37, 47, 39, 28, 27, 0, 28, 38, 47, 40, 29, 0, 29, 39, 47, 48, 41, 30, 0, 30, 40, 48, 42, 31, 0, 31, 41, 48, 43, 33, 32, 0, 33, 42, 48, 46, 45, 44, 0, 33, 43, 45, 35, 34, 0, 35, 44, 43, 46, 37, 36, 0, 37, 45, 43, 48, 47, 0, 37, 46, 48, 40, 39, 38, 0, 40, 47, 46, 43, 42, 41, 0},
    {44, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 10, 3, 0, 1, 2, 10, 11, 12, 4, 0, 1, 3, 12, 13, 14, 5, 0, 1, 4, 14, 15, 16, 6, 0, 1, 5, 16, 17, 7, 0, 1, 6, 17, 18, 8, 2, 0, 2, 7, 18, 19, 9, 0, 2, 8, 19, 20, 21, 10, 0, 2, 9, 21, 11, 3, 0, 3, 10, 21, 22, 23, 12, 0, 3, 11, 23, 13, 4, 0, 4, 12, 23, 24, 25, 14, 0, 4, 13, 25, 15, 5, 0, 5, 14, 25, 26, 27, 16, 0, 5, 15, 27, 28, 17, 6, 0, 6, 16, 28, 29, 18, 7, 0, 7, 17, 29, 30, 19, 8, 0, 8, 18, 30, 31, 20, 9, 0, 9, 19, 31, 32, 33, 21, 0, 9, 20, 33, 22, 11, 10, 0, 11, 21, 33, 34, 23, 0, 11, 22, 34, 24, 13, 12, 0, 13, 23, 34, 35, 36, 25, 0, 13, 24, 36, 26, 15, 14, 0, 15, 25, 36, 37, 38, 27, 0, 15, 26, 38, 39, 28, 16, 0, 16, 27, 39, 29, 17, 0, 17, 28, 39, 40, 30, 18, 0, 18, 29, 40, 41, 31, 19, 0, 19, 30, 41, 32, 20, 0, 20, 31, 41, 42, 43, 33, 0, 20, 32, 43, 34, 22, 21, 0, 22, 33, 43, 35, 24, 23, 0, 24, 34, 43, 42, 37, 36, 0, 24, 35, 37, 26, 25, 0, 26, 36, 35, 42, 44, 38, 0, 26, 37, 44, 39, 27, 0, 27, 38, 44, 40, 29, 28, 0, 29, 39, 44, 41, 30, 0, 30, 40, 44, 42, 32, 31, 0, 32, 41, 44, 37, 35, 43, 0, 32, 42, 35, 34, 33, 0, 37, 42, 41, 40, 39, 38, 0},
    {44, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 10, 3, 0, 1, 2, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 0, 1, 6, 16, 17, 8, 2, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 20, 10, 0, 2, 9, 20, 21, 11, 3, 0, 3, 10, 21, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 19, 0, 8, 18, 30, 31, 20, 9, 0, 9, 19, 31, 32, 21, 10, 0, 10, 20, 32, 22, 11, 0, 11, 21, 32, 33, 23, 12, 0, 12, 22, 33, 34, 35, 24, 0, 12, 23, 35, 25, 14, 13, 0, 14, 24, 35, 36, 37, 26, 0, 14, 25, 37, 38, 27, 15, 0, 15, 26, 38, 28, 16, 0, 16, 27, 38, 39, 29, 17, 0, 17, 28, 39, 40, 30, 18, 0, 18, 29, 40, 41, 31, 19, 0, 19, 30, 41, 42, 32, 20, 0, 20, 31, 42, 33, 22, 21, 0, 22, 32, 42, 34, 23, 0, 23, 33, 42, 43, 36, 35, 0, 23, 34, 36, 25, 24, 0, 25, 35, 34, 43, 44, 37, 0, 25, 36, 44, 38, 26, 0, 26, 37, 44, 39, 28, 27, 0, 28, 38, 44, 40, 29, 0, 29, 39, 44, 43, 41, 30, 0, 30, 40, 43, 42, 31, 0, 31, 41, 43, 34, 33, 32, 0, 34, 42, 41, 40, 44, 36, 0, 36, 43, 40, 39, 38, 37, 0},
    {44, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 0, 1, 6, 16, 17, 8, 2, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 11, 0, 3, 10, 21, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 31, 19, 0, 8, 18, 31, 20, 9, 0, 9, 19, 31, 32, 21, 10, 0, 10, 20, 32, 33, 22, 11, 0, 11, 21, 33, 23, 12, 0, 12, 22, 33, 34, 35, 24, 0, 12, 23, 35, 25, 14, 13, 0, 14, 24, 35, 36, 37, 26, 0, 14, 25, 37, 38, 27, 15, 0, 15, 26, 38, 28, 16, 0, 16, 27, 38, 39, 29, 17, 0, 17, 28, 39, 40, 30, 18, 0, 18, 29, 40, 41, 31, 0, 18, 30, 41, 32, 20, 19, 0, 20, 31, 41, 42, 33, 21, 0, 21, 32, 42, 34, 23, 22, 0, 23, 33, 42, 43, 36, 35, 0, 23, 34, 36, 25, 24, 0, 25, 35, 34, 43, 44, 37, 0, 25, 36, 44, 38, 26, 0, 26, 37, 44, 39, 28, 27, 0, 28, 38, 44, 40, 29, 0, 29, 39, 44, 43, 41, 30, 0, 30, 40, 43, 42, 32, 31, 0, 32, 41, 43, 34, 33, 0, 34, 42, 41, 40, 44, 36, 0, 36, 43, 40, 39, 38, 37, 0},
    {44, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 10, 3, 0, 1, 2, 10, 11, 12, 4, 0, 1, 3, 12, 13, 14, 5, 0, 1, 4, 14, 15, 16, 6, 0, 1, 5, 16, 17, 7, 0, 1, 6, 17, 18, 8, 2, 0, 2, 7, 18, 19, 9, 0, 2, 8, 19, 20, 21, 10, 0, 2, 9, 21, 11, 3, 0, 3, 10, 21, 22, 23, 12, 0, 3, 11, 23, 13, 4, 0, 4, 12, 23, 24, 25, 14, 0, 4, 13, 25, 15, 5, 0, 5, 14, 25, 26, 27, 16, 0, 5, 15, 27, 28, 17, 6, 0, 6, 16, 28, 29, 18, 7, 0, 7, 17, 29, 30, 19, 8, 0, 8, 18, 30, 31, 20, 9, 0, 9, 19, 31, 32, 21, 0, 9, 20, 32, 22, 11, 10, 0, 11, 21, 32, 33, 34, 23, 0, 11, 22, 34, 24, 13, 12, 0, 13, 23, 34, 35, 36, 25, 0, 13, 24, 36, 26, 15, 14, 0, 15, 25, 36, 37, 38, 27, 0, 15, 26, 38, 39, 28, 16, 0, 16, 27, 39, 29, 17, 0, 17, 28, 39, 40, 30, 18, 0, 18, 29, 40, 41, 31, 19, 0, 19, 30, 41, 42, 32, 20, 0, 20, 31, 42, 33, 22, 21, 0, 22, 32, 42, 43, 35, 34, 0, 22, 33, 35, 24, 23, 0, 24, 34, 33, 43, 37, 36, 0, 24, 35, 37, 26, 25, 0, 26, 36, 35, 43, 44, 38, 0, 26, 37, 44, 39, 27, 0, 27, 38, 44, 40, 29, 28, 0, 29, 39, 44, 41, 30, 0, 30, 40, 44, 43, 42, 31, 0, 31, 41, 43, 33, 32, 0, 33, 42, 41, 44, 37, 35, 0, 37, 43, 41, 40, 39, 38, 0},
    {44, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 10, 3, 0, 1, 2, 10, 11, 12, 4, 0, 1, 3, 12, 13, 14, 5, 0, 1, 4, 14, 15, 16, 6, 0, 1, 5, 16, 17, 7, 0, 1, 6, 17, 18, 8, 2, 0, 2, 7, 18, 19, 9, 0, 2, 8, 19, 20, 21, 10, 0, 2, 9, 21, 11, 3, 0, 3, 10, 21, 22, 23, 12, 0, 3, 11, 23, 13, 4, 0, 4, 12, 23, 24, 25, 14, 0, 4, 13, 25, 15, 5, 0, 5, 14, 25, 26, 27, 16, 0, 5, 15, 27, 28, 17, 6, 0, 6, 16, 28, 29, 18, 7, 0, 7, 17, 29, 30, 19, 8, 0, 8, 18, 30, 31, 20, 9, 0, 9, 19, 31, 32, 33, 21, 0, 9, 20, 33, 22, 11, 10, 0, 11, 21, 33, 34, 23, 0, 11, 22, 34, 24, 13, 12, 0, 13, 23, 34, 35, 36, 25, 0, 13, 24, 36, 26, 15, 14, 0, 15, 25, 36, 37, 27, 0, 15, 26, 37, 38, 28, 16, 0, 16, 27, 38, 29, 17, 0, 17, 28, 38, 39, 30, 18, 0, 18, 29, 39, 40, 31, 19, 0, 19, 30, 40, 32, 20, 0, 20, 31, 40, 41, 42, 33, 0, 20, 32, 42, 34, 22, 21, 0, 22, 33, 42, 35, 24, 23, 0, 24, 34, 42, 41, 43, 36, 0, 24, 35, 43, 37, 26, 25, 0, 26, 36, 43, 44, 38, 27, 0, 27, 37, 44, 39, 29, 28, 0, 29, 38, 44, 40, 30, 0, 30, 39, 44, 41, 32, 31, 0, 32, 40, 44, 43, 35, 42, 0, 32, 41, 35, 34, 33, 0, 35, 41, 44, 37, 36, 0, 37, 43, 41, 40, 39, 38, 0},
    {44, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 10, 3, 0, 1, 2, 10, 11, 12, 4, 0, 1, 3, 12, 13, 14, 5, 0, 1, 4, 14, 15, 16, 6, 0, 1, 5, 16, 17, 7, 0, 1, 6, 17, 18, 8, 2, 0, 2, 7, 18, 19, 9, 0, 2, 8, 19, 20, 21, 10, 0, 2, 9, 21, 11, 3, 0, 3, 10, 21, 22, 23, 12, 0, 3, 11, 23, 13, 4, 0, 4, 12, 23, 24, 25, 14, 0, 4, 13, 25, 15, 5, 0, 5, 14, 25, 26, 27, 16, 0, 5, 15, 27, 28, 17, 6, 0, 6, 16, 28, 29, 18, 7, 0, 7, 17, 29, 30, 19, 8, 0, 8, 18, 30, 31, 20, 9, 0, 9, 19, 31, 32, 21, 0, 9, 20, 32, 22, 11, 10, 0, 11, 21, 32, 33, 34, 23, 0, 11, 22, 34, 24, 13, 12, 0, 13, 23, 34, 35, 36, 25, 0, 13, 24, 36, 26, 15, 14, 0, 15, 25, 36, 37, 27, 0, 15, 26, 37, 38, 28, 16, 0, 16, 27, 38, 29, 17, 0, 17, 28, 38, 39, 30, 18, 0, 18, 29, 39, 40, 31, 19, 0, 19, 30, 40, 41, 32, 20, 0, 20, 31, 41, 33, 22, 21, 0, 22, 32, 41, 42, 35, 34, 0, 22, 33, 35, 24, 23, 0, 24, 34, 33, 42, 43, 36, 0, 24, 35, 43, 37, 26, 25, 0, 26, 36, 43, 44, 38, 27, 0, 27, 37, 44, 39, 29, 28, 0, 29, 38, 44, 40, 30, 0, 30, 39, 44, 42, 41, 31, 0, 31, 40, 42, 33, 32, 0, 33, 41, 40, 44, 43, 35, 0, 35, 42, 44, 37, 36, 0, 37, 43, 42, 40, 39, 38, 0},
    {44, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 10, 3, 0, 1, 2, 10, 11, 12, 4, 0, 1, 3, 12, 13, 14, 5, 0, 1, 4, 14, 15, 16, 6, 0, 1, 5, 16, 17, 7, 0, 1, 6, 17, 18, 8, 2, 0, 2, 7, 18, 19, 9, 0, 2, 8, 19, 20, 21, 10, 0, 2, 9, 21, 11, 3, 0, 3, 10, 21, 22, 23, 12, 0, 3, 11, 23, 13, 4, 0, 4, 12, 23, 24, 25, 14, 0, 4, 13, 25, 15, 5, 0, 5, 14, 25, 26, 27, 16, 0, 5, 15, 27, 28, 17, 6, 0, 6, 16, 28, 29, 18, 7, 0, 7, 17, 29, 30, 19, 8, 0, 8, 18, 30, 31, 20, 9, 0, 9, 19, 31, 32, 33, 21, 0, 9, 20, 33, 22, 11, 10, 0, 11, 21, 33, 34, 35, 23, 0, 11, 22, 35, 24, 13, 12, 0, 13, 23, 35, 36, 37, 25, 0, 13, 24, 37, 26, 15, 14, 0, 15, 25, 37, 38, 27, 0, 15, 26, 38, 39, 28, 16, 0, 16, 27, 39, 29, 17, 0, 17, 28, 39, 40, 30, 18, 0, 18, 29, 40, 41, 31, 19, 0, 19, 30, 41, 32, 20, 0, 20, 31, 41, 42, 34, 33, 0, 20, 32, 34, 22, 21, 0, 22, 33, 32, 42, 36, 35, 0, 22, 34, 36, 24, 23, 0, 24, 35, 34, 42, 43, 37, 0, 24, 36, 43, 38, 26, 25, 0, 26, 37, 43, 44, 39, 27, 0, 27, 38, 44, 40, 29, 28, 0, 29, 39, 44, 41, 30, 0, 30, 40, 44, 42, 32, 31, 0, 32, 41, 44, 43, 36, 34, 0, 36, 42, 44, 38, 37, 0, 38, 43, 42, 41, 40, 39, 0},
    {44, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 0, 1, 6, 16, 17, 8, 2, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 31, 19, 0, 8, 18, 31, 20, 9, 0, 9, 19, 31, 32, 21, 10, 0, 10, 20, 32, 33, 22, 0, 10, 21, 33, 23, 12, 11, 0, 12, 22, 33, 34, 35, 24, 0, 12, 23, 35, 25, 14, 13, 0, 14, 24, 35, 36, 26, 0, 14, 25, 36, 37, 27, 15, 0, 15, 26, 37, 28, 16, 0, 16, 27, 37, 38, 29, 17, 0, 17, 28, 38, 39, 30, 18, 0, 18, 29, 39, 40, 31, 0, 18, 30, 40, 32, 20, 19, 0, 20, 31, 40, 41, 33, 21, 0, 21, 32, 41, 34, 23, 22, 0, 23, 33, 41, 42, 43, 35, 0, 23, 34, 43, 36, 25, 24, 0, 25, 35, 43, 44, 37, 26, 0, 26, 36, 44, 38, 28, 27, 0, 28, 37, 44, 39, 29, 0, 29, 38, 44, 42, 40, 30, 0, 30, 39, 42, 41, 32, 31, 0, 32, 40, 42, 34, 33, 0, 34, 41, 40, 39, 44, 43, 0, 34, 42, 44, 36, 35, 0, 36, 43, 42, 39, 38, 37, 0},
    {43, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 10, 3, 0, 3, 9, 19, 20, 21, 11, 0, 3, 10, 21, 12, 4, 0, 4, 11, 21, 22, 23, 13, 0, 4, 12, 23, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 31, 19, 0, 8, 18, 31, 20, 10, 9, 0, 10, 19, 31, 32, 21, 0, 10, 20, 32, 22, 12, 11, 0, 12, 21, 32, 33, 34, 23, 0, 12, 22, 34, 35, 24, 13, 0, 13, 23, 35, 25, 14, 0, 14, 24, 35, 36, 37, 26, 0, 14, 25, 37, 27, 15, 0, 15, 26, 37, 38, 28, 16, 0, 16, 27, 38, 39, 29, 17, 0, 17, 28, 39, 30, 18, 0, 18, 29, 39, 40, 41, 31, 0, 18, 30, 41, 32, 20, 19, 0, 20, 31, 41, 33, 22, 21, 0, 22, 32, 41, 40, 42, 34, 0, 22, 33, 42, 35, 23, 0, 23, 34, 42, 36, 25, 24, 0, 25, 35, 42, 43, 37, 0, 25, 36, 43, 38, 27, 26, 0, 27, 37, 43, 39, 28, 0, 28, 38, 43, 40, 30, 29, 0, 30, 39, 43, 42, 33, 41, 0, 30, 40, 33, 32, 31, 0, 33, 40, 43, 36, 35, 34, 0, 36, 42, 40, 39, 38, 37, 0},
    {43, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 10, 3, 0, 3, 9, 19, 20, 21, 11, 0, 3, 10, 21, 12, 4, 0, 4, 11, 21, 22, 23, 13, 0, 4, 12, 23, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 31, 19, 0, 8, 18, 31, 20, 10, 9, 0, 10, 19, 31, 32, 33, 21, 0, 10, 20, 33, 22, 12, 11, 0, 12, 21, 33, 34, 35, 23, 0, 12, 22, 35, 36, 24, 13, 0, 13, 23, 36, 25, 14, 0, 14, 24, 36, 37, 38, 26, 0, 14, 25, 38, 27, 15, 0, 15, 26, 38, 39, 28, 16, 0, 16, 27, 39, 40, 29, 17, 0, 17, 28, 40, 30, 18, 0, 18, 29, 40, 41, 32, 31, 0, 18, 30, 32, 20, 19, 0, 20, 31, 30, 41, 34, 33, 0, 20, 32, 34, 22, 21, 0, 22, 33, 32, 41, 42, 35, 0, 22, 34, 42, 36, 23, 0, 23, 35, 42, 37, 25, 24, 0, 25, 36, 42, 43, 38, 0, 25, 37, 43, 39, 27, 26, 0, 27, 38, 43, 40, 28, 0, 28, 39, 43, 41, 30, 29, 0, 30, 40, 43, 42, 34, 32, 0, 34, 41, 43, 37, 36, 35, 0, 37, 42, 41, 40, 39, 38, 0},
    {43, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 10, 3, 0, 1, 2, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 0, 1, 6, 16, 17, 8, 2, 0, 2, 7, 17, 18, 9, 0, 2, 8, 18, 19, 20, 10, 0, 2, 9, 20, 21, 11, 3, 0, 3, 10, 21, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 19, 9, 0, 9, 18, 30, 31, 32, 20, 0, 9, 19, 32, 21, 10, 0, 10, 20, 32, 33, 22, 11, 0, 11, 21, 33, 23, 12, 0, 12, 22, 33, 34, 35, 24, 0, 12, 23, 35, 25, 14, 13, 0, 14, 24, 35, 36, 37, 26, 0, 14, 25, 37, 38, 27, 15, 0, 15, 26, 38, 28, 16, 0, 16, 27, 38, 39, 29, 17, 0, 17, 28, 39, 40, 30, 18, 0, 18, 29, 40, 31, 19, 0, 19, 30, 40, 41, 42, 32, 0, 19, 31, 42, 33, 21, 20, 0, 21, 32, 42, 34, 23, 22, 0, 23, 33, 42, 41, 36, 35, 0, 23, 34, 36, 25, 24, 0, 25, 35, 34, 41, 43, 37, 0, 25, 36, 43, 38, 26, 0, 26, 37, 43, 39, 28, 27, 0, 28, 38, 43, 40, 29, 0, 29, 39, 43, 41, 31, 30, 0, 31, 40, 43, 36, 34, 42, 0, 31, 41, 34, 33, 32, 0, 36, 41, 40, 39, 38, 37, 0},
    {43, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 0, 1, 6, 16, 17, 8, 2, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 19, 0, 8, 18, 30, 31, 20, 9, 0, 9, 19, 31, 21, 10, 0, 10, 20, 31, 32, 33, 22, 0, 10, 21, 33, 23, 12, 11, 0, 12, 22, 33, 34, 35, 24, 0, 12, 23, 35, 25, 14, 13, 0, 14, 24, 35, 36, 37, 26, 0, 14, 25, 37, 38, 27, 15, 0, 15, 26, 38, 28, 16, 0, 16, 27, 38, 39, 29, 17, 0, 17, 28, 39, 40, 30, 18, 0, 18, 29, 40, 41, 31, 19, 0, 19, 30, 41, 32, 21, 20, 0, 21, 31, 41, 42, 34, 33, 0, 21, 32, 34, 23, 22, 0, 23, 33, 32, 42, 36, 35, 0, 23, 34, 36, 25, 24, 0, 25, 35, 34, 42, 43, 37, 0, 25, 36, 43, 38, 26, 0, 26, 37, 43, 39, 28, 27, 0, 28, 38, 43, 40, 29, 0, 29, 39, 43, 42, 41, 30, 0, 30, 40, 42, 32, 31, 0, 32, 41, 40, 43, 36, 34, 0, 36, 42, 40, 39, 38, 37, 0},
    {43, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 10, 3, 0, 1, 2, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 0, 1, 6, 16, 17, 8, 2, 0, 2, 7, 17, 18, 9, 0, 2, 8, 18, 19, 20, 10, 0, 2, 9, 20, 21, 11, 3, 0, 3, 10, 21, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 19, 9, 0, 9, 18, 30, 31, 32, 20, 0, 9, 19, 32, 21, 10, 0, 10, 20, 32, 33, 22, 11, 0, 11, 21, 33, 23, 12, 0, 12, 22, 33, 34, 35, 24, 0, 12, 23, 35, 25, 14, 13, 0, 14, 24, 35, 36, 26, 0, 14, 25, 36, 37, 27, 15, 0, 15, 26, 37, 28, 16, 0, 16, 27, 37, 38, 29, 17, 0, 17, 28, 38, 39, 30, 18, 0, 18, 29, 39, 31, 19, 0, 19, 30, 39, 40, 41, 32, 0, 19, 31, 41, 33, 21, 20, 0, 21, 32, 41, 34, 23, 22, 0, 23, 33, 41, 40, 42, 35, 0, 23, 34, 42, 36, 25, 24, 0, 25, 35, 42, 43, 37, 26, 0, 26, 36, 43, 38, 28, 27, 0, 28, 37, 43, 39, 29, 0, 29, 38, 43, 40, 31, 30, 0, 31, 39, 43, 42, 34, 41, 0, 31, 40, 34, 33, 32, 0, 34, 40, 43, 36, 35, 0, 36, 42, 40, 39, 38, 37, 0},
    {42, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 10, 3, 0, 3, 9, 19, 20, 21, 11, 0, 3, 10, 21, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 25, 14, 5, 0, 5, 13, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 19, 0, 8, 18, 30, 20, 10, 9, 0, 10, 19, 30, 31, 32, 21, 0, 10, 20, 32, 22, 11, 0, 11, 21, 32, 33, 23, 12, 0, 12, 22, 33, 34, 35, 24, 0, 12, 23, 35, 25, 13, 0, 13, 24, 35, 36, 26, 14, 0, 14, 25, 36, 37, 27, 15, 0, 15, 26, 37, 28, 16, 0, 16, 27, 37, 38, 29, 17, 0, 17, 28, 38, 39, 30, 18, 0, 18, 29, 39, 31, 20, 19, 0, 20, 30, 39, 40, 32, 0, 20, 31, 40, 33, 22, 21, 0, 22, 32, 40, 34, 23, 0, 23, 33, 40, 41, 42, 35, 0, 23, 34, 42, 36, 25, 24, 0, 25, 35, 42, 37, 26, 0, 26, 36, 42, 38, 28, 27, 0, 28, 37, 42, 41, 39, 29, 0, 29, 38, 41, 40, 31, 30, 0, 31, 39, 41, 34, 33, 32, 0, 34, 40, 39, 38, 42, 0, 34, 41, 38, 37, 36, 35, 0},
    {42, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 18, 8, 0, 2, 7, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 11, 0, 3, 10, 21, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 25, 14, 5, 0, 5, 13, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 0, 7, 17, 29, 30, 19, 8, 0, 8, 18, 30, 31, 20, 9, 0, 9, 19, 31, 32, 21, 10, 0, 10, 20, 32, 33, 22, 11, 0, 11, 21, 33, 23, 12, 0, 12, 22, 33, 34, 35, 24, 0, 12, 23, 35, 25, 13, 0, 13, 24, 35, 36, 26, 14, 0, 14, 25, 36, 37, 27, 15, 0, 15, 26, 37, 28, 16, 0, 16, 27, 37, 38, 29, 17, 0, 17, 28, 38, 39, 30, 18, 0, 18, 29, 39, 31, 19, 0, 19, 30, 39, 40, 32, 20, 0, 20, 31, 40, 33, 21, 0, 21, 32, 40, 34, 23, 22, 0, 23, 33, 40, 41, 42, 35, 0, 23, 34, 42, 36, 25, 24, 0, 25, 35, 42, 37, 26, 0, 26, 36, 42, 38, 28, 27, 0, 28, 37, 42, 41, 39, 29, 0, 29, 38, 41, 40, 31, 30, 0, 31, 39, 41, 34, 33, 32, 0, 34, 40, 39, 38, 42, 0, 34, 41, 38, 37, 36, 35, 0},
    {41, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 11, 0, 3, 10, 21, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 25, 14, 5, 0, 5, 13, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 31, 19, 0, 8, 18, 31, 20, 9, 0, 9, 19, 31, 32, 21, 10, 0, 10, 20, 32, 33, 22, 11, 0, 11, 21, 33, 23, 12, 0, 12, 22, 33, 34, 35, 24, 0, 12, 23, 35, 25, 13, 0, 13, 24, 35, 36, 26, 14, 0, 14, 25, 36, 37, 27, 15, 0, 15, 26, 37, 28, 16, 0, 16, 27, 37, 38, 29, 17, 0, 17, 28, 38, 30, 18, 0, 18, 29, 38, 39, 40, 31, 0, 18, 30, 40, 32, 20, 19, 0, 20, 31, 40, 33, 21, 0, 21, 32, 40, 34, 23, 22, 0, 23, 33, 40, 39, 41, 35, 0, 23, 34, 41, 36, 25, 24, 0, 25, 35, 41, 37, 26, 0, 26, 36, 41, 38, 28, 27, 0, 28, 37, 41, 39, 30, 29, 0, 30, 38, 41, 34, 40, 0, 30, 39, 34, 33, 32, 31, 0, 34, 39, 38, 37, 36, 35, 0},
    {40, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 18, 8, 0, 2, 7, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 11, 0, 3, 10, 21, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 16, 6, 0, 6, 15, 26, 27, 17, 7, 0, 7, 16, 27, 28, 18, 0, 7, 17, 28, 29, 19, 8, 0, 8, 18, 29, 30, 20, 9, 0, 9, 19, 30, 31, 21, 10, 0, 10, 20, 31, 32, 22, 11, 0, 11, 21, 32, 23, 12, 0, 12, 22, 32, 33, 34, 24, 0, 12, 23, 34, 25, 14, 13, 0, 14, 24, 34, 35, 36, 26, 0, 14, 25, 36, 27, 16, 15, 0, 16, 26, 36, 37, 28, 17, 0, 17, 27, 37, 38, 29, 18, 0, 18, 28, 38, 30, 19, 0, 19, 29, 38, 39, 31, 20, 0, 20, 30, 39, 32, 21, 0, 21, 31, 39, 33, 23, 22, 0, 23, 32, 39, 40, 35, 34, 0, 23, 33, 35, 25, 24, 0, 25, 34, 33, 40, 37, 36, 0, 25, 35, 37, 27, 26, 0, 27, 36, 35, 40, 38, 28, 0, 28, 37, 40, 39, 30, 29, 0, 30, 38, 40, 33, 32, 31, 0, 33, 39, 38, 37, 35, 0},
    {39, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 10, 3, 0, 3, 9, 19, 20, 21, 11, 0, 3, 10, 21, 12, 4, 0, 4, 11, 21, 22, 23, 13, 0, 4, 12, 23, 24, 14, 5, 0, 5, 13, 24, 25, 15, 0, 5, 14, 25, 26, 16, 6, 0, 6, 15, 26, 27, 17, 7, 0, 7, 16, 27, 28, 18, 8, 0, 8, 17, 28, 29, 30, 19, 0, 8, 18, 30, 20, 10, 9, 0, 10, 19, 30, 31, 32, 21, 0, 10, 20, 32, 22, 12, 11, 0, 12, 21, 32, 33, 34, 23, 0, 12, 22, 34, 24, 13, 0, 13, 23, 34, 35, 25, 14, 0, 14, 24, 35, 36, 26, 15, 0, 15, 25, 36, 27, 16, 0, 16, 26, 36, 37, 28, 17, 0, 17, 27, 37, 29, 18, 0, 18, 28, 37, 38, 31, 30, 0, 18, 29, 31, 20, 19, 0, 20, 30, 29, 38, 33, 32, 0, 20, 31, 33, 22, 21, 0, 22, 32, 31, 38, 39, 34, 0, 22, 33, 39, 35, 24, 23, 0, 24, 34, 39, 36, 25, 0, 25, 35, 39, 37, 27, 26, 0, 27, 36, 39, 38, 29, 28, 0, 29, 37, 39, 33, 31, 0, 33, 38, 37, 36, 35, 34, 0},
    {37, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 10, 3, 0, 3, 9, 19, 20, 21, 11, 0, 3, 10, 21, 12, 4, 0, 4, 11, 21, 22, 23, 13, 0, 4, 12, 23, 14, 5, 0, 5, 13, 23, 24, 25, 15, 0, 5, 14, 25, 16, 6, 0, 6, 15, 25, 26, 17, 7, 0, 7, 16, 26, 27, 18, 8, 0, 8, 17, 27, 28, 29, 19, 0, 8, 18, 29, 20, 10, 9, 0, 10, 19, 29, 30, 31, 21, 0, 10, 20, 31, 22, 12, 11, 0, 12, 21, 31, 32, 33, 23, 0, 12, 22, 33, 24, 14, 13, 0, 14, 23, 33, 34, 35, 25, 0, 14, 24, 35, 26, 16, 15, 0, 16, 25, 35, 36, 27, 17, 0, 17, 26, 36, 28, 18, 0, 18, 27, 36, 37, 30, 29, 0, 18, 28, 30, 20, 19, 0, 20, 29, 28, 37, 32, 31, 0, 20, 30, 32, 22, 21, 0, 22, 31, 30, 37, 34, 33, 0, 22, 32, 34, 24, 23, 0, 24, 33, 32, 37, 36, 35, 0, 24, 34, 36, 26, 25, 0, 26, 35, 34, 37, 28, 27, 0, 28, 36, 34, 32, 30, 0},
    {42, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 17, 7, 0, 1, 6, 17, 18, 8, 2, 0, 2, 7, 18, 19, 20, 9, 0, 2, 8, 20, 21, 10, 3, 0, 3, 9, 21, 22, 23, 11, 0, 3, 10, 23, 12, 4, 0, 4, 11, 23, 24, 25, 13, 0, 4, 12, 25, 14, 5, 0, 5, 13, 25, 26, 27, 15, 0, 5, 14, 27, 16, 6, 0, 6, 15, 27, 28, 29, 17, 0, 6, 16, 29, 18, 7, 0, 7, 17, 29, 30, 19, 8, 0, 8, 18, 30, 31, 20, 0, 8, 19, 31, 32, 21, 9, 0, 9, 20, 32, 22, 10, 0, 10, 21, 32, 33, 34, 23, 0, 10, 22, 34, 24, 12, 11, 0, 12, 23, 34, 35, 25, 0, 12, 24, 35, 26, 14, 13, 0, 14, 25, 35, 36, 37, 27, 0, 14, 26, 37, 28, 16, 15, 0, 16, 27, 37, 38, 29, 0, 16, 28, 38, 30, 18, 17, 0, 18, 29, 38, 39, 31, 19, 0, 19, 30, 39, 40, 32, 20, 0, 20, 31, 40, 33, 22, 21, 0, 22, 32, 40, 41, 34, 0, 22, 33, 41, 35, 24, 23, 0, 24, 34, 41, 36, 26, 25, 0, 26, 35, 41, 42, 37, 0, 26, 36, 42, 38, 28, 27, 0, 28, 37, 42, 39, 30, 29, 0, 30, 38, 42, 40, 31, 0, 31, 39, 42, 41, 33, 32, 0, 33, 40, 42, 36, 35, 34, 0, 36, 41, 40, 39, 38, 37, 0},
    {41, 2, 3, 4, 5, 6, 7, 0, 1, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 0, 1, 6, 16, 17, 8, 2, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 20, 10, 3, 0, 3, 9, 20, 21, 22, 11, 0, 3, 10, 22, 12, 4, 0, 4, 11, 22, 23, 24, 13, 0, 4, 12, 24, 14, 5, 0, 5, 13, 24, 25, 26, 15, 0, 5, 14, 26, 27, 16, 6, 0, 6, 15, 27, 28, 17, 7, 0, 7, 16, 28, 29, 18, 8, 0, 8, 17, 29, 30, 19, 0, 8, 18, 30, 31, 20, 9, 0, 9, 19, 31, 21, 10, 0, 10, 20, 31, 32, 33, 22, 0, 10, 21, 33, 23, 12, 11, 0, 12, 22, 33, 34, 24, 0, 12, 23, 34, 25, 14, 13, 0, 14, 24, 34, 35, 36, 26, 0, 14, 25, 36, 27, 15, 0, 15, 26, 36, 37, 28, 16, 0, 16, 27, 37, 29, 17, 0, 17, 28, 37, 38, 30, 18, 0, 18, 29, 38, 39, 31, 19, 0, 19, 30, 39, 32, 21, 20, 0, 21, 31, 39, 40, 33, 0, 21, 32, 40, 34, 23, 22, 0, 23, 33, 40, 35, 25, 24, 0, 25, 34, 40, 41, 36, 0, 25, 35, 41, 37, 27, 26, 0, 27, 36, 41, 38, 29, 28, 0, 29, 37, 41, 39, 30, 0, 30, 38, 41, 40, 32, 31, 0, 32, 39, 41, 35, 34, 33, 0, 35, 40, 39, 38, 37, 36, 0},
    {46, 2, 3, 4, 5, 6, 0, 1, 6, 7, 8, 9, 3, 0, 1, 2, 9, 10, 11, 4, 0, 1, 3, 11, 12, 13, 5, 0, 1, 4, 13, 14, 15, 6, 0, 1, 5, 15, 16, 7, 2, 0, 2, 6, 16, 17, 8, 0, 2, 7, 17, 18, 19, 9, 0, 2, 8, 19, 10, 3, 0, 3, 9, 19, 20, 21, 11, 0, 3, 10, 21, 12, 4, 0, 4, 11, 21, 22, 23, 13, 0, 4, 12, 23, 24, 14, 5, 0, 5, 13, 24, 25, 15, 0, 5, 14, 25, 26, 16, 6, 0, 6, 15, 26, 27, 17, 7, 0, 7, 16, 27, 28, 18, 8, 0, 8, 17, 28, 29, 30, 19, 0, 8, 18, 30, 20, 10, 9, 0, 10, 19, 30, 31, 32, 21, 0, 10, 20, 32, 22, 12, 11, 0, 12, 21, 32, 33, 23, 0, 12, 22, 33, 34, 24, 13, 0, 13, 23, 34, 35, 25, 14, 0, 14, 24, 35, 36, 26, 15, 0, 15, 25, 36, 37, 27, 16, 0, 16, 26, 37, 28, 17, 0, 17, 27, 37, 38, 29, 18, 0, 18, 28, 38, 39, 40, 30, 0, 18, 29, 40, 31, 20, 19, 0, 20, 30, 40, 41, 42, 32, 0, 20, 31, 42, 33, 22, 21, 0, 22, 32, 42, 43, 34, 23, 0, 23, 33, 43, 44, 35, 24, 0, 24, 34, 44, 36, 25, 0, 25, 35, 44, 45, 37, 26, 0, 26, 36, 45, 38, 28, 27, 0, 28, 37, 45, 39, 29, 0, 29, 38, 45, 46, 41, 40, 0, 29, 39, 41, 31, 30, 0, 31, 40, 39, 46, 43, 42, 0, 31, 41, 43, 33, 32, 0, 33, 42, 41, 46, 44, 34, 0, 34, 43, 46, 45, 36, 35, 0, 36, 44, 46, 39, 38, 37, 0, 39, 45, 44, 43, 41, 0}
};


/* The 12 degree 5 vertices of a fullerene */
static int degree_5_vertices[12];

/* Mapping from vertexnumber to index in degree_5_vertices[].
   Is obviously only valid for vertices of degree 5 of course */
static int degree_5_vertices_index[MAXN];

#define NEXT_BITVECTOR 1
#define PREV_BITVECTOR 2

/* Max number of straight for a given length or bent expansions is 12 * 5 = 60 */
#define MAX_EXTENSIONS_SINGLE 60

/* Contains the max min distance between any 2 pentagons of a fullerene with i faces */
static int max_straight_lengths[MAXN + 1];

/* A list of shortest straight paths of the current graph (used for lookahead) */
//Important: is assumed to be > 0
#define MAX_PREV_EXTENSIONS 10

//Some straight reductions with the shortest pathlength in the current graph (before expansion)
static int straight_extensions[MAX_PREV_EXTENSIONS][MAX_STRAIGHT_LENGTH + 2];

//Size is previous_canon_straight_length + 2
static int straight_length = MAX_STRAIGHT_LENGTH + 1;
static int num_straight_extensions = 0;

#define BENT_ZERO_SIZE 11

//Remark: this is at the moment only used in case of IPR
static int bent_zero_extensions[MAX_PREV_EXTENSIONS][BENT_ZERO_SIZE];
static int num_bent_zero_extensions = 0;

#define L2_SIZE 6
static int L2_extensions[MAX_PREV_EXTENSIONS][L2_SIZE];
static int num_L2_extensions = 0;

#define B10_SIZE 6
static int bent_one_zero_extensions[MAX_PREV_EXTENSIONS][B10_SIZE];
static int num_B10_extensions = 0;


/* List of extension which have best straight colour 1 and 2 */
//Remark: is only valid for L0 at the moment!

#define STRAIGHT_COLOUR_2_L0_LENGTH_EXPANSION 4
#define STRAIGHT_COLOUR_2_L0_LENGTH 14

static int straight_colour_2_L0_extensions[MAX_PREV_EXTENSIONS][STRAIGHT_COLOUR_2_L0_LENGTH];

static int num_straight_colour_2_L0_extensions = 0;

static int best_straight_colour_1 = 0;
static int best_straight_colour_2 = 0;

static EDGE *previous_rejector[MAXN+1];


/* 
 * Global arrays with the possible bent exentsions.
 * These arrays aredynamically resized.
 * 
 * Is only used in case of IPR at the moment.
 */

#define DEFAULT_MAX_EXT_BENT_SIZE 100
EDGE **ext_bent_global;
int *ext_bent_position_global;
int *ext_bent_length_global;
int *ext_bent_use_next_global;
static int num_ext_bent_global = 0;
static int max_num_ext_bent_global = DEFAULT_MAX_EXT_BENT_SIZE;


/*******************************Methods************************************/

static void (*write_graph)(FILE*,int);
static void (*write_dual_graph)(FILE*,int);

static void search_bent_zero_reductions_ipr();
static void search_L2_reductions_ipr();


/**************************************************************************/

/* Include optional file for special processing. */

#ifdef PLUGIN
#include PLUGIN
#endif

#ifdef PRE_FILTER
Error - Trying to use an obsolete plugin
#endif

/**************************************************************************/

/*
 * Returns 1 if vertex is in degree_5_vertices[], else returns 0.
 */
static int
is_in_degree_5_vertices_list(int vertex) {
    int i;
    for(i = 0; i < 12; i++)
        if(degree_5_vertices[i] == vertex)
            return 1;
    return 0;
}

/****************************************************************************/

/*
 * Prints the current embedded graph to stdout.
 * 
 * Warning: contains some checks so it only works for fullerenes.
 */
static void
print_embedded_graph() {
    fprintf(stderr, "nv: %d, ne: %d\n", nv, ne);
    int number_deg_5 = 0;
    int i;
    EDGE *e,*ee;
    for (i = 0; i < nv; i++) {
        fprintf(stderr, "%d: ", i);
        e = ee = firstedge[i];
        int deg = 0;
        do {
            DEBUGASSERT(e->invers->invers == e);

            fprintf(stderr, "%d ", e->end);
            e = e->next;

            deg++;
        } while (e != ee);
        fprintf(stderr, "\n");

        if(deg != degree[i]) {
            fprintf(stderr, "Error: degree is invalid: %d vs %d\n", deg, degree[i]);
            exit(1);
        }
        if(deg == 5) {
            number_deg_5++;
            if(!is_in_degree_5_vertices_list(i)) {
                fprintf(stderr, "Error: is not in degree 5 vertices list\n");
                exit(1);
            }
        }
    }
    if(number_deg_5 != 12) {
        fprintf(stderr, "Error: invalid number of degree 5 vertices: %d\n", number_deg_5);
        exit(1);
    }
}

/****************************************************************************/

/*
 * Prints canon_form[].
 */
static void
print_code(unsigned char canon_form[], int length) {
    int i;
    for(i = 0; i < length; i++)
        fprintf(stderr, "%d ", canon_form[i]);
    fprintf(stderr, "\n");
}


/*********************Methods for splay tree*********************************/

void new_splaynode(SPLAYNODE *el, unsigned char *canong, int codelength, int type, int *is_new_node)
 {

    el->graph = canong;
    el->length = codelength;
    el->type = type;
    *is_new_node = 1;
    
    //fprintf(stderr, "New graph found. Nv: %d\n", nv);
    //fprintf(stderr, "Type is %d\n", type);
}

/****************************************/

void old_splaynode(SPLAYNODE *el, int *is_new_node)
 {

    *is_new_node = 0;
    //fprintf(stderr, "Graph was already present. Nv: %d\n", nv);

}

/****************************************/

int comparenodes(unsigned char *canong, int codelength, int type, SPLAYNODE *list)
 {
    int compare;

    compare = codelength - list->length;
    if(compare == 0)
        compare = memcmp(canong, list->graph, codelength * sizeof(unsigned  char));
    
    //Temp, for debugging only!
/*
    if(compare == 0) {
        if(list->type != type) {
            fprintf(stderr, "Error: graph was already present, but had different type (should never happen!): type %d vs %d\n", list->type, type);
            exit(1);
        }
        if(type == IRRED_CAP_2_8 || type == IRRED_CAP_8_2 || type == IRRED_CAP_5_5) {
            fprintf(stderr, "Info: isomorphic graph obtained with bent cap (this might happen)\n");
            exit(1);
        }
    }
*/
    return compare;
}

/****************************************/

outputnode(SPLAYNODE *liste)
 {
    fprintf(stderr, "Error: outputting of nodes not allowed\n");
    exit(1);
}

/**************************************************************************/

static void
show_group(FILE *f, int nbtot, int nbop)

/* Display the group stored in the usual place */

{
    EDGE **nb,**nblim;
    int i;

    fprintf(f,"nv=%d ne=%d nbtot=%d nbop=%d\n",nv,ne,nbtot,nbop);

    if (nbtot == 1) return;

    nblim = (EDGE**)numbering[nbtot];
    for (nb = (EDGE**)numbering[0]; nb < nblim; nb += MAXE)
    {
	for (i = 0; i < ne; ++i)
	    fprintf(f," %x-%x",nb[i]->start,nb[i]->end);
	fprintf(f,"\n");
    }
}

/******************Methods for performing an expansion*********************/

/*
 * Replaces the neighbour org_neighbour of vertex by new_neighbour.
 */
static void
replace_neighbour(int vertex, int org_neighbour, int new_neighbour) {
    EDGE *e = edge_list[vertex][org_neighbour];
    e->end = new_neighbour;
    edge_list[vertex][new_neighbour] = e;
}

/**************************************************************************/

/*
 * Replaces the neighbour org_neighbour of vertex by new_neighbour and sets
 * invers_edge as invers edge.
 */
static void
replace_neighbour_invers(int vertex, int org_neighbour, int new_neighbour, EDGE *invers_edge) {
    EDGE *e = edge_list[vertex][org_neighbour];
    e->end = new_neighbour;
    e->invers = invers_edge;
    invers_edge->invers = e;

    //Has to set, otherwise restore_neighbour() won't work
    edge_list[vertex][new_neighbour] = e;
}

/**************************************************************************/

/*
 * Replaces the neighbour old_neighbour of vertex by org_neighbour.
 */
static void
restore_neighbour(int vertex, int old_neighbour, int org_neighbour) {
    /*
     * Remark:
     * No need to restore edge_list since old values will still be valid:
     * 2 vertices which are not adjacent wont become adjacent after applying an
     * extension.
     */
    edge_list[vertex][old_neighbour]->end = org_neighbour;
}

/**************************************************************************/

/*
 * Replaces the neighbour added_vertex1 of v1 by v2 and replaces the neighbour
 * added_vertex2 of v2 by v1 and also sets the invers vertices.
 */
static void
restore_neighbour_invers(int v1, int added_vertex1, int v2, int added_vertex2) {
    /*
     * Remark:
     * No need to restore edge_list since old values will still be valid:
     * 2 vertices which are not adjacent wont become adjacent after applying an
     * extension.
     */
    //edge_list[vertex][old_neighbour]->end = org_neighbour;
    EDGE *e1 = edge_list[v1][added_vertex1];
    EDGE *e2 = edge_list[v2][added_vertex2];
    
    e1->end = v2;
    e2->end = v1;
    e1->invers = e2;
    e2->invers = e1;
}

/**************************************************************************/

/*
 * Removes edge (vertex, neighbour) from vertex and updates the degree and ne.
 */
static void
remove_edge(int vertex, int neighbour) {
    EDGE *e = edge_list[vertex][neighbour];

    //New edges are never firstedges
    DEBUGASSERT(firstedge[vertex] != e);

    e->next->prev = e->prev;
    e->prev->next = e->next;

    ne--;
    degree[vertex]--;
    DEBUGASSERT(degree[vertex] == 5);
}

/**************************************************************************/

/*
 * Inserts a new edge clockwisely after the edge (vertex, after_neigbhour).
 */
static void
insert_edge_fuller(int vertex, int new_neighbour, int after_neighbour) {
    EDGE *e_neighbour = edge_list[vertex][after_neighbour];
    DEBUGASSERT(e_neighbour->start == vertex && e_neighbour->end == after_neighbour);

    EDGE *e;
    NEWEDGE(e);
    e->label = ne;
    ne++;
    degree[vertex]++;
    DEBUGASSERT(degree[vertex] == 6);
    e->start = vertex;
    e->end = new_neighbour;
    edge_list[vertex][new_neighbour] = e;
    e->invers = NULL;

    e_neighbour->next->prev = e;
    e->next = e_neighbour->next;
    e->prev = e_neighbour;
    e_neighbour->next = e;
}

/**************************************************************************/

/*
 * Inserts a new edge clockwisely after the edge (vertex, after_neigbhour)  
 * and sets invers_edge as invers edge.
 */
static void
insert_edge_fuller_invers(int vertex, int new_neighbour, int after_neighbour, EDGE *invers_edge) {
    EDGE *e_neighbour = edge_list[vertex][after_neighbour];
    DEBUGASSERT(e_neighbour->start == vertex && e_neighbour->end == after_neighbour);

    EDGE *e;
    NEWEDGE(e);
    e->label = ne++;
    degree[vertex]++;
    DEBUGASSERT(degree[vertex] == 6);
    e->start = vertex;
    e->end = new_neighbour;
    
    //Has to set, otherwise remove_edge() won't work
    edge_list[vertex][new_neighbour] = e;
    e->invers = invers_edge;
    invers_edge->invers = e;

    e_neighbour->next->prev = e;
    e->next = e_neighbour->next;
    e->prev = e_neighbour;
    e_neighbour->next = e;
}

/**************************************************************************/

/*
 * Sets invers for edges (v1, v2) and (v2, v1).
 */
static void
set_inverse_edges(int v1, int v2) {
    EDGE *e = edge_list[v1][v2];
    EDGE *ee = edge_list[v2][v1];
    e->invers = ee;
    ee->invers = e;

    //Not setting min as this field is not used at the moment
/*
    if(e < ee) e->min = ee->min = e;
    else e->min = ee->min = ee;
*/

}

/**************************************************************************/

/*
 * Replaces old_deg_5 by new_deg_5 in degree_5_vertices
 */
static void
replace_degree_5_vertex(int old_deg_5, int new_deg_5) {
    int index = degree_5_vertices_index[old_deg_5];
    degree_5_vertices[index] = new_deg_5;
    degree_5_vertices_index[new_deg_5] = index;
}

/**************************************************************************/

/*
 * Restores old_deg_5 in degree_5_vertices[].
 */
//Remark: isn't faster as macro
static void
restore_old_degree_5_vertex(int old_deg_5) {
    /*
     * Remark: degree_5_vertices_index[old_deg_5] is still valid since once a vertex
     * has deg 6, it can't get degree 5 anymore.
     */
    degree_5_vertices[degree_5_vertices_index[old_deg_5]] = old_deg_5;
}

/**************************************************************************/

//Last edge of extend_L0
static EDGE *last_edge_L0;

/****************************************/

/* Routines for extending and reducing a fullerene.
   General principle:  extend_x(e); reduce_x(e) will extend by an extension
   of type x (x = straight or bent) and then reducie it to the original graph.
   The final graph is exactly the same as the original
   (including pointer values) except that possibly the values of
   firstedge[] might be different.
*/

static void
extend_L0(EDGE *startedge, int path[], int parallel_path[], int use_next)
{

    int i, j;
    EDGE *e;

    //firstedges will be updated by replace_neighbour

    //First create the new vertices and edges
    for(i = 0; i < 2; i++) {
        //degree[nv + i] = 0;
        firstedge[nv + i] = NULL;
        for(j = 0; j < 5; j++) {
            NEWEDGE(e);
            e->label = ne;
            ne++;
            //degree[nv + i]++;
            e->start = nv + i;
            //e->end = path[i];
            e->invers = NULL;
            if(firstedge[nv + i] == NULL)
                firstedge[nv + i] = e;
            else {
                (e - 1)->next = e;
                e->prev = e - 1;
            }
        }
        firstedge[nv + i]->prev = e;
        e->next = firstedge[nv + i];

        degree[nv + i] = 5;
        //DEBUGASSERT(degree[nv + i] == 5);
    }

    //DEBUGASSERT(ne == (ne_old + 10));
    
    if(use_next) {
        //nv
        e = firstedge[nv];
        e->end = path[0];
        //edge_list[e->start][e->end] = e;
        insert_edge_fuller_invers(path[0], nv, parallel_path[0], e);

        e = e->prev;
        e->end = path[1];
        //edge_list[e->start][e->end] = e;
        replace_neighbour_invers(path[1], parallel_path[0], nv, e);

        e = e->prev;
        e->end = nv + 1;
        //edge_list[e->start][e->end] = e;
        last_edge_L0 = e;
        e = e->prev;

        e->end = parallel_path[1];
        //edge_list[e->start][e->end] = e;
        replace_neighbour_invers(parallel_path[1], path[1], nv, e);

        e = e->prev;
        e->end = parallel_path[0];
        //edge_list[e->start][e->end] = e;
        replace_neighbour_invers(parallel_path[0], path[1], nv, e);

        //nv + 1
        e = firstedge[nv + 1];
        e->end = nv;
        //edge_list[e->start][e->end] = e;
        e->invers = last_edge_L0;
        last_edge_L0->invers = e;
        e = e->prev;

        e->end = path[1];
        //edge_list[e->start][e->end] = e;
        replace_neighbour_invers(path[1], parallel_path[1], nv + 1, e);

        e = e->prev;
        e->end = path[2];
        //edge_list[e->start][e->end] = e;
        replace_neighbour_invers(path[2], parallel_path[1], nv + 1, e);

        e = e->prev;
        e->end = parallel_path[2];
        //edge_list[e->start][e->end] = e;
        insert_edge_fuller_invers(parallel_path[2], nv + 1, path[2], e);

        e = e->prev;
        e->end = parallel_path[1];
        //edge_list[e->start][e->end] = e;
        replace_neighbour_invers(parallel_path[1], path[2], nv + 1, e);

    } else {
        //Identical, but using next instead of prev
        
        //nv
        e = firstedge[nv];
        e->end = path[0];
        //edge_list[e->start][e->end] = e;
        insert_edge_fuller_invers(path[0], nv, path[1], e);

        e = e->next;
        e->end = path[1];
        //edge_list[e->start][e->end] = e;
        replace_neighbour_invers(path[1], parallel_path[0], nv, e);

        e = e->next;
        e->end = nv + 1;
        //edge_list[e->start][e->end] = e;
        last_edge_L0 = e;
        e = e->next;

        e->end = parallel_path[1];
        //edge_list[e->start][e->end] = e;
        replace_neighbour_invers(parallel_path[1], path[1], nv, e);

        e = e->next;
        e->end = parallel_path[0];
        //edge_list[e->start][e->end] = e;
        replace_neighbour_invers(parallel_path[0], path[1], nv, e);

        //nv + 1
        e = firstedge[nv + 1];
        e->end = nv;
        //edge_list[e->start][e->end] = e;
        e->invers = last_edge_L0;
        last_edge_L0->invers = e;        
        e = e->next;

        e->end = path[1];
        //edge_list[e->start][e->end] = e;
        replace_neighbour_invers(path[1], parallel_path[1], nv + 1, e);

        e = e->next;
        e->end = path[2];
        //edge_list[e->start][e->end] = e;
        replace_neighbour_invers(path[2], parallel_path[1], nv + 1, e);

        e = e->next;
        e->end = parallel_path[2];
        //edge_list[e->start][e->end] = e;
        insert_edge_fuller_invers(parallel_path[2], nv + 1, parallel_path[1], e);

        e = e->next;
        e->end = parallel_path[1];
        //edge_list[e->start][e->end] = e;
        replace_neighbour_invers(parallel_path[1], path[2], nv + 1, e);

    }

    replace_degree_5_vertex(path[0], nv);
    replace_degree_5_vertex(parallel_path[2], nv + 1);

    nv += 2;

}

/****************************************/

static void
reduce_L0(int path[], int parallel_path[])
{

    nv -= 2;
    ne -= 10;

    //No need to restore firstedges, they will be updated by replace_neighbour

    /*
     * Also no need to restore edge_list since old values will still be valid:
     * 2 vertices which are not adjacent wont become adjacent after applying an
     * extension.
     */

    remove_edge(path[0], nv);
    remove_edge(parallel_path[2], nv + 1);

/*
    restore_neighbour(path[1], nv, parallel_path[0]);
    restore_neighbour(path[1], nv + 1, parallel_path[1]);
    restore_neighbour(path[2], nv + 1, parallel_path[1]);

    restore_neighbour(parallel_path[0], nv, path[1]);
    restore_neighbour(parallel_path[1], nv, path[1]);
    restore_neighbour(parallel_path[1], nv + 1, path[2]);
*/
    
    restore_neighbour_invers(path[1], nv, parallel_path[0], nv);
    restore_neighbour_invers(path[1], nv + 1, parallel_path[1], nv);
    restore_neighbour_invers(path[2], nv + 1, parallel_path[1], nv + 1);        
    
/*
    //Update inverse edges and min
    set_inverse_edges(path[1], parallel_path[0]);
    set_inverse_edges(path[1], parallel_path[1]);
    set_inverse_edges(path[2], parallel_path[1]);
*/

    //replace_degree_5_vertex(nv, path[0]);
    //replace_degree_5_vertex(nv + 1, parallel_path[2]);
    restore_old_degree_5_vertex(path[0]);
    restore_old_degree_5_vertex(parallel_path[2]);

    FREEEDGES(10 + 2);

}

/**************************************************************************/

/*
 * Performs an Lx expansion, with x > 0.
 */
static void
extend_straight(EDGE *startedge, int path[], int parallel_path[], int pathlength, int use_next)
{
    DEBUGASSERT(pathlength > 1);

    int i, j;
    EDGE *e;

    //firstedges will be updated by replace_neighbour

    //First create the new vertices and edges
    for(i = 0; i < pathlength; i++) {
        degree[nv + i] = 0;
        firstedge[nv + i] = NULL;
        int deg;
        if(i == 0 || i == pathlength - 1)
            deg = 5;
        else
            deg = 6;
        for(j = 0; j < deg; j++) {
            NEWEDGE(e);
            e->label = ne;
            ne++;
            degree[nv + i]++;
            e->start = nv + i;
            //e->end = path[i];
            e->invers = NULL;
            if(firstedge[nv + i] == NULL)
                firstedge[nv + i] = e;
            else {
                (e - 1)->next = e;
                e->prev = e - 1;
            }
        }
        firstedge[nv + i]->prev = e;
        e->next = firstedge[nv + i];

        DEBUGASSERT(degree[nv + i] == deg);
    }

    //DEBUGASSERT(ne == (ne_old + 6 * (pathlength - 2) + 10));

    if(use_next) {
        for(i = 0; i < pathlength; i++) {
            e = firstedge[nv + i];

            if(i > 0) {
                e->end = nv + i - 1;
                edge_list[e->start][e->end] = e;
                e = e->prev;
            }

            e->end = path[i];
            edge_list[e->start][e->end] = e;

            e = e->prev;
            e->end = path[i + 1];
            edge_list[e->start][e->end] = e;

            e = e->prev;
            if(i < pathlength - 1) {
                e->end = nv + i + 1;
                edge_list[e->start][e->end] = e;
                e = e->prev;
            }
            e->end = parallel_path[i + 1];
            edge_list[e->start][e->end] = e;

            e = e->prev;
            e->end = parallel_path[i];
            edge_list[e->start][e->end] = e;
        }
        insert_edge_fuller(path[0], nv, parallel_path[0]);
        insert_edge_fuller(parallel_path[pathlength], nv + pathlength - 1, path[pathlength]);
    } else {
        //Is identical but next instead of prev
        for(i = 0; i < pathlength; i++) {
            e = firstedge[nv + i];

            if(i > 0) {
                e->end = nv + i - 1;
                edge_list[e->start][e->end] = e;
                e = e->next;
            }

            e->end = path[i];
            edge_list[e->start][e->end] = e;

            e = e->next;
            e->end = path[i + 1];
            edge_list[e->start][e->end] = e;

            e = e->next;
            if(i < pathlength - 1) {
                e->end = nv + i + 1;
                edge_list[e->start][e->end] = e;
                e = e->next;
            }
            e->end = parallel_path[i + 1];
            edge_list[e->start][e->end] = e;

            e = e->next;
            e->end = parallel_path[i];
            edge_list[e->start][e->end] = e;
        }
        insert_edge_fuller(path[0], nv, path[1]);
        insert_edge_fuller(parallel_path[pathlength], nv + pathlength - 1, parallel_path[pathlength - 1]);
    }

    for(i = 1; i < pathlength + 1; i++) {
        replace_neighbour(path[i], parallel_path[i - 1], nv + i - 1);
        if(i < pathlength)
            replace_neighbour(path[i], parallel_path[i], nv + i);
    }

    for(i = 0; i < pathlength; i++) {
        if(i > 0)
            replace_neighbour(parallel_path[i], path[i], nv + i - 1);

        replace_neighbour(parallel_path[i], path[i + 1], nv + i);
    }

    //Update inverse edges and min
    EDGE *ee;
    for(i = 0; i < pathlength; i++) {
        e = firstedge[nv + i];
        for(j = 0; j < degree[nv + i]; j++) {
            //Could use set_inverse_edges but this is slightly less inefficient
            ee = edge_list[e->end][e->start];
            e->invers = ee;
            ee->invers = e;

/*
            if(e < ee) e->min = ee->min = e;
            else e->min = ee->min = ee;
*/

            e = e->next;
        }
    }

    replace_degree_5_vertex(path[0], nv);
    replace_degree_5_vertex(parallel_path[pathlength], nv + pathlength - 1);

    nv += pathlength;

}

/****************************************/

/*
 * Performs an Lx reduction, with x > 0.
 */
static void
reduce_straight(int path[], int parallel_path[], int pathlength)
{

    nv -= pathlength;
    ne -= 6 * (pathlength - 2) + 10;

    //No need to restore firstedges, they will be updated by replace_neighbour

    /*
     * Also no need to restore edge_list since old values will still be valid:
     * 2 vertices which are not adjacent wont become adjacent after applying an
     * extension.
     */

    remove_edge(path[0], nv);
    remove_edge(parallel_path[pathlength], nv + pathlength - 1);

    int i;
    for(i = 1; i < pathlength + 1; i++) {
        restore_neighbour(path[i], nv + i - 1, parallel_path[i - 1]);
        if(i < pathlength)
            restore_neighbour(path[i], nv + i, parallel_path[i]);
    }

    for(i = 0; i < pathlength; i++) {
        if(i > 0)
            restore_neighbour(parallel_path[i], nv + i - 1, path[i]);

        restore_neighbour(parallel_path[i], nv + i, path[i + 1]);
    }

    //Update inverse edges and min
    for(i = 1; i < pathlength + 1; i++) {
        set_inverse_edges(path[i], parallel_path[i - 1]);

        if(i < pathlength)
            set_inverse_edges(path[i], parallel_path[i]);
    }

    //replace_degree_5_vertex(nv, path[0]);
    //replace_degree_5_vertex(nv + pathlength - 1, parallel_path[pathlength]);
    restore_old_degree_5_vertex(path[0]);
    restore_old_degree_5_vertex(parallel_path[pathlength]);

    FREEEDGES(6 * (pathlength - 2) + 10 + 2);

}

/************************************************************************/

/*
 * Performs a B00 expansion.
 */
static void
extend_bent_zero(EDGE *startedge, int path[], int parallel_path[], int use_next)
{

    int i, j;
    EDGE *e;

    //firstedges will be updated by replace_neighbour

    //First create the new vertices and edges
    for(i = 0; i < 3; i++) {
        degree[nv + i] = 0;
        firstedge[nv + i] = NULL;
        int deg;
        if(i == 1)
            deg = 6;
        else
            deg = 5;
        for(j = 0; j < deg; j++) {
            NEWEDGE(e);
            e->label = ne;
            ne++;
            degree[nv + i]++;
            e->start = nv + i;
            //e->end = path[i];
            e->invers = NULL;
            if(firstedge[nv + i] == NULL)
                firstedge[nv + i] = e;
            else {
                (e - 1)->next = e;
                e->prev = e - 1;
            }
        }
        firstedge[nv + i]->prev = e;
        e->next = firstedge[nv + i];

        DEBUGASSERT(degree[nv + i] == deg);
    }

    //DEBUGASSERT(ne == (ne_old + 10 + 6));

   if(use_next) {
       //nv
        e = firstedge[nv];
        e->end = path[0];
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = path[1];
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = nv + 1;
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = parallel_path[1];
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = parallel_path[0];
        edge_list[e->start][e->end] = e;

        //nv + 1
        e = firstedge[nv + 1];
        e->end = nv;
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = path[1];
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = path[2];
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = path[3];
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = nv + 2;
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = parallel_path[1];
        edge_list[e->start][e->end] = e;

        //nv + 2
        e = firstedge[nv+2];
        e->end = nv + 1;
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = path[3];
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = path[4];
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = parallel_path[2];
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = parallel_path[1];
        edge_list[e->start][e->end] = e;


        insert_edge_fuller(path[0], nv, parallel_path[0]);
        insert_edge_fuller(path[4], nv + 2, path[3]);
    } else {
        //Is identical but next instead of prev

        //nv
        e = firstedge[nv];
        e->end = path[0];
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = path[1];
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = nv + 1;
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = parallel_path[1];
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = parallel_path[0];
        edge_list[e->start][e->end] = e;

        //nv + 1
        e = firstedge[nv + 1];
        e->end = nv;
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = path[1];
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = path[2];
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = path[3];
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = nv + 2;
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = parallel_path[1];
        edge_list[e->start][e->end] = e;

        //nv + 2
        e = firstedge[nv+2];
        e->end = nv + 1;
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = path[3];
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = path[4];
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = parallel_path[2];
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = parallel_path[1];
        edge_list[e->start][e->end] = e;


        insert_edge_fuller(path[0], nv, path[1]);
        insert_edge_fuller(path[4], nv + 2, parallel_path[2]);
    }

    replace_neighbour(path[1], parallel_path[0], nv);
    replace_neighbour(path[1], parallel_path[1], nv + 1);
    replace_neighbour(path[2], parallel_path[1], nv + 1);
    replace_neighbour(path[3], parallel_path[1], nv + 1);
    replace_neighbour(path[3], parallel_path[2], nv + 2);

    replace_neighbour(parallel_path[0], path[1], nv);
    replace_neighbour(parallel_path[1], path[1], nv);
    replace_neighbour(parallel_path[1], path[2], nv + 1);
    replace_neighbour(parallel_path[1], path[3], nv + 2);
    replace_neighbour(parallel_path[2], path[3], nv + 2);


    //Update inverse edges and min
    EDGE *ee;
    for(i = 0; i < 3; i++) {
        e = firstedge[nv + i];
        for(j = 0; j < degree[nv + i]; j++) {
            //Could use set_inverse_edges but this is slightly less inefficient
            ee = edge_list[e->end][e->start];
            e->invers = ee;
            ee->invers = e;

/*
            if(e < ee) e->min = ee->min = e;
            else e->min = ee->min = ee;
*/

            e = e->next;
        }
    }

    replace_degree_5_vertex(path[0], nv);
    replace_degree_5_vertex(path[4], nv + 2);

    nv += 3;

}

/****************************************/

/*
 * Performs a B00 reduction.
 */
static void
reduce_bent_zero(int path[], int parallel_path[])
{
    nv -= 3;

    ne -= 16;

    //No need to restore firstedges, they will be updated by replace_neighbour

    /*
     * Also no need to restore edge_list since old values will still be valid:
     * 2 vertices which are not adjacent wont become adjacent after applying an
     * extension.
     */

    remove_edge(path[0], nv);
    remove_edge(path[4], nv + 2);

    restore_neighbour(path[1], nv, parallel_path[0]);
    restore_neighbour(path[1], nv + 1, parallel_path[1]);
    restore_neighbour(path[2], nv + 1, parallel_path[1]);
    restore_neighbour(path[3], nv + 1, parallel_path[1]);
    restore_neighbour(path[3], nv + 2, parallel_path[2]);

    restore_neighbour(parallel_path[0], nv, path[1]);
    restore_neighbour(parallel_path[1], nv, path[1]);
    restore_neighbour(parallel_path[1], nv + 1, path[2]);
    restore_neighbour(parallel_path[1], nv + 2, path[3]);
    restore_neighbour(parallel_path[2], nv + 2, path[3]);

    //Update inverse edges and min
    //This is not very efficient
    set_inverse_edges(path[1], parallel_path[0]);
    set_inverse_edges(path[1], parallel_path[1]);
    set_inverse_edges(path[2], parallel_path[1]);
    set_inverse_edges(path[3], parallel_path[1]);
    set_inverse_edges(path[3], parallel_path[2]);

    //replace_degree_5_vertex(nv, path[0]);
    //replace_degree_5_vertex(nv + 2, path[4]);
    restore_old_degree_5_vertex(path[0]);
    restore_old_degree_5_vertex(path[4]);

    FREEEDGES(16 + 2);

}

/**************************************************************************/

/*
 * Performs a B_{i,j} expansion, with i+j > 0.
 * Bent_position = i and bent_length = i+j.
 */
static void
extend_bent(EDGE *startedge, int path[], int parallel_path[], int bent_position,
        int bent_length, int use_next)
{

    //int ne_old = ne;

    DEBUGASSERT(bent_position <= bent_length);
    DEBUGASSERT(bent_length > 0);

    int i, j;
    EDGE *e;

    //firstedges will be updated by replace_neighbour

    //First create the new vertices and edges
    for(i = 0; i < bent_length + 3; i++) {
        degree[nv + i] = 0;
        firstedge[nv + i] = NULL;
        int deg;
        if(i == 0 || i == bent_length + 2)
            deg = 5;
        else
            deg = 6;
        for(j = 0; j < deg; j++) {
            NEWEDGE(e);
            e->label = ne;
            ne++;
            //degree[nv + i]++;
            e->start = nv + i;
            //e->end = path[i];
            e->invers = NULL;
            if(firstedge[nv + i] == NULL)
                firstedge[nv + i] = e;
            else {
                (e - 1)->next = e;
                e->prev = e - 1;
            }
        }
        firstedge[nv + i]->prev = e;
        e->next = firstedge[nv + i];

        degree[nv + i] = deg;
    }

    //DEBUGASSERT(ne == (ne_old + 10 + 6*(bent_length + 1)));

    for(i = 0; i <= bent_position; i++) {
        e = firstedge[nv + i];
        if(use_next) {
            if(i > 0) {
                e->end = nv + i - 1;
                edge_list[e->start][e->end] = e;
                e = e->prev;
            }

            e->end = path[i];
            edge_list[e->start][e->end] = e;
            e = e->prev;

            e->end = path[i + 1];
            edge_list[e->start][e->end] = e;
            e = e->prev;

            e->end = nv + i + 1;
            edge_list[e->start][e->end] = e;
            e = e->prev;

            e->end = parallel_path[i + 1];
            edge_list[e->start][e->end] = e;
            e = e->prev;

            e->end = parallel_path[i];
            edge_list[e->start][e->end] = e;
        } else {
            //Is identical but next instead of prev
            if(i > 0) {
                e->end = nv + i - 1;
                edge_list[e->start][e->end] = e;
                e = e->next;
            }

            e->end = path[i];
            edge_list[e->start][e->end] = e;
            e = e->next;

            e->end = path[i + 1];
            edge_list[e->start][e->end] = e;
            e = e->next;

            e->end = nv + i + 1;
            edge_list[e->start][e->end] = e;
            e = e->next;

            e->end = parallel_path[i + 1];
            edge_list[e->start][e->end] = e;
            e = e->next;

            e->end = parallel_path[i];
            edge_list[e->start][e->end] = e;
        }
    }

    //Add bent
    DEBUGASSERT(i == bent_position + 1);
    e = firstedge[nv + i];
    if(use_next) {
        e->end = nv + i - 1;
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = path[i];
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = path[i + 1];
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = path[i + 2];
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = nv + i + 1;
        edge_list[e->start][e->end] = e;
        e = e->prev;

        e->end = parallel_path[i];
        edge_list[e->start][e->end] = e;
    } else {
        e->end = nv + i - 1;
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = path[i];
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = path[i + 1];
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = path[i + 2];
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = nv + i + 1;
        edge_list[e->start][e->end] = e;
        e = e->next;

        e->end = parallel_path[i];
        edge_list[e->start][e->end] = e;

    }

    //Vertices past bent
    i++;
    for(; i < bent_length + 3; i++) {
        e = firstedge[nv + i];
        if(use_next) {
            e->end = nv + i - 1;
            edge_list[e->start][e->end] = e;
            e = e->prev;

            e->end = path[i + 1];
            edge_list[e->start][e->end] = e;
            e = e->prev;

            e->end = path[i + 2];
            edge_list[e->start][e->end] = e;
            e = e->prev;

            if(i < bent_length + 2) {
                e->end = nv + i + 1;
                edge_list[e->start][e->end] = e;
                e = e->prev;
            }

            e->end = parallel_path[i];
            edge_list[e->start][e->end] = e;
            e = e->prev;

            e->end = parallel_path[i - 1];
            edge_list[e->start][e->end] = e;
        } else {
            e->end = nv + i - 1;
            edge_list[e->start][e->end] = e;
            e = e->next;

            e->end = path[i + 1];
            edge_list[e->start][e->end] = e;
            e = e->next;

            e->end = path[i + 2];
            edge_list[e->start][e->end] = e;
            e = e->next;

            if(i < bent_length + 2) {
                e->end = nv + i + 1;
                edge_list[e->start][e->end] = e;
                e = e->next;
            }

            e->end = parallel_path[i];
            edge_list[e->start][e->end] = e;
            e = e->next;

            e->end = parallel_path[i - 1];
            edge_list[e->start][e->end] = e;
        }
    }

    if(use_next) {
        insert_edge_fuller(path[0], nv, parallel_path[0]);
        insert_edge_fuller(path[bent_length + 4], nv + bent_length + 2, path[bent_length + 3]);
    } else {
        insert_edge_fuller(path[0], nv, path[1]);
        insert_edge_fuller(path[bent_length + 4], nv + bent_length + 2, parallel_path[bent_length + 2]);
    }

    for(i = 1; i <= bent_position + 1; i++) {
        replace_neighbour(path[i], parallel_path[i - 1], nv + i - 1);
        replace_neighbour(path[i], parallel_path[i], nv + i);
    }
    for(i = 0; i <= bent_position; i++) {
        if(i > 0)
            replace_neighbour(parallel_path[i], path[i], nv + i - 1);
        replace_neighbour(parallel_path[i], path[i+1], nv + i);
    }

    //Bent
    i++;
    DEBUGASSERT(i == bent_position + 2);
    replace_neighbour(path[i], parallel_path[i - 1], nv + i - 1);
    replace_neighbour(parallel_path[i - 1], path[i - 1], nv + i - 2);
    replace_neighbour(parallel_path[i - 1], path[i], nv + i - 1);
    replace_neighbour(parallel_path[i - 1], path[i + 1], nv + i);

    i++;
    for(; i < bent_length + 4; i++) {
        replace_neighbour(path[i], parallel_path[i - 2], nv + i - 2);
        replace_neighbour(path[i], parallel_path[i - 1], nv + i - 1);
    }

    for(i = bent_position + 2; i < bent_length + 3; i++) {
        replace_neighbour(parallel_path[i], path[i+1], nv + i);
        if(i < bent_length + 2)
            replace_neighbour(parallel_path[i], path[i+2], nv + i + 1);
    }

    //Update inverse edges and min
    EDGE *ee;
    for(i = 0; i < bent_length + 3; i++) {
        e = firstedge[nv + i];
        for(j = 0; j < degree[nv + i]; j++) {
            //Could use set_inverse_edges but this is slightly less inefficient
            ee = edge_list[e->end][e->start];
            e->invers = ee;
            ee->invers = e;

/*
            if(e < ee) e->min = ee->min = e;
            else e->min = ee->min = ee;
*/

            e = e->next;
        }
    }

    replace_degree_5_vertex(path[0], nv);
    replace_degree_5_vertex(path[bent_length + 4], nv + bent_length + 2);

    nv += bent_length + 3;

}

/****************************************/

/*
 * Performs a B_{i,j} reduction, with i+j > 0.
 * Bent_position = i and bent_length = i+j.
 */
static void
reduce_bent(int path[], int parallel_path[], int bent_position, int bent_length)
{

    nv -= bent_length + 3;
    ne -= 6*(bent_length + 1) + 10;

    //No need to restore firstedges, they will be updated by restore_neighbour

    /*
     * Also no need to restore edge_list since old values will still be valid:
     * 2 vertices which are not adjacent wont become adjacent after applying an
     * extension.
     */

    remove_edge(path[0], nv);
    remove_edge(path[bent_length + 4], nv + bent_length + 2);

    int i;
    for(i = 1; i <= bent_position + 1; i++) {
        restore_neighbour(path[i], nv + i - 1, parallel_path[i - 1]);
        restore_neighbour(path[i], nv + i, parallel_path[i]);
    }
    for(i = 0; i <= bent_position; i++) {
        if(i > 0)
            restore_neighbour(parallel_path[i], nv + i - 1, path[i]);
        restore_neighbour(parallel_path[i], nv + i, path[i+1]);
    }

    //Bent
    i++;
    DEBUGASSERT(i == bent_position + 2);
    restore_neighbour(path[i], nv + i - 1, parallel_path[i - 1]);
    restore_neighbour(parallel_path[i - 1], nv + i - 2, path[i - 1]);
    restore_neighbour(parallel_path[i - 1], nv + i - 1, path[i]);
    restore_neighbour(parallel_path[i - 1], nv + i, path[i + 1]);

    i++;
    for(; i < bent_length + 4; i++) {
        restore_neighbour(path[i], nv + i - 2, parallel_path[i - 2]);
        restore_neighbour(path[i], nv + i - 1, parallel_path[i - 1]);
    }

    for(i = bent_position + 2; i < bent_length + 3; i++) {
        restore_neighbour(parallel_path[i], nv + i, path[i+1]);
        if(i < bent_length + 2)
            restore_neighbour(parallel_path[i], nv + i + 1, path[i+2]);
    }

    //Before bent
    for(i = 1; i < bent_position + 2; i++) {
        set_inverse_edges(path[i], parallel_path[i - 1]);
        set_inverse_edges(path[i], parallel_path[i]);
    }

    //Bent
    set_inverse_edges(path[i], parallel_path[i - 1]);
    i++;

    //After bent
    for(; i < bent_length + 4; i++) {
        set_inverse_edges(path[i], parallel_path[i - 2]);
        set_inverse_edges(path[i], parallel_path[i - 1]);
    }

    restore_old_degree_5_vertex(path[0]);
    restore_old_degree_5_vertex(path[bent_length + 4]);

    FREEEDGES(6*(bent_length + 1) + 10 + 2);

}

/***********Methods for testing canonicity of last operation***************/

/*
 * These canonicity methods were copied from plantri 4.1.
 */

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
    EDGE *startedge[MAXN+1]; /* startedge[i] is the starting edge for
                        exploring the vertex with the number i+1 */
    int number[MAXN], i;   /* The new numbers of the vertices, starting
                        at 1 in order to have "0" as a possibility to
                        mark ends of lists and not yet given numbers */
    int last_number, actual_number, vertex;

    for (i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;

    if (givenedge->start != givenedge->end) /* no loop start */
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else last_number = 1 ;

    actual_number = 1;
    temp = givenedge;

/* A representation is a clockwise ordering of all neighbours ended with a 0.
   The neighbours are numbered in the order that they are reached by the BFS
   procedure. In case a vertex is reached for the first time, not the (new)
   number of the vertex is listed, but its colour. When the list around a
   vertex is finished, it is ended with a 0. Since the colours can be
   distinguished from the vertices (requirement for the colour function), the
   adjacency list can be reconstructed: Every time a colour is listed, its
   number would be the smallest number not given yet.
   Since the edges when a vertex is reached for the first time are remembered,
   for these edges we in fact have more than just the vertex information -- for
   these edges we also have the exact information which edge occurs in the
   cyclic order. This makes the routine work also for double edges.

   Since every representation starts with the colour of vertex 2, which is
   the same colour all the time, we do not have to store that.

   In case of a loop as the starting point, the colour of 1 is omitted.
   Nevertheless also in this case it cannot be mixed up with a non-loop
   starting point, since the number of times a colour occurs is larger
   for loop starters than for non-loop starters.

   Every first entry in a new clockwise ordering (the starting point of the
   edge it was numbered from is determined by the entries before (the first
   time it occurs in the list to be exact), so this is not given either.
   The K4 could be denoted  c3 c4 0 4 3 0 2 3 0 3 2 0 if ci is the colour
   of vertex i.  Note that the colour of vertex 1 is -- by definition --
   always the smallest one */

    while (last_number < nv)
    {
        for (run = temp->next; run != temp; run = run->next)
    /* this loop marks all edges around temp->origin. */
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                vertex = colour[vertex]; }
            else vertex = number[vertex];
            if (vertex > (*representation)) return 0;
            if (vertex < (*representation)) return 2;
            representation++;
          }
   /* check whether representation[] is also at the end of a cyclic list */
        if ((*representation) != 0) return 2;
        representation++;
   /* Next vertex to explore: */
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nv)
                /* Now we know that all numbers have been given */
    {
        for (run = temp->next; run != temp; run = run->next)
    /* this loop marks all edges around temp->origin. */
          {
            vertex = number[run->end];
            if (vertex > (*representation)) return 0;
            if (vertex < (*representation)) return 2;
            representation++;
          }
   /* check whether representation[] is also at the end of a cyclic list */
        if ((*representation) != 0) return 2;
        representation++;
   /* Next vertex to explore: */
        temp = startedge[actual_number];  actual_number++;
    }

    return 1;
}


/**************************************************************************/

/*
 * Does the same as testcanon(), but the scan visits at most NV_CANON_SHORT
 * instead of nv vertices.
 */
static int
testcanon_short(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->next" direction, an automorphism or even a better representation
   can be found. Returns 0 for failure, 1 for an automorphism and 2 for
   a better representation.  This function exits as soon as a better
   representation is found. A function that computes and returns the
   complete better representation can work pretty similar.*/
{
    EDGE *temp, *run;
    EDGE *startedge[MAXN+1]; /* startedge[i] is the starting edge for
                        exploring the vertex with the number i+1 */
    int number[MAXN], i;   /* The new numbers of the vertices, starting
                        at 1 in order to have "0" as a possibility to
                        mark ends of lists and not yet given numbers */
    int last_number, actual_number, vertex;

    for (i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;

    if (givenedge->start != givenedge->end) /* no loop start */
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else last_number = 1 ;

    actual_number = 1;
    temp = givenedge;

/* A representation is a clockwise ordering of all neighbours ended with a 0.
   The neighbours are numbered in the order that they are reached by the BFS
   procedure. In case a vertex is reached for the first time, not the (new)
   number of the vertex is listed, but its colour. When the list around a
   vertex is finished, it is ended with a 0. Since the colours can be
   distinguished from the vertices (requirement for the colour function), the
   adjacency list can be reconstructed: Every time a colour is listed, its
   number would be the smallest number not given yet.
   Since the edges when a vertex is reached for the first time are remembered,
   for these edges we in fact have more than just the vertex information -- for
   these edges we also have the exact information which edge occurs in the
   cyclic order. This makes the routine work also for double edges.

   Since every representation starts with the colour of vertex 2, which is
   the same colour all the time, we do not have to store that.

   In case of a loop as the starting point, the colour of 1 is omitted.
   Nevertheless also in this case it cannot be mixed up with a non-loop
   starting point, since the number of times a colour occurs is larger
   for loop starters than for non-loop starters.

   Every first entry in a new clockwise ordering (the starting point of the
   edge it was numbered from is determined by the entries before (the first
   time it occurs in the list to be exact), so this is not given either.
   The K4 could be denoted  c3 c4 0 4 3 0 2 3 0 3 2 0 if ci is the colour
   of vertex i.  Note that the colour of vertex 1 is -- by definition --
   always the smallest one */

    while (last_number < NV_CANON_SHORT)
    {
        for (run = temp->next; run != temp; run = run->next)
    /* this loop marks all edges around temp->origin. */
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                vertex = colour[vertex]; }
            else vertex = number[vertex];
            if (vertex > (*representation)) return 0;
            if (vertex < (*representation)) return 2;
            representation++;
          }
   /* check whether representation[] is also at the end of a cyclic list */
        if ((*representation) != 0) return 2;
        representation++;
   /* Next vertex to explore: */
        temp = startedge[actual_number];  actual_number++;
    }

/*
    while (actual_number <= nv)
    {
        for (run = temp->next; run != temp; run = run->next)
          {
            vertex = number[run->end];
            if (vertex > (*representation)) return 0;
            if (vertex < (*representation)) return 2;
            representation++;
          }
        if ((*representation) != 0) return 2;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }
*/

    return 1;
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
    EDGE *startedge[MAXN+1];
    int number[MAXN], i;
    int last_number, actual_number, vertex;

    for (i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;

    if (givenedge->start != givenedge->end)
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else last_number = 1;

    actual_number = 1;
    temp = givenedge;

    while (last_number < nv)
    {
        for (run = temp->prev; run != temp; run = run->prev)
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                vertex = colour[vertex]; }
            else vertex = number[vertex];
            if (vertex > (*representation)) return 0;
            if (vertex < (*representation)) return 2;
            representation++;
          }
        if ((*representation) != 0) return 2;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nv)
    {
        for (run = temp->prev; run != temp; run = run->prev)
          {
            vertex = number[run->end];
            if (vertex > (*representation)) return 0;
            if (vertex < (*representation)) return 2;
            representation++;
          }
        if ((*representation) != 0) return 2;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

    return 1;
}

/*****************************************************************************/

static int
testcanon_mirror_short(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->prev" direction, an automorphism or even a better representation can
   be found. Comments see testcanon -- it is exactly the same except for
   the orientation */
{
    EDGE *temp, *run;
    EDGE *startedge[MAXN+1];
    int number[MAXN], i;
    int last_number, actual_number, vertex;

    for (i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;

    if (givenedge->start != givenedge->end)
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else last_number = 1;

    actual_number = 1;
    temp = givenedge;

    while (last_number < NV_CANON_SHORT)
    {
        for (run = temp->prev; run != temp; run = run->prev)
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                vertex = colour[vertex]; }
            else vertex = number[vertex];
            if (vertex > (*representation)) return 0;
            if (vertex < (*representation)) return 2;
            representation++;
          }
        if ((*representation) != 0) return 2;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

/*
    while (actual_number <= nv)
    {
        for (run = temp->prev; run != temp; run = run->prev)
          {
            vertex = number[run->end];
            if (vertex > (*representation)) return 0;
            if (vertex < (*representation)) return 2;
            representation++;
          }
        if ((*representation) != 0) return 2;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }
*/

    return 1;
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
    EDGE *run;
    int vertex;
    EDGE *temp;
    EDGE *startedge[MAXN+1];
    int number[MAXN], i;
    int last_number, actual_number;

    for (i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;
    if (givenedge->start != givenedge->end)
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else last_number = 1;

    actual_number = 1;
    temp = givenedge;

    while (last_number < nv)
    {
        for (run = temp->next; run != temp; run = run->next)
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                *representation = colour[vertex]; }
            else *representation = number[vertex];
            representation++;
          }
        *representation = 0;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nv)
    {
        for (run = temp->next; run != temp; run = run->next)
          {
            *representation = number[run->end]; representation++;
          }
        *representation = 0;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

    return;
}

/****************************************************************************/

static void
testcanon_first_init_short(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->next" direction, an automorphism or even a better representation can
   be found. A better representation will be completely constructed and
   returned in "representation".  It works pretty similar to testcanon except
   for obviously necessary changes, so for extensive comments see testcanon */
{
    EDGE *run;
    int vertex;
    EDGE *temp;
    EDGE *startedge[MAXN+1];
    int number[MAXN], i;
    int last_number, actual_number;

    for (i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;
    if (givenedge->start != givenedge->end)
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else last_number = 1;

    actual_number = 1;
    temp = givenedge;

    while (last_number < NV_CANON_SHORT)
    {
        for (run = temp->next; run != temp; run = run->next)
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                *representation = colour[vertex]; }
            else *representation = number[vertex];
            representation++;
          }
        *representation = 0;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

/*
    while (actual_number <= nv)
    {
        for (run = temp->next; run != temp; run = run->next)
          {
            *representation = number[run->end]; representation++;
          }
        *representation = 0;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }
*/

    return;
}

/****************************************************************************/

static void
testcanon_first_init_mirror(EDGE *givenedge, int representation[],
			    int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->prev" direction, an automorphism or even a better representation can
   be found. A better representation will be completely constructed and
   returned in "representation".  It works pretty similar to testcanon except
   for obviously necessary changes, so for extensive comments see testcanon */
{
    EDGE *run;
    int vertex;
    EDGE *temp;
    EDGE *startedge[MAXN+1];
    int number[MAXN], i;
    int last_number, actual_number;

    for (i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;
    if (givenedge->start != givenedge->end)
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else last_number = 1;

    actual_number = 1;
    temp = givenedge;

    while (last_number < nv)
    {
        for (run = temp->prev; run != temp; run = run->prev)
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                *representation = colour[vertex]; }
            else *representation = number[vertex];
            representation++;
          }
        *representation = 0;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nv)
    {
        for (run = temp->prev; run != temp; run = run->prev)
          {
            *representation = number[run->end]; representation++;
          }
        *representation = 0;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

    return;
}

/****************************************************************************/

static void
testcanon_first_init_mirror_short(EDGE *givenedge, int representation[],
			    int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->prev" direction, an automorphism or even a better representation can
   be found. A better representation will be completely constructed and
   returned in "representation".  It works pretty similar to testcanon except
   for obviously necessary changes, so for extensive comments see testcanon */
{
    EDGE *run;
    int vertex;
    EDGE *temp;
    EDGE *startedge[MAXN+1];
    int number[MAXN], i;
    int last_number, actual_number;

    for (i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;
    if (givenedge->start != givenedge->end)
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else last_number = 1;

    actual_number = 1;
    temp = givenedge;

    while (last_number < NV_CANON_SHORT)
    {
        for (run = temp->prev; run != temp; run = run->prev)
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                *representation = colour[vertex]; }
            else *representation = number[vertex];
            representation++;
          }
        *representation = 0;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

/*
    while (actual_number <= nv)
    {
        for (run = temp->prev; run != temp; run = run->prev)
          {
            *representation = number[run->end]; representation++;
          }
        *representation = 0;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }
*/

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
    EDGE *run;
    int vertex;
    EDGE *temp;
    EDGE *startedge[MAXN+1];
    int number[MAXN], i;
    int better = 0; /* is the representation already better ? */
    int last_number, actual_number;

    for (i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;
    if (givenedge->start != givenedge->end)
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else last_number = 1;

    actual_number = 1;
    temp = givenedge;

    while (last_number < nv)
    {
        for (run = temp->next; run != temp; run = run->next)
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                vertex = colour[vertex]; }
            else vertex=number[vertex];
            if (better) *representation = vertex;
             else { if (vertex > (*representation)) return 0;
                     else if (vertex < (*representation))
                       { better = 1; *representation = vertex; }
                  }
            representation++;
          }
        if ((*representation) != 0) { better = 1; *representation = 0; }
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nv)
    {
        for (run = temp->next; run != temp; run = run->next)
          { vertex = number[run->end];
            if (better) *representation = vertex;
            else
              {
                if (vertex > (*representation)) return 0;
                if (vertex < (*representation))
                  { better = 1; *representation = vertex; }
              }
            representation++;
          }
        if ((*representation) != 0) { better = 1; *representation = 0; }
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

    if (better) return 2;
    return 1;
}

/****************************************************************************/

static int
testcanon_init_short(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->next" direction, an automorphism or even a better representation can
   be found. A better representation will be completely constructed and
   returned in "representation".  It works pretty similar to testcanon except
   for obviously necessary changes, so for extensive comments see testcanon */
{
    EDGE *run;
    int vertex;
    EDGE *temp;
    EDGE *startedge[MAXN+1];
    int number[MAXN], i;
    int better = 0; /* is the representation already better ? */
    int last_number, actual_number;

    for (i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;
    if (givenedge->start != givenedge->end)
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else last_number = 1;

    actual_number = 1;
    temp = givenedge;

    while (last_number < NV_CANON_SHORT)
    {
        for (run = temp->next; run != temp; run = run->next)
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                vertex = colour[vertex]; }
            else vertex=number[vertex];
            if (better) *representation = vertex;
             else { if (vertex > (*representation)) return 0;
                     else if (vertex < (*representation))
                       { better = 1; *representation = vertex; }
                  }
            representation++;
          }
        if ((*representation) != 0) { better = 1; *representation = 0; }
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

/*
    while (actual_number <= nv)
    {
        for (run = temp->next; run != temp; run = run->next)
          { vertex = number[run->end];
            if (better) *representation = vertex;
            else
              {
                if (vertex > (*representation)) return 0;
                if (vertex < (*representation))
                  { better = 1; *representation = vertex; }
              }
            representation++;
          }
        if ((*representation) != 0) { better = 1; *representation = 0; }
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }
*/

    if (better) return 2;
    return 1;
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
    EDGE *startedge[MAXN+1];
    int number[MAXN], i;
    int better = 0; /* is the representation already better ? */
    int last_number, actual_number, vertex;

    for (i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;
    if (givenedge->start != givenedge->end)
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else last_number = 1;

    actual_number = 1;
    temp = givenedge;

    while (last_number < nv)
    {
        for (run = temp->prev; run != temp; run = run->prev)
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                vertex = colour[vertex]; }
            else vertex=number[vertex];
            if (better) *representation = vertex;
            else { if (vertex > (*representation)) return 0;
                   else if (vertex < (*representation))
                     { better = 1; *representation = vertex; }
                 }
            representation++;
          }
        if ((*representation) != 0) { better = 1; *representation = 0; }
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nv)
    {
        for (run = temp->prev; run != temp; run = run->prev)
          { vertex = number[run->end];
            if (better) *representation = vertex;
            else
              {
                if (vertex > (*representation)) return 0;
                if (vertex < (*representation))
                  { better = 1; *representation = vertex; }
              }
            representation++;
          }
       if ((*representation) != 0) { better = 1; *representation = 0; }
       representation++;
       temp = startedge[actual_number];  actual_number++;
    }

    if (better) return 2;
    return 1;
}


/****************************************************************************/

static int
testcanon_mirror_init_short(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->prev" direction, an automorphism or even a better representation can
   be found. A better representation will be completely constructed and
   returned in "representation".  It works pretty similar to testcanon except
   for obviously necessary changes, so for extensive comments see testcanon */
{
    EDGE *temp, *run;
    EDGE *startedge[MAXN+1];
    int number[MAXN], i;
    int better = 0; /* is the representation already better ? */
    int last_number, actual_number, vertex;

    for (i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;
    if (givenedge->start != givenedge->end)
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else last_number = 1;

    actual_number = 1;
    temp = givenedge;

    while (last_number < NV_CANON_SHORT)
    {
        for (run = temp->prev; run != temp; run = run->prev)
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                vertex = colour[vertex]; }
            else vertex=number[vertex];
            if (better) *representation = vertex;
            else { if (vertex > (*representation)) return 0;
                   else if (vertex < (*representation))
                     { better = 1; *representation = vertex; }
                 }
            representation++;
          }
        if ((*representation) != 0) { better = 1; *representation = 0; }
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

/*
    while (actual_number <= nv)
    {
        for (run = temp->prev; run != temp; run = run->prev)
          { vertex = number[run->end];
            if (better) *representation = vertex;
            else
              {
                if (vertex > (*representation)) return 0;
                if (vertex < (*representation))
                  { better = 1; *representation = vertex; }
              }
            representation++;
          }
       if ((*representation) != 0) { better = 1; *representation = 0; }
       representation++;
       temp = startedge[actual_number];  actual_number++;
    }
*/

    if (better) return 2;
    return 1;
}

/****************************************************************************/

static void
construct_numb(EDGE *givenedge, EDGE *numbering[])

/* Starts at givenedge and writes the edges in the well defined order
   into the list.  Works like testcanon. Look there for comments. */
{
    EDGE *temp, **tail, **limit, *run;
    EDGE *startedge[MAXN+1];
    int last_number, actual_number, vertex;

    RESETMARKS_V;

    tail = numbering;
    limit = numbering+ne-1;

    MARK_V(givenedge->start);
    if (givenedge->start != givenedge->end)
      {
	MARK_V(givenedge->end);
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else last_number = 1;

    actual_number = 1;
    temp = *tail = givenedge;

    while (last_number < nv)
    {
        for (run = temp->next; run != temp; run = run->next)
    /* this loop marks all edges around temp->origin. */
          { vertex = run->end;
            if (!ISMARKED_V(vertex))
              { startedge[last_number] = run->invers;
                last_number++; MARK_V(vertex); }
                tail++; *tail = run;
          }
          if (tail != limit)
            { tail++;
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
}

/****************************************************************************/

static void
construct_numb_mirror(EDGE *givenedge, EDGE *numbering[])

/* Starts at givenedge and writes the edges in the well defined order
   into the list.  Works like testcanon. Look there for comments.  */
{
    EDGE *temp, **tail, **limit, *run;
    EDGE *startedge[MAXN+1];
    int last_number, actual_number, vertex;

    RESETMARKS_V;

    tail = numbering; /* The first entry of the numbering list */
    limit = numbering+ne-1;  /* Last valid entry of the numbering list */

    MARK_V(givenedge->start);
    if (givenedge->start != givenedge->end)
      {
	MARK_V(givenedge->end);
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else last_number = 1;

    actual_number = 1;
    temp = *tail = givenedge;

    while (last_number < nv)
    {
        for (run = temp->prev; run != temp; run = run->prev)
    /* this loop marks all edges around temp->origin. */
          { vertex = run->end;
            if (!ISMARKED_V(vertex))
              { startedge[last_number] = run->invers;
                last_number++; MARK_V(vertex); }
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
}

/****************************************************************************/

static int
canon(int lcolour[], EDGE *can_numberings[][MAXE],
      int *num_can_numberings, int *num_can_numberings_or_pres)

/* Checks whether the last vertex (number: nv-1) is canonical or not.
   Returns 1 if yes, 0 if not. One of the criterions a canonical vertex
   must fulfill is that its colour is minimal.

   IT IS ASSUMED that the values of the colour function are positive
   and at most INT_MAX-MAXN.

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
   adjacent to nv-1. In case of an orientation preserving numbering the edges
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
    int representation[MAXE];
    EDGE *numblist[MAXE], *numblist_mirror[MAXE]; /* lists of edges where
                        starting gives a canonical representation */
    int numbs = 1, numbs_mirror = 0;
    int colour[MAXN];

    for (i=0; i<nv; i++) colour[i]=lcolour[i]+MAXN;
                               /* to distinguish colours from vertices */
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
      { if (colour[i] < minstart) return 0;
        if (colour[i] == minstart)
          { run = end = firstedge[i];
            do
            {
              if (colour[run->end] > maxend) return 0;
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
        numblist_mirror[0] = startlist_last[0]; }

    for (i = 1; i < list_length_last; i++)
      { test = testcanon_init(startlist_last[i], representation, colour);
        if (test == 1) { numblist[numbs] = startlist_last[i]; numbs++; }
        else if (test == 2)
          { numbs_mirror = 0; numbs = 1; numblist[0] = startlist_last[i]; }
            test = testcanon_mirror_init(startlist_last[i],
                                                     representation, colour);
          if (test == 1)
            { numblist_mirror[numbs_mirror] = startlist_last[i];
              numbs_mirror++; }
          else if (test == 2)
            { numbs_mirror = 1; numbs = 0;
              numblist_mirror[0] = startlist_last[i]; }
      }

/* Now we know the best representation we can obtain starting at last_vertex.
   Now we will check all the others. We can return 0 at once if we find a
   better one */

    for (i = 0; i < list_length; i++)
      { test = testcanon(startlist[i], representation, colour);
        if (test == 1) { numblist[numbs] = startlist[i]; numbs++; }
        else if (test == 2) return 0;
        test = testcanon_mirror(startlist[i], representation, colour);
        if (test == 1)
          { numblist_mirror[numbs_mirror] = startlist[i]; numbs_mirror++; }
        else if (test == 2) return 0;
      }

    *num_can_numberings_or_pres = numbs;
    *num_can_numberings = numbs+numbs_mirror;

    if (*num_can_numberings>1)
      { for (i = 0; i < numbs; i++)
          construct_numb(numblist[i], can_numberings[i]);
        for (i = 0; i < numbs_mirror; i++, numbs++)
          construct_numb_mirror(numblist_mirror[i],can_numberings[numbs]);
      }
    else
      { if (numbs) can_numberings[0][0] = numblist[0];
        else can_numberings[0][0] = numblist_mirror[0]; }

    return 1;
}

/****************************************************************************/

static void
construct_canform(EDGE *givenedge, unsigned char code[])

/* Starts at givenedge and writes the canonical form
   into the list.  Works like compute_code. Look there for comments. */
{
    EDGE *run;
    int vertex;
    EDGE *temp;
    EDGE *startedge[MAXN+1];
    int number[MAXN+1], i;
    int last_number, actual_number;
    //EDGE *givenedge;

    for (i = 0; i < nv; i++) number[i] = 0;

    *code=nv; code++;
/*
    if (code_edge==NULL) givenedge=firstedge[0];
    else { givenedge=code_edge; number[nv]=0; }
*/
    number[givenedge->start] = 1;
    if (givenedge->start != givenedge->end)
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else  last_number = 1;

    actual_number = 1;
    temp = givenedge;

    while (last_number < nv)
    {   *code=number[temp->end]; code++;
        for (run = temp->next; run != temp; run = run->next)
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                *code = last_number; }
            else *code = number[vertex];
            code++;
          }
        *code = 0;  code++;
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nv)
    {  	*code=number[temp->end]; code++;
        for (run = temp->next; run != temp; run = run->next)
          {
            *code = number[run->end]; code++;
          }
        *code = 0;
        code++;
        temp = startedge[actual_number];  actual_number++;
    }
}


/****************************************************************************/

static void
construct_canform_mirror(EDGE *givenedge, unsigned char code[])

/* Starts at givenedge and writes the canonical form
   into the list.  Works like compute_code. Look there for comments. */ {


    EDGE *run;
    int vertex;
    EDGE *temp;
    EDGE *startedge[MAXN+1];
    int number[MAXN+1], i;
    int last_number, actual_number;
    //EDGE *givenedge;

    for (i = 0; i < nv; i++) number[i] = 0;

    *code=nv; code++;
/*
    if (code_edge==NULL) givenedge=firstedge[0];
    else { givenedge=code_edge; number[nv]=0; }
*/
    number[givenedge->start] = 1;
    if (givenedge->start != givenedge->end)
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else  last_number = 1;

    actual_number = 1;
    temp = givenedge;

    while (last_number < nv)
    {   *code=number[temp->end]; code++;
        for (run = temp->prev; run != temp; run = run->prev)
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                *code = last_number; }
            else *code = number[vertex];
            code++;
          }
        *code = 0;  code++;
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nv)
    {  	*code=number[temp->end]; code++;
        for (run = temp->prev; run != temp; run = run->prev)
          {
            *code = number[run->end]; code++;
          }
        *code = 0;
        code++;
        temp = startedge[actual_number];  actual_number++;
    }
    
}

/****************************************************************************/

/*
 * These methods use a different representation (which is less optimized).
 * Was copied from non_iso_pl.c so the group can be computed even if the graph
 * is not canonical.
 * This was easier and more trustworthy than modifying the existing canon()-methods.
 */

static void
testcanon_first_init_other(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->next" direction, an automorphism or even a better representation can 
   be found. A better representation will be completely constructed and 
   returned in "representation".  It works pretty similar to testcanon except 
   for obviously necessary changes, so for extensive comments see testcanon */ {
    register EDGE *run;
    register int vertex;
    EDGE *temp;
    EDGE * startedge[MAXN + 1];
    int number[MAXN], i;
    int last_number, actual_number;

    for(i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;
    if(givenedge->start != givenedge->end) {
        number[givenedge->end] = 2;
        actual_number = 1;
        last_number = 2;
        startedge[1] = givenedge->invers;
    } else {
        last_number = actual_number = 1;
    }

    temp = givenedge;

    while(last_number < nv) {
        for(run = temp->next; run != temp; run = run->next) {
            vertex = run->end;
            if(!number[vertex]) {
                startedge[last_number] = run->invers;
                last_number++;
                number[vertex] = last_number;
            }
            *representation = colour[vertex];
            representation++;
            *representation = number[vertex];
            representation++;
        }
        *representation = 0;
        representation++;
        temp = startedge[actual_number];
        actual_number++;
    }

    while(actual_number <= nv) {
        for(run = temp->next; run != temp; run = run->next) {
            vertex = run->end;
            *representation = colour[vertex];
            representation++;
            *representation = number[vertex];
            representation++;
        }
        *representation = 0;
        representation++;
        temp = startedge[actual_number];
        actual_number++;
    }

    return;
}

/****************************************************************************/

static int
testcanon_init_other(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->next" direction, an automorphism or even a better representation can 
   be found. A better representation will be completely constructed and 
   returned in "representation".  It works pretty similar to testcanon except 
   for obviously necessary changes, so for extensive comments see testcanon */ {
    register EDGE *run;
    register int col, vertex;
    EDGE *temp;
    EDGE * startedge[MAXN + 1];
    int number[MAXN], i;
    int better = 0; /* is the representation already better ? */
    int last_number, actual_number;


    for(i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;
    if(givenedge->start != givenedge->end) {
        number[givenedge->end] = 2;
        actual_number = 1;
        last_number = 2;
        startedge[1] = givenedge->invers;
    } else {
        last_number = actual_number = 1;
    }

    temp = givenedge;

    while(last_number < nv) {
        for(run = temp->next; run != temp; run = run->next) {
            vertex = run->end;
            col = colour[vertex];
            if(!number[vertex]) {
                startedge[last_number] = run->invers;
                last_number++;
                number[vertex] = last_number;
            }
            if(better) {
                *representation = col;
                representation++;
                *representation = number[vertex];
                representation++;
            } else {
                if(col > (*representation)) return(0);
                if(col < (*representation)) {
                    better = 1;
                    *representation = col;
                    representation++;
                    *representation = number[vertex];
                    representation++;
                } else {
                    representation++;
                    vertex = number[vertex];
                    if(vertex > (*representation)) return(0);
                    if(vertex < (*representation)) {
                        better = 1;
                        *representation = vertex;
                    }
                    representation++;
                }
            }
        }
        if((*representation) != 0) {
            better = 1;
            *representation = 0;
        }
        representation++;
        temp = startedge[actual_number];
        actual_number++;
    }

    while(actual_number <= nv) {
        for(run = temp->next; run != temp; run = run->next) {
            vertex = run->end;
            col = colour[vertex];
            if(better) {
                *representation = col;
                representation++;
                *representation = number[vertex];
                representation++;
            } else {
                if(col > (*representation)) return(0);
                if(col < (*representation)) {
                    better = 1;
                    *representation = col;
                    representation++;
                    *representation = number[vertex];
                    representation++;
                } else {
                    representation++;
                    vertex = number[vertex];
                    if(vertex > (*representation)) return(0);
                    if(vertex < (*representation)) {
                        better = 1;
                        *representation = vertex;
                    }
                    representation++;
                }
            }
        }
        if((*representation) != 0) {
            better = 1;
            *representation = 0;
        }
        representation++;
        temp = startedge[actual_number];
        actual_number++;
    }

    if(better) return(2);
    return(1);
}

/****************************************************************************/

static int
testcanon_mirror_init_other(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->prev" direction, an automorphism or even a better representation can 
   be found. A better representation will be completely constructed and 
   returned in "representation".  It works pretty similar to testcanon except 
   for obviously necessary changes, so for extensive comments see testcanon */ {
    EDGE *temp, *run;
    EDGE * startedge[MAXN + 1];
    int number[MAXN], i;
    int better = 0; /* is the representation already better ? */
    int last_number, actual_number, vertex, col;


    for(i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1;
    if(givenedge->start != givenedge->end) {
        number[givenedge->end] = 2;
        actual_number = 1;
        last_number = 2;
        startedge[1] = givenedge->invers;
    } else {
        last_number = actual_number = 1;
    }

    temp = givenedge;

    while(last_number < nv) {
        for(run = temp->prev; run != temp; run = run->prev) {
            vertex = run->end;
            col = colour[vertex];
            if(!number[vertex]) {
                startedge[last_number] = run->invers;
                last_number++;
                number[vertex] = last_number;
            }
            if(better) {
                *representation = col;
                representation++;
                *representation = number[vertex];
                representation++;
            } else {
                if(col > (*representation)) return(0);
                if(col < (*representation)) {
                    better = 1;
                    *representation = col;
                    representation++;
                    *representation = number[vertex];
                    representation++;
                } else {
                    representation++;
                    vertex = number[vertex];
                    if(vertex > (*representation)) return(0);
                    if(vertex < (*representation)) {
                        better = 1;
                        *representation = vertex;
                    }
                    representation++;
                }
            }
        }
        if((*representation) != 0) {
            better = 1;
            *representation = 0;
        }
        representation++;
        temp = startedge[actual_number];
        actual_number++;
    }

    while(actual_number <= nv) {
        for(run = temp->prev; run != temp; run = run->prev) {
            vertex = run->end;
            col = colour[vertex];
            if(better) {
                *representation = col;
                representation++;
                *representation = number[vertex];
                representation++;
            } else {
                if(col > (*representation)) return(0);
                if(col < (*representation)) {
                    better = 1;
                    *representation = col;
                    representation++;
                    *representation = number[vertex];
                    representation++;
                } else {
                    representation++;
                    vertex = number[vertex];
                    if(vertex > (*representation)) return(0);
                    if(vertex < (*representation)) {
                        better = 1;
                        *representation = vertex;
                    }
                    representation++;
                }
            }
        }
        if((*representation) != 0) {
            better = 1;
            *representation = 0;
        }
        representation++;
        temp = startedge[actual_number];
        actual_number++;
    }

    if(better) return(2);
    return(1);
}

/****************************************************************************/

/*
 * Computes the group of the current graph, even if it is not canonical.
 * This method is based on canon(), but uses a different canonical form.
 * 
 * Works OK if at least one vertex has valence >= 3. Otherwise some numberings 
 * are computed twice, since changing the orientation (the cyclic order around 
 * each vertex) doesn't change anything .
 *
 * Does __NOT__ work for maps with loops, except in some special
 * cases (e.g.triangulations), but works for maps with double edges.
 *
 * Important: it is assumed that vertices with minimal colour have degree <= 5. 
 */
static void
compute_group(int colour[], EDGE *can_numberings[][MAXE],
        int *num_can_numberings, int *num_can_numberings_or_pres) {
    int i, j, test;
    int minstart, maxend; /* (minstart,maxend) will be the chosen colour 
				 pair of an edge */
    EDGE * startlist[5 * MAXN], *run;
    int list_length;
    int representation[2 * MAXE + MAXN];
    EDGE * numblist[MAXE], *numblist_mirror[MAXE]; /* lists of edges where 
						starting gives a canonical representation */
    int numbs = 1, numbs_mirror = 0;

    minstart = colour[0];
    for(i = 1; i < nv; i++) if(colour[i] < minstart) minstart = colour[i];

    list_length = 0;
    maxend = 0;
    for(i = 0; i < nv; i++) {
        if(colour[i] == minstart) {
            run = firstedge[i];
            if(degree[i] > 5) {
                fprintf(stderr, "Error: vertices with minimal colour are assumed to have degree <= 5\n");
                exit(1);
            }
            for(j = 0; j < degree[i]; j++, run = run->next)
                if(colour[run->end] > maxend) {
                    list_length = 1;
                    startlist[0] = run;
                    maxend = colour[run->end];
                } else if(colour[run->end] == maxend) {
                    startlist[list_length] = run;
                    list_length++;
                }
        }
    }


    /* OK -- now we have all the pairs and have to determine the smallest representation */

    testcanon_first_init_other(startlist[0], representation, colour);
    numblist[0] = startlist[0];
    test = testcanon_mirror_init_other(startlist[0], representation, colour);
    if(test == 1) {
        numbs_mirror = 1;
        numblist_mirror[0] = startlist[0];
    } else if(test == 2) {
        numbs_mirror = 1;
        numbs = 0;
        numblist_mirror[0] = startlist[0];
    }

    for(i = 1; i < list_length; i++) {
        test = testcanon_init_other(startlist[i], representation, colour);
        if(test == 1) {
            numblist[numbs] = startlist[i];
            numbs++;
        } else if(test == 2) {
            numbs_mirror = 0;
            numbs = 1;
            numblist[0] = startlist[i];
        }

        test = testcanon_mirror_init_other(startlist[i],
                representation, colour);
        if(test == 1) {
            numblist_mirror[numbs_mirror] = startlist[i];
            numbs_mirror++;
        } else if(test == 2) {
            numbs_mirror = 1;
            numbs = 0;
            numblist_mirror[0] = startlist[i];
        }
    }

    *num_can_numberings_or_pres = numbs;
    *num_can_numberings = numbs + numbs_mirror;
    
    if(*num_can_numberings > 1) {
        for(i = 0; i < numbs; i++)
            construct_numb(numblist[i], can_numberings[i]);
        for(i = 0; i < numbs_mirror; i++, numbs++)
            construct_numb_mirror(numblist_mirror[i], can_numberings[numbs]);
    } else {
        if(numbs) can_numberings[0][0] = numblist[0];
        else can_numberings[0][0] = numblist_mirror[0];
    }
    
}

/****************************************************************************/

static void
canon_form(int colour[], EDGE *can_numberings[][MAXE],
        int *num_can_numberings, int *num_can_numberings_or_pres,
        unsigned char can_form[])

/* Computes the canonical form of the global firstedge[] map 
   and writes it into can_form.
   One of the criterions a canonical starting vertex must fulfill, 
   is that its colour is minimal. 
   A possible starting edge for the construction of a representation is 
   one with lexicographically minimal colour pair (start,INT_MAX-end).

   For information about can_numberings, see canon(). 
  
   Works OK if at least one vertex has valence >= 3. Otherwise some numberings 
   are computed twice, since changing the orientation (the cyclic order around 
   each vertex) doesn't change anything 

   Does __NOT__ work for maps with loops, except in some special
   cases (e.g.triangulations), but works for maps with double edges.
 
   Important: it is assumed that vertices with minimal colour have degree <= 5.

 */
 {
    compute_group(colour, can_numberings, num_can_numberings, num_can_numberings_or_pres);

    //Important: it is assumed that the or pres automorphisms appear first in can_numberings.
    
    if(*num_can_numberings_or_pres > 0)
        construct_canform(can_numberings[0][0], can_form);
    else
        construct_canform_mirror(can_numberings[0][0], can_form);

}

/****************************************************************************/

static int
canon_edge(EDGE *edgelist[], int num_edges,
           int lcolour[], EDGE *can_numberings[][MAXE],
           int *num_can_numberings, int *num_can_numberings_or_pres)

/*
   IT IS ASSUMED that the values of the colour function are positive and
   at most INT_MAX-MAXN.

   In case edgelist[0] == edgelist[1]->inverse, it checks whether
   edgelist[0] or edgelist[1] are canonical. Otherwise only
   edgelist[0] is checked to be canonical.

   It is only compared with the other edges contained in edgelist.
   The number of those edges in the list is given in num_edges.
   Returns 1 if yes, 0 if not.

   Edges given are not in minimal form -- but it is guaranteed that all
   colours of the startpoints are the same and all colours of the endpoints
   are the same.

   In case of only the identity automorphism, the entries of can_numberings[][]
   are undefined.

   Otherwise in can_numberings[][] the canonical numberings are stored as
   sequences of oriented edges.  For every 0 <= i,j < *num_can_numberings
   and every 0 <= k < ne the edges can_numberings[i][k] and
   can_numberings[j][k] can be mapped onto each other by an automorphism.
   The first *num_can_numberings_or_pres numberings are orientation
   preserving while the rest are orientation reversing.

   In case of an orientation preserving numbering the edges are listed in
   ->next direction, otherwise in ->prev direction.

   Works OK if at least one vertex has valence >= 3. Otherwise some numberings
   are computed twice, since changing the orientation (the cyclic order around
   each vertex) doesn't change anything */
{
    int i, test;
    int representation[MAXE];
    EDGE *numblist[MAXE], *numblist_mirror[MAXE]; /* lists of edges where
                            starting gives a canonical representation */
    int numbs = 1, numbs_mirror = 0;
    int colour[MAXN];

    for (i=0; i<nv; i++) colour[i]=lcolour[i]+MAXN;
				/* to distinguish colours from vertices */

/* First we have to determine the smallest representation of edgelist[0] */

    testcanon_first_init(edgelist[0], representation, colour);
    numblist[0] = edgelist[0];
    test = testcanon_mirror_init(edgelist[0], representation, colour);
    if (test == 1)
      { numbs_mirror = 1; numblist_mirror[0] = edgelist[0]; }
    else if (test == 2)
      { numbs_mirror = 1; numbs = 0;
        numblist_mirror[0] = edgelist[0]; }

    if ((num_edges>1) && (edgelist[0]==edgelist[1]->invers))
      { test = testcanon_init(edgelist[1], representation, colour);
        if (test == 1) { numblist[numbs] = edgelist[1]; numbs++; }
        else if (test == 2)
          { numbs_mirror = 0; numbs = 1; numblist[0] = edgelist[1]; }
        test = testcanon_mirror_init(edgelist[1],representation, colour);
        if (test == 1)
          { numblist_mirror[numbs_mirror] = edgelist[1];
            numbs_mirror++; }
        else if (test == 2)
          { numbs_mirror = 1; numbs = 0;
            numblist_mirror[0] = edgelist[1]; }
        i=2; /* start rejecting at the second entry */
      }
    else i=1; /* start rejecting at the first entry */
/* Now we know the best representation we can obtain with testedge.
   Next we will check all the others. We can return 0 at once if we find a
   better one */

    for ( ; i < num_edges; i++)
      { test = testcanon(edgelist[i], representation, colour);
        if (test == 1) { numblist[numbs] = edgelist[i]; numbs++; }
        else if (test == 2) return 0;
        test = testcanon_mirror(edgelist[i], representation, colour);
        if (test == 1)
          { numblist_mirror[numbs_mirror] = edgelist[i]; numbs_mirror++; }
        else if (test == 2) return 0;
      }

    *num_can_numberings_or_pres = numbs;
    *num_can_numberings = numbs+numbs_mirror;

    if (*num_can_numberings > 1)
      {
        for (i = 0; i < numbs; i++)
            construct_numb(numblist[i], can_numberings[i]);
        for (i = 0; i < numbs_mirror; i++, numbs++)
            construct_numb_mirror(numblist_mirror[i],can_numberings[numbs]);
      }

    return 1;
}

/****************************************************************************/

static int
canon_edge_oriented(EDGE *edgelist_or[], int num_edges_or, int can_edges_or,
		    EDGE *edgelist_inv[], int num_edges_inv, int can_edges_inv,
		    int lcolour[], EDGE *can_numberings[][MAXE],
		    int *num_can_numberings, int *num_can_numberings_or_pres)

/*
   IT IS ASSUMED that the values of the colour function are positive
   and at most INT_MAX-MAXN.

   This routine checks all num_edges_or elements of edgelist_or just for one
   orientation and all num_edges_inv elements of the list edgelist_inv just
   for the other. It returns 1 if and only if one of the first can_edges_or
   elements of the first list or first can_edges_inv elements of the second
   give an optimal numbering among all the possibilities provided by the
   lists.

   Edges given are not in minimal form -- but it is guaranteed that all
   colours of the startpoints are the same and all colours of the endpoints
   are the same.

   In case of only the identity automorphism, the entries of can_numberings[][]
   are undefined.

   Otherwise in can_numberings[][] the canonical numberings are stored as
   sequences of oriented edges.  For every 0 <= i,j < *num_can_numberings
   and every 0 <= k < ne the edges can_numberings[i][k] and
   can_numberings[j][k] can be mapped onto each other by an automorphism.
   The first *num_can_numberings_or_pres numberings are orientation
   preserving while the rest are orientation reversing.

   In case of an orientation preserving numbering the edges are listed in
   ->next direction, otherwise in ->prev direction.

   Works OK if at least one vertex has valence >= 3. Otherwise some numberings
   are computed twice, since changing the orientation (the cyclic order around
   each vertex) doesn't change anything */
{
    int i, test;
    int representation[MAXE];
    EDGE *numblist[MAXE], *numblist_mirror[MAXE]; /* lists of edges where
                            starting gives a canonical representation */
    int numbs = 1, numbs_mirror = 0;
    int colour[MAXN];

    for (i=0; i<nv; i++) colour[i]=lcolour[i]+MAXN;
			     /* to distinguish colours from vertices */

/* First we have to determine the smallest representation possible with
   edgelist_or */

    if (can_edges_or > 0)
      { testcanon_first_init(edgelist_or[0], representation, colour);
	numblist[0] = edgelist_or[0];
	for (i=1; i<can_edges_or; i++)
	  { test = testcanon_init(edgelist_or[i], representation, colour);
	    if (test == 1) { numblist[numbs] = edgelist_or[i]; numbs++; }
	    else if (test == 2)
	      { numbs = 1; numblist[0] = edgelist_or[i]; }
	  }
	i=0; /* the next for-loop can start at the beginning */
      }
    else
      { numbs=0; numbs_mirror=1;
        testcanon_first_init_mirror(edgelist_inv[0], representation, colour);
        numblist_mirror[0] = edgelist_inv[0];
	i=1; /* the next for-loop must start at position 1 */
      }

    for (   ; i<can_edges_inv; i++)
      { test = testcanon_mirror_init(edgelist_inv[i], representation, colour);
        if (test == 1)
          { numblist_mirror[numbs_mirror] = edgelist_inv[i]; numbs_mirror++; }
	else if (test == 2)
	  { numbs = 0; numbs_mirror=1; numblist_mirror[0] = edgelist_inv[i]; }
      }


    /* now we know the best we can get for a "canonical edge".
       Next we will check all the others. We can return 0 at once if we find a
       better one */

    for (i=can_edges_or ; i < num_edges_or; i++)
      { test = testcanon(edgelist_or[i], representation, colour);
        if (test == 1) { numblist[numbs] = edgelist_or[i]; numbs++; }
        else if (test == 2) return 0;
      }
    for (i=can_edges_inv ; i < num_edges_inv; i++)
      { test = testcanon_mirror(edgelist_inv[i], representation, colour);
        if (test == 1)
          { numblist_mirror[numbs_mirror] = edgelist_inv[i]; numbs_mirror++; }
        else if (test == 2) return 0;
      }

    *num_can_numberings_or_pres = numbs;
    *num_can_numberings = numbs+numbs_mirror;

    if (*num_can_numberings > 1)
      {
        for (i = 0; i < numbs; i++)
            construct_numb(numblist[i], can_numberings[i]);
        for (i = 0; i < numbs_mirror; i++, numbs++)
            construct_numb_mirror(numblist_mirror[i],can_numberings[numbs]);
      }

    return 1;
}

/****************************************************************************/

/*
 * This method is similar to canon_edge_oriented but only performs a short scan.
 * 
 * Returns 0 if none of the first can_edges_or edges of edgelist_or is canonical
 * and none of the first can_edges_inv edges of edgelist_inv is canonical,
 * else returns 1.
 * 
 * If 1 is returned edgelist_canon_or and edgelist_canon_inv contain the edges
 * which have minimal representation. The first can_edges_canon_or and
 * can_edges_canon_inv edges in the list are the can_edges which have mimimal
 * representation. At least one of them must be > 0.
 *
 */
static int
canon_edge_oriented_short(EDGE *edgelist_or[], int num_edges_or, int can_edges_or,
		    EDGE *edgelist_inv[], int num_edges_inv, int can_edges_inv,
		    int lcolour[],
                    EDGE *edgelist_canon_or[], int *num_edges_canon_or, int *can_edges_canon_or,
                    EDGE *edgelist_canon_inv[], int *num_edges_canon_inv, int *can_edges_canon_inv)


/*
   IT IS ASSUMED that the values of the colour function are positive
   and at most INT_MAX-MAXN.

   This routine checks all num_edges_or elements of edgelist_or just for one
   orientation and all num_edges_inv elements of the list edgelist_inv just
   for the other. It returns 1 if and only if one of the first can_edges_or
   elements of the first list or first can_edges_inv elements of the second
   give an optimal numbering among all the possibilities provided by the
   lists.

   Edges given are not in minimal form -- but it is guaranteed that all
   colours of the startpoints are the same and all colours of the endpoints
   are the same.

   In case of only the identity automorphism, the entries of can_numberings[][]
   are undefined.

   Otherwise in can_numberings[][] the canonical numberings are stored as
   sequences of oriented edges.  For every 0 <= i,j < *num_can_numberings
   and every 0 <= k < ne the edges can_numberings[i][k] and
   can_numberings[j][k] can be mapped onto each other by an automorphism.
   The first *num_can_numberings_or_pres numberings are orientation
   preserving while the rest are orientation reversing.

   In case of an orientation preserving numbering the edges are listed in
   ->next direction, otherwise in ->prev direction.

   Works OK if at least one vertex has valence >= 3. Otherwise some numberings
   are computed twice, since changing the orientation (the cyclic order around
   each vertex) doesn't change anything */
{
    int i, test;
    int representation[MAXE];
    //EDGE *numblist[MAXE], *numblist_mirror[MAXE]; /* lists of edges where
    //                        starting gives a canonical representation */
    int numbs = 1, numbs_mirror = 0;
    int colour[nv];

    for (i=0; i<nv; i++) colour[i]=lcolour[i]+MAXN;
			     /* to distinguish colours from vertices */

/* First we have to determine the smallest representation possible with
   edgelist_or */

    if (can_edges_or > 0)
      { testcanon_first_init_short(edgelist_or[0], representation, colour);
	edgelist_canon_or[0] = edgelist_or[0];
	for (i=1; i<can_edges_or; i++)
	  { test = testcanon_init_short(edgelist_or[i], representation, colour);
	    if (test == 1) { edgelist_canon_or[numbs] = edgelist_or[i]; numbs++; }
	    else if (test == 2)
	      { numbs = 1; edgelist_canon_or[0] = edgelist_or[i]; }
	  }
	i=0; /* the next for-loop can start at the beginning */
      }
    else
      { numbs=0; numbs_mirror=1;
        //testcanon_first_init_mirror(edgelist_inv[0], representation, colour);
        testcanon_first_init_mirror_short(edgelist_inv[0], representation, colour);
        edgelist_canon_inv[0] = edgelist_inv[0];
	i=1; /* the next for-loop must start at position 1 */
      }

    for (   ; i<can_edges_inv; i++)
      { test = testcanon_mirror_init_short(edgelist_inv[i], representation, colour);
        if (test == 1)
          { edgelist_canon_inv[numbs_mirror] = edgelist_inv[i]; numbs_mirror++; }
	else if (test == 2)
	  { numbs = 0; numbs_mirror=1; edgelist_canon_inv[0] = edgelist_inv[i]; }
      }

    *can_edges_canon_or = numbs;
    *can_edges_canon_inv = numbs_mirror;


    /* now we know the best we can get for a "canonical edge".
       Next we will check all the others. We can return 0 at once if we find a
       better one */

    for (i=can_edges_or ; i < num_edges_or; i++)
      { test = testcanon_short(edgelist_or[i], representation, colour);
        if (test == 1) { edgelist_canon_or[numbs] = edgelist_or[i]; numbs++; }
        else if (test == 2) return 0;
      }
    for (i=can_edges_inv ; i < num_edges_inv; i++)
      { test = testcanon_mirror_short(edgelist_inv[i], representation, colour);
        if (test == 1)
          { edgelist_canon_inv[numbs_mirror] = edgelist_inv[i]; numbs_mirror++; }
        else if (test == 2) return 0;
      }

    *num_edges_canon_or = numbs;
    *num_edges_canon_inv = numbs_mirror;

/*
    *num_can_numberings_or_pres = numbs;
    *num_can_numberings = numbs+numbs_mirror;

    if (*num_can_numberings > 1)
      {
        for (i = 0; i < numbs; i++)
            construct_numb(numblist[i], can_numberings[i]);
        for (i = 0; i < numbs_mirror; i++, numbs++)
            construct_numb_mirror(numblist_mirror[i],can_numberings[numbs]);
      }
*/

    return 1;
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

        RESETMARKS;

        j = 0;
        for (i = 0; i < ne; ++i, ++nb0)
            if (!ISMARKEDLO(*nb0))
            {
                ++j;
                for (nb = nb0; nb < nblim; nb += MAXE) MARKLO(*nb);
            }
        return j;
    }
}

/****************************************************************************/

static int
numfaceorbits(int nbtot, int nbop)

/* return number of orbits of faces, under the full group
   (assumed computed).  This is supposed to work even if the
   graph is only 1-connected. */
{
    EDGE **nb0,**nblim,**nb,**nboplim;
    EDGE *e,*elast,*ee;
    int i,count;

    RESETMARKS;
    count = 0;

    if (nbtot == 1)
    {
        for (i = 0; i < nv; ++i)
        {
            e = elast = firstedge[i];
            do
            {
                if (!ISMARKEDLO(e))
                {
                    ++count;
                    ee = e;
                    do
                    {
                        MARKLO(ee);
                        ee = ee->invers->prev;
                    } while (ee != e);
                }
                e = e->next;
            } while (e != elast);
        }
    }
    else
    {
        nb0 = (EDGE**) numbering[0];
        nblim = (EDGE**) numbering[nbtot];
        nboplim = (EDGE**)numbering[nbop==0?nbtot:nbop];

        for (i = 0; i < ne; ++i) nb0[i]->index = i;

        for (i = 0; i < nv; ++i)
        {
	    e = elast = firstedge[i];
	    do
	    {
                if (!ISMARKEDLO(e))
                {
                    ++count;
		    ee = e;
		    do
		    {
                        for (nb = nb0+ee->index; nb < nboplim; nb += MAXE)
                            MARKLO(*nb);
                        for ( ; nb < nblim; nb += MAXE)
                            MARKLO((*nb)->invers);
		        ee = ee->invers->prev;
		    } while (ee != e);
                }
	        e = e->next;
	    } while (e != elast);
        }
    }

    return count;
}

/****************************************************************************/

static int
numopfaceorbits(int nbtot, int nbop)

/* return number of orbits of faces, under the orientation-preserving
   group (assumed computed).  This is supposed to work even if the
   graph is only 1-connected. */
{
    EDGE **nb0,**nb,**nboplim;
    EDGE *e,*elast,*ee;
    int i,count;

    RESETMARKS;
    count = 0;

    if (nbtot == 1)
    {
        for (i = 0; i < nv; ++i)
        {
            e = elast = firstedge[i];
            do
            {
                if (!ISMARKEDLO(e))
                {
                    ++count;
                    ee = e;
                    do
                    {
                        MARKLO(ee);
                        ee = ee->invers->prev;
                    } while (ee != e);
                }
                e = e->next;
            } while (e != elast);
        }
    }
    else
    {
        nb0 = (EDGE**) numbering[0];
        nboplim = (EDGE**)numbering[nbop==0?nbtot:nbop];

        for (i = 0; i < ne; ++i) nb0[i]->index = i;

        for (i = 0; i < nv; ++i)
        {
	    e = elast = firstedge[i];
	    do
	    {
                if (!ISMARKEDLO(e))
                {
                    ++count;
		    ee = e;
		    do
		    {
                        for (nb = nb0+ee->index; nb < nboplim; nb += MAXE)
                            MARKLO(*nb);
		        ee = ee->invers->prev;
		    } while (ee != e);
                }
	        e = e->next;
	    } while (e != elast);
        }
    }

    return count;
}

/****************************************************************************/

static int
numorbits(int nbtot, int nbop)

/* return number of orbits of vertices, under the full group
   (assumed computed). */

{
    EDGE **nb0,**nblim,**nb;
    int vindex[MAXN];
    int i,count;

    if (nbtot == 1) return nv;

    nb0 = (EDGE**) numbering[0];
    nblim = (EDGE**) numbering[nbtot];

    for (i = 0; i < ne; ++i) vindex[nb0[i]->start] = i;

    RESETMARKS_V;

    count = 0;
    for (i = 0; i < nv; ++i)
        if (!ISMARKED_V(i))
        {
            ++count;
            for (nb = nb0+vindex[i]; nb < nblim; nb += MAXE)
                MARK_V((*nb)->start);
        }

    return count;
}

/****************************************************************************/

static int
numoporbits(int nbtot, int nbop)

/* return number of orbits of vertices, under the orientation-preserving
   group (assumed computed). */

{
    EDGE **nb0,**nb,**nboplim;
    int vindex[MAXN];
    int i,count;

    if (nbtot == 1) return nv;

    nb0 = (EDGE**) numbering[0];
    nboplim = (EDGE**)numbering[nbop==0?nbtot:nbop];

    for (i = 0; i < ne; ++i) vindex[nb0[i]->start] = i;

    RESETMARKS_V;

    count = 0;
    for (i = 0; i < nv; ++i)
        if (!ISMARKED_V(i))
        {
            ++count;
            for (nb = nb0+vindex[i]; nb < nboplim; nb += MAXE)
                MARK_V((*nb)->start);
        }

    return count;
}

/****************************************************************************/

static int
numorbitsonface(int nbtot, int nbop, EDGE *e)

/* return number of orbits of directed edges in the face to
   the left of edge e, under the orientation-preserving
   automorphism group (assumed computed) */
{
    register EDGE **nb0,**nblim,**nb;
    register EDGE *e1;
    register int i,j;

    RESETMARKS;

    j = 0;
    e1 = e;
    do
    {
	MARKLO(e1);
	++j;
	e1 = e1->invers->next;
    } while (e1 != e);

    if (nbtot == 1)
        return j;
    else
    {
        nb0 = (EDGE**) numbering[0];
        if (nbop == 0) nblim = (EDGE**) numbering[nbtot];
        else           nblim = (EDGE**) numbering[nbop];

        j = 0;
        for (i = 0; i < ne; ++i, ++nb0)
            if (ISMARKEDLO(*nb0))
            {
                ++j;
                for (nb = nb0; nb < nblim; nb += MAXE) UNMARK(*nb);
            }
        return j;
    }
}

/**************************************************************************/

/*
 * Reads the graph which is given in planar code and has given codelength.
 */
static void
initialize_fuller_code(unsigned char code[], int code_length)

/* decode the given code */
{
    int i;
    EDGE *e, *elast, *ee, *eelast;
    
    FREE_EDGES;

    nv = code[0];
    int len = 1;

    if (nv == 0 || nv > MAXN)
    {
	fprintf(stderr,"Error: graph of size 0 or too big. Max nv is: %d, found: %d\n", MAXN, nv);
	exit(1);
    }

    ne = 0;
    int neighbour;
    int num_deg_5 = 0;
    for (i = 0; i < nv; ++i) {
        degree[i] = 0;
        firstedge[i] = NULL;
        while(code[len] != 0) {
            NEWEDGE(e);
            e->label = ne;
            ne++;
            degree[i]++;
            e->start = i;
            neighbour = code[len] - 1;
            e->end = neighbour;
            edge_list[i][neighbour] = e;
            e->invers = NULL;
            if(firstedge[i] == NULL)
                firstedge[i] = e;
            else {
                (e - 1)->next = e;
                e->prev = e - 1;
            }
            len++;
        }
        firstedge[i]->prev = e;
        e->next = firstedge[i];

        if(degree[i] == 5) {
            degree_5_vertices_index[i] = num_deg_5;
            degree_5_vertices[num_deg_5++] = i;
        }
        len++;
    }

    DEBUGASSERT(len == code_length);
    DEBUGASSERT(num_deg_5 == 12);


    //Set inverse edges
    for (i = 0; i < nv; ++i)
    {
	e = elast = firstedge[i];
	do
	{
	    if (e->invers == NULL)
	    {
		ee = eelast = firstedge[e->end];
		if (ee != NULL)
		    do
		    {
		        if (ee->end == i && ee->invers == NULL) break;
		        ee = ee->next;
		    } while (ee != eelast);
		if (ee != NULL && ee->end == i && ee->invers == NULL)
		{
		    e->invers = ee;
		    ee->invers = e;
/*
		    if (e < ee) e->min = ee->min = e;
		    else        e->min = ee->min = ee;
*/
		}
	    }
	    e = e->next;
	} while (e != elast);
    }

    straight_length = MAX_STRAIGHT_LENGTH + 1;
    num_bent_zero_extensions = 0;
    
    for(i = 0; i <= maxnv; i++)
        previous_rejector[i] = NULL;

}

/**************************************************************************/

/*
 * Number of faces in a patch with 1 degree 5 vertex and all degree 6 vertices
 * on distance <= distance.
 */
#define compute_nf_in_patch(distance) (1 + 5 * ((distance) + 1) * (distance) / 2)

/****************************************/

static void
initialize_fuller_arrays(void)

/* initialize for fullerenes: compute values of some arrays */ {

    if(MAX_BENT_LENGTH <= 0) {
        fprintf(stderr, "Error: MAX_BENT_LENGTH is too small: %d\n", MAX_BENT_LENGTH);
        exit(1);
    }

    int i;
    if(!fulleriprswitch) {
        int min_nv_required = 0;
        int min_nv_required_prev = 0;

        int distance = 0;
        while(min_nv_required <= maxnv) {
            distance++;

            min_nv_required = 12 * compute_nf_in_patch((distance - 1) / 2);

            /*
             * Remark: better bound should be possible for even distances.
             * Could even use experimental results.
             * 
             * Important: it is assumed by determine_max_pathlength_straight()
             * that distance is monotonically increasing and that the difference
             * in distance is at most 1.
             */
            if(distance % 2 == 0) {
                //Only helps a little!
                if(distance > 2) //If there are adjacent 5-vertices, there is an L0, L1 or B00 reduction
                    min_nv_required += 20;
            }

            if(min_nv_required_prev > 12) {
                if(max_straight_lengths[min_nv_required_prev - 1] < distance - 2) {
                    fprintf(stderr, "Error: difference in distance can be at most 1\n");
                    exit(1);
                }
            }

            for(i = min_nv_required_prev; i < min_nv_required && i <= maxnv; i++)
                max_straight_lengths[i] = distance - 1;

            //Pathlength of straight expansion is max_straight_lengths[i] + 1
            if(distance - 1 >= MAX_STRAIGHT_LENGTH) {
                fprintf(stderr, "Error: distance can be at most %d\n", MAX_STRAIGHT_LENGTH - 1);
                fprintf(stderr, "Increase MAX_STRAIGHT_LENGTH in order to continue...\n");
                exit(1);
            }

            min_nv_required_prev = min_nv_required;
        }
    } else {
        //Pathlength of straight expansion is max_straight_lengths[i] + 1
        for(i = 0; i <= maxnv; i++)
            max_straight_lengths[i] = i/3 - 1;

        if(maxnv/3 - 1 >= MAX_STRAIGHT_LENGTH) {
            fprintf(stderr, "Error: distance can be at most %d\n", MAX_STRAIGHT_LENGTH - 1);
            fprintf(stderr, "Increase MAX_STRAIGHT_LENGTH in order to continue...\n");
            exit(1);
        }        
    }

/*
    for(i = 0; i <= maxnv; i++)
        fprintf(stderr, "max_straight_lengths[%d] = %d\n", i, max_straight_lengths[i]);
*/

}

/**************************************************************************/

/**
 * Returns 1 if the fullerene contains three L1 reductions which do not have 
 * any common 5-vertices, else returns 0.
 * Important it is assumed that straight_length = 3.
 */
static int contains_three_indep_L1s() {
    DEBUGASSERT(straight_length == 3);
    if(num_straight_extensions > 3) {
        RESETMARKS_V;
        int i, j, k;
        for(i = 0; i < num_straight_extensions - 2; i++) {
            MARK_V(straight_extensions[i][0]);
            MARK_V(straight_extensions[i][2]);
            for(j = i + 1; j < num_straight_extensions - 1; j++) {
                if(!ISMARKED_V(straight_extensions[j][0]) && !ISMARKED_V(straight_extensions[j][2])) {
                    MARK_V(straight_extensions[j][0]);
                    MARK_V(straight_extensions[j][2]);
                    for(k = j + 1; k < num_straight_extensions; k++) {
                        if(!ISMARKED_V(straight_extensions[k][0]) && !ISMARKED_V(straight_extensions[k][2]))
                            return 1;
                    }
                    UNMARK_V(straight_extensions[j][0]);
                    UNMARK_V(straight_extensions[j][2]);                    
                }
            }
            UNMARK_V(straight_extensions[i][0]);
            UNMARK_V(straight_extensions[i][2]);            
        }
    }
    return 0;
}

/**************************************************************************/

/**
 * Returns 1 if the fullerene contains three L1 reductions which do not have 
 * any common 5-vertices, else returns 0.
 * Important it is assumed that straight_length = 3.
 */
static int contains_three_indep_B00s() {
    if(num_bent_zero_extensions > 3) {
        RESETMARKS_V;
        int i, j, k;
        for(i = 0; i < num_bent_zero_extensions - 2; i++) {
            MARK_V(bent_zero_extensions[i][0]);
            MARK_V(bent_zero_extensions[i][3]);
            for(j = i + 1; j < num_bent_zero_extensions - 1; j++) {
                if(!ISMARKED_V(bent_zero_extensions[j][0]) && !ISMARKED_V(bent_zero_extensions[j][3])) {
                    MARK_V(bent_zero_extensions[j][0]);
                    MARK_V(bent_zero_extensions[j][3]);
                    for(k = j + 1; k < num_bent_zero_extensions; k++) {
                        if(!ISMARKED_V(bent_zero_extensions[k][0]) && !ISMARKED_V(bent_zero_extensions[k][3]))
                            return 1;
                    }
                    UNMARK_V(bent_zero_extensions[j][0]);
                    UNMARK_V(bent_zero_extensions[j][3]);                    
                }
            }
            UNMARK_V(bent_zero_extensions[i][0]);
            UNMARK_V(bent_zero_extensions[i][3]);            
        }
    }
    return 0;
}

/**************************************************************************/

/*
 * Returns the max pathlength of a straight reduction. This is also an
 * upper bound for the max length of a bent reduction.
 */
static int
determine_max_pathlength_straight() {
    int max_pathlength = maxnv - nv;
    if(max_pathlength <= 3) {
        return max_pathlength;
    } else {
        int pathlength = 2;
        while(nv + pathlength <= maxnv) {
            if(pathlength - 1 <= max_straight_lengths[nv + pathlength]) {
                max_pathlength = pathlength;
            } else {
                //max_straight_lengths[] can increase at most 1
                break;
            }
            pathlength++;
        }

        //Both straight and bent operation add max_pathlength new vertices
        if(!startswitch) {
            if(nv + max_pathlength == maxnv - 1)
                max_pathlength--;
            //For IPR each operation adds at least 3 faces
            if(fulleriprswitch && nv + max_pathlength == maxnv - 2)
                max_pathlength--;
        }
        if(max_pathlength > 3 && !fulleriprswitch) {
            /*
             * If there is an L0 reduction, all graphs obtained from G
             * have a reduction of length at most 1 (i.e. L0, L1 or B00).
             * If there is an L2 or B10 expansion which would move the two 5-vertices,
             * there would be a 6-cut but no such 6-cuts exist (at least no
             * 6-cuts without adjacent 5-vertices). So the fullerene obtained
             * by such an L2 or B10 expansion would have an L0, L1 or B00 reduction.
             */
            if((straight_length == 2 && num_straight_extensions > 0)
                    || (straight_length == 3 && contains_three_indep_L1s())
                    || contains_three_indep_B00s()) {
                //DEBUGASSERT(!fulleriprswitch); //Since ipr fullerenes never have L0 reductions
                max_pathlength = 3;
                if(nv + max_pathlength == maxnv - 1 && !startswitch)
                    max_pathlength--;                
            }

            if(max_pathlength > 4) {
                /*
                 * If G contains reductions of length 2 (so L1 or B00), all graphs obtained from G
                 * have a reduction of length at most 3 (so L2, B10 or shorter).
                 * We're not checking the B00 reductions, since the canonical
                 * reduction is almost never a B00 reduction (aprox 1.5% of the cases for nv=152)
                 */
                //Reductions need to be "different", so mirror images of eachother are not allowed
                //That's why have to use num_straight_extensions > 2 instead of > 1
                if((straight_length == 3 && num_straight_extensions > 2)
                        || num_bent_zero_extensions > 1) { //num_bent_zero_extensions > 1 since all B00 reductions are different
                    max_pathlength = 4;
                    if(nv + max_pathlength == maxnv - 1 && !startswitch)
                        max_pathlength--;
                }
                
                /*
                 * If G contains a reduction of length 3, al lgraphs obtained
                 * from G have a reduction of length at most 5. 
                 * Remark: this optimization will only help from nv=192, since
                 * in the other cases the length of the expansions is bounded.
                 */
                if(max_pathlength > 5 && straight_length == 3 
                        && num_straight_extensions > 0) {
                    max_pathlength = 5;
                    if(nv + max_pathlength == maxnv - 1 && !startswitch)
                        max_pathlength--;                    
                }
            }
        }

        /*
         * Only allowed to go above splitlevel if splitcount == 0.
         * (ie if counter == res % mod)
         */
        /*
         * Important: Only do this if not in IPR mode, since ipr mode contains some
         * optimizations for short extensions. So the order in which extensions
         * are found might be different, so splitcount might become invalid!
         * Is ok for pathlength 3 (i.e. L1 and B00).
         */
        if(splitcount != 0 && nv < splitlevel && nv + max_pathlength > splitlevel) {
            //Remark: it is assumed that splitlevel is < maxnv - 1
            if(fulleriprswitch) {
                if(splitlevel - nv <= 3)
                    max_pathlength = splitlevel - nv;
            } else
                max_pathlength = splitlevel - nv;
        }
        //For ipr it is assumed that splitlevel is < maxnv - 2
        DEBUGASSERT(startswitch || (nv + max_pathlength != maxnv - 1
                && (!fulleriprswitch || nv + max_pathlength != maxnv - 2)));

        return max_pathlength;

    }
}

/**************************************************************************/

/*
 * Marks e for L0 in the given direction and all edges which are
 * equivalent with it.
 * Important: if numb_total > 1, it is assumed that edge->index is valid.
 */
static void
mark_edges_L0(EDGE *e, int use_next, int numb_total, int npres) {
    if(use_next)
        MARK_L0_NEXT(e->label);
    else
        MARK_L0_PREV(e->label);

    //Mark equivalent edges as well
    if(numb_total > 1) {
        int i;
        int index = e->index;
        //Start from 1, since edge with i = 0 was just marked
        for(i = 1; i < npres; i++) {
            if(use_next)
                MARK_L0_NEXT(numbering[i][index]->label);
            else
                MARK_L0_PREV(numbering[i][index]->label);
        }

        //Mark orientation reversing ones
        for(; i < numb_total; i++) {
            //Note that next and prev are swapped here!
            if(use_next)
                MARK_L0_PREV(numbering[i][index]->label);
            else
                MARK_L0_NEXT(numbering[i][index]->label);
        }
    }
}

/**************************************************************************/

/*
 * Marks e for a given pathlength and direction and all edges which are
 * equivalent with it.
 * Important: if numb_total > 1, it is assumed that edge->index is valid.
 */
static void
mark_edges_straight(EDGE *e, int pathlength, int use_next, int numb_total, int npres) {
    if(use_next)
        MARK_DOUBLE_NEXT(e->label, pathlength);
    else
        MARK_DOUBLE_PREV(e->label, pathlength);

    //Mark equivalent edges as well
    if(numb_total > 1) {
        int i;
        int index = e->index;
        //Start from 1, since edge with i = 0 was just marked
        for(i = 1; i < npres; i++) {
            if(use_next)
                MARK_DOUBLE_NEXT(numbering[i][index]->label, pathlength);
            else
                MARK_DOUBLE_PREV(numbering[i][index]->label, pathlength);
        }

        //Mark orientation reversing ones
        for(; i < numb_total; i++) {
            //Note that next and prev are swapped here!
            if(use_next)
                MARK_DOUBLE_PREV(numbering[i][index]->label, pathlength);
            else
                MARK_DOUBLE_NEXT(numbering[i][index]->label, pathlength);
        }
    }
}

/**************************************************************************/

/*
 * Marks e for B00 in the given direction and all edges which are equivalent with it.
 * Important: if numb_total > 1, it is assumed that edge->index is valid.
 */
static void
mark_edges_bent_zero(EDGE *e, int use_next, int numb_total, int npres) {
    if(use_next)
        MARK_B00_NEXT(e->label);
    else
        MARK_B00_PREV(e->label);

    //Mark equivalent edges as well
    if(numb_total > 1) {
        int i;
        int index = e->index;
        //Start from 1, since edge with i = 0 was just marked
        for(i = 1; i < npres; i++) {
            if(use_next)
                MARK_B00_NEXT(numbering[i][index]->label);
            else
                MARK_B00_PREV(numbering[i][index]->label);
        }

        //Mark orientation reversing ones
        for(; i < numb_total; i++) {
            //Note that next and prev are swapped here!
            if(use_next)
                MARK_B00_PREV(numbering[i][index]->label);
            else
                MARK_B00_NEXT(numbering[i][index]->label);
        }
    }
}

/**************************************************************************/

/*
 * Marks e for a bent expansion in the given direction and all edges which are 
 * equivalent with it.
 * Important: if numb_total > 1, it is assumed that edge->index is valid.
 */
static void
mark_edges_bent(EDGE *e, int use_next, int bent_position, int bent_length, int numb_total, int npres) {
    if(use_next)
        MARK_BENT_NEXT(e->label, bent_position, bent_length);
    else
        MARK_BENT_PREV(e->label, bent_position, bent_length);

    //Mark equivalent edges as well
    if(numb_total > 1) {
        int i;
        int index = e->index;
        //Start from 1, since edge with i = 0 was just marked
        for(i = 1; i < npres; i++) {
            if(use_next)
                MARK_BENT_NEXT(numbering[i][index]->label, bent_position, bent_length);
            else
                MARK_BENT_PREV(numbering[i][index]->label, bent_position, bent_length);
        }

        //Mark orientation reversing ones
        for(; i < numb_total; i++) {
            //Note that next and prev are swapped here!
            //Remark: not MARK_BENT_PREV(numbering[i][index]->label, bent_length - bent_position, bent_length);
            //Only need to swap next and prev
            if(use_next)
                MARK_BENT_PREV(numbering[i][index]->label, bent_position, bent_length);
            else
                MARK_BENT_NEXT(numbering[i][index]->label, bent_position, bent_length);
        }
    }
}

/****************************************************************************/

/*
 * Returns the colour of vertex edge->start.
 * This colour is a bitstring which represents the degrees of its neighbours.
 * The bit at position i is 1 if the ith neighbour of the vertex has degree 5,
 * else it has value 0. The first neighbour is edge->end and the neighbours
 * are given in clockwise order.
 *
 * Remark: colour can be at most 111111 = 63.
 */
static int
get_colour_prev(EDGE *edge) {
    EDGE *e = edge;
    int i, colour = 0;
    //Is apparently slightly finer than starting from i = 0
    for(i = degree[e->start] - 1; i >= 0; i--) {
        colour |= ((degree[e->end] == 5) << i);
        e = e->prev;
    }

    return colour;
}

/****************************************************************************/

/*
 * Returns the colour of vertex edge->start.
 * This colour is a bitstring which represents the degrees of its neighbours.
 * The bit at position i is 1 if the ith neighbour of the vertex has degree 5,
 * else it has value 0. The first neighbour is edge->end and the neighbours
 * are given in counterclockwise order.
 *
 * Remark: colour can be at most 111111 = 63.
 */
static int
get_colour_next(EDGE *edge) {
    EDGE *e = edge;
    int i, colour = 0;
    for(i = degree[e->start] - 1; i >= 0; i--) {
        colour |= ((degree[e->end] == 5) << i);
        e = e->next;
    }

    return colour;
}

/****************************************************************************/

/*
 * Same as get_colour_prev(), except here only 5 edges are checked.
 */
static int
get_colour_prev_5(EDGE *edge) {
    EDGE *e = edge;
    int i, colour = 0;
    for(i = 4; i >= 0; i--) {
        colour |= ((degree[e->end] == 5) << i);
        e = e->prev;
    }

    return colour;
}

/****************************************************************************/

/*
 * Same as get_colour_next(), except here only 5 edges are checked.
 */
static int
get_colour_next_5(EDGE *edge) {
    EDGE *e = edge;
    int i, colour = 0;
    for(i = 4; i >= 0; i--) {
        colour |= ((degree[e->end] == 5) << i);
        e = e->next;
    }

    return colour;
}

/****************************************************************************/

/*
 * Same as get_colour_prev(), except here only 3 edges are checked.
 */
static int
get_colour_prev_3(EDGE *edge) {
    EDGE *e = edge;
    int i, colour = 0;
    for(i = 2; i >= 0; i--) {
        colour |= ((degree[e->end] == 5) << i);
        e = e->prev;
    }

    return colour;
}

/****************************************************************************/

/*
 * Same as get_colour_next(), except here only 3 edges are checked.
 */
static int
get_colour_next_3(EDGE *edge) {
    EDGE *e = edge;
    int i, colour = 0;
    for(i = 2; i >= 0; i--) {
        colour |= ((degree[e->end] == 5) << i);
        e = e->next;
    }

    return colour;
}

/**************************************************************************/

//7 seems to be experimentally optimal
#define PATHLENGTH_COLOUR 7

/*
 * A bitstring which represents the sequence of degree 5 vertices of a path of
 * length PATHLENGTH_COLOUR starting from edge.
 * 1 represents a degree 5 vertex, else it has value 0.
 *
 * Remark: values for edge->start and edge->end do not appear in the bitstring,
 * since it is assumed that they were already used for a different colour.
 */
static int
get_path_colour_next(EDGE *edge) {
    EDGE *e = edge;
    int i, colour = 0;
    e = e->invers->next->next->next;
    //for(i = 0; i < PATHLENGTH_COLOUR; i++) {
    for(i = PATHLENGTH_COLOUR - 1; i >= 0; i--) {
        colour |= ((degree[e->end] == 5) << i);
        e = e->invers->next->next->next;
    }
    return colour;
}

/**************************************************************************/

/*
 * A bitstring which represents the sequence of degree 5 vertices of a path
 * of length PATHLENGTH_COLOUR starting from edge.
 * 1 represents a degree 5 vertex, else it has value 0.
 *
 * Remark: values for edge->start and edge->end do not appear in the bitstring,
 * since it is assumed that they were already used for a different colour.
 */
static int
get_path_colour_prev(EDGE *edge) {
    EDGE *e = edge;
    int i, colour = 0;
    e = e->invers->prev->prev->prev;
    //for(i = 0; i < PATHLENGTH_COLOUR; i++) {
    for(i = PATHLENGTH_COLOUR - 1; i >= 0; i--) {
        colour |= ((degree[e->end] == 5) << i);
        e = e->invers->prev->prev->prev;
    }
    return colour;
}

/**************************************************************************/

/*
 * Returns 1 if the current straight extension MIGHT destroy the canonical
 * straight reduction of the current graph. Returns 0 if it will certainly NOT
 * destroy it.
 * v1 and v2 are the vertices on path[1] and path[pathlength].
 *
 * Important: all vertices of the current expansion (main path only) are assumed to be MARKEDHI_V.
 */
static int
might_destroy_previous_straight_extension(int v1, int v2, int max_length) {
    if(straight_length <= max_length) {
        int i, j;
        int marked_parallel;
        for(i = 0; i < num_straight_extensions; i++) {
            marked_parallel = 0;
            for(j = 0; j < straight_length; j++) {
                if(ISMARKEDHI_V(straight_extensions[i][j]))
                    break;
                if(ISMARKED_V(straight_extensions[i][j]))
                    marked_parallel = 1;
            }
            if(j == straight_length && (!marked_parallel
                    || (straight_extensions[i][j] != v1 && straight_extensions[i][j] != v2
                    && straight_extensions[i][j + 1] != v1 && straight_extensions[i][j + 1] != v2))) {
                return 0;
            }
        }
    }
    return 1;

}

/**************************************************************************/

/*
 * Returns 0 if the current expansion CANNOT be canonical as it won't destroy
 * the expansions which have better colour. Returns 1 if it MIGHT be canonical.
 *
 * All vertices of the current expansion are assumed to be marked_v. And the
 * first and last vertex of the straight expansion are assumed to be MARKEDHI_V.
 */
static int
might_destroy_previous_L0_extensions() {
    int i, j;
    for(i = 0; i < num_straight_colour_2_L0_extensions; i++) {
        for(j = 0; j < STRAIGHT_COLOUR_2_L0_LENGTH_EXPANSION; j++)
            if(ISMARKED_V(straight_colour_2_L0_extensions[i][j]))
                break;
        if(j == STRAIGHT_COLOUR_2_L0_LENGTH_EXPANSION) {
            //Ok, expansion not destroyed. Now check if the colour won't be modified.
            for(; j < STRAIGHT_COLOUR_2_L0_LENGTH; j++)
                if(ISMARKEDHI_V(straight_colour_2_L0_extensions[i][j]))
                    break;
            if(j == STRAIGHT_COLOUR_2_L0_LENGTH)
                return 0;
        }
    }
    return 1;
}

/**************************************************************************/

/*
 * Returns 1 if the current extension MIGHT destroy all shorter reductions, else
 * returns 0. 
 * 
 * Important: it is assumed that this method is not called for B00 or L1.
 */
static int
might_destroy_all_straight_or_bent_zero_reductions_ipr(int max_length, int max_length_bent, int start_pentagon,
                int end_pentagon) {
    DEBUGASSERT(max_length >= 3);
    if(straight_length <= max_length) {
        int i, j;
        for(i = 0; i < num_straight_extensions; i++) {
            for(j = 0; j < straight_length + 2; j++) {
                if(ISMARKED_V(straight_extensions[i][j]))
                    break;
            }
            if(j == straight_length + 2)
                return 0;
        }
    }
    //else if(straight_length == MAX_STRAIGHT_LENGTH + 1 && num_bent_zero_extensions > 0) {
    if(num_bent_zero_extensions > 0) {
        int i;
        for(i = 0; i < num_bent_zero_extensions; i++) {
            if(bent_zero_extensions[i][0] == start_pentagon || bent_zero_extensions[i][0] == end_pentagon
                    || bent_zero_extensions[i][3] == start_pentagon || bent_zero_extensions[i][3] == end_pentagon)
                continue;
/*
            //Is in fact too rough
            if(ISMARKED_V(bent_zero_extensions[i][0]) || ISMARKED_V(bent_zero_extensions[i][3]))
                continue;
*/

            /*
             * If extension goes through bent_zero_extensions[i][2], it just 
             * turns the B00 into a L2 or B10. So if max_length <= 4, the current
             * extensions cant be canonical.
             * Remark: in fact max_length <= 3 if only using straights.
             */
            if(max_length <= 4 && ISMARKED_V(bent_zero_extensions[i][2]))
                continue;
            
            if(ISMARKED_V5(bent_zero_extensions[i][1]) || ISMARKED_V5(bent_zero_extensions[i][4]))
                continue;            

/*
            if(bent_zero_extensions[i][1] == hexagon1 || bent_zero_extensions[i][1] == hexagon2
                    || bent_zero_extensions[i][4] == hexagon1 || bent_zero_extensions[i][4] == hexagon2)
                continue;
*/

            return 0;
        }
    }
    if(num_L2_extensions > 0 && max_length >= 4) {
        int i, j;
        for(i = 0; i < num_L2_extensions; i++) {
            for(j = 0; j < L2_SIZE; j++) {
                if(ISMARKED_V(L2_extensions[i][j]))
                    break;
            }
            if(j == L2_SIZE)
                return 0;
        }
    }
    if(num_B10_extensions > 0 && max_length_bent >= 4) {
        int i, j;
        for(i = 0; i < num_B10_extensions; i++) {
            for(j = 0; j < B10_SIZE; j++) {
                if(ISMARKED_V(bent_one_zero_extensions[i][j]))
                    break;
            }
            if(j == B10_SIZE)
                return 0;
        }        
    }
    return 1;
}

/**************************************************************************/

/*
 * Searches all L0 extensions starting from startedge with direction next.
 * It is assumed that startedge->begin has degree 5.
 * Marks edges which would yield an equivalent expansion.
 */
static void
find_L0_extensions_next(EDGE *startedge, EDGE *edge_ext_straight[],
        int *num_ext_straight, int ext_use_next[], int numb_total, int npres) {
    EDGE *e = startedge;
    EDGE *other_startedge;
    int can_be_canonical;
    int temp_vertex;

    /*
     * Remark: all L0 expansions will be valid: duplicate vertices cannot occur,
     * since there are no cycles of length < 5. So no need to check ISMARKED.
     */
    e = e->invers->next->next->next;
    temp_vertex = e->invers->next->next->end;

    if(degree[temp_vertex] == 5 && !ISMARKED_L0_NEXT(startedge->label)) {
        can_be_canonical = 1;

        /* Lookahead for colour 1 and 2 */
        if(straight_length == 2 && best_straight_colour_1 > 0) {
            other_startedge = e->invers->next->next->invers->next;

            int colour_one;
            colour_one = get_colour_next_5(startedge);

            int colour_one_endedge;
            colour_one_endedge = get_colour_next_5(other_startedge);

            int best_colour, other_colour;
            if(colour_one > colour_one_endedge) {
                best_colour = colour_one;
                other_colour = colour_one_endedge;
            } else {
                best_colour = colour_one_endedge;
                other_colour = colour_one;
            }

            if(best_colour < best_straight_colour_1 ||
                    (best_colour == best_straight_colour_1 && other_colour < best_straight_colour_2)) {

                //Only mark if it is possible that the LA will reject the current extension
                e = startedge;
                RESETMARKS_V;
                MARKHI_V(e->start);
                MARK_V(e->end);
                MARK_V(e->prev->end);
                e = e->invers->next->next->next;
                MARK_V(e->prev->end);
                MARK_V(e->end);
                MARKHI_V(temp_vertex);

                //Test if it destroys previous canonical extension
                can_be_canonical = might_destroy_previous_L0_extensions();
            }
        }

        if(can_be_canonical) {
            edge_ext_straight[*num_ext_straight] = startedge;
            ext_use_next[*num_ext_straight] = 1;
            (*num_ext_straight)++;
        } 

        /*
         * If current expansion doesn't destroy the previously shortest
         * reductions, the reverse or another equivqlent expansion
         * also won't destroy them since all these expansions yield
         * the same graph.
         */

        //Mark other edge for this pathlength and direction
        //+ all other edges which are equivalent to it
        if(straight_length != 2 || best_straight_colour_1 == 0) {
            other_startedge = e->invers->next->next->invers->next;
        }
        mark_edges_L0(other_startedge, 1, numb_total, npres);
    }
}

/**************************************************************************/
/*
 * Searches all L0 extensions starting from startedge with direction prev.
 * It is assumed that startedge->begin has degree 5.
 * Marks edges which would yield an equivalent expansion.
 */
static void
find_L0_extensions_prev(EDGE *startedge, EDGE *edge_ext_straight[],
        int *num_ext_straight, int ext_use_next[], int numb_total, int npres) {
    EDGE *e = startedge;
    EDGE *other_startedge;
    int can_be_canonical;
    int temp_vertex;

    /*
     * Remark: all L0 expansions will be valid: duplicate vertices cannot occur,
     * since there are no cycles of length < 5. So no need to check ISMARKED.
     */
    e = e->invers->prev->prev->prev;
    temp_vertex = e->invers->prev->prev->end;

    if(degree[temp_vertex] == 5 && !ISMARKED_L0_PREV(startedge->label)) {
        can_be_canonical = 1;

        /* Lookahead for colour 1 and 2 */
        if(straight_length == 2 && best_straight_colour_1 > 0) {
            other_startedge = e->invers->prev->prev->invers->prev;

            int colour_one;
            colour_one = get_colour_prev_5(startedge);

            int colour_one_endedge;
            colour_one_endedge = get_colour_prev_5(other_startedge);

            int best_colour, other_colour;
            if(colour_one > colour_one_endedge) {
                best_colour = colour_one;
                other_colour = colour_one_endedge;
            } else {
                best_colour = colour_one_endedge;
                other_colour = colour_one;
            }

            if(best_colour < best_straight_colour_1 ||
                    (best_colour == best_straight_colour_1 && other_colour < best_straight_colour_2)) {

                //Only mark if it is possible that the LA will reject the current extension
                e = startedge;
                RESETMARKS_V;
                MARKHI_V(e->start);
                MARK_V(e->end);
                MARK_V(e->next->end);
                e = e->invers->prev->prev->prev;
                MARK_V(e->next->end);
                MARK_V(e->end);
                MARKHI_V(temp_vertex);

                //Test if it destroys previous canonical extension
                can_be_canonical = might_destroy_previous_L0_extensions();
            }
        }

        if(can_be_canonical) {
            edge_ext_straight[*num_ext_straight] = startedge;
            ext_use_next[*num_ext_straight] = 0;
            (*num_ext_straight)++;
        } 

        /*
         * If current expansion doesn't destroy the previously shortest
         * reductions, the reverse or another equivqlent expansion
         * also won't destroy them since all these expansions yield
         * the same graph.
         */

        //Mark other edge for this pathlength and direction
        //+ all other edges which are equivalent to it
        if(straight_length != 2 || best_straight_colour_1 == 0) {
            other_startedge = e->invers->prev->prev->invers->prev;
        }
        mark_edges_L0(other_startedge, 0, numb_total, npres);
    }
}

/**************************************************************************/

/*
 * Searches all L1 extensions starting from startedge and using direction next.
 * It is assumed that startedge->begin has degree 5.
 * Marks edges which would yield the same expansion.
 */
static void
find_L1_extensions_next(EDGE *startedge, EDGE *edge_ext_straight[],
        int *num_ext_straight, int ext_use_next[], int numb_total, int npres) {

    EDGE *e = startedge;
    int can_be_canonical;

    int temp_vertex;
    e = e->invers->next->next->next;
    e = e->invers->next->next->next;
    temp_vertex = e->invers->next->next->end;

    DEBUGASSERT(startswitch || (nv + 3 != maxnv - 1
            && (!fulleriprswitch || nv + 3 != maxnv - 2)));
    if(degree[temp_vertex] == 5 && !ISMARKED_DOUBLE_NEXT(startedge->label, 3)) {
        //Only mark when needed!
        //Mark vertices of the main path HI for might_destroy_previous_straight_extension()
        //All faces must be distinct in order to be a valid expansion
        e = startedge;
        RESETMARKS_V;
        MARKHI_V(e->start);
        MARKHI_V(e->end);
        MARK_V(e->prev->end);
        e = e->invers->next->next->next;
        
        //Otherwise extended graph is not ipr
        if(fulleriprswitch && (degree[e->end] == 5 || degree[e->prev->end] == 5))
            return;        

        MARK_V(e->prev->end);
        MARKHI_V(e->end);
        e = e->invers->next->next->next;

        MARK_V(e->prev->end);
        MARKHI_V(e->end);

        //Remark: could also compute colour if straight_length == 3
        can_be_canonical = 1;
        if(straight_length < 3) {
            DEBUGASSERT(!fulleriprswitch);

            //Test if it destroys previous canonical extension
            MARKHI_V(temp_vertex);

            /*
             * Remark: could be done efficiently. E.g. if expansion already destroys
             * all other reductions without temp_vertex, all other expansions will
             * destroy them as well. But this is not a bottleneck, so ok.
             */
            can_be_canonical = might_destroy_previous_straight_extension(startedge->end, e->end, 2);

            //UNMARK_V(temp_vertex);
        }

        if(can_be_canonical) {
            edge_ext_straight[*num_ext_straight] = startedge;
            ext_use_next[*num_ext_straight] = 1;
            (*num_ext_straight)++;
        }

        /*
         * If current expansion doesn't destroy the previously shortest
         * reductions, the reverse or another equivalent expansion
         * also won't destroy them since all these expansions yield
         * the same graph.
         */

        //Mark other edge for this pathlength and direction
        //+ all other edges which are equivalent to it
        e = e->invers->next->next->invers->next;
        DEBUGASSERT(!ISMARKED_DOUBLE_NEXT(e->label, 3));
        mark_edges_straight(e, 3, 1, numb_total, npres);
    }

}

/**************************************************************************/

/*
 * Searches all L1 extensions starting from startedge and using direction prev.
 * It is assumed that startedge->begin has degree 5.
 * Marks edges which would yield the same expansion.
 */
static void
find_L1_extensions_prev(EDGE *startedge, EDGE *edge_ext_straight[],
        int *num_ext_straight, int ext_use_next[], int numb_total, int npres) {

    EDGE *e = startedge;
    int can_be_canonical;

    int temp_vertex;
    e = e->invers->prev->prev->prev;
    e = e->invers->prev->prev->prev;
    temp_vertex = e->invers->prev->prev->end;

    DEBUGASSERT(startswitch || (nv + 3 != maxnv - 1
            && (!fulleriprswitch || nv + 3 != maxnv - 2)));
    if(degree[temp_vertex] == 5 && !ISMARKED_DOUBLE_PREV(startedge->label, 3)) {
        //Only mark when needed!
        //Mark vertices of the main path HI for might_destroy_previous_straight_extension()
        //All faces must be distinct in order to be a valid expansion
        e = startedge;
        RESETMARKS_V;
        MARKHI_V(e->start);
        MARKHI_V(e->end);
        MARK_V(e->next->end);
        e = e->invers->prev->prev->prev;
        
        //Otherwise extended graph is not ipr
        if(fulleriprswitch && (degree[e->end] == 5 || degree[e->next->end] == 5))
            return;

        MARK_V(e->next->end);
        MARKHI_V(e->end);
        e = e->invers->prev->prev->prev;

        MARK_V(e->next->end);
        MARKHI_V(e->end);

        //Remark: could also compute colour if straight_length == 3
        can_be_canonical = 1;
        if(straight_length < 3) {
            //Test if it destroys previous canonical extension
            MARKHI_V(temp_vertex);

            /*
             * Remark: could be done efficiently. E.g. if expansion already destroys
             * all other reductions without temp_vertex, all other expansions will
             * destroy them as well. But this is not a bottleneck, so ok.
             */
            can_be_canonical = might_destroy_previous_straight_extension(startedge->end, e->end, 2);

            //UNMARK_V(temp_vertex);
        }

        if(can_be_canonical) {
            edge_ext_straight[*num_ext_straight] = startedge;
            ext_use_next[*num_ext_straight] = 0;
            (*num_ext_straight)++;
        }

        /*
         * If current expansion doesn't destroy the previously shortest
         * reductions, the reverse or another equivqlent expansion
         * also won't destroy them since all these expansions yield
         * the same graph.
         */

        //Mark other edge for this pathlength and direction
        //+ all other edges which are equivalent to it
        e = e->invers->prev->prev->invers->prev;
        DEBUGASSERT(!ISMARKED_DOUBLE_PREV(e->label, 3));
        mark_edges_straight(e, 3, 0, numb_total, npres);
    }

}

/**************************************************************************/

/*
 * Searches all straight extensions starting from startedge with length at most max_pathlength
 * and direction next.
 * It is assumed that startedge->begin has degree 5.
 * Marks edges which would yield the same expansion.
 *
 * Important: it is assumed that min_pathlength is at least 4, so 
 * this method won't return any L0 or L1 extensions.
 */
static void
find_straight_extensions_next(EDGE *startedge, int min_pathlength, int max_pathlength, EDGE *edge_ext_straight[],
            int *num_ext_straight, int ext_straight_length[], int ext_use_next[],
            int numb_total, int npres) {
    EDGE *e = startedge;
    //Use different method to search L0 and L1 extensions
    DEBUGASSERT(min_pathlength > 3 && max_pathlength >= min_pathlength);
    DEBUGASSERT(startswitch || (nv + max_pathlength != maxnv - 1 && (!fulleriprswitch || nv + max_pathlength != maxnv - 2)));

    int i, j, can_be_canonical, temp_vertex;

    EDGE *temp_edge;
    for(i = 1; i <= max_pathlength; i++) {
        //This method only returns Lx extensions, x >= 2
        if(i >= min_pathlength) {
            temp_vertex = e->invers->next->next->end;
            //Otherwise extended graph is not IPR
            if(degree[temp_vertex] == 5 && (startswitch || (nv + i != maxnv - 1 && (!fulleriprswitch || nv + i != maxnv - 2)))) {
                if(!ISMARKED_DOUBLE_NEXT(startedge->label, i) && (!fulleriprswitch || degree[e->start] != 5)) {
                    //Only mark when needed!
                    //Could also do this more efficiently but will mostly be L1 anyway..
                    //Mark vertices of the main path HI for might_destroy_previous_straight_extension()
                    //All faces must be distinct in order to be a valid expansion
                    e = startedge;
                    RESETMARKS_V;
                    if(fulleriprswitch)
                        RESETMARKS_V5;  
                    for(j = 0; j < i; j++) {
                        if(!ISMARKED_V(e->start)) MARKHI_V(e->start);
                        else return;

                        if(!ISMARKED_V(e->prev->end)) MARK_V(e->prev->end);
                        else return;

                        if(fulleriprswitch && j == 1) {
                            MARK_V5(e->prev->end);
                            MARK_V5(e->end);
                        }

                        if(j < i - 1) {
                            e = e->invers->next->next->next;
                        }
                    }
                    if(!ISMARKED_V(e->end)) MARKHI_V(e->end);
                    else return;
                    if(ISMARKED_V(temp_vertex)) return;
                    
                    if(fulleriprswitch) {
                        MARK_V5(e->start);
                        MARK_V5(e->prev->prev->end);
                    }
                    
                    can_be_canonical = 1;

                    //Test if it destroys previous canonical extension
                    MARKHI_V(temp_vertex);
                    
                    if(!fulleriprswitch)
                        can_be_canonical = might_destroy_previous_straight_extension(startedge->end, e->end, i - 1);
                    else
                        can_be_canonical = might_destroy_all_straight_or_bent_zero_reductions_ipr(i - 1, i - 1, startedge->start, temp_vertex);
                    
                    if(can_be_canonical) {
                        edge_ext_straight[*num_ext_straight] = startedge;
                        ext_straight_length[*num_ext_straight] = i;
                        ext_use_next[*num_ext_straight] = 1;
                        (*num_ext_straight)++;
                    }
                        
                    /*
                     * If current expansion doesn't destroy the previously shortest
                     * reductions, the reverse or another equivqlent expansion
                     * also won't destroy them since all these expansions yield
                     * the same graph.
                     */

                    //Mark other edge for this pathlength and direction
                    //+ all other edges which are equivalent to it
                    temp_edge = e->invers->next->next->invers->next;
                    DEBUGASSERT(!ISMARKED_DOUBLE_NEXT(temp_edge->label, i));
                    mark_edges_straight(temp_edge, i, 1, numb_total, npres);
                }
            }
        }

        e = e->invers->next->next->next;
        //Otherwise extended graph is not ipr
        if(fulleriprswitch && i == 1 && degree[e->prev->end] == 5)
            return;        
    }
}

/**************************************************************************/

/*
 * Searches all straight extensions starting from startedge with length at most max_pathlength
 * and direction prev.
 * It is assumed that startedge->begin has degree 5.
 * Marks edges which would yield the same expansion.
 *
 * Important: it is assumed that min_pathlength is at least 4, so 
 * this method won't return any L0 or L1 extensions.
 */
static void
find_straight_extensions_prev(EDGE *startedge, int min_pathlength, int max_pathlength, EDGE *edge_ext_straight[],
        int *num_ext_straight, int ext_straight_length[], int ext_use_next[],
        int numb_total, int npres) {
    EDGE *e = startedge;
    //Use different method to search L0 and L1 extensions
    DEBUGASSERT(min_pathlength > 3 && max_pathlength >= min_pathlength);
    DEBUGASSERT(startswitch || (nv + max_pathlength != maxnv - 1 && (!fulleriprswitch || nv + max_pathlength != maxnv - 2)));

    int i, j, can_be_canonical, temp_vertex;

    EDGE *temp_edge;
    for(i = 1; i <= max_pathlength; i++) {
        //This method only returns Lx extensions, x >= 2
        if(i >= min_pathlength) {
            temp_vertex = e->invers->prev->prev->end;

            if(degree[temp_vertex] == 5 && (startswitch || (nv + i != maxnv - 1 && (!fulleriprswitch || nv + i != maxnv - 2)))) {
                //Otherwise extended graph is not IPR
                if(!ISMARKED_DOUBLE_PREV(startedge->label, i) && (!fulleriprswitch || degree[e->start] != 5)) {
                    //Only mark when needed!
                    //Could also do this more efficiently but will mostly be L1 anyway..
                    //Mark vertices of the main path HI for might_destroy_previous_straight_extension()
                    //All faces must be distinct in order to be a valid expansion
                    e = startedge;
                    RESETMARKS_V;
                    if(fulleriprswitch)
                        RESETMARKS_V5;                      
                    for(j = 0; j < i; j++) {
                        if(!ISMARKED_V(e->start)) MARKHI_V(e->start);
                        else return;

                        if(!ISMARKED_V(e->next->end)) MARK_V(e->next->end);
                        else return;

                        if(fulleriprswitch && j == 1) {
                            MARK_V5(e->next->end);
                            MARK_V5(e->end);
                        }                        

                        if(j < i - 1) {
                            e = e->invers->prev->prev->prev;
                        }
                    }
                    if(!ISMARKED_V(e->end)) MARKHI_V(e->end);
                    else return;
                    if(ISMARKED_V(temp_vertex)) return;
                    
                    if(fulleriprswitch) {
                        MARK_V5(e->start);
                        MARK_V5(e->next->next->end);
                    }                    

                    can_be_canonical = 1;

                    //Test if it destroys previous canonical extension
                    MARKHI_V(temp_vertex);

                    if(!fulleriprswitch)
                        can_be_canonical = might_destroy_previous_straight_extension(startedge->end, e->end, i - 1);
                    else
                        can_be_canonical = might_destroy_all_straight_or_bent_zero_reductions_ipr(i - 1, i - 1, startedge->start, temp_vertex);

                    if(can_be_canonical) {
                        edge_ext_straight[*num_ext_straight] = startedge;
                        ext_straight_length[*num_ext_straight] = i;
                        ext_use_next[*num_ext_straight] = 0;
                        (*num_ext_straight)++;
                    }

                    /*
                     * If current expansion doesn't destroy the previously shortest
                     * reductions, the reverse or another equivqlent expansion
                     * also won't destroy them since all these expansions yield
                     * the same graph.
                     */

                    //Mark other edge for this pathlength and direction
                    //+ all other edges which are equivalent to it
                    temp_edge = e->invers->prev->prev->invers->prev;
                    DEBUGASSERT(!ISMARKED_DOUBLE_PREV(temp_edge->label, i));
                    mark_edges_straight(temp_edge, i, 0, numb_total, npres);
                }
            }
        }
        e = e->invers->prev->prev->prev;
        //Otherwise extended graph is not ipr
        if(fulleriprswitch && i == 1 && degree[e->next->end] == 5)
            return;
    }

}

/**************************************************************************/

/*
 * Returns 1 if the current bent expansion MIGHT destroy all straight reductions,
 * else returns 0.
 * All vertices which are part of this B00 expansion are assumed to be marked_v.
 *
 * Important: B10 has higher priority than L3, so only check if the current bent
 * expansion MIGHT destroy all L0, L1 and L2 expansions.
 * 
 * Remark: actually not all straight reductions are tested. E.g. if
 * there is an L0, it is not tested if the bent also destroys all L1 reductions etc.
 */
static int
bent_might_destroy_all_straight_reductions(int max_length) {
    if(straight_length <= max_length) {
        int i, j;
        for(i = 0; i < num_straight_extensions; i++) {
            for(j = 0; j < straight_length + 2; j++) {
                if(ISMARKED_V(straight_extensions[i][j]))
                    break;
            }
            if(j == straight_length + 2)
                return 0;
        }
    }
    return 1;
}

/****************************************/

/*
 * Method with dummy parameters to call bent_might_destroy_all_straight_reductions().
 * Only used to make sure it would have the same signature as
 * might_destroy_all_straight_or_bent_zero_reductions_ipr().
 */
static int
bent_might_destroy_all_straight_reductions_dummy(int max_length, int max_length_bent, int a, int b) {
    return bent_might_destroy_all_straight_reductions(max_length);
}

/**************************************************************************/

/*
 * Searches all B00 extensions starting from startedge and direction next.
 * It is assumed that startedge->begin has degree 5.
 * Marks edges which would yield the same expansion.
 */
static void
find_bent_zero_extensions_next(EDGE *startedge, EDGE *ext_bent_zero[],
        int *num_ext_bent_zero, int ext_bent_zero_use_next[], int numb_total, int npres) {

    if(!ISMARKED_B00_NEXT(startedge->label)) {

        /*
         * Remark: A B00 expansion can never have duplicate vertices,
         * since the only 4-face is a diamond. So no need to check marks.
         */
        EDGE *e = startedge;
        DEBUGASSERT(degree[e->start] == 5);

        //Otherwise extended graph is not ipr
        if(fulleriprswitch && degree[e->invers->next->next->end] == 5)
            return;

        e = e->invers->next->next->next;

        e = e->invers->next->next;

        e = e->invers->next->next->next;

        if(degree[e->end] != 5) return;

        //Only mark when it's a valid B00 expansion
        RESETMARKS_V;
        e = startedge;
        MARK_V(e->start);
        MARK_V(e->end);

        e = e->invers;
        MARK_V(e->next->end);
        e = e->next->next->next;

        MARK_V(e->end);
        e = e->invers;
        MARK_V(e->next->end);
        e = e->next->next;

        MARK_V(e->end);
        e = e->invers;
        MARK_V(e->next->next->end);
        e = e->next->next->next;

        //Mark necessary for LA!
        MARK_V(e->end);

        /* Mark equivalent extensions */
        //Imporant: 1 - use next
        //mark_edges_bent_zero(e->invers, 1 - use_next, numb_total, npres);
        mark_edges_bent_zero(e->invers, 0, numb_total, npres);

        //Must destroy all L0 and L1 reductions
        if(fulleriprswitch || bent_might_destroy_all_straight_reductions(3)) {
            ext_bent_zero[*num_ext_bent_zero] = startedge;
            ext_bent_zero_use_next[*num_ext_bent_zero] = 1;
            (*num_ext_bent_zero)++;
        }
    }
}

/**************************************************************************/

/*
 * Searches all B00 extensions starting from startedge and direction prev.
 * It is assumed that startedge->begin has degree 5.
 * Marks edges which would yield the same expansion.
 */
static void
find_bent_zero_extensions_prev(EDGE *startedge, EDGE *ext_bent_zero[],
        int *num_ext_bent_zero, int ext_bent_zero_use_next[], int numb_total, int npres) {

    if(!ISMARKED_B00_PREV(startedge->label)) {

        /*
         * Remark: A B00 expansion can never have duplicate vertices,
         * since the only 4-face is a diamond. So no need to check marks.
         */
        EDGE *e = startedge;
        DEBUGASSERT(degree[e->start] == 5);
        
        //Otherwise extended graph is not ipr
        if(fulleriprswitch && degree[e->invers->prev->prev->end] == 5)
            return;        

        e = e->invers->prev->prev->prev;

        e = e->invers->prev->prev;

        e = e->invers->prev->prev->prev;

        if(degree[e->end] != 5) return;

        //Only mark when it's a valid B00 expansion
        RESETMARKS_V;
        e = startedge;
        MARK_V(e->start);
        MARK_V(e->end);

        e = e->invers;
        MARK_V(e->prev->end);
        e = e->prev->prev->prev;

        MARK_V(e->end);
        e = e->invers;
        MARK_V(e->prev->end);
        e = e->prev->prev;

        MARK_V(e->end);
        e = e->invers;
        MARK_V(e->prev->prev->end);
        e = e->prev->prev->prev;

        //Mark necessary for LA!
        MARK_V(e->end);

        /* Mark equivalent extensions */
        //Imporant: 1 - use next
        mark_edges_bent_zero(e->invers, 1, numb_total, npres);

        //Must destroy all L0 and L1 reductions
        if(fulleriprswitch || bent_might_destroy_all_straight_reductions(3)) {            
            ext_bent_zero[*num_ext_bent_zero] = startedge;
            ext_bent_zero_use_next[*num_ext_bent_zero] = 0;
            (*num_ext_bent_zero)++;
        }
    }
}

/**************************************************************************/

/*
 * Searches all B00 extensions starting from startedge and direction prev.
 * It is assumed that startedge->begin has degree 5.
 * 
 * Important: does NOT mark edges which would yield the same expansion.
 * It is assumed that this method is only called for direction prev, so
 * the endedge does not have to be marked (that one will have direction next).
 */
static void
find_bent_zero_extensions_prev_dont_mark_equivalent_expansions(EDGE *startedge, EDGE *ext_bent_zero[],
        int *num_ext_bent_zero, int ext_bent_zero_use_next[], int numb_total, int npres) {
    
    //if(!ISMARKED_B00_PREV(startedge->label)) {

        /*
         * A B00 expansion can never have overlapping vertices,
         * since the only 4-face is a diamond.
         *
         */
        EDGE *e = startedge;
        DEBUGASSERT(degree[e->start] == 5);
        
        //Otherwise extended graph is not ipr
        if(fulleriprswitch && degree[e->invers->prev->prev->end] == 5)
            return;        

        e = e->invers->prev->prev->prev;

        e = e->invers->prev->prev;

        e = e->invers->prev->prev->prev;

        if(degree[e->end] != 5) return;

        //Only mark when it's a valid B00 expansion
        RESETMARKS_V;
        e = startedge;
        MARK_V(e->start);
        MARK_V(e->end);

        e = e->invers;
        MARK_V(e->prev->end);
        e = e->prev->prev->prev;

        MARK_V(e->end);
        e = e->invers;
        MARK_V(e->prev->end);
        e = e->prev->prev;

        MARK_V(e->end);
        e = e->invers;
        MARK_V(e->prev->prev->end);
        e = e->prev->prev->prev;

        //Mark necessary for LA!
        MARK_V(e->end);

        /* Mark equivalent extensions */
        //Imporant: 1 - use next
        //mark_edges_bent_zero(e->invers, 1, numb_total, npres);

        //Must destroy all L0 and L1 reductions
        if(fulleriprswitch || bent_might_destroy_all_straight_reductions(3)) {
            ext_bent_zero[*num_ext_bent_zero] = startedge;
            ext_bent_zero_use_next[*num_ext_bent_zero] = 0;
            (*num_ext_bent_zero)++;
        }
    //}
}

/**************************************************************************/

/*
 * Adds a given bent extension to ext_bent_global and resizes ext_bent_global
 * if necessary.
 */
static void
add_bent_extension(EDGE *startedge, int bent_position, int bent_length, int use_next) {
    if(num_ext_bent_global == max_num_ext_bent_global) {
        //Array is full, so make a bigger array...
        
        DEBUGASSERT(num_ext_bent_global > 0);
        //+1 to avoid arrays of size 0
        EDGE * ext_bent[num_ext_bent_global];
        int ext_bent_position[num_ext_bent_global];
        int ext_bent_length[num_ext_bent_global];
        int ext_bent_use_next[num_ext_bent_global];
        memcpy(ext_bent, ext_bent_global, sizeof(EDGE *) * num_ext_bent_global);
        memcpy(ext_bent_position, ext_bent_position_global, sizeof(int) * num_ext_bent_global);
        memcpy(ext_bent_length, ext_bent_length_global, sizeof(int) * num_ext_bent_global);
        memcpy(ext_bent_use_next, ext_bent_use_next_global, sizeof(int) * num_ext_bent_global);
        
        free(ext_bent_global);
        free(ext_bent_position_global);
        free(ext_bent_length_global);
        free(ext_bent_use_next_global);

        //Could also expand with a certain percentage
        max_num_ext_bent_global += DEFAULT_MAX_EXT_BENT_SIZE;

        ext_bent_global = (EDGE **) malloc(sizeof(EDGE*) * max_num_ext_bent_global);
        if(ext_bent_global == NULL) {
            fprintf(stderr, "Error: can't get enough memory\n");
            exit(1);
        }
        ext_bent_position_global = (int *) malloc(sizeof(int) * max_num_ext_bent_global);
        if(ext_bent_position_global == NULL) {
            fprintf(stderr, "Error: can't get enough memory\n");
            exit(1);
        }
        ext_bent_length_global = (int *) malloc(sizeof(int) * max_num_ext_bent_global);
        if(ext_bent_length_global == NULL) {
            fprintf(stderr, "Error: can't get enough memory\n");
            exit(1);
        }
        ext_bent_use_next_global = (int *) malloc(sizeof(int) * max_num_ext_bent_global);
        if(ext_bent_use_next_global == NULL) {
            fprintf(stderr, "Error: can't get enough memory\n");
            exit(1);
        }   
        
        memcpy(ext_bent_global, ext_bent, sizeof(EDGE *) * num_ext_bent_global);
        memcpy(ext_bent_position_global, ext_bent_position, sizeof(int) * num_ext_bent_global);
        memcpy(ext_bent_length_global, ext_bent_length, sizeof(int) * num_ext_bent_global);
        memcpy(ext_bent_use_next_global, ext_bent_use_next, sizeof(int) * num_ext_bent_global);        
    }
    
    ext_bent_global[num_ext_bent_global] = startedge;
    ext_bent_use_next_global[num_ext_bent_global] = use_next;
    ext_bent_position_global[num_ext_bent_global] = bent_position;
    ext_bent_length_global[num_ext_bent_global] = bent_length;
    num_ext_bent_global++;
}

/**************************************************************************/

/*
 * Searches all bent extensions starting from startedge and direction next.
 * It is assumed that startedge->begin has degree 5.
 * Marks edges which would yield the same expansion.
 * 
 * Important: ext_bent[] is ignored in case of ipr, the expansions are stored
 * in ext_bent_global[] instead.
 */
static void
find_bent_extensions_next(EDGE *startedge, int min_pathlength, int bent_position, int max_length,
        EDGE *ext_bent[], int *num_ext_bent, int ext_bent_position[], int ext_bent_length[],
        int ext_bent_use_next[], int numb_total, int npres) {
    DEBUGASSERT(max_length > 0 && bent_position <= max_length);
    //DEBUGASSERT(bent_position > 0);
    DEBUGASSERT(nv + max_length + 3 <= maxnv);
    DEBUGASSERT(startswitch || (nv + max_length + 3 != maxnv - 1 && (!fulleriprswitch || nv + max_length + 3 != maxnv - 2)));
    DEBUGASSERT(max_length >= min_pathlength);
    
    EDGE *e = startedge;
    DEBUGASSERT(degree[e->start] == 5);

    RESETMARKS_V;
    MARK_V(e->start);
    MARK_V(e->end);
    
    if(fulleriprswitch)
        RESETMARKS_V5;

    int i;
    for(i = 0; i <= bent_position; i++) {
        e = e->invers;
        //Otherwise extended graph is not ipr
        if(fulleriprswitch && i == 0) {
            if(degree[e->next->next->end] == 5)
                return;
            MARK_V5(e->next->next->end);
            MARK_V5(e->next->next->next->end);
        }        
        
        if(ISMARKED_V(e->next->end)) return;
        else MARK_V(e->next->end);
        e = e->next->next->next;
        if(ISMARKED_V(e->end)) return;
        else MARK_V(e->end);
    }

    e = e->invers;
    //if(!ISMARKED_V(e->next->end)) MARK_V(e->next->end); else return;
    e = e->next->next;
    if(ISMARKED_V(e->end)) return;
    else MARK_V(e->end);
    
    int (*proc_might_destroy_all_straights) (int, int, int, int) = NULL;
    if(!fulleriprswitch)
        proc_might_destroy_all_straights = bent_might_destroy_all_straight_reductions_dummy;
    else
        proc_might_destroy_all_straights = might_destroy_all_straight_or_bent_zero_reductions_ipr;    

    for(i = bent_position; i <= max_length; i++) {
        e = e->invers;
        if(ISMARKED_V(e->next->end)) return;
        else MARK_V(e->next->end);
        e = e->next->next->next;
        if(ISMARKED_V(e->end)) return;
        else MARK_V(e->end);

        if(i >= min_pathlength) {
            //Bent with length i adds i + 3 new vertices...
            //if((nv + i + 3 != maxnv - 1 || startswitch) && degree[e->end] == 5
            if((startswitch || (nv + i + 3 != maxnv - 1 && (!fulleriprswitch || nv + i + 3 != maxnv - 2)))
                    && degree[e->end] == 5
                    && !ISMARKED_BENT_NEXT(startedge->label, bent_position, i)
                    && (!fulleriprswitch || degree[e->prev->prev->end] != 5)) {
                //Has to be prev!
                if(ISMARKED_V(e->prev->end)) return;
                MARK_V(e->prev->end);

                //Old
                //DEBUGASSERT(bent_position >= (i - bent_position));

                /*
                 * Mark equivalent extensions.
                 * 
                 * Old:
                 * Let a := bent_position and b := i - bent_position.
                 * So this gives us expansion Bab. We only have to do the marking
                 * in case a == b. If a > b, the expansion Bba starting from the
                 * endedge in the other direction won't be accepted, so we don't
                 * have to mark it.
                 * 
                 * New:
                 * Expansions Bab with a < b are also allowed, because in case
                 *  of IPR the search is only done in one direction.
                 *
                 * Imporant: 1 - use next.
                 */
                //if(i == 2 * bent_position)
                    mark_edges_bent(e->invers, 0, i - bent_position, i, numb_total, npres);
                //mark_edges_bent(e->invers, 1 - use_next, bent_position, numb_total, npres);

                /*
                 * Remark: could use sharper bound in case > B10, but this doesn't really
                 * matter since nearly all straight reductions are <= L2.
                 */
                int max_length_straight = i + 3;

                if(fulleriprswitch) {
                    MARK_V5(e->prev->prev->end);
                    MARK_V5(e->prev->prev->prev->end);
                }

                if((*proc_might_destroy_all_straights)(max_length_straight, max_length_straight - 1, startedge->start, e->end)) {
                    //In case of IPR ext_bent[] is ignored and ext_bent_global[] is used instead
                    if(fulleriprswitch) {
                        add_bent_extension(startedge, bent_position, i, 1);
                    } else {
                        ext_bent[*num_ext_bent] = startedge;
                        ext_bent_use_next[*num_ext_bent] = 1;
                        ext_bent_position[*num_ext_bent] = bent_position;
                        ext_bent_length[*num_ext_bent] = i;
                        (*num_ext_bent)++;
                    }
                }
                UNMARK_V(e->prev->end);
                if(fulleriprswitch) {
                    UNMARK_V5(e->prev->prev->end);
                    UNMARK_V5(e->prev->prev->prev->end);
                }                

            }
        }
    }

}

/**************************************************************************/

/*
 * Searches all bent extensions starting from startedge and direction prev.
 * It is assumed that startedge->begin has degree 5.
 * Marks edges which would yield the same expansion.
 * 
 * Important: ext_bent[] is ignored in case of ipr, the expansions are stored
 * in ext_bent_global[] instead.
 */
static void
find_bent_extensions_prev(EDGE *startedge, int min_pathlength, int bent_position, int max_length,
        EDGE *ext_bent[], int *num_ext_bent, int ext_bent_position[], int ext_bent_length[],
        int ext_bent_use_next[], int numb_total, int npres) {
    DEBUGASSERT(max_length > 0 && bent_position <= max_length);
    //DEBUGASSERT(bent_position > 0);
    DEBUGASSERT(nv + max_length + 3 <= maxnv);
    DEBUGASSERT(startswitch || (nv + max_length + 3 != maxnv - 1 && (!fulleriprswitch || nv + max_length + 3 != maxnv - 2)));
    DEBUGASSERT(max_length >= min_pathlength);

    EDGE *e = startedge;
    DEBUGASSERT(degree[e->start] == 5);

    RESETMARKS_V;
    MARK_V(e->start);
    MARK_V(e->end);
    
    if(fulleriprswitch)
        RESETMARKS_V5;

    int i;
    for(i = 0; i <= bent_position; i++) {
        e = e->invers;
        //Otherwise extended graph is not ipr
        if(fulleriprswitch && i == 0) {
            if(degree[e->prev->prev->end] == 5)
                return;
            MARK_V5(e->prev->prev->end);
            MARK_V5(e->prev->prev->prev->end);            
        }
        
        if(ISMARKED_V(e->prev->end)) return;
        else MARK_V(e->prev->end);
        e = e->prev->prev->prev;
        if(ISMARKED_V(e->end)) return;
        else MARK_V(e->end);
    }

    e = e->invers;
    //if(!ISMARKED_V(e->prev->end)) MARK_V(e->prev->end); else return;
    e = e->prev->prev;
    if(ISMARKED_V(e->end)) return;
    else MARK_V(e->end);
    
    int (*proc_might_destroy_all_straights) (int, int, int, int) = NULL;
    if(!fulleriprswitch)
        proc_might_destroy_all_straights = bent_might_destroy_all_straight_reductions_dummy;
    else
        proc_might_destroy_all_straights = might_destroy_all_straight_or_bent_zero_reductions_ipr;

    for(i = bent_position; i <= max_length; i++) {
        e = e->invers;
        if(ISMARKED_V(e->prev->end)) return;
        else MARK_V(e->prev->end);
        e = e->prev->prev->prev;
        if(ISMARKED_V(e->end)) return;
        else MARK_V(e->end);

        if(i >= min_pathlength) {
            //Bent with length i adds i + 3 new vertices...
            //if((nv + i + 3 != maxnv - 1 || startswitch) 
            if((startswitch || (nv + i + 3 != maxnv - 1 && (!fulleriprswitch || nv + i + 3 != maxnv - 2)))
                    && degree[e->end] == 5
                    && !ISMARKED_BENT_PREV(startedge->label, bent_position, i)
                    && (!fulleriprswitch || degree[e->next->next->end] != 5)) {
                if(ISMARKED_V(e->next->end)) return;
                MARK_V(e->next->end);

                //Old
                //DEBUGASSERT(bent_position >= (i - bent_position));

                /*
                 * Mark equivalent extensions.
                 * 
                 * Old:
                 * Let a := bent_position and b := i - bent_position.
                 * So this gives us expansion Bab. We only have to do the marking
                 * in case a == b. If a > b, the expansion Bba starting from the
                 * endedge in the other direction won't be accepted, so we don't
                 * have to mark it.
                 * 
                 * New:
                 * Expansions Bab with a < b are also allowed, because in case
                 *  of IPR the search is only done in one direction.
                 *
                 * Imporant: 1 - use next.
                 */
                //if(i == 2 * bent_position)
                    //mark_edges_bent(e->invers, 1, bent_position, numb_total, npres);
                mark_edges_bent(e->invers, 1, i - bent_position, i, numb_total, npres);
                //mark_edges_bent(e->invers, 1 - use_next, bent_position, numb_total, npres);

                /*
                 * Remark: could use sharper bound in case > B10, but this doesn't really
                 * matter since nearly all straight reductions are <= L2.
                 */
                int max_length_straight = i + 3;

                if(fulleriprswitch) {
                    MARK_V5(e->next->next->end);
                    MARK_V5(e->next->next->next->end);
                }

                if((*proc_might_destroy_all_straights)(max_length_straight, max_length_straight - 1, startedge->start, e->end)) {
                    //In case of IPR ext_bent[] is ignored and ext_bent_global[] is used instead
                    if(fulleriprswitch) {
                        add_bent_extension(startedge, bent_position, i, 0);
                    } else {
                        ext_bent[*num_ext_bent] = startedge;
                        ext_bent_use_next[*num_ext_bent] = 0;
                        ext_bent_position[*num_ext_bent] = bent_position;
                        ext_bent_length[*num_ext_bent] = i;
                        (*num_ext_bent)++;
                    }
                }
                UNMARK_V(e->next->end);
                
                if(fulleriprswitch) {
                    UNMARK_V5(e->next->next->end);
                    UNMARK_V5(e->next->next->next->end);
                }                

            }
        }
    }

}

/**************************************************************************/

/*
 * Searches all bent extensions starting from startedge and direction next, 
 * using the method of crossing paths.
 * It is assumed that startedge->start has degree 5.
 * Marks edges which would yield an equivalent expansion.
 */
static void
find_bent_extensions_crossing_paths_ipr_next(EDGE *startedge, int min_pathlength, int max_length,
        EDGE *ext_bent[], int *num_ext_bent, int ext_bent_position[], int ext_bent_length[],
        int ext_bent_use_next[], int numb_total, int npres) {
    DEBUGASSERT(max_length > 0);
    DEBUGASSERT(nv + max_length + 3 <= maxnv);
    DEBUGASSERT(startswitch || (nv + max_length + 3 != maxnv - 1 && (!fulleriprswitch || nv + max_length + 3 != maxnv - 2)));
    DEBUGASSERT(max_length >= min_pathlength);
    
    EDGE *e = startedge;
    DEBUGASSERT(degree[e->start] == 5);
    
    /*
     * Marks are just checked afterwards (i.e. if the path is selfinterecting).
     * This is not really a problem, since mostly the paths won't selfintersect,
     * unless if they are very long.
     */
    
    int i, label_bent;
    for(i = 0; i <= max_length; i++) {
        e = e->invers;
        if(i == 0 && degree[e->next->next->end] == 5)
            return; //Can't be ipr
        
        e = e->next->next->next;
        
        if(!ISMARKED_BENT_CROSSING_NEXT(e->label))
            MARK_BENT_CROSSING_NEXT(e->label);
        UPDATE_MIN_DISTANCE_NEXT(e->label, i + 1);
        
        label_bent = e->invers->next->next->invers->label;
        if(ISMARKED_BENT_CROSSING_PREV(label_bent)) {
            if(i + MIN_DISTANCE_PREV(label_bent) - 1 <= max_length) {
                find_bent_extensions_next(startedge, min_pathlength, i, max_length,
                        ext_bent, num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, npres);
            }
        }
    }

}

/**************************************************************************/

/*
 * Searches all bent extensions starting from startedge and direction prev,
 * using the method of crossing paths.
 * It is assumed that startedge->start has degree 5.
 * Marks edges which would yield an equivalent expansion.
 */
static void
find_bent_extensions_crossing_paths_ipr_prev(EDGE *startedge, int min_pathlength, int max_length,
        EDGE *ext_bent[], int *num_ext_bent, int ext_bent_position[], int ext_bent_length[],
        int ext_bent_use_next[], int numb_total, int npres) {
    DEBUGASSERT(max_length > 0);
    DEBUGASSERT(nv + max_length + 3 <= maxnv);
    DEBUGASSERT(startswitch || (nv + max_length + 3 != maxnv - 1 && (!fulleriprswitch || nv + max_length + 3 != maxnv - 2)));
    DEBUGASSERT(max_length >= min_pathlength);
    
    EDGE *e = startedge;
    DEBUGASSERT(degree[e->start] == 5);
    
    /*
     * Marks are just checked afterwards (i.e. if the path is selfinterecting).
     * This is not really a problem, since mostly the paths won't selfintersect,
     * unless if they are very long.
     */
    
    int i, label_bent;
    for(i = 0; i <= max_length; i++) {
        e = e->invers;
        if(i == 0 && degree[e->prev->prev->end] == 5)
            return; //Can't be ipr
        
        e = e->prev->prev->prev;
        
        if(!ISMARKED_BENT_CROSSING_PREV(e->label))
            MARK_BENT_CROSSING_PREV(e->label);
        UPDATE_MIN_DISTANCE_PREV(e->label, i + 1);
        
        label_bent = e->invers->prev->prev->invers->label;
        if(ISMARKED_BENT_CROSSING_NEXT(label_bent)) {
            if(i + MIN_DISTANCE_NEXT(label_bent) - 1 <= max_length) {
                find_bent_extensions_prev(startedge, min_pathlength, i, max_length,
                        ext_bent, num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, npres);
            }
        }
    }

}

/**************************************************************************/

/*
 * Marks all vertices in the neighbourhood of vertex.
 */
static void
mark_vertices_in_neighbourhood(int vertex) {
    int i;
    EDGE *e = firstedge[vertex];
    for(i = 0; i < degree[vertex]; i++) {
        MARK_V(e->end);
        e = e->next;
    }
}

/**************************************************************************/

/*
 * Returns 1 if vertex has a marked vertex in its neighbourhood, else returns 0.
 */
static int
has_marked_vertices_in_neighbourhood(int vertex) {
    int i;
    EDGE *e = firstedge[vertex];
    for(i = 0; i < degree[vertex]; i++) {
        if(ISMARKED_V(e->end)) return 1;
        e = e->next;
    }
    return 0;
}

/**************************************************************************/

/*
 * Marks all vertices on distance <= 2 of the given edge.
 */
static void
mark_vertices_on_distance_two(EDGE *edge) {
    mark_vertices_in_neighbourhood(edge->next->next->end);
    mark_vertices_in_neighbourhood(edge->prev->prev->end);
    edge = edge->invers;
    mark_vertices_in_neighbourhood(edge->next->next->end);
    mark_vertices_in_neighbourhood(edge->prev->prev->end);
}

/**************************************************************************/

/*
 * Returns 1 if there are marked vertices on distance <= of this edge,
 * else returns 0.
 */
static int
has_marked_vertices_on_distance_two(EDGE *edge) {
    return has_marked_vertices_in_neighbourhood(edge->next->next->end) ||
            has_marked_vertices_in_neighbourhood(edge->prev->prev->end) ||
            has_marked_vertices_in_neighbourhood(edge->invers->next->next->end) ||
            has_marked_vertices_in_neighbourhood(edge->invers->prev->prev->end);

}

/**************************************************************************/

/*
 * Returns 1 if an L1 or B00 extension can destroy all L0 extensions.
 * If there are at least 2 L0 extensions which are on distance > 4 from
 * eachother, they cannot both by destroyed by a single L1 or B00 extension.
 */
static int
L1_or_B00_can_destroy_all_L0s() {
    if(straight_length == 2 && num_straight_extensions > 1) {
        int i, j;
        for(i = 0; i < num_straight_extensions - 1; i++) {
            RESETMARKS_V;
            //Mark all vertices on distance <= 2 of the 2 pentagons of L0 reduction i
            mark_vertices_on_distance_two(edge_list[straight_extensions[i][0]][straight_extensions[i][1]]);
            for(j = i + 1; j < num_straight_extensions; j++) {
                if(!has_marked_vertices_on_distance_two(edge_list[straight_extensions[j][0]][straight_extensions[j][1]]))
                    return 0;
            }
        }
    }
    return 1;
}

/**************************************************************************/

/*
 * Marks all degree 5 vertices which are in the same cluster as vertex.
 * A cluster is a connected subgraph of the current graph G where all vertices
 * are degree 5 vertices.
 * When this method is finished cluster_size contains the size of the cluster.
 */
static void
find_degree_5_clusters(int vertex, int *cluster_size) {
    MARK_V(vertex);
    (*cluster_size)++;
    DEBUGASSERT(degree[vertex] == 5);
    int i;
    EDGE *e = firstedge[vertex];
    for(i = 0; i < 5; i++) {
        if(degree[e->end] == 5 && !ISMARKED_V(e->end)) {
            find_degree_5_clusters(e->end, cluster_size);
        }
        e = e->next;
    }
}

/**************************************************************************/

/*
 * Returns 1 if the graph contains a cluster which consists of at least 6 degree
 * 5 vertices, else returns 0.
 */
static int
contains_6_cluster() {
    RESETMARKS_V;
    int cluster_size;
    int i;
    for(i = 0; i < 12; i++) {
        if(!ISMARKED_V(degree_5_vertices[i])) {
            cluster_size = 0;
            find_degree_5_clusters(degree_5_vertices[i], &cluster_size);
            if(cluster_size >= 6)
                return 1;
        }
    }
    return 0;
}

/**************************************************************************/

static void
find_extensions_fuller(int numb_total, int numb_pres, int max_pathlength_straight,
                EDGE *ext_L0[], int *num_ext_L0, int ext_L0_use_next[],
                EDGE *ext_L1[], int *num_ext_L1, int ext_L1_use_next[],
                EDGE *ext_bent_zero[], int *num_ext_bent_zero, int ext_bent_zero_use_next[],
                EDGE *ext_straight[], int *num_ext_straight, int ext_straight_length[], int ext_straight_use_next[],
                EDGE *ext_bent[], int *num_ext_bent, int ext_bent_position[], int ext_bent_length[], int ext_bent_use_next[])

/* Find all nonequivalent places for extension.
   These are listed in ext# according to the degree of the future new vertex.
   The number of cases is returned in next# (#=3,4,5). */
{
    int i, j, k;
    EDGE *e;

    *num_ext_L0 = 0;
    *num_ext_L1 = 0;
    *num_ext_bent_zero = 0;
    *num_ext_straight = 0;
    *num_ext_bent = 0;

    /*
     * Could use separate LA for B00 (when shortest red is L1), but that would
     * only help a little expensive and find_b00_extensions doesn't cost too much.
     */
    //B00 and L1 add 3 new faces
    int can_perform_L1_and_B00 = (nv + 3 < maxnv - 1 || nv + 3 == maxnv || startswitch) && max_pathlength_straight > 2 && L1_or_B00_can_destroy_all_L0s();
    
    int can_perform_L0 = nv + 2 != maxnv - 1 || startswitch;

    //Marks for L0
    if(can_perform_L0) {
        RESETMARKS_L0_NEXT;
        RESETMARKS_L0_PREV;
    }

    //Marks for straight expansion
    if(can_perform_L1_and_B00 || max_pathlength_straight > 3) {
        RESETMARKS_DOUBLE_NEXT;
        RESETMARKS_DOUBLE_PREV;
    }

    if(can_perform_L1_and_B00) {
        //Marks for B00
        RESETMARKS_B00_NEXT;
        RESETMARKS_B00_PREV;
    }

    /*
     * Distance between 2 deg 5 vertices after applying Bij is i+j+2.
     * So i+j must be <= max_pathlength - 2 - 1
     */
    int max_length_bent = max_pathlength_straight - 3;
    if(max_pathlength_straight > 3) {
        RESETMARKS_BENT_NEXT;
        RESETMARKS_BENT_PREV;
    }
    DEBUGASSERT(startswitch || nv + max_length_bent != maxnv - 1);

     /* code for trivial group */

    if (numb_total == 1)
    {
        /* Find straight extensions */
        if(can_perform_L0) {
            for(i = 0; i < 12; i++) {
                e = firstedge[degree_5_vertices[i]];
                for(k = 0; k < 5; k++) {
                    find_L0_extensions_prev(e, ext_L0, num_ext_L0, ext_L0_use_next, numb_total, 0);
                    find_L0_extensions_next(e, ext_L0, num_ext_L0, ext_L0_use_next, numb_total, 0);

                    e = e->next;
                }
            }
        }

        /* Find Lx (x > 0) and B00 extensions */
        if(can_perform_L1_and_B00) {
            for(i = 0; i < 12; i++) {
                e = firstedge[degree_5_vertices[i]];
                for(k = 0; k < 5; k++) {
                    find_L1_extensions_prev(e, ext_L1,
                            num_ext_L1, ext_L1_use_next, numb_total, 0);
                    find_L1_extensions_next(e, ext_L1,
                            num_ext_L1, ext_L1_use_next, numb_total, 0);
                    e = e->next;
                }
            }
            
            /*
             * Each non-IPR fullerene has an L0 or L1 reduction, except
             * if it contains a special cluster of six 5-vertices (i.e.
             * figure 3a in paper CPL from Hasheminezhad).
             * Given such a fullerene, then the reduced fullerene also
             * contains a 6-cluster.
             * So if the fullerene is not IPR and doesn't contain a 6-cluster,
             * only the B00 expansions which yield an IPR fullerene might
             * be canonical.
             */
            if(straight_length == 2 && !contains_6_cluster()) {
                /*
                 * In this case the B00 can only be canonical if the expanded
                 * graph is ipr. So the L0 has to be destroyed (there is only
                 * one way to do this using a B00 expansion)
                 */
                e = firstedge[straight_extensions[0][0]];
                for(k = 0; k < 5; k++) {
                    find_bent_zero_extensions_prev(e, ext_bent_zero,
                            num_ext_bent_zero, ext_bent_zero_use_next, numb_total, 0);
                    find_bent_zero_extensions_next(e, ext_bent_zero,
                            num_ext_bent_zero, ext_bent_zero_use_next, numb_total, 0);
                    e = e->next;
                }

                //Also need to start from other one
                e = firstedge[straight_extensions[0][1]];
                for(k = 0; k < 5; k++) {
                    find_bent_zero_extensions_prev(e, ext_bent_zero,
                            num_ext_bent_zero, ext_bent_zero_use_next, numb_total, 0);
                    find_bent_zero_extensions_next(e, ext_bent_zero,
                            num_ext_bent_zero, ext_bent_zero_use_next, numb_total, 0);
                    e = e->next;
                }
            } else {
                for(i = 0; i < 12; i++) {
                    e = firstedge[degree_5_vertices[i]];
                    for(k = 0; k < 5; k++) {
                        find_bent_zero_extensions_prev(e, ext_bent_zero,
                                num_ext_bent_zero, ext_bent_zero_use_next, numb_total, 0);
                        find_bent_zero_extensions_next(e, ext_bent_zero,
                                num_ext_bent_zero, ext_bent_zero_use_next, numb_total, 0);
                        e = e->next;
                    }
                }
            }
        }
        if(max_pathlength_straight > 3) {
            //If num_straight_extensions > 2, then max_pathlength_straight == 3
            if(straight_length == 2 && (num_straight_extensions == 1 || num_straight_extensions == 2)) {
                //Only perform extensions which start at that degree 5 vertex
                //(and in fact also end at it)

                /*
                 * Here there is no need to start from straight_extensions[0][1]
                 * as well, since if there are 2 distinct L0 reductions, then the
                 * expanded graph will always have degree 5 vertices on distance <= 2,
                 * so the large straight expansion won't be accepted.
                 */
                e = firstedge[straight_extensions[0][0]];
                for(k = 0; k < 5; k++) {
                    find_straight_extensions_prev(e, 4, max_pathlength_straight, ext_straight,
                            num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                    find_straight_extensions_next(e, 4, max_pathlength_straight, ext_straight,
                            num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                    for(j = 0; j <= max_length_bent; j++) {
                        find_bent_extensions_prev(e, 1, j, max_length_bent, ext_bent,
                                num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                        find_bent_extensions_next(e, 1, j, max_length_bent, ext_bent,
                                num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                    }
                    e = e->next;
                }
            } else {
                for(i = 0; i < 12; i++) {
                    e = firstedge[degree_5_vertices[i]];
                    for(k = 0; k < 5; k++) {
                        find_straight_extensions_prev(e, 4, max_pathlength_straight, ext_straight,
                                num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                        find_straight_extensions_next(e, 4, max_pathlength_straight, ext_straight,
                                num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);                        
                        for(j = 0; j <= max_length_bent; j++) {
                            find_bent_extensions_prev(e, 1, j, max_length_bent, ext_bent,
                                    num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                            find_bent_extensions_next(e, 1, j, max_length_bent, ext_bent,
                                    num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                        }                        
                        e = e->next;
                    }
                }
            }
        }
    }

  /* code for nontrivial group */

    else
    {
        int npres = numb_pres == 0 ? numb_total : numb_pres;
        int or_same_edge_found, index;

        //index is also used in mark_edges()!
        for (i = 0; i < ne; ++i) numbering[0][i]->index = i;

        /* Find straight extensions */
        RESETMARKS;
        for(i = 0; i < 12; i++) {
            e = firstedge[degree_5_vertices[i]];
            for(k = 0; k < 5; k++) {
                if(!ISMARKEDLO(e)) {
                    index = e->index;

                    //Start from 1, no need to mark current edge since it wont be visited anymore
                    for(j = 1; j < npres; j++)
                        MARKLO(numbering[j][index]);

                    or_same_edge_found = 0;
                    for(; j < numb_total; j++) {
                        MARKLO(numbering[j][index]);
                        if(numbering[j][index] == e) {
                            or_same_edge_found = 1;
                        }
                    }

                    if(can_perform_L0) {
                        find_L0_extensions_prev(e, ext_L0, num_ext_L0, ext_L0_use_next, numb_total, npres);

                        /*
                         * If there is an OR isomorphism to itself, then only do expansion
                         * in one direction, since expansion in other direction will be in the
                         * same equivalence class.
                         */
                        if(!or_same_edge_found)
                            find_L0_extensions_next(e, ext_L0, num_ext_L0, ext_L0_use_next, numb_total, npres);
                    }

                    /* Make B00 extensions starting from edge */
                    if(can_perform_L1_and_B00) {
                        find_L1_extensions_prev(e, ext_L1,
                                num_ext_L1, ext_L1_use_next, numb_total, npres);

                        if(!or_same_edge_found)
                            find_L1_extensions_next(e, ext_L1,
                                num_ext_L1, ext_L1_use_next, numb_total, npres);

                        find_bent_zero_extensions_prev(e, ext_bent_zero, 
                                num_ext_bent_zero, ext_bent_zero_use_next, numb_total, npres);

                        if(!or_same_edge_found)
                            find_bent_zero_extensions_next(e, ext_bent_zero,
                                num_ext_bent_zero, ext_bent_zero_use_next, numb_total, npres);
                    }
                    if(max_pathlength_straight > 3) {
                        find_straight_extensions_prev(e, 4, max_pathlength_straight, ext_straight,
                                num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, npres);

                        if(!or_same_edge_found)
                            find_straight_extensions_next(e, 4, max_pathlength_straight, ext_straight,
                                num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, npres);

                        for(j = 0; j <= max_length_bent; j++)
                            find_bent_extensions_prev(e, 1, j, max_length_bent, ext_bent,
                                num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, npres);

                        if(!or_same_edge_found)
                            for(j = 0; j <= max_length_bent; j++)
                                find_bent_extensions_next(e, 1, j, max_length_bent, ext_bent,
                                    num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, npres);
                    }

                }
                e = e->next;
            }
        }
    }
}

/**************************************************************************/

/*
 * Marks all vertices with MARKS_V3 on distance <= max_distance from v1 and v2
 * by doing a BFS search.
 * The vertices on distance <= 2 from v1 and v2 are marked with MARKS_V2.
 */
static void
mark_vertices_in_neighbourhood_bfs(int v1, int v2, int max_distance) {
    DEBUGASSERT(degree[v1] == 5 && degree[v2] == 5);
    
    int distance[nv];
    int queue[nv];
    int queue_size = 0;
    queue[queue_size++] = v1;
    queue[queue_size++] = v2;
    distance[v1] = 0;
    distance[v2] = 0;
    
    RESETMARKS_V2;
    RESETMARKS_V3;
    MARK_V2(v1);
    MARK_V2(v2);
    MARK_V3(v1);
    MARK_V3(v2);    
    
    DEBUGASSERT(max_distance > 0);
    
    EDGE *e;
    int i = 0, j;
    while(i < queue_size) {
        e = firstedge[queue[i]];
        for(j = 0; j < degree[queue[i]]; j++) {
            if(!ISMARKED_V3(e->end)) {
                if(distance[queue[i]] < 2)
                    MARK_V2(e->end);
                MARK_V3(e->end);
                if(distance[queue[i]] < max_distance - 1) {
                    queue[queue_size++] = e->end;
                    distance[e->end] = distance[queue[i]] + 1;
                }
            }
            e = e->next;
        }
        i++;
    }
    
}

/**************************************************************************/

/*
 * Marks all vertices with MARKS_V4 on distance <= max_distance from v1 and v2
 * by doing a BFS search.
 */
static void
mark_vertices_in_neighbourhood_bfs_v3(int v1, int v2, int max_distance) {
    DEBUGASSERT(degree[v1] == 5 && degree[v2] == 5);
    
    int distance[nv];
    int queue[nv];
    int queue_size = 0;
    queue[queue_size++] = v1;
    queue[queue_size++] = v2;
    distance[v1] = 0;
    distance[v2] = 0;
    
    RESETMARKS_V3;
    MARK_V3(v1);
    MARK_V3(v2);    
    
    DEBUGASSERT(max_distance > 0);
    
    EDGE *e;
    int i = 0, j;
    while(i < queue_size) {
        e = firstedge[queue[i]];
        for(j = 0; j < degree[queue[i]]; j++) {
            if(!ISMARKED_V3(e->end)) {
                MARK_V3(e->end);
                if(distance[queue[i]] < max_distance - 1) {
                    queue[queue_size++] = e->end;
                    distance[e->end] = distance[queue[i]] + 1;
                }
            }
            e = e->next;
        }
        i++;
    }
    
}

/**************************************************************************/

/*
 * Marks all vertices with MARKS_V4 on distance <= max_distance from v1 and v2
 * by doing a BFS search.
 */
static void
mark_vertices_in_neighbourhood_bfs_v4(int v1, int v2, int max_distance) {
    DEBUGASSERT(degree[v1] == 5 && degree[v2] == 5);
    
    int distance[nv];
    int queue[nv];
    int queue_size = 0;
    queue[queue_size++] = v1;
    queue[queue_size++] = v2;
    distance[v1] = 0;
    distance[v2] = 0;
    
    RESETMARKS_V4;
    MARK_V4(v1);
    MARK_V4(v2);    
    
    DEBUGASSERT(max_distance > 0);
    
    EDGE *e;
    int i = 0, j;
    while(i < queue_size) {
        e = firstedge[queue[i]];
        for(j = 0; j < degree[queue[i]]; j++) {
            if(!ISMARKED_V4(e->end)) {
                MARK_V4(e->end);
                if(distance[queue[i]] < max_distance - 1) {
                    queue[queue_size++] = e->end;
                    distance[e->end] = distance[queue[i]] + 1;
                }
            }
            e = e->next;
        }
        i++;
    }
    
}

/**************************************************************************/

/*
 * Returns 1 if the L1 reduction only contains hexagons in it's "neighbourhood".
 * More specifically: an L1 reduction in clockwise and counterclockwise both
 * yield a valid reduction to a smaller IPR fullerene.
 */
static int
only_hexagons_in_neighbourhood_L1(int v1, int v2, EDGE *L1_edge, int max_distance) {
    DEBUGASSERT(degree[v1] == 5 && degree[v2] == 5);
    
    int distance[nv];
    int queue[nv];
    int queue_size = 0;
    queue[queue_size++] = v1;
    queue[queue_size++] = v2;
    distance[v1] = 0;
    distance[v2] = 0;
    
    RESETMARKS_V;
    MARK_V(v1);
    MARK_V(v2);
    
    //The following 2 vertices may also be 5-vertices. Any reduction after expansion will still be valid
    MARK_V(L1_edge->prev->invers->next->next->next->end);
    MARK_V(L1_edge->next->invers->next->next->next->end);
    
    DEBUGASSERT(max_distance > 0);
    
    EDGE *e;
    int i = 0, j;
    while(i < queue_size) {
        e = firstedge[queue[i]];
        for(j = 0; j < degree[queue[i]]; j++) {
            if(!ISMARKED_V(e->end)) {
                if(degree[e->end] == 5)
                    return 0;
                
                MARK_V(e->end);
                if(distance[queue[i]] < max_distance - 1) {
                    queue[queue_size++] = e->end;
                    distance[e->end] = distance[queue[i]] + 1;
                }
            }
            e = e->next;
        }
        i++;
    }
    
    return 1;
    
}

/**************************************************************************/

/*
 * Returns 1 if the L2 reduction only contains hexagons in it's "neighbourhood".
 * More specifically: an L1 reduction in clockwise and counterclockwise both
 * yield a valid reduction to a smaller IPR fullerene.
 */
static int
only_hexagons_in_neighbourhood_L2(int v1, int v2, EDGE *edge1, EDGE *edge2, int max_distance) {
    DEBUGASSERT(degree[v1] == 5 && degree[v2] == 5);
    
    int distance[nv];
    int queue[nv];
    int queue_size = 0;
    queue[queue_size++] = v1;
    queue[queue_size++] = v2;
    distance[v1] = 0;
    distance[v2] = 0;
    
    RESETMARKS_V;
    MARK_V(v1);
    MARK_V(v2);
    
    //The following 2 vertices may also be 5-vertices. Any reduction after expansion will still be valid
    MARK_V(edge1->prev->invers->prev->prev->end);
    MARK_V(edge1->prev->invers->prev->prev->prev->end);
    MARK_V(edge1->next->invers->next->next->end);
    MARK_V(edge1->next->invers->next->next->next->end);
    
    MARK_V(edge2->prev->invers->prev->prev->end);
    MARK_V(edge2->prev->invers->prev->prev->prev->end);
    MARK_V(edge2->next->invers->next->next->end);
    MARK_V(edge2->next->invers->next->next->next->end);    
    
    DEBUGASSERT(max_distance > 0);
    
    EDGE *e;
    int i = 0, j;
    while(i < queue_size) {
        e = firstedge[queue[i]];
        for(j = 0; j < degree[queue[i]]; j++) {
            if(!ISMARKED_V(e->end)) {
                if(degree[e->end] == 5)
                    return 0;
                
                MARK_V(e->end);
                if(distance[queue[i]] < max_distance - 1) {
                    queue[queue_size++] = e->end;
                    distance[e->end] = distance[queue[i]] + 1;
                }
            }
            e = e->next;
        }
        i++;
    }
    
    return 1;
    
}

/**************************************************************************/

/*
 * Marks the 6 vertices of the L1 at index L1_extension_index in 
 * straight_extensions[] that have to remain hexagons with MARK_V4.
 */
static void mark_vertices_of_L1(int L1_extension_index) {
    DEBUGASSERT(L1_extension_index < num_straight_extensions && straight_length == 3);
    RESETMARKS_V4;
    EDGE *e = edge_list[straight_extensions[L1_extension_index][0]][straight_extensions[L1_extension_index][1]];
    int i;
    if(e->prev->prev->end == straight_extensions[L1_extension_index][3]) {
        e = e->prev->prev->invers->prev->prev;
        for(i = 0; i < 3; i++) {
            MARK_V4(e->end);
            e = e->prev;
        }
        e = edge_list[straight_extensions[L1_extension_index][4]][straight_extensions[L1_extension_index][2]]->prev->prev;
        for(i = 0; i < 3; i++) {
            MARK_V4(e->end);
            e = e->prev;
        }
    } else { //i.e. clockwise direction
        DEBUGASSERT(e->next->next->end == straight_extensions[L1_extension_index][3]);
        e = e->next->next->invers->next->next;
        for(i = 0; i < 3; i++) {
            MARK_V4(e->end);
            e = e->next;
        }
        e = edge_list[straight_extensions[L1_extension_index][4]][straight_extensions[L1_extension_index][2]]->next->next;
        for(i = 0; i < 3; i++) {
            MARK_V4(e->end);
            e = e->next;
        }        
    }
}

/**************************************************************************/

/*
 * Important: ext_bent[] is ignored, the expansions are stored
 * in ext_bent_global[] instead.
 */
static void
find_extensions_fuller_ipr(int numb_total, int numb_pres, int max_pathlength_straight,
                EDGE *ext_L1[], int *num_ext_L1, int ext_L1_use_next[],
                EDGE *ext_bent_zero[], int *num_ext_bent_zero, int ext_bent_zero_use_next[],
                EDGE *ext_straight[], int *num_ext_straight, int ext_straight_length[], int ext_straight_use_next[],
                EDGE *ext_bent[], int *num_ext_bent, int ext_bent_position[], int ext_bent_length[], int ext_bent_use_next[])

/* Find all nonequivalent places for IPR extension.
   These are listed in ext# according to the degree of the future new vertex.
   The number of cases is returned in next# (#=3,4,5). */
{
    int i, j, k;
    EDGE *e;

    *num_ext_L1 = 0;
    *num_ext_bent_zero = 0;
    *num_ext_straight = 0;
    *num_ext_bent = 0;
    
    DEBUGASSERT(straight_length != 2);

    /*
     * Could use separate LA for B00 (when shortest red is L1), but that would
     * only help a little and find_b00_extensions doesn't cost too much.
     */
    //B00 and L1 add 3 new faces
    int can_perform_L1_and_B00 = (nv + 3 < maxnv - 2 || nv + 3 == maxnv || startswitch) && max_pathlength_straight > 2;

    //Marks for straight expansion
    if(can_perform_L1_and_B00 || max_pathlength_straight > 3) {
        RESETMARKS_DOUBLE_NEXT;
        RESETMARKS_DOUBLE_PREV;
    }

    if(can_perform_L1_and_B00) {
        //Marks for B00
        RESETMARKS_B00_NEXT;
        RESETMARKS_B00_PREV;
    }

    /*
     * Distance between 2 deg 5 vertices after applying Bij is i+j+2.
     * So i+j must be <= max_pathlength - 2 - 1
     */
    int max_length_bent = max_pathlength_straight - 3;
    if(max_pathlength_straight > 3) {
        RESETMARKS_BENT_NEXT;
        RESETMARKS_BENT_PREV;
    }
    DEBUGASSERT(startswitch || (nv + max_length_bent != maxnv - 1 && nv + max_length_bent != maxnv - 2));
    
    DEBUGASSERT(max_pathlength_straight >= 3);
    
     /* code for trivial group */

    if (numb_total == 1)
    {
        /* Find Lx (x > 0) and B00 extensions */
        if(can_perform_L1_and_B00) {
            for(i = 0; i < 12; i++) {
                e = firstedge[degree_5_vertices[i]];
                for(k = 0; k < 5; k++) {
                    find_L1_extensions_prev(e, ext_L1,
                            num_ext_L1, ext_L1_use_next, numb_total, 0);
                    find_L1_extensions_next(e, ext_L1,
                            num_ext_L1, ext_L1_use_next, numb_total, 0);

                    //Searching in one direction is sufficient, since endedge is same but in other direction
                    find_bent_zero_extensions_prev_dont_mark_equivalent_expansions(e, ext_bent_zero,
                            num_ext_bent_zero, ext_bent_zero_use_next, numb_total, 0);
/*
                    find_bent_zero_extensions_next(e, ext_bent_zero,
                            num_ext_bent_zero, ext_bent_zero_use_next, numb_total, 0);
*/
                    e = e->next;
                }
            }
        }
        if(max_pathlength_straight > 3) {
            /*
             * Try to restrict the search for expansions by limiting the number
             * startedges for the search. This is done by looking at (independent)
             * reductions which remain valid after a long expansion which just
             * crosses it.
             */
            
            /*
             * Also apply this for max_pathlength_straight == 4 (i.e. L2 or B10),
             * since for such an extension which makes an L1 longer, the 5-vertices
             * are also on distance at most 3 from the 5-vertices of the L1.
             */
            int only_start_from_neighbouring_pentagons = 0;
            int vertex1, vertex2;
            int use_small_neighbourhood = 1;
            int contains_two_independent_reductions = 0;
            int vertex1_other, vertex2_other;
            if(num_bent_zero_extensions > 0) {
                only_start_from_neighbouring_pentagons = 1;
                for(j = 0; j < num_bent_zero_extensions; j++) {
                    //When B00 is made longer, it is always still a valid L1 B10 L2
                    vertex1 = bent_zero_extensions[j][0];
                    vertex2 = bent_zero_extensions[j][3];
                    mark_vertices_in_neighbourhood_bfs(vertex1, vertex2, 3);

                    if(num_bent_zero_extensions > 1) {
                        int independent_B00;
                        for(independent_B00 = j + 1; independent_B00 < num_bent_zero_extensions; independent_B00++)
                            if(!ISMARKED_V3(bent_zero_extensions[independent_B00][0])
                                    && !ISMARKED_V3(bent_zero_extensions[independent_B00][3]))
                                break;

                        if(independent_B00 < num_bent_zero_extensions) {
                            contains_two_independent_reductions = 1;
                            vertex1_other = bent_zero_extensions[independent_B00][0];
                            vertex2_other = bent_zero_extensions[independent_B00][3];
                            break;
                        }
                    }

                    if(straight_length == 3 && num_straight_extensions > 0) {
                        int independent_L1;
                        for(independent_L1 = 0; independent_L1 < num_straight_extensions; independent_L1++)
                            if(!ISMARKED_V3(straight_extensions[independent_L1][0])
                                    && !ISMARKED_V3(straight_extensions[independent_L1][2])
                                    && only_hexagons_in_neighbourhood_L1(straight_extensions[independent_L1][0], straight_extensions[independent_L1][2],
                                    edge_list[straight_extensions[independent_L1][0]][straight_extensions[independent_L1][1]], 2))
                                break;

                        if(independent_L1 < num_straight_extensions) {
                            contains_two_independent_reductions = 1;
                            vertex1_other = straight_extensions[independent_L1][0];
                            vertex2_other = straight_extensions[independent_L1][2];
                            break;
                        }
                    }
                }
                if(max_pathlength_straight > 4) {
                    if(j == num_bent_zero_extensions)
                        j = num_bent_zero_extensions - 1;
                    RESETMARKS_V4;
                    for(i = 5; i < 11; i++) {
                        DEBUGASSERT(degree[bent_zero_extensions[j][i]] == 6)
                        MARK_V4(bent_zero_extensions[j][i]);
                    }
                }
            } 
            if(!contains_two_independent_reductions &&
                    straight_length == 3 && num_straight_extensions > 0) {
                for(j = 0; j < num_straight_extensions; j++) {
                    if(only_hexagons_in_neighbourhood_L1(straight_extensions[j][0], straight_extensions[j][2],
                            edge_list[straight_extensions[j][0]][straight_extensions[j][1]], 2)) {
                        only_start_from_neighbouring_pentagons = 1;
                        vertex1 = straight_extensions[j][0];
                        vertex2 = straight_extensions[j][2];
                        mark_vertices_in_neighbourhood_bfs(vertex1, vertex2, 3);

                        if(num_straight_extensions > 1) {
                            int independent_L1;
                            for(independent_L1 = j + 1; independent_L1 < num_straight_extensions; independent_L1++)
                                if(!ISMARKED_V3(straight_extensions[independent_L1][0])
                                        && !ISMARKED_V3(straight_extensions[independent_L1][2])
                                        && only_hexagons_in_neighbourhood_L1(straight_extensions[independent_L1][0], straight_extensions[independent_L1][2],
                                        edge_list[straight_extensions[independent_L1][0]][straight_extensions[independent_L1][1]], 2))
                                    break;

                            if(independent_L1 < num_straight_extensions) {
                                if(max_pathlength_straight > 4) {
                                    mark_vertices_of_L1(j);
                                }

                                contains_two_independent_reductions = 1;
                                vertex1_other = straight_extensions[independent_L1][0];
                                vertex2_other = straight_extensions[independent_L1][2];
                                
                                break;
                            } else if(j == num_straight_extensions - 1) {
                                if(max_pathlength_straight > 4)
                                    mark_vertices_of_L1(j);
                            }
                        } else {
                            if(max_pathlength_straight > 4)
                                mark_vertices_of_L1(j);
                            break;
                        }
                    }
                }
                if(!only_start_from_neighbouring_pentagons && max_pathlength_straight <= 6) {
                    only_start_from_neighbouring_pentagons = 1;
                    use_small_neighbourhood = 0;
                    vertex1 = straight_extensions[0][0];
                    vertex2 = straight_extensions[0][2];
                    mark_vertices_in_neighbourhood_bfs(vertex1, vertex2, 3);
                }
            }

            if(contains_two_independent_reductions) {
                /*
                 * There are two independent B00 or L1 reductions, which will 
                 * stay valid after expansions which just cross it. 
                 * So only expansions which start in the neighbourhood of one
                 * reduction and which end in the neighbourhood of the other
                 * reduction are accepted. This is done using the method of
                 * crossing paths.
                 */                
                RESETMARKS_BENT_CROSSING_NEXT;
                RESETMARKS_BENT_CROSSING_PREV;
                RESETMARKS_L0_NEXT;
                //Has to search all extensions from vertex1 and vertex2
                for(i = 0; i < 2; i++) {
                    e = firstedge[i == 0 ? vertex1 : vertex2];
                    for(k = 0; k < 5; k++) {
                        find_straight_extensions_prev(e, 4, max_pathlength_straight, ext_straight,
                                num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                        find_straight_extensions_next(e, 4, max_pathlength_straight, ext_straight,
                                num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                        find_bent_extensions_crossing_paths_ipr_prev(e, 1, max_length_bent, ext_bent,
                                num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                        find_bent_extensions_crossing_paths_ipr_next(e, 1, max_length_bent, ext_bent,
                                num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);                        

                        e = e->next;
                    }
                }
                
                for(i = 0; i < 12; i++) {
                    if(ISMARKED_V3(degree_5_vertices[i]) && degree_5_vertices[i] != vertex1 &&
                            degree_5_vertices[i] != vertex2) {
                        e = firstedge[degree_5_vertices[i]];
                        for(k = 0; k < 5; k++) {
                            if(ISMARKED_V2(e->end)) {
                                MARK_L0_NEXT(e->label);
                                if(max_pathlength_straight > 4 && ISMARKED_V4(e->end)) {
                                    //Cannot happen because no such 5-cycles exist
                                    DEBUGASSERT(e->start != vertex1 && e->start != vertex2);
                                    find_straight_extensions_prev(e, 4, max_pathlength_straight, ext_straight,
                                            num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                                    find_straight_extensions_next(e, 4, max_pathlength_straight, ext_straight,
                                            num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                                    find_bent_extensions_crossing_paths_ipr_prev(e, 1, max_length_bent, ext_bent,
                                            num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                                    find_bent_extensions_crossing_paths_ipr_next(e, 1, max_length_bent, ext_bent,
                                            num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                                } else if(startswitch || (nv + 4 != maxnv - 1 && nv + 4 != maxnv - 2)) {
                                    find_straight_extensions_prev(e, 4, 4, ext_straight,
                                            num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                                    find_straight_extensions_next(e, 4, 4, ext_straight,
                                            num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                                    find_bent_extensions_crossing_paths_ipr_prev(e, 1, 1, ext_bent,
                                            num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                                    find_bent_extensions_crossing_paths_ipr_next(e, 1, 1, ext_bent,
                                            num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                                }
                            }
                            e = e->next;
                        }
                    }
                }

                //Using marked_V4 here doesn't really help
                mark_vertices_in_neighbourhood_bfs(vertex1_other, vertex2_other, 3);
                for(i = 0; i < 12; i++) {
                    if(ISMARKED_V3(degree_5_vertices[i])) {
                        e = firstedge[degree_5_vertices[i]];
                        for(k = 0; k < 5; k++) {
                            if(ISMARKED_V2(e->end) && !ISMARKED_L0_NEXT(e->label)) {
                                find_bent_extensions_crossing_paths_ipr_prev(e, 1, max_length_bent, ext_bent,
                                        num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                                find_bent_extensions_crossing_paths_ipr_next(e, 1, max_length_bent, ext_bent,
                                        num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                            }
                            e = e->next;
                        }
                    }
                }

            } else {
                /*
                 * There is one B00 or L1 reduction, which will stay a valid
                 * reduction after expansions which just cross it. So only
                 * start from 5-vertices which are in the neighbourhood of
                 * this reduction. So can't use method of crossing paths here.
                 */
                if(only_start_from_neighbouring_pentagons) {
                    if(use_small_neighbourhood) {
                        //Has to search all extensions from vertex1 and vertex2
                        for(i = 0; i < 2; i++) {
                            e = firstedge[i == 0 ? vertex1 : vertex2];
                            for(k = 0; k < 5; k++) {
                                find_straight_extensions_prev(e, 4, max_pathlength_straight, ext_straight,
                                        num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                                find_straight_extensions_next(e, 4, max_pathlength_straight, ext_straight,
                                        num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);

                                for(j = 0; j <= max_length_bent; j++) {
                                    find_bent_extensions_prev(e, 1, j, max_length_bent, ext_bent,
                                            num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                                    find_bent_extensions_next(e, 1, j, max_length_bent, ext_bent,
                                            num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                                }

                                e = e->next;
                            }
                        }
                        /*
                         * For other pentagons: treat short and long extensions separately.
                         * Since if short extensions make shortest one longer, they may
                         * be canonical, while long extensions won't be canonical in
                         * that case.
                         */
                        for(i = 0; i < 12; i++) {
                            if(ISMARKED_V3(degree_5_vertices[i]) && degree_5_vertices[i] != vertex1 &&
                                    degree_5_vertices[i] != vertex2) {
                                e = firstedge[degree_5_vertices[i]];
                                for(k = 0; k < 5; k++) {
                                    if(ISMARKED_V2(e->end)) {
                                        if(max_pathlength_straight > 4 && ISMARKED_V4(e->end)) {
                                            //Cannot happen because no such 5-cycles exist
                                            DEBUGASSERT(e->start != vertex1 && e->start != vertex2);
                                            find_straight_extensions_prev(e, 4, max_pathlength_straight, ext_straight,
                                                    num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                                            find_straight_extensions_next(e, 4, max_pathlength_straight, ext_straight,
                                                    num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                                            for(j = 0; j <= max_length_bent; j++) {
                                                find_bent_extensions_prev(e, 1, j, max_length_bent, ext_bent,
                                                        num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                                                find_bent_extensions_next(e, 1, j, max_length_bent, ext_bent,
                                                        num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                                            }
                                        } else if(startswitch || (nv + 4 != maxnv - 1 && nv + 4 != maxnv - 2)) {
                                            find_straight_extensions_prev(e, 4, 4, ext_straight,
                                                    num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                                            find_straight_extensions_next(e, 4, 4, ext_straight,
                                                    num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                                            for(j = 0; j <= 1; j++) {
                                                find_bent_extensions_prev(e, 1, j, 1, ext_bent,
                                                        num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                                                find_bent_extensions_next(e, 1, j, 1, ext_bent,
                                                        num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                                            }
                                        }
                                    }
                                    e = e->next;
                                }
                            }
                        }
                    } else {
                        for(i = 0; i < 12; i++) {
                            if(ISMARKED_V3(degree_5_vertices[i])) {
                                e = firstedge[degree_5_vertices[i]];
                                for(k = 0; k < 5; k++) {
                                    if(ISMARKED_V2(e->end)) {
                                        find_straight_extensions_prev(e, 4, max_pathlength_straight, ext_straight,
                                                num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                                        find_straight_extensions_next(e, 4, max_pathlength_straight, ext_straight,
                                                num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                                        for(j = 0; j <= max_length_bent; j++) {
                                            find_bent_extensions_prev(e, 1, j, max_length_bent, ext_bent,
                                                    num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                                            find_bent_extensions_next(e, 1, j, max_length_bent, ext_bent,
                                                    num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                                        }
                                    }
                                    e = e->next;
                                }
                            }
                        }
                    }
                } else {
                    //In this case one starts the search from all 5-vertices
                    RESETMARKS_BENT_CROSSING_NEXT;
                    RESETMARKS_BENT_CROSSING_PREV;
                    for(i = 0; i < 12; i++) {
                        e = firstedge[degree_5_vertices[i]];
                        for(k = 0; k < 5; k++) {
                            find_straight_extensions_prev(e, 4, max_pathlength_straight, ext_straight,
                                    num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);
                            find_straight_extensions_next(e, 4, max_pathlength_straight, ext_straight,
                                    num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, 0);

                            find_bent_extensions_crossing_paths_ipr_prev(e, 1, max_length_bent, ext_bent,
                                    num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);
                            find_bent_extensions_crossing_paths_ipr_next(e, 1, max_length_bent, ext_bent,
                                    num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, 0);

                            e = e->next;
                        }
                    }
                }
            }

        }
    }

  /* code for nontrivial group */

    else
    {
        
        int npres = numb_pres == 0 ? numb_total : numb_pres;
        int or_same_edge_found, index;

        //index is also used in mark_edges()!
        for (i = 0; i < ne; ++i) numbering[0][i]->index = i;

        /* Find straight extensions */
        RESETMARKS;
        for(i = 0; i < 12; i++) {
            e = firstedge[degree_5_vertices[i]];
            for(k = 0; k < 5; k++) {
                if(!ISMARKEDLO(e)) {
                    index = e->index;

                    //Start from 1, no need to mark current edge since it wont be visited anymore
                    for(j = 1; j < npres; j++)
                        MARKLO(numbering[j][index]);

                    or_same_edge_found = 0;
                    for(; j < numb_total; j++) {
                        MARKLO(numbering[j][index]);
                        if(numbering[j][index] == e) {
                            or_same_edge_found = 1;
                        }
                    }

                    /* Make B00 extensions starting from edge */
                    if(can_perform_L1_and_B00) {
                        find_L1_extensions_prev(e, ext_L1,
                                num_ext_L1, ext_L1_use_next, numb_total, npres);

                        if(!or_same_edge_found)
                            find_L1_extensions_next(e, ext_L1,
                                num_ext_L1, ext_L1_use_next, numb_total, npres);

                        find_bent_zero_extensions_prev(e, ext_bent_zero, 
                                num_ext_bent_zero, ext_bent_zero_use_next, numb_total, npres);

                        if(!or_same_edge_found)
                            find_bent_zero_extensions_next(e, ext_bent_zero,
                                num_ext_bent_zero, ext_bent_zero_use_next, numb_total, npres);
                    }
                    if(max_pathlength_straight > 3) {
                        find_straight_extensions_prev(e, 4, max_pathlength_straight, ext_straight,
                                num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, npres);

                        if(!or_same_edge_found)
                            find_straight_extensions_next(e, 4, max_pathlength_straight, ext_straight,
                                num_ext_straight, ext_straight_length, ext_straight_use_next, numb_total, npres);

                        for(j = 0; j <= max_length_bent; j++)
                            find_bent_extensions_prev(e, 1, j, max_length_bent, ext_bent,
                                num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, npres);

                        if(!or_same_edge_found)
                            for(j = 0; j <= max_length_bent; j++)
                                find_bent_extensions_next(e, 1, j, max_length_bent, ext_bent,
                                    num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next, numb_total, npres);
                    }

                }
                e = e->next;
            }
        }
    }
}

/**************************************************************************/

static int
make_dual(void)

/* Store in the rightface field of each edge the number of the face on
   the right hand side of that edge.  Faces are numbered 0,1,....  Also
   store in facestart[i] an example of an edge in the clockwise orientation
   of the face boundary, and the size of the face in facesize[i], for each i.
   Returns the number of faces. */
{
    register int i,nf,sz;
    register EDGE *e,*ex,*ef,*efx;

    RESETMARKS;

    nf = 0;
    for (i = 0; i < nv; ++i)
    {
        e = ex = firstedge[i];
        do
        {
            if (!ISMARKEDLO(e))
            {
                facestart[nf] = ef = efx = e;
                sz = 0;
                do
                {
                    ef->rightface = nf;
                    MARKLO(ef);
                    ef = ef->invers->prev;
                    ++sz;
                } while (ef != efx);
                facesize[nf] = sz;
                ++nf;
            }
            e = e->next;
        } while (e != ex);
    }

    return nf;
}

/**************************************************************************/

static void
compute_code(unsigned char code[])

/* computes a code by numbering the vertices in a breadth first manner and
   then listing them in code. Code is a sequence of numbers of length ne+nv+1.
   The first entry is the number of vertices.
   Then the numbers of the vertices adjacent to the vertex numbered 1 are
   given -- ended by a 0, and listed in clockwise orientation.
   Then those adjacent to 2, ended by a 0, etc. . In case of no
   double edges, the identification of the corresponding "half edges" leaving
   each vertex is unique. Nevertheless also for this case the following rules
   apply (not in the definition of the code, but in this routine):

   In case of double edges, the first time a new vertex
   b occurs in the list, say it is in the list of vertex a, must be matched
   with the first occurence of vertex a in the list of b. In this routine
   it will always be the first position in the list of b.

   This spanning tree
   gives a unique matching for the other "half edges" -- provided the fact
   that the ordering comes from an embedding on the sphere.

   In case of a given starting edge in code_edge, the start of this
   edge is numbered 1 and the end 2.
*/
{
    register EDGE *run;
    register int vertex;
    EDGE *temp;
    EDGE *startedge[MAXN+1];
    int number[MAXN+1], i;
    int last_number, actual_number;
    EDGE *givenedge;

    for (i = 0; i < nv; i++) number[i] = 0;

    *code=nv; code++;
    if (code_edge==NULL) givenedge=firstedge[0];
    else { givenedge=code_edge; number[nv]=0; }
    number[givenedge->start] = 1;
    if (givenedge->start != givenedge->end)
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else  last_number = 1;

    actual_number = 1;
    temp = givenedge;

    while (last_number < nv)
    {   *code=number[temp->end]; code++;
        for (run = temp->next; run != temp; run = run->next)
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                *code = last_number; }
            else *code = number[vertex];
            code++;
          }
        *code = 0;  code++;
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nv)
    {  	*code=number[temp->end]; code++;
        for (run = temp->next; run != temp; run = run->next)
          {
            *code = number[run->end]; code++;
          }
        *code = 0;
        code++;
        temp = startedge[actual_number];  actual_number++;
    }
}

/****************************************************************************/

static void
compute_code_mirror(unsigned char code[])

/* In the case of no double edges -- that is when the identifications are
   clear -- there is no problem in just returning the inverse order of
   the previously computed code. Nevertheless in the case where edge
   identifications must be made via the spanning tree described in the code,
   this is not that easy, so we can as well just compute mirror code just
   like the normal code computed before.

   In case code_edge is not NULL, its start is numbered 1 and its
   end is numbered 0.
*/
{
    register EDGE *run;
    register int vertex;
    EDGE *temp;
    EDGE *startedge[MAXN+1];
    int number[MAXN+1], i;
    int last_number, actual_number;
    EDGE *givenedge;

    for (i = 0; i < nv; i++) number[i] = 0;

    *code=nv; code++;
    if (code_edge==NULL) givenedge=firstedge[0];
    else { givenedge=code_edge->invers; number[nv]=0; }
    number[givenedge->start] = 1;
    if (givenedge->start != givenedge->end)
      {
	number[givenedge->end] = 2;
	last_number = 2;
	startedge[1] = givenedge->invers;
      }
    else  last_number = 1;

    actual_number = 1;
    temp = givenedge;

    while (last_number < nv)
    {   *code=number[temp->end]; code++;
        for (run = temp->prev; run != temp; run = run->prev)
          { vertex = run->end;
            if (!number[vertex])
              { startedge[last_number] = run->invers;
                last_number++; number[vertex] = last_number;
                *code = last_number; }
            else *code = number[vertex];
            code++;
          }
        *code = 0;  code++;
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nv)
    {  	*code=number[temp->end]; code++;
        for (run = temp->prev; run != temp; run = run->prev)
          {
            *code = number[run->end]; code++;
          }
        *code = 0;
        code++;
        temp = startedge[actual_number];  actual_number++;
    }
}

/****************************************************************************/

static void
compute_dual_code(unsigned char code[])

/* works like compute_code -- only for the dual */

{
    register EDGE *run, *run2;
    EDGE *temp;
    EDGE *startedge[MAXF+1];
    int number[MAXF+1], i;
    int last_number, actual_number;
    int nf;
    EDGE *givenedge;

    nf=2+(ne/2)-nv;
    *code=nf; code++;
    RESETMARKS; /* The face on the right has already been numbered if
		   and only if it is marked. */
    for (i = 0; i < nf; i++) number[i] = 0;

    if (code_edge==NULL) givenedge=firstedge[0];
    else givenedge=code_edge->invers;

    run=givenedge;
    do { MARKLO(run); run->rightface=1; run=run->invers->prev;
       } while (run!=givenedge);
    if (!ISMARKED(givenedge->invers)) /* no loop in the dual at this point */
      { run2=run=givenedge->invers;
        do { MARKLO(run2); run2->rightface=2; run2=run2->invers->prev;
           } while (run2!=run);
	last_number = 2;
	startedge[1] = givenedge; /* the startedge has the face it belongs
				     to on the LEFT */
      }
    else  last_number = 1;

    actual_number = 1;
    temp = givenedge->invers;
    while (last_number < nf)
    {
        *code= temp->rightface; code++;
        for (run = temp->prev->invers; run != temp; run = run->prev->invers)
	  /* run also has the face it runs around on the left */
          { if (!ISMARKED(run))
	    { startedge[last_number] = run->invers;
              last_number++;
	      run2=run;
	      do
		{ MARKLO(run2);
		  run2->rightface=last_number;
		  run2=run2->invers->prev;
                } while (run2!=run);
	      *code = last_number;
	    }
	    else *code=run->rightface;
	    code++;
	  }
        *code = 0;  code++;
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nf)
    {  	*code=temp->rightface; code++;
        for (run = temp->prev->invers; run != temp; run = run->prev->invers)
          {
            *code = run->rightface; code++;
          }
        *code = 0;
        code++;
        temp = startedge[actual_number];  actual_number++;
    }
}

/****************************************************************************/

static void
compute_dual_code_mirror(unsigned char code[])

/* works like compute_code_mirror -- only for the dual */

{
    register EDGE *run, *run2;
    EDGE *temp;
    EDGE *startedge[MAXF+1];
    int number[MAXF+1], i;
    int last_number, actual_number;
    int nf;
    EDGE *givenedge;

    nf=2+(ne/2)-nv;
    *code=nf; code++;
    RESETMARKS; /* The face on the right has already been numbered
                   if and only if it is marked. */
    for (i = 0; i < nf; i++) number[i] = 0;

    if (code_edge==NULL) givenedge=firstedge[0];
    else givenedge=code_edge->invers;

    run=givenedge;
    do { MARKLO(run); run->rightface=1; run=run->invers->prev;
       } while (run!=givenedge);
    if (!ISMARKED(givenedge->invers)) /* no loop in the dual at this point */
      { run2=run=givenedge->invers;
        do { MARKLO(run2); run2->rightface=2; run2=run2->invers->prev;
           } while (run2!=run);
	last_number = 2;
	startedge[1] = givenedge; /* the startedge has the face it
				     belongs to on the LEFT */
      }
    else  last_number = 1;

    actual_number = 1;
    temp = givenedge->invers;

    while (last_number < nf)
    {
        *code= temp->rightface; code++;
        for (run = temp->invers->next; run != temp; run = run->invers->next)
	  /* run also has the face it runs around on the left */
          { if (!ISMARKED(run))
	    { startedge[last_number] = run->invers;
              last_number++;
	      run2=run;
	      do
		{ MARKLO(run2); run2->rightface=last_number;
		  run2=run2->invers->prev;
                } while (run2!=run);
	      *code = last_number;
	    }
	  else *code=run->rightface;
	  code++;
	  }
        *code = 0;  code++;
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nf)
    {  	*code=temp->rightface; code++;
        for (run = temp->invers->next; run != temp; run = run->invers->next)
          {
            *code = run->rightface; code++;
          }
        *code = 0;
        code++;
        temp = startedge[actual_number];  actual_number++;
    }
}

/**************************************************************************/

/**
 * Code is a sequence of numbers of length ne+nv+1.
 * The first entry is the number of vertices.
 * Then the numbers of the vertices adjacent to the vertex numbered 1 are
 * given -- ended by a 0, and listed in clockwise orientation.
 * Then those adjacent to 2, ended by a 0, etc.
 * 
 * Remark: this method is used for debugging purposes only!
 */
void compute_planar_code(unsigned char code[]) {
    code[0] = nv;
    int length = 1;

    EDGE *e, *ee;
    int i;
    for (i = 0; i < nv; i++) {
        e = ee = firstedge[i];
        do {
            code[length++] = e->end + 1;
            e = e->next;
        } while (e != ee);
        code[length++] = 0;
    }

    if(length != (nv + ne + 1)) {
        fprintf(stderr, "Error: invalid code length: %d vs %d\n", length, (nv + ne + 1));
        exit(1);
    }

}

/**************************************************************************/

/*
 * Writes planar code without relabelling the graph.
 * For debugging purposes only!
 */
static void
write_planar_code_modified(FILE *f, int doflip)

/* Write in planar_code format.  Always write in next direction,
   and if doflip != 0 also write in prev direction. */
{
    size_t length;
    unsigned char code[MAXN+MAXE+1];

    length=nv+ne+1;
    compute_planar_code(code);
    if (fwrite(code,sizeof(unsigned char),length,f) != length)
    {
        fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
        perror(">E ");
        exit(1);
    }

    if (doflip)
      {
        fprintf(stderr, "Error: doflip not supported at the moment\n");
        exit(1);
        compute_code_mirror(code);
        if (fwrite(code,sizeof(unsigned char),length,f) != length)
	  {
	    fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
	    perror(">E ");
	    exit(1);
	  }
      }
}

/**************************************************************************/

static void
write_planar_code(FILE *f, int doflip)

/* Write in planar_code format.  Always write in next direction,
   and if doflip != 0 also write in prev direction. */
{
    size_t length;
    unsigned char code[MAXN+MAXE+1];

    length=nv+ne+1;
    compute_code(code);
    if (fwrite(code,sizeof(unsigned char),length,f) != length)
    {
        fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
        perror(">E ");
        exit(1);
    }
    if (doflip)
      { compute_code_mirror(code);
        if (fwrite(code,sizeof(unsigned char),length,f) != length)
	  {
	    fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
	    perror(">E ");
	    exit(1);
	  }
      }
}

/**************************************************************************/

static void
write_dual_planar_code(FILE *f, int doflip)

/* Write the dual in planar_code format.  Always write in next direction,
   and if doflip != 0 also write in prev direction. */
{
    size_t length;
    unsigned char code[MAXF+MAXE+1];

    length=3+ne+(ne/2)-nv;
    compute_dual_code(code);
    if (fwrite(code,sizeof(unsigned char),length,f) != length)
    {
        fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
        perror(">E ");
        exit(1);
    }
    if (doflip)
      { compute_dual_code_mirror(code);
      if (fwrite(code,sizeof(unsigned char),length,f) != length)
	{
	  fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
	  perror(">E ");
	  exit(1);
	}
      }
}

/**************************************************************************/

static void
write_digits(FILE *f, int doflip)

/* Write in alphabetic format.  Always write in next direction,
   and if doflip != 0 also write in prev direction.  This output
   procedure uses the internal numbering of vertices and is
   intended for debugging purposes. */
{
    register int i,k;
    register EDGE *ex,*e;
    unsigned char code[2*MAXN+2*MAXE+9];
    int nvsize;
    int w;

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

    for(i = 0; i < nv; ++i) {
        e = ex = firstedge[i];
        do {
            w = e->end;
            if(w < 10) code[k++] = w + '0';
            else code[k++] = w + 'a' - 10;
            e = e->next;
        } while(e != ex);
        code[k++] = ',';
    }
    code[k - 1] = '\n';

    if(doflip) {
        for(i = 0; i < nvsize; ++i) code[k++] = code[i];

        for(i = 0; i < nv; ++i) {
            e = ex = firstedge[i];
            do {
                w = e->end;
                if(w < 10) code[k++] = w + '0';
                else code[k++] = w + 'a' - 10;
                e = e->prev;
            } while(e != ex);
            code[k++] = ',';
        }
    }
    code[k-1] = '\n';

    if (fwrite(code,(size_t)1,(size_t)k,f) != (size_t)k)
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
    int i, j, start;
    unsigned char code[MAXN+MAXE+4];
    unsigned char precode[MAXN+MAXE+1];
    size_t length;

    length=nv+ne;
    compute_code(precode);

    if (nv >= 10)
    {
        code[0] = '0' + nv/10;
        code[1] = '0' + nv%10;
        code[2] = ' ';
        length += 3;
	start=3;
    }
    else
    {
        code[0] = '0' + nv;
        code[1] = ' ';
        length += 2;
	start=2;
    }

    for (i = 1, j=start; j < length; ++i, ++j)
	if (precode[i]==0) code[j]=',';
        else code[j]=precode[i]-1+'a';

    code[j-1]='\n';
    if (fwrite(code,sizeof(unsigned char),length,f) != length)
    {
        fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
        perror(">E ");
        exit(1);
    }
    if (doflip)
      { compute_code_mirror(precode);
        for (i = 1, j=start; j < length; ++i, ++j)
	    if (precode[i]==0) code[j]=',';
            else code[j]=precode[i]-1+'a';
        code[j-1]='\n';
        if (fwrite(code,sizeof(unsigned char),length,f) != length)
	  {
	    fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
	    perror(">E ");
	    exit(1);
	  }
      }
}

/**************************************************************************/

static void
write_dual_alpha(FILE *f, int doflip)

/* Write the dual in alphabetic format.  Always write in next direction,
   and if doflip != 0 also write in prev direction. */
{
    int i,j,start;
    unsigned char code[MAXN+MAXE+4];
    unsigned char precode[MAXN+MAXE+1];
    size_t length;

    length=2+ne+(ne/2)-nv;
    compute_dual_code(precode);

    if (precode[0] >= 10)
    {
        code[0] = '0' + precode[0]/10;
        code[1] = '0' + precode[0]%10;
        code[2] = ' ';
        length += 3;
	start=3;
    }
    else
    {
        code[0] = '0' + precode[0];
        code[1] = ' ';
        length += 2;
	start=2;
    }

    for (i = 1, j=start; j < length; ++i, ++j)
        if (precode[i]==0) code[j]=',';
        else code[j]=precode[i]-1+'a';

    code[j-1]='\n';
    if (fwrite(code,sizeof(unsigned char),length,f) != length)
    {
        fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
        perror(">E ");
        exit(1);
    }
    if (doflip)
      { compute_dual_code_mirror(precode);
        for (i = 1, j=start; j < length; ++i, ++j)
	    if (precode[i]==0) code[j]=',';
            else code[j]=precode[i]-1+'a';
        code[j-1]='\n';
        if (fwrite(code,sizeof(unsigned char),length,f) != length)
	  {
	    fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
	    perror(">E ");
	    exit(1);
	  }
      }
}

/**************************************************************************/

static void
write_code_as_sparse6(FILE *f, unsigned char code[])

/* Write a graph represented in planar_code to a file in sparse6 format.
   The imbedding is lost but the vertex numbering is the same.  This
   goes not use any global variables and works to 255 vertices. */

{
    register unsigned char *pin,*pout;
    unsigned char s6[20+2*MAXE+2*MAXF];
    int n,nb,i,j,lastj,x,k,r,rr,topbit;
    int loopcount;

    pin = code;
    n = *pin++;
    pout = s6;
    *pout++ = ':';

    if (n <= 62)
        *pout++ = 63 + n;
    else
    {
        *pout++ = 63 + 63;
        *pout++ = 63 + 0;
        *pout++ = 63 + (n >> 6);
        *pout++ = 63 + (n & 0x3F);
    }

    for (i = n-1, nb = 0; i != 0 ; i >>= 1, ++nb) {}
    topbit = 1 << (nb-1);
    k = 6;
    x = 0;

    lastj = 0;
    for (j = 0; j < n; ++j)
    {
	loopcount = 0;   /* The input code shows loops once from each end,
                            but we want each loop just once in sparse6. */
        while ((i = *pin++) != 0)
        {
	    --i;
            if (i < j || (i == j && ((++loopcount)&1)))
            {
                if (j == lastj)
                {
                    x <<= 1;
                    if (--k == 0)
                    {
                        *pout++ = 63 + x;
                        k = 6;
                        x = 0;
                    }
                }
                else
                {
                    x = (x << 1) | 1;
                    if (--k == 0)
                    {
                        *pout++ = 63 + x;
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
                                *pout++ = 63 + x;
                                k = 6;
                                x = 0;
                            }
                        }
                        x <<= 1;
                        if (--k == 0)
                        {
                            *pout++ = 63 + x;
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
                        *pout++ = 63 + x;
                        k = 6;
                        x = 0;
                    }
                }
            }
        }
    }

    if (k != 6) *pout++ = 63 + ((x << k) | ((1 << k) - 1));

    *pout++ = '\n';
    k = pout - s6;

    if (fwrite(s6,sizeof(unsigned char),(size_t)k,f) != k)
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
    unsigned char code[MAXN+MAXE+1];

    compute_code(code);
    write_code_as_sparse6(f,code);
}

/**************************************************************************/

static void
write_dual_sparse6(FILE *f, int doflip)

/* Write dual cubic graph in sparse6 format.  doflip is ignored. */
{
    unsigned char code[MAXF+MAXE+1];

    compute_dual_code(code);
    write_code_as_sparse6(f,code);
}


/**************************************************************************/

static void
write_code_as_graph6(FILE *f, unsigned char code[])

/* Write a graph represented in planar_code to a file in graph6 format.
   The imbedding is lost, loops are lost, and multiple edges are changed
   to one edge.  The vertex numbering is the same.  This goes not use any
   global variables and works to 255 vertices. */

{
    unsigned char g6[20+MAXF*(MAXF-1)/12];
    register unsigned char *pin,*pout;
    int n,nlen,bodylen,i,j,org;
    static unsigned char g6bit[] = {32,16,8,4,2,1};

    pin = code;
    n = *pin++;

    if (n <= 62)
    {
        g6[0] = 63 + n;
        nlen = 1;
    }
    else
    {
        g6[0] = 63 + 63;
        g6[1] = 63 + 0;
        g6[2] = 63 + (n >> 6);
        g6[3] = 63 + (n & 0x3F);
        nlen = 4;
    }

    pout = g6 + nlen;
    bodylen = ((n * (n-1)) / 2 + 5) / 6;
    for (i = 0; i < bodylen; ++i) pout[i] = 0;
    pout[bodylen] = '\n';

    for (i = 0, org = -1; i < n; org += i, ++i)
    {
        while ((j = *pin++) != 0)
        {
            if (j <= i)
            {
                j += org;
                pout[j/6] |= g6bit[j%6];
            }
        }
    }

    for (i = 0; i < bodylen; ++i) pout[i] += 63;

    j = nlen + bodylen + 1;
    if (fwrite(g6,sizeof(unsigned char),(size_t)j,f) != j)
    {
        fprintf(stderr,">E %s: fwrite() failed\n",cmdname);
        perror(">E ");
        exit(1);
    }
}

/**************************************************************************/

static void
write_graph6(FILE *f, int doflip)

/* Write in graph6 format.  doflip is ignored. */
{
    unsigned char code[MAXN+MAXE+1];

    compute_code(code);
    write_code_as_graph6(f,code);
}

/**************************************************************************/

static void
write_dual_graph6(FILE *f, int doflip)

/* Write dual cubic graph in graph6 format.  doflip is ignored. */
{
    unsigned char code[MAXF+MAXE+1];

    compute_dual_code(code);
    write_code_as_graph6(f,code);
}

/**************************************************************************/

static void
check_it(int code, int triang)

/* Checks these properties:
   1. Degrees are correct.
   2. Faces are triangles (if triang)
   3. start and end fields are correct.
   4. min fields are ok.
   5. vertex numbers are in range.
*/

{
    int i,j;
    EDGE *e;

    for (i = 0; i < nv; ++i)
    {
	/*
        if (degree[i] < 3)
        {
            fprintf(stderr,">E degree error, code=%d\n",code);
            exit(1);
        }
	*/

        e = firstedge[i];
        for (j = 0; j < degree[i]; ++j) e = e->next;
        if (e != firstedge[i])
        {
            fprintf(stderr,">E next error, code=%d\n",code);
            exit(1);
        }

        e = firstedge[i];
        for (j = 0; j < degree[i]; ++j) e = e->prev;
        if (e != firstedge[i])
        {
            fprintf(stderr,">E prev error, code=%d\n",code);
            exit(1);
        }

        e = firstedge[i];
        for (j = 0; j < degree[i]; ++j)
        {
            e = e->next;
            if (e->start != i)
            {
                fprintf(stderr,">E start error, code=%d\n",code);
                exit(1);
            }
	    if (e->end < 0 || e->end >= nv)
            {
                fprintf(stderr,">E end label error, code=%d\n",code);
                exit(1);
            }
	    if (e->end != e->invers->start)
            {
                fprintf(stderr,">E invers label error, code=%d\n",code);
                exit(1);
	    }
            if (e->invers->end != i)
            {
                fprintf(stderr,">E end error, code=%d\n",code);
                exit(1);
            }
	    if (triang)
                if (e->invers->next == e
                    || e->invers->next->invers->next == e
                    || e->invers->next->invers->next->invers->next != e)
            {
                fprintf(stderr,">E face error, code=%d\n",code);
                exit(1);
            }

/*
            if (e->min != e && e->min != e->invers)
            {
                fprintf(stderr,">E min error 1, code=%d\n",code);
                exit(1);
            }

	    if (e->invers->min != e->min)
	    {
                fprintf(stderr,">E min error 2, code=%d\n",code);
                exit(1);
            }
*/
        }
    }
}

/**************************************************************************/

/*
 * Returns 1 if startedge has an unmarked neighbour, else returns 0.
 * If 1 is returned, unmarked_neighbour points to the first unmarked neighbour
 * of startedge in direction "use_next".
 */
static int
has_unmarked_neighbour(EDGE *startedge, int use_next, EDGE **unmarked_neighbour) {
    EDGE *e = startedge;
    do {
        if(!ISMARKED_V(e->end)) {
            *unmarked_neighbour = e;
            return 1;
        }
        if(use_next)
            e = e->next;
        else
            e = e->prev;
    } while(e != startedge);
    
    return 0;
}

/**************************************************************************/

/*
 * Returns 1 if the graph has a spiral starting from edge in direction "use_next",
 * else returns 0.
 */
static int
has_spiral(EDGE *edge, int use_next) {
    RESETMARKS_V;
    MARK_V(edge->start);
    int i;
    for(i = 2; i < nv; i++) {
        MARK_V(edge->end);
        if(!has_unmarked_neighbour(edge->invers, use_next, &edge))
            return 0;
    }
    return 1;
}
/**************************************************************************/

/*
 * Writes the correct header to fil.
 */
static void
write_header(FILE *fil) {
    if(!uswitch && !aswitch) {
        if((!zeroswitch && !hswitch && !gswitch && !sswitch &&
                fwrite(PCODE, (size_t) 1, PCODELEN, fil) != PCODELEN)
                || (hswitch && gswitch &&
                fwrite(G6CODE, (size_t) 1, G6CODELEN, fil) != G6CODELEN)
                || (hswitch && sswitch &&
                fwrite(S6CODE, (size_t) 1, S6CODELEN, fil) != S6CODELEN)) {
            fprintf(stderr, ">E %s: error writing header\n", cmdname);
            perror(">E ");
            exit(1);
        }
    }
}

/**************************************************************************/

static void
got_one(int nbtot, int nbop)

/* This is called when a canonical fullerene is generated.  The main
   purpose is to output the fullerene and to collect some stats. */
{
    int doflip,wt;
#ifdef STATS
    int numroot;
#endif
    
    doflip = oswitch && (nbop == nbtot || nbop == 0);
    wt = doflip ? 2 : 1;
    
    if (Vswitch)
    {
	if (nbtot == 1 || (oswitch && nbop == 1))
	{
	    ADDBIG(nout_V,wt);
	    return;
	}
    }    

#ifdef FILTER
    if (!FILTER(nbtot,nbop,doflip)) return;
#endif

    //number_of_graphs_generated[nv]++;
    
    ADDBIG(nout[nv], 1);
    ADDBIG(totalout,1);
    if (oswitch) ADDBIG(totalout_op,wt);
    
    if(spiralcheck) {
        int i, j, spiral_found = 0;
        EDGE *e;
        for(i = 0; i < 12 && !spiral_found; i++) {
            e = firstedge[degree_5_vertices[i]];
            for(j = 0; j < 5; j++) {
                if(has_spiral(e, 0) || has_spiral(e, 1)) {
                    spiral_found = 1;
                    break;
                }
                e = e->next;
            }
        }

        if(!spiral_found) {
            number_without_pentagon_spiral[nv]++;

            if(!uswitch) {
                FILE *fil;
                if(write_no_penta_spiral_header) {
                    fil = fopen(no_penta_spiral_filename, zeroswitch || aswitch || gswitch || sswitch ? "w" : "wb");
                    if(fil == NULL) {
                        fprintf(stderr, "Error: can't open %s\n", no_penta_spiral_filename);
                        exit(1);
                    }
                    write_header(fil);
                    write_no_penta_spiral_header = 0;
                } else {
                    fil = fopen(no_penta_spiral_filename, zeroswitch || aswitch || gswitch || sswitch ? "a" : "ab");
                    if(fil == NULL) {
                        fprintf(stderr, "Error: can't open %s\n", no_penta_spiral_filename);
                        exit(1);
                    }
                }

                if(dswitch) (*write_dual_graph)(fil, doflip);
                else (*write_graph)(fil, doflip);

                fclose(fil);
            }
        }
        
        for(i = 0; i < nv && !spiral_found; i++)
            if(degree[i] == 6) {
                e = firstedge[i];
                for(j = 0; j < 6; j++) {
                    if(has_spiral(e, 0) || has_spiral(e, 1)) {
                        spiral_found = 1;
                        break;
                    }
                    e = e->next;
                }
            }
        
        if(!spiral_found) {
            number_without_spiral[nv]++;

            if(!uswitch) {
                FILE *fil;
                if(write_no_spiral_header) {
                    fil = fopen(no_spiral_filename, zeroswitch || aswitch || gswitch || sswitch ? "w" : "wb");
                    if(fil == NULL) {
                        fprintf(stderr, "Error: can't open %s\n", no_spiral_filename);
                        exit(1);
                    }
                    write_header(fil);
                    write_no_spiral_header = 0;
                } else {
                    fil = fopen(no_spiral_filename, zeroswitch || aswitch || gswitch || sswitch ? "a" : "ab");
                    if(fil == NULL) {
                        fprintf(stderr, "Error: can't open %s\n", no_spiral_filename);
                        exit(1);
                    }
                }

                if(dswitch) (*write_dual_graph)(fil, doflip);
                else (*write_graph)(fil, doflip);

                fclose(fil);
            }
        }
            
    }

#ifdef STATS
    numroot = wt * numedgeorbits(nbtot, nbop);
    ADDBIG(numrooted[nv], numroot);
    if (nbtot == 1) 
        ADDBIG(ntriv[nv],wt);
#endif

    if (!uswitch)
    {
        if (dswitch) (*write_dual_graph)(outfile,doflip);
        else         (*write_graph)(outfile,doflip);
    }
}

/**************************************************************************/

/*
 * Returns 1 if there is a L0 reduction from startedge, else returns 0.
 * If 1 is returned direction_bitvector contains if a next and/or prev reductions are valid.
 *
 * Important: it is assumed that startedge->end has degree 5!
 */
static int
has_L0_reduction(EDGE *startedge, unsigned char *direction_bitvector) {
    if(degree[startedge->end] == 5) {
        *direction_bitvector = 0;
        if(degree[startedge->prev->prev->end] == 6 && degree[startedge->invers->prev->prev->end] == 6)
            (*direction_bitvector) |= PREV_BITVECTOR;
        if(degree[startedge->next->next->end] == 6 && degree[startedge->invers->next->next->end] == 6)
            (*direction_bitvector) |= NEXT_BITVECTOR;
        if(*direction_bitvector > 0)
            return 1;
    }

    return 0;
}

/****************************************************************************/

/*
 * Short reductions have higher priority.
 *
 * Returns 0 if there is no reduction starting from startedge which is at least
 * as good as the cur_best reduction.
 * Returns 1 if the best reduction from startedge is as good as the cur_best.
 * Returns 2 if there is a reduction from startedge which is better than cur_best.
 *
 * If 1 is returned endedge is the edge at the other pentagon of the straightpath
 * and direction_bitvector contains if a next and/or prev reductions are valid.
 */
static int
has_short_straight_reduction(EDGE *startedge, int pathlength_cur_best, EDGE **endedge,
        unsigned char *direction_bitvector) {
    EDGE *e = startedge;
    
    /*
     * Remark: marks actually only necessary for longer reductions.
     */

    RESETMARKS_V;
    int length = 1;
    //Pathlength i means distance i-1 between 2 deg 5 vertices in the extended graph
    while (length < pathlength_cur_best - 1 && degree[e->end] != 5)
    {
        //Straight paths with length <= 2 cannot be invalid
        //Remark: could even take longer paths...
        if(length == 2) {
            MARK_V(startedge->start);
            MARK_V(startedge->prev->end);
            MARK_V(startedge->prev->prev->end);
            MARK_V(startedge->next->end);
            MARK_V(startedge->next->next->end);
        }

	e = e->invers->next->next->next;
	if (ISMARKED_V(e->start)) return 0; else MARK_V(e->start);
        if (ISMARKED_V(e->prev->end)) return 0; else MARK_V(e->prev->end);
        if (ISMARKED_V(e->next->end)) return 0; else MARK_V(e->next->end);
        length++;
    }

    if(degree[e->end] == 5) {
        e = e->invers;
        if(ISMARKED_V(e->start)) return 0;
        if(ISMARKED_V(e->prev->prev->end)) return 0;
        if(ISMARKED_V(e->next->next->end)) return 0;

        if(length < pathlength_cur_best - 1) {
            if((degree[startedge->prev->prev->end] == 6 && degree[e->prev->prev->end] == 6) ||
                (degree[startedge->next->next->end] == 6 && degree[e->next->next->end] == 6))
                return 2;
        } else {
            DEBUGASSERT(length == pathlength_cur_best - 1);
            *direction_bitvector = 0;
            if(degree[startedge->prev->prev->end] == 6 && degree[e->prev->prev->end] == 6)
                (*direction_bitvector) |= PREV_BITVECTOR;
            if(degree[startedge->next->next->end] == 6 && degree[e->next->next->end] == 6)
                (*direction_bitvector) |= NEXT_BITVECTOR;
            if(*direction_bitvector > 0) {
                *endedge = e;
                return 1;
            } else
                return 0;
        }
    }

    return 0;
}

/****************************************************************************/

/*
 * Short reductions have higher priority.
 *
 * Returns 0 if there is no reduction starting from startedge which is at least
 * as good as the cur_best reduction.
 * Returns 1 if the best reduction from startedge is as good as the cur_best.
 * Returns 2 if there is a reduction from startedge which is better than cur_best.
 *
 * If 1 is returned endedge is the edge at the other pentagon of the straightpath
 * and direction_bitvector contains if a next and/or prev reductions are valid.
 * 
 * Remark 1: this method should only be called in case the last operation was an
 * L1-extension.
 * Remark 2: faces cannot be the same, since there is only one type of 4-cycle and
 * it doesn't lead to an L1 reduction.
 */
static int
has_short_straight_reduction_L1(EDGE *startedge, int pathlength_cur_best, EDGE **endedge,
        unsigned char *direction_bitvector) {
    EDGE *e = startedge;
    
    DEBUGASSERT(pathlength_cur_best == 3);
    
    int length = 1;
    //Pathlength i means distance i-1 between 2 deg 5 vertices in the extended graph
    if(degree[e->end] != 5) {
	e = e->invers->next->next->next;
        length++;
    }

    if(degree[e->end] == 5) {
        e = e->invers;

        if(length < pathlength_cur_best - 1) {
            if((degree[startedge->prev->prev->end] == 6 && degree[e->prev->prev->end] == 6) ||
                (degree[startedge->next->next->end] == 6 && degree[e->next->next->end] == 6))
                return 2;
        } else {
            DEBUGASSERT(length == pathlength_cur_best - 1);
            *direction_bitvector = 0;
            if(degree[startedge->prev->prev->end] == 6 && degree[e->prev->prev->end] == 6)
                (*direction_bitvector) |= PREV_BITVECTOR;
            if(degree[startedge->next->next->end] == 6 && degree[e->next->next->end] == 6)
                (*direction_bitvector) |= NEXT_BITVECTOR;
            if(*direction_bitvector > 0) {
                *endedge = e;
                return 1;
            } else
                return 0;
        }
    }

    return 0;
}

/****************************************************************************/

/*
 * Short reductions have higher priority.
 *
 * Returns 0 if there is no IPR reduction starting from startedge which is at least
 * as good as the cur_best reduction.
 * Returns 1 if the best reduction from startedge is as good as the cur_best.
 * Returns 2 if there is a reduction from startedge which is better than cur_best.
 *
 * If 1 is returned endedge is the edge at the other pentagon of the straightpath
 * and direction_bitvector contains if a next and/or prev reductions are valid.
 */
static int
has_short_straight_reduction_ipr(EDGE *startedge, int pathlength_cur_best, EDGE **endedge,
        unsigned char *direction_bitvector) {
    EDGE *e = startedge;
    int v[pathlength_cur_best+1],w[pathlength_cur_best+1];
    
    RESETMARKS_V;

    v[1] = e->prev->end;
    v[0] = e->prev->prev->end;
    w[1] = e->next->end;
    w[0] = e->next->next->end;
    int length = 1;
    //Pathlength i means distance i-1 between 2 deg 5 vertices in the extended graph
    while (length < pathlength_cur_best - 1 && degree[e->end] != 5)
    {
        //Straight paths with length <= 2 cannot be invalid
        //Remark: could even take longer paths
        if(length == 2) {
            MARK_V(startedge->start);
            MARK_V(startedge->prev->end);
            MARK_V(startedge->prev->prev->end);
            MARK_V(startedge->next->end);
            MARK_V(startedge->next->next->end);
        }

	e = e->invers->next->next->next;
        //Only necessary to check if length > 2, since IPR fullerenes do not contain 5-cycles (or shorter ones)
        if(length > 2) {
            if (ISMARKED_V(e->start) || ISMARKED_V(e->prev->end) || ISMARKED_V(e->next->end)) 
                return 0;
        }
        MARK_V(e->start);
        MARK_V(e->prev->end);
        MARK_V(e->next->end);
        
/*
	if (ISMARKED_V(e->start)) return 0; else MARK_V(e->start);
        if (ISMARKED_V(e->prev->end)) return 0; else MARK_V(e->prev->end);
        if (ISMARKED_V(e->next->end)) return 0; else MARK_V(e->next->end);
*/
        length++;
        
	v[length] = e->prev->end;
	w[length] = e->next->end;        
    }
    
    DEBUGASSERT(length < pathlength_cur_best);

    if(degree[e->end] == 5) {
        e = e->invers;
        
        if(ISMARKED_V(e->start)) return 0;
        if(ISMARKED_V(e->prev->prev->end)) return 0;
        if(ISMARKED_V(e->next->next->end)) return 0;
        
        int a, b, c, d, j;
        EDGE *ea, *eb, *ec, *ed;

        a = v[0];
        ea = startedge->prev->invers->next->invers;
        
        length++;
        c = w[length] = e->prev->prev->end;
        ec = e->prev->invers->next->invers;
        v[length] = e->next->next->end;

        int is_valid_reduction_a = 0;
        if(degree[a] == 6 && degree[c] == 6) {
            is_valid_reduction_a = 1;
            
            //Has to go till 3!
            for(j = 0; is_valid_reduction_a && j < 3; ++j) {
                ea = ea->prev;
                //if ea->end == c, then the reduced fullerene is not ipr
                if(ea->end == c || degree[ea->end] == 5) is_valid_reduction_a = 0;
            }
            for(j = 0; is_valid_reduction_a && j < 3; ++j) {
                ec = ec->prev;
                //Remark: ec->end == a doesn't have to be checked anymore
                //if(ec->end == a || degree[ec->end] == 5) is_valid_reduction_a = 0; 
                if(degree[ec->end] == 5) is_valid_reduction_a = 0; 
            }

            for(j = 0; is_valid_reduction_a && j < length; ++j)
                if(degree[w[j]] == 5 && degree[v[j + 1]] == 5) is_valid_reduction_a = 0;

            for(j = 1; is_valid_reduction_a && j < length; ++j)
                if(degree[w[j]] == 5 && degree[v[j]] == 5) is_valid_reduction_a = 0;

        }

        //Because of length++
        if(is_valid_reduction_a && length < pathlength_cur_best) {
        //if(is_valid_reduction_a && length < pathlength_cur_best - 1) {
            return 2;
        }
        
        
        b = w[0];
        eb = startedge->next->invers->prev->invers;   
        d = v[length];
        ed = e->next->invers->prev->invers;
        
        int is_valid_reduction_b = 0;
        if(degree[b] == 6 && degree[d] == 6) {
            is_valid_reduction_b = 1;

            for(j = 0; is_valid_reduction_b && j < 3; ++j) {
                eb = eb->next;
                if(eb->end == d || degree[eb->end] == 5) is_valid_reduction_b = 0;
            }
            for(j = 0; is_valid_reduction_b && j < 3; ++j) {
                ed = ed->next;
                //Remark: ed->end == b doesn't have to be checked anymore
                //if(ed->end == b || degree[ed->end] == 5) is_valid_reduction_b = 0;
                if(degree[ed->end] == 5) is_valid_reduction_b = 0;
            }

            for(j = 0; is_valid_reduction_b && j < length; ++j)
                if(degree[v[j]] == 5 && degree[w[j + 1]] == 5) is_valid_reduction_b = 0;

            for(j = 1; is_valid_reduction_b && j < length; ++j)
                if(degree[v[j]] == 5 && degree[w[j]] == 5) is_valid_reduction_b = 0;

        }
        
        if(is_valid_reduction_b && length < pathlength_cur_best) {
            return 2;
        }
        
        DEBUGASSERT(length == pathlength_cur_best || (!is_valid_reduction_a && !is_valid_reduction_b));
        *direction_bitvector = 0;
        if(is_valid_reduction_a)
            (*direction_bitvector) |= PREV_BITVECTOR;
        if(is_valid_reduction_b)
            (*direction_bitvector) |= NEXT_BITVECTOR;
        if(*direction_bitvector > 0) {
            *endedge = e;
            return 1;
        } else
            return 0;

    }

    return 0;
}

/****************************************************************************/

/*
 * Short reductions have higher priority.
 *
 * Returns 0 if there is no IPR reduction starting from startedge which is at least
 * as good as the cur_best reduction.
 * Returns 1 if the best reduction from startedge is as good as the cur_best.
 * Returns 2 if there is a reduction from startedge which is better than cur_best.
 *
 * If 1 is returned endedge is the edge at the other pentagon of the straightpath
 * and direction_bitvector contains if a next and/or prev reductions are valid.
 * 
 * Remark: no need to mark since faces cannot be the same: in IPR fullerenes 
 * there are no 5-cycles which lead to a valid L2 reduction.
 */
static int
has_short_straight_reduction_L2_ipr(EDGE *startedge, int pathlength_cur_best, EDGE **endedge,
        unsigned char *direction_bitvector) {
    EDGE *e = startedge;
    int v[pathlength_cur_best+1],w[pathlength_cur_best+1];
    
    DEBUGASSERT(pathlength_cur_best == 4);
    
    v[1] = e->prev->end;
    v[0] = e->prev->prev->end;
    w[1] = e->next->end;
    w[0] = e->next->next->end;
    int length = 1;
    //Pathlength i means distance i-1 between 2 deg 5 vertices in the extended graph
    while (length < pathlength_cur_best - 1 && degree[e->end] != 5)
    {
	e = e->invers->next->next->next;
        length++;
        
	v[length] = e->prev->end;
	w[length] = e->next->end;        
    }
    
    DEBUGASSERT(length < pathlength_cur_best);

    if(degree[e->end] == 5) {
        e = e->invers;
        
        int a, b, c, d, j;
        EDGE *ea, *eb, *ec, *ed;

        a = v[0];
        ea = startedge->prev->invers->next->invers;
        
        length++;
        c = w[length] = e->prev->prev->end;
        ec = e->prev->invers->next->invers;
        v[length] = e->next->next->end;

        int is_valid_reduction_a = 0;
        if(degree[a] == 6 && degree[c] == 6) {
            is_valid_reduction_a = 1;
            
            //Has to go till 3!
            for(j = 0; is_valid_reduction_a && j < 3; ++j) {
                ea = ea->prev;
                //if ea->end == c, then the reduced fullerene is not ipr
                if(ea->end == c || degree[ea->end] == 5) is_valid_reduction_a = 0;
            }
            for(j = 0; is_valid_reduction_a && j < 3; ++j) {
                ec = ec->prev;
                //Remark: ec->end == a doesn't have to be checked anymore
                //if(ec->end == a || degree[ec->end] == 5) is_valid_reduction_a = 0; 
                if(degree[ec->end] == 5) is_valid_reduction_a = 0; 
            }

            for(j = 0; is_valid_reduction_a && j < length; ++j)
                if(degree[w[j]] == 5 && degree[v[j + 1]] == 5) is_valid_reduction_a = 0;

            for(j = 1; is_valid_reduction_a && j < length; ++j)
                if(degree[w[j]] == 5 && degree[v[j]] == 5) is_valid_reduction_a = 0;

        }

        //Because of length++
        if(is_valid_reduction_a && length < pathlength_cur_best) {
        //if(is_valid_reduction_a && length < pathlength_cur_best - 1) {
            return 2;
        }
        
        
        b = w[0];
        eb = startedge->next->invers->prev->invers;   
        d = v[length];
        ed = e->next->invers->prev->invers;
        
        int is_valid_reduction_b = 0;
        if(degree[b] == 6 && degree[d] == 6) {
            is_valid_reduction_b = 1;

            for(j = 0; is_valid_reduction_b && j < 3; ++j) {
                eb = eb->next;
                if(eb->end == d || degree[eb->end] == 5) is_valid_reduction_b = 0;
            }
            for(j = 0; is_valid_reduction_b && j < 3; ++j) {
                ed = ed->next;
                //Remark: ed->end == b doesn't have to be checked anymore
                //if(ed->end == b || degree[ed->end] == 5) is_valid_reduction_b = 0;
                if(degree[ed->end] == 5) is_valid_reduction_b = 0;
            }

            for(j = 0; is_valid_reduction_b && j < length; ++j)
                if(degree[v[j]] == 5 && degree[w[j + 1]] == 5) is_valid_reduction_b = 0;

            for(j = 1; is_valid_reduction_b && j < length; ++j)
                if(degree[v[j]] == 5 && degree[w[j]] == 5) is_valid_reduction_b = 0;

        }
        
        if(is_valid_reduction_b && length < pathlength_cur_best) {
            return 2;
        }
        
        DEBUGASSERT(length == pathlength_cur_best || (!is_valid_reduction_a && !is_valid_reduction_b));
        *direction_bitvector = 0;
        if(is_valid_reduction_a)
            (*direction_bitvector) |= PREV_BITVECTOR;
        if(is_valid_reduction_b)
            (*direction_bitvector) |= NEXT_BITVECTOR;
        if(*direction_bitvector > 0) {
            *endedge = e;
            return 1;
        } else
            return 0;

    }

    return 0;
}

/****************************************************************************/

/*
 * Short reductions have higher priority.
 *
 * Returns 0 if there is no IPR reduction starting from startedge which is at least
 * as good as the cur_best reduction.
 * Returns 1 if the best reduction from startedge is as good as the cur_best.
 * Returns 2 if there is a reduction from startedge which is better than cur_best.
 *
 * If 1 is returned endedge is the edge at the other pentagon of the straightpath
 * and direction_bitvector contains if a next and/or prev reductions are valid.
 * 
 * Remark: this method should only be called in case the last operation was an
 * L1-extension.
 */
//Important: it is assumed that the fullerene is IPR!
static int
has_short_straight_reduction_L1_ipr(EDGE *startedge, int pathlength_cur_best, EDGE **endedge,
        unsigned char *direction_bitvector) {
    
    if(degree[startedge->invers->next->next->next->end] == 5) {
        EDGE *e = startedge;
        int v[4], w[4];

        DEBUGASSERT(pathlength_cur_best == 3);

        v[1] = e->prev->end;
        v[0] = e->prev->prev->end;
        w[1] = e->next->end;
        w[0] = e->next->next->end;
        int length = 1;
        //Pathlength i means distance i-1 between 2 deg 5 vertices in the extended graph

        //fullerene is ipr
        //if(degree[e->end] != 5) {
        e = e->invers->next->next->next;
        length++;
        v[length] = e->prev->end;
        w[length] = e->next->end;
        //}        
        
        e = e->invers;

        int a, b, c, d, j;
        EDGE *ea, *eb, *ec, *ed;

        a = startedge->prev->prev->end;
        ea = startedge->prev->invers->next->invers;
        
        length++;
        c = w[length] = e->prev->prev->end;
        ec = e->prev->invers->next->invers;
        v[length] = e->next->next->end;
        
        
        int is_valid_reduction_a = 0;
        if(degree[a] == 6 && degree[c] == 6) {
            is_valid_reduction_a = 1;
            
            //Has to go till 3!
            for(j = 0; is_valid_reduction_a && j < 3; ++j) {
                ea = ea->prev;
                //if ea->end == c, then the reduced fullerene is not ipr
                if(ea->end == c || degree[ea->end] == 5) is_valid_reduction_a = 0;
            }
            for(j = 0; is_valid_reduction_a && j < 3; ++j) {
                ec = ec->prev;
                //Remark: ec->end == a doesn't have to be checked anymore
                //if(ec->end == a || degree[ec->end] == 5) is_valid_reduction_a = 0; 
                if(degree[ec->end] == 5) is_valid_reduction_a = 0;                 
            }

            for(j = 0; is_valid_reduction_a && j < length; ++j)
                if(degree[w[j]] == 5 && degree[v[j + 1]] == 5) is_valid_reduction_a = 0;

            for(j = 1; is_valid_reduction_a && j < length; ++j)
                if(degree[w[j]] == 5 && degree[v[j]] == 5) is_valid_reduction_a = 0;

        }
        
        b = startedge->next->next->end;
        eb = startedge->next->invers->prev->invers;   
        d = v[length];
        ed = e->next->invers->prev->invers;
        
        int is_valid_reduction_b = 0;
        if(degree[b] == 6 && degree[d] == 6) {
            is_valid_reduction_b = 1;

            for(j = 0; is_valid_reduction_b && j < 3; ++j) {
                eb = eb->next;
                if(eb->end == d || degree[eb->end] == 5) is_valid_reduction_b = 0;
            }
            for(j = 0; is_valid_reduction_b && j < 3; ++j) {
                ed = ed->next;
                //Remark: ed->end == b doesn't have to be checked anymore
                //if(ed->end == b || degree[ed->end] == 5) is_valid_reduction_b = 0;
                if(degree[ed->end] == 5) is_valid_reduction_b = 0;                
            }

            for(j = 0; is_valid_reduction_b && j < length; ++j)
                if(degree[v[j]] == 5 && degree[w[j + 1]] == 5) is_valid_reduction_b = 0;

            for(j = 1; is_valid_reduction_b && j < length; ++j)
                if(degree[v[j]] == 5 && degree[w[j]] == 5) is_valid_reduction_b = 0;

        }        
        
        
        DEBUGASSERT(length == pathlength_cur_best);
        *direction_bitvector = 0;
        if(is_valid_reduction_a)
            (*direction_bitvector) |= PREV_BITVECTOR;
        if(is_valid_reduction_b)
            (*direction_bitvector) |= NEXT_BITVECTOR;
        if(*direction_bitvector > 0) {
            *endedge = e;
            return 1;
        } else
            return 0;
    }

    return 0;
}

/****************************************************************************/

/**
 * See has_short_straight_reduction_L1_ipr.
 * Also checks if startedge->start is a 5-vertex.
 */
static int
has_short_straight_reduction_L1_ipr_modified(EDGE *startedge) {
    
    if(degree[startedge->start] != 5)
        return 0;
    
    if(degree[startedge->invers->next->next->next->end] == 5) {
        EDGE *e = startedge;
        int v[4], w[4];

        //DEBUGASSERT(pathlength_cur_best == 3);

        v[1] = e->prev->end;
        v[0] = e->prev->prev->end;
        w[1] = e->next->end;
        w[0] = e->next->next->end;
        int length = 1;
        //Pathlength i means distance i-1 between 2 deg 5 vertices in the extended graph

        //fullerene is ipr
        //if(degree[e->end] != 5) {
        e = e->invers->next->next->next;
        length++;
        v[length] = e->prev->end;
        w[length] = e->next->end;
        //}        
        
        e = e->invers;

        int a, b, c, d, j;
        EDGE *ea, *eb, *ec, *ed;

        a = startedge->prev->prev->end;
        ea = startedge->prev->invers->next->invers;
        
        length++;
        c = w[length] = e->prev->prev->end;
        ec = e->prev->invers->next->invers;
        v[length] = e->next->next->end;
        
        
        int is_valid_reduction_a = 0;
        if(degree[a] == 6 && degree[c] == 6) {
            is_valid_reduction_a = 1;
            
            //Has to go till 3!
            for(j = 0; is_valid_reduction_a && j < 3; ++j) {
                ea = ea->prev;
                //if ea->end == c, then the reduced fullerene is not ipr
                if(ea->end == c || degree[ea->end] == 5) is_valid_reduction_a = 0;
            }
            for(j = 0; is_valid_reduction_a && j < 3; ++j) {
                ec = ec->prev;
                //Remark: ec->end == a doesn't have to be checked anymore
                //if(ec->end == a || degree[ec->end] == 5) is_valid_reduction_a = 0; 
                if(degree[ec->end] == 5) is_valid_reduction_a = 0;                 
            }

            for(j = 0; is_valid_reduction_a && j < length; ++j)
                if(degree[w[j]] == 5 && degree[v[j + 1]] == 5) is_valid_reduction_a = 0;

            for(j = 1; is_valid_reduction_a && j < length; ++j)
                if(degree[w[j]] == 5 && degree[v[j]] == 5) is_valid_reduction_a = 0;

        }
        
        b = startedge->next->next->end;
        eb = startedge->next->invers->prev->invers;   
        d = v[length];
        ed = e->next->invers->prev->invers;
        
        int is_valid_reduction_b = 0;
        if(degree[b] == 6 && degree[d] == 6) {
            is_valid_reduction_b = 1;

            for(j = 0; is_valid_reduction_b && j < 3; ++j) {
                eb = eb->next;
                if(eb->end == d || degree[eb->end] == 5) is_valid_reduction_b = 0;
            }
            for(j = 0; is_valid_reduction_b && j < 3; ++j) {
                ed = ed->next;
                //Remark: ed->end == b doesn't have to be checked anymore
                //if(ed->end == b || degree[ed->end] == 5) is_valid_reduction_b = 0;
                if(degree[ed->end] == 5) is_valid_reduction_b = 0;                
            }

            for(j = 0; is_valid_reduction_b && j < length; ++j)
                if(degree[v[j]] == 5 && degree[w[j + 1]] == 5) is_valid_reduction_b = 0;

            for(j = 1; is_valid_reduction_b && j < length; ++j)
                if(degree[v[j]] == 5 && degree[w[j]] == 5) is_valid_reduction_b = 0;

        }        
        
        //DEBUGASSERT(length == pathlength_cur_best);
        if(is_valid_reduction_a || is_valid_reduction_b)
            return 1;
        else
            return 0;
    }

    return 0;
}

/****************************************************************************/

/*
 * Returns 1 if the straight reduction with startedge edge is already in the
 * list of straight reductions, else returns 0.
 */
static int is_already_in_list(EDGE *edge, int length) {
    int i;
    for(i = 0; i < num_straight_extensions; i++) {
        if((straight_extensions[i][0] == edge->start && straight_extensions[i][1] == edge->end)
                || (straight_extensions[i][length - 1] == edge->start && straight_extensions[i][length - 2] == edge->end))
            return 1;
    }
    return 0;
}

/****************************************************************************/

/*
 * Adds the straight reduction starting from edge with the specified length to
 * the list of shortest reductions. This list is used by
 * might_destroy_previous_straight_extension().
 * Important: It is assumed that the reduction is valid.
 *
 * The extension is stored as follows in prev_canon_straight_extensions:
 *  - position 0..length - 1 contains the straight path
 *  - position length and length + 1 are the 2 adjacent degree 6 vertices
 */
static void
add_straight_extension_to_list(EDGE *edge, int length, int use_next) {
    int i = 0;
    EDGE *e = edge;
    straight_extensions[num_straight_extensions][i++] = e->start;
    for(; i < length - 1; i++) {
        e = e->invers->next->next->next;
        straight_extensions[num_straight_extensions][i] = e->start;
    }
    e = e->invers;
    straight_extensions[num_straight_extensions][i++] = e->start;
    if(use_next) {
        straight_extensions[num_straight_extensions][i++] = edge->next->next->end;
        straight_extensions[num_straight_extensions][i++] = e->next->next->end;
    } else {
        straight_extensions[num_straight_extensions][i++] = edge->prev->prev->end;
        straight_extensions[num_straight_extensions][i++] = e->prev->prev->end;
    }

    num_straight_extensions++;

}

/****************************************************************************/

static void
add_L2_extension_to_list(EDGE *edge, int length, int use_next) {
    DEBUGASSERT(length == 4);
    int i = 0;
    EDGE *e = edge;
    L2_extensions[num_L2_extensions][i++] = e->start;
    for(; i < length - 1; i++) {
        e = e->invers->next->next->next;
        L2_extensions[num_L2_extensions][i] = e->start;
    }
    e = e->invers;
    L2_extensions[num_L2_extensions][i++] = e->start;
    if(use_next) {
        L2_extensions[num_L2_extensions][i++] = e->next->next->end;
        L2_extensions[num_L2_extensions][i++] = edge->next->next->end;
    } else {
        L2_extensions[num_L2_extensions][i++] = e->prev->prev->end;
        L2_extensions[num_L2_extensions][i++] = edge->prev->prev->end;
    }
    num_L2_extensions++;
    
    DEBUGASSERT(i == L2_SIZE);
}

/****************************************************************************/

static int bent_zero_is_already_in_list(int p1, int p2) {
    int i;
    for(i = 0; i < num_bent_zero_extensions; i++)
        if((bent_zero_extensions[i][0] == p1 && bent_zero_extensions[i][3] == p2)
                || (bent_zero_extensions[i][0] == p2 && bent_zero_extensions[i][3] == p1))
            return 1;
    return 0;
}

/****************************************************************************/

static void
add_bent_zero_extension_to_list_ipr(EDGE *startedge, int use_next) {
    int endvertex;
    if(use_next)
        endvertex = startedge->prev->prev->invers->next->next->end;
    else
        endvertex = startedge->next->next->invers->prev->prev->end;
    
    if(bent_zero_is_already_in_list(startedge->start, endvertex))
        return;
    
    int index = 0, j;
    EDGE *e = startedge;
    bent_zero_extensions[num_bent_zero_extensions][index++] = e->start;
    bent_zero_extensions[num_bent_zero_extensions][index++] = e->end;
    //bent_zero_extensions[num_bent_zero_extensions][index++] = e->prev->end;
    //bent_zero_extensions[num_bent_zero_extensions][index++] = e->next->end;
    if(use_next) {
        e = e->prev->prev;
        bent_zero_extensions[num_bent_zero_extensions][index++] = e->end;

        e = e->invers->next->next;
        DEBUGASSERT(degree[e->end] == 5);
        bent_zero_extensions[num_bent_zero_extensions][index++] = e->end;
        //bent_zero_extensions[num_bent_zero_extensions][index++] = e->prev->end;
        //bent_zero_extensions[num_bent_zero_extensions][index++] = e->next->end;
        //bent_zero_extensions[num_bent_zero_extensions][index++] = e->next->next->end;

        e = e->invers->prev->prev;
        bent_zero_extensions[num_bent_zero_extensions][index++] = e->end;
        //bent_zero_extensions[num_bent_zero_extensions][index++] = e->prev->end;

        e = e->invers->next;
        for(j = 0; j < 3; j++) {
            e = e->next;
            bent_zero_extensions[num_bent_zero_extensions][index++] = e->end;
        }
        e = startedge->invers->prev;
        for(j = 0; j < 3; j++) {
            e = e->prev;
            bent_zero_extensions[num_bent_zero_extensions][index++] = e->end;
        }
    } else {
        e = e->next->next;
        bent_zero_extensions[num_bent_zero_extensions][index++] = e->end;

        e = e->invers->prev->prev;
        DEBUGASSERT(degree[e->end] == 5);
        bent_zero_extensions[num_bent_zero_extensions][index++] = e->end;
        //bent_zero_extensions[num_bent_zero_extensions][index++] = e->next->end;
        //bent_zero_extensions[num_bent_zero_extensions][index++] = e->prev->end;
        //bent_zero_extensions[num_bent_zero_extensions][index++] = e->prev->prev->end;

        e = e->invers->next->next;
        bent_zero_extensions[num_bent_zero_extensions][index++] = e->end;
        //bent_zero_extensions[num_bent_zero_extensions][index++] = e->next->end;

        e = e->invers->prev;
        for(j = 0; j < 3; j++) {
            e = e->prev;
            bent_zero_extensions[num_bent_zero_extensions][index++] = e->end;
        }
        e = startedge->invers->next;
        for(j = 0; j < 3; j++) {
            e = e->next;
            bent_zero_extensions[num_bent_zero_extensions][index++] = e->end;
        }
    }

    DEBUGASSERT(index == BENT_ZERO_SIZE);
    num_bent_zero_extensions++;    
    
}

/****************************************************************************/

/*
 * Adds a L0 expansion which has the best (colour1, colour2) to the list.
 * Edge1 and edge2 are assumed to be the 2 "dangling" edges of the L0 expansion.
 */
static void
add_straight_colour_2_L0_extension_to_list(EDGE *edge1, EDGE *edge2) {
    int i = 0;
    /* The first 4 vertices are the vertices of the expansion */
    straight_colour_2_L0_extensions[num_straight_colour_2_L0_extensions][i++] = edge1->start;
    straight_colour_2_L0_extensions[num_straight_colour_2_L0_extensions][i++] = edge1->end;
    straight_colour_2_L0_extensions[num_straight_colour_2_L0_extensions][i++] = edge2->start;
    straight_colour_2_L0_extensions[num_straight_colour_2_L0_extensions][i++] = edge2->end;
    DEBUGASSERT(i == STRAIGHT_COLOUR_2_L0_LENGTH_EXPANSION);

    /*
     * The remaining 10 vertices are the other ones which are in the neighbourhood
     * of edge1->start and edge2->start.
     */
    int j;
    for(j = 0; j < 5; j++) {
        edge1 = edge1->next;
        straight_colour_2_L0_extensions[num_straight_colour_2_L0_extensions][i++] = edge1->end;
    }
    for(j = 0; j < 5; j++) {
        edge2 = edge2->next;
        straight_colour_2_L0_extensions[num_straight_colour_2_L0_extensions][i++] = edge2->end;
    }
    DEBUGASSERT(i == STRAIGHT_COLOUR_2_L0_LENGTH);
    num_straight_colour_2_L0_extensions++;
    DEBUGASSERT(num_straight_colour_2_L0_extensions <= MAX_PREV_EXTENSIONS);
}

/**************************************************************************/

/*
 * Searches L2 reductions in the current fullerene and saves them into
 * straight_colour_2_L0_extensions[].
 * 
 * Important: it is assumed that num_L2_extensions = 0 and that 
 * MAX_PREV_EXTENSIONS > 0.
 */
static void
search_L2_reductions_ipr() {
    //Only called for extensions which add at least 4 new vertices
    DEBUGASSERT(nv <= maxnv - 4 && MAX_PREV_EXTENSIONS > 0);

    DEBUGASSERT(num_L2_extensions == 0);
    RESETMARKS;
    EDGE *e, *ee;
    EDGE *endedge;
    unsigned char direction_bitvector;
    int i, res;
    for(i = 0; i < 12; i++) {
        e = ee = firstedge[degree_5_vertices[i]];
        do {
            if(!ISMARKEDLO(e)) {
                res = has_short_straight_reduction_L2_ipr(e, 4, &endedge,
                        &direction_bitvector);
                if(res == 1) {
                    add_L2_extension_to_list(e, 4, (direction_bitvector & NEXT_BITVECTOR) != 0);
                    if(num_L2_extensions == MAX_PREV_EXTENSIONS)
                        return;

                    //No need to mark e since it wont be visited anymore
                    //Can't mark outside if, since then endedge is invalid!
                    MARKLO(endedge);
                }
            }
            e = e->next;
        } while(e != ee);
    }
}

/****************************************************************************/

static int
are_adjacent(int x, int y)
/* Test if x and y are adjacent */
{
    EDGE *e,*ee;

    e = ee = firstedge[x];
    do
    {
	if (e->end == y) return 1;
    } while ((e = e->next) != ee);

    return 0;
}

/**************************************************************************/

/*
 * Returns 1 if there is a bent zero reduction to a smaller IPR fullerene 
 * starting from startedge with direction use_next, else returns 0.
 * If 1 is returned, endedge contains the last edge of the reduction.
 * 
 * Important: it is assumed that the fullerene is IPR.
 */
/*
 * Remark: can never occur that faces are the same, so no need to mark.
 */
static int
has_B10_reduction_ipr_endedge(EDGE *startedge, int use_next, EDGE **endedge) {
    DEBUGASSERT(degree[startedge->end] == 6);
    
    int a, b;
    EDGE *ea, *eb;
    
    EDGE *e = startedge;
    if(use_next) {
        e = e->prev->prev;
        DEBUGASSERT(degree[e->end] != 5);
        
        e = e->invers->next->next->next;
        if(degree[e->end] == 5)
            return 0;
        
        e = e->invers->next->next;
        if(degree[e->end] == 6)
            return 0;

        e = e->invers->prev->prev;
        DEBUGASSERT(degree[e->end] != 5);
        
        a = e->end;
        ea = e->invers->prev;
        
        b = startedge->end;
        eb = startedge->invers->next;

    } else {
        e = e->next->next;
        DEBUGASSERT(degree[e->end] != 5);
        
        e = e->invers->prev->prev->prev;
        if(degree[e->end] == 5)
            return 0;        

        e = e->invers->prev->prev;
        if(degree[e->end] == 6)
            return 0;

        e = e->invers->next->next;
        DEBUGASSERT(degree[e->end] != 5);
        
        b = e->end;
        eb = e->invers->next;
        
        a = startedge->end;
        ea = startedge->invers->prev;
    }

    /* Test if reduced graph is IPR */
    //No need to check v's and w's here, since there can be no conflicts after reduction
    if(are_adjacent(a, b)) return 0;
    if(degree[ea->prev->end] == 5 || degree[ea->prev->prev->end] == 5
            || degree[ea->prev->prev->prev->end] == 5) return 0;
    if(degree[eb->next->end] == 5 || degree[eb->next->next->end] == 5
            || degree[eb->next->next->next->end] == 5) return 0;

    *endedge = e;
    return 1;    

}

/****************************************************************************/

/*
 * Adds the B10 reduction starting at startedge to the list of
 * B10 reductions.
 */
static void
add_B10_extension_to_list(EDGE *startedge, int use_next) {
    EDGE *e = startedge;
    int i = 0;
    bent_one_zero_extensions[num_B10_extensions][i++] = e->start;
    if(use_next) {
        e = e->prev->prev;
        bent_one_zero_extensions[num_B10_extensions][i++] = e->end;
        
        e = e->invers->next->next->next;
        bent_one_zero_extensions[num_B10_extensions][i++] = e->end;
        
        e = e->invers->next->next;
        bent_one_zero_extensions[num_B10_extensions][i++] = e->end;

        e = e->invers->prev->prev;
    } else {
        e = e->next->next;
        bent_one_zero_extensions[num_B10_extensions][i++] = e->end;
        
        e = e->invers->prev->prev->prev;
        bent_one_zero_extensions[num_B10_extensions][i++] = e->end;
        
        e = e->invers->prev->prev;
        bent_one_zero_extensions[num_B10_extensions][i++] = e->end;

        e = e->invers->next->next;
    }    
    
    bent_one_zero_extensions[num_B10_extensions][i++] = startedge->end;
    bent_one_zero_extensions[num_B10_extensions][i++] = e->end;
    num_B10_extensions++;
    
    DEBUGASSERT(i == B10_SIZE);
}

/****************************************************************************/

static int B10_is_already_in_list(int p1, int p2) {
    int i;
    for(i = 0; i < num_B10_extensions; i++)
        if((bent_one_zero_extensions[i][0] == p1 && bent_one_zero_extensions[i][3] == p2)
                || (bent_one_zero_extensions[i][0] == p2 && bent_one_zero_extensions[i][3] == p1))
            return 1;
    return 0;
}

/****************************************************************************/

/*
 * Searches B10 reductions in the current fullerene and saves them into
 * bent_one_zero_extensions[].
 * 
 * Important: it is assumed that num_B10_extensions = 0 and that 
 * MAX_PREV_EXTENSIONS > 0.
 */
static void
search_B10_reductions_ipr() {
    //Only called for extensions which add at least 4 new vertices
    DEBUGASSERT(nv <= maxnv - 4 && MAX_PREV_EXTENSIONS > 0);
    DEBUGASSERT(num_B10_extensions == 0);
    DEBUGASSERT(fulleriprswitch);
    EDGE *e, *endedge;
    int i, j, k;
    for(i = 0; i < 12; i++) {
        e = firstedge[degree_5_vertices[i]];
        for(j = 0; j < 5; j++) {
            //if(degree[e->end] == 6) //Check not necessary for IPR
            for(k = 0; k < 2; k++) {
                if(has_B10_reduction_ipr_endedge(e, k, &endedge)
                        && !B10_is_already_in_list(e->start, endedge->start)) {
                    add_B10_extension_to_list(e, k);
                    if(num_B10_extensions == MAX_PREV_EXTENSIONS)
                        return;
                }
            }
            e = e->next;
        }
    }
}

/****************************************************************************/

/*
 * Returns 1 if one of the first can_edges_next edges of good_next or one
 * of the first can_edges_prev edges of good_prev is canonical using
 * canon_edge_oriented_short, else returns 0.
 * 
 * If 1 is returned good_next[] contains a list of all "next"-edges which could
 * be canonical and good_prev[] of all such "prev"-edges.
 * 
 * Remark: it is assu,ed that good_next, good_prev, can_edges_next and
 * can_edges_prev are valid. 
 */
static int
test_canon_edge_short(int num_good_next, int num_good_prev,
        EDGE *good_next[], int *ngood_next, int *can_edges_next, EDGE *good_prev[], int *ngood_prev, int *can_edges_prev) {
    //Just one canonical edge
    if(num_good_next + num_good_prev == 1 || nv < NV_CANON_SHORT + 20) {
        *ngood_next = num_good_next;
        *ngood_prev = num_good_prev;

        return 1;
    } else {
        DEBUGASSERT(nv >= NV_CANON_SHORT);
        
        EDGE *good_next_tmp[num_good_next];
        EDGE *good_prev_tmp[num_good_prev];

        memcpy(good_next_tmp, good_next, sizeof(EDGE *) * num_good_next);
        memcpy(good_prev_tmp, good_prev, sizeof(EDGE *) * num_good_prev);

        int res = canon_edge_oriented_short(good_next_tmp, num_good_next, *can_edges_next,
                good_prev_tmp, num_good_prev, *can_edges_prev, degree,
                good_next, ngood_next, can_edges_next,
                good_prev, ngood_prev, can_edges_prev);

        return res;
    }
    
}

/****************************************************************************/

/*
 * Returns 1 if test_edge1 or test_edge2 have the best colour among the edges
 * from good_next_tmp and good_prev_tmp. It is assumed that test_edge{1,2}
 * can only be best if testedge{1,2}_is_best. It is assumed that at least one
 * of them = 1.
 *
 * If 1 is returned good_next[] contains a list of all "next"-edges which could
 * be canonical and good_prev[] of all such "prev"-edges.
 */
static int
is_best_third_colour(EDGE *test_edge1, EDGE *test_edge2, int testedge1_is_best, int testedge2_is_best, int use_next,
        EDGE *good_next_tmp[], int num_good_next_tmp, EDGE *good_prev_tmp[], int num_good_prev_tmp,
        EDGE *good_next[], int *ngood_next, int *can_edges_next, EDGE *good_prev[], int *ngood_prev, int *can_edges_prev) {
    int i, best_colour, colour_tmp;
    int colour_testedge1, colour_testedge2;
    int num_good_next, num_good_prev;
    num_good_next = num_good_prev = 0;

    DEBUGASSERT(testedge1_is_best || testedge2_is_best);
    if(use_next) {
        if(testedge1_is_best && testedge2_is_best) {
            colour_testedge1 = get_path_colour_next(test_edge1->prev->prev);
            colour_testedge2 = get_path_colour_next(test_edge2->prev->prev);
            best_colour = MAX(colour_testedge1, colour_testedge2);
            if(colour_testedge1 == best_colour)
                good_next[num_good_next++] = test_edge1;
            if(colour_testedge2 == best_colour)
                good_next[num_good_next++] = test_edge2;
        } else if(testedge1_is_best) {
            best_colour = get_path_colour_next(test_edge1->prev->prev);
            good_next[num_good_next++] = test_edge1;
        } else {
            best_colour = get_path_colour_next(test_edge2->prev->prev);
            good_next[num_good_next++] = test_edge2;
        }
    } else {
        if(testedge1_is_best && testedge2_is_best) {
            colour_testedge1 = get_path_colour_prev(test_edge1->next->next);
            colour_testedge2 = get_path_colour_prev(test_edge2->next->next);
            best_colour = MAX(colour_testedge1, colour_testedge2);
            if(colour_testedge1 == best_colour)
                good_prev[num_good_prev++] = test_edge1;
            if(colour_testedge2 == best_colour)
                good_prev[num_good_prev++] = test_edge2;
        } else if(testedge1_is_best) {
            best_colour = get_path_colour_prev(test_edge1->next->next);
            good_prev[num_good_prev++] = test_edge1;
        } else {
            best_colour = get_path_colour_prev(test_edge2->next->next);
            good_prev[num_good_prev++] = test_edge2;
        }
    }
    *can_edges_next = num_good_next;
    *can_edges_prev = num_good_prev;

    for(i = 0; i < num_good_next_tmp; i++) {
        colour_tmp = get_path_colour_next(good_next_tmp[i]->prev->prev);
        if(colour_tmp > best_colour)
            return 0; //Better reduction
        else if(colour_tmp == best_colour)
            good_next[num_good_next++] = good_next_tmp[i];
    }
    for(i = 0; i < num_good_prev_tmp; i++) {
        colour_tmp = get_path_colour_prev(good_prev_tmp[i]->next->next);
        if(colour_tmp > best_colour)
            return 0; //Better reduction
        else if(colour_tmp == best_colour)
            good_prev[num_good_prev++] = good_prev_tmp[i];
    }

    return test_canon_edge_short(num_good_next, num_good_prev,
                good_next, ngood_next, can_edges_next, good_prev, ngood_prev, can_edges_prev);

}

/****************************************************************************/

/*
 * Returns 1 if test_edge1 or test_edge2 have the best colour among the edges
 * from good_next_tmp and good_prev_tmp. It is assumed that test_edge{1,2}
 * can only be best if testedge{1,2}_is_best. It is assumed that at least one
 * of them = 1.
 *
 * If 1 is returned good_next[] contains a list of all "next"-edges which could
 * be canonical and good_prev[] of all such "prev"-edges.
 */
static int
is_best_third_colour_bent(EDGE *test_edge1, EDGE *test_edge2, int testedge1_is_best, int testedge2_is_best, int use_next,
        EDGE *good_next_tmp[], int num_good_next_tmp, EDGE *good_prev_tmp[], int num_good_prev_tmp,
        EDGE *good_next[], int *ngood_next, int *can_edges_next, EDGE *good_prev[], int *ngood_prev, int *can_edges_prev) {
    int i, best_colour, colour_tmp;
    int colour_testedge1, colour_testedge2;
    int num_good_next, num_good_prev;
    num_good_next = num_good_prev = 0;

    DEBUGASSERT(testedge1_is_best || testedge2_is_best);
    if(use_next) {
        if(testedge1_is_best && testedge2_is_best) {
            colour_testedge1 = get_path_colour_next(test_edge1->invers->next->next);
            colour_testedge2 = get_path_colour_prev(test_edge2->invers->prev->prev);
            best_colour = MAX(colour_testedge1, colour_testedge2);
            if(colour_testedge1 == best_colour)
                good_next[num_good_next++] = test_edge1;
            if(colour_testedge2 == best_colour)
                good_prev[num_good_prev++] = test_edge2;
        } else if(testedge1_is_best) {
            best_colour = get_path_colour_next(test_edge1->invers->next->next);
            good_next[num_good_next++] = test_edge1;
        } else {
            best_colour = get_path_colour_prev(test_edge2->invers->prev->prev);
            good_prev[num_good_prev++] = test_edge2;
        }
    } else {
        if(testedge1_is_best && testedge2_is_best) {
            colour_testedge1 = get_path_colour_prev(test_edge1->invers->prev->prev);
            colour_testedge2 = get_path_colour_next(test_edge2->invers->next->next);
            best_colour = MAX(colour_testedge1, colour_testedge2);
            if(colour_testedge1 == best_colour)
                good_prev[num_good_prev++] = test_edge1;
            if(colour_testedge2 == best_colour)
                good_next[num_good_next++] = test_edge2;
        } else if(testedge1_is_best) {
            best_colour = get_path_colour_prev(test_edge1->invers->prev->prev);
            good_prev[num_good_prev++] = test_edge1;
        } else {
            best_colour = get_path_colour_next(test_edge2->invers->next->next);
            good_next[num_good_next++] = test_edge2;
        }
    }
    *can_edges_next = num_good_next;
    *can_edges_prev = num_good_prev;

    for(i = 0; i < num_good_next_tmp; i++) {
        colour_tmp = get_path_colour_next(good_next_tmp[i]->invers->next->next);
        if(colour_tmp > best_colour)
            return 0; //Better reduction
        else if(colour_tmp == best_colour)
            good_next[num_good_next++] = good_next_tmp[i];
    }
    for(i = 0; i < num_good_prev_tmp; i++) {
        colour_tmp = get_path_colour_prev(good_prev_tmp[i]->invers->prev->prev);
        if(colour_tmp > best_colour)
            return 0; //Better reduction
        else if(colour_tmp == best_colour)
            good_prev[num_good_prev++] = good_prev_tmp[i];
    }

    return test_canon_edge_short(num_good_next, num_good_prev,
                good_next, ngood_next, can_edges_next, good_prev, ngood_prev, can_edges_prev);

}

/****************************************************************************/

/*
 * Checks whether or not a 'better' reduction than the given one exists.
 *
 * The canonical reduction is the shortest reduction and some additional combinatorial
 * invariants are used as well.
 *
 * If 1 is returned good_next[] contains a list of all "next"-edges which could
 * be canonical and good_prev[] of all such "prev"-edges.
 */
static int
is_best_L0_reduction(EDGE *test_edge1, EDGE *test_edge2, int use_next, EDGE *good_next[],
        int *ngood_next, int *can_edges_next, EDGE *good_prev[], int *ngood_prev, int *can_edges_prev) {

    /* Compute first and second colour */
    EDGE *good_next_tmp[MAX_EXTENSIONS_SINGLE];
    EDGE *good_prev_tmp[MAX_EXTENSIONS_SINGLE];
    int num_good_next_tmp, num_good_prev_tmp;
    num_good_next_tmp = num_good_prev_tmp = 0;

    int colour_testedge1, colour_testedge2;
    int testedge1_is_best, testedge2_is_best;
    int best_colour, best_colour_two;
    if(use_next) {
        colour_testedge1 = get_colour_next_5(test_edge1->next->next->invers->next);
        colour_testedge2 = get_colour_next_5(test_edge2->next->next->invers->next);
        best_colour = MAX(colour_testedge1, colour_testedge2);
    } else {
        colour_testedge1 = get_colour_prev_5(test_edge1->prev->prev->invers->prev);
        colour_testedge2 = get_colour_prev_5(test_edge2->prev->prev->invers->prev);
        best_colour = MAX(colour_testedge1, colour_testedge2);
    }
    testedge1_is_best = (colour_testedge1 == best_colour);
    testedge2_is_best = (colour_testedge2 == best_colour);
    if(testedge1_is_best && testedge2_is_best)
        best_colour_two = best_colour;
    else if(testedge1_is_best)
        best_colour_two = colour_testedge2;
    else
        best_colour_two = colour_testedge1;

    int colour_tmp, colour_tmp_other;

    if(nv < maxnv - 1) {
        num_straight_extensions = 0;
        num_straight_colour_2_L0_extensions = 0;
    }

    EDGE *e, *ee;
    unsigned char direction_bitvector;
    int i;
    for(i = 0; i < 12; i++) {
        e = ee = firstedge[degree_5_vertices[i]];
        //Not faster if using j < 5...
        do {
            if(e->start < e->end) { //Important: this only works for L0!
                if(has_L0_reduction(e, &direction_bitvector)) {
                    if((direction_bitvector & NEXT_BITVECTOR) != 0) {
                        if(!use_next || (e != test_edge1 && e != test_edge2)) {
                            colour_tmp = get_colour_next_5(e->next->next->invers->next);
                            if(colour_tmp > best_colour)
                                return 0;
                            //else if(colour_tmp == best_colour)
                            //    good_next_tmp[num_good_next_tmp++] = e;

                            colour_tmp_other = get_colour_next_5(e->invers->next->next->invers->next);
                            if(colour_tmp_other > best_colour)
                                return 0;
                            //else if(colour_tmp_other == best_colour)
                            //    good_next_tmp[num_good_next_tmp++] = e->invers;

                            /* Not rejected, so also check second colour now */
                            int has_best_colour = 0;
                            if(colour_tmp == best_colour) {
                                if(colour_tmp_other > best_colour_two)
                                    return 0;
                                else if(colour_tmp_other == best_colour_two) {
                                    good_next_tmp[num_good_next_tmp++] = e;
                                    has_best_colour = 1;
                                }
                            }
                            if(colour_tmp_other == best_colour) {
                                if(colour_tmp > best_colour_two)
                                    return 0;
                                else if(colour_tmp == best_colour_two) {
                                    good_next_tmp[num_good_next_tmp++] = e->invers;
                                    has_best_colour = 1;
                                }
                            }

                            if(has_best_colour && nv < maxnv - 1 && num_straight_colour_2_L0_extensions < MAX_PREV_EXTENSIONS - 1) {
                                add_straight_colour_2_L0_extension_to_list(e->next->next->invers, e->invers->next->next->invers);
                            }
                        }
                    }
                    if((direction_bitvector & PREV_BITVECTOR) != 0) {
                        if(use_next || (e != test_edge1 && e != test_edge2)) {
                            colour_tmp = get_colour_prev_5(e->prev->prev->invers->prev);
                            if(colour_tmp > best_colour)
                                return 0;
                            //else if(colour_tmp == best_colour)
                            //    good_prev_tmp[num_good_prev_tmp++] = e;

                            colour_tmp_other = get_colour_prev_5(e->invers->prev->prev->invers->prev);
                            if(colour_tmp_other > best_colour)
                                return 0;
                            //else if(colour_tmp_other == best_colour)
                            //    good_prev_tmp[num_good_prev_tmp++] = e->invers;

                            /* Not rejected, so also check second colour now */
                            int has_best_colour = 0;
                            if(colour_tmp == best_colour) {
                                if(colour_tmp_other > best_colour_two)
                                    return 0;
                                else if(colour_tmp_other == best_colour_two) {
                                    good_prev_tmp[num_good_prev_tmp++] = e;
                                    has_best_colour = 1;
                                }
                            }
                            if(colour_tmp_other == best_colour) {
                                if(colour_tmp > best_colour_two)
                                    return 0;
                                else if(colour_tmp == best_colour_two) {
                                    good_prev_tmp[num_good_prev_tmp++] = e->invers;
                                    has_best_colour = 1;
                                }
                            }

                            //Is slightly better if also adding prev even if there was a next
                            if(has_best_colour && nv < maxnv - 1 && num_straight_colour_2_L0_extensions < MAX_PREV_EXTENSIONS - 1) {
                                    //&& (direction_bitvector & NEXT_BITVECTOR) == 0) { //If already added in next direction, prev direction is not much different...
                                add_straight_colour_2_L0_extension_to_list(e->prev->prev->invers, e->invers->prev->prev->invers);
                            }
                        }
                    }
                    if(nv < maxnv - 1 && num_straight_extensions < MAX_PREV_EXTENSIONS - 1)
                        add_straight_extension_to_list(e, 2, (direction_bitvector & NEXT_BITVECTOR) != 0);
                }
            }
            e = e->next;
        } while(e != ee);
    }

    if(nv < maxnv - 1) {
        best_straight_colour_1 = best_colour;
        best_straight_colour_2 = best_colour_two;
    }

    /* Compute third colour */
    return is_best_third_colour(test_edge1, test_edge2, testedge1_is_best, testedge2_is_best,
                use_next, good_next_tmp, num_good_next_tmp, good_prev_tmp, num_good_prev_tmp,
                good_next, ngood_next, can_edges_next, good_prev, ngood_prev, can_edges_prev);
}

/****************************************************************************/

/*
 * Returns 1 if there is an L0 or L1 reduction starting from startedge, else returns 0.
 * 
 * Important: startedge->start is assumed to have degree 5.
 */
static int
is_L0_or_L1_reduction(EDGE *startedge) {
    EDGE *e = startedge->invers;
    if(degree[e->start] == 6) {
        e = e->next->next->next->invers;
        if(degree[e->start] == 6)
            return 0;
    }
    return (degree[startedge->prev->prev->end] == 6 && degree[e->prev->prev->end] == 6)
            || (degree[startedge->next->next->end] == 6 && degree[e->next->next->end] == 6);
}

/****************************************************************************/

/*
 * Returns 1 if there is a bent zero reduction starting from startedge with
 * direction use_next, else returns 0.
 * If 1 is returned, endedge contains the last edge of the reduction.
 */
/*
 * Remark 1: Can never occur that faces are the same, so no need to mark.
 * This is because there is only one possible 4-cycle.
 * Remark 2: Searching in one direction is sufficient, since endedge is same 
 * but in other direction. Using direction prev here.
 */
static int
has_bent_zero_reduction(EDGE *startedge, EDGE **endedge) {
    EDGE *e = startedge;
    if(degree[e->end] == 5)
        return 0;
    e = e->next->next;
    if(degree[e->end] == 5)
        return 0;

    e = e->invers->prev->prev;
    if(degree[e->end] == 6)
        return 0;

    e = e->invers->next->next;
    if(degree[e->end] == 5)
        return 0;
    else {
        //MARK_B00_NEXT(e->label);
        *endedge = e;
        return 1;
    }

}

/****************************************************************************/

/*
 * Returns 1 if there is a bent zero reduction to a smaller IPR fullerene 
 * starting from startedge with direction use_next, else returns 0.
 * If 1 is returned, endedge contains the last edge of the reduction.
 * 
 * Important: it is assumed that the fullerene is IPR.
 */
/*
 * Remark 1: Can never occur that faces are the same, so no need to mark.
 * Remark 2: Searching in one direction is sufficient, since endedge is same 
 * but in other direction. Using direction prev here.
 */
static int
has_bent_zero_reduction_ipr(EDGE *startedge, EDGE **endedge) {
    DEBUGASSERT(degree[startedge->end] == 6);
    
    int a, b;
    EDGE *ea, *eb;
    
    EDGE *e = startedge;
    e = e->next->next;
    DEBUGASSERT(degree[e->end] != 5);

    e = e->invers->prev->prev;
    if(degree[e->end] == 6)
        return 0;

    e = e->invers->next->next;
    DEBUGASSERT(degree[e->end] != 5);

    b = e->end;
    eb = e->invers->next;

    a = startedge->end;
    ea = startedge->invers->prev;

    /* Test if reduced graph is IPR */
    //No need to check v's and w's here, since there can be no conflicts after reduction
    if(are_adjacent(a, b)) return 0;
    if(degree[ea->prev->end] == 5 || degree[ea->prev->prev->end] == 5
            || degree[ea->prev->prev->prev->end] == 5) return 0;
    if(degree[eb->next->end] == 5 || degree[eb->next->next->end] == 5
            || degree[eb->next->next->next->end] == 5) return 0;

/*
    if(use_next)
        MARK_B00_PREV(e->label);
    else
        MARK_B00_NEXT(e->label);
*/
    *endedge = e;
    return 1;    

}

/****************************************************************************/

/*
 * Returns 1 if there is a B10 reduction starting from startedge with
 * direction use_next, else returns 0.
 * 
 * Important: it is assumed that the fullerene is IPR.
 * If a fullerene is IPR, then there is no need to mark, since no 5-cycles
 * can lead to a valid B10 reduction.
 * If the fullerene is not IPR, then it has a valid L0, L1, or B00 reduction,
 * so then this method would not be called.
 */
static int
has_B10_reduction(EDGE *startedge, int use_next) {
    EDGE *e = startedge;
    if(degree[e->end] == 5)
        return 0;
    if(use_next) {
        e = e->prev->prev;
        if(degree[e->end] == 5)
            return 0;

        e = e->invers->next->next->next;
        if(degree[e->end] == 5)
            return 0;

        e = e->invers->next->next;
        if(degree[e->end] == 6)
            return 0;

        e = e->invers->prev->prev;
        if(degree[e->end] == 5)
            return 0;
        else {
            return 1;
        }
    } else {
        e = e->next->next;
        if(degree[e->end] == 5)
            return 0;

        e = e->invers->next->next->next;
        if(degree[e->end] == 5)
            return 0;

        e = e->invers->prev->prev;
        if(degree[e->end] == 6)
            return 0;

        e = e->invers->next->next;
        if(degree[e->end] == 5)
            return 0;
        else {
            return 1;
        }
    }

}

/****************************************************************************/

/*
 * Returns 1 if there is a B10 reduction starting from startedge with
 * direction use_next, else returns 0.
 */
/*
 * Remark: No need to mark, since in case of IPR there can't be any valid
 * 5-cycles which would be a B10 reduction.
 */
static int
has_B10_reduction_ipr(EDGE *startedge, int use_next) {
    DEBUGASSERT(degree[startedge->end] == 6);
    
    int a, b;
    EDGE *ea, *eb;    
    
    EDGE *e = startedge;
    if(use_next) {
        e = e->prev->prev;
        DEBUGASSERT(degree[e->end] != 5);

        e = e->invers->next->next->next;
        if(degree[e->end] == 5)
            return 0;

        e = e->invers->next->next;
        if(degree[e->end] == 6)
            return 0;

        e = e->invers->prev->prev;
        DEBUGASSERT(degree[e->end] != 5);
        
        a = e->end;
        ea = e->invers->prev;
        
        b = startedge->end;
        eb = startedge->invers->next;
        
    } else {
        e = e->next->next;
        DEBUGASSERT(degree[e->end] != 5);

        e = e->invers->next->next->next;
        if(degree[e->end] == 5)
            return 0;

        e = e->invers->prev->prev;
        if(degree[e->end] == 6)
            return 0;

        e = e->invers->next->next;
        DEBUGASSERT(degree[e->end] != 5);
        
        b = e->end;
        eb = e->invers->next;
        
        a = startedge->end;
        ea = startedge->invers->prev;        
    }

    /* Test if reduced graph is IPR */
    //No need to check v's and w's here, since there can be no conflicts after reduction
    if(are_adjacent(a, b)) return 0;
    if(degree[ea->prev->end] == 5 || degree[ea->prev->prev->end] == 5
            || degree[ea->prev->prev->prev->end] == 5) return 0;
    if(degree[eb->next->end] == 5 || degree[eb->next->next->end] == 5
            || degree[eb->next->next->next->end] == 5) return 0;    
    
    return 1;

}

/**************************************************************************/

static void
search_bent_zero_reductions_ipr() {
    //Only called for extensions which add at least 4 new vertices
    DEBUGASSERT(nv <= maxnv - 4);
    DEBUGASSERT(num_bent_zero_extensions == 0 && MAX_PREV_EXTENSIONS > 0);

    EDGE *e, *endedge;
    int i, j;
    for(i = 0; i < 12; i++) {
        e = firstedge[degree_5_vertices[i]];
        for(j = 0; j < 5; j++) {
            if(has_bent_zero_reduction_ipr(e, &endedge)) {
                add_bent_zero_extension_to_list_ipr(e, 0);

                if(num_bent_zero_extensions == MAX_PREV_EXTENSIONS)
                    return;
            }
            e = e->next;
        }
    }
}

/****************************************************************************/

/*
 * Checks whether or not a 'better' reduction than the given one exists.
 *
 * There may be no L0 or L1 present and some additional combinatorial invariants
 * are used as well.
 *
 * If 1 is returned good_next[] contains a list of all "next"-edges which could
 * be canonical and good_prev[] of all such "prev"-edges.
 */
static int
is_best_bent_zero_reduction(EDGE *test_edge1, EDGE *test_edge2, int use_next,
            EDGE *good_next[], int *ngood_next, int *can_edges_next, EDGE *good_prev[], int *ngood_prev, int *can_edges_prev) {
    
    DEBUGASSERT(!fulleriprswitch);

    /* First check if there are no L0 or L1 reductions */
    EDGE *e;
    int i, j;
    for(i = 0; i < 12; i++) {
        e = firstedge[degree_5_vertices[i]];
        for(j = 0; j < 5; j++) {
            if(is_L0_or_L1_reduction(e))
                return 0;
            e = e->next;
        }
    }

    //Remark: only saving reductions in case of IPR at the moment
    //Warning: When also saving in case of nonipr, nv has to be < maxnv - 1!
    if(nv < maxnv - 1) {
        num_bent_zero_extensions = 0;
    }    
    
    /* Compute first colour */
    EDGE *good_next_tmp[MAX_EXTENSIONS_SINGLE];
    EDGE *good_prev_tmp[MAX_EXTENSIONS_SINGLE];
    int num_good_next_tmp, num_good_prev_tmp;
    num_good_next_tmp = num_good_prev_tmp = 0;

    int colour_testedge1, colour_testedge2;
    int best_colour, best_colour_two;

    //RESETMARKS_B00_NEXT;
    //RESETMARKS_B00_PREV;
    EDGE *forbidden_edge;
    if(use_next) {
        colour_testedge1 = get_colour_next_5(test_edge1->invers->next);
        colour_testedge2 = get_colour_prev_5(test_edge2->invers->prev);
        best_colour = MAX(colour_testedge1, colour_testedge2);

        //To make sure test_edge1 and test_edge2 don't appear twice in the same list
        //MARK_B00_NEXT(test_edge1->label);
        //MARK_B00_PREV(test_edge2->label);
        forbidden_edge = test_edge2;
    } else {
        colour_testedge2 = get_colour_next_5(test_edge2->invers->next);
        colour_testedge1 = get_colour_prev_5(test_edge1->invers->prev);
        best_colour = MAX(colour_testedge1, colour_testedge2);

        //MARK_B00_PREV(test_edge1->label);
        //MARK_B00_NEXT(test_edge2->label);
        forbidden_edge = test_edge1;
    }

    int testedge1_is_best, testedge2_is_best;
    testedge1_is_best = (colour_testedge1 == best_colour);
    testedge2_is_best = (colour_testedge2 == best_colour);
    if(testedge1_is_best && testedge2_is_best) {
        best_colour_two = best_colour;
    } else if(testedge1_is_best) {
        best_colour_two = colour_testedge2;
    } else {
        best_colour_two = colour_testedge1;
    }

    //Searching in one direction is sufficient, since endedge is same but in other direction
    //Searching in direction prev here.
    int colour_tmp, colour_tmp_other;
    EDGE *endedge;
    for(i = 0; i < 12; i++) {
        e = firstedge[degree_5_vertices[i]];
        for(j = 0; j < 5; j++) {
            if(e != forbidden_edge) {
                //if(has_bent_zero_reduction(e, k, &endedge)) {
                if(has_bent_zero_reduction(e, &endedge)) {
                    colour_tmp = get_colour_next_5(endedge->invers->next);
                    if(colour_tmp > best_colour)
                        return 0;
                    //if(colour_tmp == best_colour)
                    //    good_next[num_good_next++] = endedge;

                    colour_tmp_other = get_colour_prev_5(e->invers->prev);
                    if(colour_tmp_other > best_colour)
                        return 0;
                    //if(colour_tmp == best_colour)
                    //    good_prev[num_good_prev++] = e;

                    if(colour_tmp == best_colour) {
                        if(colour_tmp_other > best_colour_two)
                            return 0;
                        else if(colour_tmp_other == best_colour_two) {
                            good_next_tmp[num_good_next_tmp++] = endedge;
                        }
                    }
                    if(colour_tmp_other == best_colour) {
                        if(colour_tmp > best_colour_two)
                            return 0;
                        else if(colour_tmp == best_colour_two) {
                            good_prev_tmp[num_good_prev_tmp++] = e;
                        }
                    }
                    //if(fulleriprswitch && nv < maxnv - 2 && num_bent_zero_extensions < MAX_PREV_EXTENSIONS - 1)
                    //    add_bent_zero_extension_to_list_ipr(e, 0);
                    if(nv < maxnv - 1 && num_bent_zero_extensions < MAX_PREV_EXTENSIONS - 1)
                        add_bent_zero_extension_to_list_ipr(e, 0);                    
                }
            }
            e = e->next;
        }
    }

    DEBUGASSERT(num_good_next_tmp <= MAX_EXTENSIONS_SINGLE && num_good_prev_tmp <= MAX_EXTENSIONS_SINGLE);

    /* Compute third colour */
    return is_best_third_colour_bent(test_edge1, test_edge2, testedge1_is_best, testedge2_is_best,
                use_next, good_next_tmp, num_good_next_tmp, good_prev_tmp, num_good_prev_tmp,
                good_next, ngood_next, can_edges_next, good_prev, ngood_prev, can_edges_prev);
}

/****************************************************************************/

/*
 * Checks whether or not a 'better' IPR reduction than the given one exists.
 *
 * There may be no L0 or L1 present and some additional combinatorial invariants
 * are used as well.
 *
 * If 1 is returned good_next[] contains a list of all "next"-edges which could
 * be canonical and good_prev[] of all such "prev"-edges.
 */
//Remark: uses different colour1 than is_best_bent_zero_reduction()
static int
is_best_bent_zero_reduction_ipr(EDGE *test_edge1, EDGE *test_edge2, int use_next,
            EDGE *good_next[], int *ngood_next, int *can_edges_next, EDGE *good_prev[], int *ngood_prev, int *can_edges_prev) {
    
    DEBUGASSERT(fulleriprswitch);
    
    /* First check if there are no L0 or L1 reductions */
    EDGE *e, *tmp_edge;
    int i, j;
    unsigned char tmp_bv;    
    for(i = 0; i < 12; i++) {
        e = firstedge[degree_5_vertices[i]];
        for(j = 0; j < 5; j++) {
            //Remark: could use separate method without bitvector, but won't help much
            if(has_short_straight_reduction_L1_ipr(e, 3, &tmp_edge, &tmp_bv)) {
                previous_rejector[nv] = e;
                return 0;
            }
            e = e->next;
        }
    }

    //Remark: only saving reductions in case of IPR at the moment
    //Warning: When also saving in case of nonipr, nv has to be < maxnv - 1!
    if(nv < maxnv - 2) {
        num_bent_zero_extensions = 0;
    }    

    /* Compute first colour */
    EDGE *good_next_tmp[MAX_EXTENSIONS_SINGLE];
    EDGE *good_prev_tmp[MAX_EXTENSIONS_SINGLE];
    int num_good_next_tmp, num_good_prev_tmp;
    num_good_next_tmp = num_good_prev_tmp = 0;

    int colour_testedge1, colour_testedge2;
    int best_colour, best_colour_two;

    //RESETMARKS_B00_NEXT;
    //RESETMARKS_B00_PREV;
    EDGE *forbidden_edge;
    if(use_next) {
        //colour_testedge1 = get_colour_next_5(test_edge1->invers->next);
        //colour_testedge2 = get_colour_prev_5(test_edge2->invers->prev);
        colour_testedge1 = get_colour_next_3(test_edge1->invers->prev->prev->invers->prev->prev);
        colour_testedge2 = get_colour_prev_3(test_edge2->invers->next->next->invers->next->next);        
        best_colour = MAX(colour_testedge1, colour_testedge2);

        //To make sure test_edge1 and test_edge2 don't appear twice in the same list
        //MARK_B00_NEXT(test_edge1->label);
        //MARK_B00_PREV(test_edge2->label);
        forbidden_edge = test_edge2;
    } else {
        //colour_testedge2 = get_colour_next_5(test_edge2->invers->next);
        //colour_testedge1 = get_colour_prev_5(test_edge1->invers->prev);
        colour_testedge2 = get_colour_next_3(test_edge2->invers->prev->prev->invers->prev->prev);
        colour_testedge1 = get_colour_prev_3(test_edge1->invers->next->next->invers->next->next);        
        best_colour = MAX(colour_testedge1, colour_testedge2);

        //MARK_B00_PREV(test_edge1->label);
        //MARK_B00_NEXT(test_edge2->label);
        forbidden_edge = test_edge1;
    }

    int testedge1_is_best, testedge2_is_best;
    testedge1_is_best = (colour_testedge1 == best_colour);
    testedge2_is_best = (colour_testedge2 == best_colour);
    if(testedge1_is_best && testedge2_is_best) {
        best_colour_two = best_colour;
    } else if(testedge1_is_best) {
        best_colour_two = colour_testedge2;
    } else {
        best_colour_two = colour_testedge1;
    }

    //Searching in one direction is sufficient, since endedge is same but in other direction
    //Searching in direction prev here.
    int colour_tmp, colour_tmp_other;
    EDGE *endedge;
    for(i = 0; i < 12; i++) {
        e = firstedge[degree_5_vertices[i]];
        for(j = 0; j < 5; j++) {
            if(e != forbidden_edge) {
                //if(has_bent_zero_reduction(e, k, &endedge)) {
                if(has_bent_zero_reduction_ipr(e, &endedge)) {
                    //colour_tmp = get_colour_next_5(endedge->invers->next);
                    colour_tmp = get_colour_next_3(endedge->invers->prev->prev->invers->prev->prev);
                    if(colour_tmp > best_colour)
                        return 0;
                    //if(colour_tmp == best_colour)
                    //    good_next[num_good_next++] = endedge;

                    //colour_tmp_other = get_colour_prev_5(e->invers->prev);
                    colour_tmp_other = get_colour_prev_3(e->invers->next->next->invers->next->next);
                    if(colour_tmp_other > best_colour)
                        return 0;
                    //if(colour_tmp == best_colour)
                    //    good_prev[num_good_prev++] = e;

                    if(colour_tmp == best_colour) {
                        if(colour_tmp_other > best_colour_two)
                            return 0;
                        else if(colour_tmp_other == best_colour_two) {
                            good_next_tmp[num_good_next_tmp++] = endedge;
                        }
                    }
                    if(colour_tmp_other == best_colour) {
                        if(colour_tmp > best_colour_two)
                            return 0;
                        else if(colour_tmp == best_colour_two) {
                            good_prev_tmp[num_good_prev_tmp++] = e;
                        }
                    }
                    if(nv < maxnv - 2 && num_bent_zero_extensions < MAX_PREV_EXTENSIONS - 1)
                        add_bent_zero_extension_to_list_ipr(e, 0);
                }
            }
            e = e->next;
        }
    }

    DEBUGASSERT(num_good_next_tmp <= MAX_EXTENSIONS_SINGLE && num_good_prev_tmp <= MAX_EXTENSIONS_SINGLE);

    /* Compute third colour */
    return is_best_third_colour_bent(test_edge1, test_edge2, testedge1_is_best, testedge2_is_best,
                use_next, good_next_tmp, num_good_next_tmp, good_prev_tmp, num_good_prev_tmp,
                good_next, ngood_next, can_edges_next, good_prev, ngood_prev, can_edges_prev);
}

/****************************************************************************/

/*
 * Short reductions have higher priority.
 *
 * Returns 0 if there is no reduction starting from startedge which is at least
 * as good as the cur_best reduction.
 * Returns 1 if the best reduction from startedge is as good as the cur_best.
 * Returns 2 if there is a reduction from startedge which is better than cur_best.
 *
 * Remark: is is assumed that startedge->end has degree 6
 */
static int
has_short_bent_reduction(EDGE *startedge, int bent_position, int bent_length, int use_next) {
    DEBUGASSERT(degree[startedge->end] == 6);
    DEBUGASSERT(bent_length > 0);
    DEBUGASSERT(bent_length <= 2*bent_position);

    //Mark part before bent with MARKS_V and part after bent with MARKS_V2
    RESETMARKS_V;
    MARK_V(startedge->start);
    MARK_V(startedge->end);
    if(use_next) {
        MARK_V(startedge->next->end);
        startedge = startedge->prev->prev;
    } else {
        MARK_V(startedge->prev->end);
        startedge = startedge->next->next;
    }

    int bent_of_same_length_found = 0;
    int i, j, abort;
    EDGE *e;
    for(i = 0; i <= bent_length; i++) {
        if(degree[startedge->end] == 5)
            return bent_of_same_length_found;

        //Remark: actually marks only have to be checked for sufficiently long reductions
        if(ISMARKED_V(startedge->end))
            return bent_of_same_length_found;
        else
            MARK_V(startedge->end);

        if(ISMARKED_V(startedge->prev->end))
            return bent_of_same_length_found;
        else
            MARK_V(startedge->prev->end);

        if(ISMARKED_V(startedge->next->end))
            return bent_of_same_length_found;
        else
            MARK_V(startedge->next->end);

        //Make bent
        abort = 0;
        RESETMARKS_V2;
        if(use_next) {
            e = startedge->invers->next->next;
            if(ISMARKED_V(e->next->next->end))
                abort = 1;
            else
                MARK_V2(e->next->next->end);
        } else {
            e = startedge->invers->prev->prev;
            if(ISMARKED_V(e->prev->prev->end))
                abort = 1;
            else
                MARK_V2(e->prev->prev->end);
        }

        //Go till max length or till a degree 5 vertex is met
        //Only search reductions Bij for which i >= j
        int max_length = MIN(bent_length, 2*i);
        for(j = i; j < max_length && !abort; j++) {
            if(ISMARKED_V(e->end) || ISMARKED_V2(e->end)) {
                abort = 1;
                break;
            } else
                MARK_V2(e->end);

            if(j > i) {
                if(ISMARKED_V(e->prev->end) || ISMARKED_V2(e->prev->end))
                    abort = 1;
                else
                    MARK_V2(e->prev->end);
                if(ISMARKED_V(e->next->end) || ISMARKED_V2(e->next->end))
                    abort = 1;
                else
                    MARK_V2(e->next->end);
            } else {
                if(use_next) {
                    if(ISMARKED_V(e->next->end) || ISMARKED_V2(e->next->end))
                        abort = 1;
                    else
                        MARK_V2(e->next->end);
                } else {
                    if(ISMARKED_V(e->prev->end) || ISMARKED_V2(e->prev->end))
                        abort = 1;
                    else
                        MARK_V2(e->prev->end);
                }
            }

            if(degree[e->end] == 5)
                break;
            e = e->invers->next->next->next;
        }

        //i.e. is valid reduction
        if(!abort && degree[e->end] == 5) {
            int end_vertex;
            if(use_next)
                end_vertex = e->invers->prev->prev->end;
            else
                end_vertex = e->invers->next->next->end;
            if(degree[end_vertex] == 6 && !ISMARKED_V(end_vertex) && !ISMARKED_V2(end_vertex)) {

                if(j < bent_length)
                    return 2;
                else {
                    DEBUGASSERT(j == bent_length);
                    //If same length: then the one with the longest straight path is canonical
                    if(i > bent_position)
                        return 2;
                    else if(i == bent_position)
                        bent_of_same_length_found = 1;
                }
            }
        }

        startedge = startedge->invers->next->next->next;

    }

    return bent_of_same_length_found;
}

/****************************************************************************/

/*
 * Short reductions have higher priority.
 *
 * Returns 0 if there is no reduction starting from startedge which is at least
 * as good as the cur_best reduction.
 * Returns 1 if the best reduction from startedge is as good as the cur_best.
 * Returns 2 if there is a reduction from startedge which is better than cur_best.
 *
 * Remark: is is assumed that startedge->end has degree 6
 */
static int
has_short_bent_reduction_ipr(EDGE *startedge, int bent_position, int bent_length, int use_next) {
    DEBUGASSERT(degree[startedge->end] == 6);
    DEBUGASSERT(bent_length > 0);
    DEBUGASSERT(bent_length <= 2*bent_position);
    
    int a, b, bent_neighbour;
    EDGE *ea, *eb;
    int v1[MAXN/2],w1[MAXN/2],len1;
    int v2[MAXN/2],w2[MAXN/2],len2;    
    
    len1 = 0;
    
    //Mark part before bent with MARKS_V and part after bent with MARKS_V2
    RESETMARKS_V;
    MARK_V(startedge->start);
    MARK_V(startedge->end);
    if(use_next) {
        b = startedge->end;
        eb = startedge->invers->next;

        MARK_V(startedge->next->end);
        startedge = startedge->prev->prev;
    } else {
        a = startedge->end;
        ea = startedge->invers->prev;
        
        MARK_V(startedge->prev->end);
        startedge = startedge->next->next;
    }

    int bent_of_same_length_found = 0;
    int i, j, abort;
    EDGE *e;
    for(i = 0; i <= bent_length; i++) {
        if(degree[startedge->end] == 5)
            return bent_of_same_length_found;

        if(ISMARKED_V(startedge->end))
            return bent_of_same_length_found;
        else
            MARK_V(startedge->end);

        if(ISMARKED_V(startedge->prev->end))
            return bent_of_same_length_found;
        else
            MARK_V(startedge->prev->end);

        if(ISMARKED_V(startedge->next->end))
            return bent_of_same_length_found;
        else
            MARK_V(startedge->next->end);
        
        if(i > 0) {
            if(use_next) {
                v1[len1] = startedge->prev->end;
                w1[len1] = startedge->next->end;
            } else {
                w1[len1] = startedge->prev->end;
                v1[len1] = startedge->next->end;                
            }
            len1++;
        }

        //Make bent
        abort = len2 = 0;
        RESETMARKS_V2;
        if(use_next) {
            e = startedge->invers->next->next;
            if(ISMARKED_V(e->next->next->end))
                abort = 1;
            else
                MARK_V2(e->next->next->end);
            bent_neighbour = e->next->next->end;
        } else {
            e = startedge->invers->prev->prev;
            if(ISMARKED_V(e->prev->prev->end))
                abort = 1;
            else
                MARK_V2(e->prev->prev->end);
            bent_neighbour = e->prev->prev->end;
        }

        //Go till max length or till a degree 5 vertex is met
        //Only search reductions Bij for which i >= j
        int max_length = MIN(bent_length, 2*i);
        for(j = i; j < max_length && !abort; j++) {
            if(ISMARKED_V(e->end) || ISMARKED_V2(e->end)) {
                abort = 1;
                break;
            } else
                MARK_V2(e->end);

            if(j > i) {
                if(ISMARKED_V(e->prev->end) || ISMARKED_V2(e->prev->end))
                    abort = 1;
                else
                    MARK_V2(e->prev->end);
                if(ISMARKED_V(e->next->end) || ISMARKED_V2(e->next->end))
                    abort = 1;
                else
                    MARK_V2(e->next->end);
            } else {
                if(use_next) {
                    if(ISMARKED_V(e->next->end) || ISMARKED_V2(e->next->end))
                        abort = 1;
                    else
                        MARK_V2(e->next->end);
                } else {
                    if(ISMARKED_V(e->prev->end) || ISMARKED_V2(e->prev->end))
                        abort = 1;
                    else
                        MARK_V2(e->prev->end);
                }
            }

            if(degree[e->end] == 5)
                break;

            if(use_next) {
                v2[len2] = e->prev->end;
                w2[len2] = e->next->end;
            } else {
                w2[len2] = e->prev->end;
                v2[len2] = e->next->end;                
            }
            len2++;            
            
            e = e->invers->next->next->next;
        }

        //i.e. is valid reduction
        if(!abort && degree[e->end] == 5) {
            int end_vertex;
            if(use_next)
                end_vertex = e->invers->prev->prev->end;
            else
                end_vertex = e->invers->next->next->end;
            DEBUGASSERT(degree[end_vertex] == 6);
            if(!ISMARKED_V(end_vertex) && !ISMARKED_V2(end_vertex)) {
                //Check if is valid reduction to smaller ipr
                
                if(use_next) {
                    a = end_vertex;
                    ea = e->invers->prev->prev->invers->prev;
                } else {
                    b = end_vertex;
                    eb = e->invers->next->next->invers->next;
                }

                int abort_ipr = 0;
                if(are_adjacent(a, b)) abort_ipr = 1;
                if(degree[ea->prev->end] == 5 || degree[ea->prev->prev->end] == 5
                        || degree[ea->prev->prev->prev->end] == 5) abort_ipr = 1;
                if(degree[eb->next->end] == 5 || degree[eb->next->next->end] == 5
                        || degree[eb->next->next->next->end] == 5) abort_ipr = 1;   
                
                //if j <= 1, no need to check v's and w's
                if(!abort_ipr && j > 1) {
                    int k;
                    for(k = 0; !abort_ipr && k < len1; k++)
                        if(degree[v1[k]] == 5 && degree[w1[k]] == 5)
                            abort_ipr = 1;
                    for(k = 1; !abort_ipr && k < len1; k++)
                        if(degree[v1[k-1]] == 5 && degree[w1[k]] == 5)
                            abort_ipr = 1;
                    
                    if(!abort_ipr)
                        abort_ipr = (degree[v1[len1 - 1]] == 5 && degree[bent_neighbour] == 5);
                    
                    for(k = 0; !abort_ipr && k < len2; k++)
                        if(degree[v2[k]] == 5 && degree[w2[k]] == 5)
                            abort_ipr = 1;
                    for(k = 1; !abort_ipr && k < len2; k++)
                        if(degree[w2[k-1]] == 5 && degree[v2[k]] == 5) //w and v swapped!
                            abort_ipr = 1;                    
                    
                }

                if(!abort_ipr) {
                    if(j < bent_length)
                        return 2;
                    else {
                        DEBUGASSERT(j == bent_length);
                        //If same length: then the one with the longest straight path is canonical
                        if(i > bent_position)
                            return 2;
                        else if(i == bent_position)
                            bent_of_same_length_found = 1;
                    }
                }
            }
        }

        startedge = startedge->invers->next->next->next;

    }

    return bent_of_same_length_found;
}

/****************************************************************************/

/*
 * Returns 1 if the graph has an Lx reduction with x <= max_length starting
 * from e, else returns 0.
 */
static int
hasStraight(EDGE *e, int max_length)
/* Test if there is an Straight operation starting at edge e.
 * We know there are no adjacent 5s and that e->start has degree 5.
 */
{
    int a,b,c,d;
    int length = 0;

    RESETMARKS_V;

    a = e->prev->prev->end;
    b = e->next->next->end;
    MARK_V(e->start);
    MARK_V(e->prev->end);
    MARK_V(e->prev->prev->end);
    MARK_V(e->next->end);
    MARK_V(e->next->next->end);

    while (length < max_length && degree[e->end] != 5)
    {
	e = e->invers->next->next->next;
	if (ISMARKED_V(e->start)) return 0; else MARK_V(e->start);
        if (ISMARKED_V(e->prev->end)) return 0; else MARK_V(e->prev->end);
        if (ISMARKED_V(e->next->end)) return 0; else MARK_V(e->next->end);

        length++;
    }

    if(degree[e->end] == 5) {
        e = e->invers;
        if(ISMARKED_V(e->start)) return 0;
        if(ISMARKED_V(e->prev->prev->end)) return 0;
        if(ISMARKED_V(e->next->next->end)) return 0;

        c = e->prev->prev->end;
        d = e->next->next->end;

        return (degree[a] == 6 && degree[c] == 6)
                || (degree[b] == 6 && degree[d] == 6);
    } else
        return 0;
}

/****************************************************************************/

/*
 * Returns 1 if the graph contains an Lx reduction with x <= max_length,
 * else returns 0.
 */
static int
has_straight(int max_length) {
    EDGE *e, *ee;
    int i;
    for (i = 0; i < 12; ++i) {
        e = ee = firstedge[degree_5_vertices[i]];
        do {
            if (hasStraight(e, max_length)) {
                return 1;
            }
            e = e->next;
        } while (e != ee);
    }
    return 0;
}

/****************************************************************************/
/*
 * Returns 1 if the graph has an Lx reduction with x <= max_length starting
 * from e, else returns 0.
 */
static int
hasStraightIPR(EDGE *e, int max_length)
/* Test if there is an Straight operation starting at edge e.
 * We know there are no adjacent 5s and that e->start has degree 5.
 */ {
    int a, b, c, d, ok, j;
    EDGE *ea, *eb, *ec, *ed;
    int v[max_length + 3], w[max_length + 3], len;

    RESETMARKS_V;

    v[1] = e->prev->end;
    a = v[0] = e->prev->prev->end;
    w[1] = e->next->end;
    b = w[0] = e->next->next->end;
    MARK_V(e->start);
    MARK_V(e->prev->end);
    MARK_V(e->prev->prev->end);
    MARK_V(e->next->end);
    MARK_V(e->next->next->end);

    ea = e->prev->invers->next->invers;
    eb = e->next->invers->prev->invers;

    len = 1;
    while(len <= max_length && degree[e->end] != 5) {
        e = e->invers->next->next->next;
        if(ISMARKED_V(e->start)) return 0;
        else MARK_V(e->start);
        if(ISMARKED_V(e->prev->end)) return 0;
        else MARK_V(e->prev->end);
        if(ISMARKED_V(e->next->end)) return 0;
        else MARK_V(e->next->end);
        ++len;
        v[len] = e->prev->end;
        w[len] = e->next->end;
    }

    if(degree[e->end] == 5) {
        e = e->invers;
        if(ISMARKED_V(e->start)) return 0;
        if(ISMARKED_V(e->prev->prev->end)) return 0;
        if(ISMARKED_V(e->next->next->end)) return 0;

        ++len;
        c = w[len] = e->prev->prev->end;
        d = v[len] = e->next->next->end;
        ec = e->prev->invers->next->invers;
        ed = e->next->invers->prev->invers;
        
        DEBUGASSERT(len <= max_length + 2);

        if(degree[a] == 6 && degree[c] == 6) {
            ok = 0;
            //Has to go till 3!
            for(j = 0; j < 3; ++j) {
                ea = ea->prev;
                if(ea->end == c || degree[ea->end] == 5) break;
            }
            ok += (j == 3);
            for(j = 0; j < 3; ++j) {
                ec = ec->prev;
                if(ec->end == a || degree[ec->end] == 5) break;
            }
            ok += (j == 3);

            for(j = 0; j < len; ++j)
                if(degree[w[j]] == 5 && degree[v[j + 1]] == 5) break;
            ok += (j == len);

            for(j = 1; j < len; ++j)
                if(degree[w[j]] == 5 && degree[v[j]] == 5) break;
            ok += (j == len);

            if(ok == 4) return 1;
        }

        if(degree[b] == 6 && degree[d] == 6) {
            ok = 0;
            for(j = 0; j < 3; ++j) {
                eb = eb->next;
                if(eb->end == d || degree[eb->end] == 5) break;
            }
            ok += (j == 3);
            for(j = 0; j < 3; ++j) {
                ed = ed->next;
                if(ed->end == b || degree[ed->end] == 5) break;
            }
            ok += (j == 3);

            for(j = 0; j < len; ++j)
                if(degree[v[j]] == 5 && degree[w[j + 1]] == 5) break;
            ok += (j == len);

            for(j = 1; j < len; ++j)
                if(degree[v[j]] == 5 && degree[w[j]] == 5) break;
            ok += (j == len);

            if(ok == 4) return 1;
        }

    }

    return 0;
}

/****************************************************************************/

/*
 * Returns 1 if the graph contains an Lx reduction with x <= max_length,
 * else returns 0.
 */
static int
has_straight_ipr(int max_length) {
    EDGE *e, *ee;
    int i;
    for (i = 0; i < 12; ++i) {
        e = ee = firstedge[degree_5_vertices[i]];
        do {
            if (hasStraightIPR(e, max_length)) {
                previous_rejector[nv] = e;
                return 1;
            }
            e = e->next;
        } while (e != ee);
    }
    return 0;
}

/****************************************************************************/

/**
 * Returns 1 if there is an straight IPR reduction with length at most max_length
 * which starts from edge.
 */
static int
hasStraightIPR_given(EDGE *edge, int max_length) {
    if(degree[edge->start] != 5)
        return 0;
    else
        return hasStraightIPR(edge, max_length);
}

/****************************************************************************/

/*
 * Checks whether or not a 'better' reduction than the given one exists.
 *
 * If 1 is returned good_next[] contains a list of all "next"-edges which could
 * be canonical and good_prev[] of all such "prev"-edges.
 * 
 * Important: this method is not called for B00 reductions, use 
 * is_best_bent_zero_reduction() instead.
 */
static int
is_best_bent_reduction(EDGE *test_edge1, EDGE *test_edge2, int bent_position, int bent_length, int use_next,
        EDGE *good_next[], int *ngood_next, int *can_edges_next, EDGE *good_prev[], int *ngood_prev, int *can_edges_prev) {
    //DEBUGASSERT(bent_length <= 2*bent_position);

    //Is not called for B00
    DEBUGASSERT(bent_length > 0);

    //Priorities: Lx Bx Lx+1 Bx+1 etc.
    if((!fulleriprswitch && has_straight(bent_length + 1)) 
            || (fulleriprswitch && has_straight_ipr(bent_length + 1)))
        return 0;
    
    int (*proc_short_bent) (EDGE *, int, int, int) = NULL;
    if(fulleriprswitch)
        proc_short_bent = has_short_bent_reduction_ipr;
    else
        proc_short_bent = has_short_bent_reduction;

    RESETMARKS_B00_NEXT;
    RESETMARKS_B00_PREV;
    int num_good_next, num_good_prev;
    num_good_next = num_good_prev = 0;
    //int bents_have_same_len = (2*bent_position == bent_length);
    
    //Choose the edge with the longest path as representative for the reduction
    int length_path_1 = bent_position;
    int length_path_2 = bent_length - bent_position;
    int longest_path = MAX(length_path_1, length_path_2);
    if(use_next) {
        if(length_path_1 == longest_path) {
            good_next[num_good_next++] = test_edge1;
            MARK_B00_NEXT(test_edge1->label);
        }
        if(length_path_2 == longest_path) {
            good_prev[num_good_prev++] = test_edge2;
            MARK_B00_PREV(test_edge2->label);
        }
    } else {
        if(length_path_1 == longest_path) {
            good_prev[num_good_prev++] = test_edge1;
            MARK_B00_PREV(test_edge1->label);
        }
        if(length_path_2 == longest_path) {
            good_next[num_good_next++] = test_edge2;
            MARK_B00_NEXT(test_edge2->label);
        }
    }
    *can_edges_next = num_good_next;
    *can_edges_prev = num_good_prev;

    /*
     * Check if there are no shorter bent reductions.
     * If both reductions have the same length, the one with the longest
     * straight path is 'better'.
     */
    EDGE *e;
    int i, j, k, res;
    for(i = 0; i < 12; i++) {
        e = firstedge[degree_5_vertices[i]];
        for(j = 0; j < 5; j++) {
            if(degree[e->end] == 6) //Remark: this if is not necessary in case of ipr
                /*
                 * Has to search in both directions here, since it is assumed that 
                 * first bent is longest.
                 */
                for(k = 0; k < 2; k++) {
                    if((k && !ISMARKED_B00_NEXT(e->label)) ||
                            (!k && !ISMARKED_B00_PREV(e->label))) {
                        //res = has_short_bent_reduction(e, bent_position, bent_length, k);
                        //res = (*proc_short_bent) (e, bent_position, bent_length, k);
                        //Important: now longest_path instead of bent_position!
                        res = (*proc_short_bent) (e, longest_path, bent_length, k);
                        if(res == 2)
                            return 0; //Shorter reduction found
                        else if(res == 1) {
                            if(k)
                                good_next[num_good_next++] = e;
                            else
                                good_prev[num_good_prev++] = e;
                        }
                    }
                }
            e = e->next;
        }
    }

    DEBUGASSERT(num_good_next <= MAX_EXTENSIONS_SINGLE && num_good_prev <= MAX_EXTENSIONS_SINGLE);

    *ngood_next = num_good_next;
    *ngood_prev = num_good_prev;
    
    //Remark: using is_best_third_colour_bent() here is slower.

    return 1;
}

/****************************************************************************/

/*
 * Returns 1 if the current graph contains at least 1 B00 reduction,
 * else returns 0.
 */
static int
has_B00_reductions() {
    EDGE *e;
    EDGE *endedge;
    int i, j;
    for(i = 0; i < 12; i++) {
        e = firstedge[degree_5_vertices[i]];
        for(j = 0; j < 5; j++) {
            if(has_bent_zero_reduction(e, &endedge)) {
                return 1;
            }
            e = e->next;
        }
    }
    return 0;
}

/****************************************************************************/

/*
 * Returns 1 if the current graph contains at least 1 B00 reduction,
 * else returns 0.
 */
static int
has_B00_reductions_ipr() {
    EDGE *e;
    EDGE *endedge;
    int i, j;
    for(i = 0; i < 12; i++) {
        e = firstedge[degree_5_vertices[i]];
        for(j = 0; j < 5; j++) {
            if(has_bent_zero_reduction_ipr(e, &endedge)) {
                return 1;
            }
            e = e->next;
        }
    }
    return 0;
}

/****************************************************************************/

/*
 * Returns 1 if the current graph contains at least 1 B00 reduction,
 * else returns 0.
 * 
 * Important: this method assumes that the current graph is IPR!
 * (see has_B10_reduction() for more info).
 */
static int
has_B10_reductions() {
    EDGE *e;
    int i, j, k;
    for(i = 0; i < 12; i++) {
        e = firstedge[degree_5_vertices[i]];
        for(j = 0; j < 5; j++) {
            for(k = 0; k < 2; k++) {
                if(has_B10_reduction(e, k)) {
                    return 1;
                }
            }
            e = e->next;
        }
    }
    return 0;
}

/****************************************************************************/

/*
 * Returns 1 if the current graph contains at least 1 B00 reduction,
 * else returns 0.
 */
static int
has_B10_reductions_ipr() {
    EDGE *e;
    int i, j, k;
    for(i = 0; i < 12; i++) {
        e = firstedge[degree_5_vertices[i]];
        for(j = 0; j < 5; j++) {
            for(k = 0; k < 2; k++) {
                if(has_B10_reduction_ipr(e, k)) {
                    return 1;
                }
            }
            e = e->next;
        }
    }
    return 0;
}

/****************************************************************************/

/*
 * Returns 1 if the current graph has bent reductions with max length
 * max_bent_length, else returns 0.
 */
static int
has_bent_reductions(int max_bent_length) {
    EDGE *e;
    int i, j, k, res;
    for(i = 0; i < 12; i++) {
        e = firstedge[degree_5_vertices[i]];
        if(degree[e->end] == 6)
            for(j = 0; j < 5; j++) {
                for(k = 0; k < 2; k++) {
                    res = has_short_bent_reduction(e, (max_bent_length + 1) / 2, max_bent_length, k);
                    if(res >= 1)
                        return 1;
                }
                e = e->next;
            }
    }
    return 0;
}

/****************************************************************************/

/*
 * Returns 1 if the current graph has ipr bent reductions with max length
 * max_bent_length, else returns 0.
 */
static int
has_bent_reductions_ipr(int max_bent_length) {
    EDGE *e;
    int i, j, k, res;
    for(i = 0; i < 12; i++) {
        e = firstedge[degree_5_vertices[i]];
        for(j = 0; j < 5; j++) {
            for(k = 0; k < 2; k++) {
                    res = has_short_bent_reduction_ipr(e, (max_bent_length + 1)/2, max_bent_length, k);
                if(res >= 1)
                    return 1;
            }
            e = e->next;
        }
    }
    return 0;
}

/****************************************************************************/


/*
 * Checks whether or not a 'better' reduction than the given one exists.
 *
 * The canonical reduction is the shortest reduction and some additional combinatorial
 * invariants are used as well.
 *
 * If 1 is returned good_next[] contains a list of all "next"-edges which could
 * be canonical and good_prev[] of all such "prev"-edges.
 */
static int
is_best_straight_reduction(EDGE *test_edge1, EDGE *test_edge2, int pathlength, int use_next,
            EDGE *good_next[], int *ngood_next, int *can_edges_next, EDGE *good_prev[], int *ngood_prev, int *can_edges_prev) {

    DEBUGASSERT(!fulleriprswitch);
    
    /* Compute first colour */
    EDGE *good_next_tmp[MAX_EXTENSIONS_SINGLE];
    EDGE *good_prev_tmp[MAX_EXTENSIONS_SINGLE];
    int num_good_next_tmp, num_good_prev_tmp;
    num_good_next_tmp = num_good_prev_tmp = 0;

    int colour_testedge1, colour_testedge2;
    int testedge1_is_best, testedge2_is_best;
    int best_colour;
    if(use_next) {
        colour_testedge1 = get_colour_next_5(test_edge1->next->next->invers->next);
        colour_testedge2 = get_colour_next_5(test_edge2->next->next->invers->next);
        best_colour = MAX(colour_testedge1, colour_testedge2);
    } else {
        colour_testedge1 = get_colour_prev_5(test_edge1->prev->prev->invers->prev);
        colour_testedge2 = get_colour_prev_5(test_edge2->prev->prev->invers->prev);
        best_colour = MAX(colour_testedge1, colour_testedge2);
    }
    testedge1_is_best = (colour_testedge1 == best_colour);
    testedge2_is_best = (colour_testedge2 == best_colour);

    int colour_tmp;

    if(nv < maxnv - 1) {
        num_straight_extensions = 0;
        //num_straight_colour_2_extensions = 0;
    }

    int (*proc_short_straight) (EDGE *, int, EDGE **, unsigned char *) = NULL;
    if(pathlength == 3)
        proc_short_straight = has_short_straight_reduction_L1;
    else
        proc_short_straight = has_short_straight_reduction;

    RESETMARKS;
    EDGE *e, *ee;
    EDGE *endedge;
    unsigned char direction_bitvector;
    int i, res;
    for(i = 0; i < 12; i++) {
        e = ee = firstedge[degree_5_vertices[i]];
        //Not faster if using j < 5...
        do {
            if(!ISMARKEDLO(e)) {
                //res = has_short_straight_reduction(e, pathlength, &endedge,
                //        &direction_bitvector);
                res = (*proc_short_straight) (e, pathlength, &endedge,
                        &direction_bitvector);
                if(res == 2) { //Better reduction found
                    //fprintf(stderr, "better reduction found. nv: %d, pathlen is %d and max pathlen straight: %d\n", nv, pathlength, straight_length);
                    return 0;
                } else if(res == 1) {
                    if((direction_bitvector & NEXT_BITVECTOR) != 0) {
                        if(!use_next || (e != test_edge1 && e != test_edge2)) {
                            colour_tmp = get_colour_next_5(e->next->next->invers->next);
                            if(colour_tmp > best_colour)
                                return 0;
                            else if(colour_tmp == best_colour)
                                good_next_tmp[num_good_next_tmp++] = e;

                            colour_tmp = get_colour_next_5(endedge->next->next->invers->next);
                            if(colour_tmp > best_colour)
                                return 0;
                            else if(colour_tmp == best_colour)
                                good_next_tmp[num_good_next_tmp++] = endedge;
                        }
                    }
                    if((direction_bitvector & PREV_BITVECTOR) != 0) {
                        if(use_next || (e != test_edge1 && e != test_edge2)) {
                            colour_tmp = get_colour_prev_5(e->prev->prev->invers->prev);
                            if(colour_tmp > best_colour)
                                return 0;
                            else if(colour_tmp == best_colour)
                                good_prev_tmp[num_good_prev_tmp++] = e;

                            colour_tmp = get_colour_prev_5(endedge->prev->prev->invers->prev);
                            if(colour_tmp > best_colour)
                                return 0;
                            else if(colour_tmp == best_colour)
                                good_prev_tmp[num_good_prev_tmp++] = endedge;
                        }
                    }
                    if(nv < maxnv - 1 && num_straight_extensions < MAX_PREV_EXTENSIONS - 1)
                        add_straight_extension_to_list(e, pathlength, (direction_bitvector & NEXT_BITVECTOR) != 0);

                    //No need to mark e since it wont be visited anymore
                    //Can't mark outside if, since then endedge is invalid!
                    MARKLO(endedge);
                }
            }
            e = e->next;
        } while(e != ee);
    }

    /*
     * Priorities: Lx Bx Lx+1 Bx+1 etc.
     */
    //Remark: might be cheaper to test some of this before computing the colours
    if((pathlength > 3 && has_B00_reductions())
            || (pathlength > 4 && has_B10_reductions())
            || (pathlength > 5 && has_bent_reductions(pathlength - 4)))
        return 0;

    /* Compute third colour */
    return is_best_third_colour(test_edge1, test_edge2, testedge1_is_best, testedge2_is_best,
                use_next, good_next_tmp, num_good_next_tmp, good_prev_tmp, num_good_prev_tmp,
                good_next, ngood_next, can_edges_next, good_prev, ngood_prev, can_edges_prev);

}

/****************************************************************************/

/*
 * Checks whether or not a 'better' IPR reduction than the given one exists.
 *
 * The canonical reduction is the shortest reduction and some additional combinatorial
 * invariants are used as well.
 *
 * If 1 is returned good_next[] contains a list of all "next"-edges which could
 * be canonical and good_prev[] of all such "prev"-edges.
 */
//Remark: this method uses a different colour1 than is_best_straight_reduction()!
static int
is_best_straight_reduction_ipr(EDGE *test_edge1, EDGE *test_edge2, int pathlength, int use_next,
            EDGE *good_next[], int *ngood_next, int *can_edges_next, EDGE *good_prev[], int *ngood_prev, int *can_edges_prev) {

    DEBUGASSERT(fulleriprswitch);
    
    /* Compute first colour */
    EDGE *good_next_tmp[MAX_EXTENSIONS_SINGLE];
    EDGE *good_prev_tmp[MAX_EXTENSIONS_SINGLE];
    int num_good_next_tmp, num_good_prev_tmp;
    num_good_next_tmp = num_good_prev_tmp = 0;
    
    int colour_testedge1, colour_testedge2;
    int testedge1_is_best, testedge2_is_best;
    int best_colour;
    if(!use_next) {
        //colour_testedge1 = get_colour_next_5(test_edge1->next->next->invers->next);
        //colour_testedge2 = get_colour_next_5(test_edge2->next->next->invers->next);
        colour_testedge1 = get_colour_next_3(test_edge1->next->next->invers->next->next);
        colour_testedge2 = get_colour_next_3(test_edge2->next->next->invers->next->next);        
        best_colour = MAX(colour_testedge1, colour_testedge2);
    } else {
        //colour_testedge1 = get_colour_prev_5(test_edge1->prev->prev->invers->prev);
        //colour_testedge2 = get_colour_prev_5(test_edge2->prev->prev->invers->prev);
        colour_testedge1 = get_colour_prev_3(test_edge1->prev->prev->invers->prev->prev);
        colour_testedge2 = get_colour_prev_3(test_edge2->prev->prev->invers->prev->prev);        
        best_colour = MAX(colour_testedge1, colour_testedge2);
    }
    testedge1_is_best = (colour_testedge1 == best_colour);
    testedge2_is_best = (colour_testedge2 == best_colour);
    
    int colour_tmp;
    
    if(nv < maxnv - 2) {
        num_straight_extensions = 0;
        //num_straight_colour_2_extensions = 0;
    }

    int (*proc_short_straight) (EDGE *, int, EDGE **, unsigned char *) = NULL;
    if(pathlength == 3)
        proc_short_straight = has_short_straight_reduction_L1_ipr;
    else if(pathlength == 4)
        proc_short_straight = has_short_straight_reduction_L2_ipr;
    else
        proc_short_straight = has_short_straight_reduction_ipr;

    RESETMARKS;
    EDGE *e, *ee;
    EDGE *endedge;
    unsigned char direction_bitvector;
    int i, res;
    for(i = 0; i < 12; i++) {
        e = ee = firstedge[degree_5_vertices[i]];
        //Not faster if using j < 5...
        do {
            if(!ISMARKEDLO(e)) {
                //res = has_short_straight_reduction(e, pathlength, &endedge,
                //        &direction_bitvector);
                res = (*proc_short_straight) (e, pathlength, &endedge,
                        &direction_bitvector);
                if(res == 2) { //Better reduction found
                    //fprintf(stderr, "better reduction found. nv: %d, pathlen is %d and max pathlen straight: %d\n", nv, pathlength, straight_length);
                    previous_rejector[nv] = e;
                    return 0;
                } else if(res == 1) {
                    if((direction_bitvector & NEXT_BITVECTOR) != 0) {
                        if(!use_next || (e != test_edge1 && e != test_edge2)) {
                            //colour_tmp = get_colour_next_5(e->next->next->invers->next);
                            //colour_tmp = get_colour_prev_5(e->prev->prev->invers->prev);
                            colour_tmp = get_colour_prev_3(e->prev->prev->invers->prev->prev);
                            if(colour_tmp > best_colour)
                                return 0;
                            else if(colour_tmp == best_colour)
                                good_next_tmp[num_good_next_tmp++] = e;

                            //colour_tmp = get_colour_next_5(endedge->next->next->invers->next);
                            //colour_tmp = get_colour_prev_5(endedge->prev->prev->invers->prev);
                            colour_tmp = get_colour_prev_3(endedge->prev->prev->invers->prev->prev);
                            if(colour_tmp > best_colour)
                                return 0;
                            else if(colour_tmp == best_colour)
                                good_next_tmp[num_good_next_tmp++] = endedge;
                        }
                    }
                    if((direction_bitvector & PREV_BITVECTOR) != 0) {
                        if(use_next || (e != test_edge1 && e != test_edge2)) {
                            //colour_tmp = get_colour_prev_5(e->prev->prev->invers->prev);
                            //colour_tmp = get_colour_next_5(e->next->next->invers->next);
                            colour_tmp = get_colour_next_3(e->next->next->invers->next->next);
                            if(colour_tmp > best_colour)
                                return 0;
                            else if(colour_tmp == best_colour)
                                good_prev_tmp[num_good_prev_tmp++] = e;

                            //colour_tmp = get_colour_prev_5(endedge->prev->prev->invers->prev);
                            //colour_tmp = get_colour_next_5(endedge->next->next->invers->next);
                            colour_tmp = get_colour_next_3(endedge->next->next->invers->next->next);
                            if(colour_tmp > best_colour)
                                return 0;
                            else if(colour_tmp == best_colour)
                                good_prev_tmp[num_good_prev_tmp++] = endedge;
                        }
                    }
                    //Pathlength 4 enkel nuttig voor Lx, x >= 3
                    //if(nv < maxnv - 1 && pathlength <= 4 && num_straight_extensions < MAX_PREV_EXTENSIONS - 1)
                    if(nv < maxnv - 2 && num_straight_extensions < MAX_PREV_EXTENSIONS - 1)
                        add_straight_extension_to_list(e, pathlength, (direction_bitvector & NEXT_BITVECTOR) != 0);

                    //No need to mark e since it wont be visited anymore
                    //Can't mark outside if, since then endedge is invalid!
                    MARKLO(endedge);
                }
            }
            e = e->next;
        } while(e != ee);
    }

    /*
     * Priorities: Lx Bx Lx+1 Bx+1 etc.
     */
    if((pathlength > 3 && has_B00_reductions_ipr())
            || (pathlength > 4 && has_B10_reductions_ipr())
            || (pathlength > 5 && has_bent_reductions_ipr(pathlength - 4)))
        return 0;

    /* Compute third colour */
    return is_best_third_colour(test_edge1, test_edge2, testedge1_is_best, testedge2_is_best,
                use_next, good_next_tmp, num_good_next_tmp, good_prev_tmp, num_good_prev_tmp,
                good_next, ngood_next, can_edges_next, good_prev, ngood_prev, can_edges_prev);

}

/**************************************************************************/

/*
 * Updates the edges around the 2 new vertices of an L0 extension in edge_list.
 * This method is only called if the constructed graph is canonical.
 */
//Remark: it would be useful also to do this for other straight + bent extensions!
static void
update_edge_list_L0() {
    EDGE *e;
    int i, j;
    for(i = 2; i > 0; i--) {
        e = firstedge[nv - i];
        for(j = 0; j < 5; j++) {
            edge_list[e->start][e->end] = e;
            e = e->next;
        }
    }
}

/**************************************************************************/

/*
 * The main node of the recursion for the generation of fullerenes in
 * dual representation (so all faces are triangles).
 * As this procedure is entered, nv,ne,degree etc are set for some graph,
 * and nbtot/nbop are the values returned by canon() for that graph.
 */
static void
scansimple_fuller(int nbtot, int nbop)
{

    if (nv == maxnv)
    {
        //Remark: splitlevel is assumed to be < maxnv
        got_one(nbtot,nbop);

        return;
    }

    DEBUGASSERT(nv < maxnv);

    if (nv == splitlevel)
    {
#ifdef SPLITTEST
	ADDBIG(splitcases,1);
	return;
#endif
        if (splitcount-- != 0) return;
        splitcount = mod - 1;

        //Remark: not necessary?
        //for (i = 0; i < nv; ++i) firstedge_save[i] = firstedge[i];
    }

    /*
     * Only output graphs below splitlevel if splitcount == 0.
     */
    if(startswitch && nv >= start_output && (nv >= splitlevel || splitcount == 0)) {
        got_one(nbtot,nbop);
    }


#ifdef PRE_FILTER_SIMPLE
    if (!(PRE_FILTER_SIMPLE)) return;
#endif

    int max_pathlength_straight = determine_max_pathlength_straight();

    /*
     * Can occur for graphs generated from eg (5,0)-nanotubes.
     * And can also occur when using mod/res.
     */
    if(max_pathlength_straight < 2) {
        //fprintf(stderr, "Info: pathlength is too small: %d and nv is %d\n", max_pathlength_straight, nv);
        return;
    }

    //Max number of straight expansions for given pathlength is 12 * 5 / 2 * 2 = 60

    EDGE * ext_L0[MAX_EXTENSIONS_SINGLE];
    int ext_L0_use_next[MAX_EXTENSIONS_SINGLE];

    EDGE * ext_L1[MAX_EXTENSIONS_SINGLE];
    int ext_L1_use_next[MAX_EXTENSIONS_SINGLE];

    //Max number of bent expansions for given length is 12 * 5 / 2 * 2 = 60
    EDGE * ext_bent_zero[MAX_EXTENSIONS_SINGLE];
    int ext_bent_zero_use_next[MAX_EXTENSIONS_SINGLE];

    /*
     * Number of combinations for given max_length_bent: 0, 2, 5, 9, 14, 20 = n(n+3)/2.
     *
     * Distance between 2 deg 5 vertices after applying Bij is i+j+2.
     * So i+j must be <= max_pathlength - 2 - 1
     */
    int max_straight_extensions, max_bent_extensions;
    if(max_pathlength_straight > 3) {
        max_straight_extensions = max_pathlength_straight - 3;
        max_bent_extensions = (max_pathlength_straight - 3) * max_pathlength_straight / 2;
    } else
        max_straight_extensions = max_bent_extensions = 0;

    //+1 to avoid array of size 0
    EDGE * ext_straight[max_straight_extensions * MAX_EXTENSIONS_SINGLE + 1];
    int ext_straight_length[max_straight_extensions * MAX_EXTENSIONS_SINGLE + 1];
    int ext_straight_use_next[max_straight_extensions * MAX_EXTENSIONS_SINGLE + 1];

    EDGE * ext_bent[max_bent_extensions * MAX_EXTENSIONS_SINGLE + 1];
    int ext_bent_position[max_bent_extensions * MAX_EXTENSIONS_SINGLE + 1];
    int ext_bent_length[max_bent_extensions * MAX_EXTENSIONS_SINGLE + 1];
    int ext_bent_use_next[max_bent_extensions * MAX_EXTENSIONS_SINGLE + 1];

    int num_ext_L1, num_ext_L0, num_ext_bent_zero, num_ext_straight, num_ext_bent, xnbtot, xnbop;
    int i, j, test_canon;
    EDGE *work_edge;


#ifndef FIND_EXTENSIONS_SIMPLE_FULLER
#define FIND_EXTENSIONS_SIMPLE_FULLER find_extensions_fuller
#endif

    FIND_EXTENSIONS_SIMPLE_FULLER(nbtot,nbop,max_pathlength_straight,
            ext_L0, &num_ext_L0, ext_L0_use_next, ext_L1, &num_ext_L1, ext_L1_use_next,
            ext_bent_zero, &num_ext_bent_zero, ext_bent_zero_use_next,
            ext_straight, &num_ext_straight, ext_straight_length, ext_straight_use_next,
            ext_bent, &num_ext_bent, ext_bent_position, ext_bent_length, ext_bent_use_next);

    DEBUGASSERT(num_ext_L0 <= MAX_EXTENSIONS_SINGLE);
    DEBUGASSERT(num_ext_L1 <= MAX_EXTENSIONS_SINGLE);
    DEBUGASSERT(num_ext_bent_zero <= MAX_EXTENSIONS_SINGLE);
    DEBUGASSERT(num_ext_straight <= max_straight_extensions * MAX_EXTENSIONS_SINGLE);
    DEBUGASSERT(num_ext_bent <= max_bent_extensions * MAX_EXTENSIONS_SINGLE);

    /* Perform straight expansions */
    int path[max_pathlength_straight + 1];
    int parallel_path[max_pathlength_straight + 1];

    EDGE *good_edges_prev[MAX_EXTENSIONS_SINGLE], *good_edges_next[MAX_EXTENSIONS_SINGLE];
    int num_good_prev, num_good_next, can_edges_next, can_edges_prev;

    /* Perform L0 expansions */
    for(i = 0; i < num_ext_L0; ++i) {
        work_edge = ext_L0[i];
        for(j = 0; j <= 2 ; j++) {
            path[j] = work_edge->start;
            if(ext_L0_use_next[i]) {
                parallel_path[j] = work_edge->prev->end;
                work_edge = work_edge->invers->next->next->next;
            } else {
                parallel_path[j] = work_edge->next->end;
                work_edge = work_edge->invers->prev->prev->prev;
            }
        }

        extend_L0(ext_L0[i], path, parallel_path, ext_L0_use_next[i]);

/*
        if(is_best_L0_reduction(edge_list[nv - 2][nv - 1], edge_list[nv - 1][nv - 2],
                ext_L0_use_next[i], good_edges_next, &num_good_next, &can_edges_next,
                good_edges_prev, &num_good_prev, &can_edges_prev)) {
*/
        if(is_best_L0_reduction(last_edge_L0, last_edge_L0->invers,
                ext_L0_use_next[i], good_edges_next, &num_good_next, &can_edges_next,
                good_edges_prev, &num_good_prev, &can_edges_prev)) {
            test_canon = 1;
            if(num_good_next + num_good_prev == 1) {
                test_canon = 0;
                xnbtot = xnbop = 1;
            }

            if(!test_canon || canon_edge_oriented(good_edges_next, num_good_next, can_edges_next,
                    good_edges_prev, num_good_prev, can_edges_prev, degree, numbering, &xnbtot, &xnbop)) {
                if(nv < maxnv - 1) {
                    straight_length = 2;

                    for(j = 0; j < straight_length; j++)
                        straight_extensions[num_straight_extensions][j] = nv - straight_length + j;
                    straight_extensions[num_straight_extensions][j++] = path[0];
                    straight_extensions[num_straight_extensions][j++] = parallel_path[straight_length];

                    num_straight_extensions++;

                    /*
                     * Graph is canonical, so now we should update the edges
                     * in edge_list which we didn't update yet.
                     */
                    update_edge_list_L0();
                    
                    add_straight_colour_2_L0_extension_to_list(edge_list[path[0]][nv - 2], edge_list[parallel_path[2]][nv - 1]);                    

                }

                scansimple_fuller(xnbtot, xnbop);
            }
        }

        reduce_L0(path, parallel_path);

    }

    /* Perform L1 expansions */
    for(i = 0; i < num_ext_L1; ++i) {
        work_edge = ext_L1[i];
        DEBUGASSERT(3 <= max_pathlength_straight);
        for(j = 0; j < 3 + 1; j++) {
            path[j] = work_edge->start;
            if(ext_L1_use_next[i]) {
                parallel_path[j] = work_edge->prev->end;
                work_edge = work_edge->invers->next->next->next;
            } else {
                parallel_path[j] = work_edge->next->end;
                work_edge = work_edge->invers->prev->prev->prev;
            }
        }

        extend_straight(ext_L1[i], path, parallel_path, 3, ext_L1_use_next[i]);

        if(is_best_straight_reduction(edge_list[nv - 3][nv - 2],
                edge_list[nv - 1][nv - 2], 3, ext_L1_use_next[i],
                good_edges_next, &num_good_next, &can_edges_next, good_edges_prev, &num_good_prev, &can_edges_prev)) {
            test_canon = 1;
            if(num_good_next + num_good_prev == 1) {
                test_canon = 0;
                xnbtot = xnbop = 1;
            }
            
            if(!test_canon || canon_edge_oriented(good_edges_next, num_good_next, can_edges_next,
                    good_edges_prev, num_good_prev, can_edges_prev, degree, numbering, &xnbtot, &xnbop)) {
                if(nv < maxnv - 1) {
                    straight_length = 3;
                    num_bent_zero_extensions = 0;

                    for(j = 0; j < straight_length; j++)
                        straight_extensions[num_straight_extensions][j] = nv - straight_length + j;
                    straight_extensions[num_straight_extensions][j++] = path[0];
                    straight_extensions[num_straight_extensions][j++] = parallel_path[straight_length];

                    num_straight_extensions++;
                }

                scansimple_fuller(xnbtot, xnbop);
            }

        }

        reduce_straight(path, parallel_path, 3);

    }

    /* Perform B00 expansions */
    int path_b00[5];
    int parallel_path_b00[3];

    for(i = 0; i < num_ext_bent_zero; ++i) {
        work_edge = ext_bent_zero[i];
        path_b00[0] = work_edge->start;
        path_b00[1] = work_edge->end;
        work_edge = work_edge->invers;
        if(ext_bent_zero_use_next[i]) {
            parallel_path_b00[0] = work_edge->next->end;
            work_edge = work_edge->next->next->next;
        } else {
            parallel_path_b00[0] = work_edge->prev->end;
            work_edge = work_edge->prev->prev->prev;
        }
        path_b00[2] = work_edge->end;
        work_edge = work_edge->invers;
        if(ext_bent_zero_use_next[i]) {
            parallel_path_b00[1] = work_edge->next->end;
            work_edge = work_edge->next->next;
        } else {
            parallel_path_b00[1] = work_edge->prev->end;
            work_edge = work_edge->prev->prev;
        }
        path_b00[3] = work_edge->end;
        work_edge = work_edge->invers;
        if(ext_bent_zero_use_next[i]) {
            parallel_path_b00[2] = work_edge->next->next->end;
            work_edge = work_edge->next->next->next;
        } else {
            parallel_path_b00[2] = work_edge->prev->prev->end;
            work_edge = work_edge->prev->prev->prev;
        }
        path_b00[4] = work_edge->end;

        extend_bent_zero(ext_bent_zero[i], path_b00, parallel_path_b00, ext_bent_zero_use_next[i]);

        if(is_best_bent_zero_reduction(edge_list[nv - 3][path_b00[0]], edge_list[nv - 1][path_b00[4]], ext_bent_zero_use_next[i],
                good_edges_next, &num_good_next, &can_edges_next, good_edges_prev, &num_good_prev, &can_edges_prev)) {
            test_canon = 1;
            if(num_good_next + num_good_prev == 1) {
                test_canon = 0;
                xnbtot = xnbop = 1;
            }

            if(!test_canon || canon_edge_oriented(good_edges_next, num_good_next, can_edges_next,
                    good_edges_prev, num_good_prev, can_edges_prev, degree, numbering, &xnbtot, &xnbop)) {

                if(nv < maxnv - 1) {
                    straight_length = MAX_STRAIGHT_LENGTH + 1;
                    //num_bent_zero_extensions = 0;
                    add_bent_zero_extension_to_list_ipr(edge_list[nv - 3][path_b00[0]], ext_bent_zero_use_next[i]);
                }
                
                scansimple_fuller(xnbtot, xnbop);
            }

        }

        reduce_bent_zero(path_b00, parallel_path_b00);
    }

    /* Perform longer straight expansions */
    for(i = 0; i < num_ext_straight; ++i) {
        work_edge = ext_straight[i];
        DEBUGASSERT(ext_straight_length[i] <= max_pathlength_straight);
        for(j = 0; j < ext_straight_length[i] + 1; j++) {
            path[j] = work_edge->start;
            if(ext_straight_use_next[i]) {
                parallel_path[j] = work_edge->prev->end;
                work_edge = work_edge->invers->next->next->next;
            } else {
                parallel_path[j] = work_edge->next->end;
                work_edge = work_edge->invers->prev->prev->prev;
            }
        }

        extend_straight(ext_straight[i], path, parallel_path, ext_straight_length[i], ext_straight_use_next[i]);

        if(is_best_straight_reduction(edge_list[nv - ext_straight_length[i]][nv - ext_straight_length[i] + 1],
                edge_list[nv - 1][nv - 2], ext_straight_length[i], ext_straight_use_next[i],
                good_edges_next, &num_good_next, &can_edges_next, good_edges_prev, &num_good_prev, &can_edges_prev)) {
            test_canon = 1;
            if(num_good_next + num_good_prev == 1) {
                test_canon = 0;
                xnbtot = xnbop = 1;
            }
            
            if(!test_canon || canon_edge_oriented(good_edges_next, num_good_next, can_edges_next,
                    good_edges_prev, num_good_prev, can_edges_prev, degree, numbering, &xnbtot, &xnbop)) {
                if(nv < maxnv - 1) {
                    straight_length = ext_straight_length[i];
                    num_bent_zero_extensions = 0;

                    for(j = 0; j < straight_length; j++)
                        straight_extensions[num_straight_extensions][j] = nv - straight_length + j;
                    straight_extensions[num_straight_extensions][j++] = path[0];
                    straight_extensions[num_straight_extensions][j++] = parallel_path[straight_length];

                    num_straight_extensions++;
                }
                scansimple_fuller(xnbtot, xnbop);
            }
        }

        reduce_straight(path, parallel_path, ext_straight_length[i]);
    }

    /* Perform longer bent expansions */
    /*
     * max_bent_length = max_pathlength_straight - 3
     * Max array size = max_bent_length + 5 = max_pathlength_straight + 2
     */
    int path_bent[max_pathlength_straight + 2];
    //Parallel path is 2 shorter
    int parallel_path_bent[max_pathlength_straight];
    int length, length_parallel;
    for(i = 0; i < num_ext_bent; ++i) {
        length = length_parallel = 0;
        work_edge = ext_bent[i];
        path_bent[length++] = work_edge->start;
        for(j = 0; j <= ext_bent_position[i]; j++) {
            path_bent[length++] = work_edge->end;
            work_edge = work_edge->invers;
            if(ext_bent_use_next[i]) {
                parallel_path_bent[length_parallel++] = work_edge->next->end;
                work_edge = work_edge->next->next->next;
            } else {
                parallel_path_bent[length_parallel++] = work_edge->prev->end;
                work_edge = work_edge->prev->prev->prev;
            }
        }
        DEBUGASSERT(length == ext_bent_position[i] + 2);
        DEBUGASSERT(length_parallel == ext_bent_position[i] + 1);

        path_bent[length++] = work_edge->end;
        work_edge = work_edge->invers;
        if(ext_bent_use_next[i]) {
            work_edge = work_edge->next->next;
        } else {
            work_edge = work_edge->prev->prev;
        }

        for(j = ext_bent_position[i]; j <= ext_bent_length[i]; j++) {
            path_bent[length++] = work_edge->end;
            work_edge = work_edge->invers;
            if(ext_bent_use_next[i]) {
                parallel_path_bent[length_parallel++] = work_edge->next->end;
                work_edge = work_edge->next->next->next;
            } else {
                parallel_path_bent[length_parallel++] = work_edge->prev->end;
                work_edge = work_edge->prev->prev->prev;
            }
        }

        path_bent[length++] = work_edge->end;
        if(ext_bent_use_next[i])
            parallel_path_bent[length_parallel++] = work_edge->prev->end;
        else
            parallel_path_bent[length_parallel++] = work_edge->next->end;
        DEBUGASSERT(length == ext_bent_length[i] + 5);
        DEBUGASSERT(length_parallel == ext_bent_length[i] + 3);

        extend_bent(ext_bent[i], path_bent, parallel_path_bent, ext_bent_position[i],
            ext_bent_length[i], ext_bent_use_next[i]);

        if(is_best_bent_reduction(edge_list[nv - ext_bent_length[i] - 3][path_bent[0]], edge_list[nv - 1][path_bent[ext_bent_length[i] + 4]],
                ext_bent_position[i], ext_bent_length[i], ext_bent_use_next[i],
                good_edges_next, &num_good_next, &can_edges_next, good_edges_prev, &num_good_prev, &can_edges_prev)) {
            test_canon = 1;            
            if(num_good_next + num_good_prev == 1) {
                test_canon = 0;
                xnbtot = xnbop = 1;
            }
            
            if(!test_canon || canon_edge_oriented(good_edges_next, num_good_next, can_edges_next,
                    good_edges_prev, num_good_prev, can_edges_prev, degree, numbering, &xnbtot, &xnbop)) {
                if(nv < maxnv - 1) {
                    straight_length = MAX_STRAIGHT_LENGTH + 1;
                    num_bent_zero_extensions = 0;
                }

                scansimple_fuller(xnbtot, xnbop);
            }

        }

        reduce_bent(path_bent, parallel_path_bent, ext_bent_position[i], ext_bent_length[i]);
    }

}

/**************************************************************************/

/*
 * Returns 1 if the current graph contains at least three independent short
 * ipr reductions. Two reductions are independent if the pentagons of both
 * reductions are on distance > 3.
 * 
 * Short reductions are: L1, B00, L2 and B10.
 * 
 * If 1 is returned, *max_pathlength_straight is also set.
 * If there are three independent L1 or B00 reductions, max_pathlength_straight 
 * is set to 3, else it is 4.
 */
static int
contains_at_least_three_independent_short_reductions(int *max_pathlength_straight) {
    if(straight_length == MAX_STRAIGHT_LENGTH + 1)
        num_straight_extensions = 0;
    int pentagons_short_reductions[num_bent_zero_extensions + num_straight_extensions + num_L2_extensions + num_B10_extensions][2];
    int is_short_reduction[num_bent_zero_extensions + num_straight_extensions + num_L2_extensions + num_B10_extensions];
    int num_short_reductions = 0;
    int i;
    for(i = 0; i < num_bent_zero_extensions; i++) {
        pentagons_short_reductions[num_short_reductions][0] = bent_zero_extensions[i][0];
        pentagons_short_reductions[num_short_reductions][1] = bent_zero_extensions[i][3];
        is_short_reduction[num_short_reductions] = 1;
        num_short_reductions++;
    }
    if(straight_length == 3)
        for(i = 0; i < num_straight_extensions; i++)
            if(only_hexagons_in_neighbourhood_L1(straight_extensions[i][0], straight_extensions[i][2],
                    edge_list[straight_extensions[i][0]][straight_extensions[i][1]], 2)) {
                pentagons_short_reductions[num_short_reductions][0] = straight_extensions[i][0];
                pentagons_short_reductions[num_short_reductions][1] = straight_extensions[i][2];
                is_short_reduction[num_short_reductions] = 1;
                num_short_reductions++;
            }
    for(i = 0; i < num_L2_extensions; i++)
        if(only_hexagons_in_neighbourhood_L2(L2_extensions[i][0], L2_extensions[i][3],
                edge_list[L2_extensions[i][0]][L2_extensions[i][1]], edge_list[L2_extensions[i][3]][L2_extensions[i][2]], 2)) {
            pentagons_short_reductions[num_short_reductions][0] = L2_extensions[i][0];
            pentagons_short_reductions[num_short_reductions][1] = L2_extensions[i][3];
            is_short_reduction[num_short_reductions] = 0;
            num_short_reductions++;
        }
    for(i = 0; i < num_B10_extensions; i++) {
        pentagons_short_reductions[num_short_reductions][0] = bent_one_zero_extensions[i][0];
        pentagons_short_reductions[num_short_reductions][1] = bent_one_zero_extensions[i][3];
        is_short_reduction[num_short_reductions] = 0;
        num_short_reductions++;
    }
        
    DEBUGASSERT(num_short_reductions <= num_bent_zero_extensions + num_straight_extensions + num_L2_extensions + num_B10_extensions);

    if(num_short_reductions > 2) {
        int j, k;
        for(i = 0; i < num_short_reductions - 2; i++) {
            mark_vertices_in_neighbourhood_bfs_v3(pentagons_short_reductions[i][0], pentagons_short_reductions[i][1], 3);
            for(j = i + 1; j < num_short_reductions - 1; j++)
                if(!ISMARKED_V3(pentagons_short_reductions[j][0])
                        && !ISMARKED_V3(pentagons_short_reductions[j][1])) {
                    mark_vertices_in_neighbourhood_bfs_v4(pentagons_short_reductions[j][0], 
                            pentagons_short_reductions[j][1], 3);
                    for(k = j + 1; k < num_short_reductions; k++)
                        if(!ISMARKED_V3(pentagons_short_reductions[k][0]) && !ISMARKED_V3(pentagons_short_reductions[k][1])
                                && !ISMARKED_V4(pentagons_short_reductions[k][0]) && !ISMARKED_V4(pentagons_short_reductions[k][1])) {
                            if(is_short_reduction[i] && is_short_reduction[j] && is_short_reduction[k])
                                *max_pathlength_straight = 3;
                            else
                                *max_pathlength_straight = 4;
                            return 1;
                        }
                }
        }
    }
    
    return 0;
}

/**************************************************************************/

//The maximum number of vertices on distance <= 3 of a 5-vertex x2
//2*f(3) = 62, but actually a sharper bound is possible
#define MAXN_DIST_3 62

/*
 * Returns 1 if v1 or v2 have 5-vertices (other than v1 and v2) on distance
 * at most 3, else returns 0.
 */
static int
has_5_vertices_on_distance_at_most_three(int v1, int v2) {
    RESETMARKS_V;
    
    int queue[MAXN_DIST_3];
    int distances[MAXN_DIST_3];
    int queue_size = 0;
    queue[queue_size] = v1;
    MARK_V(v1);
    distances[queue_size] = 0;
    queue_size++;
    
    queue[queue_size] = v2;
    MARK_V(v2);
    distances[queue_size] = 0;
    queue_size++;
    
    int i = 0;
    int j;
    EDGE *e;
    while(i < queue_size) {
        e = firstedge[queue[i]];
        for(j = 0; j < degree[queue[i]]; j++) {
            if(!ISMARKED_V(e->end)) {
                if(degree[e->end] == 5)
                    return 1;
                
                MARK_V(e->end);
                if(distances[i] < 2) {
                    //Meaning e->end is at distance < 2
                    queue[queue_size] = e->end;
                    distances[queue_size] = distances[i] + 1;
                    queue_size++;
                }
            }
            e = e->next;
        }
        i++;
    }
    
    return 0;
}

/**************************************************************************/

/*
 * Returns 1 if all neighbours of v are 6-vertices, else returns 0.
 */
static int all_neighbours_are_6_vertices(int v) {
    EDGE *e = firstedge[v];
    int i;
    for(i = 0; i < degree[v]; i++) {
        if(degree[e->end] == 5)
            return 0;
        e = e->next;
    }    
    return 1;
}


/**************************************************************************/

/**
 * Returns 1 if the bent zero reduction will remain a valid ipr reduction
 * after any expansion (although it can now be an L3 or B20 or B11 reduction)
 */
static int will_remain_valid_ipr_reduction(int bent_zero_reduction_index) {
    int i;
    for(i = 5; i < BENT_ZERO_SIZE; i++)
        if(!all_neighbours_are_6_vertices(bent_zero_extensions[bent_zero_reduction_index][i]))
            return 0;
    
    return 1;
}

/**************************************************************************/

/**
 * Returns 1 if at least one of the reductions from startedge_L1_reductions_parent
 * is still a valid straight reduction with length at most max_length.
 */
static int contains_shorter_straight_reduction_ipr(EDGE *startedge_L1_reductions_parent[], 
        int num_L1_reductions_parent, int max_length) {
    int i;
    for(i = 0; i < num_L1_reductions_parent; i++)
        if(hasStraightIPR_given(startedge_L1_reductions_parent[i], max_length))
            return 1;
    
    if(previous_rejector[nv] != NULL && hasStraightIPR_given(previous_rejector[nv], max_length)) {
        return 1;
    }
    
    return 0;
}

/**************************************************************************/

/**
 * Returns 1 if at least one of the reductions from startedge_L1_reductions_parent
 * is still a valid L1 reduction.
 */
static int contains_L1_reduction_ipr(EDGE *startedge_L1_reductions_parent[], int num_L1_reductions_parent) {
    int i;
    for(i = 0; i < num_L1_reductions_parent; i++)
        if(has_short_straight_reduction_L1_ipr_modified(startedge_L1_reductions_parent[i]))
            return 1;
    
    if(previous_rejector[nv] != NULL && has_short_straight_reduction_L1_ipr_modified(previous_rejector[nv])) {
        return 1;    
    }
    
    return 0;
}

/**************************************************************************/

/*
 * The main node of the recursion for the generation of IPR fullerenes in
 * dual representation (so all faces are triangles).
 * As this procedure is entered, nv,ne,degree etc are set for some graph,
 * and nbtot/nbop are the values returned by canon() for that graph.
 */
static void
scansimple_fuller_ipr(int nbtot, int nbop)
{

    if (nv == maxnv)
    {
        //Remark: splitlevel is assumed to be < maxnv
        got_one(nbtot,nbop);

        return;
    }
    
    DEBUGASSERT(nv < maxnv);

    if (nv == splitlevel)
    {
#ifdef SPLITTEST
	ADDBIG(splitcases,1);
	return;
#endif
        if (splitcount-- != 0) return;
        splitcount = mod - 1;

        //Remark: not necessary?
        //for (i = 0; i < nv; ++i) firstedge_save[i] = firstedge[i];
    }

    /*
     * Only output graphs below splitlevel if splitcount == 0.
     */
    if(startswitch && nv >= start_output && (nv >= splitlevel || splitcount == 0)) {
        got_one(nbtot,nbop);
    }
    

#ifdef PRE_FILTER_SIMPLE
    if (!(PRE_FILTER_SIMPLE)) return;
#endif
    
    int max_pathlength_straight = determine_max_pathlength_straight();

    /*
     * Can occur for graphs generated from eg (5,0)-nanotubes.
     * And can also occur when using mod/res.
     */
    if(max_pathlength_straight < 3) {
        //fprintf(stderr, "Info: pathlength is too small: %d and nv is %d\n", max_pathlength_straight, nv);
        return;
    }

    /*
     * Is necessary, since one cannot limit the pathlength to splitlevel 
     * in determine_max_pathlength_straight() if splitcount != 0.
     * This is because find_extensions_fuller_ipr() contains some optimizations
     * for short extensions. So the order in which extensions are found might
     * be different, so splitcount might become invalid!
     * But this is not necessary for pathlength 3 (i.e. L1 and B00).
     */
    int allowed_to_go_above_splitlevel = nv >= splitlevel || splitcount == 0;

    //If shortest reduction is > L1, there cannot be any B00 reductions
    if(max_pathlength_straight > 3 && straight_length == 3) {
        DEBUGASSERT(num_bent_zero_extensions == 0);
        search_bent_zero_reductions_ipr();
    }    
    
    /* Trying to limit the max pathlength */
    num_L2_extensions = num_B10_extensions = 0;
    if(max_pathlength_straight > 4) {
        int contains_indep_reductions = nv > LEVEL_THREE_INDEPENDENT && contains_at_least_three_independent_short_reductions(&max_pathlength_straight);
        if(!contains_indep_reductions) {
            //If shortest reduction > L1 or B00, the shortest reduction is >= L2
            if(straight_length == 3 || num_bent_zero_extensions > 0) {
                search_L2_reductions_ipr();
            }
            if(straight_length == 3 || num_bent_zero_extensions > 0 || num_L2_extensions > 0)
                search_B10_reductions_ipr();
            
            contains_indep_reductions = nv > LEVEL_THREE_INDEPENDENT && (num_L2_extensions > 0 || num_B10_extensions > 0)
                    && contains_at_least_three_independent_short_reductions(&max_pathlength_straight);
                
        } //else: could also search for L2's and B10's for LA, but this doesn't really help much...
        
        if(contains_indep_reductions) {
            //max_pathlength_straight = 3;
            //max_pathlength_straight = 4; //Since also L2 and B10
            if(!startswitch) {
                if(nv + max_pathlength_straight == maxnv - 1)
                    max_pathlength_straight--;
                //For IPR each operation adds at least 3 faces
                if(nv + max_pathlength_straight == maxnv - 2)
                    max_pathlength_straight--;

                if(max_pathlength_straight < 3)
                    return;
            }
        }
    }
    
    //This doesn't slow things down for small fullerenes, so ok!
    if(max_pathlength_straight > 5 &&
            (straight_length == 3 || num_bent_zero_extensions > 0)) {
        //Test how much L1 or B00 reductions there are where all vertices on
        //distance <= 3 are 6-vertices
        //No matter what expansion is applied, these will still be a valid L3/B20/B11 
        //reduction after expansion
        
        //Remark: for B00 not all vertices on distance <= 3 have to be 6-vertices
        //if the neighbours of a,b,c,a',b',c' are all 6-vertices it is sufficient!
        int contains_isolated_L1_or_B00_reductions = 0;
        int i;
        for(i = 0; i < num_bent_zero_extensions; i++) {
            //if(!has_5_vertices_on_distance_at_most_three(bent_zero_extensions[i][0],
            //        bent_zero_extensions[i][3]))
            if(will_remain_valid_ipr_reduction(i)) {
                contains_isolated_L1_or_B00_reductions = 1;
                break;
            }
        }
        if(straight_length == 3 && !contains_isolated_L1_or_B00_reductions)
            for(i = 0; i < num_straight_extensions; i++)
                if(!has_5_vertices_on_distance_at_most_three(straight_extensions[i][0],
                        straight_extensions[i][2])) {
                    contains_isolated_L1_or_B00_reductions = 1;
                    break;
                }

        if(contains_isolated_L1_or_B00_reductions) {
            max_pathlength_straight = 5;
            if(!startswitch) {
                if(nv + max_pathlength_straight == maxnv - 1)
                    max_pathlength_straight--;
                //For IPR each operation adds at least 3 faces
                if(nv + max_pathlength_straight == maxnv - 2)
                    max_pathlength_straight--;
            }
        }

    }

    //Max number of straight expansions for given pathlength is 12 * 5 / 2 * 2 = 60

    EDGE * ext_L1[MAX_EXTENSIONS_SINGLE];
    int ext_L1_use_next[MAX_EXTENSIONS_SINGLE];

    //Max number of bent expansions for given length is 12 * 5 / 2 * 2 = 60
    EDGE * ext_bent_zero[MAX_EXTENSIONS_SINGLE];
    int ext_bent_zero_use_next[MAX_EXTENSIONS_SINGLE];

    /*
     * Number of combinations for given max_length_bent: 0, 2, 5, 9, 14, 20 = n(n+3)/2.
     * Remark: could use sharper boundary: 1, 3, 5, 8, 11, 15, 19, 24,...
     *
     * Distance between 2 deg 5 vertices after applying Bij is i+j+2.
     * So i+j must be <= max_pathlength - 2 - 1
     */
    int max_straight_extensions, max_bent_extensions;
    if(max_pathlength_straight > 3) {
        max_straight_extensions = max_pathlength_straight - 3;
        max_bent_extensions = (max_pathlength_straight - 3) * max_pathlength_straight / 2;
    } else
        max_straight_extensions = max_bent_extensions = 0;

    //+1 to avoid array of size 0
    EDGE * ext_straight[max_straight_extensions * MAX_EXTENSIONS_SINGLE + 1];
    int ext_straight_length[max_straight_extensions * MAX_EXTENSIONS_SINGLE + 1];
    int ext_straight_use_next[max_straight_extensions * MAX_EXTENSIONS_SINGLE + 1];

    /*
     * Creation fails if max_bent_extensions becomes too big (which would cause a segmentation fault).
     * Therefore using malloc. But instead of making new arrays each time, we
     * keep a global list of bent expansions and then copy it to local arrays
     * (which are much smaller as there are only a very limited number of valid
     * bent expansions).
     */
/*
    EDGE * ext_bent[max_bent_extensions * MAX_EXTENSIONS_SINGLE + 1];
    int ext_bent_position[max_bent_extensions * MAX_EXTENSIONS_SINGLE + 1];
    int ext_bent_length[max_bent_extensions * MAX_EXTENSIONS_SINGLE + 1];
    int ext_bent_use_next[max_bent_extensions * MAX_EXTENSIONS_SINGLE + 1];
*/

/*
    EDGE **ext_bent = (EDGE **) malloc(sizeof(EDGE*) * (max_bent_extensions * MAX_EXTENSIONS_SINGLE + 1));
    if(ext_bent == NULL) {
        fprintf(stderr, "Error: can't get enough memory\n");
        exit(1);
    }
    int *ext_bent_position = (int *) malloc(sizeof(int) * (max_bent_extensions * MAX_EXTENSIONS_SINGLE + 1));
    if(ext_bent_position == NULL) {
        fprintf(stderr, "Error: can't get enough memory\n");
        exit(1);
    }    
    int *ext_bent_length = (int *) malloc(sizeof(int) * (max_bent_extensions * MAX_EXTENSIONS_SINGLE + 1));
    if(ext_bent_length == NULL) {
        fprintf(stderr, "Error: can't get enough memory\n");
        exit(1);
    } 
    int *ext_bent_use_next = (int *) malloc(sizeof(int) * (max_bent_extensions * MAX_EXTENSIONS_SINGLE + 1));
    if(ext_bent_use_next == NULL) {
        fprintf(stderr, "Error: can't get enough memory\n");
        exit(1);
    }    
*/

    int num_ext_L1, num_ext_bent_zero, num_ext_straight, num_ext_bent, xnbtot, xnbop;
    int i, j, test_canon;
    EDGE *work_edge;
    

#ifndef FIND_EXTENSIONS_SIMPLE_FULLER_IPR
#define FIND_EXTENSIONS_SIMPLE_FULLER_IPR find_extensions_fuller_ipr
#endif

    FIND_EXTENSIONS_SIMPLE_FULLER_IPR(nbtot,nbop,max_pathlength_straight,
            ext_L1, &num_ext_L1, ext_L1_use_next,
            ext_bent_zero, &num_ext_bent_zero, ext_bent_zero_use_next,
            ext_straight, &num_ext_straight, ext_straight_length, ext_straight_use_next,
            //ext_bent_global, &num_ext_bent_global, ext_bent_position_global, ext_bent_length_global, ext_bent_use_next_global);
            NULL, &num_ext_bent_global, NULL, NULL, NULL);
    
    num_ext_bent = num_ext_bent_global;
    //+1 to avoid arrays of size 0
/*
    EDGE * ext_bent[num_ext_bent + 1];
    int ext_bent_position[num_ext_bent + 1];
    int ext_bent_length[num_ext_bent + 1];
    int ext_bent_use_next[num_ext_bent + 1];
*/
    //Using malloc just in case array would be too big...
    EDGE **ext_bent = (EDGE **) malloc(sizeof(EDGE*) * (num_ext_bent + 1));
    if(ext_bent == NULL) {
        fprintf(stderr, "Error: can't get enough memory\n");
        exit(1);
    }
    int *ext_bent_position = (int *) malloc(sizeof(int) * (num_ext_bent + 1));
    if(ext_bent_position == NULL) {
        fprintf(stderr, "Error: can't get enough memory\n");
        exit(1);
    }    
    int *ext_bent_length = (int *) malloc(sizeof(int) * (num_ext_bent + 1));
    if(ext_bent_length == NULL) {
        fprintf(stderr, "Error: can't get enough memory\n");
        exit(1);
    }
    int *ext_bent_use_next = (int *) malloc(sizeof(int) * (num_ext_bent + 1));
    if(ext_bent_use_next == NULL) {
        fprintf(stderr, "Error: can't get enough memory\n");
        exit(1);
    }
    
    if(num_ext_bent > 0) {
        memcpy(ext_bent, ext_bent_global, sizeof(EDGE *) * num_ext_bent);
        memcpy(ext_bent_position, ext_bent_position_global, sizeof(int) * num_ext_bent);
        memcpy(ext_bent_length, ext_bent_length_global, sizeof(int) * num_ext_bent);
        memcpy(ext_bent_use_next, ext_bent_use_next_global, sizeof(int) * num_ext_bent);
    }
    
    DEBUGASSERT(num_ext_L1 <= MAX_EXTENSIONS_SINGLE);
    DEBUGASSERT(num_ext_bent_zero <= MAX_EXTENSIONS_SINGLE);
    DEBUGASSERT(num_ext_straight <= max_straight_extensions * MAX_EXTENSIONS_SINGLE);
    DEBUGASSERT(num_ext_bent <= max_bent_extensions * MAX_EXTENSIONS_SINGLE);
    
    EDGE *startedge_L1_reductions_parent[MAX_PREV_EXTENSIONS];
    int num_L1_reductions_parent = 0;
    if(straight_length == 3) {
        for(i = 0; i < num_straight_extensions; i++)
            startedge_L1_reductions_parent[i] = edge_list[straight_extensions[i][0]][straight_extensions[i][1]];
        num_L1_reductions_parent = num_straight_extensions;
    }
    
    /* Perform straight expansions */
    int path[max_pathlength_straight + 1];
    int parallel_path[max_pathlength_straight + 1];

    EDGE *good_edges_prev[MAX_EXTENSIONS_SINGLE], *good_edges_next[MAX_EXTENSIONS_SINGLE];
    int num_good_prev, num_good_next, can_edges_next, can_edges_prev;


    /* Perform L1 expansions */
    for(i = 0; i < num_ext_L1; ++i) {
        work_edge = ext_L1[i];
        DEBUGASSERT(3 <= max_pathlength_straight);
        for(j = 0; j < 3 + 1; j++) {
            path[j] = work_edge->start;
            if(ext_L1_use_next[i]) {
                parallel_path[j] = work_edge->prev->end;
                work_edge = work_edge->invers->next->next->next;
            } else {
                parallel_path[j] = work_edge->next->end;
                work_edge = work_edge->invers->prev->prev->prev;
            }
        }

        extend_straight(ext_L1[i], path, parallel_path, 3, ext_L1_use_next[i]);
        
        if(is_best_straight_reduction_ipr(edge_list[nv - 3][nv - 2],
                edge_list[nv - 1][nv - 2], 3, ext_L1_use_next[i],
                good_edges_next, &num_good_next, &can_edges_next, good_edges_prev, &num_good_prev, &can_edges_prev)) {
            test_canon = 1;
            if(num_good_next + num_good_prev == 1) {
                test_canon = 0;
                xnbtot = xnbop = 1;
            }
            
            if(!test_canon || canon_edge_oriented(good_edges_next, num_good_next, can_edges_next,
                    good_edges_prev, num_good_prev, can_edges_prev, degree, numbering, &xnbtot, &xnbop)) {
                if(nv < maxnv - 2) {
                    straight_length = 3;
                    num_bent_zero_extensions = 0;

                    /*
                     * At the moment only doing this in case of IPR.
                     * If this method would be a bottleneck, then never add
                     * the last reduction, since that's the only one which
                     * can be duplicate.
                     */
                    if(!is_already_in_list(edge_list[nv - 3][nv - 2], 3)) {
                        for(j = 0; j < straight_length; j++)
                            straight_extensions[num_straight_extensions][j] = nv - straight_length + j;
                        straight_extensions[num_straight_extensions][j++] = path[0];
                        straight_extensions[num_straight_extensions][j++] = parallel_path[straight_length];

                        num_straight_extensions++;
                    }
                } 
                scansimple_fuller_ipr(xnbtot, xnbop);
            }

        }

        reduce_straight(path, parallel_path, 3);

    }

    /* Perform B00 expansions */
    int path_b00[5];
    int parallel_path_b00[3];

    for(i = 0; i < num_ext_bent_zero; ++i) {
        work_edge = ext_bent_zero[i];
        path_b00[0] = work_edge->start;
        path_b00[1] = work_edge->end;
        work_edge = work_edge->invers;
        if(ext_bent_zero_use_next[i]) {
            parallel_path_b00[0] = work_edge->next->end;
            work_edge = work_edge->next->next->next;
        } else {
            parallel_path_b00[0] = work_edge->prev->end;
            work_edge = work_edge->prev->prev->prev;
        }
        path_b00[2] = work_edge->end;
        work_edge = work_edge->invers;
        if(ext_bent_zero_use_next[i]) {
            parallel_path_b00[1] = work_edge->next->end;
            work_edge = work_edge->next->next;
        } else {
            parallel_path_b00[1] = work_edge->prev->end;
            work_edge = work_edge->prev->prev;
        }
        path_b00[3] = work_edge->end;
        work_edge = work_edge->invers;
        if(ext_bent_zero_use_next[i]) {
            parallel_path_b00[2] = work_edge->next->next->end;
            work_edge = work_edge->next->next->next;
        } else {
            parallel_path_b00[2] = work_edge->prev->prev->end;
            work_edge = work_edge->prev->prev->prev;
        }
        path_b00[4] = work_edge->end;

        extend_bent_zero(ext_bent_zero[i], path_b00, parallel_path_b00, ext_bent_zero_use_next[i]);

        if(!contains_L1_reduction_ipr(startedge_L1_reductions_parent, num_L1_reductions_parent)) {
            if(is_best_bent_zero_reduction_ipr(edge_list[nv - 3][path_b00[0]], edge_list[nv - 1][path_b00[4]], ext_bent_zero_use_next[i],
                    good_edges_next, &num_good_next, &can_edges_next, good_edges_prev, &num_good_prev, &can_edges_prev)) {
                test_canon = 1;
                if(num_good_next + num_good_prev == 1) {
                    test_canon = 0;
                    xnbtot = xnbop = 1;
                }

                if(!test_canon || canon_edge_oriented(good_edges_next, num_good_next, can_edges_next,
                        good_edges_prev, num_good_prev, can_edges_prev, degree, numbering, &xnbtot, &xnbop)) {

                    if(nv < maxnv - 2) {
                        straight_length = MAX_STRAIGHT_LENGTH + 1;
                        add_bent_zero_extension_to_list_ipr(edge_list[nv - 3][path_b00[0]], ext_bent_zero_use_next[i]);
                    }
                    scansimple_fuller_ipr(xnbtot, xnbop);
                }

            }
        }

        reduce_bent_zero(path_b00, parallel_path_b00);
    }

    /* Perform longer straight expansions */
    for(i = 0; i < num_ext_straight; ++i) {
        if(allowed_to_go_above_splitlevel || nv + ext_straight_length[i] <= splitlevel) {
            work_edge = ext_straight[i];
            DEBUGASSERT(ext_straight_length[i] <= max_pathlength_straight);
            for(j = 0; j < ext_straight_length[i] + 1; j++) {
                path[j] = work_edge->start;
                if(ext_straight_use_next[i]) {
                    parallel_path[j] = work_edge->prev->end;
                    work_edge = work_edge->invers->next->next->next;
                } else {
                    parallel_path[j] = work_edge->next->end;
                    work_edge = work_edge->invers->prev->prev->prev;
                }
            }

            extend_straight(ext_straight[i], path, parallel_path, ext_straight_length[i], ext_straight_use_next[i]);

            if(!contains_shorter_straight_reduction_ipr(startedge_L1_reductions_parent, num_L1_reductions_parent, 
                        ext_straight_length[i] - 3)) {
                if(is_best_straight_reduction_ipr(edge_list[nv - ext_straight_length[i]][nv - ext_straight_length[i] + 1],
                        edge_list[nv - 1][nv - 2], ext_straight_length[i], ext_straight_use_next[i],
                        good_edges_next, &num_good_next, &can_edges_next, good_edges_prev, &num_good_prev, &can_edges_prev)) {
                    test_canon = 1;
                    if(num_good_next + num_good_prev == 1) {
                        test_canon = 0;
                        xnbtot = xnbop = 1;
                    }

                    if(!test_canon || canon_edge_oriented(good_edges_next, num_good_next, can_edges_next,
                            good_edges_prev, num_good_prev, can_edges_prev, degree, numbering, &xnbtot, &xnbop)) {
                        if(nv < maxnv - 2) {
                            straight_length = ext_straight_length[i];
                            num_bent_zero_extensions = 0;

                            /*
                             * At the moment only doing this in case of IPR.
                             * If this method would be a bottleneck, then never add
                             * the last reduction, since that's the only one which
                             * can be duplicate.
                             */
                            if(!is_already_in_list(edge_list[nv - ext_straight_length[i]][nv - ext_straight_length[i] + 1], ext_straight_length[i])) {
                                for(j = 0; j < straight_length; j++)
                                    straight_extensions[num_straight_extensions][j] = nv - straight_length + j;
                                straight_extensions[num_straight_extensions][j++] = path[0];
                                straight_extensions[num_straight_extensions][j++] = parallel_path[straight_length];

                                num_straight_extensions++;
                            }
                        }
                        scansimple_fuller_ipr(xnbtot, xnbop);
                    }
                }
            }

            reduce_straight(path, parallel_path, ext_straight_length[i]);
        }
    }

    /* Perform longer bent expansions */
    /*
     * max_bent_length = max_pathlength_straight - 3
     * Max array size = max_bent_length + 5 = max_pathlength_straight + 2
     */
    int path_bent[max_pathlength_straight + 2];
    //Parallel path is 2 shorter
    int parallel_path_bent[max_pathlength_straight];
    int length, length_parallel;
    for(i = 0; i < num_ext_bent; ++i) {
        if(allowed_to_go_above_splitlevel || nv + ext_bent_length[i] + 3 <= splitlevel) {
            length = length_parallel = 0;
            work_edge = ext_bent[i];
            path_bent[length++] = work_edge->start;
            for(j = 0; j <= ext_bent_position[i]; j++) {
                path_bent[length++] = work_edge->end;
                work_edge = work_edge->invers;
                if(ext_bent_use_next[i]) {
                    parallel_path_bent[length_parallel++] = work_edge->next->end;
                    work_edge = work_edge->next->next->next;
                } else {
                    parallel_path_bent[length_parallel++] = work_edge->prev->end;
                    work_edge = work_edge->prev->prev->prev;
                }
            }
            DEBUGASSERT(length == ext_bent_position[i] + 2);
            DEBUGASSERT(length_parallel == ext_bent_position[i] + 1);

            path_bent[length++] = work_edge->end;
            work_edge = work_edge->invers;
            if(ext_bent_use_next[i]) {
                work_edge = work_edge->next->next;
            } else {
                work_edge = work_edge->prev->prev;
            }

            for(j = ext_bent_position[i]; j <= ext_bent_length[i]; j++) {
                path_bent[length++] = work_edge->end;
                work_edge = work_edge->invers;
                if(ext_bent_use_next[i]) {
                    parallel_path_bent[length_parallel++] = work_edge->next->end;
                    work_edge = work_edge->next->next->next;
                } else {
                    parallel_path_bent[length_parallel++] = work_edge->prev->end;
                    work_edge = work_edge->prev->prev->prev;
                }
            }

            path_bent[length++] = work_edge->end;
            if(ext_bent_use_next[i])
                parallel_path_bent[length_parallel++] = work_edge->prev->end;
            else
                parallel_path_bent[length_parallel++] = work_edge->next->end;
            DEBUGASSERT(length == ext_bent_length[i] + 5);
            DEBUGASSERT(length_parallel == ext_bent_length[i] + 3);

            extend_bent(ext_bent[i], path_bent, parallel_path_bent, ext_bent_position[i],
                    ext_bent_length[i], ext_bent_use_next[i]);
            
            if(!contains_shorter_straight_reduction_ipr(startedge_L1_reductions_parent, num_L1_reductions_parent, 
                            ext_bent_length[i] + 1)) {
                if(is_best_bent_reduction(edge_list[nv - ext_bent_length[i] - 3][path_bent[0]], edge_list[nv - 1][path_bent[ext_bent_length[i] + 4]],
                        ext_bent_position[i], ext_bent_length[i], ext_bent_use_next[i],
                        good_edges_next, &num_good_next, &can_edges_next, good_edges_prev, &num_good_prev, &can_edges_prev)) {
                    test_canon = 1;
                    if(num_good_next + num_good_prev == 1) {
                        test_canon = 0;
                        xnbtot = xnbop = 1;
                    }

                    if(!test_canon || canon_edge_oriented(good_edges_next, num_good_next, can_edges_next,
                            good_edges_prev, num_good_prev, can_edges_prev, degree, numbering, &xnbtot, &xnbop)) {
                        if(nv < maxnv - 2) {
                            straight_length = MAX_STRAIGHT_LENGTH + 1;
                            num_bent_zero_extensions = 0;
                        }
                        scansimple_fuller_ipr(xnbtot, xnbop);
                    }

                }
            }

            reduce_bent(path_bent, parallel_path_bent, ext_bent_position[i], ext_bent_length[i]);
        }
    }
    
    free(ext_bent);
    free(ext_bent_position);
    free(ext_bent_length);
    free(ext_bent_use_next);

}

/**************************************************************************/

/*
 * DFS algorithm to mark all vertices of a cap.
 */
static void 
mark_cap_vertices_recursion(int vertex) {
    DEBUGASSERT(ISMARKED_V2(vertex));
    EDGE *e, *ee;
    e = ee = firstedge[vertex];
    do {
        if(!ISMARKED_V2(e->end) && !ISMARKED_V(e->end)) {
            MARK_V2(e->end);
            mark_cap_vertices_recursion(e->end);
        }
        e = e->next;
    } while (e != ee);
}

/**************************************************************************/

/*
 * Returns 1 if vertex is the center of a 6-cluster of type IV, else returns 0.
 */
static int 
is_center_of_type4_patch(int vertex, int ismarked_cap) {
    DEBUGASSERT(degree[vertex] == 5 && (ISMARKED_V(vertex) || ISMARKED_V2(vertex) == ismarked_cap));

    EDGE *e, *ee, *e_temp;
    e = ee = firstedge[vertex];
    do {
        if(ISMARKED_V2(e->end) != ismarked_cap)
            return 0;

        e_temp = e->invers->next->next->next;
        if((!ISMARKED_V(e_temp->end) && ISMARKED_V2(e_temp->end) != ismarked_cap) || degree[e_temp->end] != 5)
            return 0;
        e = e->next;
    } while (e != ee);

    e = e_temp->invers->next;
    int start = e->start;
    //int end = e->end;
    DEBUGASSERT(degree[start] == 5 && (ISMARKED_V(start) || ISMARKED_V2(start) == ismarked_cap));
    int counter = 0;
    while(1) {
        if((!ISMARKED_V(e->end) && ISMARKED_V2(e->end) != ismarked_cap) || degree[e->end] != 6)
            return 0;

        e = e->invers->next->next->next;

        if((!ISMARKED_V(e->end) && ISMARKED_V2(e->end) != ismarked_cap) || degree[e->end] != 5)
            return 0;

        e = e->invers->next->next;

        counter++;

        if(e->start == start) {
            DEBUGASSERT(e->end == end);
            
            DEBUGASSERT(counter == 5);

            return 1;
        }

    }

}

/**************************************************************************/

/*
 * Returns 1 if the cap with the boundary defined by path contains a cap of
 * type IV, else returns 0.
 */
static int 
cap_contains_type4_patch(int path[], int path_length) {
    //First mark all vertices within the cap
    RESETMARKS_V;
    int i;
    for(i = 0; i < path_length; i++)
        MARK_V(path[i]);

    //Important: both caps need to contain a type 4 patch otherwise fullerene
    //will be reducible after applying ring operation!

    RESETMARKS_V2;
    for(i = 0; i < nv; i++) {
        if(!ISMARKED_V(i)) {
            MARK_V2(i);
            mark_cap_vertices_recursion(i);
            break;
        }
    }

    //Now test if cap contains a patch of type 4
    int j;
    for(j = 0; j < 2; j++) {
        for(i = 0; i < nv; i++)
            if(degree[i] == 5 && (ISMARKED_V(i) || ISMARKED_V2(i) == j) &&
                    is_center_of_type4_patch(i, j)) {
                break;
            }
        if(i == nv)
            return 0;
    }

    return 1;

}

/**************************************************************************/

/**
 * Returns 1 if an (23)^el (32)^m path was found starting from e, else returns 0.
 */
static int 
has_lm_path(EDGE *e, int el, int m, int path[], int *path_length, int outer_path[]) {
    DEBUGASSERT(el > 0);
    *path_length = 0;
    int outer_path_length = 0;

    //Right: e->prev
    //Left: e->next

    RESETMARKS_V;

    //e = e->invers;
    int start = e->start;
    int end = e->end;

    path[(*path_length)++] = e->start;
    path[(*path_length)++] = e->end;

    //Do not mark beginedge

    MARK_V(e->end);

    //IMPORTANT: Had to swap next and prev since starting from different beginedge!
    //i.e. going in reverse direction!

    int i;
    //Go el-1 times (right left)
    for (i = 0; i < el-1; i++) {
        outer_path[outer_path_length++] = e->invers->prev->end;

        //e = e->invers->next->next->next;
        e = e->invers->prev->prev->prev;
        if(ISMARKED_V(e->end))
            return 0;
        else
            MARK_V(e->end);

        if(m > 0 || i < el - 2)
            path[(*path_length)++] = e->end;
    }
    DEBUGASSERT(outer_path_length == el - 1);
    
    outer_path[outer_path_length++] = e->invers->prev->end;

    if(m > 0) {
        DEBUGASSERT(*path_length == el + 1);
        
        outer_path[outer_path_length++] = e->invers->prev->prev->end;

        //Go left
        //e = e->invers->next->next;
        e = e->invers->prev->prev->prev->prev;
        if(ISMARKED_V(e->end))
            return 0;
        else
            MARK_V(e->end);
        
        path[(*path_length)++] = e->end;

        //Go m-1 times (right left)
        for(i = 0; i < m - 1; i++) {
            DEBUGASSERT(m > 1);

            outer_path[outer_path_length++] = e->invers->prev->end;
            //e = e->invers->next->next->next;
            e = e->invers->prev->prev->prev;
            if(ISMARKED_V(e->end))
                return 0;
            else
                MARK_V(e->end);

            if(i < m - 2)
                path[(*path_length)++] = e->end;
        }

        //Go right
        //e = e->invers->next->next->next->next;
        e = e->invers->prev->prev;

    } else {
        DEBUGASSERT(*path_length == el);

        //e = e->invers->next->next->next;
        e = e->invers->prev->prev->prev;
    }

    if(outer_path_length != *path_length) {
        fprintf(stderr, "Error: outer_path_length and path_length are different: %d vs %d\n",
                outer_path_length, *path_length);
        exit(1);
    }

    if(*path_length != el + m) {
        fprintf(stderr, "Error: invalid path length: %d vs %d expected\n", *path_length, (el + m));
        exit(1);
    }

    return (e->start == start) && (e->end == end);

}

/**************************************************************************/

/**
 * Adds a ring of hexagons to the path which contains a bent.
 * Since there is a bent, there is only one possible extension.
 */
static void 
add_ring_bent(int path[], int path_length, int outer_path[], int bent_position) {

    DEBUGASSERT(nv + path_length <= maxnv);

    //int ne_old = ne;

    int i, j;
    EDGE *e;
    //First create the new vertices and edges
    for(i = 0; i < path_length; i++) {
        degree[nv + i] = 0;
        firstedge[nv + i] = NULL;
        for(j = 0; j < 6; j++) {
            NEWEDGE(e);
            ne++;
            degree[nv + i]++;
            e->start = nv + i;
            //e->end = path[i];
            e->invers = NULL;
            if(firstedge[nv + i] == NULL)
                firstedge[nv + i] = e;
            else {
                (e - 1)->next = e;
                e->prev = e - 1;
            }
        }
        firstedge[nv + i]->prev = e;
        e->next = firstedge[nv + i];

        DEBUGASSERT(degree[nv + i] == 6);
    }

    //DEBUGASSERT(ne == (ne_old + 6*path_length));

    e = firstedge[nv];
    e->end = path[0];
    edge_list[e->start][e->end] = e;

    e = e->next;
    e->end = path[1];
    edge_list[e->start][e->end] = e;

    e = e->next;
    e->end = nv + 1;
    edge_list[e->start][e->end] = e;

    e = e->next;
    e->end = outer_path[0];
    edge_list[e->start][e->end] = e;

    e = e->next;
    e->end = nv + path_length - 1;
    edge_list[e->start][e->end] = e;

    e = e->next;
    e->end = path[path_length - 1];
    edge_list[e->start][e->end] = e;


    for(i = 1; i < bent_position; i++) {
        e = firstedge[nv + i];
        e->end = path[i];
        edge_list[e->start][e->end] = e;

        e = e->next;
        e->end = path[i+1];
        edge_list[e->start][e->end] = e;

        e = e->next;
        e->end = nv + i + 1;
        edge_list[e->start][e->end] = e;

        e = e->next;
        e->end = outer_path[i];
        edge_list[e->start][e->end] = e;

        e = e->next;
        DEBUGASSERT(i > 0);
        e->end = outer_path[i-1];
        edge_list[e->start][e->end] = e;

        e = e->next;
        e->end = nv + i - 1;
        edge_list[e->start][e->end] = e;
    }
    DEBUGASSERT(i == bent_position);

    //Bend
    e = firstedge[nv + i];
    e->end = path[i];
    edge_list[e->start][e->end] = e;

    e = e->next;
    e->end = nv + i + 1;
    edge_list[e->start][e->end] = e;

    e = e->next;
    e->end = outer_path[i+1];
    edge_list[e->start][e->end] = e;

    e = e->next;
    e->end = outer_path[i];
    edge_list[e->start][e->end] = e;

    e = e->next;
    e->end = outer_path[i - 1];
    edge_list[e->start][e->end] = e;

    e = e->next;
    e->end = nv + i - 1;
    edge_list[e->start][e->end] = e;
    i++;

    //Not i = 0!
    for(; i < path_length; i++) {
        e = firstedge[nv + i];
        e->end = path[i];
        edge_list[e->start][e->end] = e;

        e = e->next;
        e->end = nv + ((i + 1) % path_length);
        edge_list[e->start][e->end] = e;

        e = e->next;
        e->end = outer_path[(i + 1) % path_length];
        edge_list[e->start][e->end] = e;

        e = e->next;
        DEBUGASSERT(i > 0);
        e->end = outer_path[i];
        edge_list[e->start][e->end] = e;

        e = e->next;
        e->end = nv + i - 1;
        edge_list[e->start][e->end] = e;

        e = e->next;
        e->end = path[i - 1];
        edge_list[e->start][e->end] = e;
    }

    //Now also modify edges of path and outer_path

    replace_neighbour(path[0], outer_path[0], nv);

    for(i = 1; i < bent_position; i++) {
        DEBUGASSERT(i > 0);
        replace_neighbour(path[i], outer_path[i-1], nv + i - 1);
        replace_neighbour(path[i], outer_path[i], nv + i);
    }

    DEBUGASSERT(i == bent_position);
    replace_neighbour(path[i], outer_path[i-1], nv + i - 1);
    replace_neighbour(path[i], outer_path[i], nv + i);
    replace_neighbour(path[i], outer_path[i+1], nv + i + 1);
    i++;

    for(; i < path_length; i++) {
        DEBUGASSERT(i > 0);
        replace_neighbour(path[i], outer_path[i], nv + i);
        replace_neighbour(path[i], outer_path[(i+1) % path_length], nv + ((i + 1) % path_length));
    }


    replace_neighbour(outer_path[0], path[path_length - 1], nv + path_length - 1);
    replace_neighbour(outer_path[0], path[0], nv);
    replace_neighbour(outer_path[0], path[1], nv + 1);

    for(i = 1; i < bent_position; i++) {
        DEBUGASSERT(i > 0);
        replace_neighbour(outer_path[i], path[i], nv + i);
        replace_neighbour(outer_path[i], path[i + 1], nv + i + 1);
    }

    DEBUGASSERT(i == bent_position);
    replace_neighbour(outer_path[i], path[i], nv + i);
    i++;

    for(; i < path_length; i++) {
        DEBUGASSERT(i > 0);
        replace_neighbour(outer_path[i], path[i - 1], nv + i - 1);
        replace_neighbour(outer_path[i], path[i], nv + i);
    }
    
    //Update inverse edges and min
    EDGE *ee;
    for(i = 0; i < path_length; i++) {
        e = firstedge[nv + i];
        for(j = 0; j < degree[nv + i]; j++) {
            //Could use set_inverse_edges but this is slightly less inefficient
            ee = edge_list[e->end][e->start];
            e->invers = ee;
            ee->invers = e;

            e = e->next;
        }
    }

    nv += path_length;

}

/**************************************************************************/

/**
 * Is the inverse of add_ring_bent.
 */
static void 
remove_ring_bent(int path[], int path_length, int outer_path[], int bent_position) {
    //Restore
    nv -= path_length;
    ne -= 6 * path_length;
    
    //No need to restore firstedges, they will be updated by replace_neighbour
    
    /*
     * Also no need to restore edge_list since old values will still be valid:
     * 2 vertices which are not adjacent wont become adjacent after applying an
     * extension.
     */    
    
    //Restore edges
    restore_neighbour(path[0], nv, outer_path[0]);

    int i;
    for(i = 1; i < bent_position; i++) {
        DEBUGASSERT(i > 0);
        restore_neighbour(path[i], nv + i - 1, outer_path[i - 1]);
        restore_neighbour(path[i], nv + i, outer_path[i]);
    }

    DEBUGASSERT(i == bent_position);
    restore_neighbour(path[i], nv + i - 1, outer_path[i - 1]);
    restore_neighbour(path[i], nv + i, outer_path[i]);
    restore_neighbour(path[i], nv + i + 1, outer_path[i + 1]);
    i++;

    for(; i < path_length; i++) {
        DEBUGASSERT(i > 0);
        restore_neighbour(path[i], nv + i, outer_path[i]);
        restore_neighbour(path[i], nv + ((i + 1) % path_length), outer_path[(i + 1) % path_length]);
    }


    restore_neighbour(outer_path[0], nv + path_length - 1, path[path_length - 1]);
    restore_neighbour(outer_path[0], nv, path[0]);
    restore_neighbour(outer_path[0], nv + 1, path[1]);

    for(i = 1; i < bent_position; i++) {
        DEBUGASSERT(i > 0);
        restore_neighbour(outer_path[i], nv + i, path[i]);
        restore_neighbour(outer_path[i], nv + i + 1, path[i + 1]);
    }

    DEBUGASSERT(i == bent_position);
    restore_neighbour(outer_path[i], nv + i, path[i]);
    i++;

    for(; i < path_length; i++) {
        DEBUGASSERT(i > 0);
        restore_neighbour(outer_path[i], nv + i - 1, path[i - 1]);
        restore_neighbour(outer_path[i], nv + i, path[i]);
    }
    
    /* Update inverse edges and min */
    set_inverse_edges(path[0], outer_path[0]);
    
    for(i = 1; i < bent_position; i++) {
        set_inverse_edges(path[i], outer_path[i - 1]);
        set_inverse_edges(path[i], outer_path[i]);
    }

    DEBUGASSERT(i == bent_position);
    set_inverse_edges(path[i], outer_path[i - 1]);
    set_inverse_edges(path[i], outer_path[i]);
    set_inverse_edges(path[i], outer_path[i + 1]);
    i++;

    for(; i < path_length; i++) {
        DEBUGASSERT(i > 0);
        set_inverse_edges(path[i], outer_path[i]);
        set_inverse_edges(path[i], outer_path[(i + 1) % path_length]);
    }    

    FREEEDGES(6 * path_length);
}

/**************************************************************************/

/**
 * Adds a ring of hexagons to the path which contains a bent.
 * Since there is a bent, there is only one possible extension.
 */
static void 
add_ring_bent_dispatch(int path[], int path_length, int outer_path[], int bent_position) {
    
    if(nv + path_length > maxnv)
        return;
    
    int nbtot, nbop, is_new_node;
    
    int type;
    if(bent_position == 2)
        type = IRRED_CAP_2_8;
    else if(bent_position == 8)
        type = IRRED_CAP_8_2;
    else
        type = IRRED_CAP_5_5;
    
    add_ring_bent(path, path_length, outer_path, bent_position);
    
    int codelength = nv + ne + 1;
    unsigned char *can_form = (unsigned char *) malloc(sizeof(unsigned char) * codelength);
    if(can_form == NULL) {
        fprintf(stderr, "Error: malloc of can_form failed\n");
        exit(1);
    }
    canon_form(degree, numbering, &nbtot, &nbop, can_form);
    splay_insert(&worklist, can_form, codelength, type, &is_new_node);    

    if(is_new_node) { //i.e. is canon

        /*
         * Only go above splitlevel if splitcount == 0.
         */
        if(splitcount == 0 || nv <= splitlevel) {
            if(nv < maxnv - 2) {
                DEBUGASSERT(fulleriprswitch);
                straight_length = MAX_STRAIGHT_LENGTH + 1;
                num_bent_zero_extensions = 0;
            }
            scansimple_fuller_ipr(nbtot, nbop);
        }
        
        /*
         * Important: Also go above splitlevel even if splitcount != 0, since the same
         * irreducible ipr fullerene might also be generated in a different branch.
         * So all modulo branches should generate all irreducible fullerenes,
         * otherwise one cannot be certainly a fullerene > splitlevel is canonical.
         * But of course only call scansimple_fuller_ipr() if splitcount == 0.
         */

        //Add more rings (i.e. recursion)
        int outer_path_new[path_length];
        int i;
        for(i = 0; i < path_length; i++)
            outer_path_new[i] = nv - path_length + i;
        add_ring_bent_dispatch(path, path_length, outer_path_new, bent_position);


    }
    
    remove_ring_bent(path, path_length, outer_path, bent_position);

}

/**************************************************************************/

/**
 * Adds a ring of hexagons to the path which contains no bent.
 * One cap is rotated in clockwise direction with a given offset.
 */
static void 
add_ring_no_bent(int path[], int path_length, int outer_path[], int offset) {
    DEBUGASSERT(nv + path_length <= maxnv);
    DEBUGASSERT(offset < path_length);
    
    int i, j;
    EDGE *e;
    //First create the new vertices and edges
    for(i = 0; i < path_length; i++) {
        //degree[nv + i] = 0;
        firstedge[nv + i] = NULL;
        for(j = 0; j < 6; j++) {
            NEWEDGE(e);
            e->label = ne;
            ne++;
            //degree[nv + i]++;
            e->start = nv + i;
            //e->end = path[i];
            e->invers = NULL;
            if(firstedge[nv + i] == NULL)
                firstedge[nv + i] = e;
            else {
                (e - 1)->next = e;
                e->prev = e - 1;
            }
        }
        firstedge[nv + i]->prev = e;
        e->next = firstedge[nv + i];

        degree[nv + i] = 6;
    }

    //DEBUGASSERT(ne == (ne_old + 6*path_length));

    for(i = 0; i < path_length; i++) {
        e = firstedge[nv + i];
        e->end = path[i];
        edge_list[e->start][e->end] = e;

        e = e->next;
        e->end = path[(i+1) % path_length];
        edge_list[e->start][e->end] = e;

        e = e->next;
        e->end = nv + ((i+1) % path_length);
        edge_list[e->start][e->end] = e;

        e = e->next;
        e->end = outer_path[(i + 1 + offset) % path_length];
        edge_list[e->start][e->end] = e;

        e = e->next;
        e->end = outer_path[(i + offset) % path_length];
        edge_list[e->start][e->end] = e;

        e = e->next;
        e->end = nv + ((i - 1 + path_length) % path_length);
        edge_list[e->start][e->end] = e;
    }


    //Now also modify edges of path and outer_path
    for(i = 0; i < path_length; i++) {
        replace_neighbour(path[i], outer_path[(i - 1 + path_length) % path_length], nv + ((i - 1 + path_length) % path_length));
        replace_neighbour(path[i], outer_path[i], nv + i);
    }

    for(i = 0; i < path_length; i++) {
        //replace_neighbour(outer_path[i], path[i], nv + ((i - 1 + path_length) % path_length));
        //replace_neighbour(outer_path[i], path[(i + 1) % path_length], nv + i);
        
        replace_neighbour(outer_path[i], path[i], nv + ((i - 1 - offset + path_length) % path_length));
        replace_neighbour(outer_path[i], path[(i + 1) % path_length], nv + ((i - offset + path_length) % path_length));        
    }

    //Update inverse edges and min
    EDGE *ee;
    for(i = 0; i < path_length; i++) {
        e = firstedge[nv + i];
        for(j = 0; j < degree[nv + i]; j++) {
            //Could use set_inverse_edges but this is slightly less inefficient
            ee = edge_list[e->end][e->start];
            e->invers = ee;
            ee->invers = e;

/*
            if(e < ee) e->min = ee->min = e;
            else e->min = ee->min = ee;
*/

            e = e->next;
        }
    }

    nv += path_length;

}

/**************************************************************************/

/*
 * Is the inverse of add_ring_no_bent.
 */
static void 
remove_ring_no_bent(int path[], int path_length, int outer_path[], int offset) {
    //Restore
    nv -= path_length;
    ne -= 6 * path_length;
    
    //No need to restore firstedges, they will be updated by replace_neighbour
    
    /*
     * Also no need to restore edge_list since old values will still be valid:
     * 2 vertices which are not adjacent wont become adjacent after applying an
     * extension.
     */    

    //Restore edges
    int i;
    for(i = 0; i < path_length; i++) {
        restore_neighbour(path[i], nv + ((i - 1 + path_length) % path_length), outer_path[(i - 1 + path_length) % path_length]);
        restore_neighbour(path[i], nv + i, outer_path[i]);
    }

    for(i = 0; i < path_length; i++) {
        //restore_neighbour(outer_path[i], nv + ((i - 1 + path_length) % path_length), path[i]);
        //restore_neighbour(outer_path[i], nv + i, path[(i + 1) % path_length]);
        restore_neighbour(outer_path[i], nv + ((i - 1 - offset + path_length) % path_length), path[i]);
        restore_neighbour(outer_path[i], nv + ((i - offset + path_length) % path_length), path[(i + 1) % path_length]);
    }
    
    /* Update inverse edges and min */
    for(i = 0; i < path_length; i++) {
        set_inverse_edges(path[i], outer_path[(i - 1 + path_length) % path_length]);
        set_inverse_edges(path[i], outer_path[i]);
    }

    FREEEDGES(6 * path_length);

}

/**************************************************************************/

/**
 * Adds a ring of hexagons to a cap whose boundary contains no bent.
 * Max_offset determines how much the cap can be rotated at most.
 */
static void 
add_ring_no_bent_dispatch(int path[], int path_length, int outer_path[],
                int max_offset) {
    if(nv + path_length > maxnv)
        return;
    
    //Temp
    int type = path_length == 9 ? IRRED_CAP_9_0 : IRRED_CAP_10_0;
    
    /*
     * The offset determines how much the cap is rotated.
     */
    int i, nbtot, nbop, is_new_node;
    for(i = 0; i < max_offset; i++) {

        add_ring_no_bent(path, path_length, outer_path, i);
        
        int codelength = nv + ne + 1;
        unsigned char *can_form = (unsigned char *) malloc(sizeof(unsigned char) * codelength);
        if(can_form == NULL) {
            fprintf(stderr, "Error: malloc of can_form failed\n");
            exit(1);
        }
        canon_form(degree, numbering, &nbtot, &nbop, can_form);
        splay_insert(&worklist, can_form, codelength, type, &is_new_node);

        if(is_new_node) { //i.e. is canon

            /*
             * Only go above splitlevel if splitcount == 0.
             */
            if(splitcount == 0 || nv <= splitlevel) {
                if(nv < maxnv - 2) {
                    DEBUGASSERT(fulleriprswitch);
                    straight_length = MAX_STRAIGHT_LENGTH + 1;
                    num_bent_zero_extensions = 0;
                }
                scansimple_fuller_ipr(nbtot, nbop);
            }

            /*
             * Also go above splitlevel even if splitcount != 0, since the same
             * irreducible ipr fullerene might also be generated in a different branch.
             * So all modulo branches should generate all irreducible fullerenes,
             * otherwise one cannot be certainly a fullerene > splitlevel is canonical.
             * But of course only call scansimple_fuller_ipr() if splitcount == 0.
             */            

            //Add more rings (i.e. recursion)
            int outer_path_new[path_length];
            int j;
            for(j = 0; j < path_length; j++)
                outer_path_new[j] = nv - path_length + j;
            add_ring_no_bent_dispatch(path, path_length, outer_path_new, max_offset);

        }

        remove_ring_no_bent(path, path_length, outer_path, i);
        
    }

}

/**************************************************************************/

#define RINGSIZE_5_0 5

/*
 * The current graph is assumed to be a (5,0)-type nanotube.
 * As this procedure is entered, nv,ne,degree etc are assumed to be valid.
 * This procedure will first compute the group of the graph and call
 * scansimple_fuller, then add a ring of degree 6 vertices to the nanotube
 * and recursively calls this method and repeats doing this until maxnv is
 * reached.
 * 
 * Important: it is assumed that this is the last construction method which
 * is called, since remove_ring_no_bent() is not called (but could easily do this).
 */
static void
scansimple_and_add_ring() {

    /*
     * Only go above splitlevel if splitcount == 0.
     * If splitcount != 0, it might still be 0 in next iteration
     * (in case nv == splitlevel).
     *
     * Remark: splitlevel is assumed to be < maxnv.
     */
    if(splitcount == 0 || nv <= splitlevel) {
        if(nv < maxnv - 1) {
            straight_length = MAX_STRAIGHT_LENGTH + 1;
            num_bent_zero_extensions = 0;
        }        
        
        int nbtot, nbop;
        compute_group(degree, numbering, &nbtot, &nbop);
        scansimple_fuller(nbtot, nbop);
    } else //Return since these fullerenes can only be generated in one way (not from different modulo branch)
        return;

    //Add ring of degree 6 vertices
    if(nv + RINGSIZE_5_0 <= maxnv) {
        EDGE *e, *ee;
        int i;
        int path[RINGSIZE_5_0], outer_path[RINGSIZE_5_0], path_length;
        for(i = 0; i < nv; i++) {
            e = ee = firstedge[i];
            do {
                if(has_lm_path(e, RINGSIZE_5_0, 0, path, &path_length, outer_path)) {
                    DEBUGASSERT(path_length == RINGSIZE_5_0);

                    //Offset doesn't matter since the caps are symmetrical
                    add_ring_no_bent(path, path_length, outer_path, 0);

                    scansimple_and_add_ring();

                    //Restore not necessary as this is the last method which is called
                    //But could certainly call remove_ring_no_bent() if necessary.

                    return;
                }
                e = e->next;
            } while(e != ee);
        }
        fprintf(stderr, "Error: no %d,0 boundary found\n", RINGSIZE_5_0);
        exit(1);
    }
}

/**************************************************************************/

#define MIN_RINGSIZE_IPR 9
#define MAX_RINGSIZE_IPR 10

/*
 * The current graph is assumed to be a type 5,0 nanotube.
 * As this procedure is entered, nv,ne,degree etc are assumed to be valid.
 * This procedure will first compute the group of the graph and call
 * scansimple_fuller, then add a ring of degree 6 vertices to the nanotube
 * and recursively calls this method and repeats doing this until maxnv is
 * reached.
 */
static void
add_rings_irreducible_ipr() {

    DEBUGASSERT(nv <= maxnv);

    /*
     * Only go above splitlevel if splitcount == 0.
     */
    if(splitcount == 0 || nv <= splitlevel) {
        if(nv < maxnv - 2) {
            DEBUGASSERT(fulleriprswitch);
            straight_length = MAX_STRAIGHT_LENGTH + 1;
            num_bent_zero_extensions = 0;
        }

        int nbtot, nbop;
        compute_group(degree, numbering, &nbtot, &nbop);
        scansimple_fuller_ipr(nbtot, nbop);
    } 
    //else
    //    return;

    /*
     * Also go above splitlevel even if splitcount != 0, since the same
     * irreducible ipr fullerene might also be generated in a different branch.
     * So all modulo branches should generate all irreducible fullerenes,
     * otherwise one cannot be certainly a fullerene > splitlevel is canonical.
     * But of course only call scansimple_fuller_ipr() if splitcount == 0.
     */    
    
    //Ring operation adds at least MIN_RINGSIZE_IPR 6-vertices
    if(nv + MIN_RINGSIZE_IPR <= maxnv) {
        EDGE *e, *ee;
        int i;
        int path[MAX_RINGSIZE_IPR], outer_path[MAX_RINGSIZE_IPR], path_length;
        for(i = 0; i < nv; i++) {
            e = ee = firstedge[i];
            do {
                //Remark: it is assumed that the fullerene contains caps of one of the 4 irreducible types
                if(has_lm_path(e, 5, 5, path, &path_length, outer_path)) {
                    add_ring_bent_dispatch(path, path_length, outer_path, 5);

                    //Stop since other 5,5 extensions will yield the same fullerene
                    return;
                }

                if(has_lm_path(e, 2, 8, path, &path_length, outer_path)) {
                    add_ring_bent_dispatch(path, path_length, outer_path, 2);

                    //Stop since other 2,8 extensions will yield the same fullerene
                    return;
                }

                if(has_lm_path(e, 8, 2, path, &path_length, outer_path)) {
                    add_ring_bent_dispatch(path, path_length, outer_path, 8);

                    //Stop since other 8,2 extensions will yield the same fullerene
                    return;
                }

                if(has_lm_path(e, 9, 0, path, &path_length, outer_path)) {
                    /*
                     * For cap III offset 0 and 1 are sufficient (i.e. only
                     * 2 possibilities).
                     */
                    add_ring_no_bent_dispatch(path, path_length, outer_path, 2);

                    return;
                }

                if(has_lm_path(e, 10, 0, path, &path_length, outer_path)) {
                    
                    /* 
                     * Is necessary because there are multiple caps with a 10 0 boundary
                     * But the type 4 patch is the only irreducible one
                     */
                    if(cap_contains_type4_patch(path, path_length)) {
                        //Remark: it is assumed that both caps contain a IV-cap
                        /*
                         * For cap IV offset 0 and 1 are also sufficient, but in fact
                         * offset is only required here if current number of rings is odd.
                         * But this is not a problem, since this method hardly uses any cpu time.
                         */
                        add_ring_no_bent_dispatch(path, path_length, outer_path, 2);

                        return;
                    }
                }

                e = e->next;
            } while(e != ee);
        }

        fprintf(stderr, "Error: no boundary of an irreducible cap found!\n");
        exit(1);

    }
    
}

/***********************************************************************/

static int
getswitchvalue(char *arg, int *pj)

/* Find integer value for switch.
   arg is a pointer to a command-line argument.
   pj is an index into arg, which is updated.
   The value of the switch is the function return value.
   For example, if arg="-xyz1432q" and *pj=3 (pointing at "z"),
       the value 1432 is returned and *pj=7 (pointing at "2").
   An absent value is equivalent to 0.
*/

{
    int j,ans;

    ans = 0;
    for (j = *pj; arg[j+1] >= '0' && arg[j+1] <= '9'; ++j)
        ans = ans * 10 + (arg[j+1] - '0');

    *pj = j;
    return ans;
}

/****************************************************************************/

static void
check_switch(char sw, char *ok_switches)

/* If ok_switches[sw] is zero, write an error message and exit. */

{
    if (!ok_switches[sw])
    {
	fprintf(stderr,">E %s:  -%c is not permitted\n",cmdname,sw);
	exit(1);
    }
}

/****************************************************************************/

static void
decode_command_line(int argc, char *argv[])

/* Decode the command line, setting the global variables which
   give the switch values.  Some basic checking is done too, but
   the most detailed checking is done later.  If an error is
   found, this procedure never returns.

   The values for numerical parameters (minconnec, minimumdeg,
   edgebound[0..1], maxfacesize, polygonsize are set as -1 if the
   parameter is not mentioned, 0 if it appears without an integer
   following, and the given integer value if there is one.  Negative
   values are not currently allowed.
*/

{
    int i,j,ares,amod;
    char *arg,*as;
    int badargs,argsgot;
    char ok_switches[256];

    for (i = 0; i < argc ; ++i) fprintf(stderr,"%s ",argv[i]);
    fprintf(stderr,"\n");

    cmdname = argv[0];

    for (i = 0; i < 256; ++i) ok_switches[i] = 0;
    for (as = SWITCHES; *as != '\0'; ++as) ok_switches[*as] = 1;
    ok_switches['['] = ok_switches[']'] = ok_switches[' '] = 0;
    ok_switches[':'] = ok_switches['-'] = ok_switches['#'] = 0;
    for (as = SECRET_SWITCHES; *as != '\0'; ++as) ok_switches[*as] = 1;

    argsgot = 0;
    badargs = FALSE;
    outfilename = NULL;
    aswitch = FALSE;
    gswitch = FALSE;
    sswitch = FALSE;
    hswitch = FALSE;
    oswitch = FALSE;
    dswitch = FALSE;
    uswitch = FALSE;
    vswitch = FALSE;
    qswitch = FALSE;
    fulleriprswitch = FALSE;
    startswitch = FALSE;
    spiralcheck = FALSE;
    zeroswitch = FALSE;
    res = 0; mod = 1;
    start_output = 0;

    for (i = 1; !badargs && i < argc; ++i)
    {
        arg = argv[i];
        if (arg[0] == '-' && arg[1] != '\0')
        {
        for (j = 1; arg[j] != '\0'; ++j)
	    if (arg[j] == '\0') { }
	    BOOLSWITCH('o',oswitch)
	    BOOLSWITCH('d',dswitch)
	    BOOLSWITCH('h',hswitch)
	    BOOLSWITCH('a',aswitch)
	    BOOLSWITCH('g',gswitch)
	    BOOLSWITCH('s',sswitch)
	    BOOLSWITCH('u',uswitch)
	    BOOLSWITCH('v',vswitch)
            BOOLSWITCH('V',Vswitch)
            BOOLSWITCH('q',qswitch)
            BOOLSWITCH('I',fulleriprswitch)
            BOOLSWITCH('r',spiralcheck)
	    BOOLSWITCH('0',zeroswitch)
            else if (arg[j]=='S') {CHECKSWITCH('S'); startswitch = TRUE; start_output = getswitchvalue(arg,&j);}
#ifdef PLUGIN_SWITCHES
            PLUGIN_SWITCHES
#endif
            else
	    {
		CHECKSWITCH(arg[j]);
                badargs = TRUE;
	    }
        }
        else if (argsgot >= 3)
            badargs = TRUE;
        else if (argsgot == 0)
        {
            j = -1;
            maxnv = getswitchvalue(arg,&j);
            if (arg[j+1] == 'd' && arg[j+2] == '\0')
                if (maxnv & 1)
		{
		    fprintf(stderr,">E %s: n with 'd' must be even\n",cmdname);
		    exit(1);
		}
                else  maxnv = maxnv / 2 + 2;
            else if (arg[j+1] != '\0')
                badargs = TRUE;
            ++argsgot;
        }
        else
        {
            if (arg[0] == '-')
            {
                if (argsgot == 0) badargs = TRUE;
            }
            else if (sscanf(arg,"%d/%d",&ares,&amod) == 2)
            {
                res = ares;
                mod = amod;
            }
            else
                outfilename = arg;
            ++argsgot;
        }
    }

    if (argsgot == 0) badargs = TRUE;

    if (badargs)
    {
        fprintf(stderr,
            ">E Usage: %s %s n [res/mod] [outfile]\n",cmdname,SWITCHES);
        exit(1);
    }

    if (res < 0 || res >= mod)
    {
        fprintf(stderr,">E %s: must have 0 <= res < mod\n",cmdname);
        exit(1);
    }

}

/****************************************************************************/

static void
initialize_splitting(int minlevel, int hint, int maxlevel)

/* Set splitlevel and splitcount.  minlevel and maxlevel are bounds
   on its value.  It must be that both minlevel and maxlevel are at
   least equal to the largest starting order (nv for external calls
   to scansimple() or similar routines), and at most equal to the
   smallest parent of a parent of an internal-output graph (call
   from scansimple() or similar to got_one() or similar).  The size
   of internal-output graphs is maxnv-1 for planar triangulations
   and maxnv for other classes.

   hint is a desirable value, which can be anything as the actual
   value used is forced between minlevel and maxlevel.  For plugins,
   the value of splithint is used instead if it is >= 0.

   In case there is no way to use splitting within those limits,
   it is turned off by setting splitlevel=0.  In that case only
   subcase 0 should produce output.

   Splitting occurs at the first level where nv >= splitlevel.
   If an operation can add k vertices, it must be that
   splitlevel <= maxnv - 5.
*/
{
    splitlevel = hint;
#ifdef PLUGIN
    if (splithint >= 0) splitlevel = splithint;
#endif

    if (splitlevel > maxlevel) splitlevel = maxlevel;

    if (splitlevel < minlevel && splitlevel > 0)
    {
	if (minlevel <= maxlevel) splitlevel = minlevel;
	else                      splitlevel = 0;
    }
    if (mod == 1) splitlevel = 0;

    splitcount = res;
}

/****************************************************************************/

static void
open_output_file(void)

/* Open the output file, and write a header if one is called for.
   All the needed information is in global vars.  Also check if
   maxn is too large for this format, and set the global procedure
   variables write_graph() and write_dual_graph().
*/
{
    int nvf;

    if (aswitch + gswitch + sswitch + uswitch >= 2)
    {
        fprintf(stderr,">E %s: -a, -g, -s, -u are incompatible\n",cmdname);
        exit(1);
    }

    if (!uswitch)
    {
        nvf = dswitch ? 2*(maxnv-2) : maxnv;
        if((aswitch && nvf > 99)
                || (gswitch && nvf > 255)
                || (sswitch && nvf > 255)
                || (!aswitch && !gswitch && !sswitch && nvf > 255))
        {
            fprintf(stderr,">E %s: n is too large for that output format\n",
                cmdname);
            exit(1);
        }
    }

    msgfile = stderr;
    if (outfilename == NULL)
    {
        outfilename = "stdout";
        outfile = stdout;
    }
    else if ((outfile = fopen(outfilename,
        zeroswitch || aswitch || gswitch || sswitch ? "w" : "wb")) == NULL)
    {
        fprintf(stderr,
          ">E %s: can't open %s for writing\n",cmdname,outfilename);
        perror(">E ");
        exit(1);
    }

    if (zeroswitch)
        write_graph = write_dual_graph = write_digits;
    else if (aswitch)
    {
        write_graph = write_alpha;
        write_dual_graph = write_dual_alpha;
    }
    else if (gswitch)
    {
        write_graph = write_graph6;
        write_dual_graph = write_dual_graph6;
    }
    else if (sswitch)
    {
        write_graph = write_sparse6;
        write_dual_graph = write_dual_sparse6;
    }
    else
    {
        write_graph = write_planar_code;
        write_dual_graph = write_dual_planar_code;
    }

    write_header(outfile);
}

/****************************************************************************/

/*
 * Sets the correct filenames for the outputfiles for the spirals.
 */
static void
make_outputfiles_spirals() {
    sprintf(no_penta_spiral_filename, "No_pentagon_spiral_%d", maxnv);
    sprintf(no_spiral_filename, "No_spiral_%d", maxnv);
    
    char strbuffer[50];
    if(startswitch) {
        sprintf(strbuffer, "_%d", start_output);
        strcat(no_penta_spiral_filename, strbuffer);
        strcat(no_spiral_filename, strbuffer);
    }
    
    if(fulleriprswitch) {
        sprintf(strbuffer, "_ipr");
        strcat(no_penta_spiral_filename, strbuffer);
        strcat(no_spiral_filename, strbuffer);
    }
    
    if(mod > 1) {
        sprintf(strbuffer, "_m_%d_%d", res, mod);
        strcat(no_penta_spiral_filename, strbuffer);
        strcat(no_spiral_filename, strbuffer);        
    }
}

/****************************************************************************/

static void
simple_dispatch(void)

/* All the cases not handled in the other dispatch routines. */
{
    int startingsize,nbtot,nbop,i,hint;

    startingsize = 12;

    CHECKRANGE(maxnv,"n",startingsize,MAXN);
    
    if(startswitch) {
        if(start_output < startingsize || start_output > maxnv) {
            fprintf(stderr, ">E %s: start value is %d, but must be %d <= start < %d\n", cmdname, start_output, startingsize, maxnv);
            exit(1);
        }
        //Disable startswitch -- is slightly faster
        if(start_output == maxnv) {
            startswitch = FALSE;
        }
    } else
        start_output = maxnv;

    strcpy(outtypename,"fullerenes");

    open_output_file();

    if(spiralcheck)
        make_outputfiles_spirals();

    if(fulleriprswitch)
        hint = MIN(MAX(maxnv - 20, 12), MAX_SPLITLEVEL_IPR);
    else
        hint = MIN(MAX(maxnv - 20, 12), MAX_SPLITLEVEL);
    
    //Important: it is assumed by other methods that splitlevel is < maxnv - 1
    //For fulleriprswitch and !startswitch it is assumed that splitlevel is < maxnv - 2
    initialize_splitting(startingsize, hint, maxnv - 3);
    if(mod > 1 && !qswitch) {
        fprintf(stderr, "Splitlevel is %d\n", splitlevel);
    }

    if (splitlevel == 0 && res > 0) return;


    //Remark: already verified before that maxnv isn't too big for outputformat

    //Init fullerenes
    initialize_fuller_arrays();

    if(fulleriprswitch) {
        //Global arrays which dynamically resize which store the bent extensions
        ext_bent_global = (EDGE **) malloc(sizeof(EDGE*) * DEFAULT_MAX_EXT_BENT_SIZE);
        if(ext_bent_global == NULL) {
            fprintf(stderr, "Error: can't get enough memory\n");
            exit(1);
        }
        ext_bent_position_global = (int *) malloc(sizeof(int) * DEFAULT_MAX_EXT_BENT_SIZE);
        if(ext_bent_position_global == NULL) {
            fprintf(stderr, "Error: can't get enough memory\n");
            exit(1);
        }
        ext_bent_length_global = (int *) malloc(sizeof(int) * DEFAULT_MAX_EXT_BENT_SIZE);
        if(ext_bent_length_global == NULL) {
            fprintf(stderr, "Error: can't get enough memory\n");
            exit(1);
        }
        ext_bent_use_next_global = (int *) malloc(sizeof(int) * DEFAULT_MAX_EXT_BENT_SIZE);
        if(ext_bent_use_next_global == NULL) {
            fprintf(stderr, "Error: can't get enough memory\n");
            exit(1);
        }
        
        //nv + ne + 1
        int max_codelength = maxnv + 2 * (maxnv - 2)*3 + 1;

        //Extend irreducible IPR fullerenes with a valid boundary (and no rings)
        for(i = 0; i < NUM_IRRED_IPR_NO_RING; i++) {
            //If nv would be > MAXN errors could occur
            if(code_irred_ipr_lengths[i] > max_codelength)
                continue;

            initialize_fuller_code(code_irred_ipr[i], code_irred_ipr_lengths[i]);

            add_rings_irreducible_ipr();

        }

        //Extend irreducible IPR fullerenes which don't have a valid boundary
        for(i = 0; i < NUM_IRRED_IPR_INVALID_BOUNDARY; i++) {
            //If nv would be > MAXN errors could occur
            if(code_irred_ipr_invalid_boundary_lengths[i] > max_codelength)
                continue;

            initialize_fuller_code(code_irred_ipr_invalid_boundary[i], code_irred_ipr_invalid_boundary_lengths[i]);

            DEBUGASSERT(nv <= maxnv);

            //Is always canonical

            /*
             * Only go above splitlevel if splitcount == 0.
             */
            if(splitcount == 0 || nv <= splitlevel) {
                compute_group(degree, numbering, &nbtot, &nbop);
                scansimple_fuller_ipr(nbtot, nbop);
            }
        }
        
        free(ext_bent_global);
        free(ext_bent_position_global);
        free(ext_bent_length_global);
        free(ext_bent_use_next_global);
        
    } else {

        //nv = 12 and always <= splitlevel
        initialize_fuller_code(code_c20, CODELENGTH_C20);

        compute_group(degree, numbering, &nbtot, &nbop);
        scansimple_fuller(nbtot, nbop);

        if(maxnv >= 16) {
            //unsigned long long int number_generated_before = number_of_graphs_generated[maxnv];

            initialize_fuller_code(code_c28, CODELENGTH_C28);

            /*
             * Only go above splitlevel if splitcount == 0.
             */
            if(splitcount == 0 || nv <= splitlevel) {
                compute_group(degree, numbering, &nbtot, &nbop);
                scansimple_fuller(nbtot, nbop);
            }

            //if(!qswitch)
            //    fprintf(stderr, "Number of fullerenes with %d vertices generated from C28: %llu\n", maxnv, number_of_graphs_generated[maxnv] - number_generated_before);

            if(maxnv >= 17) {
                //number_generated_before = number_of_graphs_generated[maxnv];

                /*
                 * Remark: could also generate the (5,0)-type nanotube fullerenes
                 * by recursively adding a ring of hexagons to C20.
                 */
                initialize_fuller_code(code_c30, CODELENGTH_C30);

                /*
                 * Important: this is assumed to be the last construction
                 * method which is called (since no restore is done).
                 */
                scansimple_and_add_ring();

                //if(!qswitch)
                //    fprintf(stderr, "Number of fullerenes with %d vertices generated from (5,0) nanotubes: %llu\n", maxnv, number_of_graphs_generated[maxnv] - number_generated_before);

            }
        }
    }

}

/****************************************************************************/

static void
unused_functions(void)

/* Don't call this, it is just to avoid warning messages about
 * functions defined but not used. */
{
    (void) numedgeorbits(0,0);
    (void) numfaceorbits(0,0);
    (void) numopfaceorbits(0,0);
    (void) numorbits(0,0);
    (void) numoporbits(0,0);
    (void) numorbitsonface(0,0,NULL);
    (void) make_dual();
    (void) show_group(NULL,0,0);
    check_it(0,0);
    
    (void) get_colour_prev(NULL);
    (void) get_colour_next(NULL);
    (void) canon(NULL, NULL, NULL,NULL);
    (void) canon_edge(NULL, 0, NULL, NULL, NULL, NULL);
    
    print_embedded_graph();
    print_code(NULL, 0);
    write_planar_code_modified(NULL, 0);

    unused_functions();
}

/****************************************************************************/

int
main(int argc, char *argv[])
{
    int i;
#if CPUTIME
    struct tms timestruct0,timestruct1;

    times(&timestruct0);
#endif

    if (argc > 1
        && (strcmp(argv[1],"-help") == 0 || (strcmp(argv[1],"--help") == 0)))
    {
	fprintf(stderr,"Buckygen version %s\n",VERSION);
	fprintf(stderr,
            "Usage: %s %s n [res/mod] [outfile]\n",argv[0],SWITCHES);
#ifdef HELPMESSAGE
	HELPMESSAGE;
#endif
        fprintf(stderr,"See buckygen-guide.txt for more information.\n");
	return 0;
    }

    decode_command_line(argc,argv);

#ifdef SPLITTEST
    if (mod == 1) mod = 2;
    uswitch = TRUE;
    aswitch = gswitch = sswitch = FALSE;
#endif
    
    for (i = start_output; i <= maxnv; ++i)
    {
        ZEROBIG(nout[i]);
    }

    ZEROBIG(totalout);
    ZEROBIG(totalout_op);
    ZEROBIG(nout_V);

#ifdef STATS
    ZEROBIG(total_triv);
    ZEROBIG(total_numrooted);
    for (i = start_output; i <= maxnv; ++i)
    {
        ZEROBIG(numrooted[i]);
        ZEROBIG(ntriv[i]);
    }
#endif

#ifdef SPLITTEST
    ZEROBIG(splitcases);
#endif

#ifdef PLUGIN_INIT
    PLUGIN_INIT;
#endif

    simple_dispatch();

#if CPUTIME
    times(&timestruct1);
#endif
    
    if(qswitch)
        return 0;    

    dosummary = 1;
#ifdef SUMMARY
    nv = maxnv;
    SUMMARY();
#endif
    
    if(spiralcheck) {
        for(i = start_output; i <= maxnv; i++)
            if(number_without_pentagon_spiral[i] > 0)
                fprintf(stderr, "Number of fullerenes with %d vertices which do not have a spiral starting at a pentagon: %llu\n", i, number_without_pentagon_spiral[i]);
        for(i = start_output; i <= maxnv; i++)
            if(number_without_spiral[i] > 0)
                fprintf(stderr, "Number of fullerenes with %d vertices which do not have a spiral: %llu\n", i, number_without_spiral[i]);
    }
    
    for(i = start_output; i <= maxnv; i++) {
        fprintf(stderr, "Number of fullerenes generated with %d vertices: ", i);
        PRINTBIG(stderr, nout[i]);
        fprintf(stderr, "\n");
    }
    
#ifdef SPLITTEST
    PRINTBIG(msgfile,splitcases);
    fprintf(msgfile," splitting cases at level=%d",splitlevel);
#if CPUTIME
    fprintf(msgfile,"; cpu=%.2f sec\n",
	(double)(timestruct1.tms_utime+timestruct1.tms_stime
              -timestruct0.tms_utime-timestruct0.tms_stime) / (double)CLK_TCK);
#else
    fprintf(msgfile,"\n");
#endif
    return 0;
#endif

    if (!dosummary) return 0;

    if (oswitch && vswitch)
    {
	PRINTBIG(msgfile,totalout);
	fprintf(msgfile," isomorphism classes\n");
    }

    PRINTBIG(msgfile,(oswitch ? totalout_op : totalout));
    fprintf(msgfile," %s",outtypename);
    if (uswitch) fprintf(msgfile," generated");
    else         fprintf(msgfile," written to %s",outfilename);
#if CPUTIME
    fprintf(msgfile,"; cpu=%.2f sec\n",
	    (double)(timestruct1.tms_utime+timestruct1.tms_stime
	      -timestruct0.tms_utime+timestruct0.tms_stime) / (double)CLK_TCK);
#else
    fprintf(msgfile,"\n");
#endif
    if (Vswitch)
    {
	fprintf(msgfile,"Suppressed ");
	PRINTBIG(msgfile,nout_V);
	fprintf(msgfile," with trivial group.\n");
    }    

#ifdef STATS
    ZEROBIG(total_numrooted);
    ZEROBIG(total_triv);
    for (i = start_output; i <= maxnv; ++i)
    {
        SUMBIGS(total_numrooted,numrooted[i]);
        SUMBIGS(total_triv,ntriv[i]);
    }    
    
    for(i = start_output; i <= maxnv; i++) {
        fprintf(stderr, "Num rooted maps with %d vertices: ", i);
        PRINTBIG(msgfile, numrooted[i]);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "Total: ");
    PRINTBIG(msgfile, total_numrooted);
    fprintf(msgfile, " rooted maps\n");
    for(i = start_output; i <= maxnv; i++) {
        fprintf(stderr, "Num with %d vertices and trivial group: ", i);
        PRINTBIG(msgfile, ntriv[i]);
        fprintf(stderr, "\n");
    }
    fprintf(msgfile, "Total number with trivial group: ");
    PRINTBIG(msgfile, total_triv);
    fprintf(msgfile, "\n");
#endif

    return 0;
}

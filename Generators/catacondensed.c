#include <stdio.h> 
#include <ctype.h>
#include <string.h>
#include <unistd.h>
/* #include <malloc.h> */

#ifndef NOTIMES
#include <sys/times.h>
#endif //NOTIMES

#if __STDC__
#include <stdlib.h>
#endif

#ifdef __alpha
#define LONGTYPE unsigned long int
#else
#define LONGTYPE unsigned long long int
#endif

#define MAXNV 30            /* the maximum number of vertices -- at most CHAR_MAX-1 */
#define MAXE (6*MAXNV - 12) /* the maximum number of oriented edges */
#define MAXF (2*MAXNV-4)    /* the maximum number of faces */
#define MAXAUTS 12          /* the maximum number of automorphisms */
#define SPLITLEVEL 16       /* which is the level (number of hexagons) where the 
			       option m shall split it into parts */
#define EMPTY (MAXNV+1)
#define UNNAMED 0 /*EMPTY*/
#define OCCUPIED 1

#define Cs 0
#define C2v 1
#define C2h 2
#define D2h 3
#define C3h 4
#define D3h 5
#define C6h 6
#define D6h 7

typedef struct e /* The data type used for edges */
{ 
	int start;         /* vertex where the edge starts */
	int end;           /* vertex where the edge ends */ 
	struct e *prev;    /* previous edge in clockwise direction */
	struct e *next;    /* previous edge in clockwise direction */
	struct e *invers;  /* the edge that is inverse to this one */
	int dummy1, dummy2;  /* ints for temporary use. Every function may use
                                them but may NEVER rely on them not having changed 
				after another function has been called */
	int fwdlbl,bwdlbl;           /* for the canon_label function */
} EDGE;

typedef struct e2 /* The shortened data type used for edges in the triangular net */
{ 
	char *start;        /* char pointer to where the edge starts. The real name in the 
			      net is never needed. This is the name of the vertex mapped
			      here and 0 in case no vertex was already mapped here. It is used
			      as an integer pointer with all edges around one vertex pointing
			      to the same integer, so that modification of the value can be
			      done faster */
	char *end; 
	struct e2 *next;    /* previous edge in clockwise direction */
	struct e2 *nextnext; /* = ->next->next */
	struct e2 *invers;  /* the edge that is inverse to this one */
} EDGE2;



typedef EDGE* PLANMAP[MAXNV]; /* Planmap[i] is an arbitrary edge starting at vertex i. 
                            WHICH edge is chosen is arbitrary. No function can rely 
			    on this edge not changing after something "has been done
			    to the graph" -- when returning, it might be another 
			    arbitrary edge. */

#define FALSE 0
#define TRUE  1

/* Global variables */

int maxnv=0, nv; /* The vertex number of the final graph -- that is: The facenumber of the
                the dual (maxnv) and the temporary vertex number during the construction */

int ne; /* number of ORIENTED edges -- twice the number of unoriented edges */

int C,H; /* the number of C-atoms and H-atoms -- are computed evry time a skeleton is
	    given to next_step */
int CHdifference; /* always equal to 2 times the number of hexagons -2 */
int Cconstant; /* always equal to 5*maxnv+1 */

EDGE *boundary_edge; /* This is always an edge with the boundary on the left side 
			(prev-direction). It may only be altered in delete-vertex and
			add-vertex. It is always the edge leaving the last vertex
			and having the outer face on the left. */

EDGE *boundary_edges[MAXNV]; /* Here the old boundary edges are stored in order to
				  repair the boundary edge value after deletion of a vertex */


LONGTYPE number_of_skeletons=0;
LONGTYPE triv_skeletons=0;
LONGTYPE number_of_labellings=0;
LONGTYPE counter=0; /* The counter how many structures are accepted
				     (helicenes or benzenoids) */
LONGTYPE formula[4*MAXNV+3][2*MAXNV+5]={{0}};
/* formula[x][y] gives the number of structures with chemical formula C_x H_y */
LONGTYPE groupformula[8][4*MAXNV+3][2*MAXNV+5]={{{0}}};
/* The chemical formulas sorted with respect to their groups */
LONGTYPE catas=0; /* the number of catacondensed structures */
int group;

char catacondensed; /* Is the structure catacondensed ? */
int must_be_catacondensed=0;
#define catanumber CHdifference
                /* the number of oriented edges in the dual of a catacondensed structure:
                   2*maxnv-2 -- the same as Chdifference */

PLANMAP map;    /* What it is all about: This contains the graph to be constructed */

int degree[MAXNV]; /* The valency of the vertices */

int components[MAXNV]; /* The number of boundary components of the vertex = the number of
			  times it occurs in the boundary. For degree 6 it is 0, for
			  degree 5 or 1 it is one, for 2,4 it is one or two and for 3
			  it is one, two or three. The entry components[i] gives the 
			  number of components of vertex i for 0<=i<=nv. For larger i it
			  is undefined. */

EDGE *can_numberings[MAXAUTS][MAXE]; /* the canonical numberings */
int number_can_numberings, number_can_numb_or_pres; 
  /* The number of canonical numberings and orientation preserving canonical numberings */

EDGE *new_vertex[MAXNV][6];
                /* When a new vertex is added to the boundary, it is taken from this
		   list. The list contains (pointers to) edges leaving a vertex with some 
		   pre-initialised edges and their invers. new_vertex[a][b] contains
		   an edge starting at vertex a and being pre-initialised to form
		   a star with b edges. new_vertex[x][0] is not used. */

char possible_labels[7][4]; /* possible_labels[i][j] contains the number of labels
			       for a vertex with degree i and j components.
			       Fields corresponding to impossible combinations
			       like possible_labels[6][2] are not initialized, 
			       so they contain a random value */

char label[MAXNV]; /* The labels of the skeleton describing the helicene (1,2 or 
		      maximally 3) interpreted as: 1,2 or 3 boundary edges when
		      occuring on the boundary for the first time after 
		      "boundary_edge" */


int variable_positions[MAXNV]; /* list of vertices, where more than one
				  label is possible */
int number_of_possibilities[MAXNV]; /* how many possibilities are there ? This field is used
				       differently in case of benzenoids and helicenes. For
				       helicenes it gives the number of possibilities at the
				       [i]th variable position -- for benzenoids it gives
				       the number at vertex [i] */
int maxlevel; /* number of variable positions minus 1 */
/* These variables are global in order not to pass them in the recursive call
   of construct_labels */

int global_init; /* a marker whether the canon_label() routine is called for a new 
		    skeleton or the same one as before */
int just_skeletons=0; /* shall just skeletons be generated */
int modulo=0, part;   /* shall the generation be split -- which part is to be generated */
int modulocounter=0;
int pl_code_out=0, bec_out; /* shall planarcode or BEC be written to outfile (stdout) ? */
char logfilename[100];
int just_count=0; /* by default all isomers are really formed in the memory of the computer --
		     this can be switched off */
int benzenoids=0; /* shall just benzenoid structures be generated ? */
int detailed=0; /* shall additional data like the group and the formula becomputed and
		   displayed ? Option: d */
FILE *outfile; /* The file to write data to -- default: stdout */


/* for the embedding in the net in case of benzenoids, the following variables are
   used: */

EDGE2 *edgenet[MAXNV]; /* This is an edge in the net that has the outer face of the 
			   embedding on the right and is the first edge of 
			   the vertex [number] when running around the boundary
			   starting from boundary_edge in clockwise direction. So
			   the direction of the edge is against the running direction.*/

int embed_from_here[MAXNV+1][2]; /* which vertices are embedded from here -- at most two 
				    they are listed in clockwise direction */



int checkedges[MAXNV+1]; /* how many edges must(may) be checked when the vertex is embedded. 
			    The checking starts at the second edge in ->next direction
			    after the referenece edge for the embedding.
			    This is sometimes more than just the number of boundary edges */

EDGE2 *startnet; /* an edge in the center of the net used for embedding the helicenes when 
		    testing for being benzenoid */

int jump[MAXNV]; /* How many edges are BETWEEN the first edge with boundary on the left
		    leaving this vertex and the next one in clockwise direction ? 
		    This field is only valid for vertices with 2 boundary components. */
int jumpname[MAXNV]; /* What is the name of the vertex at the end of that edge ? */


static int canon();


static int markvalue_v = 30000;
static int marks__v[MAXNV+1];
#define RESETMARKS_V {int mki; if ((++markvalue_v) > 30000) \
       { markvalue_v = 1; for (mki=0;mki<MAXNV;++mki) marks__v[mki]=0;}}
#define UNMARK_V(x) (marks__v[x] = 0)
#define ISMARKED_V(x) (marks__v[x] == markvalue_v)
#define MARK_V(x) (marks__v[x] = markvalue_v)

#ifndef NOTIMES
/**************************** HORLOGE.H   *******************************/

/* Sorry to use the french word horloge, but the aim is to avoid 
   conflicts with clock */


typedef struct horloge{
  clock_t start_utime;
  clock_t start_stime;
  clock_t lap_utime;
  clock_t lap_stime;
  struct tms *time_struct;
  int is_started;
  clock_t clock_ticks;
  double ucpu,scpu;
}Horloge;

/* allocation of the structure */
extern Horloge *AllocHorloge(void);
/* initialisation (start of count ) */
extern int InitHorloge(Horloge *clock);
/* compute elapsed time since InitHorloge */
extern int EvalHorloge(Horloge *clock);
/* print time to file *out : CPU, sys et reel */
extern int PrintHorloge(FILE *out,Horloge *clock);
/* print CPU time to file *out */
extern int PrintCPU(FILE *out,Horloge *clock);
/* gives CPU time *
extern double getCPU(Horloge *clock);
 gives system CPU time 
extern double getSCPU(Horloge *clock);*/

/******************************HORLOGE.C*******************************/
 
/*---------------------------------------------------------------------*\

Module:       horloge.c

Date:         12.04.97

Auteur:       G. CAPOROSSI

Description: Fonctions d'allocation et utilisation de l'horloge

\*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*\
Fonction:            AllocHorloge

Parametres Entree: Horloge *clock

Parametres Sortie:  int 1 initialisation reussie
                        0 si problemes de memoire

Description:        alloue la memoire d'une horloge 

\*---------------------------------------------------------------------*/

Horloge *AllocHorloge(void)
{
  Horloge *clock;
 /* allocation de la memoire de l'horloge */
  clock = (Horloge *)malloc(sizeof(Horloge));
  clock->ucpu = 0.0;
  clock->scpu = 0.0;
  clock->clock_ticks = sysconf(_SC_CLK_TCK);
  clock->time_struct = (struct tms *)malloc(sizeof(struct tms));
  if(!clock->time_struct){
    fprintf(stderr,"Allocation Error in Horloge.");
    exit(1);
  }
  return(clock);
}

/*---------------------------------------------------------------------*\
Fonction:            InitHorloge

Parametres Entree: Horloge *clock

Parametres Sortie:  int 1 initialisation reussie
                        0 si problemes de memoire

Description:        initialise une horloge (debut a l'heure courante)

\*---------------------------------------------------------------------*/

int InitHorloge(Horloge *clock)
{

  if(clock && clock->time_struct){
   times((clock->time_struct));
   clock->start_utime = clock->time_struct->tms_utime;
   clock->start_stime = clock->time_struct->tms_stime;
   clock->ucpu = 0.0;
   clock->scpu = 0.0;
  }else{
    fprintf(stderr, "InitHorloge: Unallocated structure!!\n");
    exit(1);
  }
  return(1);
}

/*---------------------------------------------------------------------*\
Fonction:            EvalHorloge

Parametres Entree: Horloge *clock

Parametres Sortie:  int 1 initialisation reussie
                        0 si problemes de memoire

Description:        met a jour l'horloge

\*---------------------------------------------------------------------*/

int EvalHorloge(Horloge *clock)
{
  if(!clock || !clock->time_struct){
   fprintf(stderr,"Clock not allocated\n");
   return 0;
  }  
  times((clock->time_struct));
  clock->lap_utime = clock->time_struct->tms_utime;
  clock->lap_stime = clock->time_struct->tms_stime;
  clock->ucpu = (double)(clock->lap_utime-clock->start_utime)/(double)clock->clock_ticks;
  clock->scpu =  (double)(clock->lap_stime-clock->start_stime)/(double)clock->clock_ticks;
  return(1);
}

/*---------------------------------------------------------------------*\
Fonction:            PrintHorloge

Parametres Entree: Horloge *clock
                   FILE    *out

Parametres Sortie:  int 1 initialisation reussie
                        0 si problemes de memoire

Description:        affiche les temps cpu, sys et reel

\*---------------------------------------------------------------------*/


int PrintHorloge(FILE *out,Horloge *clock)
{
    if(clock->ucpu > 1e-6) {
      fprintf(out,"\tCPU Time        : %f\n",clock->ucpu);
      fprintf(out,"\tSystem Time     : %f\n",clock->scpu);
//      fprintf(out,"\tReal time       : %f\n",clock->dure);
    }else{
      fprintf(out,"\tCPU Time     :Too small to calculate\n");  
      fprintf(out,"\tSystem time    :Too small to calculate\n");  
//      fprintf(out,"\tReal Time      :Too small to calculate\n");
    }
return(0);
}


/*---------------------------------------------------------------------*\
Fonction:            PrintCPU

Parametres Entree: Horloge *clock
                   FILE    *out

Parametres Sortie:  int 1 initialisation reussie
                        0 si problemes de memoire

Description:        affiche les temps cpu,

\*---------------------------------------------------------------------*/


int PrintCPU(FILE *out,Horloge *clock)
{
    if(clock && clock->ucpu > 1e-6) 
      fprintf(out,"\tCPU Time        : %f\n",clock->ucpu);
    else
      fprintf(out," (CPU Time   NA    )\n");  
    
return(0);
}
#endif //NOTIMES

/*****************************WRITEVERTEX*******************************/

void writevertex(int visited[],int level,int radius, EDGE2 *edge)
{
int i;

if ((*edge->start==UNNAMED) || (visited[*edge->start])) return;
(visited[*edge->start])=1;
fprintf(stderr,"Vertex %d is surrounded by:",*edge->start-1);
for (i=0; i<6; i++, edge=edge->next) fprintf(stderr," %d",*(edge->end)-1);
fprintf(stderr,"\n");

if (level<radius)
{ for (i=0; i<6; i++, edge=edge->next)
    writevertex(visited,level+1,radius, edge->invers);
}
}


/******************************WRITENET**********************************/

void writenet()
{

int visited[MAXNV];
int i;

fprintf(stderr,"\n\n The net:\n");

for (i=0; i<MAXNV; i++) visited[i]=0;

writevertex(visited,0,maxnv,startnet);
}

/******************************WRITEGROUP******************************/

void writegroup()


{

#ifdef __alpha

  if (group==Cs) fprintf(stderr,"structure %lu group=Cs\n",counter);
  else if (group==C2h) fprintf(stderr,"structure %lu group=C2h\n",counter);
  else if (group==C3h) fprintf(stderr,"structure %lu group=C3h\n",counter);
  else if (group==C6h) fprintf(stderr,"structure %lu group=C6h\n",counter);
  else if (group==D2h) fprintf(stderr,"structure %lu group=D2h\n",counter);
  else if (group==D3h) fprintf(stderr,"structure %lu group=D3h\n",counter);
  else if (group==D6h) fprintf(stderr,"structure %lu group=D6h\n",counter);
  else if (group==C2v) fprintf(stderr,"structure %lu group=C2v\n",counter);
  else fprintf(stderr,"Unknown group.\n");

#else

  if (group==Cs) fprintf(stderr,"structure %llu group=Cs\n",counter);
  else if (group==C2h) fprintf(stderr,"structure %llu group=C2h\n",counter);
  else if (group==C3h) fprintf(stderr,"structure %llu group=C3h\n",counter);
  else if (group==C6h) fprintf(stderr,"structure %llu group=C6h\n",counter);
  else if (group==D2h) fprintf(stderr,"structure %llu group=D2h\n",counter);
  else if (group==D3h) fprintf(stderr,"structure %llu group=D3h\n",counter);
  else if (group==D6h) fprintf(stderr,"structure %llu group=D6h\n",counter);
  else if (group==C2v) fprintf(stderr,"structure %llu group=C2v\n",counter);
  else fprintf(stderr,"Unknown group.\n");
#endif

}
/******************************WRITELABELS******************************/

void writelabels()

{
int i;

for (i=0; i<nv; i++) fprintf(stderr,"label[%d]=%d\n",i,label[i]);
}



/******************************WRITEAUTOMS*******************************/

void writeautoms()
{
int i,j;

if (number_can_numberings==1) { fprintf(stderr,"Just identity\n");
				return; }
for (i=0; i<ne; i++) can_numberings[0][i]->dummy1=0;

for (i=0; i<ne; i++)
  { if (can_numberings[0][i]->dummy1==0)
      {
	fprintf(stderr,"Equivalent edges: \n");
	for (j=0; j<number_can_numberings; j++) 
	  { fprintf(stderr,"%d->%d \n",can_numberings[j][i]->start,can_numberings[j][i]->end);
	    can_numberings[j][i]->dummy1=1; }
      }
  }
}

/******************************WRITEMAP***********************************/

void writemap()

/* Writes a man-readable representation of map to stderr */

{

EDGE *run;
int i;

fprintf(stderr,"\nNumber of vertices: %d   Number of edges: %d\n",nv,ne);
for (i=0; i<nv; i++)
  { fprintf(stderr,"%d:",i);
    run=map[i];
    fprintf(stderr," %d",run->end);
    for (run=run->next; run!=map[i]; run=run->next) fprintf(stderr," %d",run->end);
    fprintf(stderr,"      degree: %d components: %d\n",degree[i], components[i]);
  }
fprintf(stderr,"\n");
}


/*******************************WRITEEDGE********************************/

void writeedge (EDGE *edge)
{
fprintf(stderr,"Adress: %p\n",edge);
fprintf(stderr,"%d->%d\n",edge->start, edge->end);
fprintf(stderr,"next: %p prev: %p invers: %p\n",edge->next, edge->prev, edge->invers);
fprintf(stderr,"dummy1: %d dummy2: %d fwdlbl: %d bwdlbl: %d\n",
	edge->dummy1, edge->dummy2, edge->fwdlbl, edge->bwdlbl);
}


/******************************WRITEMAP_EXPLICITLY***************************/

void writemap_explicitly()

{

int i;
EDGE *run;

fprintf(stderr,"\nNumber of vertices: %d\n",nv);
for (i=0; i<nv; i++)
  { fprintf(stderr,"%d -- degree: %d  components: %d\n",i,degree[i], components[i]);
    run=map[i];
    writeedge(run);
    for (run=run->next; run!=map[i]; run=run->next) writeedge(run);
  }
fprintf(stderr,"\n");
}

/*****************************WRITEPOSITIONS******************************/

void writepositions(EDGE *starts[], int how_much[][6])
{
int i,j;

fprintf(stderr,"\nPositions:\n");
for (i=0; starts[i]; i++)
  { fprintf(stderr,"%d->%d    ",starts[i]->start, starts[i]->end);
    for (j=0; how_much[i][j]; j++) fprintf(stderr," %d",how_much[i][j]);
    fprintf(stderr,"\n");
  }
}



/******************************WRITECODE***********************************/

void writecode(FILE *fil)

/* Writes the planarcode of map to fil */

{

static FILE *fil0=NULL;
static int first_write=1;
EDGE *run;
int i;

if (fil0==NULL) fil0=fil;
else if (fil0 != fil) 
	 { fprintf(stderr,"Sorry -- just prepared to write to one file per run.\n");
	   exit(1); }
if (first_write) { fprintf(fil0,">>planar_code<<"); first_write=0; }

putc(nv,fil0);
for (i=0; i<nv; i++)
  { 
    run=map[i]; putc(run->end+1,fil0); 
    for (run=run->next; run!=map[i]; run=run->next) { putc(run->end+1,fil0); }
    putc(0,fil0);
  }
}

/****************************FILL_IN**********************************************/

void fill_in(FILE *fil, EDGE *edge, int *number , int prev[], int next[], int how_many)

/* fills in the next field for edge and the prev field for edge->invers->next.
   The code for the how_many valency 2 vertices in between is written to fil. */
{
int thisnumber, nextnumber;

thisnumber=edge->dummy1;
nextnumber=edge->invers->next->dummy1;

if (how_many)
  {
    (*number)++; next[thisnumber]=(*number);
    putc(thisnumber,fil);
    for (how_many--; how_many>0; how_many--)
	      { (*number)++; putc((*number),fil); putc(0,fil);
		 putc((*number)-1,fil);
	      }
    putc(nextnumber,fil); putc(0,fil);
    prev[nextnumber]=(*number); 
	}
else 
{ next[thisnumber]=nextnumber;
  prev[nextnumber]=thisnumber;
}
}


/************************COMPUTE_BEC***********************************/

void compute_bec(char bec[])
{
  char bec_bak[4*MAXNV];
  int pos,
    found[MAXNV],
    improved,
    worse,
    alt,
    lbec,
    se,
    i;

  EDGE *ed;

  for(pos = 0;pos < nv;pos++)
    found[pos] = 0;

  bec[0] = '\0';
  bec_bak[0] = '\0';

  lbec = 0;
  ed = boundary_edge;
  do {
    se = ed->start;
    if(label[se]) {
      if(found[se]) {
	  bec[lbec] = 6 - degree[se] - label[se] + 48;
      } else
	bec[lbec] = label[se] + 48;
      found[se] = 1;
    } else {
      if(components[se] == 1)
	bec[lbec] = 6 - degree[se] + 48;
      else
	bec[lbec] = 49;
    }
    lbec++;
    bec[lbec] = '\0';
    ed = ed->invers->next;
  }while(ed != boundary_edge);

  for(i=lbec;i>=0;i--) bec[i+lbec] = bec[i];

  /* now we have A boundary code but let's make it ordered properly..*/
  improved = 1;
  while(improved) {
    improved = 0;
    alt = 0;

    while(alt < lbec && !improved) {
      alt++;

      if(bec[alt] >= bec[0]) {
	/*
	found an alternative possible start
	try foreward
	*/
	pos = alt;
	worse = 0;
	for(i=0;i<lbec && !improved && !worse;i++) {
	  if(bec[pos+i] > bec[i]) { 
	    improved = 1;
	    for(i=0;i<lbec;i++)
	      bec[i] = bec[alt+i];
	    bec[lbec] = '\0';

	    for(i=lbec;i>=0;i--) bec[i+lbec] = bec[i];
	  }else
	  if(bec[pos+i] < bec[i]) 
	    worse = 1;
	}
	/* now try backward */
	if(!improved) {
	  pos = alt + lbec;
	  worse = 0;
	  for(i=0;i<lbec && !improved && !worse;i++) {
	    if(bec[pos-i] > bec[i]) { 

	      improved = 1;	  
	      strcpy(bec_bak,bec);
	      for(i=0;i<lbec;i++)
		bec[i] = bec_bak[pos-i];
	      bec[lbec] = '\0';

	      for(i=lbec;i>=0;i--) bec[i+lbec] = bec[i];
	    }else
	      if(bec[pos-i] < bec[i]) 
		worse = 1;
	  }
	}
      }
    }
  }
  /* now we found (hopefully) the bec code of the helicene */
  bec[lbec] = '\0';
  return;
}
/*************************WRITE_BEC************************************/

void write_bec(FILE *fil)
{
  char code[4*MAXNV];

  compute_bec(code);
  fprintf(fil,"%s\n",code);
  return;
}


/******************************WRITE_HELICENE_PL***********************************/

void write_helicene_pl(FILE *fil)

/* Writes the planarcode of the helicene to fil */

{

static FILE *fil0=NULL;
static int first_write=1;
EDGE *run;
static EDGE *where[4*MAXNV+2]; /* where[x] is a pointer to an edge with the helicene vertex 
				  number x on the left hand side */
static int i,j, vertex, runvertex, valency, visited[MAXNV];
static int prev[4*MAXNV+2], next[4*MAXNV+2]; /* The vertex previous and next on the boundary
						of the vertex [x] */
static int total_vertices, deg2_vertices, deg3_vertices;


if (fil0==NULL) fil0=fil;
else if (fil0 != fil) 
	 { fprintf(stderr,"Sorry -- just prepared to write to one file per run.\n");
	   exit(1); }
if (first_write) { fprintf(fil0,">>planar_code<<"); first_write=0; }

total_vertices= 5*nv - (ne/2) +1; /* Note that ne is the number of directed edges */
deg3_vertices= 2*nv-2;
deg2_vertices= total_vertices-deg3_vertices;


/* in dummy1 the name of the vertex on the left hand side of the edge is written; 
   dummy2 is 1 if and only if on the left side there is the outer face. */

for (i=0; i<nv; i++)
  { visited[i]=0;
    run=map[i];
    for (j=0; j<degree[i]; j++)
      { run->dummy1=run->dummy2=0;
	run=run->next; }
  }

run=boundary_edge;
do
  { run->dummy2=1; run=run->invers->next; }
while (run != boundary_edge);



for (i=0, vertex=deg2_vertices; i<nv; i++) /* the degree 2 vertices get the smallest numbers,
					    so these ones start at deg2_vertices+1*/
  { run=map[i]; 
    for (j=0; j<degree[i];j++)
      { /* every vertex is on the left hand side of some edge -- so we only do lefts */
	if (run->dummy1==0) /* no vertex on the left side yet */
	  { vertex++; /* new vertex */
	    where[vertex]=run;
	    run->dummy1=vertex;
	    if (run->dummy2==0) /* On the left there is a triangle */
	      { run->prev->invers->dummy1=vertex;
		run->invers->next->dummy1=vertex;
	      }
	  }
	run=run->next;
      }
  }

runvertex=0;
putc(total_vertices,fil);
run=boundary_edge;
do
  { vertex=run->end;
    valency=degree[vertex];
    switch(valency)
      {
      case 1: { fill_in(fil, run, &runvertex, prev, next, 4);
		break; } /* occurs only once in the boundary */
      case 2: { if (components[vertex]==1) fill_in(fil, run, &runvertex, prev, next, 3);
		else
		  {
		    switch(label[vertex])
		      {
		      case 3: { if (!visited[vertex]) 
				  fill_in(fil, run, &runvertex, prev, next, 2);
				else fill_in(fil, run, &runvertex, prev, next, 0);
				break; }
		      case 2: { fill_in(fil, run, &runvertex, prev, next, 1); break; }
		      case 1: { if (visited[vertex]) 
				  fill_in(fil, run, &runvertex, prev, next, 2);
				else fill_in(fil, run, &runvertex, prev, next, 0);
				break; }
		      }
		    visited[vertex]=1;
		  }
		break;
	      }
      case 3: { if (components[vertex]==1) fill_in(fil, run, &runvertex, prev, next, 2);
		else 
		  if (components[vertex]==2) 
		    { 
		      switch(label[vertex])
		      {
		      case 2: { if (visited[vertex]) 
				  fill_in(fil, run, &runvertex, prev, next, 0);
				else fill_in(fil, run, &runvertex, prev, next, 1);
				break; }
		      case 1: { if (visited[vertex]) 
				  fill_in(fil, run, &runvertex, prev, next, 1);
				else fill_in(fil, run, &runvertex, prev, next, 0);
				break; }
		      }
		    }
		  else /*components==3 */ fill_in(fil, run, &runvertex, prev, next, 0);
		visited[vertex]=1;
		break;
	      }
      case 4: { if (components[vertex]==1) fill_in(fil, run, &runvertex, prev, next, 1);
		else fill_in(fil, run, &runvertex, prev, next, 0);
		break;
	      }
      case 5: { fill_in(fil, run, &runvertex, prev, next, 0); break; }
      default: { fprintf(stderr,"Valency %d on the boundary -- error.\n",valency);
		 exit(1); }
      }
    run=run->invers->next;
  } while (run != boundary_edge);


for (i=deg2_vertices+1; i<=total_vertices; i++)
  { run=where[i];
    putc(run->invers->dummy1,fil);
    if (run->dummy2)
      { putc(prev[i],fil); putc(next[i],fil); }
    else { putc(run->prev->dummy1,fil); putc(run->invers->next->invers->dummy1,fil); }
    putc(0,fil);
  }

}



/****************************WRITECOMPONENTS*****************************/

void writecomponents()
{
int i;
fprintf(stderr,"\n Components:\n");
for (i=0; i<nv; i++) fprintf(stderr,"%d:%d ",i,components[i]);
}




/************************CANON_LABEL*************************************/

int canon_label(EDGE *starts[],int or_pres, int total, int init)
{
  int i,j,   /* loop indices */
    ed;      /* number for the edge ref in the list */
  static int list_len;/* how many elements in the list */
  char found[MAXNV]; /* did we already see this one? */
  EDGE *current,     /* currentb edge treated */
    *first_edge;
  int cs;      /* current->start */
  int rj,redj; /* ref_start[j] and ref_ed+/- j*/
  int curlst;  /* index of current in the list */
  int numbs=1, numbs_mirror=0; /* count the orientation preserving/reversing 
                                  automorphisms */
  int found_autom;

  static char ref_start[2*MAXNV], /* ->start from the reference edge for
				    that label in list */
    bak_val[2*MAXNV];          /* 0 if first time encountered 6 - deg[v] 
				  otherwise*/



  if(!or_pres) or_pres = total; /* all automorphisms are orientation preserving */

  if(init) { /* make the structures for a new skeleton */
    for(i=0;i<nv;i++) {
      found[i] = 0;
    }   /* no vertex found so far */
  
    /* first turn to set up ref_start[] and bak_val[] */
    current = starts[0];
    list_len = 0;
    do {
      cs = current->start;
      current->fwdlbl = list_len;
      if(label[cs]) {
	/* set up bak_val[i] = 0 or 6 - degree[cs] to make easily the 
	   reduced BEC later */
	if(found[cs]) {
	  bak_val[list_len] = 6 - degree[cs];
	}else{
	  bak_val[list_len] = 0;
	}
	/* edge from which we are refering in the list */
	ref_start[list_len] = cs;
	found[cs] = 1;
	list_len ++;
      }
      current = current->invers->next;
    } while(current != starts[0]);

    /* set up the fwdlbl and bwdlbl which shows the labels in each
       way */
    current = starts[0];
    while(!label[current->end]) current = current->invers->next;
    current = current->invers;
    curlst = current->next->fwdlbl;
    first_edge = current;
    do {
      if(label[current->start]) {
	current->bwdlbl = current->next->fwdlbl;
	curlst = current->bwdlbl;
      } else
	current->bwdlbl = curlst;
      current = current->next->invers;
    } while(current != first_edge);
  } /* if init */

  /* start the work in general case */

  if(or_pres > 1)
    for(i=1;i<or_pres;i++) {
      found_autom=1;
      ed = starts[i]->fwdlbl;
      for(j=0;j<list_len;j++) {
	if(ed+j >= list_len) ed -= list_len;
	/* make some short cuts */
	rj = ref_start[j];
	redj = ref_start[ed+j];
	/* treat each case separately when comparing...*/
	if(!bak_val[j]) {
	  if(!bak_val[ed+j]) {
	    if(label[rj] < label[redj]) return(0);
	    if(label[rj] > label[redj]) { found_autom=0; break; };
	  }else{
	    if(label[rj] < bak_val[ed+j] - label[redj]) return(0);
	    if(label[rj] > bak_val[ed+j] - label[redj]) { found_autom=0; break; };
	  }
	}else{
	  if(!bak_val[ed+j]) {
	    if(bak_val[j] - label[rj] < label[redj]) return(0);
	    if(bak_val[j] - label[rj] > label[redj]) { found_autom=0; break; };
	  }else{
	    if(bak_val[j] - label[rj] < bak_val[ed+j] - label[redj]) return(0);
	    if(bak_val[j] - label[rj] > bak_val[ed+j] - label[redj]) { found_autom=0; break; };
	  }
	}
      }
      if (found_autom) numbs++;
    }

  /* orientation reversing edges */
  if(or_pres < total)
    for(i=or_pres;i<total;i++) {
      found_autom=1;
      ed = starts[i]->bwdlbl;
      for(j=0;j < list_len;j++) {
	if(ed-j < 0) ed += list_len;
	/* short cuts */
	rj = ref_start[j];
	redj = ref_start[ed-j];
	/* treat separately each case */
	if(!bak_val[j]) {
	  if(!bak_val[ed-j]) {
	    if(label[rj] < label[redj]) return(0);
	    if(label[rj] > label[redj]) { found_autom=0; break; };
	  }else{
	    if(label[rj] < bak_val[ed-j] - label[redj]) return(0);
	    if(label[rj] > bak_val[ed-j] - label[redj]) { found_autom=0; break; };
	  }
	}else{
	  if(!bak_val[ed-j]) {
	    if(bak_val[j] - label[rj] < label[redj]) return(0);
	    if(bak_val[j] - label[rj] > label[redj]) { found_autom=0; break; };
	  }else{
	    if(bak_val[j] - label[rj] < bak_val[ed-j] - label[redj]) return(0);
	    if(bak_val[j] - label[rj] > bak_val[ed-j] - label[redj]) { found_autom=0; break; };
	  }
	}
      }
      if (found_autom) numbs_mirror++;
    }

        if (numbs<numbs_mirror) { i=numbs; numbs=numbs_mirror; numbs_mirror=i; }
	if ((numbs==1) && (numbs_mirror==0)) group=Cs;
	else if ((numbs==1) && (numbs_mirror==1)) group=C2v;
	else if ((numbs==2) && (numbs_mirror==0)) group=C2h;
	else if ((numbs==2) && (numbs_mirror==2)) group=D2h;
	else if ((numbs==3) && (numbs_mirror==0)) group=C3h;
	else if ((numbs==3) && (numbs_mirror==3)) group=D3h;
	else if ((numbs==6) && (numbs_mirror==0)) group=C6h;
	else if ((numbs==6) && (numbs_mirror==6)) group=D6h;
	else { fprintf(stderr,"Problem -- didn't recognize group\n"); exit(1); }

  return(1);
}


/***********************WRITE_UP*****************************************/

void write_up()
{ 

counter++;
 if (detailed) { 
                 groupformula[group][C][H]++;
		 if (catacondensed) catas++; }

if (just_count) return;

if (pl_code_out) write_helicene_pl(outfile);
else if (bec_out) write_bec(outfile);
 }


/***********************CONSTRUCT_LABELS**********************************/

void construct_labels(int level)
{
char i;
char *labelp;

labelp=label+variable_positions[level];
for (i=number_of_possibilities[level]; i != 0 ;i--)
  { *labelp=i;
  if (level==maxlevel) 	{ write_up(); }
    else construct_labels(level+1);
  }
}

/***********************CONSTRUCT_LABELS_SYMM**********************************/

void construct_labels_symm(int level, EDGE *starts[])
{
char i;
char *labelp;

labelp=label+variable_positions[level];
for (i=number_of_possibilities[level]; i != 0 ;i--)
  { 
    *labelp=i;
    if (level==maxlevel) 
      { 
	if (canon_label(starts, number_can_numb_or_pres, number_can_numberings,
			global_init)) 
	  write_up(); 
        global_init=0;}
    else construct_labels_symm(level+1,starts);
  }
}


/*******************************NEXT_STEP*********************************/

void next_step()
{
static int num_vp; /* number of variable positions */
static int i,dummy;
static LONGTYPE num_labels;
static EDGE *starts[MAXAUTS];

if (number_can_numberings==1) triv_skeletons++;
number_of_skeletons++;

if (just_skeletons) return;

 if (detailed)
   { catacondensed= (ne==catanumber);
     C=Cconstant-(ne>>1);
     H=C-CHdifference;;
   }

global_init=1; /* next time the canon_label routine is called, it must
		  be initialised for a new skeleton */

/* compute the positions where various labels are possible */
num_labels=1;
for (i=num_vp=0; i<nv ; i++)
  { dummy=possible_labels[degree[i]][components[i]];
    if (dummy==1) label[i]=0; 
    else 
      { variable_positions[num_vp]=i;
	number_of_possibilities[num_vp]=dummy;
	num_labels *= dummy;
	num_vp++;
      }
  }

number_of_labellings += num_labels;

 if (num_labels==1) { write_up(); }
else {
       maxlevel=num_vp-1; 
       if (number_can_numberings==1) 
	 { if (just_count) { counter += num_labels; 
	                     if (detailed) { groupformula[Cs][C][H] += num_labels;
			                     if (catacondensed) catas += num_labels; }
			     }
                           else { construct_labels(0); }
	 }
       else { /* Look for the orbit of "boundary_edge": */
	      for (i=0; can_numberings[0][i] != boundary_edge; i++);
	      /* write them all to starts: */
	      for (dummy=0; dummy<number_can_numberings; dummy++) 
		starts[dummy]=can_numberings[dummy][i];
	      construct_labels_symm(0,starts);
	    }

     }




}


/******************************EMBED_BENZENOID***************************/

void embed_benzenoid(int *which)

/* This function continues the construction of the labels and 
   simultaneously embeds the structures into the hexagonal net.
   Whenever it embeds a vertex, it checks whether the edges that are assumed 
   to be boundary edges really do not lead to embedded vertices.
   Furthermore all the places that lead to other components than the one
   one comes from, must also be empty, since whenever a vertex is embedded, it 
   is FIRST visited.

 */


{

int vertex, fixvertex, jumpvertex, i, j,test, poss;
EDGE2 *startedge, *startedgenn, *run_net, *buffer, *run2;
char *delete; /* is there an entry that must be reset to UNNAMED */
char *charp;


vertex=*which; 


/* if (vertex==EMPTY) { write_up(); return; } */

startedge=edgenet[vertex];

delete=NULL;

startedgenn=startedge->nextnext;


/* OK -- then let's embed the vertex */



if (number_of_possibilities[vertex]==1) 
  { 
    /* some edgenet[] entries have to be initialised */
    if (components[vertex]==1) 
      { /* One vertex has to be embedded from here: */
	fixvertex=embed_from_here[vertex][0];
	for (i=checkedges[vertex], run_net=startedgenn; i != 1; i--, run_net=run_net->next);
	/* to find the place to insert the vertex */
	buffer=run_net->invers;
	poss=checkedges[fixvertex];
	run2=buffer->nextnext;
	if (poss==3)
	  { if (*run2->end == UNNAMED)
	      { run2=run2->next;
		if (*run2->end == UNNAMED)
		  { run2=run2->next;
		    if (*run2->end == UNNAMED)
		      { edgenet[fixvertex]=buffer;
			delete=buffer->start;
			*delete=OCCUPIED /*+fixvertex*/;
		      } else return;
		   } else return;
	         } else return;
	  }
	else 
	  if (poss==4)
	  { if (*run2->end == UNNAMED)
	      { run2=run2->next;
		if (*run2->end == UNNAMED)
		  { run2=run2->next;
		    if (*run2->end == UNNAMED)
		      { run2=run2->next;
			if (*run2->end == UNNAMED)
			  { edgenet[fixvertex]=buffer;
			    delete=buffer->start;
			    *delete=OCCUPIED /*+fixvertex*/;
			} else return;
		    } else return;
		} else return;
	      } else return;
	  }
	else 
	   if (poss==2)
	      { if (*run2->end == UNNAMED)
		  { run2=run2->next;
		    if (*run2->end == UNNAMED)
		      { edgenet[fixvertex]=buffer;
			delete=buffer->start;
			*delete=OCCUPIED /*+fixvertex*/;
		      } 
		    else return;
		  } 
	      else return;
	      }
	  else 
	     if (poss==1)
		{ if (*run2->end == UNNAMED)
		    { edgenet[fixvertex]=buffer;
		      delete=buffer->start;
		      *delete=OCCUPIED /*+fixvertex*/;
		    } 
		else return;
		}
	else if (poss==0) 
	      { edgenet[fixvertex]=buffer;
		delete=buffer->start;
		*delete=OCCUPIED /*+fixvertex*/;
	      } 


	if (*(which+1)==EMPTY) { write_up(); } else embed_benzenoid(which+1);
      }
    else if (components[vertex]==2)
      { 
	if ((jumpvertex=embed_from_here[vertex][1])>=0)
	  { for (i=checkedges[vertex], run_net=startedgenn; i != 1; i--, run_net=run_net->next);
	    buffer=run_net->invers;
	    for (j=checkedges[jumpvertex], test=1, run2=buffer->nextnext; j ;
		 run2=run2->next, j--) if (*run2->end != UNNAMED) test=0;
	    if (test)
	      { edgenet[jumpvertex]=buffer;
		delete=buffer->start;
		*delete=OCCUPIED /*+jumpvertex*/;
	      }
	    else return;
	  }

	fixvertex=embed_from_here[vertex][0];
	/* This one is ALWAYS embedded after vertex */
	buffer=startedgenn->invers;
	for (j=checkedges[fixvertex], test=1, run2=buffer->nextnext; j ;
	     run2=run2->next, j--) if (*run2->end != UNNAMED) test=0;
	if (test)
	  {
	    edgenet[fixvertex]=buffer;
	    charp=buffer->start;
	    *charp=OCCUPIED /*+fixvertex*/;
	    if (*(which+1)==EMPTY) write_up(); else embed_benzenoid(which+1);
	    *charp=UNNAMED;
	  }
      }
    else /* that is: (components[vertex]==3) -- the other vertices MUST be embedded later 
	    and we know that 3 places must be checked */
      { 
	buffer=startedgenn->invers;
	fixvertex=embed_from_here[vertex][0];
	run2=buffer->nextnext;
	if (*run2->end == UNNAMED)
	  { run2=run2->next;
	    if (*run2->end == UNNAMED)
	      { run2=run2->next;
		if (*run2->end == UNNAMED)
		  {
		    delete=buffer->start;
		    edgenet[fixvertex]=buffer;
		    *delete=OCCUPIED /*+fixvertex*/;
		} else return;
	    } else return;
	} else return;

	fixvertex=embed_from_here[vertex][1];
	buffer=startedgenn->nextnext->invers;

	run2=buffer->nextnext;
	if (*run2->end == UNNAMED)
	  { run2=run2->next;
	    if (*run2->end == UNNAMED)
	      { run2=run2->next;
		if (*run2->end == UNNAMED)
		  {
		    edgenet[fixvertex]=buffer;
		    charp=buffer->start;
		    *charp=OCCUPIED /*+fixvertex*/;
		    if (*(which+1)==EMPTY) write_up(); else embed_benzenoid(which+1);
		    *charp=UNNAMED;
		} 
	    } 
	} 
      } /* end 3 components */
  } /* end 1 possibility */

else
  { /* number of components is always 2 */
    poss=number_of_possibilities[vertex];
    if (poss==3)
      {
	fixvertex=embed_from_here[vertex][0];
	run_net=startedgenn; 
	if (*run_net->invers->nextnext->nextnext->end != UNNAMED)
	  /* At a position neighbouring both the faces where the hexagon is placed for 
	     label 1 and 2 a forbidden neighbour was detected, so only 3 is possible */
	  { run_net=run_net->nextnext;
	    buffer=run_net->invers;
	    run2=buffer->nextnext; 
	    if (*run2->end == UNNAMED)
	      { run2=run2->next;
		if (*run2->end == UNNAMED)
		  { run2=run2->next;
		    if (*run2->end == UNNAMED)
		    { label[vertex]=3;
		      edgenet[fixvertex]=buffer;
		      charp=buffer->start;
		      *charp=OCCUPIED /*+fixvertex*/;
		      if (*(which+1)==EMPTY) write_up(); else embed_benzenoid(which+1);
		      *charp=UNNAMED;
		    }
		  }
	      }
	  }
	else /* that is: this face is OK -- and need not be checked again */
	  { /* OK first try label 1: */
	    buffer=run_net->invers;
	    run2=buffer->nextnext; 
	    if (*run2->end == UNNAMED)
	      { 
		run2=run2->next; 
		if (*run2->end == UNNAMED)
		  { label[vertex]=1;
		    edgenet[fixvertex]=buffer;
		    charp=buffer->start;
		    *charp=OCCUPIED /*+fixvertex*/;
		    if (*(which+1)==EMPTY) write_up(); else embed_benzenoid(which+1);
		    *charp=UNNAMED;
		  }
	      }
	    run_net=run_net->next;
	    buffer=run_net->invers;
	    if (*buffer->nextnext->nextnext->end == UNNAMED)
	      /* otherwise this time the face between position 2 and 3 made a problem */
	      {
		buffer=run_net->invers;
		run2=buffer->nextnext->next; 
		if (*run2->end == UNNAMED)
		  { label[vertex]=2;
		    edgenet[fixvertex]=buffer;
		    charp=buffer->start;
		    *charp=OCCUPIED /*+fixvertex*/;
		    if (*(which+1)==EMPTY) write_up(); else embed_benzenoid(which+1);
		    *charp=UNNAMED;
		  }
		run_net=run_net->next;
		buffer=run_net->invers;
		run2=buffer->nextnext->next; 
		if (*run2->end == UNNAMED)
		  { 
		    run2=run2->next; 
		    if (*run2->end == UNNAMED)
		      { label[vertex]=3;
			edgenet[fixvertex]=buffer;
			charp=buffer->start;
			*charp=OCCUPIED /*+fixvertex*/;
			if (*(which+1)==EMPTY) write_up(); else embed_benzenoid(which+1);
			*charp=UNNAMED;
		      }
		  }
	      }
	    /* else do nothing else */
	  }
      }


    else /* that is: labels 1 and 2 possible */
      {
	if ((jumpvertex=embed_from_here[vertex][1])>=0)
	  {
	    /* First fix the edgenet entry that will stay constant */
	    for (i=checkedges[vertex], run_net=startedgenn; i != 1; i--, run_net=run_net->next);
	    buffer=run_net->invers;
	    for (j=checkedges[jumpvertex], test=1, run2=buffer->nextnext; j ;
		 run2=run2->next, j--) if (*run2->end != UNNAMED) test=0;
	    if (test)
	      { edgenet[jumpvertex]=buffer;
		delete=buffer->start;
		*delete=OCCUPIED /*+jumpvertex*/;
	      }
	    else return;
	  }
	
	/* Now the other one: */
	fixvertex=embed_from_here[vertex][0];
	/* fixvertex must ALWAYS be embedded after vertex */
	
	run_net=startedgenn;  /* One extra ->next for the boundary edge
				 (It must AT LEAST be one) */
	/* Now run_net is correct for the MINIMAL possible label (that is 1) */
	buffer=run_net->invers;
	run_net=run_net->next;
	for (j=checkedges[fixvertex], test=1, run2=buffer->nextnext; j ;
	     run2=run2->next, j--) if (*run2->end != UNNAMED) test=0;
	if (test)
	  
	  { label[vertex]=1;
	    edgenet[fixvertex]=buffer;
	    charp=buffer->start;
	    *charp=OCCUPIED /*+fixvertex*/;
	    if (*(which+1)==EMPTY) write_up(); else embed_benzenoid(which+1);
	    *charp=UNNAMED;
	  }
	buffer=run_net->invers;
	for (j=checkedges[fixvertex], test=1, run2=buffer->nextnext; j ;
	     run2=run2->next, j--) if (*run2->end != UNNAMED) test=0;
	if (test)
	  { label[vertex]=2;
	    edgenet[fixvertex]=buffer;
	    charp=buffer->start;
	    *charp=OCCUPIED /*+fixvertex*/;
	    if (*(which+1)==EMPTY) write_up(); else embed_benzenoid(which+1);
	    *charp=UNNAMED;
	  }
      }
  }



if (delete) *delete=UNNAMED;

}


/******************************EMBED_BENZENOID_SYM***************************/

void embed_benzenoid_sym(int *which, EDGE *starts[])

/* This function continues the construction of the labels and 
   simultaneously embeds the structures into the hexagonal net.
   Whenever it embeds a vertex, it checks whether the edges that are assumed 
   to be boundary edges really do not lead to embedded vertices.
   Furthermore all the places that lead to other components than the one
   one comes from, must also be empty, since whenever a vertex is embedded, it 
   is FIRST visited.

 */


{

int vertex, fixvertex, jumpvertex, i, j,test, poss;
EDGE2 *startedge, *startedgenn, *run_net, *buffer, *run2;
char *delete; /* is there an entry that must be reset to UNNAMED */
char *charp;


vertex=*which; 


if (vertex==EMPTY) { if (canon_label(starts, number_can_numb_or_pres, number_can_numberings,
				       global_init)) write_up();
			 global_init=0;
		     return; }

startedge=edgenet[vertex];

delete=NULL;

startedgenn=startedge->nextnext;


/* OK -- then let's embed the vertex */



if (number_of_possibilities[vertex]==1) 
  { 
    /* some edgenet[] entries have to be initialised */
    if (components[vertex]==1) 
      { /* One vertex has to be embedded from here: */
	fixvertex=embed_from_here[vertex][0];
	for (i=checkedges[vertex], run_net=startedgenn; i != 1; i--, run_net=run_net->next);
	/* to find the place to insert the vertex */
	buffer=run_net->invers;
	poss=checkedges[fixvertex];
	run2=buffer->nextnext;
	if (poss==3)
	  { if (*run2->end == UNNAMED)
	      { run2=run2->next;
		if (*run2->end == UNNAMED)
		  { run2=run2->next;
		    if (*run2->end == UNNAMED)
		      { edgenet[fixvertex]=buffer;
			delete=buffer->start;
			*delete=OCCUPIED /*+fixvertex*/;
		      } else return;
		   } else return;
	         } else return;
	  }
	else 
	  if (poss==4)
	  { if (*run2->end == UNNAMED)
	      { run2=run2->next;
		if (*run2->end == UNNAMED)
		  { run2=run2->next;
		    if (*run2->end == UNNAMED)
		      { run2=run2->next;
			if (*run2->end == UNNAMED)
			  { edgenet[fixvertex]=buffer;
			    delete=buffer->start;
			    *delete=OCCUPIED /*+fixvertex*/;
			} else return;
		    } else return;
		} else return;
	      } else return;
	  }
	else 
	   if (poss==2)
	      { if (*run2->end == UNNAMED)
		  { run2=run2->next;
		    if (*run2->end == UNNAMED)
		      { edgenet[fixvertex]=buffer;
			delete=buffer->start;
			*delete=OCCUPIED /*+fixvertex*/;
		      } 
		    else return;
		  } 
	      else return;
	      }
	  else 
	     if (poss==1)
		{ if (*run2->end == UNNAMED)
		    { edgenet[fixvertex]=buffer;
		      delete=buffer->start;
		      *delete=OCCUPIED /*+fixvertex*/;
		    } 
		else return;
		}
	else if (poss==0) 
	  { edgenet[fixvertex]=buffer;
	    delete=buffer->start;
	    *delete=OCCUPIED /*+fixvertex*/;
	  }

	embed_benzenoid_sym(which+1,starts);
      }
    else if (components[vertex]==2)
      { 
	if ((jumpvertex=embed_from_here[vertex][1])>=0)
	  { for (i=checkedges[vertex], run_net=startedgenn; i != 1; i--, run_net=run_net->next);
	    buffer=run_net->invers;
	    for (j=checkedges[jumpvertex], test=1, run2=buffer->nextnext; j ;
		 run2=run2->next, j--) if (*run2->end != UNNAMED) test=0;
	    if (test)
	      { edgenet[jumpvertex]=buffer;
		delete=buffer->start;
		*delete=OCCUPIED /*+jumpvertex*/;
	      }
	    else return;
	  }

	fixvertex=embed_from_here[vertex][0];
	/* This one is ALWAYS embedded after vertex */
	buffer=startedgenn->invers;
	for (j=checkedges[fixvertex], test=1, run2=buffer->nextnext; j ;
	     run2=run2->next, j--) if (*run2->end != UNNAMED) test=0;
	if (test)
	  {
	    edgenet[fixvertex]=buffer;
	    charp=buffer->start;
	    *charp=OCCUPIED /*+fixvertex*/;
	    embed_benzenoid_sym(which+1,starts);
	    *charp=UNNAMED;
	  }
      }
    else /* that is: (components[vertex]==3) -- the other vertices MUST be embedded later 
	    and we know that 3 places must be checked */
      { 
	buffer=startedgenn->invers;
	fixvertex=embed_from_here[vertex][0];
	run2=buffer->nextnext;
	if (*run2->end == UNNAMED)
	  { run2=run2->next;
	    if (*run2->end == UNNAMED)
	      { run2=run2->next;
		if (*run2->end == UNNAMED)
		  {
		    delete=buffer->start;
		    edgenet[fixvertex]=buffer;
		    *delete=OCCUPIED /*+fixvertex*/;
		} else return;
	    } else return;
	} else return;

	fixvertex=embed_from_here[vertex][1];
	buffer=startedgenn->nextnext->invers;

	run2=buffer->nextnext;
	if (*run2->end == UNNAMED)
	  { run2=run2->next;
	    if (*run2->end == UNNAMED)
	      { run2=run2->next;
		if (*run2->end == UNNAMED)
		  {
		    edgenet[fixvertex]=buffer;
		    charp=buffer->start;
		    *charp=OCCUPIED /*+fixvertex*/;
		    embed_benzenoid_sym(which+1,starts);
		    *charp=UNNAMED;
		} 
	    } 
	} 
      } /* end 3 components */
  } /* end 1 possibility */

else
  { /* number of components is always 2 */
    poss=number_of_possibilities[vertex];
    if (poss==3)
      {
	fixvertex=embed_from_here[vertex][0];
	run_net=startedgenn; 
	if (*run_net->invers->nextnext->nextnext->end != UNNAMED)
	  /* At a position neighbouring both the faces where the hexagon is placed for 
	     label 1 and 2 a forbidden neighbour was detected, so only 3 is possible */
	  { run_net=run_net->nextnext;
	    buffer=run_net->invers;
	    run2=buffer->nextnext; 
	    if (*run2->end == UNNAMED)
	      { run2=run2->next;
		if (*run2->end == UNNAMED)
		  { run2=run2->next;
		    if (*run2->end == UNNAMED)
		    { label[vertex]=3;
		      edgenet[fixvertex]=buffer;
		      charp=buffer->start;
		      *charp=OCCUPIED /*+fixvertex*/;
		      embed_benzenoid_sym(which+1,starts);
		      *charp=UNNAMED;
		    }
		  }
	      }
	  }
	else /* that is: this face is OK -- and need not be checked again */
	  { /* OK first try label 1: */
	    buffer=run_net->invers;
	    run2=buffer->nextnext; 
	    if (*run2->end == UNNAMED)
	      { 
		run2=run2->next; 
		if (*run2->end == UNNAMED)
		  { label[vertex]=1;
		    edgenet[fixvertex]=buffer;
		    charp=buffer->start;
		    *charp=OCCUPIED /*+fixvertex*/;
		    embed_benzenoid_sym(which+1,starts);
		    *charp=UNNAMED;
		  }
	      }
	    run_net=run_net->next;
	    buffer=run_net->invers;
	    if (*buffer->nextnext->nextnext->end == UNNAMED)
	      /* otherwise this time the face between position 2 and 3 made a problem */
	      {
		buffer=run_net->invers;
		run2=buffer->nextnext->next; 
		if (*run2->end == UNNAMED)
		  { label[vertex]=2;
		    edgenet[fixvertex]=buffer;
		    charp=buffer->start;
		    *charp=OCCUPIED /*+fixvertex*/;
		    embed_benzenoid_sym(which+1,starts);
		    *charp=UNNAMED;
		  }
		run_net=run_net->next;
		buffer=run_net->invers;
		run2=buffer->nextnext->next; 
		if (*run2->end == UNNAMED)
		  { 
		    run2=run2->next; 
		    if (*run2->end == UNNAMED)
		      { label[vertex]=3;
			edgenet[fixvertex]=buffer;
			charp=buffer->start;
			*charp=OCCUPIED /*+fixvertex*/;
			embed_benzenoid_sym(which+1,starts);
			*charp=UNNAMED;
		      }
		  }
	      }
	    /* else do nothing else */
	  }
      }


    else /* that is: labels 1 and 2 possible */
      {
	if ((jumpvertex=embed_from_here[vertex][1])>=0)
	  {
	    /* First fix the edgenet entry that will stay constant */
	    for (i=checkedges[vertex], run_net=startedgenn; i != 1; i--, run_net=run_net->next);
	    buffer=run_net->invers;
	    for (j=checkedges[jumpvertex], test=1, run2=buffer->nextnext; j ;
		 run2=run2->next, j--) if (*run2->end != UNNAMED) test=0;
	    if (test)
	      { edgenet[jumpvertex]=buffer;
		delete=buffer->start;
		*delete=OCCUPIED /*+jumpvertex*/;
	      }
	    else return;
	  }
	
	/* Now the other one: */
	fixvertex=embed_from_here[vertex][0];
	/* fixvertex must ALWAYS be embedded after vertex */
	
	run_net=startedgenn;  /* One extra ->next for the boundary edge
				 (It must AT LEAST be one) */
	/* Now run_net is correct for the MINIMAL possible label (that is 1) */
	buffer=run_net->invers;
	run_net=run_net->next;
	for (j=checkedges[fixvertex], test=1, run2=buffer->nextnext; j ;
	     run2=run2->next, j--) if (*run2->end != UNNAMED) test=0;
	if (test)
	  
	  { label[vertex]=1;
	    edgenet[fixvertex]=buffer;
	    charp=buffer->start;
	    *charp=OCCUPIED /*+fixvertex*/;
	    embed_benzenoid_sym(which+1,starts);
	    *charp=UNNAMED;
	  }
	buffer=run_net->invers;
	for (j=checkedges[fixvertex], test=1, run2=buffer->nextnext; j ;
	     run2=run2->next, j--) if (*run2->end != UNNAMED) test=0;
	if (test)
	  
	  { label[vertex]=2;
	    edgenet[fixvertex]=buffer;
	    charp=buffer->start;
	    *charp=OCCUPIED /*+fixvertex*/;
	    embed_benzenoid_sym(which+1,starts);
	    *charp=UNNAMED;
	  }
      }
  }



if (delete) *delete=UNNAMED;

}



/******************************CONSTRUCT_LABELS_BENZENOID***************************/

void construct_labels_benzenoid(EDGE *starts[])


/* This function is the beginning of the construction of the labels and 
   simultaneously embedds the structures into the hexagonal net (well --
   implemented as its trigonal dual... 

   It works as follows: 

   Only those hexagons are embedded that have at least one edge in the boundary. The 
   aim is to embed them in a way that no two hexagons are embedd at the same place
   and that on the outer side of edges that are supposed to be in the boundary,
   there is no hexagon.

   Starting at boundary_edge and running around the boundary in clockwise direction, 
   the vertices are numbered according to the time they first occur on the boundary. 
   Every vertex except the first (boundary_edge->start) is embedded "from" another 
   vertex, called its father  -- that is: that vertex that is the first one in 
   counterclockwise direction around the boundary. 
   So e.g. a vertex with valency 1 is embedded from its unique neighbour -- a vertex 
   with only one boundary component is embedded from the unique vertex on the boundary 
   in counterclockwise direction, etc. The other way around: A valency 1 vertex (except
   the first one) embeds no other vertex, while a valency 3 vertex with 3 boundary 
   components embeds all its neighbours except the one that embedded him.

   The vertices that embed others are written to a list (to_place[]) and 
   boundary_edge->start and ->end are embedded. Then a recursive routine is started
   that first embeds the vertices that must be embedded from to_place[0], then those
   that must be embedded from to_place[1]... etc.

   Whenever a vertex v is embedded, it is checked that on the other side of all 
   the outer edges there is no other hexagon. Furthermore it is checked that the 
   positions where a new vertex must be embedded from v and the positions where 
   vertices of another block, that is not yet embedded, but will be embedded from 
   here, will be placed, are still free. Note that in case of various possibilities 
   to place the vertices, there is no ambiguity -- the positions must be either
   outside edges or will lead to the new component -- in any case one must check all
   those positions that do not belong to the component C one comes from -- and possibly
   the last position in C, if it must be embedded from here, but its place
   is fixed by the number of edges going to the


   So if a vertex is connected to its father by a bridge, then it starts a new
   block. From this vertex 1 to 3 edges might lead to the new block, but in all cases
   all the 5 positions different from the bridge to the father are checked to be free.

   ---------------------------------------------------------------------------------
   All the positions checked by vertex F at the time it was embedded are still free 
   when F starts to embed its sons.
   ---------------------------------------------------------------------------------
   This means especially that the first face in clockwise direction coming from the 
   father need not be checked by the son, since it was OK when embedding the father 
   -- and analogously for the last position in case the son is connected by a bridge.
   ---------------------------------------------------------------------------------
   Furthermore it means that the sons can be embedded without checking whether 
   their positions have been occupied in the meantime.
   ---------------------------------------------------------------------------------
   
   This is obvious, if the child S is embedded immediately after the father F.
   If not, after the father was embedded, another face was embedded. 
   This can be no problem, if just one face is embedded from the fathers father
   (let's call it FF) (easy to be checked, since there are only very few possible
   positions).

   Note that otherwise it IS POSSIBLE that at the positions checked before F is added, 
   vertices are placed. We just claim that in this case, the face has already been 
   removed in the backtracking procedure when we return to embed F's sons.

   In case after F is embedded other vertices are embedded, they belong to (one or more)
   different blocks than F that are separated by the cutvertex FF. Since we run around 
   the boundary, the whole block (and all that are adjacent to it) is embedded, before
   returning to embed F's sons. The boundary curve of these blocks together with the
   edge(s) along which it sticks to FF is a simple closed curve J in the plane -- that is:
   A Jordancurve. Since F sticks to FF at another edge, local arguments at FF show
   that F must be outside the bounded region of J.

   Now assume that one of these vertices (hexagons) E was embedded neighbouring to F. Since
   it would not have been embedded there if F would be at one of those positions that are
   checked to be free when embedding E, it must be neighbouring E at an edge that is not
   checked -- that is an edge leading to an interior hexagon. So F is an interior hexagon
   of J (a contradiction, so the places must still be free).

   From this it can be deduced that when the structure is embedded, no two vertices are 
   placed at the same position and that on the outer side of every boundary edge there
   is a free position -- that is: We have a benzenoid.


*/

{
int i,j,vertex;
EDGE *run,*run2;
int first[MAXNV];
int number_on_boundary[MAXNV+1]; /* The inverse of to_place */
int to_place_counter=0;
static int to_place[MAXNV+1]; /* This gives the order in which the vertices are to be placed */

/* First the order in that the vertices have to be placed is determined. They are 
   just taken by running in clockwise direction around the inner dual: */


vertex=boundary_edge->start;


for (i=0; i<maxnv; i++) { first[i]=1;
			  run=map[i];
			  do { run->dummy1=0; run=run->next; } while (run != map[i]);
			}
number_on_boundary[vertex]=0;
number_on_boundary[boundary_edge->end]=1;
first[vertex]=first[boundary_edge->end]=0;

/* First mark the edges with the outer face on the left and determine when a vertex
occurs first on the boundary: */
boundary_edge->dummy1=1;
for (run=boundary_edge->invers->next, j=2; run != boundary_edge; run=run->invers->next)
  { run->dummy1=1; /* This tells that the outer face is on the left. */
    if (first[run->end]) { number_on_boundary[run->end]=j;
			   first[run->end]=0; j++; }
  }



first[vertex]=1;

/* the number of boundary components of vertex must be 1, since boundary_edge starts at the 
   last vertex, so the inner dual can not be disconnected, when it is removed */

embed_from_here[vertex][0]=boundary_edge->end;


for (run=boundary_edge->invers->next; run != boundary_edge; run=run->invers->next)
  { j=run->start; 
    if (first[j]==0) /* This means that it wasn't visited in this
			second run before */
      /* A vertex must be added to the to_place list only if some vertices
	 have to be embedded from there */
      { first[j]=1;
	if (components[j]==2) /* This means that it wasn't visited in this
				 second run before */
	  { to_place[to_place_counter]=j; to_place_counter++;
	    embed_from_here[j][0]=run->end;
	    for (run2=run->next, i=0; run2->dummy1 != 1; run2=run2->next, i++);
	    jump[j]=i;
	    if (degree[j]-jump[j]==2) checkedges[j]=3; /* comes from a bridge */
	    else checkedges[j]=6-degree[j]+jump[j];
	    if (number_on_boundary[run2->end] > number_on_boundary[j])
	      { embed_from_here[j][1]=run2->end; 
		checkedges[j]++; /* The place where the second vertex must be placed, must 
				    /may also be checked */
	      }
	    else embed_from_here[j][1]= -1;
	  }
	else 
	  if (components[j]==3) { checkedges[j]=3; 
				  embed_from_here[j][0]=run->end;
				  embed_from_here[j][1]=run->next->end;
				  to_place[to_place_counter]=j; to_place_counter++;
				}
	  else /* 1 component */
	    { if (degree[j]==1) checkedges[j]=3;
	      else { checkedges[j]=5-degree[j];
		     if ( number_on_boundary[run->end] > number_on_boundary[j])
		       { embed_from_here[j][0]=run->end;
			 checkedges[j]++;
			 to_place[to_place_counter]=j; to_place_counter++;
		       }
		   }
	    }
      }
  }

to_place[to_place_counter]=EMPTY; /* as a mark that there isn't more to embed */

/* Now really embed the first vertex: */


edgenet[boundary_edge->end]=startnet->invers;
edgenet[vertex]=NULL;
*(startnet->start)=OCCUPIED /*+vertex*/;
*(startnet->end)=OCCUPIED /*+boundary_edge->end*/;

if (starts) embed_benzenoid_sym(to_place,starts);
else embed_benzenoid(to_place);

*startnet->start=*(startnet->end)=UNNAMED;

}


/*******************************NEXT_STEP_BENZENOIDS*********************************/

void next_step_benzenoids()
{

static int i,dummy;
static LONGTYPE num_labels;
static EDGE *starts[MAXAUTS];

if (number_can_numberings==1) triv_skeletons++;
number_of_skeletons++;

if (just_skeletons) return;

if (detailed) 
  {
    catacondensed= (ne==catanumber);
    C=Cconstant-(ne>>1);
    H=C-CHdifference;;
  }

global_init=1; /* next time the canon_label routine is called, it must
		  be initialised for a new skeleton */

/* compute the positions where various labels are possible */
num_labels=1;
for (i=0; i<nv ; i++)
  { dummy=possible_labels[degree[i]][components[i]];
    number_of_possibilities[i]=dummy;
    if (dummy==1) label[i]=0;  else num_labels *= dummy;
  }

number_of_labellings += num_labels;

if ((number_can_numberings==1) || (num_labels==1))
  { construct_labels_benzenoid(NULL); }
else { /* Look for the orbit of "boundary_edge": */
  for (i=0; can_numberings[0][i] != boundary_edge; i++);
  /* write them all to starts: */
  for (dummy=0; dummy<number_can_numberings; dummy++) 
    starts[dummy]=can_numberings[dummy][i];
  construct_labels_benzenoid(starts);
     }

}

/********************************INIT**************************************/

void init()

/* initialises the new_vertex[][] field, planmap, n */

{

int i,j,k;
EDGE *list, *invers;

/* first the new_vertex field: */

for (i=2; i<MAXNV; i++) /* vertices 0 and 1 are never replaced */
  { for (j=1; j<=5; j++) 
      { list=new_vertex[i][j]=(EDGE *)malloc(j*sizeof(EDGE));
	invers=(EDGE *)malloc(j*sizeof(EDGE));
	if ((list==NULL)||(invers==NULL))
	  { fprintf(stderr,"Can not get more memory in init()  (1).\n");
	    exit(1); }
	for (k=0; k<j; k++)
	  { list[k].start=invers[k].end=i;
	    list[k].invers=invers+k;
	    invers[k].invers=list+k;
	    if (k==0) list[k].prev=list+j-1; else list[k].prev=list+k-1;
	    if (k==(j-1)) list[k].next=list; else list[k].next=list+k+1;
	  }
      }
  }

nv=ne=2;

map[0]=(EDGE *)malloc(sizeof(EDGE));
map[1]=(EDGE *)malloc(sizeof(EDGE));
	
if ((map[0]==NULL)||(map[1]==NULL))
  { fprintf(stderr,"Can not get more memory in init()   (2).\n");
    exit(1); }

map[0]->start=0; map[0]->end=1;
map[0]->prev=map[0]->next=map[0];
map[0]->invers=map[1];

map[1]->start=1; map[1]->end=0;
map[1]->prev=map[1]->next=map[1];
map[1]->invers=map[0];

degree[0]=degree[1]=1;
for (i=2; i<MAXNV; i++) degree[i]=0;

components[0]=components[1]=1;

boundary_edge=boundary_edges[1]=map[1];

canon(degree,can_numberings,&number_can_numberings,&number_can_numb_or_pres);

/* Now the possible_labels[][][] field will be initialized */

possible_labels[1][1]=1; /* just one possibility */
possible_labels[2][1]=1; 
possible_labels[2][2]=3;
possible_labels[3][1]=1; 
possible_labels[3][2]=2;
possible_labels[3][3]=1;
possible_labels[4][1]=1;
possible_labels[4][2]=1;
possible_labels[5][1]=1;
possible_labels[6][0]=1;
}




/******************************ADD_VERTEX*********************************/

void add_vertex(EDGE *start, int valency)

/* adds a new vertex starting at start->start and in prev direction from
   start and connected also to the next valency-1 vertices in clockwise orientation
   along the boundary. */

{

EDGE *new;
int vertex;

new=map[nv]=new_vertex[nv][valency];
boundary_edges[nv]=boundary_edge=new->next;
new=new->invers;

 if (must_be_catacondensed && (valency!=1)) 
   { fprintf(stderr,"Trying to add vertex of valency %d with option catacondensed -- ERROR !\n", valency);
     exit(1); }

switch (valency)
  { 
  case 1: { vertex=start->start;
            new->start=new->invers->end=vertex;
	    new->prev=start->prev; new->next=start;
	    new->prev->next=start->prev=new;
	    degree[vertex]++;
	    components[vertex]++;
	    break;
	  } 
  case 2: { vertex=start->start;
	    new->start=new->invers->end=vertex;
	    new->prev=start->prev; new->next=start;
	    new->prev->next=start->prev=new;
	    degree[vertex]++; /* the number of components does not change */
	    /* Next vertex: */
	    start=start->invers->next;
	    new=new->invers->prev->invers;
	    vertex=start->start;
	    new->start=new->invers->end=vertex;
	    new->prev=start->prev; new->next=start;
	    new->prev->next=start->prev=new;
	    degree[vertex]++; /* the number of components does not change */
	    break;
	  }
  case 3: { vertex=start->start;
	    new->start=new->invers->end=vertex;
	    new->prev=start->prev; new->next=start;
	    new->prev->next=start->prev=new;
	    degree[vertex]++; /* the number of components does not change */
	    /* Next vertex: */
	    start=start->invers->next;
	    new=new->invers->prev->invers;
	    vertex=start->start;
	    new->start=new->invers->end=vertex;
	    new->prev=start->prev; new->next=start;
	    new->prev->next=start->prev=new;
	    degree[vertex]++; 
	    components[vertex]--; /* the number of components of the middle vertex
					   is decreased by one */
	    /* Next vertex: */
	    start=start->invers->next;
	    new=new->invers->prev->invers;
	    vertex=start->start;
	    new->start=new->invers->end=vertex;
	    new->prev=start->prev; new->next=start;
	    new->prev->next=start->prev=new;
	    degree[vertex]++; /* the number of components does not change */
	    break;
	  }
  default: { fprintf(stderr,"Error in add_vertex -- Trying to add a vertex of valency %d\n",
		     valency);
	     exit(1); }
  }

degree[nv]=valency;
components[nv]=1;
nv++;
ne += (2*valency);
}

/******************************DELETE_VERTEX*********************************/

void delete_vertex()

/* deletes the last vertex (number nv-1) from map and repaires the relevant
   variables */

{

EDGE *delete;

nv--;
ne -= (2*degree[nv]);
delete=boundary_edge->invers;
boundary_edge=boundary_edges[nv-1];

switch (degree[nv])
  { 
  case 1: { delete->prev->next=delete->next;
	    delete->next->prev=delete->prev;
	    degree[delete->start]--;
	    components[delete->start]--;
	    break;
	  }
  case 2: { delete->prev->next=delete->next;
	    delete->next->prev=delete->prev;
	    degree[delete->start]--; /* number of components doesn't change */
	    delete=delete->invers->next->invers;
	    delete->prev->next=delete->next;
	    delete->next->prev=delete->prev;
	    degree[delete->start]--; /* number of components doesn't change */
	    break;
	  }
  case 3: { delete->prev->next=delete->next;
	    delete->next->prev=delete->prev;
	    degree[delete->start]--; /* number of components doesn't change */
	    delete=delete->invers->next->invers;
	    delete->prev->next=delete->next;
	    delete->next->prev=delete->prev;
	    degree[delete->start]--; 
	    components[delete->start]++;/* number of components increases by one */
	    delete=delete->invers->next->invers;
	    delete->prev->next=delete->next;
	    delete->next->prev=delete->prev;
	    degree[delete->start]--; /* number of components doesn't change */
	    break;
	  }
  default: { fprintf(stderr,"Error in delete_vertex -- Trying to delete a vertex of valency %d\n",
		     degree[nv]);
	     exit(1); }
  }

}



/**************************************************************************/

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
    EDGE *startedge[MAXNV]; /* startedge[i] is the starting edge for 
                        exploring the vertex with the number i+1 */
    int number[MAXNV], i;   /* The new numbers of the vertices, starting 
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
          { 
            vertex = number[run->end];
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
    EDGE *startedge[MAXNV];
    int number[MAXNV], i; 
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
            if (vertex > (*representation)) return(0);
            if (vertex < (*representation)) return(2);
            representation++; 
          }
        if ((*representation) != 0) return(2); 
        representation++;
        temp = startedge[actual_number];  actual_number++; 
    }


    while (actual_number <= nv) 
    {  
        for (run = temp->prev; run != temp; run = run->prev)
          { 
            vertex = number[run->end];
            if (vertex > (*representation)) return(0);
            if (vertex < (*representation)) return(2);
            representation++;
          }
        if ((*representation) != 0) return(2); 
        representation++;
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
    register int vertex;
    EDGE *temp;  
    EDGE *startedge[MAXNV]; 
    int number[MAXNV], i; 
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
testcanon_first_init_mirror(EDGE *givenedge, int representation[], int colour[])
 
{
    register EDGE *run;
    register int vertex;
    EDGE *temp;  
    EDGE *startedge[MAXNV]; 
    int number[MAXNV], i; 
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

static int 
testcanon_init(EDGE *givenedge, int representation[], int colour[])

/* Tests whether starting from a given edge and constructing the code in
   "->next" direction, an automorphism or even a better representation can 
   be found. A better representation will be completely constructed and 
   returned in "representation".  It works pretty similar to testcanon except 
   for obviously necessary changes, so for extensive comments see testcanon */
{
    register EDGE *run;
    register int vertex;
    EDGE *temp;  
    EDGE *startedge[MAXNV]; 
    int number[MAXNV], i; 
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
             else { if (vertex > (*representation)) return(0);
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
                if (vertex > (*representation)) return(0);
                if (vertex < (*representation))
                  { better = 1; *representation = vertex; }
              }
            representation++;
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
    EDGE *startedge[MAXNV]; 
    int number[MAXNV], i; 
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
            else { if (vertex > (*representation)) return(0);
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
                if (vertex > (*representation)) return(0);
                if (vertex < (*representation))
                  { better = 1; *representation = vertex; }
              }
            representation++;
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
    EDGE *startedge[MAXNV]; 
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
    EDGE *startedge[MAXNV];
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




/******************************CANON*******************************************/

static int 
canon(int colour_prev[], EDGE *can_numberings[][MAXE], 
      int *num_can_numberings, int *num_can_numberings_or_pres)

/* Checks whether the last vertex (number: nv-1) is canonical or not. 
   Returns 1 if yes, 0 if not. One of the criterions a canonical vertex 
   must fulfill, is that its colour is minimal. 
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
   each vertex) doesn't change anything. For this case a special routine can
   be called that is essentially just canon() with the mirror images deleted. 

   For the helicenes program, we NEED that in this case the automorphisms are 
   listed as orientation preserving AND orientation reversing, so this routine
   is not called.

   For the helicenes program there is exactly one face that is not a triangle (in case
   nv>3 -- otherwise the vertex with minimum colour -- which is minimum degree in this
   case -- can always be deleted). This face is chosen as the outer face. A vertex
   lying in the outer face can not be deleted in case the graph breaks in two (or more)
   parts if this is done. This is exactly the case if it occurs in the boundary more than
   once. The last vertex (nv-1) can always be deleted by construction -- it was just added !
   Vertices with valency one can also always be deleted, so in case the last vertex has
   valency one (and therefore all possibly canonical ones) nothing has to be checked.

   Since interior vertices always have valency 6, there are always some with smaller valency,
   so we do not have to care for only boundary vertices being regarded canonical.
   Nevertheless by forbidding vertices that do not occur in the boundary, nothing is lost.

*/
{
	int i, j, last_vertex, test;
	int minstart, maxend; /* (minstart,maxend) will be the chosen colour 
				pair of an edge */
	EDGE *startlist_last[5], *startlist[5*MAXNV], *run, *end;
	int list_length_last, list_length;
	int representation[2*MAXE+MAXNV];
	EDGE *numblist[MAXE], *numblist_mirror[MAXE]; /* lists of edges where 
				starting gives a canonical representation */
	int numbs = 1, numbs_mirror = 0;
	int colour[MAXNV];

        /* commented out for helicenes:
	for (i=0; (degree[i]<3) && (i<nv); i++);
	if (i==nv)  did not find a vertex with degree >=3 
            return(special(colour, can_numberings,num_can_numberings,num_can_numberings_or_pres));
         */

	for (i=0; i<nv; i++) colour[i]=colour_prev[i]+MAXNV;

	last_vertex = nv-1;
	minstart = colour[last_vertex];
       

/* determine the smallest possible end for the vertex "last_vertex" */

	list_length_last = 1; startlist_last[0] = end = map[last_vertex];
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
	  if (components[i]==1) /* Just for helicenes: must occur only once
				   in the boundary */
  	  { if (colour[i] < minstart) return(0);
    	    if (colour[i] == minstart)
      	    { run = end = map[i];
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
    	  for (i = 0, j=numbs; i < numbs_mirror; i++, j++) 
	      construct_numb_mirror(numblist_mirror[i], 
				     can_numberings[j]);
  	}
	else 
	{ if (numbs) can_numberings[0][0] = numblist[0];
	  else can_numberings[0][0] = numblist_mirror[0]; }

	/* in case of maxdegree<3 numbs and numbs_mirror are not correct. In case of a path the group
           must be recomputed depending on the labels anyway, so only the case of a cycle -- to be exact:
           of a 3-cycle must be detected: */
	if (detailed && (ne==6) && (nv==3)) numbs=numbs_mirror=3;

	if (numbs<numbs_mirror) { i=numbs; numbs=numbs_mirror; numbs_mirror=i; }
	if ((numbs==1) && (numbs_mirror==0)) group=Cs;
	else if ((numbs==1) && (numbs_mirror==1)) group=C2v;
	else if ((numbs==2) && (numbs_mirror==0)) group=C2h;
	else if ((numbs==2) && (numbs_mirror==2)) group=D2h;
	else if ((numbs==3) && (numbs_mirror==0)) group=C3h;
	else if ((numbs==3) && (numbs_mirror==3)) group=D3h;
	else if ((numbs==6) && (numbs_mirror==0)) group=C6h;
	else if ((numbs==6) && (numbs_mirror==6)) group=D6h;
	else { fprintf(stderr,"Problem -- didn't recognize group\n"); exit(1); }

	return(1);
}




/****************************COMPUTE_ORBITS******************************/

void compute_orbits(EDGE *starts[], int howmuch[][6])

/* deletes isomorphic positions from the list by using the entries of
   can_numberings[][], number_can_numberings and number_can_numb_or_pres.
   The function is only called in case of more than one automorphism. */


{ int i,j,k,l, puffer, puffer2, pufferpos, limit;
  EDGE *run, *run2, *remember;
  EDGE *newstarts[3*MAXNV]; /* The new starts */
  int new_hm[3*MAXNV][6]; /* the new numbers how far one must go */
  int number_starts=0; /* number of (new) starts */

  for (i=0;i<nv;i++)
    { run=map[i];
      for (j=degree[i]; j>0; j--) { run->dummy1= -1; run=run->next; }
    }


  for (i=0; starts[i] != NULL ; i++)
     starts[i]->dummy1=i;
  /* In dummy1 the position in the starts-list is noted */

/* for every 0<=j<ne, the set of edges 
   { can_numberings[i][j] | 0<=i<number_can_numberings }
   is a set of equivalent edges in which every edge in the class of edges equivalent 
   occurs EXACTLY once. In case there are orientation preserving numberings, the
   first number_can_numb_or_pres ones are equivalent to the first one by an orientation 
   preserving automorphism, the rest by an orientation reversing one. In case of no
   orientation preserving numberings, all automorphisms are orientation preserving.

   In { can_numberings[0][i] | 0<=i<ne } every edge occurs once.

   So what we do is the following: We run through this list and take the
   first edge that can serve as a start (dummy1>=0). All other starting edges are
   deleted from the list. Furthermore dummy1 is set to -2 to note that this set
   has been checked and may not be checked when it occurs a second time.

   The dummy2 entry of the edges is initialised to the position in the new
   list in case the edge occurs there as a starting position and to -1 else.
 */


if (number_can_numb_or_pres) limit=number_can_numb_or_pres; 
  else limit=number_can_numberings;
/* In case of no orientation preserving canonical numberings, all automorphisms
   mapping the first numbering onto the others are orienatation preserving --
   otherwise only the first number_can_numb_or_pres ones */


for (i=0; i<ne; i++)
  if (can_numberings[0][i]->dummy1 >= 0) /* new set */
    { run=can_numberings[0][i];
      newstarts[number_starts]=run;
      puffer=run->dummy1;
      for (k=0; howmuch[puffer][k] !=0; k++)
	new_hm[number_starts][k]=howmuch[puffer][k];
      new_hm[number_starts][k]=0;
      run->dummy2=number_starts; /* position in new list */
      number_starts++;
      for (j=1; j<limit; j++)
	{ run=can_numberings[j][i];
	  run->dummy1=run->dummy2= -2; }
    }


/* Now we have to deal with those automorphisms that reverse the orientation.
   If edge1 is mapped onto edge2 by such an automorphism, then the position
   (edge1, x) with x the valency of the vertex to be added makes the following
   position superfluous: Go from edge2 x-1 entries in counterclockwise direction
   around the boundary and take the inverse edge e2'. The position (e2',x) is
   equivalent to (edge1, x) and will be deleted (if it hasn't been deleted before).
   The first time a position occurs all equivalent ones are deleted. Since always
   one of the set of equivalent positions is kept (being careful with positions 
   that are stabilized under the automorphism), we do not delete too much and 
   since for every position whose mirror image is equivalent to some other (not
   reflected) position this mirror edge occors in the list of equivalent edges 
   of the relevant starting edge, we delete every duplicate. */



if (number_can_numb_or_pres && (number_can_numberings > number_can_numb_or_pres))
  { for (i=0; i<ne; i++)
      if (can_numberings[0][i]->dummy1 >= 0) /* The set contains a starting edge */
	{ run2=can_numberings[0][i];
	  puffer=run2->dummy2;
	  for (j=number_can_numb_or_pres ; j<number_can_numberings ; j++)
	    { remember=can_numberings[j][i]->next;
	      /* This is the edge where adding one vertex would give the 
		 same result */
	      for (k=0; (pufferpos=new_hm[puffer][k]) != 0 ; k++)
		{ run=remember;
		  for (l=1; l<pufferpos; l++) run=run->prev->invers;
		  if ((run->dummy2 >= 0) && (run != run2))
		    /* is it in the new list and not an invariant position ? */
		    { puffer2=run->dummy2;
		      for (l=0; (new_hm[puffer2][l] != pufferpos) 
			                            && (new_hm[puffer2][l] != 0); l++);
		      /* Now we found the entry that must be deleted or found it isn't 
			 there any more */
			for (; new_hm[puffer2][l] != 0 ; l++) 
			  new_hm[puffer2][l]=new_hm[puffer2][l+1];
		      /* It is deleted so that no gaps occur and still a 0 ends the lists */
		    }
		}
	    }
	}
  }


/* Now  replace the starts and how_much fields */

for (i=j=0; i< number_starts; i++)
  if (new_hm[i][0] != 0) /* not all positions were deleted */
    { starts[j]=newstarts[i];
      for (k=0; new_hm[i][k] != 0; k++) howmuch[j][k]=new_hm[i][k];
      howmuch[j][k]=0;
      j++;
    }
starts[j]=NULL;

}


/***************************WRITE_COMBINATRICA******************************************************/ 
void write_combinatrica()
{
  FILE *out;
  EDGE *ed;
  int i,j;

  out = fopen("graph.cmb","w");

  for(i = 0;i<nv;i++) {
    j = 1;
    fprintf(out,"%d 0.5 0.5 ",i+1);
    for(ed = map[i];j<=degree[i];j++) {
      fprintf(out,"%d ",ed->end + 1);
      ed = ed->next;
    }
    fprintf(out,"\n");
  }
  fclose(out);

}


/**************************COMPUTE_POSITIONS****************************/

void compute_positions(EDGE *starts[],int how_much[][6])

{

int midpoint[MAXNV], endpoint[MAXNV], one_add[MAXNV]; /* is it a possible midpoint,
							 start- and endpoint or is it 
							 even possible to add a valency 
							 one vertex */
int vertex;
EDGE *run, *next, *overnext;
int numberstarts, diffpos, sum, val, comp;
int ones=0, twos=0; /* the number of vertices with valency one/two vertices 
		       that can be deleted  */


 if (must_be_catacondensed)
   {
     for (vertex=0; vertex<nv; vertex++)
       { if (degree[vertex]+components[vertex]<5) endpoint[vertex]=one_add[vertex]=1;
	 else endpoint[vertex]=one_add[vertex]=0; 
       }
   }
 else 
   {
     for (vertex=0; vertex<nv; vertex++)
       { val=degree[vertex]; comp=components[vertex];
       if (val<6)
	 { sum=val+comp;
	 if (sum<5) { endpoint[vertex]=one_add[vertex]=1; }
	 else { one_add[vertex]=0;
	        if (sum<6) endpoint[vertex]=1;
		else endpoint[vertex]=0;
	 }
	 if ((comp>=2) || (val==5)) midpoint[vertex]=1;
	 else midpoint[vertex]=0;
	 if (val==1) ones++;
	 else if ((val==2) && (comp==1)) twos++;
	 }
       }
   }

run=boundary_edge;
numberstarts=0;


if (must_be_catacondensed || ((ones>=2) && (nv>2))) /* definitely only ones can be added */
  { do
      { vertex=run->start;
      //if (endpoint[vertex]) /* start possible */
	  { if (one_add[vertex])
	      { starts[numberstarts]=run; how_much[numberstarts][0]=1;
		how_much[numberstarts][1]=0;
		numberstarts++; 
	      }
	  }
	run=run->invers->next;
      }
  while (run != boundary_edge);
  }
else
  { if (ones==1) /* a two is possible in case it touches the one,
		    a three never */
      { next=run->invers->next;
	do
	  { vertex=run->start;
	    diffpos=0;
	    if (endpoint[vertex]) /* start possible */
	      { if (one_add[vertex])
		  { starts[numberstarts]=run; how_much[numberstarts][0]=1;
		    diffpos=1; }
		if (endpoint[next->start] && 
		    ((degree[run->start]==1) || (degree[next->start]==1)))
		  { if (diffpos==0) starts[numberstarts]=run;
		    how_much[numberstarts][diffpos]=2;
		    diffpos++; }
		if (diffpos) {	how_much[numberstarts][diffpos]=0;
				numberstarts++; }
	      }
	    run=next; next=next->invers->next;
	  }
	while (run != boundary_edge);
      }
    else /* That is: either the special case nv=2 which doesn't hurt or there
	    is absolutely no one -- noone there ? :-)*/
      { next=run->invers->next; overnext=next->invers->next;
	do
	  { vertex=run->start; 
	    diffpos=0;
	    if (endpoint[vertex]) /* start possible */
	      { if (one_add[vertex])
		  { starts[numberstarts]=run; how_much[numberstarts][0]=1;
		    diffpos=1; }
		if (endpoint[next->start])
		  { if (diffpos==0) starts[numberstarts]=run;
		    how_much[numberstarts][diffpos]=2;
		    diffpos++; }
		if ((midpoint[next->start]) && (endpoint[overnext->start]))
		  { if ((degree[run->start]==2) && (components[run->start]==1))
		      sum=1; else sum=0;
		    if ((degree[next->start]==2) && (components[next->start]==1))
		      sum++;
		    if ((degree[overnext->start]==2) && (components[overnext->start]==1))
		      sum++;
		    if (sum>=twos)
		      {
			{ if (diffpos==0) starts[numberstarts]=run;
			  how_much[numberstarts][diffpos]=3;
			  diffpos++; }
			      }
		  } /* end midpoint && endpoint */
		if (diffpos) {	how_much[numberstarts][diffpos]=0;
					numberstarts++; }
	      }
	    run=next; next=overnext; overnext=overnext->invers->next;
	  }
	while (run != boundary_edge);
      } /* end remaining case (no one or nv==2) */
  }/* ende else nach ones >=2 */
starts[numberstarts]=NULL;
}


/*****************************CONSTRUCT**********************************/

void construct()
/* The construction routine -- adds a new vertex in every possible way and
   calls the next step (the labelling routine) if n=maxnv or construct 
   if n<maxnv */

{

EDGE *starts[3*MAXNV]; /* Each vertex can occur up to 3 times in the boundary */
int how_much[3*MAXNV][6];
EDGE *run;
int i,j;

if (modulo && (nv==SPLITLEVEL))
  { if (modulocounter==modulo) modulocounter=1; 
    else { modulocounter++; return; }
  }
    


compute_positions(starts, how_much);



/* The automorphisms were computed in the previous step */
if (number_can_numberings>1) compute_orbits(starts, how_much);


for (run=starts[0], i=0; run != NULL; i++, run=starts[i])
  { 
    for (j=0 ; how_much[i][j] != 0; j++)
      { add_vertex(run,how_much[i][j]);
	if (canon(degree, can_numberings, &number_can_numberings,&number_can_numb_or_pres))
	  { if (nv==maxnv) /* labelling the vertices */
	      { if (benzenoids) next_step_benzenoids();
	      else next_step(); }
	  else construct(); /* add another vertex */
	  }
	delete_vertex();
      }
  }

}

/****************************USAGE****************************************/

void usage(char *name)
{
fprintf(stderr,"Usage: %s x [B] [b] [c] [d] [f outfile] [m a b] [p] [s] [C] \n\n",name);
fprintf(stderr,"The integer x is the number of hexagons.\n");
fprintf(stderr,"Option B makes that BECode is written to outfile (default stdout).\n");
fprintf(stderr,"Option b makes that only benzenoids are generated.\n");
fprintf(stderr,"Option c makes that the structures are not all formed in the memory, but just counted.\n");
fprintf(stderr,"Option d makes the program compute and display data about groups and chemical formulas.\n");
fprintf(stderr,"Option f makes that data is written to outfile instead of stdout.\n");
fprintf(stderr,"Option m makes that the generation is splitted into b parts\n");
fprintf(stderr,"         and part number a (0<=a<b) shall be generated.\n");
fprintf(stderr,"Option p makes that planarcode is written to outfile (default stdout).\n");
fprintf(stderr,"Option s makes that just skeletons are generated.\n");
fprintf(stderr,"Option C makes that just catacondensed structures are generated.\n");

fprintf(stderr,"\n For just skeletons no additional detailed information is computed.\n\n ");
exit(1);
}


/*****************************WRITE_RESULTS*******************************/
#ifndef NOTIMES
void write_results(FILE *outfile, Horloge *watch)
#else
void write_results(FILE *outfile)
#endif //NOTIMES
{
  int i,j,k;
  LONGTYPE dummy;

 if (must_be_catacondensed) 
   { if (benzenoids) fprintf(outfile,"Only catacondensed benzenoids generated!\n");
     else fprintf(outfile,"Only catacondensed fusenes generated!\n"); }



#ifdef __alpha
fprintf(outfile,"number of skeletons: %lu\n",number_of_skeletons);
fprintf(outfile,"With trivial group: %lu\nWith nontrivial group: %lu\n\n",triv_skeletons,number_of_skeletons-triv_skeletons);
if (!just_skeletons) { fprintf(outfile,"number of labelled skeletons: %lu\n",number_of_labellings);
		       if (benzenoids) fprintf(outfile,"Accepted %lu benzenoids.\n\n",counter);
		       else fprintf(outfile,"Accepted %lu fusenes.\n\n",counter);
		       if (detailed)
			 {
fprintf(outfile,"%lu catacondensed %lu pericondensed,\n",catas,counter-catas);


		       fprintf(outfile,"Numbers of structures grouped with respect to the chemical formula:\n");
		       for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			 { for (k=Cs, dummy=0; k<=D6h; k++) dummy+=groupformula[k][i][j];
			   if (dummy) fprintf(outfile,"C_%d H_%d: %lu\n",i,j,dummy); }

		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[Cs][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry Cs: %lu\n",dummy);
			   fprintf(outfile,"Grouped with respect to the chemical formula:\n");
			   for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			     { if (groupformula[Cs][i][j]) 
			       fprintf(outfile,"Cs: C_%d H_%d: %lu\n",i,j,groupformula[Cs][i][j]); }
			 }


		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[C2h][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry C2h: %lu\n",dummy);
			   fprintf(outfile,"Grouped with respect to the chemical formula:\n");
			   for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			     { if (groupformula[C2h][i][j]) 
			       fprintf(outfile,"C2h: C_%d H_%d: %lu\n",i,j,groupformula[C2h][i][j]); }
			 }

		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[D2h][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry D2h: %lu\n",dummy);
			   fprintf(outfile,"Grouped with respect to the chemical formula:\n");
			   for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			     { if (groupformula[D2h][i][j]) 
			       fprintf(outfile,"D2h: C_%d H_%d: %lu\n",i,j,groupformula[D2h][i][j]); }

			 }

		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[C2v][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry C2v: %lu\n",dummy);
			   fprintf(outfile,"Grouped with respect to the chemical formula:\n");
		       for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			 { if (groupformula[C2v][i][j]) 
                                fprintf(outfile,"C2v: C_%d H_%d: %lu\n",i,j,groupformula[C2v][i][j]); }
			 }


		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[C3h][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry C3h: %lu\n",dummy);
			   fprintf(outfile,"Grouped with respect to the chemical formula:\n");
			   for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			     { if (groupformula[C3h][i][j]) 
			       fprintf(outfile,"C3h: C_%d H_%d: %lu\n",i,j,groupformula[C3h][i][j]); }
			 }

		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[D3h][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry D3h: %lu\n",dummy);
		       fprintf(outfile,"Grouped with respect to the chemical formula:\n");
		       for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			 { if (groupformula[D3h][i][j]) 
			   fprintf(outfile,"D3h: C_%d H_%d: %lu\n",i,j,groupformula[D3h][i][j]); }
			 }

		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[C6h][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry C6h: %lu\n",dummy);
			   fprintf(outfile,"Grouped with respect to the chemical formula:\n");
			   for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			     { if (groupformula[C6h][i][j]) 
			       fprintf(outfile,"C6h: C_%d H_%d: %lu\n",i,j,groupformula[C6h][i][j]); }
			 }

		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[D6h][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry D6h: %lu\n",dummy);
			   fprintf(outfile,"Grouped with respect to the chemical formula:\n");
			   for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			     { if (groupformula[D6h][i][j]) 
                                fprintf(outfile,"D6h: C_%d H_%d: %lu\n",i,j,groupformula[D6h][i][j]); }
			 }
			 }/* end detailed */
                   }
#else
fprintf(outfile,"number of skeletons: %llu\n",number_of_skeletons);
fprintf(outfile,"With trivial group: %llu\nWith nontrivial group: %llu\n\n",triv_skeletons,number_of_skeletons-triv_skeletons);
if (!just_skeletons) { fprintf(outfile,"number of labelled skeletons: %llu\n",number_of_labellings);
		       if (benzenoids) fprintf(outfile,"Accepted %llu benzenoids.\n\n",counter);
		       else fprintf(outfile,"Accepted %llu fusenes.\n\n",counter);
		     }

		       if (detailed)
			 {
fprintf(outfile,"%llu catacondensed %llu pericondensed,\n",catas,counter-catas);


		       fprintf(outfile,"Numbers of structures grouped with respect to the chemical formula:\n");
		       for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			 { for (k=Cs, dummy=0; k<=D6h; k++) dummy+=groupformula[k][i][j];
			   if (dummy) fprintf(outfile,"C_%d H_%d: %llu\n",i,j,dummy); }

		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[Cs][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry Cs: %llu\n",dummy);
			   fprintf(outfile,"Grouped with respect to the chemical formula:\n");
			   for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			     { if (groupformula[Cs][i][j]) 
			       fprintf(outfile,"Cs: C_%d H_%d: %llu\n",i,j,groupformula[Cs][i][j]); }
			 }


		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[C2h][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry C2h: %llu\n",dummy);
			   fprintf(outfile,"Grouped with respect to the chemical formula:\n");
			   for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			     { if (groupformula[C2h][i][j]) 
			       fprintf(outfile,"C2h: C_%d H_%d: %llu\n",i,j,groupformula[C2h][i][j]); }
			 }

		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[D2h][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry D2h: %llu\n",dummy);
			   fprintf(outfile,"Grouped with respect to the chemical formula:\n");
			   for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			     { if (groupformula[D2h][i][j]) 
			       fprintf(outfile,"D2h: C_%d H_%d: %llu\n",i,j,groupformula[D2h][i][j]); }

			 }

		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[C2v][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry C2v: %llu\n",dummy);
			   fprintf(outfile,"Grouped with respect to the chemical formula:\n");
		       for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			 { if (groupformula[C2v][i][j]) 
                                fprintf(outfile,"C2v: C_%d H_%d: %llu\n",i,j,groupformula[C2v][i][j]); }
			 }


		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[C3h][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry C3h: %llu\n",dummy);
			   fprintf(outfile,"Grouped with respect to the chemical formula:\n");
			   for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			     { if (groupformula[C3h][i][j]) 
			       fprintf(outfile,"C3h: C_%d H_%d: %llu\n",i,j,groupformula[C3h][i][j]); }
			 }

		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[D3h][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry D3h: %llu\n",dummy);
		       fprintf(outfile,"Grouped with respect to the chemical formula:\n");
		       for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			 { if (groupformula[D3h][i][j]) 
			   fprintf(outfile,"D3h: C_%d H_%d: %llu\n",i,j,groupformula[D3h][i][j]); }
			 }

		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[C6h][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry C6h: %llu\n",dummy);
			   fprintf(outfile,"Grouped with respect to the chemical formula:\n");
			   for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			     { if (groupformula[C6h][i][j]) 
			       fprintf(outfile,"C6h: C_%d H_%d: %llu\n",i,j,groupformula[C6h][i][j]); }
			 }

		       for (i=6, dummy=0;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++) dummy+=groupformula[D6h][i][j];
		       if (dummy)
			 {
			   fprintf(outfile,"Numbers of structures with symmetry D6h: %llu\n",dummy);
			   fprintf(outfile,"Grouped with respect to the chemical formula:\n");
			   for (i=6;i<=4*maxnv+2;i++) for (j=6;j<=2*maxnv+5;j++)
			     { if (groupformula[D6h][i][j]) 
                                fprintf(outfile,"D6h: C_%d H_%d: %llu\n",i,j,groupformula[D6h][i][j]); }
			 }
			 }/* end detailed */

#endif
#ifndef NOTIMES
EvalHorloge(watch);
PrintHorloge(outfile,watch);
 if(watch->ucpu > 1e-6)
   {
     fprintf(outfile,"%f skeletons/sec\n",(double)number_of_skeletons / watch->ucpu);
     if (!just_skeletons)
       { if (benzenoids) fprintf(outfile,"%f benzenoids/sec\n",(double)counter / watch->ucpu);
         else fprintf(outfile,"%f fusenes/sec\n",(double)counter / watch->ucpu);}
   }
#endif //NOTIMES 
}


/********************************ADD_NETVERTEX******************************/

void add_netvertex(EDGE2 *start, EDGE2** nextedge, EDGE2 *edgelist,
		   char *starts)

/* adds a new vertex and glues it to the end of edge start. Triangles in both
   directions are formed to obtain a part of the trigonal net. In nextedge
   the next edge where a vertex must be added (they are to be added in a spiral 
   fashion) is returned.
   The edges are the next 6 after edgelist and the start pointer is set to starts */


{
EDGE2 *run, *run2, *ed_prev;
int i;

/* First initialize the new vertex and the edges going out of it: */


  run=edgelist;
  run->next=edgelist+1;  run->invers=NULL; run->start=starts;
  run->end=NULL;  run->nextnext=edgelist+2;
  for (i=1; i<5; i++)
    { run=edgelist+i;
      run->next=edgelist+i+1; run->invers=NULL; run->start=starts;
      run->end=NULL;
      if (i==4) run->nextnext=edgelist; else run->nextnext=edgelist+i+2;
    }
  run=edgelist+5;
  run->next=edgelist; run->invers=NULL; run->start=starts;
  run->end=NULL; run->nextnext=edgelist+1;

edgelist->invers=start; start->invers=edgelist;
edgelist->end=start->start; start->end=starts;

if ((run2=start->next->invers) != NULL) /* there is already a vertex */
  { ed_prev=edgelist->nextnext->nextnext->next; /* former ->prev */
    run2->next->invers=ed_prev;
    run2->next->end=starts;
    ed_prev->invers=run2->next; 
    ed_prev->end=run2->next->start;
    *nextedge=run2->nextnext; }
else *nextedge=start->next;

ed_prev=start->nextnext->nextnext->next;

if ((run2=ed_prev->invers) != NULL) /* there is already a vertex */
  { ed_prev=run2->nextnext->nextnext->next;
    ed_prev->invers=edgelist->next;
    ed_prev->end=starts;
    edgelist->next->invers=ed_prev;
    edgelist->next->end=ed_prev->start; }



}

/********************************INIT_NET********************************/

/* Builds a hexagonal net that is used to embed the structures when testing
   for being benzenoid. The radius is one larger than necessary to fit in, so
   that the neighbouring hexagons are always well defined. */

void init_net(int radius)

{ static EDGE2 *edgelist; /* the edges used in the net */
  static char* starts; /* the starting positions adressed from the edges */
  int hexagonnumber;
  int i;
  EDGE2 *run, *nextedge;

  radius++;

  hexagonnumber=1+(3*(radius*(radius-1)));
  starts=(char*)malloc(hexagonnumber*sizeof(char));
  if (starts==NULL) { fprintf(stderr,"No memory for the starts field for the net.\n");
		      exit(1); }
  for (i=0; i<hexagonnumber; i++) starts[i]=UNNAMED;
  edgelist=(EDGE2*)malloc(6*hexagonnumber*sizeof(EDGE2));
  if (edgelist==NULL) { fprintf(stderr,"No memory for the edgelist field for the net.\n");
			exit(1); }
  
/* First initialize the first vertex and the edges going out of it: */

  run=startnet=nextedge=edgelist;
  run->next=edgelist+1;  run->invers=NULL; run->start=starts;
  run->nextnext=edgelist+2;
  run->end=NULL;
  for (i=1; i<5; i++)
    { run=edgelist+i;
      run->next=edgelist+i+1; run->invers=NULL; run->start=starts;
      run->end=NULL;
      if (i==4) run->nextnext=edgelist; else run->nextnext=edgelist+i+2;
    }
  run=edgelist+5;
  run->next=edgelist;  run->invers=NULL; run->start=starts;
  run->end=NULL; run->nextnext=edgelist+1;

edgelist+=6; starts++;


for (i=1; i<hexagonnumber; i++, edgelist+=6, starts++)
add_netvertex(nextedge, &nextedge, edgelist, starts);

}

/*******************************MAIN**************************************/

int main(int argc, char *argv[])

{
int i;
char dummy[30];
FILE *logfile;

#ifndef NOTIMES
Horloge *watch;  /* watch not clock since this last one may already be used */
watch = AllocHorloge();
InitHorloge(watch);
#endif //NOTIMES

if (argc<2) usage(argv[0]);


outfile=stdout;

for (i=1; i<argc; i++)
  { if (isdigit(argv[i][0])) maxnv=atoi(argv[i]);
    else 
      if (argv[i][0]=='B') bec_out=1;
      else if (argv[i][0]=='b') benzenoids=1;
      else if (argv[i][0]=='c') just_count=1;
      else if (argv[i][0]=='C') must_be_catacondensed=1;
      else if (argv[i][0]=='d') detailed=1;
      else if (argv[i][0]=='f') 
	{ i++;
	  if (i>=argc) { fprintf(stderr,"No file given.\n"); exit(1); }
	  outfile=fopen(argv[i],"w");
	  if (outfile==NULL) { fprintf(stderr,"Can not open file %s for writing.\n",argv[i]);
			       exit(1); }
	}
      else if (argv[i][0]=='m')
	{ i++; part=atoi(argv[i]); i++; modulo=atoi(argv[i]);
	  if ((modulo==0) || (part<0) || (part>=modulo)) usage(argv[0]);
	  modulocounter=modulo-part;
	}
      else if (argv[i][0]=='p') pl_code_out=1;
      else if (argv[i][0]=='s') just_skeletons=1;
      else{ fprintf(stderr,"Unrecognized option: %s\n",argv[i]); 
	    usage(argv[0]); }
  }

 if (just_skeletons) detailed=0;

CHdifference=2*maxnv-2;
Cconstant=5*maxnv+1;

 if (must_be_catacondensed) 
   { if (benzenoids) fprintf(stderr,"Generating only catacondensed benzenoids!\n");
     else fprintf(stderr,"Generating only catacondensed fusenes!\n"); }
if (modulo && (maxnv<=SPLITLEVEL)) 
  { fprintf(stderr,"When using option m there must be at least %d hexagons\n",
	    SPLITLEVEL+1);
    exit(1); }

if (benzenoids && just_skeletons) 
  { fprintf(stderr,"Benzenoids and just skeletons together isn't possible.\n");
    exit(1); }

if (benzenoids && just_count) 
  { fprintf(stderr,"Just counting Benzenoids isn't possible -- setting just_count to 0.\n");
    just_count=0; }


if (benzenoids) init_net(maxnv);

sprintf(logfilename,"%s.%d.",argv[0],maxnv);
if (just_skeletons) strcat(logfilename,"s.");
if (benzenoids) strcat(logfilename,"b.");
if (must_be_catacondensed) strcat(logfilename,"cata.");
if (modulo) { sprintf(dummy,"m_%d_%d.",part,modulo);
	      strcat(logfilename,dummy); }
strcat(logfilename,"log");

logfile=fopen(logfilename,"w");
if (logfile==NULL) { fprintf(stderr,"Cannot open logfile %s\n",logfilename);
		     exit(1); }

for (i=0; i<argc; i++) fprintf(stderr,"%s ",argv[i]); fprintf(stderr,"\n\n");
for (i=0; i<argc; i++) fprintf(logfile,"%s ",argv[i]); fprintf(logfile,"\n\n");

if (sizeof(LONGTYPE) < 8)
  { fprintf(stderr,"Warning: size of LONGTYPE only %d bytes ! \n",
	    (int)sizeof(LONGTYPE));
    fprintf(logfile,"Warning: size of LONGTYPE only %d bytes ! \n",
	    (int)sizeof(LONGTYPE));
  }

if ((maxnv<2) || (maxnv>MAXNV)) 
  { fprintf(stderr,"The number of faces must be in the range 2 to %d.\n",MAXNV);
    fprintf(logfile,"The number of faces must be in the range 2 to %d.\n",MAXNV);
    exit(1); }

fclose(logfile);

init();

if (maxnv==2) { number_of_skeletons=1; number_of_labellings=1; catacondensed=1; group=D2h; C=10; H=8; write_up(); }
else construct();
#ifndef NOTIMES
write_results(stderr, watch);
#else
write_results(stderr);
#endif //NOTIMES

logfile=fopen(logfilename,"a");
if (logfile==NULL) { fprintf(stderr,"Cannot open logfile %s\n",logfilename);
		     exit(1); }
#ifndef NOTIMES
write_results(logfile, watch);
#else
write_results(logfile);
#endif //NOTIMES

if (outfile != stdout) fclose(outfile);
fclose(logfile);
return(0);
}







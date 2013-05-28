#include "fullerene_caps.h"

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

if (first_call) { fprintf(fil,">>planar_code<<"); first_call=0; 
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

if (first_call) { fprintf(fil,">>planar_code<<"); first_call=0; 
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

if (rows== -1) return NULL;

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


/********************************IS_NUMBER*********************************

int isnumber(char string[])
{
while (*string != 0) { if (!isdigit(*string)) return 0; string++; }
return 1;
}

**********************************MAIN*************************************/

void mainb(int argc , char *argv[])

{

EDGE *merke;
int i;

maxlabel=300;
init_marks();

fprintf(stderr,"Command:\n");
for (i=0; i< argc; i++) fprintf(stderr,"%s ",argv[i]); fprintf(stderr,"\n");


if (argc<3) usage(argv[0]);

if (!isdigit(argv[1][0])) usage(argv[0]); l=atoi(argv[1]);
if (!isdigit(argv[2][0])) usage(argv[0]); m=atoi(argv[2]);
if ((l<0) || (m<0)) usage(argv[0]);
if (l<m) { i=l; l=m; m=i; }


init_marks();
init_new_paths(maxlabel);

/* make_n_gon(5,maxlabel);
  writemap();
  merke=make_n_gon(6,maxlabel);
  fprintf(stderr,"Ergebnis: %d\n",compute_sequence(merke, sequence, starts,&length));
  fprintf(stderr,"Anz %d laenge %d: ",sequence[0],length);
  for (j=1; j<=sequence[0]; j++) 
  fprintf(stderr,"%d->%d: %d, ",starts[j]->start,starts[j]->end,sequence[j]); 
  fprintf(stderr,"\n");
  writemap();

  for ( i=1; i<10; i++)
  { fprintf(stderr,"Runde %d:\n",i);
  if (i%4) merke=add_n_gon(6, merke); else merke=add_n_gon(5, merke);
  compute_sequence(merke, sequence, starts,&length);
  fprintf(stderr,"Ergebnis: %d\n",compute_sequence(merke, sequence, starts,&length));
   fprintf(stderr,"Anz %d laenge %d: ",sequence[0],length);
   for (j=1; j<=sequence[0]; j++) 
   fprintf(stderr,"%d->%d: %d, ",starts[j]->start,starts[j]->end,sequence[j]); 
   fprintf(stderr,"\n");
   writemap();
   }

   i-=2;

   for ( ; i>=0; i--)
   { fprintf(stderr,"Runde %d:\n",i);
   merke=delete_n_gon(merke);
   writemap();
   }
*/



merke=make_tube(20,3);
/* write_tube(merke-3*8,32); 

fprintf(stderr,"%d->%d\n",merke->start,merke->end);
exit(0);
*/

RESETMARKS_E; fprintf(stderr,"hier\n");

construct_ipr=0;
merke=make_n_gon(6,maxlabel);
merke=make_n_gon(5,maxlabel);
for ( i=1; i<=1; i++) merke=add_n_gon(5, merke);
for ( i=1; i<=7; i++) merke=add_n_gon(6, merke);
for ( i=1; i<=0; i++) merke=add_n_gon(5, merke);
for ( i=1; i<=1; i++) merke=add_n_gon(6, merke);
for ( i=1; i<=1; i++) merke=add_n_gon(5, merke);
for ( i=1; i<=1; i++) merke=add_n_gon(6, merke);
for ( i=1; i<=1; i++) merke=add_n_gon(5, merke);
for ( i=1; i<=2; i++) merke=add_n_gon(6, merke);
for ( i=1; i<=2; i++) merke=add_n_gon(5, merke);
for ( i=1; i<=110; i++) merke=add_n_gon(6, merke);
for ( i=1; i<=0; i++) merke=add_n_gon(5, merke);

write_planar_code_label(stdout, merke);

/*
fprintf(stderr,"is_ipr: %d\n",is_ipr);

writemap();

get_representation(merke,representation);
for (i=0; i<2*nv-2; i++) fprintf(stderr,"%d ",representation[i]); fprintf(stderr,"\n");

fprintf(stderr,"%d\n",check_representation(merke->prev->invers->prev->invers->prev,representation,1));

if (auts_statistic[1]) fprintf(stderr,"With group C1: %d \n",auts_statistic[1]);
if (auts_statistic[2]) fprintf(stderr,"With group C2: %d \n",auts_statistic[2]);
if (auts_statistic[3]) fprintf(stderr,"With group C3: %d \n",auts_statistic[3]);
if (auts_statistic[5]) fprintf(stderr,"With group C5: %d \n",auts_statistic[5]);
if (auts_statistic[6]) fprintf(stderr,"With group C6: %d \n",auts_statistic[6]);

if (auts_statistic_mirror[1]) fprintf(stderr,"With group Cs: %d \n",auts_statistic_mirror[1]);
if (auts_statistic_mirror[2]) fprintf(stderr,"With group C2v: %d \n",auts_statistic_mirror[2]);
if (auts_statistic_mirror[3]) fprintf(stderr,"With group C3v: %d \n",auts_statistic_mirror[3]);
if (auts_statistic_mirror[5]) fprintf(stderr,"With group C5v: %d \n",auts_statistic_mirror[5]);
if (auts_statistic_mirror[6]) fprintf(stderr,"With group C6v: %d \n",auts_statistic_mirror[6]);
*/

RESETMARKS_E; fprintf(stderr,"hier2\n");

}

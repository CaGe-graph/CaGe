/* Program to fill boundaries with 5- and 6-gons so that every interior
   vertex has degree 3 and the vertices on the boundary have the given
   degrees. The boundaries are given in the form of a 2-3-series, (e.g.
   222232223) that describes the degrees of the vertices as see when 
   running (e.g. clockwise) along the boundary.

   The 2-3-series are to be considered cyclic and the output will be the
   same for two series that are identical as cyclic sequences, e.g.

   222232322233 <=> 322223232223  <=> 322232322223 (order reversed) 

   The number of pentagons must be at most 5 (otherwise there might
   even exist infinitely many fillings).

   USAGE: vul_in "2-3-series" [o] [t] [ipr] [ppath] [H]

   e.g.: vul_in 222232322233 will give the number of non-isomorphic fillings
   of 222232322233. Because neither stdout nor a file is given, no structures
   are written (but all are formed in the memory).

   vul_in 222232322233 o would construct the structures and write them to stdout.

   The option t makes it more or less just test for existence of a filling. As soon
   as one structure is found, the search is stopped. This structure can be outputted 
   using option 'o' though.

   The option ipr restricts the generation to structures with isolated pentagons only
   -- that is structures where no two (bounded) pentagons share an edge. The case
   of the boundary 22222 where the bounded and unbounded face are both pentagons and
   share 5 edges is also considered to be ipr.

    Option H makes the program place vertices of degree 1 (H-atoms) in the outer face 
    so that every boundary vertex of degree 2 gets degree 3.

    Option p (e.g. p/users/dummyuser/ ) makes the program look for datafiles
    in a certain directory (in the example in /users/dummyuser/ ).


   In case the program shall be used as a (sub-)routine in another program, one can 
   compile it with

   '-DMAINFUNCTION="int generate(int argc, char *argv[])"'

   and use "generate()" as that function.

*/

#ifndef MAINFUNCTION
#define MAINFUNCTION int main(int argc, char *argv[])
#define NORMAL_MAIN
#endif


#include <stdio.h> 
#include <limits.h> 
#include <string.h>
#if __STDC__
#include <stdlib.h>
#endif

#ifndef NOTIMES
#include <sys/times.h>
#ifndef CLK_TCK
#include <time.h>
#endif
#endif // NOTIMES

#ifndef MAXN
#define MAXN 5000            /* the maximum number of vertices; see above */
#endif
#define MAXE (3*MAXN)   /* the maximum number of oriented edges */
#define MAXF (2*MAXN-4)    /* the maximum number of faces */

#define MAXBOUNDARYLENGTH (2560) 
#define BOUNDARYFIELDS (((MAXBOUNDARYLENGTH-1)>>6)+2) // including safety for one extra field to set to 0

#define INSIDE (INT_MAX)
#define OUTSIDE (INT_MAX-1)
#define NOTFORBIDDEN (INT_MAX-2)

#define BIT(i) ( 1ULL <<(i))
#define NBIT(i) (~BIT(i))
#define BITMASK(i) (((i)==63)?(~(0ULL)):(BIT((i)+1)-1ULL)) // the bits on positions 0,1,...,i set, the others zero

/* Operations on the representations of the boundary */
#define MASK64 (63ULL)
#define SETBIT(boundary,position) ((boundary)[(position)>>6]|=BIT((position) & MASK64))
#define IS_SET(boundary,position) ((((boundary)[(position)>>6] & BIT((position) & MASK64))==0ULL)?0:1)
#define IS_SET_ULL(boundary,position) ((boundary)[(position)>>6] & BIT((position) & MASK64))
#define NOT_SET(boundary,position) ((((boundary)[(position)>>6] & BIT((position) & MASK64))==0ULL)?1:0)
#define NOT_SET_ULL(boundary,position) (!((boundary)[(position)>>6] & BIT((position) & MASK64)))
#define DELBIT(boundary,position) ((boundary)[(position)>>6]&=NBIT((position) & MASK64))
#define FIRSTBIT(boundary) (((boundary)[0])&1ULL)
#define CLEARBOUNDARY(boundary){int ii; for (ii=0;ii<BOUNDARYFIELDS;ii++) (boundary)[ii]=0ULL;}
#define COPYBOUNDARY(old,oldlengte,new,newlengte) \
{int _c; for (_c=((oldlengte)-1)>>6;_c>=0;_c--) (new)[_c]=(old)[_c]; (newlengte)=(oldlengte); }
#define COPYBOUNDARY2(old,oldlengte,new) \
{int _c; for (_c=((oldlengte)-1)>>6;_c>=0;_c--) (new)[_c]=(old)[_c]; }

#define SHIFT(boundary,lengte) \
{int _c, limit; limit=((lengte)-1)>>6; \
if (FIRSTBIT(boundary)) { \
  for (_c=0;_c<limit;_c++) {(boundary)[_c]=((boundary)[_c]>>1);\
                          if (FIRSTBIT((boundary)+_c+1)) SETBIT((boundary)+_c,63); }\
  (boundary)[limit]=(boundary)[limit]>>1; \
  SETBIT((boundary),lengte-1);}\
else { for (_c=0;_c<limit;_c++) {(boundary)[_c]=((boundary)[_c]>>1);\
                          if (FIRSTBIT((boundary)+_c+1)) SETBIT((boundary)+_c,63); }\
       (boundary)[limit]=(boundary)[limit]>>1; \
                         }}

// E_SHIFT doet hetzelfde als shift, maar een 1 op het einde verdwijnt 
#define E_SHIFT(boundary,lengte) \
{int _c, limit; limit=((lengte)-1)>>6; \
       for (_c=0;_c<limit;_c++) {(boundary)[_c]=((boundary)[_c]>>1);\
                                 if (FIRSTBIT((boundary)+_c+1)) SETBIT((boundary)+_c,63); }\
       (boundary)[limit]=(boundary)[limit]>>1; \
                          }


#define MINKNOWNBOUNDARY 7 // for which boundary lengths are the values stored
// MINKNOWNBOUNDARY is a constant and maxknownboundary is a variable computed at runtime
int data_read=0; // is the data for small boundaries read?

#define SKIP(length) ((1<<((length)-6))-2)    /* after how many bytes do the codes for length start? */
#define WHICHBYTE(number) ((int)(((unsigned long long int)(number))>>4))
/* in fact not "number" is analyzed, but "number" without the final 1 */
#define WHICHBIT(number) ((int)(((unsigned long long int)(number)>>1) & 7ULL))
//#define SETBIT(length,number) (binfield[SKIP(length)+WHICHBYTE(number)]|= (((unsigned char)1)<<WHICHBIT(number)))
#define ISPOSSIBLE(length,bitstring) \
(((length)>maxknownboundary) || ((length)<7) ||\
(binfield[SKIP(length)+WHICHBYTE(*(bitstring))] & (((unsigned char)1)<<WHICHBIT(*(bitstring)))))
#define IMPOSSIBLE(length,bitstring) (!(ISPOSSIBLE(length,bitstring)))
/* these makros assume that the boundary fits into one variable of
   type ULL in cases where the existence is looked up. But of course it would NEVER be possible
   to come close to length 64 for storing information on which boundares exist...*/


unsigned char *binfield=NULL; /* the field that stores the existence data for short boundaries */

#define WRITEUP { if (outputgraphs) write_planar_code(stdout); number_of_patches++;}

#define BOUNDARYTYPE unsigned long long int
typedef struct e /* The data type used for edges */
{ 
    int start;         /* vertex where the edge starts */
    int end;           /* vertex where the edge ends */ 
    struct e *prev;    /* previous edge in clockwise direction */
    struct e *next;    /* next edge in clockwise direction */
    struct e *inverse; /* the edge that is inverse to this one 
                          in case of edge->end == INSIDE or OUTSIDE the value 
			  of inverse is undefined */
    int pentagonleft; /* is there a pentagon on the left hand side of this directed edge?
		         This is only properly filled in, in case option ipr is used.
			 BUT: even in this case it is only guaranteed to be correct for edges with an 
			 interior that is to be filled on the right -- and the outer face is never 
			 counted as a pentagon. */
} EDGE;

typedef struct gtn /* The data type used for edges */
{ 
  EDGE *startedge; // what is the edge giving the boundary that has to be filled at this node?
  struct gtn *parent;
  struct gtn *firstpart;
  struct gtn *secondpart;
  /* firstpart and secondpart describe the parts that this inner boundary was split into */
  long long int node_id;
} GLUETREENODE;

long long int node_id_counter=0;

GLUETREENODE *lastnode; /* to be able to easily notice when the last boundary has been filled
			   and the patch can be written */
GLUETREENODE *backtrack_to; /* if a boundary can't be filled, the value backtrack_to is set to
                               the parent of the node that couldn't be filled. If equal
                               to NULL this means that no such node exists (or the whole boundary
                               can't be filled and the program is done. */


static int nv;             /* number of vertices; they are 0..nv-1 */
/* the number of oriented edges (including dangling ones) is always 3*nv */

BOUNDARYTYPE boundary[BOUNDARYFIELDS]; /* the binary description of the boundary */
EDGE *otherboundaries[MAXN/2]; /* the list of other edges of interna; boundaries to fill
				  after splitting one face. MAXN/2 is clearly larger than the upper
				  bound. */
int boundarylength, pentagons;
int auts, mirror;/* the rotations coded as minimal steplength in the direction of the reading
		    of the vertex degrees, and whether the boundary has a mirror
		    symmetry. The mirror symmetry is coded as a number. <0 means no mirror symmetry.
		    A number x greater or equal to 0 means: go x edges in direction of
		    the cycle and turn around -- from there on the sequence in the other direction 
		    is also minimal. */
EDGE *referenceedge; /* the starting edge of the construction and the edge that is the reference
			for the isomorphism rejection. The representation must be minimal
			from THIS edge compared to all edges equivalent to this one under the 
			automorphism group of the boundary. */

long long int number_of_patches, number_finished;

int outputgraphs=0, testing=0, maxknownboundary=0, ipr=0, ipr_option=0, add_H=0;
/* ipr_option is the flag and ipr is the flag used for the generation -- in case
   of ipr_option==1 and pentagons<2 the flag ipr is 0 otherwise it is the same as
   ipr_option */
int number_of_twos;

EDGE *list_of_edges=NULL; /* an array of MAXE edges */
#define STARTEDGE(i) list_of_edges+(3*(i)) /* the first edge of vertex i */

//#define FORBIDDEN(e) ((e)->forbidden!=NOTFORBIDDEN)

/*static int markvalue = 30000;
  #define RESETMARKS {int mki; if ((markvalue += 2) > (INT_MAX-3)) \
         { markvalue = 2; for (mki=0;mki<MAXE;++mki) list_of_edges[mki].mark=0;}}
  #define MARKLO(e) (e)->mark = markvalue
  #define MARKHI(e) (e)->mark = markvalue+1
  #define UNMARK(e) (e)->mark = markvalue-1
  #define ISMARKED(e) ((e)->mark >= markvalue)
  #define ISMARKEDLO(e) ((e)->mark == markvalue)
  #define ISMARKEDHI(e) ((e)->mark > markvalue)
*/

/* to see how the following variables are used and where the values come from,
   look into the comments of check_glueable */
int *minlist=NULL;
int minlist_nonipr[6]={38,33,28,25,22,21};
int minlist_ipr[6]={38,33,28,27,26,29};

/* minboundary[i] is the minimum boundary length for an _IPR_ patch with i pentagons */
int minboundary[6]={6,5,12,15,18,19};


void writegraph()
{
  int i,j;
  EDGE *run;

  fprintf(stderr,"----------------------nv=%d --------------------\n",nv);
  for (i=0; i<nv;i++)
    {
      fprintf(stderr,"%d:",i);
      run=STARTEDGE(i);
      for (j=0;j<3;j++,run=run->next)
	{ if (run->end==INSIDE) fprintf(stderr," in");
	else if (run->end==OUTSIDE) fprintf(stderr," out");
	else fprintf(stderr," %d",run->end);
	}
      fprintf(stderr,"\n");
    }

  fprintf(stderr,"---------------------------------------------------------\n");
}

void writeboundary(BOUNDARYTYPE boundary[], int lengte)
{

  int i;
  fprintf(stderr,"--------------------the--boundary----(length %d)----------------------------\n",lengte);
  for (i=lengte-1; i>=0; i--)
    {if ((i+1)%64==0) fprintf(stderr," "); 
    if (IS_SET(boundary,i)) fprintf(stderr,"3"); else  fprintf(stderr,"2");
    }
  fprintf(stderr,"\n");
  fprintf(stderr,"----------------------------------------------------\n");

}
void writeboundary_0(BOUNDARYTYPE boundary[], int lengte)
{

  int i;
  //fprintf(stderr,"--------------------the--boundary----(length %d)----------------------------\n",lengte);
  for (i=lengte-1; i>=0; i--)
    {if ((i+1)%64==0) fprintf(stderr," "); 
    if (IS_SET(boundary,i)) fprintf(stderr,"3"); else  fprintf(stderr,"2");
    }
  fprintf(stderr,"\n");
  //fprintf(stderr,"----------------------------------------------------\n");

}

void writeboundarypart(BOUNDARYTYPE boundary[], int lengte, int end, int start)
{

  int i;
  fprintf(stderr,"--------------------the--boundary----(length %d)----------------------------\n",lengte);
  for (i=end; i>=start; i--)
    {if ((i+1-start)%64==0) fprintf(stderr," "); 
    if (IS_SET(boundary,i)) fprintf(stderr,"3"); else  fprintf(stderr,"2");
    }
  fprintf(stderr,"\n");
  fprintf(stderr,"----------------------------------------------------\n");

}

void writeboundary_long(BOUNDARYTYPE boundary[], int lengte)
{

  int i;
  fprintf(stderr,"--------------------the--boundary----(length %d)----------------------------\n",lengte);
  for (i=lengte-1; i>=0; i--)
    {if ((i+1)%64==0) fprintf(stderr," "); 
    if (IS_SET(boundary,i)) fprintf(stderr,"(%d)3",i); else  fprintf(stderr,"(%d)2",i);
    }
  fprintf(stderr,"\n");
  fprintf(stderr,"----------------------------------------------------\n");

}


 int compare(BOUNDARYTYPE a[],BOUNDARYTYPE b[],int length)
   {
     int limit;
     limit=(length-1)>>6;

     while ((limit>0) && (a[limit]==b[limit])) limit--;

     /* can't return the difference since the return value shall be integer */

     if (a[limit]==b[limit]) return 0;
     if (a[limit]<b[limit]) return -1;
     return 1;
   }




void cutboundary(BOUNDARYTYPE oldb[], BOUNDARYTYPE newb[], int *newblengte,
		 int upper,int lower)
/* For CUTBOUNDARY it is necessary that upper>lower>=0. Then new becomes the sequence of bits
   from position upper to lower (including both) */

{
  int max_field, start_field, i, shiftright, restleft, bitslaatste;

#ifdef TEST
  if ((upper<lower)|| (lower<0)) { fprintf(stderr,"Problem in cutboundary\n"); exit(34); }
#endif

  /* one boundary element more than necessary is erased because some routines assume that
     they can add two some twos in front without having to do anything. */


  if (upper<=63)
    { newb[1]=0ULL;
      newb[0] = (oldb[0]&BITMASK(upper))>>lower;
      *newblengte=upper-lower+1;
      return;
    }

  if (upper<=127)
    { newb[2]=0ULL;
      if (lower>63)
	{ newb[1]=0ULL;
	  newb[0] = (oldb[1]&BITMASK(upper-64))>>(lower-64);
	  *newblengte=upper-lower+1;
	  return;
	}

      if (lower)
	{ 
	  newb[0] = (oldb[0]>>lower) | ((oldb[1]&BITMASK(upper-64))<<(64-lower));
	  newb[1] = (oldb[1]&BITMASK(upper-64)) >> lower;
	}
      else 
	{
	  newb[0] = oldb[0];
	  newb[1] = oldb[1]& BITMASK(upper-64);
	}

      *newblengte=upper-lower+1;
      return;
    }

  max_field= (upper>>6);
  start_field= (lower>>6);
  shiftright= lower & 63;
  restleft= 64-shiftright;
  bitslaatste= upper & 63;

  newb[max_field+1]=newb[max_field]=0ULL;

  if (max_field==start_field)
    {
      newb[0] = (oldb[max_field]&BITMASK(bitslaatste))>>shiftright;
      *newblengte=upper-lower+1;
      return;
    }
  


  if (shiftright !=0)
    {
      newb[0] = (oldb[start_field]>>shiftright);
      for (i=0, start_field++; start_field<max_field; start_field++)
	{
	  newb[i]= newb[i] | (oldb[start_field]<<restleft);
	  i++;
	  newb[i]= oldb[start_field]>>shiftright;
	}

      newb[i]= newb[i] | ((oldb[max_field] & BITMASK(bitslaatste)) << restleft);
      i++;
    }
  else
    for (i=0; start_field<max_field; start_field++, i++)
      {
	newb[i]= oldb[start_field];
      }

  newb[i]= (oldb[max_field] & BITMASK(bitslaatste)) >>shiftright;
  *newblengte=upper-lower+1;
  return;

}

void shiftboundary(BOUNDARYTYPE oldb[], BOUNDARYTYPE newb[], int lengte, int shift)

     /* Shifts the boundary by shift positions. oldb and neb can be the same! */

{
  int maxfield, i, shiftright, restlengte, oldpointer;
  BOUNDARYTYPE bufferboundary[2*BOUNDARYFIELDS];


#ifdef TEST
  if (shift>=lengte) { fprintf(stderr,"Problem in shiftfunction\n"); exit(37); }
#endif

  /* one boundary element more than necessary is erased because some routines assume that
     they can add two some twos in front without having to do anything. */

  if (shift==0) { if (oldb!=newb) COPYBOUNDARY2(oldb,lengte,newb); return; }

  /* now we can safely assume that shift-1>=0 */

  if (lengte<=64)
    { 
      newb[0] = ((oldb[0]>>shift) | (oldb[0] <<(lengte-shift))) & BITMASK(lengte-1);
      return;
    }

  maxfield= ((lengte-1)>>6);
  oldb[maxfield+1]=0ULL;
  restlengte= (lengte&63);
  shiftright= (shift>>6);
  shift &= 63;

  if (restlengte)
    { for (i=0;i<maxfield;i++) bufferboundary[i]=oldb[i];
      bufferboundary[maxfield]=oldb[maxfield] | oldb[0]<<restlengte;
      for (i=1;i<=shiftright;i++) bufferboundary[maxfield+i]=(oldb[i-1]>>(64-restlengte)) | (oldb[i]<<restlengte);
      if (i<=maxfield) bufferboundary[maxfield+i]=(oldb[i-1]>>(64-restlengte)) | (oldb[i]<<restlengte);
      else  bufferboundary[maxfield+i]=(oldb[i-1]>>(64-restlengte)) | (oldb[0]<<restlengte);
    }
  else
   {
     for (i=0;i<=maxfield;i++) bufferboundary[i]=bufferboundary[maxfield+1+i]=oldb[i];
    }

  if (shift)
    {
      oldpointer=shiftright;

      for (i=0; i<=maxfield; i++, oldpointer++)
	newb[i] = ((bufferboundary[oldpointer]>>shift) | (bufferboundary[oldpointer+1] << (64-shift)));
    }  
  else
   {
      oldpointer=shiftright;
      for (i=0; i<=maxfield; i++, oldpointer++)
	newb[i] = bufferboundary[oldpointer];
       
    } 

  if (restlengte) newb[maxfield] &= BITMASK(restlengte-1);

  return;

}


void checkleadingtwos(BOUNDARYTYPE boundary[],int lengte)
{
  int i,limit;

  limit=(((lengte-1)>>6)+1)<<6;

  for (i=lengte;i<limit;i++)
    if (IS_SET(boundary,i)) 
      { fprintf(stderr,"checkleadingtwos: Position %d is set and should be empty (length %d)\n", i,lengte);
      writeboundary(boundary,lengte);
      writeboundary(boundary,MAXBOUNDARYLENGTH);
      exit(20);
      }
  return;

}


void checkgraph(BOUNDARYTYPE boundary[], int length, EDGE *start)
{
  //int insides=0; 
  int i, j;
  EDGE *run;

  for (i=length-1, run=start;i>=0;i--)
    {
      if (IS_SET(boundary,i))
	{
	  if (run->inverse->prev->end!=INSIDE)
	    { fprintf(stderr,"graphtest: boundary or start wrong (1) -- position %d\n",length-i);
	      fprintf(stderr,"edge %d->%d\n",run->start,run->end);
	      writegraph(); writeboundary(boundary,length);
	      fprintf(stderr,"startedge %d->%d\n",start->start,start->end);
	      exit(0); }
	  //insides++;
	  run=run->inverse->next;
	}
      else
	{ if (run->inverse->prev->end==INSIDE)
	    { fprintf(stderr,"graphtest: boundary or start wrong (2)\n"); exit(0); }
	  run=run->inverse->prev;
	}
    }

  for (i=0; i<nv; i++)
    for (j=0;j<3; j++)
      {
	run=STARTEDGE(i)+j;
	if (run->start!=i) { fprintf(stderr,"graphtest: wrong startvalue (3)\n"); exit(0); }
	if ((run->end>=nv)  && (run->end!=INSIDE) && (run->end!=OUTSIDE))
	  { fprintf(stderr,"graphtest: wrong endvalue (4)\n"); exit(0); }
	if (run->end<0) { fprintf(stderr,"graphtest: wrong endvalue (5)\n"); exit(0); }
	if (run->end==i) { fprintf(stderr,"graphtest: wrong endvalue (6)\n"); exit(0); }
	if ((run->end!=INSIDE) && (run->end!=OUTSIDE) && (run->inverse->inverse!=run))
	  { fprintf(stderr,"graphtest: problem with the inverse values (7)\n"); exit(0); }
	//if (run->end==INSIDE) insides--;
      }

  //if (insides)  { fprintf(stderr,"graphtest: there are insides not in the inner boundary... (8)\n"); exit(0); }
  return;
}



/**************************************************************************/

static void
compute_code_sh_H(unsigned short code[], int *length)
 
     /* computes planar_code for unsigned short and adds vertices of degree 1
	for every edge pointing to OUTSIDE */

{
    register EDGE *run;
    int i, j; 
    unsigned short *start;
    int nextnumber, neighbours[2*MAXN];

    start=code;
 
    nextnumber=nv; code++;

    for (i=0; i<nv; i++)
      { run=STARTEDGE(i);
        for (j=0; j<3 ; j++)
	  {
	    if (run->end != OUTSIDE)
		{ *code = (run->end)+1; code++; }
	    else { neighbours[nextnumber]=i; nextnumber++; *code=nextnumber; code++; }
	    run=run->next;
	  }
	*code=0; code++;
      }
     
	for (;i<nextnumber;i++) {*code=neighbours[i]+1; code++; *code=0; code++; }
    
    *start=nextnumber;
    *length=code-start;
    return;
}




/**************************************************************************/

static void
compute_code_H(unsigned char code[], int *length)
 
     /* computes planar_code for unsigned short and adds vertices of degree 1
	for every edge pointing to OUTSIDE */

{
    register EDGE *run;
    int i, j; 
    unsigned char *start;
    int nextnumber, neighbours[2*MAXN];

    start=code;
 
    nextnumber=nv; code++;

    for (i=0; i<nv; i++)
      { run=STARTEDGE(i);
        for (j=0; j<3 ; j++)
	  {
	    if (run->end != OUTSIDE)
		{ *code = (run->end)+1; code++; }
	    else { neighbours[nextnumber]=i; nextnumber++; *code=nextnumber; code++; }
	    run=run->next;
	  }
	*code=0; code++;
      }
     
	for (;i<nextnumber;i++) {*code=neighbours[i]+1; code++; *code=0; code++; }
    
    *start=nextnumber;
    *length=code-start;
    return;
}



/**************************************************************************/

static void
compute_code_sh(unsigned short code[], int *length)
 
     /* computes planar_code for unsigned char */

{
    register EDGE *run;
    register int vertex;
    EDGE *temp;  
    EDGE *startedge[MAXN+1]; 
    int number[MAXN+1], i; 
    int last_number, actual_number;
    EDGE *givenedge;
    unsigned short *start;

    start=code;

    for (i = 0; i < nv; i++) number[i] = 0;
 
    *code=nv; code++;

    givenedge=referenceedge; 
    number[givenedge->start] = 1; 

    number[givenedge->end] = 2;
    last_number = 2;
    startedge[1] = givenedge->inverse;

    actual_number = 1;
    temp = givenedge;

    while (last_number < nv)
      {   if (temp->end!=OUTSIDE) { *code=number[temp->end]; code++; }
        for (run = temp->next; run != temp; run = run->next)
          { vertex = run->end;
            if (vertex != OUTSIDE) 
	      { if (!number[vertex])
		{ startedge[last_number] = run->inverse;
                  last_number++; number[vertex] = last_number; 
		  *code = last_number; }
	      else *code = number[vertex]; 
	      code++;
	      }
          }
        *code = 0;  code++;
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nv) 
      {  	if (temp->end!=OUTSIDE) { *code=number[temp->end]; code++; }
        for (run = temp->next; run != temp; run = run->next)
	  if (run->end!=OUTSIDE) { *code = number[run->end]; code++; }
          
        *code = 0;
        code++;
        temp = startedge[actual_number];  actual_number++;
    }

    *length= code-start;
    return;

}



/**************************************************************************/

static void
compute_code(unsigned char code[], int *length)
 
     /* computes planar_code for unsigned char */

{
    register EDGE *run;
    register int vertex;
    EDGE *temp;  
    EDGE *startedge[MAXN+1]; 
    int number[MAXN+1], i; 
    int last_number, actual_number;
    EDGE *givenedge;
    unsigned char *start;

    start=code;

    for (i = 0; i < nv; i++) number[i] = 0;
 
    *code=nv; code++;

    givenedge=referenceedge; 
    number[givenedge->start] = 1; 

    number[givenedge->end] = 2;
    last_number = 2;
    startedge[1] = givenedge->inverse;

    actual_number = 1;
    temp = givenedge;

    while (last_number < nv)
      {   if (temp->end!=OUTSIDE) { *code=number[temp->end]; code++; }
        for (run = temp->next; run != temp; run = run->next)
          { vertex = run->end;
            if (vertex != OUTSIDE) 
	      { if (!number[vertex])
		{ startedge[last_number] = run->inverse;
                  last_number++; number[vertex] = last_number; 
		  *code = last_number; }
	      else *code = number[vertex]; 
	      code++;
	      }
          }
        *code = 0;  code++;
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nv) 
      {  	if (temp->end!=OUTSIDE) { *code=number[temp->end]; code++; }
        for (run = temp->next; run != temp; run = run->next)
	  if (run->end!=OUTSIDE) { *code = number[run->end]; code++; }
          
        *code = 0;
        code++;
        temp = startedge[actual_number];  actual_number++;
    }

    *length=code-start;
    return;
}



static void
write_planar_code(FILE *f)   

     /* Write in planar_code format. */
{
    int length;
    unsigned char code[MAXN+MAXE+1];
    unsigned short shcode[MAXN+MAXE+1];
    static int first=1;

    if (first) { fprintf(stdout,">>planar_code<<"); first=0; }
    
    if (add_H)
      {
	if (nv+number_of_twos<=255)
	  {
	    compute_code_H(code,&length);
	    if (fwrite(code,sizeof(unsigned char),length,f) != length)
	      {
		fprintf(stderr,"fwrite() failed -- exiting!\n");
		exit(1);
	      }
	  }
	else 
	  {
	    compute_code_sh_H(shcode,&length);
	    putc(0,f);
	    if (fwrite(shcode,sizeof(unsigned short),length,f) != length)
	      {
		fprintf(stderr,"fwrite() failed -- exiting\n");
		exit(1);
	      }
	  }
      }
    else /* do not add H */
      {
	if (nv<=255)
	  {
	    compute_code(code,&length);
	    if (fwrite(code,sizeof(unsigned char),length,f) != length)
	      {
		fprintf(stderr,"fwrite() failed -- exiting!\n");
		exit(1);
	      }
	  }
	else 
	  {
	    compute_code_sh(shcode,&length);
	    putc(0,f);
	    if (fwrite(shcode,sizeof(unsigned short),length,f) != length)
	      {
		fprintf(stderr,"fwrite() failed -- exiting\n");
		exit(1);
	      }
	  }
      }


    return;
}



/**************************************************************************/

static void
compute_code_debug_long(unsigned short code[], int *length)
 
     /* computes planar_code for unsigned short for intermediate graphs -- when also
	edges pointing to "INSIDE" are still present. It uses (almost) the original vertex numbers,
	which are not guaranteed to give a proper code -- so just for debugging! 

	BUT: the original vertex numbers are shifted by one, because 0 has a special meaning
	in planarcode.
     */

{
    register EDGE *run;
    int i, j; 
    unsigned short *start;
    int nextnumber, neighbours[2*MAXN];

    start=code;
 
    nextnumber=nv; code++;

    for (i=0; i<nv; i++)
      { run=STARTEDGE(i);
        for (j=0; j<3 ; j++)
	  {
	    if (run->end != OUTSIDE)
	      { if (run->end != INSIDE)
		{ *code = (run->end)+1; code++; }
	      //else { neighbours[nextnumber]=i+1; nextnumber++; *code=nextnumber; code++; }
	      }
	    //else
	    //  {
	    //	*code = nextnumber+1; code++; 
	    //	newvertices[nextnumber]=i; nextnumber++;
	    //  }
	    run=run->next;
	  }
	*code=0; code++;
	  }
     
	for (;i<nextnumber;i++) {*code=neighbours[i]; code++; *code=0; code++; }
    
    //for (; i<nextnumber;i++) {*code=newvertices[i]+1; *code++; *code=0; code++;}

    *start=nextnumber;
    *length=code-start;
    return;
}


static void
write_planar_code_debug()   

     /* Write in planar_code format -- also intermediate graphs with edges to 
	"INSIDE" still present. */
{
    int length;
    unsigned short shcode[MAXN+MAXE+1];
    static int first=1;
    FILE *f=stdout;

    if (first) { fprintf(f,">>planar_code<<"); first=0; }
    
    compute_code_debug_long(shcode,&length);
    putc(0,f);
    if (fwrite(shcode,sizeof(unsigned short),length,f) != length)
      {
	fprintf(stderr,"fwrite() failed -- exiting!\n");
	exit(1);
      }
    fflush(stdout);

 
    return;
}

void initedges()
     /* pre-initialize the edges around each vertex */
{ int i,j;
  EDGE *e[3];

  if (list_of_edges!=NULL) return;

  list_of_edges=malloc(sizeof(EDGE)*MAXE);
  if (list_of_edges==NULL) 
    { fprintf(stderr,"Can't allocate memory for edges -- exiting.\n");
      exit(7); }

 for (i=0;i<MAXN;i++)
   {
     e[0]=STARTEDGE(i); e[1]=e[0]+1; e[2]=e[1]+1;
     for (j=0;j<3;j++) 
       { (e[j])->start=i; 
         (e[j])->pentagonleft=0;
         (e[j])->next=e[(j+1)%3];
         (e[j])->prev=e[(j+2)%3];
       }
   }
}

void initboundary(BOUNDARYTYPE boundary[], int lengte, EDGE **start)
/* Initializes the outer boundary of the patch that is to be filled.
   In start a pointer to the edge that points to the first item in boundarylist 
   and has the interior on the right is returned */
{ int i;
  EDGE *run, *startinvers, *back, *forward; 

  run=STARTEDGE(0);
  if (IS_SET(boundary,lengte-1))
    {
      run->end=INSIDE;
      startinvers=run->next;
      startinvers->end=lengte-1;
      forward=startinvers->next;
      forward->end=1;
    }
  else
    {
      run->end=OUTSIDE;
      startinvers=run->prev;
      startinvers->end=lengte-1;
      forward=startinvers->prev;
      forward->end=1;
    }

  for (i=1;i<boundarylength;i++)
    { run=STARTEDGE(i);
      if (IS_SET(boundary,lengte-1-i))
	{
	  run->end=INSIDE;
	  back=forward->inverse=run->next;
	  back->inverse=forward;
	  back->end=i-1;
	  forward=back->next;
	  forward->end=i+1;
	}
      else
	{
	  run->end=OUTSIDE;
	  back=forward->inverse=run->prev;
	  back->inverse=forward;
	  back->end=i-1;
	  forward=back->prev;
	  forward->end=i+1;
	}
    }

  forward->end=0;
  forward->inverse=startinvers;
  startinvers->inverse=forward;

  *start=forward;

  nv=lengte;
  return;

}

void usage(char name[])
{
  fprintf(stderr,"Usage: %s \"2-3-series\" [ipr] [o] [t] [H] [ppath] \n",name);
  fprintf(stderr,"e.g. %s 222232322233 \n",name);
  fprintf(stderr,"Without an output option, structures are just counted.\n");
  fprintf(stderr,"Option ipr makes the program generate only structures with\n");
  fprintf(stderr,"isolated pentagons -- so without pentagons sharing an edge.\n");
  fprintf(stderr,"With option o they are written as planacode to stdout.\n");
  fprintf(stderr,"Option t (test existence) makes it look just one structure\n");
  fprintf(stderr,"Option H makes the program place vertices of degree 1 (H-atoms)\n");
  fprintf(stderr,"in the outer face so that every boundary vertex of degree 2 gets degree 3.\n");
  fprintf(stderr,"Option p (e.g. p/users/dummyuser/ ) makes the program look for datafiles\n");
  fprintf(stderr,"in a certain directory (in the example in /users/dummyuser/ )\n");
  exit(4);

}


void canonize(BOUNDARYTYPE boundary[], int lengte, int *auts, int *mirror, int *numberoftwos)
     /* canonizes the sequence in boundary in a way that -- as a number -- it is minimal.
        This means that as many 0s as possible are in the beginning (vertices of degree 2).
	For auts and mirror look at the definition of the global variable.
     */
{
  int i, step, twos, dummy, result, onlymirror; 
  BOUNDARYTYPE min[BOUNDARYFIELDS], inverse[BOUNDARYFIELDS];

  *auts=0;

  COPYBOUNDARY(boundary,lengte,min,dummy);
  twos=0;

  /* the direction of shifting seems to be wrong, but the stepsize in each direction
     is of course the same... */
  for (i=1, step=1; i<lengte; i++, step++)
    { 
      SHIFT(boundary,lengte);
      if (FIRSTBIT(boundary)) // otherwise it's never a minimum
	{ result=compare(boundary,min,lengte);
	  if (result<0)
	    {  COPYBOUNDARY(boundary,lengte,min,dummy); step=0; }
	  else
	    if (result==0) { *auts=step; break; }
	}
    }

  /* *auts has the correct value after this loop even if the inverse is smaller */

  CLEARBOUNDARY(inverse);

  for (i=0; i<lengte; i++)
    if (IS_SET(min,i)) SETBIT(inverse,lengte-i-1);

  if (*auts==0) step=lengte; else step=*auts;

  *mirror= -1; // for now

  for (i=onlymirror=0; i<step; i++) // in less than step steps everything is seen
    { 
      if (FIRSTBIT(inverse))
	{ result=compare(inverse,min,lengte);
	  if (result<0)
	    { 
	      COPYBOUNDARY(inverse,lengte,min,dummy);
	      onlymirror=1;
	    }
	  else
	    if ((result==0) && !onlymirror) 
	      { (*mirror)=i; /* in less than step steps no two identical codes can be found
				except for identical to the mirror */
	      break;
	      }
	}
      SHIFT(inverse,lengte);
    }

  COPYBOUNDARY(min,lengte,boundary,dummy);

  for (twos=0, i=lengte-1; !(IS_SET(min,i)); twos++, i--);
  *numberoftwos=twos;

#ifdef TEST
  if (twos<2) { fprintf(stderr,"That shouldn't happen -- less than two 2s (just %d) in the beginning\n",twos);
                writeboundary(boundary,lengte);
                exit(17); }
#endif

  return;
}

void biggest_start(BOUNDARYTYPE boundary[], int lengte, EDGE **start, int *number_of_twos)
     /* sets *start to point at an edge pointing to a vertex that is
	the first degree 2 vertex in a sequence of 2s of (almost) maximal
	length. In fact the first edge in clockwise direction from
	*start with this property is chosen. *start must be a boundary
	edge with the inside on the right. 

	In fact as soon as a sequence of length 3 is found, it is taken, because
	it is as good as length 4 -- no identification with the opposite side
	is possible.

	In boundary the boundary as seen from *start is stored.

	In case it is detected that there is a series of 5 or more 2s, *start is set to NULL.
	This boundary cannot be filled.

     */
{
  int i, best, twos, shiftvalue;
  EDGE *bestedge, *run;

#ifdef TEST
  checkleadingtwos(boundary,lengte);
#endif

  if ((*boundary)==0ULL) { *number_of_twos=lengte; return; }


  run=*start;
  // searching a good begin starting at a degree 3 vertex
  twos=1;
#ifdef TEST
  if (IS_SET_ULL(boundary,lengte-1)) 
    { fprintf(stderr,"problem in biggest boundary -- start should point at 2\n");
      exit(1); }
#endif
  for (i=lengte-2; NOT_SET_ULL(boundary,i); i--) twos++;


  while (!(FIRSTBIT(boundary)))
    { 
      SHIFT(boundary,lengte);
      run=run->next->inverse;
      twos++;
    }

  if (twos>=3) // goed enough
    {
      *start=run; 
      *number_of_twos=twos;
      return;
    }


  SHIFT(boundary,lengte);
  run=run->prev->inverse;

  // now run points at a degree 3 vertex 

  bestedge=NULL;
  twos=0;
  best=1;
  shiftvalue=0;

  //writegraph();
  //writeboundary(boundary,lengte);

  for (i=0; (i<lengte); i++)
    { 
      if (IS_SET_ULL(boundary,i)) // run->start is a degree 3 vertex
	{ 
	run=run->prev->inverse;
	twos=0;
      }
      else 
	{ 
	run=run->next->inverse;
	twos++;
	if ((twos>best) && (IS_SET_ULL(boundary,i+1)))
	  { if (twos>=5) { *start=NULL; return; }
	  best=twos; bestedge=run; shiftvalue=i+1;
	  if (best>=3) break;
	  }
	}
    }

  *start=bestedge; 
  *number_of_twos=best;

  //writegraph();
  //writeboundary(boundary,lengte);

  //fprintf(stderr,"bestedge %d->%d\n",bestedge->start,bestedge->end);
  //fprintf(stderr,"length=%d, shift=%d\n",lengte,shiftvalue);
  //writeboundary_0(boundary,lengte);

  shiftboundary(boundary,boundary,lengte,shiftvalue);

  //writeboundary_0(boundary,lengte);

  return;

}


int check_boundary(BOUNDARYTYPE boundary[], int lengte)
     /* performs some basic checks whether the boundary is possible -- some checks were
        already performed while reading the code.*/

{ int i, twos;

  if (lengte<5) { fprintf(stderr,"Boundary too short -- no filling exists.\n"); return 0; }

  if (lengte<=6) { if (boundary[0] != 0ULL) 
                    { fprintf(stderr,"No filling exists.\n"); 
		      return 0; } 
                   else return 1;
                 }

  /* this check is always correct (checks a necessary criterion) but in case of non-canonical
     boundary strings it may miss something: */

  if (boundary[0]==0ULL)
    { fprintf(stderr,"Too much twos in a row -- no filling exists\n"); return 0; }

  while (NOT_SET(boundary,0)) SHIFT(boundary,lengte); 
  /* to make sure sequences of twos are in one part */


  for (i=twos=0; i<lengte; i++)
    {
      if (IS_SET(boundary,i)) twos=0;
      else { twos++; 
             if (twos==5) 
	       { 
#ifdef NORMAL_MAIN
fprintf(stderr,"A series of at least five 2's in boundary -- impossible to fill\n");
#endif
	         return 0;
	       }
           }
    }
  return 1;
}

int read_boundary(char boundarystring[], BOUNDARYTYPE boundary[],
		   int *boundarylength, int *pentagons)
     /* transfers a 2-3-string into the bitrepresentation in boundary. */
{
  int twos, threes, lengte;
  char *teken;

  twos=threes=lengte=0;

  CLEARBOUNDARY(boundary);

  for (teken=boundarystring; *teken != 0; teken++, lengte++)
    { if (lengte==MAXBOUNDARYLENGTH) 
      { fprintf(stderr,"Sorry -- boundary chosen too large for constant maxboundarylength\n");
         exit(19); }
      switch(*teken)
      { case '3': { SETBIT(boundary,lengte);
                    threes++; break; }
        case '2': { twos++; break; }
        default:  { fprintf(stderr,"Found %c in what should be 2-3sequence!\n",*teken);
	            fprintf(stderr,"Exiting!\n"); exit(2); }
      }
    }

  (*pentagons)=6-(twos-threes);

  number_of_twos=twos; // a global variable used when coding with additional H is chosen
  if ((*pentagons)<0 || (*pentagons)>5) 
    { 
#ifdef NORMAL_MAIN
fprintf(stderr,"This sequence is not possible with 0...5 pentagons (needs %d).\n",*pentagons);
#endif
      return 0;
    }

  *boundarylength=lengte;

  return 1;
}

int check_hexagonboundary(BOUNDARYTYPE boundary[], int length)
     /* Checks whether the boundary is closed in the hexagonal lattice. This is a necessary 
	but not sufficient test whether the boundary can be filled. Returns 1 if it is closed 
	and 0 otherwise. */
{

  int veca,vecb,suma,sumb,i,newa,newb;

  veca=suma=1;
  vecb=sumb=0;

  for (i=length-1; i>=0; i--)
    { if (IS_SET_ULL(boundary,i))
      { //newa=vecb;
        newb= vecb - veca;
	suma += vecb;
	sumb += newb;
	veca=vecb;
	vecb=newb;
      }
    else /* degree 2 */
      {
	newa= veca-vecb;
        //newb= veca;
	suma += newa;
	sumb += veca;
	vecb= veca;
	veca=newa;
      }
    }


  if ((suma!=1) || (sumb!=0) || (veca!=1) || (vecb!=0)) return 0;

  return 1;

}


int both_fillable(BOUNDARYTYPE boundary[],int length, int secondglueposition, int hexpatch)
     /* Checks whether gluing a hexagon to the boundaries at positions 0,length-1, length-2, length-3
	and secondglueposition+1, secondglueposition is possible by checking whether the two partial
	boundaries can be filled using the list of known boundaries.

	It is assumed that the boundary does not contain a sequence of more than two 2's -- otherwise
	difficulties may occur when searching for the first 3-entry in the parts.

	hexpatch is 0 if both parts would contain pentagons, 1 if the first part (length-4, length-5...)
	contains a pentagon, 2 if the second part has 0 pentagons and 3 if both parts have 0 pentagons.
     */
{

  BOUNDARYTYPE localboundary[BOUNDARYFIELDS];
  int locallength, limit, dummy;

  /* there is no sequence longer than two, so just test those */
  if (NOT_SET(boundary,length-4) && NOT_SET(boundary,secondglueposition+2))
    { if (NOT_SET(boundary,length-5) ||  NOT_SET(boundary,secondglueposition+3)) return 0; }
  /* this would be a sequence of at least 5 twos */

  if (NOT_SET(boundary,1) && NOT_SET(boundary,secondglueposition-1))
    { if (NOT_SET(boundary,2) ||  NOT_SET(boundary,secondglueposition-2)) return 0; }

  locallength=length-secondglueposition-3;

  if (locallength<=maxknownboundary)
    {
      
      
      for (limit=secondglueposition+2; NOT_SET(boundary,limit); limit++); 
      // stops at a degree 3 vertex -- last vertex degree 3 necessary to search in list
      cutboundary(boundary,localboundary,&dummy,length-4,limit);

#ifdef TEST
      { int i;
      for (i=locallength-1; i>length-4-limit; i--) 
	if (IS_SET(localboundary,i)) 
	  { fprintf(stderr,"problem after cutboundary!\n"); 
	  fprintf(stderr,"length=%d limit=%d\n",length,limit);
	  fprintf(stderr,"Problem for i=%d\n",i);
	  exit(1); }
      }	    
#endif
      if (IMPOSSIBLE(locallength,localboundary)) return 0;
    }
  else
    {
      if (hexpatch & 1)
	{
	  cutboundary(boundary,localboundary,&dummy,length-4,secondglueposition+2);

	  if (!check_hexagonboundary(localboundary,locallength)) return 0;
	}
    }
  // Now check the other side



  locallength=secondglueposition+1;

  if (locallength<=maxknownboundary)
    {

      for (limit=1; NOT_SET(boundary,limit); limit++);  // stops at a degree 3 vertex
      
      cutboundary(boundary,localboundary,&dummy,secondglueposition-1,limit);

      if (IMPOSSIBLE(locallength,localboundary)) return 0;
    }
  else
    { 
      if (hexpatch & 2)
	{
	  cutboundary(boundary,localboundary,&dummy,secondglueposition-1,1);

	  if (!check_hexagonboundary(localboundary,locallength)) return 0;
	}
    }


  return 1;

}

int check_glueable_list(EDGE *start,
			BOUNDARYTYPE boundary[],int length,
			EDGE *gluelist[], int *listlengte, int pentagons)
     /* checks whether there can be a face glued to the first two 2s in the
	sequence by looking for possible counterparts (two 3s) and checking whether according to
	the values read from the bindata_boundaries both parts can possibly be filled in. */

{
  int previous_set, i, wouldbepentagons, pentagon_distribution;
  EDGE *run;


  *listlengte=0;

#ifdef TEST
  if ((NOT_SET(boundary,0)) || (NOT_SET(boundary,length-3)) 
      || IS_SET(boundary,length-1)  || IS_SET(boundary,length-2)) 
    { fprintf(stderr,"Problems with startsequence (55)\n"); exit(50); }
#endif 


  run=start->inverse->prev->inverse->prev->inverse->next;
  /* here it is used that the sequence starts with exactly 2 twos */

  wouldbepentagons=3; 
  /* the first three that becomes a two in the boundary of the patch that is cut off leads
     to p=6-(twos - threes)=5 in the beginning. The last three will be counted as a three
     but changed to a two when the cut off part is considered -- tis is equalized by an initial
     minus two.*/
  for (i=4;i<=10;i++) 
    if (IS_SET_ULL(boundary,length-i)) 
      { run=run->inverse->next; wouldbepentagons++; }  
    else { run=run->inverse->prev; wouldbepentagons--; }

  if ((previous_set=IS_SET(boundary,length-11))) 
    { run=run->inverse->next; wouldbepentagons++; }
  else { run=run->inverse->prev; wouldbepentagons--; }

  // for comments on why 11 see check_glueable()

   for (i=length-12; i>=8; i--)
     { 
       if (IS_SET_ULL(boundary,i))
	 { if (wouldbepentagons==0) pentagon_distribution=1; 
	   else pentagon_distribution=0; 
	   if (wouldbepentagons==pentagons) pentagon_distribution|=2;
	 if (previous_set && (wouldbepentagons>=0) && (wouldbepentagons<=pentagons) 
	     && both_fillable(boundary,length,i,pentagon_distribution)) 
	   { gluelist[*listlengte]=run; (*listlengte)++; }

	 wouldbepentagons++;
	 run=run->inverse->next;  
	 previous_set=1;
       }
       else { previous_set=0; run=run->inverse->prev; wouldbepentagons--;}
     }

 if (*listlengte) return 1; else return 0;

}

int check_glueable_1_pent(EDGE *start,
			  BOUNDARYTYPE boundary[], int length,
			  EDGE *gluelist[], int *listlengte)
     /* 
	Checks whether when embedding the boundary in the
	hexagonal lattice, the boundary passes a certain series of 3
	edges in the lattice with the middle edge the edge opposite to
	the one following the first edge. "Opposite" means opposite in
	the hexagon in the lattice. 
	This would mean that there may be a way to add a hexagon and glue them to
	each other so that in one part there are only hexagons.

	check_glueable is only called in case of two 2s in the beginning of the boundary.

	It returns 1 if such an edge exists and 0 otherwise.

	In case of one pentagon present, it is NOT necessary that such an edge is passed
	when passing the boundary in one direction -- but in ONE OF the directions it
	must be passed.

	For the choice of vectors look at check_glueable_0_pent

     */

{

  int veca,vecb,suma,sumb,i,newa,newb,pents;
  EDGE *run;


#ifdef TEST
if (IS_SET(boundary,length-1) || IS_SET(boundary,length-2) || NOT_SET(boundary,length-3))
  { fprintf(stderr,"No suitable boundary for check_glueable!\n");
    writeboundary(boundary,length); 
    exit(18);
  }

#endif

 *listlengte=0;
 run=start;

  veca=suma=1;
  vecb=sumb=0;
  pents=4;

  /* The sum is the startpoint of the NEXT (so not the present) vector
     so there must be an edge starting at suma=1 sumb=2 and ending at
     suma=0 sumb=1 and at the beginning and end of this edge must be degree 3 */

  for (i=length-1; i>=12; i--) /* the last 13-1=12 edges cannot be the ones to identify it to */
    { if (IS_SET_ULL(boundary,i))
      { //newa=vecb; 
	pents++;
	run=run->inverse->next;
        newb= vecb - veca;
	suma += vecb;
	sumb += newb;
	veca=vecb;
	vecb=newb;
	/* the sum is the endpoint INCLUDING the new vector */
	if (((veca== -1) && (vecb==-1) && (IS_SET_ULL(boundary,i-1)) && (suma==0) && (sumb==1)) &&
	    // what follows is just checking that no 5 threes in a row are built
	    ((IS_SET_ULL(boundary,1) || IS_SET_ULL(boundary,i-2)) ||
	     (IS_SET_ULL(boundary,2) && IS_SET_ULL(boundary,i-3))) &&
	    ((IS_SET_ULL(boundary,length-4) || IS_SET_ULL(boundary,i+1)) ||
	     (IS_SET_ULL(boundary,length-5) && IS_SET_ULL(boundary,i+2)))
	    && (pents==0))
	  { //fprintf(stderr,"1pents a %d->%d  %d->%d\n",start->start,start->end,run->start,run->end);
	    gluelist[*listlengte]=run; (*listlengte)++; 
	  }
      }
    else /* degree 2 */
      { pents--;
	run=run->inverse->prev;
	newa= veca-vecb;
        //newb= veca;
	suma += newa;
	sumb += veca;
	vecb= veca;
	veca=newa;
      }
    }

  // Now the other direction!
  // It is not possible that in both directions the same edge is found -- this would mean
  // the boundary would be closed in the hexagonal lattice -- and since it contains a
  // pentagon this can't be.

  veca=-1;
  vecb=0;

  /* here we run in the other direction, so now the sum is the ENDpoint of the next edge 
     along the boundary, but beginning point in the running direction --
     so in the beginning it is 0,0 -- the original beginning point */

  suma=sumb=0;

  run=start;

  pents=2;
  /* here it is used that the sequence starts with two 2s. So when glueing the hexagon to
     the opposite edge, 4 vertices with degree 2 have not been counted. */

  for (i=0; i<length-15; i++) /* the last 13-1+3=15 edges cannot be the ones to identify it to 
				The plus three comes from the initial 3 edges -- the place where 
				the face is glued to. */
    { 
      if (IS_SET_ULL(boundary,i))
      { //newb=veca;
	pents++;
	run=run->prev->inverse;
        newa= veca - vecb;
	vecb=veca;
	veca=newa;
	suma += veca;
	sumb += vecb;
	/* the sum is the endpoint INCLUDING the new vector */
	if (((veca== 1) && (vecb== 1) && (IS_SET_ULL(boundary,i+1)) && (suma==1) && (sumb==2)) &&
	    // what follows is just checking that no 5 threes in a row are built
	    ((IS_SET_ULL(boundary,1) || IS_SET_ULL(boundary,i-1)) ||
	     (IS_SET_ULL(boundary,2) && IS_SET_ULL(boundary,i-2))) &&
	    ((IS_SET_ULL(boundary,length-4) || IS_SET_ULL(boundary,i+2)) ||
	     (IS_SET_ULL(boundary,length-5) && IS_SET_ULL(boundary,i+3)))
	    && (pents==0))
	  /* the endpoint is also changed in this direction! */
	  { 
	    gluelist[*listlengte]=run; (*listlengte)++; 
	  }		   
      }
    else /* degree 2 */
      { pents--;
	run=run->next->inverse;
	newb= vecb-veca;
        //newa= vecb;
	veca=vecb;
	vecb= newb;
	suma += veca;
	sumb += vecb;
      }
    }

  if (*listlengte) return 1; else return 0;

}

int check_glueable_0_pent(EDGE * start, BOUNDARYTYPE boundary[],int length, 
			  EDGE *gluelist[], int *listlengte)
     /* 
	For 0 pentagons.

	It is assumed that the boundary is closed in the hexagonal
	lattice.  Checks whether when embedding the boundary in the
	hexagonal lattice, the boundary passes a certain series of 3
	edges in the lattice with the middle edge the edge opposite to
	the one following the first edge. "Opposite" means opposite in
	the hexagon in the lattice. 
	This would mean that there may be a way to add a hexagon and glue them to
	each other.

	check_glueable_0 is only called in case of two 2s in the beginning of the boundary.

	It returns 1 if such an edge exists and 0 otherwise.

	The basis is a vector a being one side of a hexagon and a vector b being a rotated by 120
	degrees in clockwise direction.

     */

{

  int veca,vecb,suma,sumb,i,newa,newb,pents;
  EDGE *run;


 *listlengte=0;
 run=start;

  veca=suma=1;
  vecb=sumb=0;
  pents=4; /* the two 2s that come from the two corners not counted give an excess of 2
	      equivalent to 4 pentagons */

  /* The sum is the startpoint of the NEXT (so not the present) vector
     so there must be an edge starting at suma=1 sumb=2 and ending at
     suma=0 sumb=1 and at the beginning and end of this edge must be degree 3 */

  for (i=length-1; i>0; i--) /* the last place need not be checked */
    { if (IS_SET_ULL(boundary,i))
      { //newa=vecb;
	pents++;
	run=run->inverse->next;
        newb= vecb - veca;
	suma += vecb;
	sumb += newb;
	veca=vecb;
	vecb=newb;
	/* the sum is the endpoint INCLUDING the new vector */
	if (((veca== -1) && (vecb==-1) && (IS_SET_ULL(boundary,i-1)) && (suma==0) && (sumb==1))&&
	    // what follows is just checking that no 5 threes in a row are built
	    ((IS_SET_ULL(boundary,1) || IS_SET_ULL(boundary,i-2)) ||
	     (IS_SET_ULL(boundary,2) && IS_SET_ULL(boundary,i-3))) &&
	    ((IS_SET_ULL(boundary,length-4) || IS_SET_ULL(boundary,i+1)) ||
	     (IS_SET_ULL(boundary,length-5) && IS_SET_ULL(boundary,i+2)))
	    && (pents==0))
	  { //fprintf(stderr,"0pents %d->%d  %d->%d\n",start->start,start->end,run->start,run->end);
	    gluelist[*listlengte]=run; (*listlengte)++; 
	  }
      }
    else /* degree 2 */
      { pents--;
	run=run->inverse->prev;
	newa= veca-vecb;
        //newb= veca;
	suma += newa;
	sumb += veca;
	vecb= veca;
	veca=newa;
      }
    }

  if (*listlengte) return 1; else return 0;

}


int check_glueable(EDGE * start, BOUNDARYTYPE boundary[],int length, int pentagons,
		   EDGE *gluelist[], int *listlengte)
     /* 

	check_glueable is only called in case of two 2s in the beginning of the boundary.
	It computes a list of edges to which the sequence of two 2s starting at start
	can possibly be glued.

	It returns 1 if such an edge exists and 0 otherwise.

     */

{


#ifdef TEST
if (IS_SET(boundary,length-1) || IS_SET(boundary,length-2) || NOT_SET(boundary,length-3))
  { fprintf(stderr,"No suitable boundary for check_glueable (2)!\n");
    writeboundary(boundary,length); 
    exit(18);
  }
#endif

/* the earliest position to which it can be glued is length-11, length -12
   (this would contain 3 pentagons). Otherwise there would always be a sequence 
   of 3 2's in the boundary. But you can't have 3 pentagons on both sides,
   so one side must contain a hexagon and two pentagons -- that would
   give the shortest boundary.

   The following values for the shortest boundary length are based on the minimum 
   values for two parts plus 2 for the initial two 2s.

   For the parts we get as minimum boundary length

   0 pentagons: 18
   1 pentagon : 13
   2 pentagons: 10
   3 pentagons: 9

   IPR:
   0 pentagons: 18
   1 pentagon : 13
   2 pentagons: 12
   3 pentagons: 15


   for 4 and 5 the values would give worse bounds, so that the lower bound is achieved by the sum
   of two smaller values.

*/


if (length<minlist[pentagons]) return 0; 


if ((pentagons>1) || (data_read && (length<2*maxknownboundary+2))) 
  return check_glueable_list(start, boundary, length, gluelist, listlengte, pentagons);
/* then this test has a better chance to detect non-existence -- this is a guess -- maybe the border
   should even be chosen higher... */ 

  if (pentagons==1) return check_glueable_1_pent(start, boundary, length, gluelist, listlengte);

  if (pentagons==0) return check_glueable_0_pent(start, boundary, length, gluelist, listlengte);

  fprintf(stderr,"Problem in program -- I shouldn't reach this point!\n"); exit(123);

}


void addface5(EDGE *start, BOUNDARYTYPE boundary[], int boundarylength, int twos, 
	     EDGE **newstart, BOUNDARYTYPE newboundary[], int *newboundarylength, int *newtwos)
     /* like addface -- only that it is checked whether two neighbouring pentagons are produced.
	Even in that case the pentagon is added in order to be compatible with the other routines
	where it is added but biggeststart sets newstart to NULL.
     */

{
  EDGE *run, *realstart, *glueedge;
  int newvertices, commonvertices, i;

#define size 5

  //fprintf(stderr,"adding %d gon at %d->%d\n",size,(start)->start,(start)->end);


  realstart=start->next;
  COPYBOUNDARY(boundary,boundarylength,newboundary,(*newboundarylength));

#ifdef TEST
  if (realstart->end != INSIDE) { fprintf(stderr,"Should point to inside...\n"); exit(10); }
  if (start->inverse->prev->end == INSIDE) { fprintf(stderr,"Should NOT point to inside...\n"); exit(11); }
#endif

  commonvertices=twos+2;
  newvertices=size-commonvertices;

  if (newvertices+nv > MAXN) 
    { fprintf(stderr,"Problem -- the number of vertices would exceed MAXN=%d\n", MAXN);
      exit(20); }

  for (run=start->inverse->prev; run->end!=INSIDE; run=run->inverse->prev);
  //search end of where to glue


  switch(newvertices)
    {
    case 0:{ realstart->inverse=run; run->inverse=realstart;
             realstart->end=run->start; run->end=realstart->start;
	     realstart->pentagonleft=1;
	     *newboundarylength=boundarylength-size+2;
	     E_SHIFT(newboundary,boundarylength);
	     DELBIT(newboundary,boundarylength-size);
	     *newstart=realstart->next->inverse; /* points to first 2 */
	     break;
           }
    case 1:{ glueedge=STARTEDGE(nv); 
             glueedge->end=INSIDE; glueedge++; /* that is the same as next in this context */
	     glueedge->end=realstart->start; realstart->end=nv;
	     glueedge->inverse=realstart; realstart->inverse=glueedge;
	     glueedge++;
	     glueedge->pentagonleft=realstart->pentagonleft=1;
	     glueedge->end=run->start; run->end=nv;
	     glueedge->inverse=run; run->inverse=glueedge;
	     *newboundarylength=boundarylength-size+4;
	     E_SHIFT(newboundary,boundarylength);
	     DELBIT(newboundary,boundarylength-size+1);
	     SETBIT(newboundary,boundarylength-size+2);
	     /* the first 0 must not be set -- it's already there */
	     *newstart=realstart->next->inverse; /* points to first 2 */
	     nv++;
             break;
           }
    default: { fprintf(stderr,"Shit! This point should never be reached in addface5!\n"); exit(14); }

    }

  for (i=0, run=start; i<=twos; i++, run=run->inverse->prev)
    {
      if (run->pentagonleft) { *newstart=NULL; return; }
    }

  biggest_start(newboundary, *newboundarylength, newstart, newtwos);


  return;

#undef size

}


void addface(EDGE *start, BOUNDARYTYPE boundary[], int boundarylength, int twos, int size, 
	     EDGE **newstart, BOUNDARYTYPE newboundary[], int *newboundarylength, int *newtwos)
     /* adds a face of size "size" to the partial patch starting at the edge "start" 
	which must have the interior on the right and run from a vertex with an edge
	to the interior (considered as deg 3) to one without such an edge (considered as
	degree 2).
	In newstart resp. newboundary the new start and new boundary are returned.
	The new start is at the beginning of a series of twos with maximal length 
	and has the interior on the right.
     */
{
  EDGE *run, *realstart, *glueedge, *glueedge2;
  int newvertices, commonvertices;

  //fprintf(stderr,"adding %d gon at %d->%d\n",size,(start)->start,(start)->end);

  realstart=start->next;
  COPYBOUNDARY(boundary,boundarylength,newboundary,(*newboundarylength));

#ifdef TEST
  if (realstart->end != INSIDE) { fprintf(stderr,"Should point to inside...\n"); exit(10); }
  if (start->inverse->prev->end == INSIDE) { fprintf(stderr,"Should NOT point to inside...\n"); exit(11); }
  if ((size!=5) && (size!=6)) 
    { fprintf(stderr,"size must be 5 or 6 \n"); exit(12); }
#endif

  commonvertices=twos+2;
  newvertices=size-commonvertices;

  if (newvertices+nv > MAXN) 
    { fprintf(stderr,"Problem -- the number of vertices would exceed MAXN=%d\n", MAXN);
      exit(20); }

  for (run=start->inverse->prev; run->end!=INSIDE; run=run->inverse->prev);
  //search end of where to glue


  switch(newvertices)
    {
    case 0:{ realstart->inverse=run; run->inverse=realstart;
             realstart->end=run->start; run->end=realstart->start;
	     *newboundarylength=boundarylength-size+2;
	     E_SHIFT(newboundary,boundarylength);
	     DELBIT(newboundary,boundarylength-size);
	     *newstart=realstart->next->inverse; /* points to first 2 */
	     break;
           }
    case 1:{ glueedge=STARTEDGE(nv); 
             glueedge->end=INSIDE; glueedge++; /* that is the same as next in this context */
	     glueedge->end=realstart->start; realstart->end=nv;
	     glueedge->inverse=realstart; realstart->inverse=glueedge;
	     glueedge++;
	     glueedge->end=run->start; run->end=nv;
	     glueedge->inverse=run; run->inverse=glueedge;
	     *newboundarylength=boundarylength-size+4;
	     E_SHIFT(newboundary,boundarylength);
	     DELBIT(newboundary,boundarylength-size+1);
	     SETBIT(newboundary,boundarylength-size+2);
	     /* the first 0 must not be set -- it's already there */
	     *newstart=realstart->next->inverse; /* points to first 2 */
	     nv++;
             break;
           }
    case 2:{ // the algorithm ensures that here the size is always 6
#ifdef TEST
      if (size!=6) { fprintf(stderr,"algorithm implies that the size should be 6 here...\n"); exit(13); }
#endif
             glueedge=STARTEDGE(nv); 
             glueedge->end=INSIDE; glueedge++; /* that is the same as next in this context */
	     glueedge->end=realstart->start; realstart->end=nv;
	     glueedge->inverse=realstart; realstart->inverse=glueedge;
	     glueedge++;
	     glueedge2=STARTEDGE(nv+1);
	     glueedge2->end=INSIDE; glueedge2++;
	     glueedge->end=nv+1; glueedge2->end=nv;
	     glueedge->inverse=glueedge2; glueedge2->inverse=glueedge;
	     glueedge2++;
	     glueedge2->end=run->start; run->end=nv+1;
	     glueedge2->inverse=run; run->inverse=glueedge2;

	     *newboundarylength=boundarylength;
	     E_SHIFT(newboundary,boundarylength);
	     DELBIT(newboundary,boundarylength-4);
	     SETBIT(newboundary,boundarylength-3);
	     SETBIT(newboundary,boundarylength-2);
	     *newstart=realstart->next->inverse; /* points to first 2 */
	     nv+=2;
             break;
           }
    default: { fprintf(stderr,"Shit! This point should never be reached!\n"); exit(14); }

    }

  biggest_start(newboundary, *newboundarylength, newstart, newtwos);


  return;

}



void removeface(EDGE *start, int howmuchnew)
     /* This routine just restores the values of the edges in the boundary as before
	adding the face on the left of start and fixes nv. The binary encoding of the
	boundary and the startedge for that encoding were stored, so they don't have to be 
	restored. *start must be the first edge on the boundary of the face that is to
	be removed. howmuchnew is the number of new vertices. */

{ 
#ifdef TEST
  if (start->next->end==INSIDE) { fprintf(stderr,"wrong edge given.\n"); exit(16); }
#endif

  start->end=INSIDE;
  nv -= howmuchnew;
  for ( ; howmuchnew; howmuchnew--) { start=start->inverse->next; } 
  start->inverse->end=INSIDE;

  return;
}

void removeface_pent_ipr(EDGE *start, int howmuchnew)
     /* like removeface -- only that the pentagonleft values are reset */


{ 
#ifdef TEST
  if (start->next->end==INSIDE) { fprintf(stderr,"wrong edge given.\n"); exit(16); }
#endif

  start->end=INSIDE;
  start->pentagonleft=0;
  nv -= howmuchnew;
  for ( ; howmuchnew; howmuchnew--) { start=start->inverse->next; start->pentagonleft=0;} 
  start->inverse->end=INSIDE;

  return;
}


/****************************************************************************/
 
static int
testrep(EDGE *givenedge, int representation[])
 
/* Returns 0 if a representation constructed from this edge would be smaller than
   the given representation and 1 otherwise (also if it is equal). */

{
    register EDGE *run;
    register int vertex;
    EDGE *temp;  
    EDGE *startedge[MAXN+1]; 
    int number[MAXN], i; 
    int last_number, actual_number;

#ifdef TEST

    if (givenedge->end==OUTSIDE) 
      { fprintf(stderr,"impossible starting edge for representation!\n");
        exit(0); }
   if (givenedge->inverse->next->end!=OUTSIDE) 
      { fprintf(stderr,"illegal starting edge for representation!\n");
        exit(0); }

#endif
 
    for (i = 0; i < nv; i++) number[i] = 0;
 
    number[givenedge->start] = 1; 

    number[givenedge->end] = 2;
    last_number = 2;
    startedge[1] = givenedge->inverse;

    actual_number = 1;
    temp = givenedge;

    while (last_number < nv)
    {  
        for (run = temp->next; run != temp; run = run->next)
          { vertex = run->end;
            if (vertex != OUTSIDE)
	      { if (!number[vertex])
		{ startedge[last_number] = run->inverse;
                  last_number++; number[vertex] = last_number; 
		}
	      if (*representation < number[vertex]) return 1;
	      else if (*representation > number[vertex]) return 0;
	      representation++;
	      }
          }
        if (*representation) return 0;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nv) 
    {  
        for (run = temp->next; run != temp; run = run->next)
	  { vertex = run->end;
	    if (vertex != OUTSIDE)
	      { 
		if (*representation < number[vertex]) return 1;
		else if (*representation > number[vertex]) return 0;
		representation++;
	      }
	  }
        if (*representation) return 0;
	representation++;
	temp = startedge[actual_number];  actual_number++;
	  
    }

    return 1;
}


/****************************************************************************/
 
static int
testrep_mirror(EDGE *givenedge, int representation[])
 
/* Returns 0 if a representation constructed from this edge (which must have the outside
   on its right) -- in ANTI-clockwise direction would be smaller than
   the given representation and 1 otherwise (also if it is equal). */

{
    register EDGE *run;
    register int vertex;
    EDGE *temp;  
    EDGE *startedge[MAXN+1]; 
    int number[MAXN], i; 
    int last_number, actual_number;

#ifdef TEST

    if (givenedge->end==OUTSIDE) 
      { fprintf(stderr,"impossible starting edge for representation!\n");
        exit(0); }
    if (givenedge->inverse->prev->end!=OUTSIDE) 
      { fprintf(stderr,"illegal starting edge for representation! %d->%d\n",givenedge->start,givenedge->end);
        writegraph();
        exit(0); }
#endif
 
    for (i = 0; i < nv; i++) number[i] = 0;

    number[givenedge->start] = 1; 

    number[givenedge->end] = 2;
    last_number = 2;
    startedge[1] = givenedge->inverse;

    actual_number = 1;
    temp = givenedge;

    while (last_number < nv)
    {  
        for (run = temp->prev; run != temp; run = run->prev)
          { vertex = run->end;
            if (vertex != OUTSIDE)
	      { if (!number[vertex])
		  { startedge[last_number] = run->inverse;
		  last_number++; number[vertex] = last_number; 
		  }
		
		  if (*representation < number[vertex]) return 1;
		  else if (*representation > number[vertex]) return 0;
		  representation++;
	      }	       
          }
        if (*representation) return 0;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }


    while (actual_number <= nv) 
    {  
        for (run = temp->prev; run != temp; run = run->prev)
	  { vertex = run->end;
	    if (vertex != OUTSIDE)
	      { 
		if (*representation < number[vertex]) return 1;
		else if (*representation > number[vertex]) return 0;
		representation++;
	      }
	  }
        if (*representation) return 0;
	representation++;
	temp = startedge[actual_number];  actual_number++;
	  
    }

    return 1;
}




/****************************************************************************/
 
static void
getrep_init(EDGE *givenedge, int representation[])
 
/* Constructs a representation in clockwise direction starting at the edge givenedge.
   The representation is written to representation[]. This is the reference and afterwards
   is tested whether from some of the edges on the boundary that are equivalent under boundary
   isomorphisms a smaller representation can be found. */

{
    register EDGE *run;
    register int vertex;
    EDGE *temp;  
    EDGE *startedge[MAXN+1]; 
    int number[MAXN], i; 
    int last_number, actual_number;

#ifdef TEST

    if (givenedge->end==OUTSIDE) 
      { fprintf(stderr,"impossible starting edge for representation!\n");
        exit(0); }
   if (givenedge->inverse->next->end!=OUTSIDE) 
      { fprintf(stderr,"illegal starting edge for representation!\n");
        exit(0); }

#endif
 
    for (i = 0; i < nv; i++) number[i] = 0;
 
    number[givenedge->start] = 1; 

    number[givenedge->end] = 2;
    last_number = 2;
    startedge[1] = givenedge->inverse;

    actual_number = 1;
    temp = givenedge;

    while (last_number < nv)
    {  
        for (run = temp->next; run != temp; run = run->next)
          { vertex = run->end;
            if (vertex != OUTSIDE)
	      { if (!number[vertex])
		{ startedge[last_number] = run->inverse;
                last_number++; number[vertex] = last_number; 
		}
	      *representation = number[vertex]; 
	      representation++;
	      }
	  }
	*representation = 0;
	representation++;
	temp = startedge[actual_number];  actual_number++;
    }

    while (actual_number <= nv) 
    {  
        for (run = temp->next; run != temp; run = run->next)
	  if (run->end != OUTSIDE)
          { 
            *representation = number[run->end]; representation++;
          }
        *representation = 0;
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }

    return;
}


void ready()

     /* when a patch is ready it is tested whether the lexicographic minimal
	representation is the one starting at the reference edge. If yes, it is accepted, otherwise it
	is rejected. */
{

  int reference[4*MAXN];
  int i,j;
  EDGE *run;

  number_finished++; 

  if ((auts==0) && (mirror<0)) { WRITEUP; return; }

  getrep_init(referenceedge, reference);

  if (auts>0)
  for (i=(boundarylength/auts) -1, run=referenceedge; i>0 ; i--)
  {
    for (j=0;j<auts;j++)
      { run=run->inverse->next; if (run->end==OUTSIDE) run=run->next; }
      if (testrep(run,reference)==0) return;
  }

  if (mirror>=0)
    { run=referenceedge;
      for (j=0;j<mirror;j++)
      { run=run->inverse->next; if (run->end==OUTSIDE) run=run->next; }
      run=run->inverse;
      /* now the starting edge is one for the testing the mirror representations */

      if (auts==0) i=1; else i=(boundarylength/auts);
      for ( ; i>0 ; i--)
	{
	  for (j=0;j<auts;j++)
	    { run=run->inverse->prev; if (run->end==OUTSIDE) run=run->prev; }
	  if (testrep_mirror(run,reference)==0) return;
	}
    }

  WRITEUP;

  return;

}

void glue(EDGE *start, EDGE *otherside)
     /* glues the sequence of 3 edges starting at start to the edge otherside in order
	to form a hexagon going from boundary to boundary. It is assumed that the sequence
	of 3 edges starts with a degree 3 vertex, then two degree 2 vertex come and then another
	degree 3 vertex. The otherside must start and end in a degree 3 vertex. "degree 3 vertex"
	means that rthere is an edge going to INSIDE */


{EDGE *buffer, *buffer2;


#ifdef TEST
 if (otherside->next->end!=INSIDE) { fprintf(stderr,"Error glue (1)\n"); exit(0); }
 if (otherside->inverse->prev->end!=INSIDE) { fprintf(stderr,"Error glue (2)\n"); exit(0); }
 if (start->next->end!=INSIDE) { fprintf(stderr,"Error glue (3)\n"); exit(0); }
 if (start->inverse->prev->inverse->prev->inverse->prev->end!=INSIDE) 
   { fprintf(stderr,"Error glue (4)\n"); exit(0); }

#endif

 buffer=start->next;
 buffer->end=otherside->end;
 buffer->inverse=buffer2=otherside->inverse->prev;

 buffer2->end=buffer->start;
 buffer2->inverse=buffer;

 buffer=start->inverse->prev->inverse->prev->inverse->prev;
 buffer->end=otherside->start;
 buffer->inverse=buffer2=otherside->next;

 buffer2->end=buffer->start;
 buffer2->inverse=buffer;

 return;
}

void unglue(EDGE *start, EDGE *otherside)
{EDGE *buffer, *buffer2;


// fprintf(stderr,"-------------------unglueing %d->%d   %d->%d\n",start->start,start->end,otherside->start,otherside->end); 

 buffer=start->next;
 buffer->end=INSIDE;
 buffer->inverse=NULL;

 buffer2=otherside->inverse->prev;

 buffer2->end=INSIDE;
 buffer2->inverse=NULL;

 buffer=start->inverse->prev->inverse->prev->inverse->prev;
 buffer->end=INSIDE;
 buffer->inverse=NULL;

 buffer2=otherside->next;

 buffer2->end=INSIDE;
 buffer2->inverse=NULL;

 return;
}


void determine_boundary(BOUNDARYTYPE boundary[], int *length, EDGE **startedge, int *pentagons, 
			EDGE *start, int *twosinrow)
     /* Determines the boundary, the length and the number of pentagons on the right hand side
	of start. It is assumed that the interior to fill is produced by inserting a face --
	or to be exact: that there is at least one edge going to the inside!
     */
{ EDGE *end;
  int twos, threes, pos; 


  CLEARBOUNDARY(boundary);
  while (start->next->end != INSIDE) start=start->next->inverse;


  //fprintf(stderr,"%d->%d\n",start->start,start->end);

  *startedge=start;

  end=start;
  start=start->prev->inverse;
  threes=1;
  SETBIT(boundary,0);
  twos=0;

  for (pos=1; start!=end; pos++)
    { start=start->next;
    if (start->end== INSIDE) { threes++; start=start->next->inverse; SETBIT(boundary,pos);}
    else { twos++; start=start->inverse; }
    }

  *length=pos;
  *pentagons=6-twos+threes;


#ifdef TEST
  if ((*pentagons>5) || (*pentagons<0)) 
    { fprintf(stderr,"pentagon problem... Boundary would have %d pentagons\n",*pentagons); 
      writeboundary(boundary,*length);
      writegraph();
      //write_planar_code_debug();
      exit(44);}
  if (*length<9) { fprintf(stderr,"length problem...\n"); 
                   writeboundary(boundary,*length);
		   exit(45);}
#endif

  biggest_start(boundary, *length, startedge, twosinrow);

  return;
}

void splitnode(GLUETREENODE *node,EDGE *first, EDGE *second)
{
  GLUETREENODE *dummy;

  dummy=malloc(sizeof(GLUETREENODE));
  if (dummy==NULL) { fprintf(stderr,"Can't allocate space for gluetreenode -- exiting\n");
                     exit(1); }
  dummy->startedge=first;
  dummy->firstpart=dummy->secondpart=NULL;
  dummy->parent=node;
  dummy->node_id= node_id_counter;  node_id_counter++; 
  node->firstpart=dummy;

  dummy=malloc(sizeof(GLUETREENODE));
  if (dummy==NULL) { fprintf(stderr,"Can't allocate space for gluetreenode -- exiting\n");
                     exit(1); }
  dummy->startedge=second;
  dummy->firstpart=dummy->secondpart=NULL;
  dummy->parent=node;
  dummy->node_id= node_id_counter;  node_id_counter++; 
  node->secondpart=dummy;

  if (node==lastnode) lastnode=node->secondpart;

  return;
}

void unite(GLUETREENODE *node)
{
  if ((node->secondpart)==lastnode) lastnode=node;
  free(node->secondpart); node->secondpart=NULL;
  free(node->firstpart); node->firstpart=NULL;

  return;
}

void search_next_node(GLUETREENODE *node,GLUETREENODE **newnode)
     // searches the next boundary to be filled
{
  GLUETREENODE *run, *prev;

#ifdef TEST
  if (node->parent==NULL) { fprintf(stderr,"Expected parent -- problem\n"); exit(1); }
#endif

  prev=node; run=node->parent;

  //while this is a second child go back

  while (prev==run->secondpart)
    {
      prev=run; run=prev->parent;
#ifdef TEST
  if (run==NULL) { fprintf(stderr,"Expected parent -- problem\n"); exit(1); }
  // this should only happen when started on "lastnode"
#endif
    }

  //So it was a first child -- now go to the second child

  run=run->secondpart;

  // Now look for the first in this part

  while (run->firstpart!=NULL) run=run->firstpart;

  *newnode=run;

  return;
}

int neighbours_pentagon(EDGE *start)
     /* tests whether there is an edge with the same face on the right as *start that
	has pentagonleft=1 -- so a pentagon on the other side. It returns a 1 if this
	is the case and a 0 if this is not the case. This function may only be called
	for pentagons on the right -- this fact is used in the function! */
{
  int i;

  if (start->pentagonleft) return 1;

  for (i=0; i<5; i++) 
    { start=start->inverse->prev;
      if (start->pentagonleft) return 1;
    }
  return 0;
}

void fill(BOUNDARYTYPE boundary[], int length, EDGE *start, int pentagons, int twosinrow,
	  GLUETREENODE *node)
/* it is assumed that boundary starts with at least two 0s and ends with a 1 
   (unless it's all zero) and that *start points to the first
   item in the sequence with the interior on the right.

   Otherboundaries is an array of edges that have places on the right that still must be filled.
   If start == NULL this is a sign that the next other boundary must now be filled (the previous
   one has just been filled). In this case the boundary has first to be computed and also the 
   number of pentagons, the starting edge and twosinrow. These otherboundaries are boundaries that occur 
   when faces are glued in that go across the boundary and produce two parts.

   Furthermore the beginning must be a "biggest start".
*/
{
  int i, newlength, newtwos, gluelistlengte;
  EDGE *newstart, *gluelist[MAXN];
  BOUNDARYTYPE newboundary[BOUNDARYFIELDS];
  int newfilling; /* in case of a new filling that can't be completed the routine can backtrack
		     until the point where the starting edge of this boundary was added */
  GLUETREENODE *newnode;
  long long int old_number_finished;

#ifdef TEST
  static long long int itcounter=0;
 long long int localitcounter;

  itcounter++;
  localitcounter=itcounter;
#endif

  if (backtrack_to != NULL) return;
  if (testing && (number_of_patches != 0LL)) return;

  old_number_finished=number_finished;

  if (start==NULL) 
    { 
      determine_boundary(boundary, &length, &start, &pentagons, 
			 node->startedge,&twosinrow);
    /* boundary is the local variable from the previous depth only made for
       the following (so this) depth -- so nothing is destroyed there */
      newfilling=1;
    }
  else newfilling=0;

  if (ipr && (length<minboundary[pentagons])) return;

#ifdef TEST
  if (start==NULL) 
{ 
  writegraph();
  //write_planar_code_debug();
fprintf(stderr,"Start shouldn't be NULL here...\n"); exit(123); }
#endif  

  if (boundary[0]== 0ULL)
    { if (ipr && (length==5) && (neighbours_pentagon(start)==1)) return;
      if ((length==5) || (length==6)) 
      { if (node==lastnode) ready(); 
        else { search_next_node(node,&newnode);
               fill(newboundary, 0 , NULL, 0, 0, newnode);
             }
      }
    return;
    }


#ifdef TEST

  checkleadingtwos(boundary,length);

  checkgraph(boundary, length, start);


if (!(FIRSTBIT(boundary)) && boundary) 
  { fprintf(stderr,"No 3 at end of sequence...\n");
    writeboundary(boundary,length); 
    exit(17); }

  for (i=1;i<=twosinrow;i++)
    if (IS_SET_ULL(boundary,length-i))
    { fprintf(stderr,"Bit %d should be zero...\n",i); 
    writeboundary(boundary,length); 
    exit(15); }

if (((length)<=maxknownboundary) && ((length)>=7) && (SKIP(length)+WHICHBYTE(*(boundary))>=SKIP(maxknownboundary+1)))
{fprintf(stderr,"trying to access too large bitfield! length=%d\n",length); 
 fprintf(stderr,"%d\n",SKIP(maxknownboundary+1));
 fprintf(stderr,"trying to access byte %d allocated just %d\n",
	 SKIP(length)+(WHICHBYTE(*(boundary))),SKIP(maxknownboundary+1));
exit(345); }

//if (newfilling && ((length<8) || IMPOSSIBLE(length,boundary) ||(twosinrow>=5)))
//  { fprintf(stderr,"This shouldn't happen for a new filling!\n"); 
//  writeboundary(boundary,length);
//  fprintf(stderr,"The node is %lld, localitcounter %lld\n",node->node_id,localitcounter);
// exit(1); }

// This can happen if the boundary was built from a very large one with 0 or 1 pentagons
// so that it was just checked
// that the parts are OK



#endif

  if ((length<8) || IMPOSSIBLE(length,boundary)) return;

  if (twosinrow>=5) return;


    /* these problems are detected BEFORE adding a face from boundary to boundary, so can't
       occur in the case of a new filling */

  if (twosinrow==4)
    { // here you MUST add a hexagon
      addface(start,boundary,length,4,6,&newstart,newboundary,&newlength,&newtwos);
      if (newstart!=NULL) fill(newboundary,newlength,newstart,pentagons,newtwos, node);
      removeface(start->next,0);
      if (newfilling && (old_number_finished==number_finished)) backtrack_to=node->parent;
      //this boundary can't be filled
      return;
    }

  if (twosinrow==3) // can't be connected to other side
    { 
      if (pentagons)
	{ 
	  if (ipr)
	    {
	      addface5(start,boundary,length,3,&newstart,newboundary,&newlength,&newtwos);
	      if ((newstart!=NULL) &&
		  ((pentagons>1) || check_hexagonboundary(newboundary,newlength)))
		fill(newboundary,newlength,newstart,pentagons-1,newtwos,node);
	      removeface_pent_ipr(start->next,0);
	    }
	  else
	    {
	      addface(start,boundary,length,3,5,&newstart,newboundary,&newlength,&newtwos);
	      if ((newstart!=NULL) &&
		  ((pentagons>1) || check_hexagonboundary(newboundary,newlength)))
		fill(newboundary,newlength,newstart,pentagons-1,newtwos,node);
	      removeface(start->next,0);
	    }
	}

      addface(start,boundary,length,3,6,&newstart,newboundary,&newlength,&newtwos);
      if (newstart!=NULL) fill(newboundary,newlength,newstart,pentagons,newtwos, node);
      removeface(start->next,1);
      if (newfilling && (old_number_finished==number_finished)) backtrack_to=node->parent;
      //this boundary can't be filled
      return;
    }

  if (twosinrow==2) // can be connected to other side...
{ 
      if (pentagons) // that can't go to the other side!
	{
	  if (ipr)
	    {
	      addface5(start,boundary,length,2,&newstart,newboundary,&newlength,&newtwos);
	      if ((newstart!=NULL) &&
		  ((pentagons>1) || check_hexagonboundary(newboundary,newlength)))
		fill(newboundary,newlength,newstart,pentagons-1,newtwos,node);
	      removeface_pent_ipr(start->next,1);
	    }
	  else
	    {
	      addface(start,boundary,length,2,5,&newstart,newboundary,&newlength,&newtwos);
	      if ((newstart!=NULL) &&
		  ((pentagons>1) || check_hexagonboundary(newboundary,newlength)))
		fill(newboundary,newlength,newstart,pentagons-1,newtwos,node);
	      removeface(start->next,1);
	    }
	}

      addface(start,boundary,length,2,6,&newstart,newboundary,&newlength,&newtwos);
      if (newstart!=NULL) fill(newboundary,newlength,newstart,pentagons,newtwos,node); 
      removeface(start->next,2);

      if (check_glueable(start,boundary,length,pentagons,gluelist,&gluelistlengte))
	{ 
	  for (i=0; i<gluelistlengte; i++)
	    { glue(start,gluelist[i]);
	      splitnode(node,start->prev->inverse,gluelist[i]->prev->inverse);
	      fill(newboundary, 0 , NULL, 0, 0, node->firstpart);
	      if (backtrack_to == node) backtrack_to = NULL;
	      // this is the point where the unfillable boundary was added
	      unglue(start,gluelist[i]);
	      unite(node);
	    }
	}
      if (newfilling && (old_number_finished==number_finished)) backtrack_to=node->parent;
      //this boundary can't be filled
      return;
    }

#ifdef TEST

  fprintf(stderr,"I should not reach this point! twosinrow: %d \n",twosinrow); 
  fprintf(stderr,"start: %d->%d\n",start->start,start->end);
  writegraph();
  writeboundary(boundary,length);
  //write_planar_code_debug();
  exit(16);
#endif

}

MAINFUNCTION 
//main()

{ EDGE *start;
  int numberoftwos, i;
  char filename[100];
  FILE *fil=NULL; 
  GLUETREENODE glueroot;
  char *path=NULL; // a path where to look for datafiles
  static int firststart=1;

  number_of_patches=number_finished=0LL;
  outputgraphs=testing=0;

  //for (i=0;i<argc;i++) fprintf(stderr,"%s ",argv[i]); fprintf(stderr,"\n");

  if (sizeof(unsigned long long int)!=8)
    { fprintf(stderr,"This program is designed to run on machines with \n");
      fprintf(stderr,"sizeof(unsigned long long int)=8. Sorry!\n");
      exit(1);
    }

  if ((argc<2) || ((argv[1][0]!='2') && (argv[1][0]!='3'))) usage(argv[0]);

  if (read_boundary(argv[1], boundary, &boundarylength, &pentagons)==0) return 0;

  for  (i=2; i<argc ; i++) 
    switch(argv[i][0])
      {
      case 'o': { if (strcmp(argv[i],"o")==0) outputgraphs=1; else usage(argv[0]); break; }
      case 't': { if (strcmp(argv[i],"t")==0) testing=1;else usage(argv[0]);  break; }
      case 'p': { path=argv[i]+1; break; }
      case 'H': { if (strcmp(argv[i],"H")==0) add_H=1; else usage(argv[0]); break; }
      case 'i': { if (strcmp(argv[i],"ipr")==0) ipr_option=1; else usage(argv[0]); break; }
      default: usage(argv[0]);
      }



  if (binfield==NULL && firststart)
    { fil=NULL;
    firststart=0;
    for (maxknownboundary=33; (fil==NULL) && (maxknownboundary>8); )
      { maxknownboundary--;
      if (path!=NULL) 
	sprintf(filename,"%s/bindata_boundaries_length_%d_%d",path,MINKNOWNBOUNDARY,maxknownboundary);
	  else sprintf(filename,"bindata_boundaries_length_%d_%d",MINKNOWNBOUNDARY,maxknownboundary);
	fil=fopen(filename,"r");
      }
      if (fil==NULL) 
	{ fprintf(stderr,"Can't find file with precomputed existence data -- will run without this information.\n");
	maxknownboundary=0;
	binfield=NULL;
	}
      else
	{
	  binfield=malloc(SKIP(maxknownboundary+1)*sizeof(unsigned char));
	  if (binfield==NULL) { fprintf(stderr,"Can't allocate room for existence data! Exiting!\n"); exit(12); } 
 
	  if(fread(binfield,sizeof(unsigned char),SKIP(maxknownboundary+1),fil)!=SKIP(maxknownboundary+1))
	    { fprintf(stderr,"File with precomputed existence data found, but can't read enough items!\n"); 
	    fprintf(stderr,"This is a hint that something is wrong, so I'd better exit!\n");
	    exit(33); }
	  fprintf(stderr,"Using precomputed existence data up to boundary length %d\n",maxknownboundary);
	  data_read=1;
	  fclose(fil);
	}



    }


  if (check_boundary(boundary, boundarylength))
    {

  canonize(boundary, boundarylength, &auts, &mirror, &numberoftwos);

#ifdef NORMAL_MAIN
  fprintf(stderr,"The canonical boundary:\n");
  writeboundary(boundary,boundarylength);
  //fprintf(stderr,"auts=%d, mirror=%d, pentagons=%d, lengte %d\n",auts,mirror,pentagons,boundarylength);
  fprintf(stderr,"pentagons=%d, length %d\n",pentagons,boundarylength);
#endif
  //check_boundary(boundary, boundarylength);

  if (pentagons<2) ipr=0; // no flag needed -- will always be ipr
  else ipr=ipr_option;

  if (ipr) minlist=minlist_ipr; else minlist=minlist_nonipr;

  if ((pentagons==0) && check_hexagonboundary(boundary,boundarylength)==0)
    {
#ifdef NORMAL_MAIN
fprintf(stderr,"Boundary not fillable -- no pentagons and not closed in hexagonal lattice.\n");
#endif
      return 0; }

  initedges();

  initboundary(boundary,boundarylength,&start);
  
  glueroot.startedge=referenceedge=start;
  glueroot.firstpart=glueroot.secondpart=NULL;
  glueroot.parent=NULL;
  glueroot.node_id=node_id_counter; node_id_counter++;
  lastnode=&glueroot;
  backtrack_to=NULL;



  if (boundary[0]!=0ULL) fill(boundary,boundarylength,NULL,pentagons,numberoftwos,&glueroot);
  else ready();

#ifdef NORMAL_MAIN
 
  if (testing) { if (number_of_patches) fprintf(stderr,"fillable\n"); 
                 else fprintf(stderr,"not fillable\n"); }
  else fprintf(stderr,"Number of patches: %lld\n",number_of_patches);
#endif
    }// end of if (check_boundary)

  if (number_of_patches > (long long int) INT_MAX) 
    { fprintf(stderr,"Number of structures too large for int: %lld \n",number_of_patches);
    return -1; }

  //i=number_of_patches; fwrite(&i,4,1,stdout);

  //fprintf(stderr,"returning %d\n",(int)number_of_patches);
  return (int)number_of_patches;
}

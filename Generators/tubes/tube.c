#include "caps_routines.c"
#include "iso.c"
#define NEXT_ENTRY(i) (((i+1)>number_of_rows) ? 1:(i+1))
#define ENTRY_SUM(i,k) ((x=((i+k)%number_of_rows)) ? x : (i+k))
#define XMOD2(x) ((x%2)?((x+1)/2) : (x/2))
int counter=0;
int outputtube_length=0;
EDGE *outputtube_edge;
/* The following constants are used only for debugging and testing; 
   not valid for a final version */
#define OUTPUT 1
void output_map();
int output_glue(),add_tail();
int output_unglue(), last_patch();
int header=0;
int no_output = 0;
int sign=0;


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
  if(!no_output){ 
    if (outputtube_length){
      output_glue(mark);
      write_planar_code_label_fix(fil,nv);
      //write_planar_code_label(fil,edge);
      output_unglue(mark);
    }
    else output_map();
  }
  counter++;
  return(counter);
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


/************************** output_unglue **********************************/
int output_unglue (EDGE *mark)

/* A routine to perform the final ungluing before we return
   from the isomorphism routine.*/

{
  EDGE *edge1, *edge2; 
  int i;

  if (m==0) return output_unglue_zigzag(mark);

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

  if (m==0) return output_glue_zigzag(mark);

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
  // unsigned char zeichen;
  // unsigned char null=0;
  unsigned short zeichen;
  unsigned short null=0;
  FILE *file;
  
  if (OUTPUT) file = stdout;
  else file = stderr;

  if (!header){fprintf(file,">>planar_code le<<");header=1;}
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

  mark = start = *pmark;
  if (hex_number){    /* The "normal" case, if the number of hexagons is 
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

  new_mark = mark;
  number_of_faces = sequence[0];
  if (!(number_of_faces))return(0);
  if (length == 1){
    if ( (!construct_ipr||is_ipr) &&(m_bound == m) && (number_of_faces==l)&&lm_patch_is_canonical(new_mark)){
      output_patch(mark);  return(1);}
    else return(0); 
  }
  new_m_bound = m_bound+1;
  if (new_m_bound > m) return(0);
  if (length == 2){
    //  finish_patch(mark, sequence, m_bound  );
    //  return(0);
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
	if ((nv+1) <= maxnv_cap) {
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
  EDGE *edge=NULL;
  int number_of_rows=0;
  int number_of_faces=0,max_number;
  int new_sequence[7];
  EDGE *new_starts[7];
  EDGE *new_starts_mirror[7];
  EDGE *new_mark;
  EDGE *startedge;
  int new_length;
  int new_numstarts;
  int new_numstarts_mirror;
  int boundary;

  
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
	if ((2*number_of_faces + nv)<=maxnv_cap) {/* Number of vertices after 
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
	    number_of_faces--;
	    edge = add_n_gon(6,edge);
	    number_of_faces++;
	    if (number_of_faces == 1)new_mark = edge->prev->invers->prev;
	    /* The only case where the _first_ face is deleted and so a new mark 
	       has to be set. */
	    new_length = length;
	    canonical=compute_sequence(new_mark, new_sequence, new_starts,&new_length);
	    if(canonical) /* condition is_ipr=1 always fulfilled*/
	      construct_patch(new_mark, new_sequence, new_starts, new_length);
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
	  if ((sequence[0]>1) && (number_of_faces = sequence[NEXT_ENTRY(1)])>1){
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
      number_of_faces = sequence[2];
      edge = operation(0,number_of_faces+1,1, starts[1], &new_mark);
      /* Adding of a complete layer of faces; hexagon at the end would 
	 never be canonical;*/
      new_length = 1;
      canonical=compute_sequence(new_mark, new_sequence, new_starts,&new_length);	
      if (canonical &&(!construct_ipr||is_ipr))
	construct_patch(new_mark, new_sequence, new_starts, new_length);
      edge = reduce(edge, number_of_faces+2);is_ipr=1;
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
      if(nv+1<=maxnv_cap) {
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
    if (sequence[0]>0){
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
	      if (((nv+number_of_faces*2)<=maxnv_cap)){
		edge = operation(0,number_of_faces-1,1,startedge,&new_mark);
		new_length = length;
		new_sequence[0] =  sequence[NEXT_ENTRY(i)]+number_of_faces;
		new_sequence[new_length-1] = j;
		if (new_length == 1)new_sequence[0] = number_of_faces + j;
		for(k=1; k<new_length-1; k++) new_sequence[k] = sequence[ENTRY_SUM((i),(k+1))];
		if (!construct_ipr||is_ipr)
		  construct_notpc_patch(new_mark, new_sequence, 1, new_length);
		if ((nv+1) <=maxnv_cap){
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
		  if (!sequence[NEXT_ENTRY(i)]) { /* Additional face can be added */
		    if (nv+1 <=maxnv_cap){ /* Additional pentagon */
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
		      if(nv+1 <= maxnv_cap){/* Additional hexagon */
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
    if (length == 2){ /* BEDINGUNGEN UEBERPRUEFEN!!*/
      /* A complete row of hexagons + additional pentagon*/
      number_of_rows = sequence[0]; /* Necessary for the macro NEXT_ENTRY(i) */
      for (i=1;i<=2;i++){
	number_of_faces = sequence[i];
	if ((l>=(sequence[i]+1))&&(m>=sequence[NEXT_ENTRY(i)])){/* Optimisation*/
	  if ((2*number_of_faces+1+nv)<=maxnv_cap){
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
	  }
	  edge = reduce(edge, number_of_faces+1);is_ipr=1;
	}
      }
    }
    if ((length == 1) && (!construct_ipr||is_ipr)){
      last_patch(mark, sequence[1]);
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
int finish_patch(EDGE *mark, int sequence[], int m_bound  )
{
  EDGE *edge, *start, *end, *new_mark;
  int i;
  int number_of_faces, number_of_rows;
  
  number_of_faces = sequence[0];
  number_of_rows = m - m_bound;
  if (l == (number_of_faces + sequence[1]) && (number_of_faces >= number_of_rows)){
    start = new_mark = mark;  
    for (i=0; i<number_of_rows-1; i++){
    edge = add_n_gon(6, start);
      start = edge->prev->invers->prev;
      edge = operation_notpc(number_of_faces-1, 0, &new_mark);
      number_of_faces--;
    }
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
  return(0);
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
    while((additional_faces <= max_add)&&( (pentagon && nv+additional_faces*4-1<=maxnv_cap)||(!pentagon && nv+additional_faces*4<=maxnv_cap))){
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
      if (!construct_ipr||is_ipr){sign=1;
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
int last_patch(EDGE *mark, int x)
{
  int i,number_of_rows;
  EDGE *edge;
  EDGE *new_mark;
  EDGE *end_edge;


  number_of_rows = m + l - x;
  /* ABFRAGE NACH DER ECKENZAHL NOCH EINBAUEN? */
  if  ((l<=x) && (number_of_rows  > 0) && (number_of_rows <= l)){
    edge = new_mark = mark;
    for(i=1;i<number_of_rows;i++){ 
      edge = operation_notpc(l-i+1,0,&new_mark);
    }
    edge = operation_notpc(l-number_of_rows,1,&new_mark); 
    if ((!construct_ipr||is_ipr)&&lm_patch_is_canonical(new_mark)) output_patch(new_mark);
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
  int length = 0;

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
  
  for(i=5; i<= (l+m); i++) limit += (2*i+1);
  return (limit-1);
}
/********************* void *********************************/
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
  outputtube_edge =  make_tube(maxnv_cap+1,outputtube_length-1); /**Anfangsknoten korrekt?*/
  isomorphism_tube = make_tube(maxnv_cap+1, l+m);
}
/********************* main *********************************/
int main(int argc , char *argv[])
{
  EDGE *merke=NULL;
  int i, tubelength;
  int sequence[7];
  EDGE *starts[7];
  
  fprintf(stderr,"Command:\n");
  for (i=0; i< argc; i++) fprintf(stderr,"%s ",argv[i]); fprintf(stderr,"\n");
  

  if (argc<3) usage(argv[0]);

  if (!isdigit(argv[1][0])) usage(argv[0]); l=atoi(argv[1]);
  if (!isdigit(argv[2][0])) usage(argv[0]); m=atoi(argv[2]);
  if ((l<0) || (m<0)) usage(argv[0]);
  if (l<m) { i=l; l=m; m=i; }

  if (((l+m)<5) || (((l+m)==5) && (m!=0)))
    { fprintf(stderr,"No caps for parameters %d %d possible\n",l,m);
      exit(0);
    }

    for (i=3; i< argc; i++){
      switch( argv[i][0])
	{
	case 'n': {
	  if (!(strcmp(argv[i], "noout"))){no_output = 1;}
	  else { fprintf(stderr,"Nonidentified option: %s \n",argv[i]); exit(0); }
	  break;
	}
	case 'i': {
	  if (!(strcmp(argv[i], "ipr"))) {construct_ipr=1;}
	  else {fprintf(stderr,"Nonidentified option: %s \n",argv[i]); exit(0);}
	  break;
	}
	case 't': {
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

    if (outputtube_length>l+m) tubelength=outputtube_length;
      else tubelength=l+m;

  maxnv_cap = limit_vertices();  /* maximum number of vertices in a cap */
  maxnv = maxlabel = (2*(l+m)*(tubelength) + maxnv_cap);  
                            /* 2*(l+m) boundary length; */

  init(); /* Needs the correct value of maxnv_cap */
  cap(merke, sequence, starts);
fprintf(stderr,"graphs: %d\n",counter);

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

 return 0;
}




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



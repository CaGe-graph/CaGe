/* Test plugin for PRE_FILTER_POLY */
/* cc -o plantri_preg -O4 -DPLUGIN='"regfilt.c"' plantri.c  */

#define PRE_FILTER_BIPPOLY PRE_FILTER_POLY
#define PRE_FILTER_POLY  pre_filter_polyreg(oldfeas,noldfeas)

#define PLUGIN_INIT test_whether_5reg()

void  test_whether_5reg()
{
  if ( (pswitch==FALSE) || ((maxnv*minimumdeg)!=(2*edgebound[1])))
  {fprintf(stderr,">E Only for regular polyhedra -- check degree and vertex numbers!\n");
    exit(1);}
/* when PLUGIN_INIT is applied the meaning of edgebound[1] is still the maximum number of undirected edges --
   this changes later to  the maximum number of directed edges. */

  if (minimumdeg!=5) 
  { fprintf(stderr,">E Should only be used for regular polyhedra with degree 5.\n"); 
    fprintf(stderr,">E For smaller degrees specialized options exist -- see manual.\n");
    exit(1); }
}


int pre_filter_polyreg(EDGE *feas[], int nfeas)
/* feas[]: list of (undirected) edges that can in principle still be removed
   Return 0 if there are degrees that can't be reduced to minpolydeg */
{
  int i;
  int verw[MAXN]; //verw[i]: how many edges in feas[] can be removed at vertex i

  RESETMARKS_V;
  for (i=0;i<nv;i++) verw[i]=0;

   for (i=0; i < nfeas; i++)
     { verw[feas[i]->start]++;  verw[feas[i]->end]++;  } 

  for (i=0;i<nv;i++)
    { 
      if (degree[i]-verw[i]>minpolydeg) return 0; 
     // it is impossible to remove enough edges to make this vertex have degree minpolydeg
      if (degree[i]-verw[i]==minpolydeg) { MARK_V(i); } // at this vertex all edges in feas[] MUST be removed
      else { verw[i]=(degree[i]-minpolydeg);  } // this many can really be removed
    }

  for (i=0; i < nfeas; i++)
    { if (ISMARKED_V(feas[i]->start)) { if (--verw[feas[i]->end] <0) return 0; }
      // return 0 if more edges must be removed than can be removed
      else   { if (ISMARKED_V(feas[i]->end)) if (--verw[feas[i]->start]<0) return 0; } 
      // why else?: if feas[i]->start is marked too, the value verw[feas[i]->start] will never be less than 0 anyway
    }
  
  return 1;

}
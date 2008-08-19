/* Sample of PLUGIN file to use with plantri.c 

   To use this, compile plantri.c using 
       cc -o plantri_min4 -O '-DPLUGIN="min4.c"' plantri.c

   This plug-in deletes those triangulations whose minimum
   degree is 3.
*/

unsigned long long int mask[MAXN];

/*********************************************************************
The following are the two compulsory plug-in procedures. FILTER is used
on completed triangulations.  SUMMARY is called at the end in case we
want to write a summary. */

static int
FILTER(int nbtot, int nbop, int doflip)
{
	register int i;

	for (i = nv; --i >= 0;)
	   if (degree[i] == 3) return FALSE;
	
	return TRUE;
}

static void
SUMMARY()
{
}

/*********************************************************************
The following replaces the code that chooses legal extensions.  Basic
idea: if mindeg >= 4, then we need consider at most 2 vertices of
degree 3 (except for nv=4), and if there are 2 they have a common
neighbour.  For nv=maxnv-1, no degree 3 extensions are allowed. */

#undef FIND_EXTENSIONS
#define FIND_EXTENSIONS find_extensions_m4

find_extensions_m4(int numb_total, int numb_pres,
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
	    if (nv == maxnv-1)
	       *numext3 = 0;
	    else
	    {
                k = 0;
                for (i = 0; i < nv; ++i)
                {
                    e = ex = firstedge[i];
                    do
                    {
                        e1 = e->invers->prev;
                        if (e1 > e)
                        {
                            e1 = e1->invers->prev;
                            if (e1 > e) 
			    {
				if (deg3 < 2 || degree[i] == 3 ||
					degree[e->end] == 3 ||
					degree[e1->start] == 3)
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
                            if ((degree[e->end] == 3) 
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
                        
            if (deg3 == 0 && deg4 <= 2)
            {
                k = 0;
                for (i = 0; i < nv; ++i)
                {
                    if (degree[i] < 6) continue;
                    e = ex = firstedge[i];
                    do
                    {
                        e1 = e->next->next->next;
                        if ((degree[e->end] == 4)
                                 + (degree[e1->end] == 4) == deg4) 
                            ext5[k++] = e;
                        e = e->next; 
                    } 
                    while (e != ex);
                }
                *numext5 = k;
            }
            else
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

	    if (nv == maxnv-1)
		*numext3 = 0;
	    else
	    {
                k = 0;
                for (i = 0; i < ne; ++i)
                {
                    e = nb0[i];
                    if (e->mark) continue;
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

            if (deg3 == 0 && deg4 <= 2)
            {
                for (i = 0; i < ne; ++i)
                    nb0[i]->mark = 0;

                k = 0;
                for (i = 0; i < ne; ++i)
                {
                    e = nb0[i];
                    if (e->mark || degree[e->start] < 6) continue;
                    
                    if ((degree[e->end] == 4) 
                                + (degree[e->next->next->next->end] == 4) != deg4)
                        continue;
                    ext5[k++] = e;
 
                    for (nb1 = nb0 + i; nb1 < nbop; nb1 += MAXE) 
                        (*nb1)->mark = 1;
 
                    for (; nb1 < nblim; nb1 += MAXE) 
                        (*nb1)->prev->prev->prev->mark = 1; 
                } 
                *numext5 = k;
            }
            else
                *numext5 = 0;
        }
}


#define PRE_FILTER mindeg4_prune()

static int
mindeg4_prune()
{
	register int i,deg3;
	int a,b,v1,v2,v3,w;
	int ident,j;
	register EDGE *e;
	unsigned long long int vmask;
	static int init=1;

	if (init) {
	  init=0;
	  if (8*sizeof(unsigned long long)<MAXN)
	    { fprintf(stderr,"Constant MAXN too large -- at most %d possible.\n",8*sizeof(unsigned long long));
	    exit(1); }
	  mask[0]=1;
	  for(i=1;i<MAXN;i++) mask[i]=mask[i-1]<<1;
	}
	
	if (nv == 4) return TRUE;

	deg3 = 0; 
        for (i = 0; i < nv; ++i)
           if (degree[i] == 3)
	   {  
		if (deg3 == 0) a = i;
		else           b = i;
		++deg3;
	   }

	if (deg3 < 2) return TRUE;
	if (deg3 > 2) return FALSE;

	e = firstedge[a];
	v1 = e->end;
	e = e->next;
	v2 = e->end;
	v3 = e->next->end;


	e = firstedge[b];
	w = e->end;
	if (w == v1 || w == v2 || w == v3) return TRUE;
	e = e->next;

	
	e = firstedge[a];
	vmask=mask[e->end];
	e = e->next;
	vmask|=mask[e->end];
	vmask|=mask[e->next->end];

	
	e = firstedge[b]; 
	if (mask[e->end] & vmask) ident=1; else ident=0;
	e = e->next;
	if (mask[e->end] & vmask) 
	  { if (ident) return TRUE; else ident=1; }
	if (mask[e->next->end] & vmask) 
	  if (ident) return TRUE;


	return FALSE;
}





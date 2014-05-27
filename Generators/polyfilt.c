/* Test plugin for PRE_FILTER_POLY */
/* cc -o plantri_ptest -O4 -DPLUGIN='"polyfily.c"' plantri.c  */

#define PRE_FILTER_BIPPOLY PRE_FILTER_POLY
#define PRE_FILTER_POLY  pre_filter_poly(ne-edgebound[1],oldfeas,noldfeas)

static int
pre_filter_poly(int need, EDGE *feas[], int nfeas)
/* Return 0 if an extra need directed edges are impossible */
{
    int i,v,navail,gd,fdeg[MAXN];

    RESETMARKS_V;
    navail = 0;
    for (i = 0; i < nfeas; ++i)
    {
	v = feas[i]->start;
	if (ISMARKED_V(v))
	{
	    if (--fdeg[v] > 0) ++navail;
        }
	else
	{
	    fdeg[v] = degree[v] - minpolydeg;
	    MARK_V(v);
	    ++navail;
	}
	v = feas[i]->end;
	if (ISMARKED_V(v))
	{
	    if (--fdeg[v] > 0) ++navail;
        }
	else
	{
	    fdeg[v] = degree[v] - minpolydeg;
	    MARK_V(v);
	    ++navail;
	}
    }

    return navail >= need;
}

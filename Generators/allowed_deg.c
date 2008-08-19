/* PLUGIN file to use with plantri.c 

   To use this, compile plantri.c using 
   cc -o plantri_ad -O '-DPLUGIN="allowed_deg.c"' plantri.c

   This plug-in deletes those triangulations 
   with vertex degrees which are not allowed. 
   Allowed degrees may be defined by using the -F switch, for example

   plantri_ad -F7F5 14

   makes all triangulations with 14 vertices and degrees 5 or 7.

   authors: Gunnar Brinkmann and Ulrike von Nathusius,
   contains parts of maxdeg.c by Brendan McKay.

   The upper and lower limits for the number of faces to be used
   can be given like e.g. plantri_ad -F7_1^3F5F6 14 forcing between
   one and 3 vertices with valency 7. At the moment this is only 
   implemented as a filter at the end. It could be used for better 
   bounding criteria -- should be implemented once...

   The nonstandard (but common) long long type is required.
   (Warning: some versions of the Sun "cc" compiler give incorrect
   results with programs using long long.)
*/

#include<ctype.h>

#define gauss_oben(x,y) (((x)+(y)-1)/(y))
#define FILTER ad_filter
#define PRE_FILTER_SIMPLE allowed_deg_prune()

static int maxdeg = 0;
static unsigned long long int LISTE=0;
static unsigned long long int mask[MAXN];
static int error_up[MAXN];
static int error_down[MAXN];
static int error_of_degree[MAXN];
static int maxnumber[MAXN],minnumber[MAXN],facelist[MAXN];
static int grenzen=0;

int nur_noch_E3();

/********************************* Filter **************************/

static int 
ad_filter(int nbtot, int nbop, int doflip) 
{
  int i, run, anzahl[MAXN];

if (grenzen)
  { 
    for (run=0; facelist[run]; run++) anzahl[facelist[run]]=0;
    for(i=0;i<nv ; i++) { if ( !( mask[degree[i]] & LISTE)) return(0);
			  (anzahl[degree[i]])++; }
    for (run=0; facelist[run]; run++) 
      if ((anzahl[facelist[run]]<minnumber[facelist[run]]) || 
	  (anzahl[facelist[run]]>maxnumber[facelist[run]])) return(0);
  }
else
  for(i=0;i<nv ; i++) if ( !( mask[degree[i]] & LISTE)) return(0);

return(1);

}

/************************** Switches ********************************/

/* The following adds the switch f to those normally present.
   arg is the address of the command-line argument, and j is the
   index where 'F' might be.  The value of j must be left on the
   last digit of the switch value. */

#undef SWITCHES
#define SWITCHES "[-F#[_#^#] -uagsh -odG -v]"

#define PLUGIN_SWITCHES else if (arg[j]=='F') list_of_allowed_degrees(arg, &j); 

#define PLUGIN_INIT if (minconnec >= 0 || polygonsize >= 0 \
   || minimumdeg >= 0 || pswitch || xswitch || tswitch) \
  {fprintf(stderr,">E Usage: %s %s n [res/mod] [outfile]\n",cmdname,SWITCHES);\
   exit(1);}

/************************* Prefilter ********************************/
/*
  The following is used to prune the search tree at levels below maxnv.
  Consider the expansion operations E3, E4, E5.  The basic ideas are:
  (1) Only E5 can reduce the degree of a vertex, then only by 1.
  (2) If there are 3 or more vertices of degree 3, E5 and E4 will never be 
  used, unless G is the K4.
  (3) Only E4 can reduce the number of vertices of degree 3.
  (4) The quantity (2 * #degree-3 + #degree-4) is reduced by at most one
  by E3 and E4.  E5 can reduce it by 2 as long as it becomes 0. */


static int
allowed_deg_prune() 
{
  register int i,levs,excess,d3,d4;
  unsigned long long int vmask=0;
  int a,j,x,eintraege,error,nur_E3;
  int eckenliste[MAXN];
  register EDGE *e, *start;

	levs = maxnv - nv;          /* remaining number of steps */
	excess = d3 = d4 = 0;
	for (i = 0; i < nv; ++i)
	{
	  if      (degree[i] == 3) ++d3;
	  else if (degree[i] == 4) ++d4;
	  else if (degree[i] > maxdeg) excess += degree[i] - maxdeg;
	}
	nur_E3 = ((d3>2)|| ((d3==2) && nur_noch_E3()));
	if (error_of_degree[3] && (nv>4) && nur_E3) return FALSE;
	/* Corollary of (2): if there are at least 3 vertices of 
	   degree 3, they can never disappear (unless G=K4).*/

	if (excess) /* By (1): */
	  {
	    if (nur_E3 || excess > levs) return FALSE;
	    if (d3 > 0 && excess >= levs) return FALSE;
	    
	    i = d3 + d3 + d4; /* By (4): */ 
	    if (i > 0 && excess > levs - i + 2) return FALSE;
	  }
	
	if (!nur_E3)
	  { 
	    /* In this case it is not possible to exclude any of the three
	       operations. */
	    for (i=error=0;i<nv;i++){
	      x=degree[i];
	      error += error_of_degree[x];
	    }
	    
	    if (error_up[3]&&error_up[5])
	      {
		/* The proof and explanation for this part follows 
		   at the end of the program. */
		if (gauss_oben(error, 2)>levs) return FALSE;
	      }
	    else if (gauss_oben(error, 3)>levs) return FALSE;
	  } /* end: all operations possible */

	else {
	  /* Only Operation E3 can be used, so that "error_of_degree" may be 
	     substituted by "error_up" . In the case "K4" later on other 
	     operations may also be used, but the error of degree 3 is equal 
	     to "error_up" in any case. */
  
	  for(i=error=0;i<nv;i++){  
	    error+= error_up[degree[i]];
	    if (gauss_oben(error, 3)>levs) return FALSE;
	  }
	  /* Ranking of the vertices with respect to the error of 
	     their degrees */
	  
	  for (i=eintraege=error=0; i<nv; i++){	
	    error=error_up[degree[i]];
	    if (error){
	      for (j=eintraege-1;(j>=0) && 
		     (error_up[degree[eckenliste[j]]] < error); j--)
		eckenliste[j+1]=eckenliste[j];
	      eckenliste[j+1]=i; 
	      eintraege++;
	    }
	  }

	  /* Choose a set of vertices which are pairwise independent in 
	     the sense that one operation E3 can only change the error 
	     of one vertex of the set. So the number of remaining steps 
	     cannot be less than the sum of errors of this vertex-set. 
	     In order to get an effective condition one starts the 
	     computation of neighbours with vertices which have a large 
	     error.
	   */
		  
	  for(i=error=vmask=0;i<eintraege; i++) 
	    {
	      a = eckenliste[i];          /* computation of the neighbours */
	      if (!(vmask & mask[a]))
		{
		  start = e = firstedge[a];
		  vmask|=mask[e->end]; /* vmask: set of forbidden vertices */
		  while((e=e->next) != start){
		    vmask|=mask[e->end];
		  }
		  error += error_up[degree[a]];
		  /* error: minimum number of necessary steps */ 
		  if (error>levs) return FALSE;
		}
	    }
	}
	return TRUE;
}	

/*******************  nur_noch_E3  *******************************/

int
nur_noch_E3()
{
  register EDGE *e;
  unsigned long long int vmask;
  int a,b,i,ident,deg3;
  
  /* This function may only be used, if there are exactly two vertices of 
     degree 3. Two vertices of degree three that do not have two common 
     neighbours can only disappear (get a higher degree) by inserting 
     at least two new such vertices with the same property, so only E3
     can be used from now on:
     The distance between two such vertices can not decrease with E3, 
     and if E3 is applied to one of them, the new 3-vertex will have
     (at least) the same distances from the remaining old one.

     The function will return TRUE, if they have no common neighbours 
     so that only E3 can be used further, otherwise FALSE. 
   */

  a=b=0;
  for (i = deg3 = 0; i < nv ; ++i)
          if (degree[i] == 3)
	    {  
	      if (deg3 == 0) {a = i;deg3=1;}
	      else           {b = i;i=nv;}
	    }
	e = firstedge[a];
	vmask=mask[e->end]; /* vmask will be the set of neighbours of a */
	vmask|=mask[e->prev->end];
	vmask|=mask[e->next->end];

	
	e = firstedge[b]; /* Test if two of the neighbours of b are in vmask*/
	if (mask[e->end] & vmask) ident=1; else ident=0;
	if (mask[e->prev->end] & vmask) 
	  { if (ident) return FALSE; else ident=1; }
	if (mask[e->next->end] & vmask) 
	  if (ident) return FALSE;

	return TRUE;
}

/************************** list_of_allowed_degrees ***********************/

/* The allowed degrees taken from the F-Switch LISTE are described as bits 
   of the binary representation of an integer.
   The definition of the error_of_degree is explained in the text below. 
   */

void
list_of_allowed_degrees(char arg[], int *pj )
{  
  static int init=1, maxdegree;
  int i,n,j,x,run; 
  if (init) {
    init=0;
    maxdegree=8*sizeof(unsigned long long)-1;
    mask[0]=1;
    for(i=1;(i<=maxdegree) && (i<MAXN);i++) mask[i]=mask[i-1]<<1;
    for(i=0;i<MAXN;i++)
      error_up[i]=error_down[i]=error_of_degree[i]=MAXN-1;
    for(i=0;i<MAXN;i++) facelist[i]=0;
  }
  j=*pj;
  if  (!isdigit(arg[j+1]))
    {fprintf(stderr,"No degrees given! Problem: %s\n",arg+j);exit(0);} 
  else 
    {  /* get the switch values of option -F */
      n = atoi (arg+j+1);
      if (n>=MAXN) {fprintf(stderr,"Maximum n is %d!\n",MAXN);exit(0);}
      if (n>maxdegree) {fprintf(stderr,"Maximum degree is %d!\n",maxdegree);exit(0);}
      LISTE |= mask[n];
      maxnumber[n]=MAXN; minnumber[n]=0;
      for (run=0; facelist[run]; run++); facelist[run]=n;
      if(n>maxdeg) maxdeg = n;
      for (i=3; i<=n; i++){
	if ( error_up[i] > (n-i)) error_up[i]=n-i;
      }
      if(n>=5){
	for(i=n;i<MAXN;i++){
	  if(error_down[i]> (i-n)){ 
	    error_down[i]=i-n;
	  }
	}
      }
      if(error_up[3]&&error_up[5]) x=error_up[5] - 1;
      else x=error_up[5];
      for(i=3;i<MAXN;i++){
	if ((1+x)*error_down[i]<error_up[i]){
	  error_of_degree[i]=(1+x)*error_down[i]; }
	else {
	  error_of_degree[i]=error_up[i];
	}
      }
      j++; 
      while ((arg[j]>='0') && (arg[j]<='9')) j++; 
      j--;
      /* Two times the same to  be independent of the order */
      if (arg[j+1]=='_') { grenzen=1; j+=2;
			 minnumber[n]=atoi(arg+j);
			 while ((arg[j]>='0') && (arg[j]<='9')) j++; j--; }
      if (arg[j+1]=='^') { grenzen=1; j+=2;
			 maxnumber[n]=atoi(arg+j);
			 while ((arg[j]>='0') && (arg[j]<='9')) j++; j--; }
      if (arg[j+1]=='_') { grenzen=1; j+=2;
			 minnumber[n]=atoi(arg+j);
			 while ((arg[j]>='0') && (arg[j]<='9')) j++; j--; }
      if (arg[j+1]=='^') { grenzen=1; j+=2;
			 maxnumber[n]=atoi(arg+j);
			 while ((arg[j]>='0') && (arg[j]<='9')) j++; j--; }
      *pj = j;
    }
} 

/****************** Annotations ************ ********************/

/*
 \LaTeX text with mathematical details to the definition of errors:

\documentclass[fleqn,10pt,a4paper]{article}
\usepackage{epsfig,amssymb}
\usepackage{german}
\usepackage{latexsym}
\parskip1.5ex
\parindent0em
\mathindent1cm
\newtheorem{satz}{Proposition}
\newtheorem{lemma}[satz]{Lemma}
\newtheorem{defin}[satz]{Definition}
\newtheorem{kor}[satz]{Corollary}
\newcommand{\zz}{ {\mathbb{Z}}_2 \! \times \!{\mathbb{Z}}_2}

\begin{document}
If the degree to be used in the graphs constructed by the program are
restricted to $k_1, \ldots k_m$ it is obvious that in an intermediate
level a vertex with a degree different from $ k_1, \ldots k_m$ has a
special error which must be ``repaired'' until the last level $maxnv$
is reached. There are several possibilities to assign an exact value
to each deviation. The aim of a sensible definition of errors should
be to give an efficient condition to prune the search tree. 

We will use that the degrees 3 and 4 can only be increased and that
--- commonly spoken --- it is more ``difficult'' to decrease a degree. 

Understanding the algorithm of plantri is an essential prerequisite for
the following.  
\begin{defin}\em

$nv$: actual number of vertices\\
MAXN: upper bound for $maxnv$\\
$maxnv$: target number of vertices \\
$levs=maxnv-nv$: remaining steps up to $maxnv$\\
$k \in \{0,\ldots \mbox{MAXN} -1\}$ degree, $i\in \{0,\ldots maxnv -1\}$
vertex\\$K:=\{ k_1, \ldots k_m\}$: set of allowed degrees\\
degree$_{levs}(i)$: degree of vertex $i$ at level $maxnv - levs$\\
\begin{eqnarray}
\mbox{error\_up}(k)&:= &\min(\{(k_j -k)|k_j\geq k\} \cup \{\infty\})\\\nonumber
  &(=&\mbox{distance to the next allowed degree above }k).\\
\mbox{error\_down}(k)&:=& \min(\{(k-k_j)|5\leq k_j< k\}\cup \{\infty\})\\\nonumber
  &(=&\mbox{distance to the next allowed degree below }k\mbox{ not less than 5.})\\
x&=&\left \{ \begin{array}{lll}
\mbox{error\_up}(5)-1 & \mbox{if} & 3,5\not\in K,\; 4 \in K\\
\mbox{error\_up}(5)  & \mbox{else} & \end{array}\right.\\
\mbox{error\_of\_degree}(k) & := & \left \{ \begin{array}{ll}
\mbox{error\_up}(k) & \mbox{if}\quad k\leq 5\\
\min (\mbox{error\_up}(k), (1+x)\cdot\mbox{error\_down}(k)) & \mbox{else}\end{array}\right.\\
\mbox{error}_{levs}(i) &:= & \mbox{error\_of\_degree}(\mbox{degree}_{levs}(i))\\
\mbox{sum\_of\_errors}_{levs} &:= & \sum_{i=0}^{nv-1}\mbox{error}_{levs}(i)
\end{eqnarray}
\end{defin}

\begin{satz}
The change of the ``sum\_of\_errors'' at each step is at most 3.
\end{satz}
To prove this assertion we need following lemma:
\begin{lemma}
The following inequalities hold:
\begin{eqnarray}
\mbox{error\_of\_degree}(k+1)&\geq &\mbox{error\_of\_degree}(k)-1\quad\forall k\geq 3\\
\mbox{error\_of\_degree}(k-1)&\geq& \mbox{error\_of\_degree}(k)-(1+x)\\
&&\;\forall k,x \in \mathbb{N}, k\geq 4 \mbox{ and }x\geq 0  \nonumber
\end{eqnarray}
\end{lemma}
{\bf Proof of the lemma:}
The first case of the first inequality: both errors are taken as error\_up.
\begin{eqnarray*}
\mbox{error\_of\_degree}(k+1)&=&\mbox{error\_up}(k+1)\geq \mbox{error\_up}(k)-1\\&=&\mbox{error\_of\_degree}(k)-1
\end{eqnarray*}
The second case (both error\_down):
\begin{eqnarray*}
\mbox{error\_of\_degree}(k+1)&=&(1+x)\cdot\mbox{error\_down}(k+1)\\
\mbox{error\_of\_degree}(k)&=&(1+x)\cdot\mbox{error\_down}(k)\\
\mbox{error\_of\_degree}(k+1)&=&(1+x)\cdot\mbox{error\_down}(k+1)\\
&\geq&(1+x)\cdot\mbox{(error\_down}(k)+1)\\
&=&\mbox{error\_of\_degree}(k)+(1+x)\\
& >& \mbox{error\_of\_degree}(k)-1
\end{eqnarray*}
Third case:
\begin{eqnarray*}
\mbox{error\_of\_degree}(k+1)&=&\mbox{error\_up}(k+1)\\
&\leq &(1+x)\cdot\mbox{error\_down}(k+1)\\
\mbox{error\_of\_degree}(k)&=&(1+x)\cdot\mbox{error\_down}(k)\\
&\leq&\mbox{error\_up(k)}\\
\mbox{error\_of\_degree}(k+1)&=&\mbox{error\_up}(k+1)\\
&\geq &\mbox{error\_up(k)}-1\\
&\geq &(1+x)\cdot(\mbox{error\_down}(k))-1\\
&\geq&\mbox{error\_of\_degree}(k)-1
\end{eqnarray*}
The last case --- error\_down for $k+1$, error\_up for $k$ --- is only
possible, if one of them is 0. The correctness of the inequality in this
case can be easily seen.

The second inequality can be proven analogously.
The two inequalities will be used in the proof of the proposition in the
following form:
\begin{eqnarray}
1&\geq& \mbox{error\_of\_degree}(k)-\mbox{error\_of\_degree}(k+1)\\
1+x&\geq &\mbox{error\_of\_degree}(k)-\mbox{error\_of\_degree}(k-1)
\end{eqnarray}
{\bf Proof of the proposition:}
Difference between the error of step ``$levs$'' and step ``$levs-1$'':
\begin{eqnarray*}
nv &=& maxnv - ({levs}-1)\\
\Delta& :=& \mbox{sum\_of\_errors}_{levs}-\mbox{sum\_of\_errors}_{levs-1}\\
&=&\sum_{i:{\rm degree}_{levs}(i)\not={\rm degree}_{levs-1}(i)}
(\mbox{error}_{levs}(i)-\mbox{error}_{levs-1}(i))-\mbox{error}_{levs-1}(nv)
\end{eqnarray*}
Approximation of the summands if
$j:=\mbox{degree}_{levs}(i)<\mbox{degree}_{{levs}-1}(i)=j+1$: \\
$\mbox{error}_{levs}(i)-\mbox{error}_{levs-1}(i)$=
$\mbox{error\_of\_degree}(j)-\mbox{error\_of\_degree}(j+1)\leq 1$\\
Otherwise we have $\mbox{error}_{levs}(i)-\mbox{error}_{levs-1}(i)\leq 1+x$\\
So 
\[\Delta \leq 
\left\{ \begin{array}{rcll} 
3\cdot1&-&\mbox{error\_of\_degree}(3)\quad & \mbox{if E3}\\
2\cdot1&-&\mbox{error\_of\_degree}(4)\quad & \mbox{if E4}\\
2\cdot1&+&1\cdot(1+x)-\mbox{error\_of\_degree}(5)\quad & \mbox{if E5}
\end{array} 
\right.\] 
\[\leq 
\left\{ \begin{array}{rll}
3& \mbox{if E3}&\\
2& \mbox{if E4}&\\
3& \mbox{if E5 }&
\end{array}
\right.\]\[
\leq 3 
\]
\begin{satz}
In the case that vertices of degree 3 and 5 are not allowed, 
$x$ can be chosen so that the change in the sum\_of\_errors is at most
two for each step.
\end{satz}
{\bf Proof:}
Choose $x=$error\_of\_degree(5)-1. With the same argumentation as above we get
\[\Delta \leq 
\left\{ \begin{array}{rcll} 
3\cdot1&-&\mbox{error\_of\_degree}(3)\quad & \mbox{if E3}\\
2\cdot1&-&\mbox{error\_of\_degree}(4)\quad & \mbox{if E4}\\
2\cdot1&+&1\cdot(1+x)-\mbox{error\_of\_degree}(5)\quad & \mbox{if E5}
\end{array} 
\right.\] 
\[\leq	
\left\{ \begin{array}{rcll} 
3\cdot1&-&1\quad & \mbox{if E3}\\
2\cdot1&-&0\quad & \mbox{if E4}\\
2\cdot1&+&1+\mbox{error\_of\_degree}(5)-1-\mbox{error\_of\_degree}(5)\quad & \mbox{if E5}
\end{array} 
\right.\] 

\begin{kor}
If $\mbox{sum\_of\_errors} > n \cdot \mbox{levs}$ with $n=3$ in the general
case and $n=2$ if degrees 3 and 5 are not allowed, the errors cannot be
balanced up to $maxnv$. 
\end{kor}
{\bf Proof:}
With \\
$\Delta= \mbox{sum\_of\_errors}_{levs}-\mbox{sum\_of\_errors}_{levs-1}\leq n$\\
we get following inequality:
\begin{eqnarray*}
\mbox{sum\_of\_errors}_{levs} & \leq & \mbox{sum\_of\_errors}_{levs-1}+n\\
& \leq & \mbox{sum\_of\_errors}_{levs-2}+n\cdot 2\leq \ldots\\
& \leq & \mbox{sum\_of\_errors(0)}+n \cdot levs\\
&   =  & n \cdot {levs}
\end{eqnarray*}
The result of this corollary is used in the function ``allowed\_deg\_prune''
to prune the search tree, if
$\quad\frac {\mbox{sum\_of\_errors}_{levs}} n >levs$.
\end{document}

*/

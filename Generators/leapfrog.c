/* Liest graphen im planarcode.

*/


#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include<limits.h>
#include <time.h>
#include <sys/times.h>


#define N 10000     /* Maximal moegliche Anzahl der Knoten KN_MAX-2*/
#define N2 30000    /* Maximal moegliche leapfroggroesse */
#define MAXVAL 20  /* maximale valenz */

#define aussen   (N+2) /* so werden kanten nach aussen markiert */

#define infty    LONG_MAX
#define FL_MAX   UCHAR_MAX
#define KN_MAX   USHRT_MAX
#define unbelegt FL_MAX
#define leer     KN_MAX-1
#define f_leer    FL_MAX-1
#define False    0
#define True     1
#define nil      0
#define reg      3

/* Typ-Deklarationen: */

typedef  char BOOL; /* von 0 verschieden entspricht True */


typedef unsigned short GRAPH[N+1][MAXVAL]; /* fuer schreibegraph und Isomorphietest */

/* Element der Adjazenztabelle: */

typedef struct K {
                  unsigned short ursprung; /* bei welchem knoten startet die kante */
                  unsigned short name;  /* Identifikation des Knotens, mit
                                      dem Verbindung besteht */
		   long dummy;   /* fuer alle moeglichen zwecke */
		   int kantennummer;
                  struct K *prev;  /* vorige Kante im Uhrzeigersinn */
                  struct K *next;  /* naechste Kante im Uhrzeigersinn */
		   struct K *invers; /* die inverse Kante (in der Liste von "name") */
                 } KANTE;

/* "Ueberschrift" der Adjazenztabelle (Array of Pointers): */
typedef KANTE PLANMAP[N+1][MAXVAL];
                 /* Jeweils N KANTEn */
                 /* ACHTUNG: 1. Zeile der Adjazenztabelle hat Index 0 -
                    wird fast nicht benutzt, N Zeilen auch ausreichend
		     In [0][0].name wird aber die knotenzahl gespeichert
		     und in [0][1].name die Zahl der gerichteten Kanten */


schreibegraph(g)
GRAPH g;
{
int i,x,y, unten,oben, maxvalence;


/*
maxvalence=1;
for (i=1; i<=g[0][0]; i++)
         while ((g[i][maxvalence]!=leer) && (g[i][maxvalence]))
                                                         maxvalence++;*/

maxvalence=3;



if (g[0][0]<100)
{

fprintf(stderr," ");

fprintf(stderr,"|%2d",g[0][0]);

for(x=1; (x <= g[0][0])&&(x<=24); x++)  fprintf(stderr,"|%2d",x); fprintf(stderr,"|\n");

fprintf(stderr," ");

for(x=0; (x <= g[0][0])&&(x<=24); x++) fprintf(stderr,"|==");    fprintf(stderr,"|\n");

for(x=0; x < maxvalence; x++)
 {
  fprintf(stderr," |  ");
  for(y=1; (y<=g[0][0])&&(y<=24); y++)
      if (g[y][x] ==leer) fprintf(stderr,"|  "); else fprintf(stderr,"|%2d",g[y][x]);
      fprintf(stderr,"|\n");
 }

unten=25; oben=48;

while(g[0][0]>=unten)
{
fprintf(stderr,"\n");

fprintf(stderr,"    ");

for(x=unten; (x <= g[0][0])&&(x<=oben); x++)  fprintf(stderr,"|%2d",x); fprintf(stderr,"|\n");

fprintf(stderr,"    ");

for(x=unten; (x <= g[0][0])&&(x<=oben); x++) fprintf(stderr,"|==");    fprintf(stderr,"|\n");

for(y=0; y < maxvalence; y++)
 {
  fprintf(stderr,"    ");
  for(x=unten; (x <= g[0][0])&&(x<=oben); x++)
      if (g[x][y]==leer) fprintf(stderr,"|  "); else fprintf(stderr,"|%2d",g[x][y]);
      fprintf(stderr,"|\n");
 }
unten += 24; oben += 24;
}
}

else
{

fprintf(stderr," ");

fprintf(stderr,"|%3d",g[0][0]);

for(x=1; (x <= g[0][0])&&(x<=16); x++)  fprintf(stderr,"|%3d",x); fprintf(stderr,"|\n");

fprintf(stderr," ");

for(x=0; (x <= g[0][0])&&(x<=16); x++) fprintf(stderr,"|===");    fprintf(stderr,"|\n");

for(x=0; x < maxvalence; x++)
 {
  fprintf(stderr," |   ");
  for(y=1; (y<=g[0][0])&&(y<=16); y++)
      if (g[y][x] ==leer) fprintf(stderr,"|   "); else fprintf(stderr,"|%3d",g[y][x]);
      fprintf(stderr,"|\n");
 }

unten=17; oben=32;

while(g[0][0]>=unten)
{
fprintf(stderr,"\n");

fprintf(stderr,"     ");

for(x=unten; (x <= g[0][0])&&(x<=oben); x++)  fprintf(stderr,"|%3d",x); fprintf(stderr,"|\n");

fprintf(stderr,"     ");

for(x=unten; (x <= g[0][0])&&(x<=oben); x++) fprintf(stderr,"|===");    fprintf(stderr,"|\n");

for(y=0; y < maxvalence; y++)
 {
  fprintf(stderr,"     ");
  for(x=unten; (x <= g[0][0])&&(x<=oben); x++)
      if (g[x][y]==leer) fprintf(stderr,"|   "); else fprintf(stderr,"|%3d",g[x][y]);
      fprintf(stderr,"|\n");
 }
unten += 16; oben += 16;
}
}


}


/**************************MAP_TO_GRAPH******************************/

void map_to_graph(PLANMAP map, GRAPH g)

{
KANTE *run, *merk;
int i,val,j;

g[0][0]=map[0][0].name;

for (i=1; i<=map[0][0].name; i++)
   {
     merk=map[i]; g[i][0]=merk->name; run=merk->next; val=1;
     while (run!=merk) { g[i][val]=run->name; val++; run=run->next;
     }
     /*for(j=val; j<=3; j++) g[i][j]=leer;*/
   }
}


/**************************SCHREIBEMAP***********************************/

void schreibemap(PLANMAP map)
{
GRAPH g;
map_to_graph(map,g);
schreibegraph(g);
}



/************************LEAPFROGCODE*****************************/

int leapfrogcode( FILE *fil, PLANMAP map, int adj[] )

{
/* berechnet den leapfrog und schreibt ihn als planarcode auf fil
  geht dabei nicht ueber den Graphen mit einem Knoten in jeder Flaeche
  und dann das Dual, sondern ordnet jeder gerichteten Kante einen Knoten
  zu (der z.B. rechts davon liegt. */

int lf_knotenzahl, knotenzahl, codelaenge,i,j;
unsigned short code_s[7*N2];
unsigned char code_c[7*N2];
KANTE *startedge[N];
int zaehler;
static int first_call=1;

if (first_call)
 { fprintf(fil,">>planar_code<<"); first_call=0; }

knotenzahl=map[0][0].name;
lf_knotenzahl=map[0][1].name;
if (lf_knotenzahl > N2) { fprintf(stderr,"Constant N2 too small, %d > %d\n",lf_knotenzahl,N2);
			 exit(0); }


for (i=zaehler=1; i<= knotenzahl; i++)
 for (j=0; j<adj[i]; j++, zaehler++) { map[i][j].dummy=zaehler; startedge[zaehler]=map[i]+j; }
/* dummy gibt die Nummer des Knotens rechts der Kante an */

if (lf_knotenzahl <= UCHAR_MAX)
 { code_c[0]=lf_knotenzahl;
   codelaenge=1;
   for (i=1; i<= lf_knotenzahl; i++)
     { code_c[codelaenge]=startedge[i]->invers->dummy; codelaenge++;
	code_c[codelaenge]=startedge[i]->invers->prev->dummy; codelaenge++;
	code_c[codelaenge]=startedge[i]->next->invers->dummy; codelaenge++;
	code_c[codelaenge]=0; codelaenge++; }
   fwrite(code_c,sizeof(unsigned char),codelaenge,fil); }

else
 { code_s[0]=lf_knotenzahl;
   codelaenge=1;
   for (i=1; i<= lf_knotenzahl; i++)
     { code_s[codelaenge]=startedge[i]->invers->dummy; codelaenge++;
	code_s[codelaenge]=startedge[i]->invers->prev->dummy; codelaenge++;
	code_s[codelaenge]=startedge[i]->next->invers->dummy; codelaenge++;
	code_s[codelaenge]=0; codelaenge++; }
   putc(0,fil);
   fwrite(code_s,sizeof(unsigned short),codelaenge,fil); }

return(codelaenge);
}




/*************************DECODIEREPLANAR******************************/

void decodiereplanar(unsigned short* code, PLANMAP graph, int adj[N])
{
int i,j,k,puffer,zaehler, kantenzaehler;
KANTE *edge;


/*for (i=0; i< 260*4+1; i++) fprintf(stderr," %d ",code[i]);*/

graph[0][0].name = code[0];
graph[0][1].name=0;

kantenzaehler=0;
zaehler=1;

for(i=1;i<=graph[0][0].name;i++)
   { adj[i]=0;
     for(j=0; code[zaehler]; j++, zaehler++)
	{ if (j==MAXVAL) { fprintf(stderr,"MAXVAL too small: %d\n",MAXVAL);
			   exit(0); }
	  graph[i][j].name=code[zaehler];
	  graph[i][j].ursprung=i;
	  if (code[zaehler]>i) { kantenzaehler++; graph[i][j].kantennummer=kantenzaehler; }
	}
     adj[i]=j; graph[0][1].name += j;
     for(j=1, k=0; j<adj[i]; j++, k++)
	{ graph[i][j].prev=graph[i]+k; graph[i][k].next=graph[i]+j; }
     graph[i][0].prev=graph[i]+adj[i]-1; graph[i][adj[i]-1].next=graph[i];
     zaehler++; /* 0 weglesen */
   }

for(i=1;i<=graph[0][0].name;i++)
   {
     for(j=0; j<adj[i]; j++) if (graph[i][j].name > i)
	{ puffer=graph[i][j].name;
	  for (k=0; graph[puffer][k].name != i; k++);
	  graph[i][j].invers=graph[puffer]+k;
	  graph[puffer][k].invers=graph[i]+j;
	  graph[puffer][k].kantennummer = -(graph[i][j].kantennummer);
	}
   }
}




/*******************MAIN********************************/

main(argc,argv)

int argc;
char *argv[];


{
GRAPH graph;
int zaehlen, welchergraph;
unsigned short code[4*N+2];
unsigned char code2[4*N+2];
int lauf, nullenzaehler;
PLANMAP map;
unsigned char ucharpuffer;
int too_large, i;
int adj[N];

if (N>USHRT_MAX-2) { fprintf(stderr,"Constant N (%d) may be at most %d.\n",N,USHRT_MAX-2);
		     exit(0); }


fprintf(stderr,"Warning ! The leapfrogs are written to stdout in binary format.\n");

welchergraph=zaehlen=0;

if (argc==2) welchergraph=atoi(argv[1]);


for (;fread(&ucharpuffer,sizeof(unsigned char),1,stdin);)
 { if (ucharpuffer=='>') /* koennte ein header sein -- oder 'ne 62, also ausreichend fuer
			     unsigned char */
     { code[0]=ucharpuffer;
	lauf=1; nullenzaehler=0;
	code[1]=(unsigned short)getc(stdin);
	if(code[1]==0) nullenzaehler++;
	code[2]=(unsigned short)getc(stdin);
	if(code[2]==0) nullenzaehler++;
	lauf=3;
	/* jetzt wurden 3 Zeichen gelesen */
	if ((code[1]=='>') && (code[2]=='p')) /*garantiert header*/
	  { while ((ucharpuffer=getc(stdin)) != '<');
	    /* noch zweimal: */ ucharpuffer=getc(stdin);
	    if (ucharpuffer!='<') { fprintf(stderr,"Problems with header -- single '<'\n"); exit(1); }
	    if (!fread(&ucharpuffer,sizeof(unsigned char),1,stdin)) exit(0);
	    /* kein graph drin */
	    lauf=1; nullenzaehler=0; }
	/* else kein header */
     }
   else { lauf=1; nullenzaehler=0; }

   if (ucharpuffer!=0) /* kann noch in unsigned char codiert werden ... */
                       { too_large=0;
			  code[0]=ucharpuffer;
			  if (code[0]>N) { fprintf(stderr,"Constant N too small %d > %d \n",code[0],N); exit(1); }
			  while(nullenzaehler<code[0])
			    { code[lauf]=(unsigned short)getc(stdin);
			      if(code[lauf]==0) nullenzaehler++;
			      lauf++; }
			}
   else  { too_large=1;
           fread(code,sizeof(unsigned short),1,stdin);
	    if (code[0]>N) { fprintf(stderr,"Constant N too small %d > %d \n",code[0],N); exit(1); }
	    lauf=1; nullenzaehler=0;
	    while(nullenzaehler<code[0])
	      { fread(code+lauf,sizeof(unsigned short),1,stdin);
		if(code[lauf]==0) nullenzaehler++;
		lauf++; }
	  }

   zaehlen++;
   if ( (!welchergraph) || (welchergraph==zaehlen) )
     { decodiereplanar(code,map,adj);
	leapfrogcode( stdout, map, adj );
	if (welchergraph == zaehlen) break;
     }
 }

fprintf(stderr,"Transformed %d graphs. \n",zaehlen);



return(0);

}

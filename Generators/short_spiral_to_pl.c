#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include<limits.h>
# include <time.h>
# include <sys/times.h>

#define S        80          /* Maximale Anzahl der 6-Ecke */
#define N        ((4*S)+20)    /* Maximal moegliche Anzahl der Knoten */

#define LISTENLAENGE 3000   /* die anzahl der bei code 3 gepufferten graphen */




#define aussen   (N+2) /* so werden kanten nach aussen markiert */

#define infty    LONG_MAX
#define unbelegt UCHAR_MAX
#define leer     USHRT_MAX-1
#define FL_MAX   UCHAR_MAX
#define KN_MAX   USHRT_MAX
#define False    0
#define True     1
#define nil      0
#define reg      3
#define filenamenlaenge 30 /* sollte vorerst reichen */


/* Typ-Deklarationen: */

typedef signed char BOOL; /* von 0 verschieden entspricht True */

typedef unsigned short KNOTENTYP;   
typedef unsigned char FLAECHENTYP; /* Bereich 1..252 */

typedef KNOTENTYP GRAPH[N+1][3]; /* fuer schreibegraph und Isomorphietest */

/* Element der Adjazenztabelle: */


typedef struct iL {
		  struct iL *next_item;
		  FLAECHENTYP code[8]; /* gleich auf 8 -- mit malloc wuerde auch mehr
					allociert */
		} ITEMLISTE; /* die Items -- d.h. Codes */


typedef struct sL {
                 struct sL **next_level;
		  int number_next;
		  ITEMLISTE *items;
		  ITEMLISTE *last_item;
		} SEQUENZLISTE; /* die verzweigung der liste nach der Sequenz */


typedef struct SL {
		  int total_items; 
		  int total_maps; 
                 SEQUENZLISTE *sechser[S+1];
		  } S_LISTE; /* die erste stufe der liste -- verzweigung nach Anzahl der 6-Ecke */

typedef struct K {
                  KNOTENTYP ursprung; /* bei welchem knoten startet die kante */
                  KNOTENTYP name;  /* Identifikation des Knotens, mit
                                      dem Verbindung besteht */
		   long dummy;   /* fuer alle moeglichen zwecke */
		   BOOL nostart;
		   BOOL noleft; /* fuer die Rekonstruktion: Keine Flaeche links hiervon */
		   BOOL noright; /* fuer die Rekonstruktion: Keine Flaeche rechts hiervon */
		   BOOL mirror_nostart;
		   KNOTENTYP mininame; /* jeweils fuer den minimalitaetstest */
                  struct K *prev;  /* vorige Kante im Uhrzeigersinn */
                  struct K *next;  /* naechste Kante im Uhrzeigersinn */
		   struct K *invers; /* die inverse Kante (in der Liste von "name") */
                 } KANTE;

typedef struct  {
                  int laenge;
                  int sequenz[7];  /* die laenge der luecke. Konvention: Beendet durch "leer" */
		   KANTE *kanten[7];/* die letzten aussenkanten vor der sequenz */
		   char k_marks[7]; /* 1 wenn anfang einer kanonischen Sequenz, 0 sonst */
		 } SEQUENZ;


typedef struct le { FLAECHENTYP code[12];
		    struct le *smaller;
		    struct le *larger; } LISTENTRY;


/* "Ueberschrift" der Adjazenztabelle (Array of Pointers): */
typedef KANTE PLANMAP[N+1][3];
                 /* Jeweils 3 KANTEn */
                 /* ACHTUNG: 1. Zeile der Adjazenztabelle hat Index 0 -
                    wird fast nicht benutzt, N Zeilen auch ausreichend
		     In [0][0].name wird aber die knotenzahl gespeichert */


static unsigned char cc__[65536];

/**********************SCHREIBEGRAPH**********************************/

void schreibegraph(g)
GRAPH g;
{
int x,y, unten,oben, maxvalence;
fprintf(stderr,"\n\n ");

/*maxvalence=0;
for (x=1; x<=g[0][0]; x++)
{ for (y=1; g[x][y] != leer; y++);
  if (y>maxvalence) maxvalence=y; }*/

maxvalence=reg;


fprintf(stderr,"|%2d",g[0][0]);

for(x=1; (x <= g[0][0])&&(x<=24); x++)  fprintf(stderr,"|%2d",x); fprintf(stderr,"|\n");

fprintf(stderr," ");

for(x=0; (x <= g[0][0])&&(x<=24); x++) fprintf(stderr,"|==");    fprintf(stderr,"|\n");

for(x=0; x < maxvalence; x++)
 {
  fprintf(stderr," |  ");
  for(y=1; (y<=g[0][0])&&(y<=24); y++)
      if (g[y][x] ==leer) fprintf(stderr,"|  ");
/*       else if (g[y][x] ==MARKE) fprintf(stderr,"| M");*/
      else if (g[y][x] ==aussen) fprintf(stderr,"| a");
      else fprintf(stderr,"|%2d",g[y][x]);
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
      if (g[x][y]==leer) fprintf(stderr,"|  "); 
/*       else if (g[x][y] ==MARKE) fprintf(stderr,"| M");*/
      else if (g[x][y] ==aussen) fprintf(stderr,"| a");
      else fprintf(stderr,"|%2d",g[x][y]);
      fprintf(stderr,"|\n");
 }
unten += 24; oben += 24;
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
     for(j=val; j<=reg; j++) g[i][j]=leer;
   }
}

/**************************SCHREIBEMAP***********************************/

void schreibemap(PLANMAP map)
{
GRAPH g;
map_to_graph(map,g);
schreibegraph(g);
}

/*************************LONGCODE*****************************/

int long_code( PLANMAP map)
{
/* Codiert die Einbettung in codeK oder codeF und gibt die laenge des codes zurueck */
int i,zaehler;
KANTE *merke, *lauf;
FLAECHENTYP codeF[4*N];
KNOTENTYP codeK[4*N];
static int first_call=1;

if (first_call)
 { fprintf(stdout,">>planar_code<<"); first_call=0; }

if (map[0][0].name <= FL_MAX)
{
zaehler=1;
codeF[0]=map[0][0].name;
for(i=1;i<=map[0][0].name;i++)
   { merke=map[i]; codeF[zaehler]=merke->name; zaehler++;
     for(lauf=merke->next; lauf!=merke; lauf=lauf->next) 
	           { codeF[zaehler]=lauf->name; zaehler++; }
     codeF[zaehler]=0; zaehler++; }
fwrite(codeF,sizeof(FLAECHENTYP),zaehler,stdout);
}
else /* zu viele knoten fuer FLAECHENTYP */
{
zaehler=1;
codeK[0]=map[0][0].name;
for(i=1;i<=map[0][0].name;i++)
   { merke=map[i]; codeK[zaehler]=merke->name; zaehler++;
     for(lauf=merke->next; lauf!=merke; lauf=lauf->next) 
	           { codeK[zaehler]=lauf->name; zaehler++; }
     codeK[zaehler]=0; zaehler++; }
fwrite(codeK,sizeof(KNOTENTYP),zaehler,stdout);
}

return(zaehler);
}

/*******************INIT_MAP************************/

void init_map(PLANMAP map)
{
int i,j;

map[0][0].name=0;

for (i=1; i<=N; i++)
{
map[i][0].next= map[i]+1; map[i][0].prev= map[i]+2;
map[i][1].next= map[i]+2; map[i][1].prev= map[i];
map[i][2].next= map[i]; map[i][2].prev= map[i]+1;

for (j=0; j<3; j++) 
         { map[i][j].ursprung=i;
	    map[i][j].name=leer;
           map[i][j].invers=nil; }
}
}


/********************BAUE_POLYGON*******************/
/* Baut ein einzelnes leeres Polygon mit n Ecken (n>=3) 
  und initialisiert map */

void baue_polygon(int n, PLANMAP map, KANTE **marke )
{
int j;

if (n<3) { fprintf(stderr,"Error, no 2-gons allowed !\n"); return; }

/* sicherheitshalber erstmal loeschen und initialisieren */

/*init_map(map);*/

/* Immer: erster Eintrag zurueck, 2. nach aussen, dritter vor */

map[1][0].name=n;   map[1][1].name=aussen;   map[1][2].name=2;
map[1][0].invers=map[n]+2; map[1][1].invers=nil; map[1][2].invers=map[2];

(*marke)=map[1]+1;

for (j=2; j<n; j++)
{
map[j][0].name=j-1;   map[j][1].name=aussen;   map[j][2].name=j+1;
map[j][0].invers=map[j-1]+2; map[j][1].invers=nil; map[j][2].invers=map[j+1];
}

map[n][0].name=n-1;   map[n][1].name=aussen;   map[n][2].name=1;
map[n][0].invers=map[n-1]+2; map[n][1].invers=nil; map[n][2].invers=map[1];

map[0][0].name=n;

}

/*********************ADD_POLYGON***********************************/

void add_polygon(int n, PLANMAP map, KANTE *start, KANTE **lastout)
/* fuegt ein weiteres polygon einer Reihe an. Dabei ist n die groesse des polygons. 
  Angefuegt wird immer an start. Die Marke wird nicht versetzt. Ueber lastout wird
  die letzte Aussenkante des Polygons zurueckgegeben. */


{
int new_tempknz, tempknz;
KANTE *ende;
int common_vertices;


for (ende=start->next->invers->next, common_vertices=2; ende->name != aussen; 
	  ende=ende->invers->next) { common_vertices++;
				     if (common_vertices==7) { *lastout=nil; return; }
				   }


if (n<common_vertices) 
  { fprintf(stderr,"polygon to insert too small !\n"); 
    exit(0); }

/* es muessen also n-common_vertices knoten hinzugefuegt werden */

tempknz=map[0][0].name;
new_tempknz=tempknz+n-common_vertices;

if (n-common_vertices==0) /* dann kommt kein knoten dazu */
 {
   start->name=ende->ursprung; start->invers=ende;
   ende->name=start->ursprung; ende->invers=start;
   for (common_vertices=1; ende->name!=aussen; ende=ende->invers->prev)
     { common_vertices++; if(common_vertices==8) {*lastout=nil; return; } }
   *lastout=ende;
   return;
 }

if (n-common_vertices==1) /* dann kommt nur ein knoten dazu */
{
tempknz++;
start->name=tempknz; start->invers=map[tempknz];
map[tempknz][0].name=start->ursprung; map[tempknz][0].invers=start;
map[tempknz][1].name=aussen; map[tempknz][1].invers=nil;
map[tempknz][2].name=ende->ursprung; map[tempknz][2].invers=ende;
ende->name=tempknz; ende->invers=map[tempknz]+2;
*lastout=map[tempknz]+1;
map[0][0].name=tempknz;
return;
}


/* es bleibt: mindestens zwei neue knoten */

tempknz++;
start->name=tempknz; start->invers=map[tempknz];
map[tempknz][0].name=start->ursprung; map[tempknz][0].invers=start;
map[tempknz][1].name=aussen; map[tempknz][1].invers=nil;
map[tempknz][2].name=tempknz+1; map[tempknz][2].invers=map[tempknz+1];

for (tempknz++; tempknz<new_tempknz; tempknz++)
   { map[tempknz][0].name=tempknz-1; map[tempknz][0].invers=map[tempknz-1]+2;
     map[tempknz][1].name=aussen; map[tempknz][1].invers=nil;
     map[tempknz][2].name=tempknz+1; map[tempknz][2].invers=map[tempknz+1]; }

/* und nun noch den letzten knoten */
map[tempknz][0].name=tempknz-1; map[tempknz][0].invers=map[tempknz-1]+2;
map[tempknz][1].name=aussen; map[tempknz][1].invers=nil;
map[tempknz][2].name=ende->ursprung; map[tempknz][2].invers= ende;
ende->name=tempknz; ende->invers=map[tempknz]+2;
*lastout=map[tempknz]+1;
map[0][0].name=tempknz;
}


/**********************************DECODIERE************************/

void decodiere(PLANMAP map, FLAECHENTYP code[])
{
KANTE *run;
int stelle,zaehler;

/*for (i=0;i<12;i++) fprintf(stderr,"%d ",code[i]); fprintf(stderr," \n");*/

map[0][0].name=0;

if (code[0]==1) { baue_polygon(5,map,&run); stelle=1; }
          else { baue_polygon(6,map,&run); stelle=0; }
zaehler=2;

while (run != nil)
 { 
   if (code[stelle]==zaehler) { add_polygon( 5, map, run, &run); stelle++; }
                         else add_polygon(6,map,run,&run);
   zaehler++;
 }

/*schreibemap(map);*/

}


int lcode(unsigned char *code, int codelengte)
{
 static unsigned char *start=0, *end=0, *bufferend;
 int i;

 if (start+codelengte>end)
   { 
     for (i=0;i<end-start; i++) cc__[i]=start[i];
     end=cc__+i+fread(cc__+i,sizeof(unsigned char),65000,stdin);
     start=cc__;
     if (start+codelengte>end) 
	{ if (start==end) return EOF;
	else { fprintf(stderr,"problem with the codes\n"); exit(1); }
	}
   }

 bufferend=start+codelengte;
 for ( ; start!=bufferend; start++, code++) *code=*start;
 if (start>end) { fprintf(stderr,"Problem with the codes !\n"); exit(0); }
 return 1; 
}



/************************MAIN*****************************************/

int main(argc,argv)

int argc;
char *argv[];

{
int zaehler=0;
FLAECHENTYP code[13];
unsigned char laenge_gleich;
int welcher;
PLANMAP map;

code[12]=0;
welcher=0;
if (argc==2) welcher= atoi(argv[1]);

init_map(map);

while ((lcode(&laenge_gleich,1)!=EOF) && 
      !((welcher != 0)  && (zaehler>=welcher)))
 { /*fprintf(stderr,"laenge_gleich:%d \n",laenge_gleich);*/
   zaehler++;
   lcode(code+laenge_gleich,12-laenge_gleich);

/*if (code[0] != 1) fprintf(stderr,"NR: %d \n", zaehler);*/

if ((welcher==0) || (welcher==zaehler))
{    decodiere(map,code);
   long_code(map);  }

 }
if (welcher==0)
fprintf(stderr,"Transformed %d maps. \n",zaehler);
else
fprintf(stderr,"Transformed 1 map. \n");
return 0;
}

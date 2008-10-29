/* zu beweisen: es gibt immer eine flaeche mit 2 aufeinander folgenden aussenkanten, so
dass der patch bei der entfernung nicht zerfaellt */

/* Programm zur Generierung von hydocarbonstrukturen. In wirklichkeit werden nur patches mit
Valenz 2 und 3 (2 nur am rand) erzeugt. An die Valenz 2 Strukturen wird zum Schluss nur noch
Wasserstoff angeklebt. */

#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include<string.h>
#include<limits.h>
#include<sys/stat.h>

#ifndef NOTIMES
#include<time.h>
#include<sys/times.h>
#endif //NOTIMES

#define N        (450)    /* Maximal moegliche Anzahl der Knoten */


#define aussen   (N+2) /* so werden kanten nach aussen markiert. Die Aussenkante ist (wenn ueberhaupt
			  vorhanden) immer die 2.Kante -- d.h. mit index [1]*/

#define infty    INT_MAX-3 /* damit ein ++ nicht etwas negatives daraus macht */
#define leer     USHRT_MAX-1
#define unbelegt USHRT_MAX
#define False    0
#define True     1
#define nil      0
#define reg      3
#define filenamenlaenge 30 /* sollte vorerst reichen */

#ifndef NOTIMES
#include <sys/times.h>
#if !defined(CLK_TCK) && !defined(_SC_CLK_TCK)
#include <time.h>
#endif
#if !defined(CLK_TCK) && !defined(_SC_CLK_TCK)
#include <unistd.h>
#endif
#if !defined(CLK_TCK) && defined(_SC_CLK_TCK)
#define CLK_TCK sysconf(_SC_CLK_TCK)
#endif
#ifndef CLK_TCK
#define CLK_TCK 60     /* If the CPU time stated by the program appears
		      to be out by a constant ratio, the most likely
		      explanation is that the code got to this point but
		      60 is the wrong guess.  Another common value is 100. */
#endif

#define time_factor CLK_TCK
#endif //NOTIMES


/* Typ-Deklarationen: */

typedef  signed char BOOL; /* von 0 verschieden entspricht True */

typedef unsigned short KNOTENTYP;   
typedef unsigned short FLAECHENTYP; /* DO NOT CHANGE ! */

typedef KNOTENTYP GRAPH[N+1][3]; /* fuer schreibegraph und Isomorphietest */

/* Element der Adjazenztabelle: */


typedef struct K {
                   KNOTENTYP ursprung; /* bei welchem knoten startet die kante */
                   KNOTENTYP name;  /* Identifikation des Knotens, mit
                                       dem Verbindung besteht */
		   long dummy;   /* fuer alle moeglichen zwecke */
		   int aussenkanten;
		   BOOL kanpossible;
		   BOOL mirror_kanpossible;
		   BOOL ist_rand;
		   BOOL lastedge[N];  
				  /* Um von jedem Orbit genau eine Kante zu nehmen, um fortzufahren,
				     wird die erste Kante markiert, die nicht mehr genommen werden 
				     muss (Ausser wenn es auch die erste ist). An alle Kanten von 
				     der ersten (einschliesslich) bis zu der hier markierten 
				     (ausschliesslich) wird dann angefuegt. Das [N] korrespondiert
				     zu der Tiefe, in der es gesetzt wird */
                   struct K *prev;  /* vorige Kante im Uhrzeigersinn */
                   struct K *next;  /* naechste Kante im Uhrzeigersinn */
		   struct K *invers; /* die inverse Kante (in der Liste von "name") */
                  } KANTE;

typedef struct  {
                   int laenge;
                   int sequenz[7];  /* die laenge der luecke. Konvention: Beendet durch "leer" */
		   KANTE *kanten[7];/* die letzten aussenkanten vor der sequenz */
		   signed char k_marks[7]; /* 1 wenn anfang einer kanonischen Sequenz, 0 sonst */
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


/*********************GLOBALE*VARIABLEN********************************/

/* Die folgenden Variablen werden von fast allen Programmteilen benutzt
   oder werden aus anderen Gruenden global verwaltet. Trotzdem werden
   sie in einigen Faellen auch an Funktionen uebergeben, um die
   Parameterabhaengigkeit der Funktionen zu betonen */
/* Variablen-Deklarationen: */

void komplettieren();
void bestimme_darstellung();
void bestimme_darstellung_inv();
void entferne_polygon();
BOOL kanonizitaetstest();

int C; /* die Anzahl der C-Atome */
int H; /* die Anzahl der H-Atome */
int maxgap=N; /* die maximale Anzahl von Kohlenstoffatomen am Rand in Folge ohne H-Bindung
		 -- manche sagen, nur <= 4 ist interessant. konstr_maxgap wird im programm teilweise 
		 manipuliert, muss aber mindestens 2 sein. kann mittels der option gap
		 gestezt werden */
int konstr_maxgap=N; /* wenn nachtraeglich Ringe gelegt werden, dann kann bei der EIGENTLICHEN
		      konstruktion kein gap der groesse>2 auftreten. Beim Legen des LETZTEN
		      Ringes koennen aber vielleicht 3er gaps entstehen -- deshalb muss maxgap
		      waehrend der konstuktion unterschieden werden */
int compu_maxgap=N; /* die minimale Schranke, die aus rechnerischen Gruenden schon bestimmt werden
		     kann (manchmal koennen einfach keine 4er Luecken auftreten oder so) */


int pentagons, hexagons;
int boundarylength, interior; /* laenge des randes, bzw anzahl der inneren knoten beim fertigen
				 patch */
int tempint[N]; /* momentane innere -- d.h. nicht-rand-knoten. die anzahl steigt monoton */
BOOL IPR=0, noout=0, without_H=0, peri_condensed=0, to_stdout=0;

int strukturzaehler=0;

int reconstruct3[N]; /* enthaelt die anzahl der hexagone mit 3 aussenkanten, die nach einem kompletten
		       ring eingefuegt werden muessen. Wenn erst 2 ringe , dann ein ring mit 3er, 
			dann ein ring mit 4er angefuegt werden muessen, enthielte reconstruct3 
			[4,,3,0,0] (beachte die reihenfolge) */
int reconstruct4[N]; /* entsprechend oben die Anzahl der 4er hexagone */
int constructlevelzaehler= -1; /* der maximale gueltige eintrag. -1 wenn einfach nur rausgeschrieben
				  (also keine ringe angelegt) werden muss */

int dbz=0, dbm=0;

BOOL komplettiermodus; /* ist der komplettiermodus oder der konstruktionsmodus beim 
			  kanonizitaetstest gefragt ? */

FILE *outfile;

int memcmp_own(), memcmp_owninv(); 

#ifdef DEBUG
/********************SCHREIBEZYKLISCHELISTE**********************/

/* gibt die adjazenzliste incl pointer ... aus */

void schreibezyklischeliste(KNOTENTYP which, PLANMAP map)
{
KANTE *lauf, *li;

li=map[which];

fprintf(stderr,"Start of vertex adjacency list: %d\n",which);
fprintf(stderr,"name: %d  address: %d  prev: %d  next: %d inverse: %d  origin: %d\n",li->name, li, li->prev, li->next, li->invers, li->ursprung);
for (lauf=li->next; lauf !=li; lauf=lauf->next)
fprintf(stderr,"name: %d  address: %d  prev: %d  next: %d  inverse: %d  origin: %d\n",lauf->name, lauf, lauf->prev, lauf->next, lauf->invers, lauf->ursprung);
fprintf(stderr,"End of adjacency list\n");
}

/*********************SCHREIBELISTEN*****************************/

void schreibelisten(PLANMAP map) /*alle*/

{
KNOTENTYP i;
for(i=1; i<=map[0][0].name; i++) schreibezyklischeliste(i,map);
}


/**********************SCHREIBESEQUENZ*********************/

void schreibesequenz(signed char *s, int l)
{
int i;

fprintf(stderr,"\n");
for (i=0; i<l; i++) fprintf(stderr," %d",s[i]);
fprintf(stderr,"\n");
}

/**********************SCHREIBESEQUENZ_INV*********************/

void schreibesequenz_inv(signed char *s, int l)
{
int i;

fprintf(stderr,"\n");
for (i=0; i<l; i++) fprintf(stderr," %d",s[-i]);
fprintf(stderr,"\n");
}


/*********************SCHREIBEKANTE***************************/

void schreibeKANTE(KANTE * li)
{
fprintf(stderr,"address: %d, name: %d, next: %d, prev: %d, inverse: %d\n",
                li,li->name,li->next,li->prev,li->invers);
}


/**********************MAPCOMPARE************************************/

int mapcompare(PLANMAP map)
{

if (map[1][0].name != 5 )  return(0);
if (map[1][1].name != 6 )  return(0);
if (map[1][2].name != 2 )  return(0);

if (map[2][0].name != 1 )  return(0);
if (map[2][1].name != 9 )  return(0);
if (map[2][2].name != 3 )  return(0);

if (map[3][0].name != 2 )  return(0);
if (map[3][1].name != 20 )  return(0);
if (map[3][2].name !=  4)  return(0);

if (map[4][0].name != 3 )  return(0);
if (map[4][1].name != 23 )  return(0);
if (map[4][2].name !=  5)  return(0);

if (map[16][0].name != 11 )  return(0);
if (map[16][1].name != 27 )  return(0);
if (map[16][2].name != 17 )  return(0);

if (map[23][0].name != 22 )  return(0);
if (map[23][1].name != 24 )  return(0);
if (map[23][2].name != 4 )  return(0);

return(1);
}




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

#endif
// end of ifdef DEBUG

/**********************CHECKSIZE_RIGHT_2**************************************/

/* bestimmt die groesse der flaeche rechts von edge -- ist da keine gibt's 
   hier aber keine Probleme, sondern 6 wird zurueckgegeben -- dient nur
   zur Ueberpruefung, ob da ein 5-Eck ist.

   Es wird allerdings angenommen, dass edge->name nicht aussen ist
*/


int checksize_right_2( KANTE* edge)
{
KANTE *run; 
int zaehler=1;

for (run=edge->invers->prev; run != edge; run=run->invers->prev) 
  { if (run->name==aussen) return(6); zaehler++; }
return(zaehler);
}




/**********************CHECKSIZE_2**************************************/

/* bestimmt die groesse der flaeche links von edge -- ist da keine gibt's 
   hier aber keine Probleme, sondern 6 wird zurueckgegeben -- dient nur
   zur Ueberpruefung, ob da ein 5-Eck ist.

   Es wird allerdings angenommen, dass edge->name nicht aussen ist
*/


int checksize_2( KANTE* edge)
{
KANTE *run; 
int zaehler=1;

for (run=edge->invers->next; run != edge; run=run->invers->next) 
  { if (run->name==aussen) return(6); 
    zaehler++;  }
return(zaehler);
}


      
	      
/************************CHECK_IPR***************************************/

BOOL check_ipr(PLANMAP map)
{
int i,j;
KANTE *run;

/* nur voruebergehend als sicherer test zur ueberpruefung gedacht */

for (i=1; i<=map[0][0].name; i++) for (j=0;j<3; j++) map[i][j].dummy=0;

for (i=1; i<=map[0][0].name; i++) 
  for (j=0;j<3; j++) 
    { run=map[i]+j;
      if (!run->dummy)
	{
	  run->dummy=1;
	  if (run->invers != nil) run->invers->dummy=1;
	  if (run->name != aussen)
	    if ((checksize_2(run)==5) && (checksize_right_2(run)==5)) 
	      { fprintf(stderr,"error with %d-->%d, exiting\n",run->ursprung,run->name);  exit(1); }
	}
    }
return(1);
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


/**********************BAUE_POLYGON*********************/
/* Baut ein einzelnes leeres Polygon mit n Ecken (n>=3) 
   und initialisiert map */

void baue_polygon(int n, PLANMAP map, KANTE **marke )
{
int j;

if (n<3) { fprintf(stderr,"Error, no 2-gons allowed !\n"); return; }

/* sicherheitshalber erstmal loeschen und initialisieren */

init_map(map);

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

/**********************CHECKSIZE_RIGHT**************************************/

/* bestimmt die groesse der flaeche rechts von edge -- ist da keine gibt's Probleme */

int checksize_right( KANTE* edge)
{
KANTE *run; 
int zaehler=1;

for (run=edge->invers->prev; run != edge; run=run->invers->prev) zaehler++;
return(zaehler);
}



/**********************CHECKSIZE**************************************/

/* bestimmt die groesse der flaeche links von edge -- ist da keine gibt's Probleme */

int checksize(KANTE* edge)
{
KANTE *run; 
int zaehler=1;

for (run=edge->invers->next; run != edge; run=run->invers->next) zaehler++;
return(zaehler);
}

/*************************CODIEREPLANAR*****************************/

int codiereplanar( PLANMAP map, unsigned char *codeF, unsigned short *codeK, int dolarge )
{
/* Codiert die Einbettung in codeK oder codeF und gibt die laenge des codes zurueck */
/* dolarge !=0 heisst: auf jeden fall den grossen code nehmen */
int i,zaehler;
KANTE *merke, *lauf;

if ((map[0][0].name <= UCHAR_MAX) && !dolarge)
{
zaehler=1;
codeF[0]=(unsigned char)map[0][0].name;
for(i=1;i<=map[0][0].name;i++)
    { merke=map[i]; codeF[zaehler]=(unsigned char)merke->name; zaehler++;
      for(lauf=merke->next; lauf!=merke; lauf=lauf->next)
	if (lauf->name !=aussen) { codeF[zaehler]=(unsigned char)lauf->name; zaehler++; }
      codeF[zaehler]=0; zaehler++; }
}
else /* zu viele knoten fuer unsigned char */
{
zaehler=1;
codeK[0]=map[0][0].name;
for(i=1;i<=map[0][0].name;i++)
    { merke=map[i]; codeK[zaehler]=merke->name; zaehler++;
      for(lauf=merke->next; lauf!=merke; lauf=lauf->next) 
	if (lauf->name != aussen) { codeK[zaehler]=lauf->name; zaehler++; }
      codeK[zaehler]=0; zaehler++; }
}
return(zaehler);
}


/********************FACEOK************************************/

/* siehe randok --- gecheckt wird: Der Rand hat nur eine komponente und das komplement enthaelt
   mindestens zwei kanten */

BOOL faceok(PLANMAP map,KANTE *start)

{ KANTE *run;
  int innenkanten=0, komponenten=0;

/* angefangen wird mit einer randkante */

/*if (start->name == aussen || !start->ist_rand) 
  { fprintf(stderr,"ERROR -- starting at exterior or not boundary edge\n");
			     exit(0); } */

if ( !start->next->ist_rand ) komponenten++;
/* anfang von aussenteil gefunden */

run=start->invers->prev; /* naechste Kante in dem face */

while (run != start)
  { 
    if (!run->ist_rand) innenkanten++;
    else  if ( !(run->next->ist_rand) ) komponenten++; /* anfang von aussenteil gefunden */
    run=run->invers->prev; 
  }

if ((komponenten==1) && (innenkanten>=2)) return(1);
  else return(0);
}




/***********************RANDOK************************************/

/* Ist der Rand strictly peri-condensed ? (alle inneren knoten zusammenhaengend und keine
   "cata-condensed appendages" -- d.h. teile, die ueber nur eine Kante dranhaengen */

BOOL randok(PLANMAP map)

{

int i,j;
KANTE *run, *edge;

if (map[0][0].name==6) return(1);

edge=nil;
for (i=1; i<= map[0][0].name; i++) for (j=0; j<3; j++) 
     { map[i][j].ist_rand=0;
       if ((edge==nil) && ((map[i][j]).name==aussen) && ((map[i][j]).next->invers->next->name !=aussen))
	 edge=map[i]+j;
     }


run=edge; 
do
  { run->ist_rand=1;
    if (run->name==aussen) { run->next->ist_rand=run->prev->ist_rand=1;
			     run=run->next->invers->next;
			   }

    else { run->prev->ist_rand=1;
	   run=run->invers->next;
	 }
  } while (run != edge);

run=edge->next->invers->next; 

while (run != edge)
  { if (run->name==aussen) /* dann ist es der erste in einer reihe */
      { 
	run=run->next->invers->next;
	while ((run->name==aussen) && (run != edge))
	    run=run->next->invers->next;
      }
    else /* d.h. es ist eine "randkante" */
      { if (!faceok(map,run)) return(0);
	run=run->invers->next;
      }
  }
return(1);
}






/*********************AUFSCHREIBEN*********************************/

void aufschreiben(PLANMAP map)
{
static int write_header = 1;
int merke,i, aussenknoten, codelaenge;
unsigned short wechselliste[N];
unsigned short nuller;
unsigned char code[4*N+2];
unsigned short codel[4*N+2];
int dolarge=0; /* nimm auf jeden fall unsigned short fuer den code */

if (peri_condensed && !randok(map)) return;

strukturzaehler++;
/*fprintf(stderr,"%d structures\n",strukturzaehler);*/

/*check_ipr(map);*/

if (noout) return;

if (write_header) {
  unsigned short word = ((unsigned short) 'b' << 8) | 'l';
  fprintf (outfile, ">>planar_code %ce<<", * (char *) &word);
  write_header = 0;
}

if (without_H)
  { codelaenge=codiereplanar(map,code,codel,dolarge);
    if (map[0][0].name <= UCHAR_MAX) fwrite(code,sizeof(unsigned char),codelaenge,outfile);
      else { fputc(0,outfile); fwrite(codel,sizeof(unsigned short),codelaenge,outfile); }
    return; 
  }



merke=map[0][0].name;

/* das anbauen von valenz 1 knoten verlaesst sich darauf, dass 
   "aussen" immer an der 2.stelle ( ..[1]) steht. Map selbst wird nur soweit
   noetig veraendert -- haupsaechlich wird nur der Code manipuliert.*/
/* "aussen" durch valenz 1 knoten ersetzen: */
for (i=1, aussenknoten=0; i<=merke; i++)
   if (map[i][1].name == aussen)
      { wechselliste[aussenknoten]=i; 
	aussenknoten++;
	map[i][1].name=merke+aussenknoten;
      }

if (merke+aussenknoten > UCHAR_MAX) dolarge=1;
codelaenge=codiereplanar(map,code, codel,dolarge);

if (dolarge)
{nuller=0;
 codel[0]=merke+aussenknoten;
 fputc(0,outfile);
 fwrite(codel,sizeof(unsigned short),codelaenge,outfile);

for (i=0; i<aussenknoten; i++)
  { fwrite(wechselliste+i,sizeof(unsigned short),1,outfile);
    fwrite(&nuller,sizeof(unsigned short),1,outfile);
    map[wechselliste[i]][1].name=aussen;
  }
}
else
{
  code[0]=merke+aussenknoten;
  fwrite(code,sizeof(unsigned char),codelaenge,outfile);
  
  for (i=0; i<aussenknoten; i++)
    { fputc((unsigned char)wechselliste[i],outfile);
      fputc((unsigned char)0,outfile);
      map[wechselliste[i]][1].name=aussen;
    }
}

    
/*schreibemap(map);*/
}

/************************MARKIERE_ORBIT******************************/

/* diese Funktion ist praktisch wie kanonizitaetstest -- nur dass nicht abgebrochen
   wird, wenn etwas besseres gefunden wird */

void markiere_orbit(PLANMAP map, KANTE *edge, int tiefe, KANTE **firstedge)

/* tiefe gibt hier an, wieviel flaechen schon drin sind */

{ int i,j,k, dreizaehler, randlaenge;
  KNOTENTYP randsequenz[3*N];
  KANTE *mirror_edge, *merke, *last;
  KNOTENTYP *comparestart;
  KANTE *run, *orbitkante[N+1];
  GRAPH graph0, graph1;
  int test, orbitlaenge, facelength;
  int min, ende;
  int aussenzaehler, orig_aussenkanten, restfaces;

restfaces=hexagons+pentagons;


/* eine kanonische flaeche muss 2 aufeinander folgende aussenkanten haben. Der patch darf
   aber nicht zerfallen, wenn man sie entfernt. Das heisst, dass von einer kante nur dann
   so gestartet werden kann, dass sie kanonisch ist, wenn vorher oder nachher eine
   zweite aussenkante kommt. Es ist nicht moeglich, dass von einer isolierten kante aus
   die sequenz kanonisch ist und das kriterium "zwei aufeinander folgende randkanten" von
   einer anderen Stelle der flaeche erfuellt wird, da dann der rand nicht zusammenhaengend
   waere (ausserdem sind selbst 6-Ecke dafuer zu klein)*/

/* also: suche zwei aufeinander folgende aussenkanten mit edge am rechten rand: */

while ((edge->name != aussen) || (edge->prev->invers->prev->name != aussen) ||
       (edge->next->invers->next->name==aussen))
  { if (edge->name==aussen) edge=edge->next->invers->next;
    else edge=edge->invers->next; }

graph0[0][0]=graph1[0][0]=0; /* als Erkennungsmarke, dass er noch belegt werden muss */

for (i=1; i<= map[0][0].name; i++) for (j=0; j<3; j++) map[i][j].ist_rand=0;
/* kann auch noch optimiert werden durch zaehlerabhaengige marke, so dass nur alle 1000
   Schritte oder so initialisiert werden muss */


/* bei mirror_edge ist graph1 die referenz, sonst (bei edge besser als mirror_edge) graph0 */


/* fuer die Minimalitaet der FLAECHE muss festgestellt werden, von welchem Randpunkt aus
   gesehen und in welcher Richtung sie minimal ist */

for (run=edge->prev->invers->prev, facelength=0; run->name==aussen; 
                          run=run->prev->invers->prev, facelength++)
  { run->kanpossible=run->mirror_kanpossible=0;
    run->next->ist_rand=run->prev->ist_rand=1;
    (run->lastedge)[tiefe]=0; }

mirror_edge=run->next->invers->next;
mirror_edge->mirror_kanpossible=1;
orig_aussenkanten=facelength+1;
edge->aussenkanten=mirror_edge->aussenkanten=orig_aussenkanten;


/* Jetzt ist die linke kante gefunden. Name: mirror_edge. sie liegt facelength knoten von der
   rechten kante entfernt */


/* nun wird die randsequenz belegt */


(edge->lastedge)[tiefe]=0;
edge->kanpossible=1;
edge->mirror_kanpossible=0;
edge->next->ist_rand=edge->prev->ist_rand=1; 
run=edge->next->invers->next; /*insbesondere immer ungleich mirror_edge*/
randsequenz[0]=0;
randlaenge=1;

while (run != mirror_edge)
{   (run->lastedge)[tiefe]=0;
    if (run->name==aussen) /* dann ist es der erste in einer reihe */
      { merke=run; run=run->next->invers->next;
	merke->kanpossible=merke->mirror_kanpossible=0;
	merke->next->ist_rand=merke->prev->ist_rand=1;
	randsequenz[randlaenge]=0; randlaenge++;
	aussenzaehler=1;
	last=merke;
	while (run->name==aussen)
	  { (run->lastedge)[tiefe]=0;
	    run->kanpossible=run->mirror_kanpossible=0;
	    run->next->ist_rand=run->prev->ist_rand=1;
	    last=run;
	    run=run->next->invers->next;
	    aussenzaehler++;
	    randsequenz[randlaenge]=0; randlaenge++;
	  }
	if (aussenzaehler == orig_aussenkanten) 
	  { merke->mirror_kanpossible=last->kanpossible=1;
	    merke->aussenkanten=last->aussenkanten=aussenzaehler; }
      }
    else /* d.h. es ist eine "innenkante" (die erste einer folge) */
      { dreizaehler=1;
	run->ist_rand=run->prev->ist_rand=1;
	randsequenz[randlaenge]=1; randlaenge++;
	run->kanpossible=run->prev->kanpossible=0; 
	run->mirror_kanpossible=run->prev->mirror_kanpossible=0; 
	run=run->invers->next;
	while (run->name != aussen)
	  { (run->lastedge)[tiefe]=0;
	    dreizaehler++;
	    run->ist_rand=run->prev->ist_rand=1;
	    randsequenz[randlaenge]=1; randlaenge++;
	    run->kanpossible=run->prev->kanpossible=0; 
	    run->mirror_kanpossible=run->prev->mirror_kanpossible=0; 
	    run=run->invers->next;
	  }
      }
  }

/* jetzt noch die aussenkanten von mirror_edge bis edge */

for (i=1; i<=facelength; i++) { randsequenz[randlaenge]=0; randlaenge++; }

for (i=0, j=randlaenge, k=2*randlaenge; i<randlaenge; i++, j++, k++)
                                            randsequenz[k]=randsequenz[j]=randsequenz[i];

/* fuer die Markierung des Bereiches kann man auch immer bei "edge" starten --
   vergleiche den kanonizitaetstest */

    /*if (dbz==57925) fprintf(stderr,"non-mirror part\n");*/
    
    /* zunaechst werden nur orientierbare automorphismen ueberprueft (drehungen). Dabei wird ein
       Bereich markiert, auf dem gestartet werden muss: */

    *firstedge=edge; /* sofern keine Spiegelungen das verderben ... */    
    orbitlaenge=0;
    for (i=1, run=edge->next->invers->next; i<randlaenge; i++)
      { 
	if (run->kanpossible)
	  { 
	    if (run->aussenkanten > orig_aussenkanten) test = -1;
	      else test= memcmp_own(randsequenz,randsequenz+i,randlaenge);
	    /* sequenz soll maximal sein */
	    if (test==0)
	      { 
		if (graph0[0][0]==0) bestimme_darstellung(map, edge, graph0);
		bestimme_darstellung(map, run, graph1);
		test= memcmp_own(graph1[1], graph0[1], 3*graph0[0][0]);
	        /* code soll minimal sein */
		if (test==0)
		  { (run->lastedge)[tiefe]=1; 
		    /* diese kante markiert das ende -- sie muss nicht mehr bearbeitet
		       werden, ausser wenn es die erste ist */
		    /*fprintf(stderr,"set lastedge mark at %d\n", run->ursprung);*/
		    i= infty; 
		  }
	      } /* ende test==0 */
	  } /* end kanpossible */
	if (run->name==aussen) run=run->next->invers->next;
	else run=run->invers->next;
	orbitlaenge++;
      }
    
    if (i < infty) { (edge->lastedge)[tiefe]=1; /* die marke war noch nicht gesetzt */
		      orbitlaenge++; }

    /* Jetzt werden nur Spiegelungen gesucht */
    /* alles in die andere Richtung */
    
    orbitkante[1]=edge;
    comparestart=randsequenz+randlaenge-1;
    min=orbitlaenge+1;
    /* es kann sein, dass die kante auf sich selbst gespiegelt wird -- ist aber ein Spiegelpunkt 
       ausserhalb des Orbits, so ist auch einer innerhalb des Orbits, der dann ausgewertet
       werden kann, es reicht also bis orbitlaenge zu gehen, ausser wenn der erste ein Fixpunkt 
       ist -- aber dann reicht orbitlaenge+1 */
    for (i=2, run=edge->next->invers->next; i<=min; i++)
      /* es wird noch in die normale richtung gelaufen */
      { orbitkante[i]=run;
	if (run->mirror_kanpossible)
	  { test= memcmp_owninv(randsequenz,comparestart+i,randlaenge);
	    /* sequenz soll maximal sein */
	    if (test==0)
	      { 
		if (graph0[0][0]==0) bestimme_darstellung(map, edge, graph0);
		bestimme_darstellung_inv(map, run, graph1);
		test= memcmp_own(graph1[1], graph0[1], 3*graph0[0][0]);
	        /* code soll minimal sein */
		if (test==0)
		  { 
		    /* es wurde ein spiegelbild des ersten knotens gefunden */
		    /* die Spiegelachse liegt also in der Mitte */
		    /* alles was zwischen startpunkt und spiegelachse liegt
		       wird auf einen aequivalenten Punkt in der zweiten
		       Haelfte des Orbits abgebildet */
		     if (i%2) /* ungerade heisst: es gibt fixpunkt */
			  *firstedge=orbitkante[(i+1)/2];
		        else *firstedge=orbitkante[i/2];
		     /* falls (no_dreh) ist das die einzige Spiegelung und auch lastedge muss
			jetzt gesetzt werden, sonst muss es eine weitere Spiegelung geben, 
			die den Punkt, der das Drehbild des ersten Punktes ist (orbitlaenge+1), 
			in dieses Spiegelbild abbildet. Die Achse muss in der Mitte zwischen 
			diesem Spiegelbild und dem Drehbild liegen -- es kann die gleiche prozedur 
			benutzt werden */
		     
		     if (i%2)
		       { if (orbitlaenge %2) /* d.h. nur ein fixpunkt */
			   ende = (i+orbitlaenge+2)/2;
		       else /* 2 fixpunkte */
			 ende = (i+1+orbitlaenge)/2;
		       }
		     else /* d.h. i gerade */
		       {
			 if (orbitlaenge%2) /* d.h. ein fixpunkt */
			   ende = (i+orbitlaenge+1)/2;
			 else /* garkein fixpunkt */
			   ende = (i+2+orbitlaenge)/2; }
		     while (i<ende) { if (run->name==aussen) run=run->next->invers->next;
		     else run=run->invers->next;
				      i++; }
		     run->lastedge[tiefe]=1;
		     i=infty;
	      } /* ende test==0 */
	    } /* ende test==0 (aeussere Schleife) */
	  } /* end kanpossible */
	if (run->name==aussen) run=run->next->invers->next;
	else run=run->invers->next;
	} /* ende for ueber randknoten */

return;

}


/*********************ADD_POLYGON3_4***********************************/

int add_polygon3_4( PLANMAP map, KANTE *start, KANTE **lastout, int dreier, int vierer)
/* fuegt ein weiteres 6-Eck einer Reihe an. Dabei ist n die groesse des polygons. 
   hier muessen 3 oder 4 aussenkanten sein -- gibt die anzahl der aussenkanten
   zurueck.
*/


{
int new_tempknz, tempknz;
KANTE *ende;
int common_vertices;



for (ende=start->next->invers->next, common_vertices=2; ende->name != aussen; 
	  ende=ende->invers->next) common_vertices++;

/* genau 3 oder 4 aussenkanten */
if (dreier && vierer) { if ( common_vertices > 3 )  { *lastout=nil; return(0); } }
 else if (dreier) { if ( common_vertices != 3 )  { *lastout=nil; return(0); } }
       else /*d.h. vierer*/ { if ( common_vertices != 2 )  { *lastout=nil; return(0); } }

/* es muessen also mindestens 3 knoten hinzugefuegt werden */

tempknz=map[0][0].name;
new_tempknz=tempknz+6-common_vertices;

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

return(6-common_vertices);
}


/*********************ADD_HEXAGON_IN_RING***********************************/

void add_hexagon_in_ring( PLANMAP map, KANTE *start, KANTE **lastout)
/* fuegt ein weiteres polygon einer Reihe an. Dabei ist n die groesse des polygons. 
   Angefuegt wird immer an start. Ueber lastout wird  die letzte Aussenkante des Polygons 
   zurueckgegeben. Die Boundingkriterien sind anders als in add_polygon
   n sollte hier immer 6 sein.
*/


{
int new_tempknz, tempknz;
KANTE *ende;
int common_vertices;

for (ende=start->next->invers->next, common_vertices=2; ende->name != aussen; 
	  ende=ende->invers->next) common_vertices++;

/* hier kann es passieren, dass beim letzten Polygon sogar gar keine aussenkante dran ist */

/* es muessen also 6-common_vertices knoten hinzugefuegt werden */

tempknz=map[0][0].name;
new_tempknz=tempknz+6-common_vertices;

if (6-common_vertices==0) /* dann kommt kein knoten dazu */
  { start->name=ende->ursprung; start->invers=ende;
    ende->name=start->ursprung; ende->invers=start;
    *lastout=nil;
    return;
  }

if (6-common_vertices==1) /* dann kommt nur ein knoten dazu */
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


/************************ENTFERNE_RING***************************/

void entferne_ring(PLANMAP map, KANTE *run)
/* entfernt den ring, in dem run von einem inneren auf einen randknoten
   zeigt */

{
KANTE *merke;

run->name=aussen; run=run->invers;
run->invers->invers=nil;
merke=run->prev;
(map[0][0].name)--;

run=merke->invers->next;

while (run != merke)
{
(map[0][0].name)--;
if (run->name == aussen) run=run->next->invers->next;
else { run->next->invers->name=aussen;
       run->next->invers->invers=nil;
       run=run->invers->next; }
}
}



/**************************GAPOK*********************************/

int gapok(PLANMAP map, KANTE *aussenkante, int maxgap)
/* ueberprueft ob ein gap existiert, das groesser als maxgap ist.
   In dem Falle wird 0 zurueckgegeben, sonst die Anzahl der Aussenkanten 
   */

{
KANTE *run, *endedge;
int aussenzaehler, dreizaehler;

run=aussenkante;
if (run->name != aussen) /* die Kante suchen, ab der die Flaeche eingefuegt wird,
			    die diese ueberstreicht */
  { run=run->prev;
    while (run->name != aussen){ run=run->invers->prev; }
  }


endedge=run->prev->invers->prev;
while (endedge->name != aussen) endedge=endedge->invers->prev;

aussenzaehler=1;

while (run != endedge)
{   
    if (run->name==aussen) /* dann ist es der erste in einer reihe */
      { run=run->next->invers->next;
	aussenzaehler++;
	while ((run->name==aussen) && (run != endedge))
	  { run=run->next->invers->next;
	    aussenzaehler++;
	  }
      }
    else /* d.h. es ist eine "innenkante" (die erste einer folge) */
      { dreizaehler=1;
	run=run->invers->next;
	while (run->name != aussen)
	  { dreizaehler++;
	    run=run->invers->next;
	  }
	if (dreizaehler>maxgap) return(0); 
      }
  }

return(aussenzaehler);
}

/*************************ADD_ANOTHER************************************/

/* Baut an einem patch weiter, indem ein weiteres dreier hexagon angefuegt wird */

void add_another(PLANMAP map, KANTE* start, int wievielnoch3, int wievielnoch4, int level)

{ KANTE *testedge, *run, *firstedge;
  int tiefe, neu;

    tiefe=map[0][0].name;
    neu=add_polygon3_4(map,start,&testedge,wievielnoch3,wievielnoch4);
    if (testedge != nil) /* es wurde wirklich etwas eingefuegt */
      { 
	if (kanonizitaetstest(map,testedge, tiefe, &firstedge)) 
	                                    /* weiterbauen da flaeche kanonisch war: */
	  { 
	    if ((wievielnoch3+wievielnoch4) ==1)
	      komplettieren(map,level-1,testedge);
	    else 
	      { run=firstedge;
		if (run->name != aussen) /* die Kante suchen, ab der die Flaeche eingefuegt wird,
					    die diese ueberstreicht */
		  { run=run->prev;
		    while (run->name != aussen){ run=run->invers->prev; }
		  }
		do
		  { 
		    if (neu==3) add_another(map,run,wievielnoch3-1,wievielnoch4,level);
		      else add_another(map,run,wievielnoch3,wievielnoch4-1,level);
		    run=run->next->invers->next;
		    while ((run->name != aussen) && (!(run->lastedge)[tiefe])) run=run->invers->next; }
		while (!(run->lastedge)[tiefe]);
	      }
	  }
	entferne_polygon(map,start);
      }
}



/*************************ADD_RING*********************************/

void add_ring(PLANMAP map, KANTE *aussenkante, int level)
{
KANTE *run, *run2, *merke, *merke2;
int number_to_add, i, tiefe, tiefe2, neu;

run=aussenkante;
if (run->name != aussen) /* die Kante suchen, ab der die Flaeche eingefuegt wird,
			    die diese ueberstreicht */
  { run=run->prev;
    while (run->name != aussen){ run=run->invers->prev; }
  }

number_to_add= gapok(map,run,2);
if (number_to_add==0) return;

merke=run;

for (i=1; i<= number_to_add; i++) add_hexagon_in_ring(map, run, &run);
if (run==nil) run=merke->invers->prev;
if (run->name != aussen)   { run=run->prev;
			     while (run->name != aussen){ run=run->invers->prev; }
			   }
if ((reconstruct3[level]==0) && (reconstruct4[level]==0)) komplettieren(map, level-1, run);
else { tiefe=map[0][0].name;
       markiere_orbit(map,run, tiefe, &run);
       if (run->name != aussen) /* die Kante suchen, ab der die Flaeche eingefuegt wird,
				   die diese ueberstreicht */
	 { run=run->prev;
	   while (run->name != aussen){ run=run->invers->prev; }
	 }
       do
	 {  neu=add_polygon3_4( map, run, &merke2, reconstruct3[level], reconstruct4[level]);
	   if (merke2 != nil) /* oder aequivalent neu != 0 */
	     { if ((reconstruct3[level]+reconstruct4[level])==1) komplettieren(map,level-1,merke2);
	          else {tiefe2=map[0][0].name-1; /* sonst konflikt mit naechstem aufruf */
		        /* da es die einzige Flaeche mit >=3 aussenkanten ist, ist sie sowieso
			   kanonisch -- es reicht also: */
		        markiere_orbit(map, merke2, tiefe2, &run2);
			if (run2->name != aussen) 
			  { run2=run2->prev;
			    while (run2->name != aussen){ run2=run2->invers->prev; }
			  }
			do
			  { if (neu==3) add_another(map,run2,reconstruct3[level]-1,reconstruct4[level],level);
			    else add_another(map,run2,reconstruct3[level],reconstruct4[level]-1,level);
			    run2=run2->next->invers->next;
			    while ((run2->name != aussen) && (!(run2->lastedge)[tiefe2])) 
			      run2=run2->invers->next; }
			while (!(run2->lastedge)[tiefe2]);
		      }
	       entferne_polygon(map,run); }
	   run=run->next->invers->next;
	   while ((run->name != aussen) && (!(run->lastedge)[tiefe])) run=run->invers->next; }
       while (!(run->lastedge)[tiefe]);
     }
entferne_ring(map,merke);
}



/***********************KOMPLETTIEREN*******************************/

void komplettieren(PLANMAP map, int level, KANTE *aussenkante)
{

/*fprintf(stderr,"completing %d %d\n",level,constructlevelzaehler);*/

if (level>=0) komplettiermodus=1;

if (level== -1) { if ((constructlevelzaehler < 0) || (compu_maxgap <= maxgap) ||
		      gapok(map,aussenkante,maxgap)) aufschreiben(map); 
		      return; }
else  add_ring(map,aussenkante,level);

if (level==constructlevelzaehler) komplettiermodus=0; /* wieder ausschalten */

/*fprintf(stderr,"finished completing %d\n",level);*/
}




/*********************ADD_POLYGON***********************************/

void add_polygon(int n, PLANMAP map, KANTE *start, KANTE **lastout, int tiefe)
/* fuegt ein weiteres polygon einer Reihe an. Dabei ist n die groesse des polygons. 
   Angefuegt wird immer an start. Die Marke wird nicht versetzt. Ueber lastout wird
   die letzte Aussenkante des Polygons zurueckgegeben. 
   Einige einfache Branchingkriterien werden schon hier angewendet.
*/


{
int new_tempknz, tempknz;
KANTE *ende;
int common_vertices;

/*fprintf(stderr,"start add_polygon: %d->%d\n",start->ursprung, start->name);
schreibemap(map);*/

if (IPR && (n==5))
  {
    if (checksize_right(start->next)==5) { *lastout=nil; return; }
    for (ende=start->next->invers->next, common_vertices=2; ende->name != aussen; 
	  ende=ende->invers->next) { if (checksize_right(ende)==5) { *lastout=nil; return; }
                                     common_vertices++;
				   }
  }
else for (ende=start->next->invers->next, common_vertices=2; ende->name != aussen; 
	  ende=ende->invers->next) common_vertices++;

/*if (dbz>=6790) fprintf(stderr,"common vertices %d\n", common_vertices);*/

tempint[tiefe+1]=tempint[tiefe]+ common_vertices-2;

/* ab jetzt: mindestens 2 aussenkanten */
if ( ((n-1)<=common_vertices) || (map[0][0].name+n-common_vertices > C)
    || (tempint[tiefe+1] > interior))
   { *lastout=nil; return; }
   /* kann eh nicht kanonisch sein (oder sogar gar nicht gehen) -- 
      gar nicht erst etwas einfuegen */

/* es muessen also n-common_vertices knoten hinzugefuegt werden */

tempknz=map[0][0].name;
new_tempknz=tempknz+n-common_vertices;

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

/************************ENTFERNE_POLYGON********************************/

void entferne_polygon(PLANMAP map, KANTE *start)

{
KANTE *run;

start->name=aussen;
run=start->invers->next;
start->invers=nil;

while (run->name==aussen) { (map[0][0].name)--;
			    run=run->next->invers->next; }
run=run->prev;
run->name=aussen; run->invers=nil;
}

/***********************BESTIMME_DARSTELLUNG_INV***************************/

void bestimme_darstellung_inv(PLANMAP map, KANTE *start, GRAPH graph)
/* bestimmt die kanonische Entwicklung in gespiegelter Form von Punkt start 
   und schreibt es in graph 
   Die kanonische Entwicklung ist einfach: start->name muss aussen sein.
   start->ursprung bekommt 1, next bekommt 2 und prev bekommt 3. danach werden
   von urbild(2) an fortlaufend immer neue knoten ohne namen in next richtung 
   vom Vorgaenger mit minimalem bild aus nummeriert. Der "Knoten aussen" bekommt 
   immer bild 0, steht also immer am anfang der Liste, die mit dem kleinsten knoten 
   beginnt und dann in next-Richtung aufgezaehlt wird.
*/
{
KNOTENTYP bild[N+3], urbild[N+3]; /* Platz fuer "aussen" */
int i, minort, minwert, nextnumber, tempknz;
KANTE *kante;

if (start->name != aussen) { fprintf(stderr,"not starting at exterior edge -- error !\n");
			     exit(0); }

tempknz=map[0][0].name;

for (i=1; i<=tempknz; i++) bild[i]=unbelegt;
bild[aussen]=0;

graph[0][0]=tempknz; graph[0][1]=graph[0][2]=0;

bild[start->ursprung]=1; bild[start->next->name]=2; bild[start->prev->name]=3;
urbild[1]=start->ursprung; urbild[2]=start->next->name; urbild[3]=start->prev->name;

graph[1][0]=0; graph[1][1]=2; graph[1][2]=3;

for (i=2, nextnumber=4; i<=tempknz; i++)
    { kante=map[urbild[i]]; minort=0; minwert=bild[kante->name];
      kante=kante->next; /* nur suchrichtung -- muss und darf nicht umgedreht werden */
      if (bild[kante->name]<minwert) { minort=1; minwert=bild[kante->name]; }
      kante=kante->next;
      if (bild[kante->name]<minwert)  minort=2; 

      kante=map[urbild[i]]+minort;
      graph[i][0]=bild[kante->name]; /* die naechsten sind immer ungleich aussen */
      kante=kante->next; if (bild[kante->name]==unbelegt)
	                      { graph[i][1]=bild[kante->name]=nextnumber;
				urbild[nextnumber]=kante->name;
				nextnumber++; }
                         else graph[i][1]=bild[kante->name];
      kante=kante->next; if (bild[kante->name]==unbelegt)
	                      { graph[i][2]=bild[kante->name]=nextnumber;
				urbild[nextnumber]=kante->name;
				nextnumber++; }
                         else graph[i][2]=bild[kante->name];
    }

}





/***********************BESTIMME_DARSTELLUNG***************************/

void bestimme_darstellung(PLANMAP map, KANTE *start, GRAPH graph)
/* bestimmt die kanonische Entwicklung von Punkt start und schreibt es
   in graph 
   Die kanonische Entwicklung ist einfach: start->name muss aussen sein.
   start->ursprung bekommt 1, prev bekommt 2 und next bekommt 3. danach werden
   von urbild(2) an fortlaufend immer neue knoten ohne namen in prev richtung 
   vom Vorgaenger mit minimalem bild aus nummeriert. Der "Knoten aussen" bekommt 
   immer bild 0, steht also immer am anfang der Liste, die mit dem kleinsten knoten 
   beginnt und dann in prev-Richtung aufgezaehlt wird.
*/
{
KNOTENTYP bild[N+3], urbild[N+3]; /* Platz fuer "aussen" */
int i, minort, minwert, nextnumber, tempknz;
KANTE *kante;


if (start->name != aussen) { fprintf(stderr,"not starting at exterior edge -- error !\n");
			     exit(0); }

tempknz=map[0][0].name;

for (i=1; i<=tempknz; i++) bild[i]=unbelegt;
bild[aussen]=0;

graph[0][0]=tempknz; graph[0][1]=graph[0][2]=0;

bild[start->ursprung]=1; bild[start->prev->name]=2; bild[start->next->name]=3;
urbild[1]=start->ursprung; urbild[2]=start->prev->name; urbild[3]=start->next->name;

graph[1][0]=0; graph[1][1]=2; graph[1][2]=3;

for (i=2, nextnumber=4; i<=tempknz; i++)
    { kante=map[urbild[i]]; minort=0; minwert=bild[kante->name];
      kante=kante->next;
      if (bild[kante->name]<minwert) { minort=1; minwert=bild[kante->name]; }
      kante=kante->next;
      if (bild[kante->name]<minwert)  minort=2; 

      kante=map[urbild[i]]+minort;
      graph[i][0]=bild[kante->name]; /* die naechsten sind immer ungleich aussen */
      kante=kante->prev; if (bild[kante->name]==unbelegt)
	                      { graph[i][1]=bild[kante->name]=nextnumber;
				urbild[nextnumber]=kante->name;
				nextnumber++; }
                         else graph[i][1]=bild[kante->name];
      kante=kante->prev; if (bild[kante->name]==unbelegt)
	                      { graph[i][2]=bild[kante->name]=nextnumber;
				urbild[nextnumber]=kante->name;
				nextnumber++; }
                         else graph[i][2]=bild[kante->name];
    }

}

/**************************MEMCMP_OWNINV*************************************/


int memcmp_owninv(adr1,adr2, anzahl)

/* der erste wird hier mit dem ZWEITEN IN UMGEKEHRTER RICHTUNG verglichen */

KNOTENTYP *adr1, *adr2;
int anzahl;

{
for ( ;(*adr1 == *adr2) && (anzahl>1); adr1++, adr2--, anzahl--);
return (((int)*adr1) - ((int)*adr2));
}


/**************************MEMCMP_OWNINV2*************************************/


int memcmp_owninv2(adr1,adr2, anzahl)

/* der ERSTE IN UMGEKEHRTER RICHTUNG wird hier mit dem ZWEITEN IN NORMALER 
   RICHTUNG verglichen */

KNOTENTYP *adr1, *adr2;
int anzahl;

{
for ( ;(*adr1 == *adr2) && (anzahl>1); adr1--, adr2++, anzahl--);
return (((int)*adr1) - ((int)*adr2));
}

/**************************MEMCMP_OWN_BOTHINV*************************************/


int memcmp_own_bothinv(adr1,adr2, anzahl)

/* beide in umgekehrter richtung */

KNOTENTYP *adr1, *adr2;
int anzahl;

{
for ( ;(*adr1 == *adr2) && (anzahl>1); adr1--, adr2--, anzahl--);
return (((int)*adr1) - ((int)*adr2));
}

/**************************MEMCMP_OWN*************************************/


int memcmp_own(adr1,adr2, anzahl)

/* das normale memcmp_own fuer short statt char */

KNOTENTYP *adr1, *adr2;
int anzahl;

{
for ( ;(*adr1 == *adr2) && (anzahl>1); adr1++, adr2++, anzahl--);
return (((int)*adr1) - ((int)*adr2));
}


/********************FACECONNECTED************************************/

/* ueberprueft, ob der Rand eines Flaechenstueckes zusammenhaengend ist */

BOOL faceconnected(PLANMAP map,KANTE *start)

{ KANTE *run, *puffer;
  int zaehler=0;

if (start->name != aussen) { fprintf(stderr,"ERROR -- not starting at exterior edge\n");
			     exit(0); }

start=start->next;

run=start->invers->prev; /* naechste Kante in dem face */

if (!(run->ist_rand)) zaehler=1;

while (run != start)
  { puffer=run->invers->prev; 
    if ((run->ist_rand) && !(puffer->ist_rand)) { zaehler++;
						  if (zaehler==2) { /*schreibemap(map);
								    fprintf(stderr,"start: %d\n",start->ursprung);*/
								    return(0); }  }
    run=puffer;
  }

return(1);
}


/************************KANONIZITAETSTEST******************************/

/* diese Funktion berechnet, ob die letzte angefuegte flaeche kanonisch ist und 
   den Orbit von moeglichen Startkanten auf dem Rand. 

   Eine Flaeche ist kanonisch, wenn 
   1.) Es gibt keine Flaeche, die mehr aussenkanten hat und entfernt werden kann, 
       ohne dass der Patch zerfaellt
   2.) unter allen denen, bei denen 1.) mit der zuletzt eingefuegten uebereinstimmt
       darf es keine geben, mit einer besseren (groesseren) Randsequenz.
   3.) Falls 1.) und 2.) identisch sind, darf es keine geben mit einer kleineren
       Entwicklung wie in "bestimme_darstellung" berechnet.


als optimierung kann Punkt 1 schon bei add_polygon beruecksichtigt werden -- erstmal aber
ohne. Da sind EIN PAAR organisatorische sachen doch noetig....

*/

BOOL kanonizitaetstest(PLANMAP map, KANTE *edge, int tiefe, KANTE **firstedge)

/* tiefe gibt hier an, wieviel flaechen schon drin sind */

{ int i,j,k, dreizaehler, randlaenge;
  KNOTENTYP randsequenz[3*N];
  int shrandsequenz[3*N]; /* die kurzform der randsequenz: ich merke mir nicht 0 fuer aussen, 1
			     fuer innen, sondern: erster eintrag "laenge der ersten aussensequenz",
			     dann "minus laenge der kommenden innensequenz"...
			     Wird nur fuer die Abschaetzung der Moeglichkeit der Vervollstaendigung
			     gebraucht -- nicht fuer die kanonizitaet. Waere vermutlich schneller, 
			     wenn man die auch fuer die vergleiche benutzt -- naja, vielleicht 
			     schreibe ich das ja mal um */
  int shrandlaenge;
  KANTE *mirror_edge, *merke, *last;
  KNOTENTYP *comparestart, *optstart;
  KANTE *run, *orbitkante[N+1];
  GRAPH graph0, graph1;
  int test, orbitlaenge, facelength;
  int min, ende;
  BOOL take_mirror;
  int aussenzaehler, orig_aussenkanten, restfaces;
  int gapc[5]; /* nur die stellen 3 und 4 werden benutzt; */
  int summanden[N], anzahl_summanden; /*die aufsummierung der luecken zwischen dreiern. Mehr als
					2 summanden: Nur noch dreier. zwei -- beide mit wert >2:
					ebenso. Bei einer einer-luecke zwischen 2 dreiern wird der
					3er ignoriert und die luecke als 2er gezaehlt */
  int zaehler, suche;



restfaces=hexagons+pentagons;

if ((map[0][0].name + 2*restfaces) > C) if (!komplettiermodus) return(0);
    /* Jede angefuegte Flaeche muss mindestens einen neuen Knoten enthalten -- sonst kann sie eh
       nicht kanonisch sein -- dieses kriterium erkennt (manchmal), ob der patch zu gross werden wuerde */

/* eine kanonische flaeche muss 2 aufeinander folgende aussenkanten haben. Der patch darf
   aber nicht zerfallen, wenn man sie entfernt. Das heisst, dass von einer kante nur dann
   so gestartet werden kann, dass sie kanonisch ist, wenn vorher oder nachher eine
   zweite aussenkante kommt. Es ist nicht moeglich, dass von einer isolierten kante aus
   die sequenz kanonisch ist und das kriterium "zwei aufeinander folgende randkanten" von
   einer anderen Stelle der flaeche erfuellt wird, da dann der rand nicht zusammenhaengend
   waere (ausserdem sind selbst 6-Ecke dafuer zu klein)*/


graph0[0][0]=graph1[0][0]=0; /* als Erkennungsmarke, dass er noch belegt werden muss */
gapc[3]=gapc[4]=0;

for (i=1; i<= map[0][0].name; i++) for (j=0; j<3; j++) map[i][j].ist_rand=0;
/* kann auch noch optimiert werden durch zaehlerabhaengige marke, so dass nur alle 1000
   Schritte oder so initialisiert werden muss */


/* bei mirror_edge ist graph1 die referenz, sonst (bei edge besser als mirror_edge) graph0 */


/* fuer die Minimalitaet der FLAECHE muss festgestellt werden, von welchem Randpunkt aus
   gesehen und in welcher Richtung sie minimal ist */

for (run=edge->prev->invers->prev, facelength=0; run->name==aussen; 
                          run=run->prev->invers->prev, facelength++)
  { run->kanpossible=run->mirror_kanpossible=0;
    run->next->ist_rand=run->prev->ist_rand=1;
    (run->lastedge)[tiefe]=0; }

mirror_edge=run->next->invers->next;
mirror_edge->mirror_kanpossible=1; 
orig_aussenkanten=facelength+1;
if (orig_aussenkanten >= 3) (gapc[orig_aussenkanten])++;
edge->aussenkanten=mirror_edge->aussenkanten=orig_aussenkanten;
shrandsequenz[0]=orig_aussenkanten;
shrandlaenge=1;

/* Jetzt ist die linke kante gefunden. Name: mirror_edge. sie liegt facelength knoten von der
   rechten kante entfernt */


/* nun wird die randsequenz belegt */


(edge->lastedge)[tiefe]=0;
edge->kanpossible=1; 
edge->mirror_kanpossible=0;
edge->next->ist_rand=edge->prev->ist_rand=1; 
run=edge->next->invers->next; /*insbesondere immer ungleich mirror_edge*/
randsequenz[0]=0;
randlaenge=1;


while (run != mirror_edge)
{   (run->lastedge)[tiefe]=0;
    if (run->name==aussen) /* dann ist es der erste in einer reihe */
      { 
	merke=run; run=run->next->invers->next;
	merke->kanpossible=merke->mirror_kanpossible=0;
	merke->next->ist_rand=merke->prev->ist_rand=1;
	randsequenz[randlaenge]=0; randlaenge++;
	aussenzaehler=1;
	last=merke;
	while (run->name==aussen)
	  { (run->lastedge)[tiefe]=0;
	    run->kanpossible=run->mirror_kanpossible=0;
	    run->next->ist_rand=run->prev->ist_rand=1;
	    last=run;
	    run=run->next->invers->next;
	    aussenzaehler++;
	    randsequenz[randlaenge]=0; randlaenge++;
	  }
	shrandsequenz[shrandlaenge]=aussenzaehler;
	shrandlaenge++;
	if (aussenzaehler >= 3) (gapc[aussenzaehler])++;
	if (aussenzaehler > orig_aussenkanten) return(0);
	                                 /* d.h. aussenzaehler >= 3 -- die dazugehoerige Flaeche
					    kann also auf jeden fall entfernt werden */
	else if (aussenzaehler == orig_aussenkanten) 
	  { merke->mirror_kanpossible=last->kanpossible=1;
	    merke->aussenkanten=last->aussenkanten=aussenzaehler; }
      }
    else /* d.h. es ist eine "innenkante" (die erste einer folge) */
      { 
	dreizaehler=1;
	run->ist_rand=run->prev->ist_rand=1;
	randsequenz[randlaenge]=1; randlaenge++;
	run->kanpossible=run->prev->kanpossible=0; 
	run->mirror_kanpossible=run->prev->mirror_kanpossible=0; 
	run=run->invers->next;
	while (run->name != aussen)
	  { (run->lastedge)[tiefe]=0;
	    dreizaehler++;
	    run->ist_rand=run->prev->ist_rand=1;
	    randsequenz[randlaenge]=1; randlaenge++;
	    run->kanpossible=run->prev->kanpossible=0; 
	    run->mirror_kanpossible=run->prev->mirror_kanpossible=0; 
	    run=run->invers->next;
	  }
	if (dreizaehler>konstr_maxgap) if (!komplettiermodus) return(0); /* >_2_Luecken wuerden nicht mehr geschlossen */
	shrandsequenz[shrandlaenge]= -dreizaehler;
	shrandlaenge++;
      }
  }


/* jetzt noch die aussenkanten von mirror_edge bis edge */

for (i=1; i<=facelength; i++) { randsequenz[randlaenge]=0; randlaenge++; }

if (!komplettiermodus)
{
  if ( (randlaenge + pentagons) > boundarylength) return(0);
  /* das kriterium "mindestens 2 aussenkanten, bewirkt, dass die randlaenge monoton waechst --
     bei einfuegen eines 5-Eckes sogar streng monoton */

  for (i=0; i<shrandlaenge; i++) shrandsequenz[shrandlaenge+i]=shrandsequenz[i];

  if ((gapc[4]>=2) && (tiefe>=3))
    if (pentagons || ((randlaenge + 4*restfaces) > boundarylength) 
	|| ((map[0][0].name + 4*restfaces)) > C) return(0);
  /* sobald ich 2 vierer habe und mindestens 3 flaechen, kommen nur noch vierer, da alle anderen faces
     nicht kanonisch waeren. */
  
  /* jezt die 3er auswerten: */
  
  if (gapc[3]>=2)
    { for (suche=0; shrandsequenz[suche]!=3; suche++); 
      anzahl_summanden=zaehler=0; i=suche+1;
      suche+=shrandlaenge;
      while (i<=suche)
	{
	  while (shrandsequenz[i] != 3)
	    { if (shrandsequenz[i]<0) zaehler += shrandsequenz[i];
	      i++; }
	  if (zaehler == -1) zaehler--;
	  else { summanden[anzahl_summanden]=zaehler;
		 anzahl_summanden++; zaehler=0; }
	  i++;
	}
      
      /* jezt auswerten */
      if (anzahl_summanden>1)
	if ((anzahl_summanden>=3) || ((summanden[0]< -2) && (summanden[1] < -2)))
	  /* nur noch dreier: */
	  if ((randlaenge + 2*hexagons + 3*pentagons) > boundarylength) return(0);
    }
} /* ende if not komplettiermodus */


for (i=0, j=randlaenge, k=2*randlaenge; i<randlaenge; i++, j++, k++)
                                            randsequenz[k]=randsequenz[j]=randsequenz[i];



/* feststellen, ob edge oder mirror_edge besser ist */

test = memcmp_owninv(randsequenz,randsequenz+2*randlaenge-facelength,randlaenge);
if (test<0)
  { optstart=randsequenz+2*randlaenge-facelength; take_mirror=1; }
else if (test > 0)
  { take_mirror=0; }
else /* d.h. test==0 */
  {
    bestimme_darstellung(map, edge, graph0);
    bestimme_darstellung_inv(map, mirror_edge, graph1);
    if (memcmp_own(graph1[1], graph0[1], 3*graph0[0][0])<0) 
      { take_mirror=1; 
	optstart=randsequenz+2*randlaenge-facelength;
      }
    else take_mirror=0;
  }


/* jetzt enthaelt optstart den optimalen startpunkt fuer die sequenz und (eventuell -- d.h. falls
   graph0[0][0]!=0) graph0 die entwicklung von diesem Punkt aus */

/* fuer die Markierung des Bereiches koennte man auch immer bei "edge" starten, da aber aus
   effizienzgruenden die markierung und der kanonizitaetstest in einem gemacht werden, ist die folgende
   Fallunterscheidung noetig: */


if (take_mirror==0)
  {
    /*fprintf(stderr,"non-mirror part\n");*/
    
    /* zunaechst werden nur orientierbare automorphismen ueberprueft (drehungen). Dabei wird ein
       Bereich markiert, auf dem gestartet werden muss: */

    *firstedge=edge; /* sofern keine Spiegelungen das verderben ... */    
    orbitlaenge=0;
    for (i=1, run=edge->next->invers->next; i<randlaenge; i++)
      { 
	if (run->kanpossible)
	  { 
	    if (run->aussenkanten > orig_aussenkanten) test = -1;
	      else test= memcmp_own(randsequenz,randsequenz+i,randlaenge);
	    /* sequenz soll maximal sein */
	    if (test<0) { if (faceconnected(map,run)) return(0); }
	    if (test==0)
	      { 
		if (graph0[0][0]==0) bestimme_darstellung(map, edge, graph0);
		bestimme_darstellung(map, run, graph1);
		test= memcmp_own(graph1[1], graph0[1], 3*graph0[0][0]);
	        /* code soll minimal sein */
		if (test<0) { if (faceconnected(map,run)) return(0); }
		if (test==0)
		  { (run->lastedge)[tiefe]=1; 
		    /* diese kante markiert das ende -- sie muss nicht mehr bearbeitet
		       werden, ausser wenn es die erste ist */
		    /*fprintf(stderr,"set lastedge mark at %d\n", run->ursprung);*/
		    i= infty; 
		  }
	      } /* ende test==0 */
	  } /* end kanpossible */
	if (run->name==aussen) run=run->next->invers->next;
	else run=run->invers->next;
	orbitlaenge++;
      }
    
    if (i < infty) { (edge->lastedge)[tiefe]=1; /* die marke war noch nicht gesetzt */
		      orbitlaenge++; }

    /* Jetzt werden nur Spiegelungen gesucht */
    /* alles in die andere Richtung */
    
    orbitkante[1]=edge;
    comparestart=randsequenz+randlaenge-1;
    min=orbitlaenge+1;
    /* es kann sein, dass die kante auf sich selbst gespiegelt wird -- ist aber ein Spiegelpunkt 
       ausserhalb des Orbits, so ist auch einer innerhalb des Orbits, der dann ausgewertet
       werden kann, es reicht also bis orbitlaenge zu gehen, ausser wenn der erste ein Fixpunkt 
       ist -- aber dann reicht orbitlaenge+1 */
    for (i=2, run=edge->next->invers->next; i<=min; i++)
      /* es wird noch in die normale richtung gelaufen */
      { orbitkante[i]=run;
	if (run->mirror_kanpossible)
	  { test= memcmp_owninv(randsequenz,comparestart+i,randlaenge);
	    /* sequenz soll maximal sein */
	    if (test<0) { if (faceconnected(map,run)) return(0); }
	    if (test==0)
	      { 
		if (graph0[0][0]==0) bestimme_darstellung(map, edge, graph0);
		bestimme_darstellung_inv(map, run, graph1);
		test= memcmp_own(graph1[1], graph0[1], 3*graph0[0][0]);
	        /* code soll minimal sein */
		if (test<0) { if (faceconnected(map,run)) return(0); }
		if (test==0)
		  { 
		    /* es wurde ein spiegelbild des ersten knotens gefunden */
		    /* die Spiegelachse liegt also in der Mitte */
		    /* alles was zwischen startpunkt und spiegelachse liegt
		       wird auf einen aequivalenten Punkt in der zweiten
		       Haelfte des Orbits abgebildet */
		     if (i%2) /* ungerade heisst: es gibt fixpunkt */
			  *firstedge=orbitkante[(i+1)/2];
		        else *firstedge=orbitkante[i/2];
		     /* falls (no_dreh) ist das die einzige Spiegelung und auch lastedge muss
			jetzt gesetzt werden, sonst muss es eine weitere Spiegelung geben, 
			die den Punkt, der das Drehbild des ersten Punktes ist (orbitlaenge+1), 
			in dieses Spiegelbild abbildet. Die Achse muss in der Mitte zwischen 
			diesem Spiegelbild und dem Drehbild liegen -- es kann die gleiche prozedur 
			benutzt werden */
		     
		     if (i%2)
		       { if (orbitlaenge %2) /* d.h. nur ein fixpunkt */
			   ende = (i+orbitlaenge+2)/2;
		       else /* 2 fixpunkte */
			 ende = (i+1+orbitlaenge)/2;
		       }
		     else /* d.h. i gerade */
		       {
			 if (orbitlaenge%2) /* d.h. ein fixpunkt */
			   ende = (i+orbitlaenge+1)/2;
			 else /* garkein fixpunkt */
			   ende = (i+2+orbitlaenge)/2; }
		     while (i<ende) { if (run->name==aussen) run=run->next->invers->next;
		     else run=run->invers->next;
				      i++; }
		     run->lastedge[tiefe]=1;
		     i=infty;
	      } /* ende test==0 */
	    } /* ende test==0 (aeussere Schleife) */
	  } /* end kanpossible */
	if (run->name==aussen) run=run->next->invers->next;
	else run=run->invers->next;
	} /* ende for ueber randknoten */
  } /* ende take_mirror==0 */

else /* d.h. take_mirror==1 */
  {
    
     /*if (dbz==57925) fprintf(stderr,"mirror part %d\n",mirror_edge->ursprung); */

    /* zunaechst werden nur orientierbare automorphismen ueberprueft (drehungen). Dabei wird ein
       Bereich markiert, auf dem gestartet werden muss: */
    
    (mirror_edge->lastedge)[tiefe]=1; 
    orbitlaenge=0;
    for (i=1, run=mirror_edge->prev->invers->prev; i<randlaenge-facelength; i++)
      { 
	if (run->mirror_kanpossible)
	  { 
	    if (run->aussenkanten > orig_aussenkanten) test = -1;
	    else test= memcmp_own_bothinv(optstart,optstart-i,randlaenge);
	    /* sequenz soll maximal sein */
	    if (test<0) { if (faceconnected(map,run)) return(0); }
	    if (test==0)
	      { 
		if (graph1[0][0]==0) bestimme_darstellung_inv(map, mirror_edge, graph1);
		bestimme_darstellung_inv(map, run, graph0);
		test= memcmp_own(graph0[1], graph1[1], 3*graph0[0][0]);
	        /* code soll minimal sein */
		if (test<0) { if (faceconnected(map,run)) return(0); }
		if (test==0)
		  { *firstedge=run;
		    /* diese kante markiert das ende -- sie muss nicht mehr bearbeitet
		       werden, ausser wenn es die erste ist */
		    /*fprintf(stderr,"set lastedge mark at %d\n", run->ursprung);*/
		    i= infty; 
		  }
	      } /* ende test==0 */
	  } /* end kanpossible */
	if (run->name==aussen) run=run->prev->invers->prev;
	else run=run->invers->prev;
	orbitlaenge++;
      }
    
    if (i < infty) { /*if (dbz ==207) fprintf(stderr,"no rotation found\n");*/
                      *firstedge=mirror_edge; /* die marke war noch nicht gesetzt */
		      orbitlaenge += (facelength+1); 
		       }
    
    /* Jetzt werden nur Spiegelungen gesucht */
    /* alles in die andere Richtung */
    /* gelaufen */
    run=mirror_edge;
    orbitkante[1]=mirror_edge;
    comparestart=randsequenz+2*randlaenge-facelength+1;
    min=orbitlaenge+1; 
    for (i=2, run=mirror_edge->prev->invers->prev; i<=min; i++)
      /* es wird noch in spiegelrichtung gelaufen */
      { 
	orbitkante[i]=run;
	if (run->kanpossible)
	  { 
	    test= memcmp_owninv2(optstart,comparestart-i,randlaenge);
	    /* sequenz soll maximal sein */
	    if (test<0) { if (faceconnected(map,run)) return(0); }
	    if (test==0)
	      { if (graph1[0][0]==0) bestimme_darstellung_inv(map, mirror_edge, graph1);
		bestimme_darstellung(map, run, graph0);
		test= memcmp_own(graph0[1], graph1[1], 3*graph0[0][0]);
	        /* code soll minimal sein */
		if (test<0) { if (faceconnected(map,run)) return(0); }
		if (test==0)
		  { /* es wurde ein spiegelbild des ersten knotens gefunden */
		    /* die Spiegelachse liegt also in der Mitte */
			if (i%2) /* ungerade heisst: es gibt fixpunkt -- an dem muss aber nicht mehr
				    angefuegt werden. */
			  { if (orbitkante[(i+1)/2]->name == aussen)
			      (orbitkante[(i+1)/2]->lastedge)[tiefe] = 1;
			  else (orbitkante[(i+1)/2]->next->lastedge)[tiefe] = 1; }
		      else 
			{ if (orbitkante[i/2]->name == aussen)
			    (orbitkante[i/2]->lastedge)[tiefe] = 1; 
			  else (orbitkante[i/2]->next->lastedge)[tiefe] = 1; }
			/* kommentar siehe oben: */
			    if (i%2)
			      { if (orbitlaenge %2) /* d.h. nur ein fixpunkt */
				  ende = (i+2+orbitlaenge)/2;
				else /* 2 fixpunkte */
				  ende = (i+1+orbitlaenge)/2;
			      }
			    else /* d.h. i gerade */
			      {
				if (orbitlaenge%2) /* d.h. ein fixpunkt */
				  ende = (i+orbitlaenge+1)/2;
				else /* garkein fixpunkt */
				  ende = (i+2+orbitlaenge)/2; }
			    while (i<ende) { if (run->name==aussen) run=run->prev->invers->prev;
			                     else run=run->invers->prev;
					     i++; }
			    if (run->name ==aussen) *firstedge=run;
			    else *firstedge=run->next;
			    i=infty;
	      } /* ende test==0 */
	    } /* ende test==0 (aeussere Schleife) */
	  } /* end kanpossible */
	if (run->name==aussen) run=run->prev->invers->prev;
	else run=run->invers->prev;
      }
  } /* ende take_mirror==1 */

return(1);

}



/*************************BAUE_WEITER************************************/

/* Baut an einem patch weiter, indem an der Kante "start" eine neue Flaeche
   angefuegt wird. Dann wird ueberprueft, ob die kanonisch war ... */

void baue_weiter(PLANMAP map, KANTE* start, int tiefe)
/* da am anfang der programmentwicklung eventuell mal KEIN Knoten hinzugefuegt wurde (sondern 
   nur eine Kante), wird tiefe mitgezaehlt. Tiefe gibt immer an, die wievielte Flaeche eingefuegt 
   werden muss -- nicht, wieviel flaechen schon drin sind */

{ KANTE *testedge, *run, *firstedge;
  

/*if (dbz==306) exit(0);
if (strukturzaehler>=2025) {dbz++; fprintf(stderr,"dbz: %d\n",dbz); }*/

/*fprintf(stderr,"start build %d (%d->%d)  hexagons: %d pentagons: %d\n",tiefe,start->ursprung,start->name,hexagons,pentagons);*/
/*schreibemap(map); */


if (hexagons)
  { 
    add_polygon(6,map,start,&testedge,tiefe);
    if (testedge != nil) /* es wurde wirklich etwas eingefuegt */
      { 
	hexagons--; 
	if (kanonizitaetstest(map,testedge, tiefe, &firstedge)) 
	                                    /* weiterbauen da flaeche kanonisch war: */
	  { 
	    if ((hexagons==0) && (pentagons==0) && (map[0][0].name==C)) 
	      komplettieren(map,constructlevelzaehler,testedge);
	  else 
	    if (map[0][0].name < C)
	      { run=firstedge;
		if (run->name != aussen) /* die Kante suchen, ab der die Flaeche eingefuegt wird,
					    die diese ueberstreicht */
		  { run=run->prev;
		    while (run->name != aussen){ run=run->invers->prev; }
		  }
		do
		  { 
		    baue_weiter(map,run,tiefe+1);
		    run=run->next->invers->next;
		    while ((run->name != aussen) && (!(run->lastedge)[tiefe])) run=run->invers->next; }
		while (!(run->lastedge)[tiefe]);
	      }
	  }
	entferne_polygon(map,start);
	hexagons++;
     }
  }


if (pentagons)
  { 
    add_polygon(5,map,start,&testedge,tiefe); 
      if (testedge != nil) /* es wurde wirklich etwas eingefuegt */
	{ pentagons--;
	  if (kanonizitaetstest(map,testedge,tiefe,&firstedge)) 
	                 /* weiterbauen da flaeche kanonisch war: */
	    { if ((hexagons==0) && (pentagons==0) && (map[0][0].name==C)) 
		komplettieren(map,constructlevelzaehler,testedge);
	    else 
	      if (map[0][0].name < C)
		{ run=firstedge;
		if (run->name != aussen) /* die Kante suchen, ab der die Flaeche eingefuegt wird,
					    die diese ueberstreicht */
		  { run=run->prev;
		    while (run->name != aussen){ run=run->invers->prev; }
		  }
		  do
		    { baue_weiter(map,run,tiefe+1);
		      run=run->next->invers->next;
		      while ((run->name != aussen) && (!(run->lastedge)[tiefe])) run=run->invers->next; }
		  while (!(run->lastedge)[tiefe]);
		}
	    }
	  entferne_polygon(map,start);
	  pentagons++;
	}
  }
}



/**************************KONSTRUIERE***********************************/

/* die basisroutine, die die konstruktion beginnt */

void konstruiere(PLANMAP map)
{
KANTE *nextstart;

fprintf(stderr,"Start construction: C: %d H: %d\n", C,H);

if (interior<0) return; /* kann passieren */

komplettiermodus=0;

tempint[1]=0;

if (hexagons)
  {
    baue_polygon(6,map, &nextstart);
    hexagons--;
    if (hexagons || pentagons) baue_weiter(map,nextstart,2);
       else if ((C==6) && (H==6)) komplettieren(map,constructlevelzaehler,nextstart);
    hexagons++;
  }

if (pentagons)
  {
    baue_polygon(5,map, &nextstart);
    pentagons--;
    if (hexagons || pentagons) baue_weiter(map,nextstart,2);
       else if ((C==5) && (H==5)) komplettieren(map,constructlevelzaehler,nextstart);
    pentagons++;
  }

}



/************************MIN_RAND*****************************************/

/* berechnet den minimal moeglichen Rand bei vorgegebener 5-Eck und 6-Eck-Zahl
   unter der Voraussetzung, dass der Spiralalgorithmus, startend mit den 5-Ecken
   einen Patch mit minimaler Randlaenge erzeugt */

int min_rand(int hexagons, int pentagons)
{
int level, faces, gesamtfaces, randlaenge;
/* ursprungskonfiguration der 5-Ecke, bzw eines 6-Eckes: level 0 */

gesamtfaces=hexagons+pentagons;

switch (pentagons)
  {
  case 0: { /* level ist das erste nicht volle level */
	    for (level=faces=1; faces+(6*level) <=gesamtfaces; level++)
	                faces += (6*level);
	    randlaenge=6+12*(level-1);
	    faces= gesamtfaces-faces; /* der rest */
	    if (faces) randlaenge += 2*(faces/level)+2;
	    return(randlaenge);
	  }
  case 1: { /* level ist das erste nicht volle level */
	    for (level=faces=1; faces+(5*level) <=gesamtfaces; level++)
	                faces += (5*level);
	    randlaenge=5+10*(level-1);
	    faces= gesamtfaces-faces; /* der rest */
	    if (faces) randlaenge += 2*(faces/level)+2;
	    return(randlaenge);
	  }
  case 2: { /* level ist das erste nicht volle level */
	    for (level=1, faces=2; faces+2+(4*level) <=gesamtfaces; level++)
	                faces += (4*level+2);
	    randlaenge=8+8*(level-1);
	    faces= gesamtfaces-faces; /* der rest */
	    if (faces)
	      { randlaenge+=2;
		faces -= level; /* der erste verlaengert den rand um 2, die (level-1) danach
				   verlaengern ihn nicht */
		if (faces>0)
		  { randlaenge+=2;
		    faces -= level; /* der erste verlaengert den rand um 2, die (level-1) danach
				       verlaengern ihn nicht */
		    if (faces>0)
		      { randlaenge+=2;
			faces -= (level+1); /* der erste verlaengert den rand um 2, die level (!!) 
					       danach verlaengern ihn nicht */
			if (faces>0) randlaenge += 2;
		      }
		  }
	      }
	    return(randlaenge);
	  }
  case 3: { /* level ist das erste nicht volle level */
	    for (level=1, faces=3; faces+3+(3*level) <=gesamtfaces; level++)
	                faces += (3*level+3);
	    randlaenge=9+6*(level-1);
	    faces= gesamtfaces-faces; /* der rest */
	    if (faces)
	      { randlaenge+=2;
		faces -= level; /* der erste verlaengert den rand um 2, die (level-1) danach
				   verlaengern ihn nicht */
		if (faces>0)
		  { randlaenge+=2;
		    faces -= (level+1); /* der erste verlaengert den rand um 2, die level (!!) 
					   danach verlaengern ihn nicht */
		    if (faces>0) randlaenge += 2;
		  }
	      }
	    return(randlaenge);
	  }
  case 4: { /* level ist das erste nicht volle level */
	    for (level=1, faces=4; faces+4+(2*level) <=gesamtfaces; level++)
	                faces += (2*level+4);
	    randlaenge=10+4*(level-1);
	    faces= gesamtfaces-faces; /* der rest */
	    if (faces)
	      { randlaenge+=2;
		faces -= (level+1); /* der erste verlaengert den rand um 2, die level (!!) 
				       danach verlaengern ihn nicht */
		if (faces>0) randlaenge += 2;
	      }
	    return(randlaenge);
	  }
  case 5: { /* Hier wird ein 6-Eck mit in die Anfangskonfiguration hineingenommen */
            if (hexagons==0) return(11);
            /* level ist das erste nicht volle level */
	    for (level=1, faces=6; faces+5+level <=gesamtfaces; level++)
	                faces += (level+5);
	    randlaenge=11+2*(level-1);
	    faces= gesamtfaces-faces; /* der rest */
	    if (faces) randlaenge += 2;
	    return(randlaenge);
	  }
  default: { fprintf(stderr,"This program is designed for only up to 5 pentagons\n");
	     exit(0); }
	     /* Wobei der returnwert 12 fuer 6 natuerlich leicht ist ... */ 
  }
return(0); /* dummy damit der Compiler nicht meckert -- kommt hier nie an */
}


/***********************ZERLEGBAR***************************************/

BOOL zerlegbar()
/* ueberprueft, ob der rand sich selbst beruehren kann. Wenn ja, dann muss
   ein flaschenhals existieren aus maximal 2 hexagonen, an dem der Patch
   geteilt werden kann -- erinnerung: im rand sind nur 6-Ecke -- sonst
   wird die funktion garnicht aufgerufen */
{
int pent1, pent2; /* fuenfecke in teil 1 und 2 */
int hex1, hex2, starthex1, endhex2;

/* In teil 1 sollen obda mindestens so viel fuenfecke sein, wie in Teil 2 */
/* Beim zerlegen wird die Bruecke verdoppelt und in beiden Teilen gezaehlt 
   fuer zweierbruecken folgt min1+min2 <= rand+10
   fuer einerbruecken min1+min2 <= rand+6 -- daraus folgt, falls ich in beiden Teilen
   ein hexagon hinzutue: min1+min2 <= rand+10*/

if (hexagons<10) return(0);

for (pent1=pentagons, pent2=0; pent1 >= pent2; pent1--, pent2++)
  { switch(pent1)
	  { case 0: { starthex1=7; break; }
	    case 1: { starthex1=5; break; }
	    case 2: { starthex1=6; break; }
	    case 3: { starthex1=6; break; }
	    case 4: { starthex1=6; break; }
	    case 5: { starthex1=6; break; }
	  default: { fprintf(stderr,"ERROR in function zerlegbar\n"); exit(0); }
	  }
    switch(pent2)
	  { case 0: { endhex2=7; break; }
	    case 1: { endhex2=5; break; }
	    case 2: { endhex2=6; break; }
	    case 3: { endhex2=6; break; }
	    case 4: { endhex2=6; break; }
	    case 5: { endhex2=6; break; }
	  default: { fprintf(stderr,"ERROR in function zerlegbar\n"); exit(0); }
	  }

    for (hex1=starthex1, hex2=hexagons-starthex1+2; hex2>=endhex2; hex1++, hex2--)
     { if (min_rand(hex1,pent1)+min_rand(hex2,pent2) <= (boundarylength+10)) return(1);
        }
    /* die patches koennten EVENTUELL so zusammengefuegt werden, dass die randlaenge passt */
  }

return(0);
}


/*************************BERECHNE_AUFGABEN*******************************/

/* ueberprueft, ob 6-Eck Schalen entfernt werden koennen und ob eventuell
   noch 3-er angefuegt werden muessen */

void berechne_aufgaben(int level)
{
static PLANMAP map;
int C_alt0, H_alt0, boundarylength_alt0, interior_alt0, hexagons_alt0, konstr_maxgap_alt0;
int C_alt, H_alt, boundarylength_alt, interior_alt, hexagons_alt, konstr_maxgap_alt;
int i, anzahldreier,anzahlvierer;

if ((((pentagons==0) && boundarylength < 4+(2*hexagons)) 
                || ((pentagons >0) && (min_rand(hexagons+1,pentagons-1) > boundarylength+1))) &&
     (!zerlegbar()) )
  { 
    /* 1.+2.Zeile: Es kann kein 5-Eck im Rand liegen und falls es nur hexagone gibt, koennen die
       nicht nur durch >=3er aufgebaut werden. 
       3.Zeile: Der Fall, dass die Sequenz der aeusseren 6-Ecke sich selbst beruehrt und 
       einen Flaschenhals aus 1 oder 2 6-Ecken bildet, wird hiermit abgefangen. */
  for (anzahlvierer=1;
       (min_rand(hexagons-anzahlvierer,pentagons) <= boundarylength-(4*anzahlvierer));
       anzahlvierer++); 
  anzahlvierer--; /* Berechnung der maximalzahl der 4er im Rand */
  while (anzahlvierer>=0)
    {
      reconstruct4[level]=anzahlvierer; 
      C_alt0=C; H_alt0=H;  boundarylength_alt0=boundarylength; interior_alt0=interior;
      hexagons_alt0=hexagons; konstr_maxgap_alt0=konstr_maxgap; 
      C -= (4*anzahlvierer);
      H -= (2*anzahlvierer);
      boundarylength= 2*H-6+pentagons;
      interior = C - boundarylength;
      hexagons= (C-(2*pentagons)-H+2)/2;
      for (anzahldreier=1;
	   (min_rand(hexagons-anzahldreier,pentagons) <= boundarylength-(2*anzahldreier));
	   anzahldreier++);
      anzahldreier--; /* Berechnung der maximalzahl der 3er im Rand */
      while (anzahldreier>=0)
	{ 
	  reconstruct3[level]=anzahldreier; 
	  C_alt=C; H_alt=H;  boundarylength_alt=boundarylength; interior_alt=interior;
	  hexagons_alt=hexagons; konstr_maxgap_alt=konstr_maxgap; 
	  C -= (boundarylength+anzahldreier);
	  H = boundarylength - H -anzahldreier;
	  boundarylength= 2*H-6+pentagons;
	  interior = C - boundarylength;
	  hexagons= (C-(2*pentagons)-H+2)/2;
	  konstr_maxgap=2;
	  berechne_aufgaben(level+1);
	  C=C_alt; H=H_alt;  boundarylength=boundarylength_alt; interior=interior_alt;
	  hexagons=hexagons_alt;
	  konstr_maxgap=konstr_maxgap_alt;
	  anzahldreier--;
	}
      C=C_alt0; H=H_alt0;  boundarylength=boundarylength_alt0; interior=interior_alt0;
	  hexagons=hexagons_alt0;
	  konstr_maxgap=konstr_maxgap_alt0;
	  anzahlvierer--;
    }
}

else
  { konstr_maxgap_alt=konstr_maxgap; 
    if ((min_rand(hexagons+1,pentagons) > boundarylength-4) && (konstr_maxgap>3)) konstr_maxgap=3;
    /* wenn ich in eine 4er luecke ein hexagon einfuege erhalte ich einen kuerzeren rand 
       -- wenn das nicht moeglich ist, gibt es keine 4er Luecken */
    if ((min_rand(hexagons+1,pentagons) > boundarylength-2) && (konstr_maxgap>2)) konstr_maxgap=2;
    /* analog oben */
    constructlevelzaehler=level-1;

fprintf(stderr,"So far: %d structures\ntask:\n",strukturzaehler);
for (i=0; i<=constructlevelzaehler; i++) fprintf(stderr,"v:%d d:%d",reconstruct4[i], reconstruct3[i]);
fprintf(stderr,"\n");

    konstruiere(map);
    konstr_maxgap=konstr_maxgap_alt;
  }
}




/**************************GAP1***********************************/

/* speziell fuer den trivialen Sonderfall gap 1 */

void gap1(int pentagons,int hexagons)
{
PLANMAP map;
KANTE *nextstart;
int local_pentagons, local_hexagons;

fprintf(stderr,"Start construction: C: %d H: %d\n", C,H);

local_pentagons=pentagons;
local_hexagons=hexagons;

if (local_pentagons > 2) { fprintf(stderr,"ERROR!\n"); exit(12); }

if (local_pentagons)
  {
    baue_polygon(5,map, &nextstart);
    local_pentagons--;
    if ((C==5) && (H==5)) { aufschreiben(map);  return; }
  }
else
  if (local_hexagons)
  {
    baue_polygon(6,map, &nextstart);
    local_hexagons--;
    if ((C==6) && (H==6)) { aufschreiben(map);  return; }
  }

while (local_hexagons)
  { add_polygon(6,map,nextstart, &nextstart,0);
    local_hexagons--;
    nextstart=nextstart->prev->invers->next->invers->prev;
  }

if(local_pentagons)
  { add_polygon(5,map,nextstart, &nextstart,0);
    local_pentagons--;
  }

aufschreiben(map);  
return;   

}



/**************************MAIN*******************************************/

int main(argc,argv)

int argc; char *argv[];

{ int i;
#ifndef NOTIMES
  struct tms TMS;
#endif //NOTIMES
  char outfilename[filenamenlaenge], logfilename[filenamenlaenge];
  FILE *logfile = NULL;

if (argc<4) exit(0);

C = atoi(argv[1]);
H = atoi(argv[2]);
pentagons=atoi(argv[3]);


/* Die Anzahl der Knoten mit ungerader Valenz muss gerade sein */
if ((C-H) %2) { fprintf(stderr,"C-H must be even -- otherwise no structures can exist.\n"); 
		exit(0); }
if (H<5) { fprintf(stderr,"H must be at least 5.\n"); exit(0); }


for (i=4; i<argc; i++)
    switch (argv[i][0])
      {
      case 'i': { if (strcmp(argv[i],"ipr")==0) IPR=1;
		    else { fprintf(stderr,"Nonidentified option: %s\n",argv[i]); exit(0); }
		  break; }
      case 'g': { if (strcmp(argv[i],"gap")==0) { i++; konstr_maxgap=maxgap=atoi(argv[i]);
						  if (maxgap<1) 
						    { fprintf(stderr,"maxgap must be at least 1\n");
						      exit(0); } 
						  if ((maxgap==1) && (pentagons>=3))
						    { fprintf(stderr,"No structures possible !\n");
						      exit(0); } 
						}
		    else { fprintf(stderr,"Nonidentified option: %s\n",argv[i]); exit(0); }
		  break; }
      case 'l': { if (strcmp(argv[i],"logerr")==0) logfile = stderr;
		  break; }
      case 'n': { if (strcmp(argv[i],"noout")==0) noout=1;
		    else { fprintf(stderr,"Nonidentified option: %s\n",argv[i]); exit(0); }
		  break; }
      case 'w': { if (strcmp(argv[i],"without_H")==0) without_H=1;
		    else { fprintf(stderr,"Nonidentified option: %s\n",argv[i]); exit(0); }
		  break; }
      case 's': { if (strcmp(argv[i],"stdout")==0) to_stdout=1;
		    else { fprintf(stderr,"Nonidentified option: %s\n",argv[i]); exit(0); }
		  break; }
      case 'p': { if (strcmp(argv[i],"peri_condensed")==0) peri_condensed=1;
                  else if (strcmp(argv[i],"pid")==0) 
                    {fprintf(stdout,"%d\n",getpid());  fflush(stdout);}
		    else { fprintf(stderr,"Nonidentified option: %s\n",argv[i]); exit(0); }
		  break; }
	       /* filtere die raus, die strictly peri-condensed sind */

      default: { fprintf(stderr,"Nonidentified option: %s\n",argv[i]); exit(0); }
      }

if (pentagons > 5) { fprintf(stderr,"The number of pentagons may be at most 5.\n"); exit(0); }
  /* keine PRINZIPIELLEN Gruende. Nur dass sonst der Rand zu klein werden kann. Eine
     genauere Analyse fuer 7... ist noetig, aber auch fuer groessere 5-Eck-Zahlen ist
     das Programm sicherlich anpassbar */
  /* von 6 auf 5 geaendert am 20.4., da ab jetzt die Konstruktion so geaendert wird, dass
     immer mindestens 2 neue Knoten gebaut werden -- und das geht nicht fuer 6 Pentagons */

if (((C+H > N) && !(noout || without_H)) || (C>N))
  { fprintf(stderr,"Constant N too small\n"); exit(0); }


boundarylength= 2*H-6+pentagons;
interior = C - boundarylength;
hexagons= (C-(2*pentagons)-H+2)/2;

if (interior<0) { fprintf(stderr,"Too much H or too few C -- not possible.\n"); exit(0); }

if (min_rand(hexagons,pentagons) > boundarylength) 
  { /*fprintf(stderr," %d\n", min_rand(hexagons,pentagons));*/
    fprintf(stderr,"Boundary too short -- no structures exist.\n"); 
		exit(0); }


if (C>=N) { fprintf(stderr,"The constant \"N\" is too small !\n");
	    exit(0); }


sprintf(outfilename,"c%sh%s_%spent",argv[1],argv[2],argv[3]);
if (IPR) strcat(outfilename,"_ipr");
if (peri_condensed) strcat(outfilename,"_pc");

if (logfile == NULL) {
  strcpy(logfilename,outfilename);
  strcat(logfilename,".log");

  logfile=fopen(logfilename,"w");
}

if (without_H) strcat(outfilename,"_noH");

if (to_stdout) outfile=stdout;
  else  if (!noout) outfile=fopen(outfilename,"wb");



if (min_rand(hexagons+1,pentagons) > boundarylength-4) compu_maxgap=3;
/* wenn ich in eine 4er luecke ein hexagon einfuege erhalte ich einen kuerzeren rand 
   -- wenn das nicht moeglich ist, gibt es keine 4er Luecken. Noch zu beweisen: auch keine
   groesseren (plausibel -- aber das reicht nicht) */
if (min_rand(hexagons+1,pentagons) > boundarylength-2) compu_maxgap=2;
/* analog oben */




fprintf(stderr,"C: %d\tH: %d\thexagons: %d\tpentagons: %d\n",C,H,hexagons,pentagons);
fprintf(stderr,"boundary: %d\tinterior vertices: %d\n",boundarylength,interior);

if (fileno (logfile) != fileno (stderr)) {
  fprintf(logfile,"C: %d\tH: %d\thexagons: %d\tpentagons: %d\n",C,H,hexagons,pentagons);
  fprintf(logfile,"boundary: %d\tinterior vertices: %d\n",boundarylength,interior);
}
fprintf(logfile,"IPR: %d\tperi_condensed: %d\twithout_H: %d\n",IPR,peri_condensed,without_H);
fprintf(logfile,"noout: %d\tstdout: %d\n",noout,to_stdout);



if (maxgap == 1) gap1(pentagons,hexagons);
  else berechne_aufgaben(0);


fprintf(stderr,"%d structures\n",strukturzaehler);
fprintf(logfile,"%d structures\n",strukturzaehler);

#ifndef NOTIMES
times(&TMS);
fprintf(stderr,"\nTotal generation time: %.1f seconds\n",(double)TMS.tms_utime/time_factor);
fprintf(logfile,"\nTotal generation time: %.1f seconds\n",(double)TMS.tms_utime/time_factor);
#endif //NOTIMES

return(0);
}



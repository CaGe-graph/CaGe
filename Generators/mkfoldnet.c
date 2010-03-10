
/* Aenderung von Gunnar Brinkmann eingebaut: Wenn es mit -DNOTIMES kompiliert wird, werden die
auf Laufzeit basierenden Funktionen (randomisierte Wahl eines anderen Teilbaumes fuer das Ausschneiden)
auf Basis der Schrittzahlen gewaehlt. Die Schrittzahlen sind nicht optimiert -- einfach geraten und
das sollte nicht verwendet werden -- die Absicht ist einzig und allein, es auch auf Windows zum
Laufen zu bekommen */

#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>

#ifndef NOTIMES
#include<time.h>
#endif //NOTIMES

#define FlaechenGroesse 50
/* FlaechenGroesse-1 ist die maximal erlaubte Anzahl der Knoten in einer 
   Flaeche */

#define Max_Flaechenzahl 401
/* Max_Flaechenzahl-1 ist die maximal erlaubte Anzahl der Flaechen in einem 
   Graphen-Aufschnitt */

#define ENTFERNUNGSFAKTOR 1.0/20.0
/* ENTFERNUNGSFAKTOR  bestimmt, wie stark ein Knoten von einer Flaeche 
   entfernt sein darf, um noch als "in dieser Flaeche" zu gelten. Je groesser
   der ENTFERNUNGSFAKTOR, desto eher wird der Knoten als "in dieser Flaeche" 
   anerkannt */

#define ZEITLIMIT 20.0
/* Wird ZEITLIMIT (in Sekunden) fuer die Laufzeit der Funktion 
   einbettung_aufschnitt(...) ueberschritten, so wird der Zufallsgenerator 
   aktiv und damit wird ein Teil der Aufschnitte verworfen */
#define Wkeit 25
/* mit der Wahrscheinlichkeit "Wkeit" (in %) wird der aktuelle Teilbaum 
   verworfen, wenn die Funktion einbettung_aufschnitt(...) ihr Zeitlimit 
   ueberschritten hat*/

#define LISTENLAENGE 300

#define mKZdF FlaechenGroesse*2 

/*#define FAKTOR1 70.0*/
   
#define FAKTOR1 30.0
/*Je kleiner FAKTOR1, desto wieter muessen nicht benachbarte Kanten 
  voneinander entfernt sein */
#define FAKTOR1_h2 900.0
/*muss gelten(!!!): FAKTOR1_h2=FAKTOR1*FAKTOR1*/

#define FAKTOR2 50.0
/* Je groesser FAKTOR1, desto naeher muessen urspruenglich gleiche Knoten 
   zueinander liegen */
#define FAKTOR2_h2 2500.0
/* muss gelten(!!!): FAKTOR2_h2=FAKTOR2*FAKTOR2*/

/************************ Typ-Definitionen **********************************/

typedef double (*koordinaten_3d)[3];
typedef double (*koordinaten_2d)[2];

typedef int SURFACE[FlaechenGroesse];   /*Eintraege (Knoten der Flaeche) sind 
				      in fl[] im Uhrzeigersinn augelistet*/
typedef struct Edge{
  int start;
  int end;
  struct Edge *prev;
  struct Edge *next;
  struct Edge *invers;
} EDGE;


typedef struct Edge_d{
  int start;
  int end;
  struct Edge_d *prev;
  struct Edge_d *next;
  struct Edge_d *invers;
  struct Edge *connect; /* Zeigt auf die Kante, die zw. end und start leigt; 
			   mit kante->start=surfaces[start][i] und 
			   kante->end=surfaces[start][i+1] fuer irgendein i. 
			   (bzw. kante->end=surfaces[end][j] und 
			   kante->start=surfaces[end][j+1])  */ 
  int start_con[2]; /* start_con[0]=i mit 
		       flaechen[start][i]==kante->connect->start
		       und start_con[1]=j mit 
		       flaechen[start][j]==kante->connect->end
		       (i. d. R. j==i+1)*/
  int end_con[2];  /* end_con[0]=k mit 
		      flaechen[end][k]==kante->connect->end
		      und end_con[1]=l mit 
		      flaechen[end][l]==kante->connect->start
		      (i. d. R. k==l+1)*/

} EDGE_DUAL;

typedef struct Kantenliste_D { /*Verkettete Liste von Vektoren 
				 (vom Typ EDGE_DUAL)*/
  EDGE_DUAL liste[LISTENLAENGE];
  struct Kantenliste_D *next;
} KANTENLISTE_DUAL;

typedef struct Edge_add{
  int start;
  int end;
  int add;               /* edge->add=0, wenn die Kante nicht auf dem Rand 
			    liegt;
			    edge->add=1, wenn auf dem Rand und edge->prev
			    auf dem Rand;
			    edge->add=2, wenn auf dem Rand und edge->next
			    auf dem Rand;*/
  struct Edge_add *prev;
  struct Edge_add *next;
  struct Edge_add *invers;
} EDGE_Add;


/************************ Globale Variablen *********************************/

int MAX_Flaechenzahl=0; 

int Knotenzahl;
int Kantenzahl;
EDGE **firstedge;             /* wird gleich *map[] gesetzt*/
EDGE_Add ***triangulierungen; /* hier werden spaedter Triangulierungen 
			         abgespeichert; triangulierungen[i][j] ist
			         ein Zeiger auf eine Kante i-ter Flaeche,
			         die von dem Knoten flaechen[i][j] (Typ 
				 *SURFACE) ausgeht */

int print_header;
char *pagelabel, *pageno;


/* --- include lese_vega2_fert.c ---------------------------------------- */
/*#include<stdio.h>
  #include<stdlib.h>
  #include<ctype.h>*/

/*#define LISTENLAENGE 100*/
int MAX_Knotenzahl=0;

typedef struct Kantenliste {  /*Verkettete Liste von Vektoren (vom Typ EDGE)*/
  EDGE liste[LISTENLAENGE];
  struct Kantenliste *next;
} KANTENLISTE;


int alloziere_kantenliste ( KANTENLISTE **aktuelle_kantenliste, int neu){
  
  /* Diese Funktion alloziert eine neue KANTENLISTE (falls noetig), oder
     aktualisiert bereits vorhandene (falls lese_vega schon mal aufgeruffen
     wurde .
     Rueckgabewert: 1, wenn der Speicher alloziert wurde b.z.w. beriets zur
                       Verfuegung steht;
                    2, wenn der Speicher reicht nicht mehr aus.*/

  if(neu){
   if((*aktuelle_kantenliste=(KANTENLISTE*)malloc(sizeof(KANTENLISTE)))==NULL){
     fprintf(stderr," Speicherplatz reicht nicht aus(l.v.3)\n");
     return 2;
   }
   (*aktuelle_kantenliste)->next=NULL;
  }
  else{
    if( ((*aktuelle_kantenliste)->next)==NULL ){
      if(((*aktuelle_kantenliste)->next=(KANTENLISTE*)malloc(sizeof(KANTENLISTE)))==NULL){
	fprintf(stderr," Speicherplatz reicht nicht aus (l.v.4)\n");
	return 2;
      }
      (*aktuelle_kantenliste)->next->next=NULL;
    }
    *aktuelle_kantenliste=(*aktuelle_kantenliste)->next;
  }
  return 1;
}


int lese_den_rest_weg(FILE *fil, int  kleinigkeit){

  /*Rueckgabewert: 1, wenn der Eingabekopf erfolgreich eingelesen wurde,
                       oder es gibt ihn nicht;
                   2, wenn dieser nicht akzeptiert werden konnte;
		   EOF, wenn EOF.*/

  int test, j;
  int x; 
  while(isspace(x=getc(fil))); 
  if(x==EOF){
    return EOF;
  }
  if((x=='<') && ((x=getc(fil))=='<'))
    return 1;
  if(x==EOF){
    return EOF;
  }
  else{
    for(j=0, test=1; (j<120) && test; j++)
      if(((x=getc(fil))=='<') && ((x=getc(fil))=='<')){
	test=0;
      }
    if(test){
      fprintf(stderr, "Eingabefehler: der Eingabekopf ist enweder zu lang, oder weicht stark vom Standart ab\n");
      return 2;
    }
    fprintf(stderr,"Der Eingabekopf weicht vom Standart ab, trotztdem versuche die Daten auszuwerten\n");
    return 1;
  }
  
  if(kleinigkeit)
    fprintf(stderr,"Der Eingabekopf weicht vom Standart ab, trotztdem versuche die Daten auszuwerten\n");
  return 1;
}



int lese_kopf_weg(FILE *fil, int *Eingabedimension){

  /* Diese Funktion liest den Kopf der Eingabe (falls vorhanden).
     Jeder Kopf von der Form >> ... << wird akzeptiert (aber er muss nicht zu
     lang sein). Danach (falls der Kopf ueberhaupt vorhanden ist) muss Vega-
     Code folgen. Alles andere wird als Eingabefehler betrachtet.
     Als Stndart ist der Kopf >>writegraph3d planar<< vorgesehen (whitespaces
     spielen keine Rolle).
     Rueckgabewert: 1, wenn der Eingabekopf erfolgreich eingelesen wurde,
                       oder es gibt ihn nicht;
                    2, wenn dieser nicht akzeptiert werden konnte;
		    EOF, wenn EOF.*/

  char kopfmuster[18]="writegraph3dplanar";
  int x;
  int i, test, indikator;
  int save_dim;
  
  save_dim=*Eingabedimension;

  while(isspace(x=getc(fil)));
  if(x==EOF){
    return EOF;
  }
  
  
  if(x=='>' && ((x=getc(fil))=='>')){
    if(x==EOF){
      return EOF;
    }
    for(test=1, i=0; test && (i<18); i++){
      while(isspace(x=getc(fil)));
      if(x==EOF){
	return EOF;
      }
      
      if(x!=kopfmuster[i]){
	if(!(kopfmuster[i]=='3' && x=='2'))
	  test=0;
	else
	  *Eingabedimension=2;
      }
      else{
	if(x=='3')
	  *Eingabedimension=3; 
      }
    }
    if(test){
      if((indikator=lese_den_rest_weg(fil, 0))!=1){
	if(indikator==2){
	  return 2;
	}
	else{
	  return EOF;
	}
      }
    }
    else{
      *Eingabedimension=save_dim;
      if((indikator=lese_den_rest_weg(fil, 1))!=1){
	if(indikator==2){
	  return 2;
	}
	else{
	  return EOF;
	}
      }
    } 
  }
  if(x==EOF){
    return EOF;
  }
  
  if((x>='0') && (x<='9')){
    if(ungetc(x, fil)==EOF){
      fprintf(stderr," Fehler aufgetretten\n");
      return EOF;
    }
    return 1;
  }
  
  while(isspace(x=getc(fil)));
  if(x==EOF){
    return EOF;
  }
  if((x>='0') && (x<='9')){
    if(ungetc(x, fil)==EOF){
      fprintf(stderr," Fehler aufgetretten\n");
      return EOF;
    }
    return 1;
  }
  else{ 
    fprintf(stderr, "Eingabefehler5: die Eingabe ist nicht vom Vega-Format\n");
    return 2;
  }
}



int lese_vega(FILE *fil, EDGE ***map, int *knotenzahl, koordinaten_3d *koord){

  /* An die Stelle *knotenzahl wird Knotenzahl geschrieben.
     An die Stelle **map[] wird ein Zeiger auf ein Vektor von Zeigern auf EDGE
     geschrieben,derart dass map[i] ein Zeiger auf eine Kante sein wird, mit 
     der Knoten i inzident ist. Durch map[] erhaelt man auch ein Zugriff auf 
     die restlichen Kanten eines jeden Knoten i.
     Achtung: Soll die Funktion mehrmalls in einem Programm aufgerufen werden,
     so darf nicht der von der Funktion reservierte Speicherbereich ( map[] 
     und restliche Knoten)freigemacht werden !!! (da die Funktion diesen bei 
     weiteren Aufrufen benutzen wird).
     Rueckgabewert: 1, wenn der Graph erfolgreich eingelesen wurde;
		    EOF, wenn EOF, auch wenn der Vega-Code vom Standart ab-
		       weicht (EOF an falscher Stelle auftritt), dafuer wird 
		       es aber eine Fehlermeldung geben. */

  int i, j, start, ende, test, indikator;
  int k1_1, k1_2, k2_1, k2_2;
  char x;
  EDGE *kante;
  EDGE *kante2;
  EDGE **map_neu;
  float krd;

  static KANTENLISTE *Anfang;
  KANTENLISTE *aktuelle_kantenliste;
  int kantennummer=0;
  static int Eingabedimension=3;
  koordinaten_3d koord_neu;

  if((indikator=lese_kopf_weg(fil, &Eingabedimension))!=1){
    if(indikator==2)
      exit(2);
    else
      return EOF;
  }

  if(Eingabedimension!=3){
    fprintf(stderr, " Eingabefehler: Eingabe muss dreidimensional sein!\n");
    exit(3);
  }
  
  if(!MAX_Knotenzahl){  
    /* hier wird beim ersten Funktionsaufruf Speicher fuer
       map[] alloziert und Zeiger auf die erste KANTENLISTE gesetzt*/

    MAX_Knotenzahl=20;
    if((*map=(EDGE**)malloc(MAX_Knotenzahl*sizeof(EDGE*)))==NULL || (*koord=(koordinaten_3d)malloc((MAX_Knotenzahl)*sizeof(double)*3))==NULL){
      fprintf(stderr, " Speicher reicht nicht aus (lese_vega 1)");
      exit(2);
    }
    if((alloziere_kantenliste(&Anfang, 1))!=1)
      exit(2);
  }

  aktuelle_kantenliste=Anfang;
  
  if((fscanf(fil,"%d",&start))==EOF){
    /*fprintf(stderr,"Eingabefehler: Eingabe ist nicht vom Vega-Format\n");*/
    return EOF;
  }

  /* hier wird kante fuer map[0] gesetzt*/
  kante=(*map)[0]=aktuelle_kantenliste->liste+kantennummer;
  kantennummer=kantennummer+1;
  kante->start=kante->end=0;
  kante->prev=kante->next=kante->invers=kante;

  Kantenzahl=-1;
  
  for(i=0; start!=0; i++){

    if(start>=MAX_Knotenzahl){

      if((map_neu=(EDGE**)malloc((start+100)*sizeof(EDGE*)))==NULL || (koord_neu=(koordinaten_3d)malloc((start+100)*sizeof(double)*3))==NULL){
	fprintf(stderr," Speicherplatz reicht nicht aus(l.v.1)\n");
	fprintf(stderr," start %d\n", start);

	exit(2);
      }
      memcpy(map_neu, *map, (size_t)MAX_Knotenzahl*sizeof(EDGE*));
      memcpy(koord_neu, *koord, (size_t)MAX_Knotenzahl*sizeof(double)*3);
      MAX_Knotenzahl=start+100;
      free(*map);
      free(*koord);
      *koord=koord_neu;
      *map=map_neu;
    }


    for(j=0; j<Eingabedimension; j++){
      /*  hier werden Koordinaten des Knotens 'start' weggelesen  */

      if((fscanf(fil,"%f",&krd))==EOF){
	fprintf(stderr,"Eingabefehler1: Eingabe ist nicht vom Vega-Format\n");
	return EOF;
      }
      (*koord)[start][j]=krd;
    }    

    /* hier werden Kanten, die von dem Knoten 'start' ausgehen, gebildet und 
       Zeiger auf die vorige und die naechste Kanten gesetzt (inverse nicht)*/
    
    for(test=1; (x=getc(fil))!='\n'; ){
      if((x>='0')&&(x<='9')){
	if(ungetc(x, fil)==EOF){
	  fprintf(stderr," Fehler aufgetretten\n");
	  return EOF;
	}
	if((fscanf(fil, "%d", &ende))==EOF){
	  fprintf(stderr,"Eingabefehler2: Eingabe ist nicht vom Vega-Format\n");
	  return EOF;
	}
	if(ende>=MAX_Knotenzahl){
	  
	  if((map_neu=(EDGE**)malloc((ende+100)*sizeof(EDGE*)))==NULL || (koord_neu=(koordinaten_3d)malloc((ende+100)*sizeof(double)*3))==NULL){
	    fprintf(stderr," Speicherplatz reicht nicht aus(l.v.2)\n");
	    fprintf(stderr," start %d\n", ende);

	    exit(2);
	  }
	  
	  memcpy(map_neu, *map, (size_t)MAX_Knotenzahl*sizeof(EDGE*));
	  memcpy(koord_neu, *koord, (size_t)MAX_Knotenzahl*sizeof(double)*3);

	  MAX_Knotenzahl=ende+100;
	  free(*map);
	  free(*koord);
	  *koord=koord_neu;
	  *map=map_neu;
	}

	if(kantennummer==LISTENLAENGE){
	  if((alloziere_kantenliste(&aktuelle_kantenliste, 0))!=1)
	    exit(2);
	  kantennummer=0;
	  Kantenzahl=Kantenzahl+LISTENLAENGE;
	}

	if(test){
	  kante=kante2=(*map)[start]=aktuelle_kantenliste->liste+kantennummer;
	  kantennummer=kantennummer+1;
	  test=0;
	}
	else{
	  kante=aktuelle_kantenliste->liste+kantennummer;
	  kantennummer=kantennummer+1;
	}

	kante->start=start;
	kante->end=ende;
      	kante2->next=kante;
	kante->prev=kante2;
	kante2=kante;

      }
    }

    (*map)[start]->prev=kante2;
    kante2->next=(*map)[start];

    if((fscanf(fil, "%d", &start))==EOF){	
      fprintf(stderr,"Eingabefehler3: Eingabe ist nicht vom Vega-Format \n");
      return EOF;
    }
     
  }

  *knotenzahl=i;   /* Knotenzahl glech der Anzahl der eingelesenen Knoten*/

  Knotenzahl=i;
  Kantenzahl=Kantenzahl+kantennummer;
  firstedge=*map;

  if((*knotenzahl)>=MAX_Knotenzahl){
    fprintf(stderr, "knotenzahl>=MAX_Knotenzahl\n");
    exit(4);
  }
    
    
  
  /* hier werden inversen Kanten gekoppelt und ueberprueft, ob solche
     existieren. Wenn nicht - Fehlermeldung */
  for(i=1; i<=(*knotenzahl); i++){
    kante=(*map)[i];
    do{
      k1_1=kante->start;
      k1_2=kante->end;
 
      if(k1_2>k1_1){
	
	test=1;
	for(j=1; j<=(*knotenzahl) && test; j++){
	  if(i!=j){
	    kante2=(*map)[j];
	    do{
	      k2_1=kante2->start;
	      k2_2=kante2->end;
	      if(k1_1==k2_2 && k1_2==k2_1){
		kante->invers=kante2;
		kante2->invers=kante;
		test=0;
		break;
	      }
	      kante2=kante2->next;
	    }
	    while(kante2!=(*map)[j]);
	  }
	}
	if(test){
	  fprintf(stderr,"Eingabefehler (inputerror): Kante %d->%d existiert, aber %d->%d nicht\n", k1_1, k1_2, k1_2, k1_1);
	  exit(3);
	}	
      }
      kante=kante->next;
    }
    while(kante!=(*map)[i]);
  }

  /*for(i=1; i<=(*knotenzahl); i++)
    fprintf(stderr, "%d: %f %f %f\n", i, (*koord)[i][0], (*koord)[i][1], (*koord)[i][2]);
    fprintf(stderr, "\n\n");*/
  
  
  return 1;
}

/* --- end include lese_vega2_fert.c ------------------------------------ */

/*****************************************************************************/

void mache_vega2d(FILE *fil, koordinaten_2d koord, 
		  koordinaten_3d koord3, 
		  EDGE *map[], 
		  EDGE_DUAL *map_d[], 
		  char graph_d[Max_Flaechenzahl][Max_Flaechenzahl], 
		  int knotenzahl,
		  SURFACE *flaechen_neu,
		  SURFACE *flaechen, 
		  int flaechenzahl, int *einbettungsreihenfolge){

  /* Diese Funktion fuegt Koordinaten (koord) und kombinatorische Struktur
     (map) von Graphen zusammen zu einem Vega-Code. 
     An der Stelle *fil wird ein zum Schreiben bereiter Zeiger auf FILE ge-
     schrieben.
     An der Stelle koord wird Koordinatenvektor geschrieben
     An der Stelle *map[] wird Zeiger auf die Struktur geschrieben, die mit
     Hilfe von lese_vega(...) augebaut wurde.*/
  
  int i, j, k, l, knoten, knoten_alt, akt_flaeche;
  EDGE *kante;
  int *marks;
  int ek_alt, endfl;
  EDGE_DUAL *kante_d;
  double x_max, y_max, x_min, y_min, save, v_x, v_y, sf, d_y, d_x;

  char **graph;

  if((marks=(int*)malloc((knotenzahl+1)*sizeof(int)))==NULL){
    fprintf(stderr," Speicherplatz reicht nicht aus(m.v.2)\n");
    exit(2);
  }

  if((graph=(char**)malloc((knotenzahl+1)*sizeof(char*)))==NULL){
    fprintf(stderr," Speicherplatz reicht nicht aus(2)(m.v.2)\n");
    exit(2);
  }
  else{
    for(i=0; i<=knotenzahl; ++i){
      if((graph[i]=(char*)malloc((knotenzahl+1)*sizeof(char)))==NULL){
	fprintf(stderr," Speicherplatz reicht nicht aus(3)(m.v.2)\n");
	exit(2);
      }
    }
  }

  for(i=0; i<=knotenzahl; ++i){
    for(j=0; j<=knotenzahl; ++j){
      graph[i][j]=0;
    }
  }

  for(i=0; i<=knotenzahl; ++i)
    marks[i]=0;

  if(flaechenzahl==1){
    for(j=0; flaechen_neu[1][j]; ++j){
      i=flaechen_neu[1][j];
      l=flaechen[1][j];
      kante=map[l];
      do{
	for(k=0; flaechen[1][k]; ++k){
	  if(flaechen[1][k]==kante->end){
	    knoten=flaechen_neu[1][k];
	    break;
	  }
	}
	graph[i][knoten]=1;
	graph[knoten][i]=1;
	kante=kante->next;
      }
      while(kante!=map[l]);
    }
  }
  else{
    for(k=1; k<=flaechenzahl; ++k){
      i=einbettungsreihenfolge[k];
      
      /*fprintf(stderr, " (5) 11\n");*/
      
      for(j=0; j<FlaechenGroesse; ++j){
	knoten=flaechen_neu[i][j];
	
	/*fprintf(stderr, " (5) 12 knoten = %d\n", knoten);*/
	
	if(knoten){
	  if(!marks[knoten]){

	    marks[knoten]=1;
	    
	    knoten_alt=flaechen[i][j];
	    
	    ek_alt=flaechen[i][j+1];
	    
	    if(!ek_alt){
	      ek_alt=flaechen[i][0];
	    }
	    
	    kante=map[knoten_alt];
	    
	    /*fprintf(stderr, " (5) ok 1 ek_alt %d\n", ek_alt);*/
	    
	    while((kante->end)!=ek_alt)
	      kante=kante->next;
	    
	    /*fprintf(stderr, " (5) ok 2\n");*/
	    
	    kante_d=map_d[i];
	    while((kante_d->connect)!=kante)
	      kante_d=kante_d->next;
	    
	    endfl=kante_d->end;
	    
	    /*fprintf(stderr, " (5) ok 3  kante_d: %d->%d\n", kante_d->start, endfl);*/
	    
	    while(graph_d[kante_d->start][endfl] && endfl){
	      kante=kante->prev;
	      kante_d=kante_d->invers;
	      /*fprintf(stderr, " (5) ok 3.05 kante_d: %d->%d\n", kante_d->start, kante_d->end);*/
	      while((kante_d->connect)!=kante)
		kante_d=kante_d->prev;
	      endfl=kante_d->end;
	      /*fprintf(stderr, " (5) ok 3.1 endfl %d\n", endfl);*/
	    }
	  
	    do{
	      
	      /*fprintf(stderr, " (5) 21   %d %d\n", kante_d->start, kante_d->end);*/
	      akt_flaeche=kante_d->start;
	      
	      if(!graph[knoten][flaechen_neu[akt_flaeche][kante_d->start_con[1]]]){
		if(kante_d->end)
		  graph[knoten][flaechen_neu[akt_flaeche][kante_d->start_con[1]]]=1;
		else 
		  graph[knoten][flaechen_neu[akt_flaeche][kante_d->start_con[1]]]=2;
	      }
	      kante=kante->next;
	      
	      while((kante_d->connect)!=kante->invers)
		kante_d=kante_d->next;

	      /*fprintf(stderr, " (5) 21.5   %d %d\n", kante_d->start, kante_d->end);*/

	      if(kante_d->end)
		kante_d=kante_d->invers;

	      /*fprintf(stderr, " (5) 22   %d %d\n", kante_d->start, kante_d->end);*/
	      
	    }
	    while(graph_d[kante_d->start][kante_d->end] && kante_d->end);

	    /*fprintf(stderr, " (5) 23\n");*/

	    if(kante_d->end)
	      kante_d=kante_d->invers;
	    
	    if(!graph[knoten][flaechen_neu[kante_d->start][kante_d->start_con[0]]]){
	      if(kante_d->end)
		graph[knoten][flaechen_neu[kante_d->start][kante_d->start_con[0]]]=1;
	      else 
		graph[knoten][flaechen_neu[kante_d->start][kante_d->start_con[0]]]=2;
	    }
		
	    /*fprintf(stderr, " (5) 83\n");*/
	  }
	}
	else 
	  break;
	
      }
    }

  }
  
  /*fprintf(stderr, " (5) 100\n");*/

  x_min=x_max=koord[1][0];
  y_min=y_max=koord[1][1];


  for(i=2; i<=knotenzahl; ++i){
    if(koord[i][0]>x_max){
      x_max=koord[i][0];
    }
    else{
      if(koord[i][0]<x_min){
	x_min=koord[i][0];
      }
    }

    if(koord[i][1]>y_max){
      y_max=koord[i][1];
    }
    else{
      if(koord[i][1]<y_min){
	y_min=koord[i][1];
      }
    }
  }

  d_x=x_max-x_min;
  d_y=y_max-y_min;

  /*fprintf(stderr, " (5) 110\n");*/
  if(d_y<d_x){   /*Tausche Koordinaten*/
    save=d_x;
    d_x=d_y;
    d_y=save;
    
    save=x_max;
    x_max=y_max;
    y_max=save;
    save=x_min;
    x_min=y_min;
    y_min=save;

    for(i=1; i<=knotenzahl; ++i){
      save=koord[i][0];
      koord[i][0]=koord[i][1];
      koord[i][1]=save;
    }
  } 
  
  /*fprintf(stderr, " (5) 120\n");*/

  if((d_y/d_x) > (730.0/480.0)){
    sf=730.0/d_y;
  }
  else{
    sf=480.0/d_x;
  }

  v_x=60-x_min*sf;
  v_y=60-y_min*sf;

  for(i=1; i<=knotenzahl; ++i){
    koord[i][0]=koord[i][0]*sf+v_x;
    koord[i][1]=koord[i][1]*sf+v_y;
  }

  /*fprintf(stderr, " (5) 130\n");*/

  if(print_header)
    fprintf(fil, "%%!PS-Adobe-3.0\n");
  fprintf(fil, "\n%%%%Page: %s %s\n\n", pagelabel, pageno);
  
  fprintf(fil, "newpath\n");
  
  /*fprintf(stderr, " (5) 140\n");*/
  for(i=1; i<knotenzahl; ++i){
    for(j=i+1; j<=knotenzahl; ++j){
      if(graph[i][j]==1){
	fprintf(fil, "%f %f moveto\n", koord[i][0], koord[i][1]);
	fprintf(fil, "%f %f lineto\n", koord[j][0], koord[j][1]);
	fprintf(fil, "1 setlinewidth\n");
	fprintf(fil, "stroke\n");
      }
      else{
	if(graph[i][j]==2){
	  fprintf(fil, "%f %f moveto\n", koord[i][0], koord[i][1]);
	  fprintf(fil, "%f %f lineto\n", koord[j][0], koord[j][1]);
	  fprintf(fil, "2.0 setlinewidth\n");
	  fprintf(fil, "stroke\n");
	}
      }
    }
  }
  
  fprintf(fil, "showpage\n");
   
}

/***************************************************************************/



int alloziere_kantenliste_dual (KANTENLISTE_DUAL **aktuelle_kantenliste, 
				int neu){
  
  /* Diese Funktion alloziert eine neue KANTENLISTE_DUAL (falls noetig), oder
     aktualisiert bereits vorhandene (falls lese_vega schon mal aufgeruffen
     wurde .
     Rueckgabewert: 1, wenn der Speicher alloziert wurde b.z.w. beriets zur
                       Verfuegung steht;
                    2, wenn der Speicher reicht nicht mehr aus.*/
  
  if(neu){
   if((*aktuelle_kantenliste=(KANTENLISTE_DUAL*)malloc(sizeof(KANTENLISTE_DUAL)))==NULL){
     fprintf(stderr," Speicherplatz reicht nicht aus(auf.3)\n");
     exit (2);
   }
   (*aktuelle_kantenliste)->next=NULL;
  }
  else{
    if( ((*aktuelle_kantenliste)->next)==NULL ){
      if(((*aktuelle_kantenliste)->next=(KANTENLISTE_DUAL*)malloc(sizeof(KANTENLISTE_DUAL)))==NULL){
	fprintf(stderr," Speicherplatz reicht nicht aus (auf.4)\n");
	exit (2);
      }
      (*aktuelle_kantenliste)->next->next=NULL;
    }
    *aktuelle_kantenliste=(*aktuelle_kantenliste)->next;
  }
  return 1;
}
  



/***************************************************************************/


void mache_flaechen(EDGE_DUAL ***map_d, EDGE *map[], int *flaechenzahl,
		    SURFACE **flaechen, int knotenzahl, int groesste_flaeche){
  
  int flaeche[FlaechenGroesse];
  int i, j, k, l, m, min_knoten, test, test2;
  EDGE *min_kante, *startkante, *kante;
  static KANTENLISTE_DUAL *Anfang;
  EDGE_DUAL **map_d_neu;
  SURFACE *flaechen_neu;
  int k1_1, k1_2, k2_1, k2_2;
  int kantennummer=0;
  KANTENLISTE_DUAL *aktuelle_kantenliste;
  EDGE_DUAL *kante_d, *kante2_d, *vorige_kante_d;
  int kp0_start, kp1_start, kp0_end, kp1_end;

  
  *flaechenzahl=0;

  if(!MAX_Flaechenzahl){  
    /* hier wird beim ersten Funktionsaufruf Speicher fuer
       map_d[] alloziert und Zeiger auf die erste KANTENLISTE_DUAL gesetzt*/
    
    MAX_Flaechenzahl=100/*200*/;
    if((*map_d=(EDGE_DUAL**)malloc(MAX_Flaechenzahl*sizeof(EDGE_DUAL*)))==NULL || (*flaechen=(SURFACE*)malloc(MAX_Flaechenzahl*sizeof(SURFACE)))==NULL){
      fprintf(stderr, " Speicher reicht nicht aus (mache_flaechen 1)");
      exit(2);
    }
    if((alloziere_kantenliste_dual(&Anfang, 1))!=1)
      exit(2);
  }
  
  aktuelle_kantenliste=Anfang;

  for(i=1; i<=knotenzahl; ++i){
    
    startkante=map[i];
    do{
      
      /*fprintf(stderr, "kante %d->%d \n", startkante->start, startkante->end);*/
      kante=min_kante=startkante;
      min_knoten=startkante->start;
      j=0;
      do{
	++j;
	kante=kante->invers->prev;
	if((kante->start) < min_knoten){
	  min_kante=kante;
	  min_knoten=kante->start;
	} 
      }
      while(kante!=startkante);
      if(j<=groesste_flaeche){
	if(j>=(FlaechenGroesse-1)){  /*es sind nur maximal (FlaechenGroesse-1)-
				       eckige Flaechen erlaubt*/
	  fprintf(stderr, " Eingabefehler: Flaechen haben zu viele Ecken\n");
	  exit(3);
	}
	
	flaeche[0]=min_knoten;
	kante=min_kante->invers->prev;
	for(j=1; kante!=min_kante; ++j){
	  flaeche[j]=kante->start;
	  kante=kante->invers->prev;
	}
	for(; j<FlaechenGroesse; ++j){
	  flaeche[j]=0;
	}
	test=1;
      }
      else{
	test=0;
      }

      for(j=1; j<=(*flaechenzahl) && test; ++j){
	k=0;
	do{
	  if(flaeche[k]!=(*flaechen)[j][k]){
	    test2=0;
	    for(m=0; m<FlaechenGroesse && (*flaechen)[j][m]; ++m){
	      if(flaeche[k]==(*flaechen)[j][m]){
		test2=1;
		break;
	      }
	    }
	    if(!test2)
	      break;
	  }
	  else{
	    if(flaeche[k]==0){
	      test=0;
	      break;
	    }
	  }
	  ++k;
	}
	while(k<FlaechenGroesse);
      }


      if(test){
	++(*flaechenzahl);
	if(MAX_Flaechenzahl<=(*flaechenzahl)){  	  
	  if((map_d_neu=(EDGE_DUAL**)malloc((MAX_Flaechenzahl+50)*sizeof(EDGE_DUAL*)))==NULL || (flaechen_neu=(SURFACE*)malloc((MAX_Flaechenzahl+50)*sizeof(SURFACE)))==NULL){
	    fprintf(stderr," Speicherplatz reicht nicht aus(auf.1)\n");
	    fprintf(stderr," MAX_Fl %ld\n", (MAX_Flaechenzahl+50)*sizeof(EDGE_DUAL*));
	    
	    exit(2);
	  }
	  /*memcpy(map_d_neu, *map_d, (size_t)MAX_Flaechenzahl*sizeof(EDGE_DUAL*));*/
	  memcpy(flaechen_neu, *flaechen, (size_t)MAX_Flaechenzahl*sizeof(SURFACE));
	  MAX_Flaechenzahl=MAX_Flaechenzahl+50;
	  free(*map_d);
	  free(*flaechen);
	  *flaechen=flaechen_neu;
	  *map_d=map_d_neu;
	}
	
	for(k=0; k<FlaechenGroesse; ++k)
	  (*flaechen)[*flaechenzahl][k]=flaeche[k];
      }
      
      startkante=startkante->next;
    }
    while(startkante!=map[i]);
  }
 

  /* hier werden im dualen Graphen Zeiger auf next und prev Kanten gesetzt 
     und start, end, start_con[], end_con[] bestimmt*/


  /*hier wird kante fuer map_d[0] gesetzt*/
  kante_d=(*map_d)[0]=aktuelle_kantenliste->liste+kantennummer;
  ++kantennummer;

  kante_d->start=kante_d->end=0;
  kante_d->next=kante_d->prev=kante_d->invers=kante_d;
  kante_d->connect=map[0];
  kante_d->start_con[0]=0;
  kante_d->start_con[1]=0;
  kante_d->end_con[0]=0;
  kante_d->end_con[1]=0;

  /*fprintf(stderr," 1. map_d[0]: %d->%d, inv: %d->%d, prev: %d->%d, next: %d->%d \n", (*map_d)[0]->start, (*map_d)[0]->end, (*map_d)[0]->invers->start, (*map_d)[0]->invers->end, (*map_d)[0]->prev->start, (*map_d)[0]->prev->end, (*map_d)[0]->next->start, (*map_d)[0]->next->end);*/

  if((*flaechenzahl)==1){
  
    kante_d=(*map_d)[1]=aktuelle_kantenliste->liste+kantennummer;
    ++kantennummer;

    kante=map[(*flaechen)[1][0]];
    while((kante->end)!=(*flaechen)[1][1])
      kante=kante->next;
    kante_d->connect=kante;

    kante_d->start_con[0]=0;
    kante_d->start_con[1]=1;
    kante_d->end_con[0]=1;
    kante_d->end_con[1]=0;
    
    kante_d->start=1;
    kante_d->end=1;
    kante_d->prev=kante_d;
    kante_d->next=kante_d;
  }
  else{

    for(k=0; k<FlaechenGroesse; ++k)
      (*flaechen)[0][k]=0;
    
    for(i=1; i<=(*flaechenzahl); ++i){
      for(k=0; k<FlaechenGroesse; ++k){
	if((*flaechen)[i][k]==0){
	  kante_d->next=(*map_d)[i];
	  (*map_d)[i]->prev=kante_d;
	  break;
	}
	else{
	  if((*flaechen)[i][k+1]==0){
	    k1_1=(*flaechen)[i][k];
	    k1_2=(*flaechen)[i][0];
	    kp0_start=k;
	    kp1_start=0;
	  }
	  else{
	    k1_1=(*flaechen)[i][k];
	    k1_2=(*flaechen)[i][k+1];
	    kp0_start=k;
	    kp1_start=k+1;
	  }
	}
	test=1;
	for(j=1; j<=(*flaechenzahl) && test; ++j){
	  if(j!=i && (*flaechen)[j][0]<=k1_1){
	    for(l=0; l<FlaechenGroesse && test; ++l){
	      if((*flaechen)[j][l]==0){
		break;
	      }
	      else{
		if((*flaechen)[j][l+1]==0){
		  k2_1=(*flaechen)[j][l];
		  k2_2=(*flaechen)[j][0];
		  kp0_end=l;
		  kp1_end=0;
		}
		else{
		  k2_1=(*flaechen)[j][l];
		  k2_2=(*flaechen)[j][l+1];
		  kp0_end=l;
		  kp1_end=l+1;
		}
	      }
	      if(k1_1==k2_2 && k1_2==k2_1){
		test=0;
		if(kantennummer==LISTENLAENGE){
		  if((alloziere_kantenliste_dual(&aktuelle_kantenliste, 0))!=1)
		    exit(2);
		  kantennummer=0;
		}
		if(k==0){
		  kante_d=vorige_kante_d=(*map_d)[i]=aktuelle_kantenliste->liste+kantennummer;
		  ++kantennummer;
		}
		else{
		  kante_d=aktuelle_kantenliste->liste+kantennummer;
		  ++kantennummer;
		}
		
		kante=map[k1_1];
		while((kante->end)!=k1_2)
		  kante=kante->next;
		kante_d->connect=kante;
		
		kante_d->start_con[0]=kp0_start;
		kante_d->start_con[1]=kp1_start;
		kante_d->end_con[0]=kp0_end;
		kante_d->end_con[1]=kp1_end;
		
		kante_d->start=i;
		kante_d->end=j;
		kante_d->prev=vorige_kante_d;
		vorige_kante_d->next=kante_d;
		
		vorige_kante_d=kante_d;
	      }
	    }
	  }
	  if(j==(*flaechenzahl) && test){
	    /*exit(0);*/
	    
	    if(kantennummer==LISTENLAENGE){
	      if((alloziere_kantenliste_dual(&aktuelle_kantenliste, 0))!=1)
		exit(2);
	      kantennummer=0;
	    }
	    if(k==0){
	      kante_d=vorige_kante_d=(*map_d)[i]=aktuelle_kantenliste->liste+kantennummer;
	      ++kantennummer;
	    }
	    else{
	      kante_d=aktuelle_kantenliste->liste+kantennummer;
	      ++kantennummer;
	    }
	    
	    kante=map[k1_1];
	    while((kante->end)!=k1_2)
	      kante=kante->next;
	    kante_d->connect=kante;
	    
	    kante_d->start_con[0]=kp0_start;
	    kante_d->start_con[1]=kp1_start;
	    kante_d->end_con[0]=0;
	    kante_d->end_con[1]=0;
	    
	    kante_d->start=i;
	    kante_d->end=0;
	    kante_d->prev=vorige_kante_d;
	    vorige_kante_d->next=kante_d;
	    
	    vorige_kante_d=kante_d;
	  }
	}
      }
    }
    
  }
  
  /*fprintf(stderr," map_d[0]: %d->%d, inv: %d->%d, prev: %d->%d, next: %d->%d \n", (*map_d)[0]->start, (*map_d)[0]->end, (*map_d)[0]->invers->start, (*map_d)[0]->invers->end, (*map_d)[0]->prev->start, (*map_d)[0]->prev->end, (*map_d)[0]->next->start, (*map_d)[0]->next->end);*/

  /* hier for(...){...} werden inversen Kanten gekoppelt und ueberprueft, ob 
     solche existieren. Wenn nicht - Fehlermeldung */
  if((*flaechenzahl)==1){
    (*map_d)[1]->invers=(*map_d)[1];
  }
  else{
    for(i=1; i<=(*flaechenzahl); ++i){
      kante_d=(*map_d)[i];
      do{
	k1_1=kante_d->start;
	k1_2=kante_d->end;
	
	if(k1_2==0){
	  kante_d->invers=(*map_d)[0];
	}
	else{
	  if(k1_1>k1_2){
	    
	    test=1;
	    for(j=1; j<=(*flaechenzahl) && test; ++j){
	      if(i!=j){
		kante2_d=(*map_d)[j];
		do{
		  k2_1=kante2_d->start;
		  k2_2=kante2_d->end;
		  
		  if(k1_1==k2_2 && k1_2==k2_1 && (kante_d->connect->start)==(kante2_d->connect->end)){

		    if(i==1 && k1_2==0){
		      fprintf(stderr, " kante_d: %d->%d  kante2_d: %d->%d\n", k1_1, k1_2, k2_1, k2_2);
		      exit(2);
		    }

		    kante_d->invers=kante2_d;
		    kante2_d->invers=kante_d;
		    test=0;
		    break;
		  }
		  kante2_d=kante2_d->next;
		}
		while(kante2_d!=(*map_d)[j]);
	      }
	    }
	    if(test){
	      fprintf(stderr,"Eingabefehler (inputerror): Kante %d->%d existiert, aber %d->%d nicht\n", k1_1, k1_2, k1_2, k1_1);
	      exit(3);
	    }	
	  }
	}
	kante_d=kante_d->next;
      }
      while(kante_d!=(*map_d)[i]);
    }
    
  }
  
  /*fprintf(stderr, " Nachbarn im Uhrzeigersinn \n");
    for(i=1; i<=(*flaechenzahl); i++){
    kante_d=(*map_d)[i];
    fprintf(stderr, " %d: ", i);
    do{
    fprintf(stderr, " %d", kante_d->end);
    kante_d=kante_d->next;
    }
    while(kante_d!=(*map_d)[i]);
    fprintf(stderr, "\n");
    }
    
    fprintf(stderr, " Nachbarn gegen Uhrzeigersinn \n");
    for(i=1; i<=(*flaechenzahl); i++){
    kante_d=(*map_d)[i];
    fprintf(stderr, " %d: ", i);
    do{
    fprintf(stderr, " %d", kante_d->end);
    kante_d=kante_d->prev;
    }
    while(kante_d!=(*map_d)[i]);
    fprintf(stderr, "\n");
    }
    
    fprintf(stderr, " Inversen \n");
    for(i=1; i<=(*flaechenzahl); i++){
    kante_d=(*map_d)[i];
    fprintf(stderr, " %d: ", i);
    do{
    fprintf(stderr, " %d", kante_d->invers->start);
    kante_d=kante_d->next;
    }
    while(kante_d!=(*map_d)[i]);
    fprintf(stderr, "\n");
    }*/


}


/***************************************************************************/

int bette_C_ein(int A, int A2, int B, int B2, int C, int C2, 
		koordinaten_3d koord3, koordinaten_2d koord2){

  double AB_3[3], F_3[3];
  double AB_2[2], F1[2], F2[2];
  double L_AB, L_AB2, L_AF, L_BF, L_CF, tC, tF;
  double d1, d2, d3, d4;
  
  AB_3[0]=koord3[B][0]-koord3[A][0];
  AB_3[1]=koord3[B][1]-koord3[A][1];
  AB_3[2]=koord3[B][2]-koord3[A][2];

  if(!C){

    koord2[A2][0]=0;
    koord2[A2][1]=0;

    L_AB=AB_3[0]*AB_3[0]+AB_3[1]*AB_3[1]+AB_3[2]*AB_3[2];
    
    koord2[B2][1]=sqrt(L_AB);

    koord2[B2][0]=0;


    return 1;
  }
  else{
   AB_2[0]=koord2[B2][0]-koord2[A2][0];
   AB_2[1]=koord2[B2][1]-koord2[A2][1];  
 
   L_AB2=AB_2[0]*AB_2[0]+AB_2[1]*AB_2[1]; 
  }

  /*fprintf(stderr, " \t (3) 20 \n");*/

  L_AB=AB_3[0]*AB_3[0]+AB_3[1]*AB_3[1]+AB_3[2]*AB_3[2];

  if(fabs(sqrt(L_AB)-sqrt(L_AB2))/sqrt(L_AB) > 0.05){
    fprintf(stderr, "Achtung! Verzehrung des Aufschnittes(AB %f A=%d, B=%d)\n\n\n", (sqrt(L_AB)-sqrt(L_AB2))/sqrt(L_AB), A, B);
    exit(3);
  }

  
  /* Berechnung von Koordinaten von F in 3d und Laengen (hoch 2) 
     von AF, BF, CF */

  
  tF=((koord3[C][0]-koord3[A][0])*AB_3[0]+(koord3[C][1]-koord3[A][1])*AB_3[1]+(koord3[C][2]-koord3[A][2])*AB_3[2])/L_AB;

  F_3[0]=koord3[A][0]+tF*AB_3[0];
  F_3[1]=koord3[A][1]+tF*AB_3[1];
  F_3[2]=koord3[A][2]+tF*AB_3[2];

  d1=F_3[0]-koord3[A][0];
  d2=F_3[1]-koord3[A][1];
  d3=F_3[2]-koord3[A][2];

  L_AF=d1*d1+d2*d2+d3*d3;

  /*fprintf(stderr, " \t (3) 24 \n");*/

  d1=F_3[0]-koord3[B][0];
  d2=F_3[1]-koord3[B][1];
  d3=F_3[2]-koord3[B][2];

  L_BF=d1*d1+d2*d2+d3*d3;

  d1=F_3[0]-koord3[C][0];
  d2=F_3[1]-koord3[C][1];
  d3=F_3[2]-koord3[C][2];

  L_CF=d1*d1+d2*d2+d3*d3;
  
  /* Berechnung von Koordinaten von F in 2d */

  tF=sqrt(L_AF/L_AB);
  
  F1[0]=koord2[A2][0]+tF*AB_2[0];
  F1[1]=koord2[A2][1]+tF*AB_2[1];
  
  F2[0]=koord2[A2][0]-tF*AB_2[0];
  F2[1]=koord2[A2][1]-tF*AB_2[1];

  /*fprintf(stderr, " \t (3) 28 \n");*/

  d1=F1[0]-koord2[B2][0];
  d2=F1[1]-koord2[B2][1];
  d3=F2[0]-koord2[B2][0];
  d4=F2[1]-koord2[B2][1];

  if(fabs(L_BF-(d1*d1+d2*d2)) > fabs(L_BF-(d3*d3+d4*d4))){
    F1[0]=F2[0];
    F1[1]=F2[1];
  }
  
  /* Berechnung von Koordinaten von C in 2d */
  
  tC=sqrt(L_CF/L_AB);

  /*fprintf(stderr, " \t (3) 30 \n");*/

  koord2[C2][0]=F1[0]+tC*AB_2[1];
  koord2[C2][1]=F1[1]-tC*AB_2[0];

  return 1;
}

/****************************************************************************/

/* --- include triang.c ------------------------------------------------- */
/*#include<stdio.h>*/
/*#include<stdlib.h>*/

/*#define FlaechenGroesse 50*/
#define KantenZahl 100
/*#define ENTFERNUNGSFAKTOR 1.0/10.0*/

/*typedef struct Edge_add{
  int start;
  int end;
  int add;*/               /* edge->add=0, wenn die Kante nicht auf dem Rand 
			      liegt;
			      edge->add=1, wenn auf dem Rand und edge->prev
			      auf dem Rand;
			      edge->add=2, wenn auf dem Rand und edge->next
			      auf dem Rand;*/
/*struct Edge_add *prev;
  struct Edge_add *next;
  struct Edge_add *invers;
  } EDGE_Add;*/

EDGE_Add **LIste;

/*EDGE_Add ***triangulierungen;*/


#ifndef MAXN
#define MAXN 300            /* the maximum number of vertices; see above */
#endif
#define MAXE (6*MAXN-12)   /* the maximum number of oriented edges */
#define MAXF (2*MAXN-4)    /* the maximum number of faces */

#undef FALSE
#undef TRUE
#define FALSE 0
#define TRUE  1

/* Global variables */

int nv;
int ne;

/*static int markvalue = 30000;*/

/* and the same for vertices */

static int markvalue_v = 30000;
static int marks__v[MAXN];

#define RESETMARKS_V {int mki; if ((++markvalue_v) > 30000) \
       { markvalue_v = 1; for (mki=0;mki<MAXN;++mki) marks__v[mki]=0;}}
#define ISMARKED_V(x) (marks__v[x] == markvalue_v)
#define MARK_V(x) (marks__v[x] = markvalue_v)


/**************************************************************************/

static int 
testcanon(EDGE *givenedge, int representation[], int colour[])
     
     /* Tests whether starting from a given edge and constructing the code in
	"->next" direction, an automorphism 
	can be found. Returns 0 for failure, 1 for an automorphism. */
     
{
  EDGE *temp, *run;  
  EDGE *startedge[MAXN]; /* startedge[i] is the starting edge for 
			    exploring the vertex with the number i+1 */
  int number[MAXN], i;   /* The new numbers of the vertices, starting 
			    at 1 in order to have "0" as a possibility to
			    mark ends of lists and not yet given numbers */
  int last_number, actual_number, vertex;
  
  for (i = 0; i < nv; i++) number[i] = 0;
  
  number[givenedge->start] = 1; 
  
  if (givenedge->start != givenedge->end) /* no loop start */
    {
      number[givenedge->end] = 2;
      last_number = 2;
      startedge[1] = givenedge->invers;
    }
  else last_number = 1 ; 
  
  actual_number = 1;
  temp = givenedge;
  
  /* A representation is a clockwise ordering of all neighbours ended with a 0.
     The neighbours are numbered in the order that they are reached by the BFS 
     procedure. In case a vertex is reached for the first time, not the (new)
     number of the vertex is listed, but its colour. When the list around a
     vertex is finished, it is ended with a 0. Since the colours can be 
     distinguished from the vertices (requirement for the colour function), the
     adjacency list can be reconstructed: Every time a colour is listed, its
     number would be the smallest number not given yet.
     Since the edges when a vertex is reached for the first time are remembered,
     for these edges we in fact have more than just the vertex information -- for
     these edges we also have the exact information which edge occurs in the
     cyclic order. This makes the routine work also for double edges.
     
     Since every representation starts with the colour of vertex 2, which is
     the same colour all the time, we do not have to store that. 
     
     In case of a loop as the starting point, the colour of 1 is omitted. 
     Nevertheless also in this case it cannot be mixed up with a non-loop
     starting point, since the number of times a colour occurs is larger
     for loop starters than for non-loop starters.
     
     Every first entry in a new clockwise ordering (the starting point of the
     edge it was numbered from is determined by the entries before (the first
     time it occurs in the list to be exact), so this is not given either. 
     The K4 could be denoted  c3 c4 0 4 3 0 2 3 0 3 2 0 if ci is the colour
     of vertex i.  Note that the colour of vertex 1 is -- by definition --
     always the smallest one */
  
  while (last_number < nv)
    {  
      for (run = temp->next; run != temp; run = run->next)
	/* this loop marks all edges around temp->origin. */
	{ vertex = run->end;
	if (!number[vertex])
	  { startedge[last_number] = run->invers;
	  last_number++; number[vertex] = last_number;
	  vertex = colour[vertex]; }
	else vertex = number[vertex];
	if (vertex != (*representation)) return(0);
	representation++; 
	}
      /* check whether representation[] is also at the end of a cyclic list */
      if ((*representation) != 0) return(0); 
      representation++;
      /* Next vertex to explore: */
      temp = startedge[actual_number];  actual_number++; 
    }
  
  while (actual_number <= nv) 
    /* Now we know that all numbers have been given */
    {  
      for (run = temp->next; run != temp; run = run->next)
	/* this loop marks all edges around temp->origin. */
	{ 
	  vertex = number[run->end];
	  if (vertex != (*representation)) return(0);
	  representation++;
	}
      /* check whether representation[] is also at the end of a cyclic list */
      if ((*representation) != 0) return(0); 
      representation++;
      /* Next vertex to explore: */
      temp = startedge[actual_number];  actual_number++; 
    }
  
  return(1);
}

/*****************************************************************************/

static int 
testcanon_mirror(EDGE *givenedge, int representation[], int colour[])
     
     /* Tests whether starting from a given edge and constructing the code in
	"->prev" direction, an automorphism can 
	be found. Comments see testcanon -- it is exactly the same except for 
	the orientation */
{
  EDGE *temp, *run;  
  EDGE *startedge[MAXN];
  int number[MAXN], i; 
  int last_number, actual_number, vertex;
  
  for (i = 0; i < nv; i++) number[i] = 0;
  
  number[givenedge->start] = 1; 
  
  if (givenedge->start != givenedge->end)
    {
      number[givenedge->end] = 2;
      last_number = 2;
      startedge[1] = givenedge->invers;
    }
  else last_number = 1; 
  
  actual_number = 1;
  temp = givenedge;
  
  while (last_number < nv)
    {  
      for (run = temp->prev; run != temp; run = run->prev)
	{ vertex = run->end;
	if (!number[vertex])
	  { startedge[last_number] = run->invers;
	  last_number++; number[vertex] = last_number;
	  vertex = colour[vertex]; }
	else vertex = number[vertex];
	if (vertex != (*representation)) return(0);
	representation++; 
	}
      if ((*representation) != 0) return(0); 
      representation++;
      temp = startedge[actual_number];  actual_number++; 
    }
  
  while (actual_number <= nv) 
    {  
      for (run = temp->prev; run != temp; run = run->prev)
	{ 
	  vertex = number[run->end];
	  if (vertex != (*representation)) return(0);
	  representation++;
	}
      if ((*representation) != 0) return(0); 
      representation++;
      temp = startedge[actual_number];  actual_number++; 
    }
  
  return(1);
}

/****************************************************************************/

static void
testcanon_first_init(EDGE *givenedge, int representation[], int colour[])
     
     /* Constructs a first representation without comparing */
     
{
  register EDGE *run;
  register int vertex;
  EDGE *temp;  
  EDGE *startedge[MAXN]; 
  int number[MAXN], i; 
  int last_number, actual_number;
  
  for (i = 0; i < nv; i++) number[i] = 0;
  
  number[givenedge->start] = 1; 
  if (givenedge->start != givenedge->end)
    {
      number[givenedge->end] = 2;
      last_number = 2;
      startedge[1] = givenedge->invers;
    }
  else last_number = 1; 
  
  actual_number = 1;
  temp = givenedge;
  
  while (last_number < nv)
    {  
      for (run = temp->next; run != temp; run = run->next)
	{ vertex = run->end;
	if (!number[vertex])
	  { startedge[last_number] = run->invers;
	  last_number++; number[vertex] = last_number; 
	  *representation = colour[vertex]; }
	else *representation = number[vertex]; 
	representation++;
	}
      *representation = 0;
      representation++;
      temp = startedge[actual_number];  actual_number++;
    }
  
  while (actual_number <= nv) 
    {  
      for (run = temp->next; run != temp; run = run->next)
	{ 
	  *representation = number[run->end]; representation++;
	}
      *representation = 0;
      representation++;
      temp = startedge[actual_number];  actual_number++;
    }
  
  return;
}

/****************************************************************************/

static int 
testcanon_init(EDGE *givenedge, int representation[], int colour[])
     
     /* Tests whether starting from a given edge and constructing the code in
	"->next" direction, an automorphism can 
	be found. It works pretty similar to testcanon except 
	for obviously necessary changes, so for extensive comments see testcanon */
{
  register EDGE *run;
  register int vertex;
  EDGE *temp;  
  EDGE *startedge[MAXN]; 
  int number[MAXN], i; 
  int last_number, actual_number;
  
  for (i = 0; i < nv; i++) number[i] = 0;
  
  number[givenedge->start] = 1; 
  if (givenedge->start != givenedge->end)
    {
      number[givenedge->end] = 2;
      last_number = 2;
      startedge[1] = givenedge->invers;
    }
  else last_number = 1; 
  
  actual_number = 1;
  temp = givenedge;
  
  while (last_number < nv)
    {  
      for (run = temp->next; run != temp; run = run->next)
	{ vertex = run->end;
	if (!number[vertex])
	  { startedge[last_number] = run->invers;
	  last_number++; number[vertex] = last_number; 
	  vertex = colour[vertex]; }
	else vertex=number[vertex];
	if (vertex != (*representation)) return(0);
	representation++; 
	}
      if ((*representation) != 0) return(0);
      representation++;
      temp = startedge[actual_number];  actual_number++;
    }
  
  while (actual_number <= nv) 
    {  
      for (run = temp->next; run != temp; run = run->next)
	{ vertex = number[run->end]; 
	if (vertex != (*representation)) return(0);
	representation++;
	}
      if ((*representation) != 0)  return(0);
        representation++;
        temp = startedge[actual_number];  actual_number++;
    }
  
  return(1);
}

/****************************************************************************/

static int 
testcanon_mirror_init(EDGE *givenedge, int representation[], int colour[])
     
     /* Tests whether starting from a given edge and constructing the code in
   "->prev" direction, an automorphism can 
   be found. */
{
  EDGE *temp, *run;  
  EDGE *startedge[MAXN]; 
  int number[MAXN], i; 
  int last_number, actual_number, vertex;
  
  for (i = 0; i < nv; i++) number[i] = 0;
  
  number[givenedge->start] = 1;
  if (givenedge->start != givenedge->end)
    {
      number[givenedge->end] = 2;
      last_number = 2;
      startedge[1] = givenedge->invers;
    }
  else last_number = 1; 
  
  actual_number = 1;
  temp = givenedge;
  
  while (last_number < nv)
    {  
      for (run = temp->prev; run != temp; run = run->prev)
	{ vertex = run->end;
	if (!number[vertex])
	  { startedge[last_number] = run->invers;
	  last_number++; number[vertex] = last_number; 
	  vertex = colour[vertex]; }
	else vertex=number[vertex];
	if (vertex != (*representation)) return(0);
	representation++; 
	}
      if ((*representation) != 0) return(0);
      representation++;
      temp = startedge[actual_number];  actual_number++;
    }
  
  while (actual_number <= nv) 
    {  
      for (run = temp->prev; run != temp; run = run->prev)
	{ vertex = number[run->end]; 
	if (vertex != (*representation)) return(0);
	representation++;
	}
      if ((*representation) != 0) return(0);
       representation++;
       temp = startedge[actual_number];  actual_number++;
    }
  
  return(1);
}

/****************************************************************************/

static void
construct_numb(EDGE *givenedge, EDGE /* *numbering[]*/ **numbering)

/* Starts at givenedge and writes the edges in the well defined order 
   into the list.  Works like testcanon. Look there for comments. */
{
  EDGE *temp, **tail, **limit, *run;  
  EDGE *startedge[MAXN]; 
  int last_number, actual_number, vertex;
  
  RESETMARKS_V;
  
  tail = numbering; 
  limit = numbering+ne-1;  
  
  MARK_V(givenedge->start); 
  if (givenedge->start != givenedge->end)
    {
      MARK_V(givenedge->end);
      last_number = 2;
      startedge[1] = givenedge->invers;
    }
  else last_number = 1; 
  
  actual_number = 1;
  temp = *tail = givenedge;
  
  while (last_number < nv)
    {  
      for (run = temp->next; run != temp; run = run->next)
	/* this loop marks all edges around temp->origin. */
	{ vertex = run->end;
	if (!ISMARKED_V(vertex))
	  { startedge[last_number] = run->invers;
	  last_number++; MARK_V(vertex); }
	tail++; *tail = run; 
	}
      if (tail != limit)
	{ tail++;
	*tail = temp = startedge[actual_number];  actual_number++; }
    }
  
  while (tail != limit) 
    /* Now we know that all numbers have been given */
    {  
      for (run = temp->next; run != temp; run = run->next)
	/* this loop marks all edges around temp->origin. */
	{ tail++; *tail = run; }
      if (tail != limit)
        { 
	  /* Next vertex to explore: */
	  tail++;
	  *tail = temp = startedge[actual_number];  actual_number++; }
    }
}

/****************************************************************************/

static void 
construct_numb_mirror(EDGE *givenedge, EDGE /* *numbering[]*/ **numbering)
     
     /* Starts at givenedge and writes the edges in the well defined order 
	into the list.  Works like testcanon. Look there for comments.  */
{
  EDGE *temp, **tail, **limit, *run;  
  EDGE *startedge[MAXN]; 
  int last_number, actual_number, vertex;
  
  RESETMARKS_V;
  
  tail = numbering; /* The first entry of the numbering list */
  limit = numbering+ne-1;  /* Last valid entry of the numbering list */
  
  MARK_V(givenedge->start); 
  if (givenedge->start != givenedge->end)
    {
      MARK_V(givenedge->end);
      last_number = 2;
      startedge[1] = givenedge->invers;
    }
  else last_number = 1; 
  
  actual_number = 1;
  temp = *tail = givenedge;
  
  while (last_number < nv)
    {  
      for (run = temp->prev; run != temp; run = run->prev)
	/* this loop marks all edges around temp->origin. */
	{ vertex = run->end;
	if (!ISMARKED_V(vertex))
	  { startedge[last_number] = run->invers;
	  last_number++; MARK_V(vertex); }
	tail++; *tail = run; 
	}
      if (tail != limit)
	{
	  tail++;
	  *tail = temp = startedge[actual_number];  actual_number++; }
    }
  
  while (tail != limit) 
      /* Now we know that all numbers have been given */
    {  
      for (run = temp->prev; run != temp; run = run->prev)
    /* this loop marks all edges around temp->origin. */
	{ tail++; *tail = run; }
      if (tail != limit)
        { 
	  /* Next vertex to explore: */
	  tail++;
	  *tail = temp = startedge[actual_number];  actual_number++; }
    }
}

/****************************************************************************/

static int 
group(int lcolour[], EDGE /*numberings[][MAXE]*/ ***numberings, 
      int *num_numberings, int *num_numberings_or_pres)
     
     /* Computes the group of the coloured map.
	
	IT IS ASSUMED that the values of the colour function are positive
	and at most INT_MAX-MAXN.
	
	In numberings[][] the numberings are stored as sequences 
	of oriented edges.  For every 0 <= i,j < *num_numberings and every 
	0 <= k < ne the edges numberings[i][k] and numberings[j][k] can 
	be mapped onto each other by an automorphism. The first 
	*num_numberings_or_pres numberings are orientation preserving while 
	the rest is orientation reversing.
	
	In case of only 1 automorphism, numberings doesn't contain any well
	defined information.
	
	In case of nontrivial automorphisms, can[0] starts with a list of edges 
	adjacent to nv-1. In case of an orientation preserving numbering the edges 
	are listed in ->next direction, otherwise in ->prev direction.
	
	Works OK if at least one vertex has valence >= 3. Otherwise some numberings 
	are computed twice, since changing the orientation (the cyclic order around 
	each vertex) doesn't change anything */
{
  int i, last_vertex, test;
  int minstart, maxend; /* (minstart,maxend) will be the chosen colour 
			   pair of an edge */
  EDGE *startlist_last[5], *startlist[5*MAXN], *run, *end;
  int list_length_last, list_length;
  int representation[MAXE];
  EDGE *numblist[MAXE], *numblist_mirror[MAXE]; /* lists of edges where 
						   starting gives a canonical 
						   representation */
  int numbs = 1, numbs_mirror = 0;
  int colour[MAXN];
  
  for (i=0; i<nv; i++) colour[i]=lcolour[i]+MAXN;
  /* to distinguish colours from vertices */
  last_vertex = nv-1;
  minstart = colour[last_vertex];
  
  /* determine the possible starting edges around "last_vertex" */
  
  list_length_last = 1; startlist_last[0] = end = firstedge[last_vertex];
  maxend = colour[end->end];
  
  for (run = end->next; run != end; run = run->next)
    { if (colour[run->end] == maxend)
      { startlist_last[list_length_last] = run; list_length_last++; }
    }
  
  /* Now we can determine a list 
     of all edges with this colour pair. */
  
    list_length = 0;
    
    for (i = 0; i < last_vertex; i++) 
      { 
        if (colour[i] == minstart)
          { run = end = firstedge[i];
	  do
            {
              if (colour[run->end] == maxend)
                { startlist[list_length] = run; list_length++; }
              run = run->next;
            } while (run != end);
          }
      }
    
    /* OK -- we have to determine the 
       automorphisms around vertex "last_vertex": */

    testcanon_first_init(startlist_last[0], representation, colour);
    numblist[0] = startlist_last[0];
    test = testcanon_mirror_init(startlist_last[0], representation, colour);
    if (test == 1) 
      { numbs_mirror = 1; numblist_mirror[0] = startlist_last[0]; }

    for (i = 1; i < list_length_last; i++)
      { test = testcanon_init(startlist_last[i], representation, colour);
      if (test == 1) { numblist[numbs] = startlist_last[i]; numbs++; }
      test = testcanon_mirror_init(startlist_last[i], 
				   representation, colour);
      if (test == 1)  
	{ numblist_mirror[numbs_mirror] = startlist_last[i]; 
	numbs_mirror++; }
      }

    /* Now we will check all the others. */
    
    for (i = 0; i < list_length; i++)
      { test = testcanon(startlist[i], representation, colour);
      if (test == 1) { numblist[numbs] = startlist[i]; numbs++; }
        test = testcanon_mirror(startlist[i], representation, colour);
        if (test == 1) 
          { numblist_mirror[numbs_mirror] = startlist[i]; numbs_mirror++; }
      }

    *num_numberings_or_pres = numbs;
    *num_numberings = numbs+numbs_mirror;
    
    if (*num_numberings>1)
      { for (i = 0; i < numbs; i++) 
          construct_numb(numblist[i], numberings[i]); 
      for (i = 0; i < numbs_mirror; i++, numbs++) 
	construct_numb_mirror(numblist_mirror[i], numberings[numbs]);
      }
    else 
      { if (numbs) numberings[0][0] = numblist[0];
      else numberings[0][0] = numblist_mirror[0]; }
    
    return(1);
}


/*****************************************************************************/


/*void gebe_graph(EDGE_Add *Map[], int eckzahl){

  EDGE_Add *kante, *startkante;
  int i;
  
  for(i=0; i<eckzahl; i++){
    kante=startkante=Map[i];
    fprintf(stderr, "\n\n knoten %d:\n Uhrzeigersinn: ", i);
    do{
      fprintf(stderr, " %d", kante->end);
      kante=kante->next;
    }
    while(kante!=startkante);

    fprintf(stderr, "\n gegen Uhrzeigersinn: ", i);
    do{
      fprintf(stderr, " %d", kante->end);
      kante=kante->prev;
    }
    while(kante!=startkante);

    fprintf(stderr, "\n inverse Kanten: ", i);
    do{
      fprintf(stderr, " %d->%d,", kante->invers->start, kante->invers->end);
      kante=kante->next;
    }
    while(kante!=startkante);
    fprintf(stderr, "\n");
  }
  }*/

/*****************************************************************************/

int passt(int knoten1, int knoten2, int knoten3, int eckzahl){
  /* Prueft, ob Kante mit knoten2 als Endknoten zwischen Kanten mit knoten1 
     und knoten3 als Endknoten passt; alle drei Kanten haben den gleichen
     Startknoten*/
  
  for(knoten1=(knoten1+1)%eckzahl; knoten1!=knoten3; knoten1=(knoten1+1)%eckzahl){
    if(knoten1==knoten2)
      return 1;
  }
  return 0;
}

/****************************************************************************/

int zaehle_flaechen(EDGE_Add *Map[], SURFACE *flaechen, int flaechennummer, 
		   koordinaten_3d koord3, int eckzahl){

  /* diese Funktion zaehlt die Anzahl der Flaechen, die durch die aktuelle
     Triangulierung (*Map[]) der Flaeche mit der Nummer "flaechennummer" 
     entstehen*/ 
  
  int uKnoten[FlaechenGroesse]={0};
  EDGE_Add *zuuFlaechen[FlaechenGroesse];
  EDGE_Add *zuuKanten[KantenZahl];
  EDGE_Add *kante, *fl_kante;
  int flaechenzahl=1;
  int zahl_uknoten;
  int i, j;
  int ukantenzahl;
  int eckzahl_m;

  int A, B, C, P;
  double AB[3], AC[3], n[3];
  double Abst, L_BC, d;
  double d1, d2, d3;

  kante=zuuFlaechen[0]=Map[0];

  uKnoten[0]=1;
  uKnoten[1]=1;
  uKnoten[kante->next->end]=1;

  zahl_uknoten=3;
  eckzahl_m=eckzahl-1;
  
  for(i=0; i<flaechenzahl && zahl_uknoten!=eckzahl; i++){

    fl_kante=zuuFlaechen[i];
    ukantenzahl=0;

    if(!(fl_kante->next->add)){ /* d.h. fl_kante->next nicht auf dem Rand*/
      zuuKanten[0]=fl_kante->next;
      ukantenzahl++;
    }
    if(!(fl_kante->next->invers->next->add)){ /* fl_kante->next->invers->next
					      nicht auf dem Rand*/
      zuuKanten[ukantenzahl]=fl_kante->next->invers->next;
      ukantenzahl++;
    }

    for(j=0; j<ukantenzahl && zahl_uknoten!=eckzahl; j++){

      kante=zuuKanten[j];
      
      /*if((d=(kante->start)-(kante->end))==1 || d==(-eckzahl_m)){
	fprintf(stderr, " d=1 !!!\n");
	exit(3);
	}*/


      if(!uKnoten[kante->next->end]){
	/* d.h. Kante liegt nicht auf dem Rand und der dritte Knoten der
	   Flaeche (die rechts von der Kante liegt) gehoert noch zu keiner 
	   Flaeche */
     

	A=flaechen[flaechennummer][kante->prev->end];
	B=flaechen[flaechennummer][kante->start];
	C=flaechen[flaechennummer][kante->end];
	P=flaechen[flaechennummer][kante->next->end];

	AB[0]=koord3[B][0]-koord3[A][0];
	AB[1]=koord3[B][1]-koord3[A][1];
	AB[2]=koord3[B][2]-koord3[A][2];
	
	AC[0]=koord3[C][0]-koord3[A][0];
	AC[1]=koord3[C][1]-koord3[A][1];
	AC[2]=koord3[C][2]-koord3[A][2];
	
	n[0]=AB[1]*AC[2]-AB[2]*AC[1];
	n[1]=AB[2]*AC[0]-AB[0]*AC[2];
	n[2]=AB[0]*AC[1]-AB[1]*AC[0];
	
	d=n[0]*koord3[A][0]+n[1]*koord3[A][1]+n[2]*koord3[A][2];
	Abst=fabs((n[0]*koord3[P][0]+n[1]*koord3[P][1]+n[2]*koord3[P][2]-d)/sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]));
	d1=koord3[C][0]-koord3[B][0];
	d2=koord3[C][1]-koord3[B][1];
	d3=koord3[C][2]-koord3[B][2];
	L_BC=sqrt(d1*d1+d2*d2+d3*d3);
	
	if(Abst>L_BC*ENTFERNUNGSFAKTOR){ /*d.h Knoten P gehoert nicht zu der
					   Flaeche ABC*/
	  zuuFlaechen[flaechenzahl]=kante;
	  flaechenzahl++;
	  	  
	}
	else{
	  if(!(kante->next->add)){
	    zuuKanten[ukantenzahl]=kante->next;
	    ukantenzahl++;
	  }
	  if(!(kante->next->invers->next->add)){
	    zuuKanten[ukantenzahl]=kante->next->invers->next;
	    ukantenzahl++;
	  }
	}

	zahl_uknoten++;
	uKnoten[kante->next->end]=1;
      }
      else{
	fprintf(stderr, " Knotenposition %d der Flaeche %d beriets untersucht\n", kante->next->end, flaechennummer);
	exit (3);
      }
    }
  }
  
  if(zahl_uknoten!=eckzahl){
    fprintf(stderr, "zahl_uknoten!=eckzahl (%d, %d) (z.f.)\n", zahl_uknoten, eckzahl);
    exit (3);
  }
  
  return flaechenzahl;
}

/****************************************************************************/

int vergleich(int neu, SURFACE *flaechen, koordinaten_3d koord3, 
	      int triangulierung[], int flaechennummer){

  /* Diese Funktion vergleicht die Anzahl der Flaechen bei der aktuellen
     Triangulierung (4. Position) mit bisher minimalen Anzahl, und nimmt
     die bessere (d.h. mit kleineren Flaechenzahl) Triangulierng*/



  static EDGE_Add *Liste;  /* "Liste" ist diejenige Liste, deren Eintraege zum
			   Aufbau der neuen Triangulierung verwendet werden*/
  static EDGE_Add *Liste1, *Liste2;
  static EDGE_Add **Map;   /* Eintraege aus "*Map[]" zeigen auf Eintraege aus
			  der "Liste"*/
  static EDGE_Add **Map1, **Map2;
  EDGE_Add *kante, *nexte, *kan_inv;
  static int kantenzahl, eckzahl=0;
  static int min_flaechenzahl;
  int flaechenzahl;
  int kantennr, i, j;
  int endk, startk;

  
  if(neu){

    if(eckzahl){
      free(Map);
      free(Liste);
    }

    eckzahl=triangulierung[0];
    kantenzahl=eckzahl*4-6;
    min_flaechenzahl=eckzahl;

    if((Liste1=(EDGE_Add*)malloc(kantenzahl*sizeof(EDGE_Add)))==NULL || (Liste2=(EDGE_Add*)malloc(kantenzahl*sizeof(EDGE_Add)))==NULL || (Map1=(EDGE_Add**)malloc(eckzahl*sizeof(EDGE_Add*)))==NULL || (Map2=(EDGE_Add**)malloc(eckzahl*sizeof(EDGE_Add*)))==NULL){
      fprintf(stderr," Speicherplatz reicht nicht aus (vergl.)\n");
      exit (2);
    }
    
    Liste=Liste1;
    Map=Map1;

    /*hier (in for(){..}) werden Flaechen angelegt (nur die aeusseren Kanten)*/
    for(j=1; j<3; j++){
      kantennr=0;
      for(i=0; i<eckzahl; i++){
	kante=Map[i]=Liste+kantennr;
	kantennr++;
	nexte=Liste+kantennr;
	kantennr++;
	
	kante->start=nexte->start=i;
	kante->next=kante->prev=nexte;
	nexte->next=nexte->prev=kante;

	kante->add=1;
	nexte->add=2;

	if(i){
	  nexte->invers=kan_inv;
	  nexte->end=i-1;
	  kan_inv->invers=nexte;
	  kan_inv->end=i;
	}
	
	kan_inv=kante;
      }
      kante=Map[0]->next;
      kante->end=kan_inv->start;
      kan_inv->end=0;
      kante->invers=kan_inv;
      kan_inv->invers=kante;
      
      /*gebe_graph(Map, eckzahl);*/
      
      Liste=Liste2;
      Map=Map2;
    }

  }

  kantennr=eckzahl*2;
  j=eckzahl*2-5;

  /*fprintf(stderr, " Triangulierungskanten:\n");*/
  /*hier (for{...}) werden Triangulierungskanten zugefuegt*/
  for(i=1; i<j; i=i+2){
    nexte=Liste+kantennr;
    kantennr++;
    kan_inv=Liste+kantennr; /* inverse der nexten */
    kantennr++;

    startk=triangulierung[i];
    endk=triangulierung[i+1];

    /*fprintf(stderr, "tk %d->%d\n", startk, endk);*/

    nexte->start=kan_inv->end=startk;
    nexte->end=kan_inv->start=endk;
    nexte->invers=kan_inv;
    kan_inv->invers=nexte;

    kan_inv->add=nexte->add=0;

    kante=Map[startk];
    while(!passt(kante->end, endk, kante->next->end, eckzahl)){
      /*fprintf(stderr, " %d->%d passt nicht zw %d->%d und %d->%d\n", kante->start, endk, kante->start, kante->end, kante->next->start, kante->next->end);*/
      kante=kante->next;
    }

    nexte->next=kante->next;
    nexte->prev=kante;
    kante->next->prev=nexte;
    kante->next=nexte;

    kante=Map[endk];
    while(!passt(kante->end, startk, kante->next->end, eckzahl)){
      /*fprintf(stderr, " %d->%d passt nicht zw %d->%d und %d->%d\n", kante->start, startk, kante->start, kante->end, kante->next->start, kante->next->end);*/
      kante=kante->next;
    }

    kan_inv->next=kante->next;
    kan_inv->prev=kante;
    kante->next->prev=kan_inv;
    kante->next=kan_inv;
  }
  
  /*fprintf(stderr, " \n");*/

  if(kantennr!=kantenzahl){
    fprintf(stderr, "kantennr!=kantenzahl (vergleich)\n");
    exit(3);
  }

  if(neu==2)
    flaechenzahl=1;
  else 
    flaechenzahl=zaehle_flaechen(Map, flaechen, flaechennummer, koord3, eckzahl);
  

  if(min_flaechenzahl>flaechenzahl){

    /*fprintf(stderr, "  verbesserung  fz=%d min_fz=%d\n", flaechenzahl, min_flaechenzahl);*/

    min_flaechenzahl=flaechenzahl;
    if(Liste==Liste1){
      triangulierungen[flaechennummer]=Map1;
      Liste=Liste2;
      Map=Map2;

      LIste[flaechennummer]=Liste1;
    }
    else{
      triangulierungen[flaechennummer]=Map2;
      Liste=Liste1;
      Map=Map1;

      LIste[flaechennummer]=Liste2;
    }
    
    if(min_flaechenzahl<3){
      /*fprintf(stderr, "  vertig\n");*/
      return 1;
    }
  }

  if(!neu){
    for(kantennr=eckzahl*2; kantennr<kantenzahl; kantennr++){
      kante=Liste+kantennr;
      kante->next->prev=kante->prev;
      kante->prev->next=kante->next;
    }
  }

  return 0;
}

/****************************************************************************/

int trianguliere(int eckzahl, int neu, int *trz, int tiefe, int flaeche[], 
		 SURFACE *flaechen, koordinaten_3d koord3, 
		 int triangulierung[], int flaechennummer){
  /* diese Funktion erzeugt alle moeglichen Triangulierungen der Flaeche
     mit der Nummer "flaechnnummer"*/
  
  static char verboten[FlaechenGroesse-1][FlaechenGroesse-1];
  int neue_flaeche[FlaechenGroesse-1];
  int i, j, l, k, m;
  static int eckzahl_anf, min_flaechenzahl;
  static int TrAnzahl[16]={0, 0, 0, 1, 2, 5, 14, 42, 132, 429, 1430, 4862, 16796, 58768, 208012, 742900};

  if(neu){
    eckzahl_anf=eckzahl;
    triangulierung[0]=eckzahl;
    min_flaechenzahl=eckzahl-2;
    for(i=0; i<eckzahl; i=i+1){
      flaeche[i]=i;
      for(j=0; j<eckzahl; j=j+1){ 
	verboten[i][j]=0;
      }
    }
  }

  if(eckzahl==3){
    j=tiefe*2-1;
    k=eckzahl_anf*2-5;

    (*trz)=(*trz)+1;

    if((*trz)==1){
      if(vergleich(1, flaechen, koord3, triangulierung, flaechennummer)==1){
	return 1;
      }
    }
    else{
      if(vergleich(0, flaechen, koord3, triangulierung, flaechennummer)==1)
	return 1;
    }
    
    if(eckzahl_anf<16){
      if(TrAnzahl[eckzahl_anf]>(*trz)){
	return 0;
      }
      else{
	return 1;
      }
    }
    
    return 0;
  }


  if(eckzahl==4)
    i=2;
  else
    i=0;

  do{
    l=i+2;
    k=flaeche[i];
    m=flaeche[l%eckzahl];
    if(!(verboten[k][m]) || (verboten[k][m] >= tiefe)){
      
      j=tiefe*2-1;
      triangulierung[j]=k;
      triangulierung[j+1]=m;

      for(j=0; j<(eckzahl-1) ; j++){
	neue_flaeche[j]=flaeche[(l+j)%eckzahl];
      }

      if(trianguliere(eckzahl-1, 0, trz, tiefe+1, neue_flaeche, flaechen, koord3, triangulierung, flaechennummer)){
	return 1;
      }

      verboten[k][m]=tiefe;
      verboten[m][k]=tiefe;
      
      if(eckzahl>4){
	for(j=0; j<(eckzahl-1); j++){
	  l=j+2;
	  if(verboten[(k=neue_flaeche[j])][(m=neue_flaeche[l%(eckzahl-1)])]>tiefe){
	    verboten[k][m]=0;
	    verboten[m][k]=0;
	  }		
	}
      }
    }
    i=i+1;
  }
  while(i<eckzahl);
  
  return 0;
}

/********************************************************************/

int trianguliere_flaechen(SURFACE *flaechen, koordinaten_3d koord3, 
			  int flaechenzahl){
  /* Diese Funktion trianguliert alle Flaechen (von der Anzahl "flaechenzahl")
     des Graphen (angegeben duch Listen der Knoten seiner Flaechen "flaechen")
     unter Beruechsichtigung der Symmetrien des Graphen */

  int flaeche[mKZdF];
  int *triangulierung; /* Eine Triangulierung wird durch eine Liste von
			   Dreiecken angegeben, d.h. trg[i], trg[i+1], trg[i+2]
			   i=1, 4, 7,... sind die Knotenpositionen 
			   (aufsteigend nach Groesse sortiert) der Dreiecke 
			   der Zerlegung.
			   tirangulierung[0]=(Eckzahl der Flaeche)*/
  int trz;
  int i, j, k, l;
  int eckzahl, kantenz;
  EDGE_Add *kante;

  EDGE ***numberings;
  int lcolour[MAXN]={0};
  int num_numberings=0;
  int num_numberings_or_pres=0;

  int *flaechengroessen;
  EDGE *kn, *skn, *knn, *min_kn;

  int min_knoten, z, sp, uzs, vertig;
  int position_neu, position_alt, knoten_neu, knoten_alt;
  int alloziert=1;

  
  nv=Knotenzahl;
  ne=Kantenzahl;

  if((numberings=(EDGE***)malloc(ne*2*sizeof(EDGE**)))==NULL){
    alloziert=0;
  }
  else{
    k=ne*2;
    for(i=0; i<k && alloziert; i++){
      if((numberings[i]=(EDGE**)malloc(ne*sizeof(EDGE*)))==NULL){
	alloziert=0;
      } 
    }

    if(!alloziert){
      for(i=i-2; i>0; i--){
	free(numberings[i]);
      }
      free(numberings);
    }
  }

  if((flaechengroessen=(int*)malloc((flaechenzahl+1)*sizeof(int)))==NULL || (LIste=(EDGE_Add**)malloc((flaechenzahl+1)*sizeof(EDGE_Add*)))==NULL){
    fprintf(stderr, "Speicher reicht nicht aus (tf1)\n");
    exit(4);
  }

  if(Knotenzahl>=MAXN){
    fprintf(stderr, " Der Graph ist zu gross\n");
    exit (2);
  }

  if(alloziert){
    knn=firstedge[0];
    
    for(i=0; i<Knotenzahl; i++){
      kn=skn=firstedge[i]=firstedge[i+1];
      do{
	kn->start=(kn->start)-1;
	kn->end=(kn->end)-1;
	kn=kn->next;
      }
      while(kn!=skn);
    }
    
    group(lcolour, numberings, &num_numberings, &num_numberings_or_pres);
    
    for(i=Knotenzahl; i>0; i--){
      kn=skn=firstedge[i]=firstedge[i-1];
      do{
	kn->start=(kn->start)+1;
	kn->end=(kn->end)+1;
	kn=kn->next;
      }
      while(kn!=skn);
    }

    firstedge[0]=knn;

    /*fprintf(stderr, " Automorphismen=%d, davon gegen die Laufrichtung %d\n", num_numberings, num_numberings_or_pres);*/
  }
  
  for(i=1; i<=flaechenzahl; i++){
    
    trz=0;

    for(j=0; flaechen[i][j]; j++);

    flaechengroessen[i]=j;

    if((triangulierung=(int*)malloc((flaechengroessen[i]*2-5)*sizeof(int)))==NULL){
      fprintf(stderr," Speicherplatz reicht nicht aus(tf1)\n");
    }

      
    if(i>1 && alloziert){
      /*fprintf(stderr," tf 10\n");*/
      
      kn=firstedge[flaechen[i][0]];
      j=flaechen[i][1];
      
      while((kn->end)!=j)
	kn=kn->next;
      
      for(j=0; j<Kantenzahl; j++){
	if(kn==numberings[0][j]){
	  sp=j;
	  break;
	}
      }
      /*fprintf(stderr," tf 15\n");*/
      
      vertig=0;
      for(z=1; z<num_numberings && !vertig; z++){
	
	/*fprintf(stderr," tf 16\n");*/
	
	if(z<num_numberings_or_pres){
	  uzs=1;
	}
	else{
	  uzs=0;
	}
	
	if(uzs)
	  min_kn=kn=skn=numberings[z][sp];
	else
	  min_kn=kn=skn=numberings[z][sp]->invers;
	
	min_knoten=skn->start;
	eckzahl=0;
	do{
	  eckzahl++;
	  
	  kn=kn->invers->prev;
	  if((kn->start) < min_knoten){
	    min_kn=kn;
	    min_knoten=kn->start;
	  } 
	}
	while(kn!=skn);
	
	/*fprintf(stderr," tf 20\n");*/
	
	flaeche[0]=min_knoten;
	kn=min_kn->invers->prev;
	for(j=1; kn!=min_kn ; j++){
	  flaeche[j]=kn->start;
	  kn=kn->invers->prev;
	}
	
	/*fprintf(stderr," tf 25\n");*/
	for(j=1; j<i; j++){
	  if(flaechengroessen[j]==eckzahl){
	    for(k=0; flaechen[j][k] && flaeche[k]==flaechen[j][k]; k++);
	    if(k==eckzahl)
	      break;
	  }
	}
	/*fprintf(stderr," tf 30\n");*/
	
	if(j<i){  /*d.h. wir koennen die Triangulierung von Flaeche[j][...]
		    auf Flaeche[i][...] uebertragen (unter einer bestimmten 
		    Knoten-Permutation) */
	  
	  
	  /*fprintf(stderr, " \t hier gewesen!!!!\n\n");*/
	  
	  knoten_alt=(numberings[z][sp])->start;
	  knoten_neu=(numberings[0][sp])->start;
	    
	  for(l=0; flaechen[j][l]!=knoten_alt; l++);
	  position_alt=l;
	  for(l=0; flaechen[i][l]!=knoten_neu; l++);
	  position_neu=l;
	  if(uzs){
	    for(l=0; l<eckzahl; l++)
	      flaeche[(position_alt+l)%eckzahl]=(position_neu+l)%eckzahl;
	  }
	  else{
	    for(l=0; l<eckzahl; l++){
	      flaeche[(position_alt+l)%eckzahl]=position_neu;
	      if(position_neu)
		position_neu=position_neu-1;
	      else 
		position_neu=eckzahl-1; 
	    }
	  }
	  
	  kantenz=eckzahl*4-6;
	  triangulierung[0]=eckzahl;
	  k=1;
	  for(l=eckzahl*2; l<kantenz; l=l+2){
	    kante=LIste[j]+l;
	    triangulierung[k]=flaeche[kante->start];
	    triangulierung[k+1]=flaeche[kante->end];
	    k=k+2;
	    /*fprintf(stderr, "%d %d\n", flaechen[i][kante->start], flaechen[i][kante->end]);*/
	  }
	  vergleich(2, flaechen, koord3, triangulierung, i);
	  vertig=1;
	}
	
      }
      
      if(!vertig){
	trianguliere(flaechengroessen[i], 1, &trz, 1, flaeche, flaechen, koord3, triangulierung, i);
      }
    }
    else{
      trianguliere(flaechengroessen[i], 1, &trz, 1, flaeche, flaechen, koord3, triangulierung, i);
    }
    
    free(triangulierung);
  }

  if(alloziert){
    for(i=Kantenzahl*2-1; i>0; i--){
      free(numberings[i]);
    }
    free(numberings);
  }

  return 1;
}
/* --- end include triang.c --------------------------------------------- */

void einbettung_flaeche(EDGE_DUAL *map_d[], int flaeche, int startflaeche,     
			SURFACE *flaechen, SURFACE *flaechen_neu, 	      
			koordinaten_3d koord3, koordinaten_2d koord2){
  
  EDGE_DUAL *kante_d;
  int position_A, position_B, position_C;   
  EDGE_Add **Map;
  EDGE_Add *reihenfolge[FlaechenGroesse];
  EDGE_Add *fl_kante;
  int i, flaechenzahl;
  int eingebettet[FlaechenGroesse]={0};

  Map=triangulierungen[flaeche];

  if(startflaeche){

    kante_d=map_d[flaeche];
    while((kante_d->end)!=startflaeche)
      kante_d=kante_d->next;
    position_A=kante_d->start_con[0];

    if((position_B=kante_d->start_con[1])!=((Map[position_A])->end)){
      fprintf(stderr, "position_B(=%d)!=Map[%d]->end(=%d)\n", position_B, position_A, Map[position_A]->end);
      exit (3);
    }
    
  }
  else{

    position_A=0;
    position_B=1;
    bette_C_ein(flaechen[flaeche][0], flaechen_neu[flaeche][0], flaechen[flaeche][1], flaechen_neu[flaeche][1], 0, 0, koord3, koord2);

  }


  eingebettet[position_A]=1;
  eingebettet[position_B]=1;
  
  reihenfolge[0]=Map[position_A];
  flaechenzahl=1;

  /*fprintf(stderr, "ef 6\n");*/

  for(i=0; i<flaechenzahl; ++i){
    fl_kante=reihenfolge[i];

    if(!eingebettet[fl_kante->next->end]){

      if(!(fl_kante->next->add)){ /* d.h. fl_kante->next nicht auf dem Rand*/
	reihenfolge[flaechenzahl]=fl_kante->next;
	++flaechenzahl;
      }

      if(!(fl_kante->next->invers->next->add)){ /* fl_kante->next->invers->next
						   nicht auf dem Rand*/
	reihenfolge[flaechenzahl]=fl_kante->next->invers->next;
	++flaechenzahl;
      }
      
    }
    else
      exit(0);
    position_A=fl_kante->start;
    position_B=fl_kante->end;
    position_C=fl_kante->next->end;

    bette_C_ein(flaechen[flaeche][position_A], flaechen_neu[flaeche][position_A], flaechen[flaeche][position_B], flaechen_neu[flaeche][position_B], flaechen[flaeche][position_C], flaechen_neu[flaeche][position_C], koord3, koord2);
    
    eingebettet[position_C]=1;
  }
  
  for(i=0; flaechen[flaeche][i]; ++i);

  if(i!=(flaechenzahl+2)){
    fprintf(stderr, " nicht alle Knoten der Flaeche %d wurden eingebettet\n", flaeche);
    exit(2);
  }
  
  /*fprintf(stderr, "ef 100\n");*/
}

/***************************************************************************/  

int korrektur_knotennummer(int angeklebt[][2], EDGE_DUAL *map_d[], 
			   SURFACE *flaechen_neu, koordinaten_2d *koord2, 
			   int *einbettungsreihenfolge,int knotenzahl, 
			   int letzte_pos, int flaechenzahl){
  /* letzte_pos ist die Position der Flaeche, deren Knoten ihre Nummer 
     weiterhin behalten werden, in der Einbettungsreihenfolge. Knoten, die 
     den Flaechen gehoeren, die in der Einbettungsreihenfolge nach letzte_pos
     stehen werden umnummeriert*/
  
  static int erster=1;
  int knoten, i, j;
  int kp0_start, kp1_start, kp0_end, kp1_end;
  int startflaeche, endflaeche;
  EDGE_DUAL *kante;
  
  if(!letzte_pos){
    knoten=0;
    startflaeche=einbettungsreihenfolge[1];
    for(j=0; ; ++j){
      if(flaechen_neu[startflaeche][j]){
	++knoten;
	flaechen_neu[startflaeche][j]=knoten; 
      }
      else
	break;
    }
    letzte_pos=1;
  }
  else{
    /* hier wird maximaler Knoten der Flaeche auf der Position letzte_pos in 
       der Einettungsriehenfolge ermittelt */
    startflaeche=einbettungsreihenfolge[letzte_pos];
    knoten=flaechen_neu[startflaeche][0];
    for(j=1; ; ++j){
      if(flaechen_neu[startflaeche][j]){
	if(flaechen_neu[startflaeche][j]>knoten)
	  knoten=flaechen_neu[startflaeche][j];
      }
      else
	break;
    }
  }

  for(i=letzte_pos+1; i<=flaechenzahl; ++i){
    startflaeche=einbettungsreihenfolge[i];
    kante=map_d[startflaeche];
    endflaeche=angeklebt[startflaeche][0]; /* Flaeche, von der aus die Flaeche 
					      "startflaeche" gebaut wird */
    while((kante->end)!=endflaeche)
      kante=kante->next;

    kp0_start=kante->start_con[0];
    kp1_start=kante->start_con[1];
    kp0_end=kante->end_con[0];
    kp1_end=kante->end_con[1];
    
    for(j=0; j<FlaechenGroesse; ++j){
      if(!flaechen_neu[startflaeche][j])
	break;
      if(j==kp0_start){
	flaechen_neu[startflaeche][j]=flaechen_neu[endflaeche][kp1_end];
      }
      else{
	if(j==kp1_start){
	  flaechen_neu[startflaeche][j]=flaechen_neu[endflaeche][kp0_end];
	}
	else{
	  ++knoten;
	  flaechen_neu[startflaeche][j]=knoten;
	}
      }
    }
  }

  
 
  if(erster){
    erster=0;
    if((*koord2=(koordinaten_2d)malloc((knoten+1)*2*sizeof(double)))==NULL){
      fprintf(stderr, " Speicher reicht nicht aus (korrektur 1)\n");
      exit(2);
    }
  }

  /*fprintf(stderr, "reihenfolge:\n");

    for(i=1; i<=(flaechenzahl); i++){
    fprintf(stderr, "%d ", einbettungsreihenfolge[i]);
    }
    fprintf(stderr, "\n");
    
    fprintf(stderr, "neue Flaechen\n");
    for(i=1; i<=(flaechenzahl); i++){
    fprintf(stderr, "%d:  ", i);
    for(k=0; k<FlaechenGroesse; k++){
    fprintf(stderr, "%d ", flaechen_neu[i][k]);
    if(flaechen_neu[i][k]==0)
    break;
    }
    fprintf(stderr, "\n");
    }*/

  return knoten;
}


/***************************************************************************/

int korrektur_einbettungsreihenfolge (int angeklebt[Max_Flaechenzahl][2], 
				      EDGE_DUAL *map_d[], int pos1, 
				      int flaechenzahl, 
				      int *einbettungsreihenfolge){

  /* Rueckgabewert ist die Position in der Einbettungsreihenfolge, von der aus 
     wir die Flaechen neu einbetten muessen*/

  int nein[2]; /* nein[1] ist die Nummer der Flaeche, die nicht eingebettet 
		  werden konnte, wenn man ihre Einbettung bei aktuellen
	          Einbettungsreihenfolge von der Flaeche nein[0] startet;
	          nein[0]=angeklebt[nein[1]][0];*/
  EDGE_DUAL *nein_kante, *kante, *startkante;
  int startpos; /* position, von der aus die Breitensuche fortgesetzt wird */
  int test, i;
  int eingetr;
  static char marks[Max_Flaechenzahl];

  marks[0]=1;
  
  do{
    
    nein[1]=einbettungsreihenfolge[pos1];
    nein[0]=angeklebt[nein[1]][0];
    
    startpos=angeklebt[nein[1]][1]+1;
    /*fprintf(stderr, " aufruf %d\n nein %d->%d\n pos1=%d, startpos=%d\n", aufruf, nein[0], nein[1], pos1, startpos);*/
    
    nein_kante=NULL;
    while(startpos==pos1){
      nein_kante=map_d[nein[0]];
      
      while((nein_kante->end)!=nein[1])
	nein_kante=nein_kante->next;
      
      for(i=1; i<pos1; ++i)
	marks[einbettungsreihenfolge[i]]=1;
      for(i=pos1; i<=flaechenzahl; ++i)
	marks[einbettungsreihenfolge[i]]=0;

      /*fprintf(stderr, "ok 2\n");*/

      kante=nein_kante->next;
      for(test=0; !test && kante!=map_d[nein[0]]; ){
	if(!marks[kante->end])
	  test=1;
	kante=kante->next;
      }
      /*fprintf(stderr, "ok 3\n");*/

      if(!test){
	if(nein[0]==einbettungsreihenfolge[1]){
	  fprintf(stderr, "Zusammenhaengender Aufschnitt dieses Objektes ist unmoeglich mit dieser Methode\n");
	  exit (1);
	}
	/*fprintf(stderr, "auch hier gewesen\n");*/
	pos1=startpos-1;
	nein[1]=nein[0];
	nein[0]=angeklebt[nein[1]][0];
	startpos=angeklebt[nein[1]][1]+1;
	nein_kante=NULL;
      }
      else{
	startpos=startpos-1;
      }
      /*fprintf(stderr, "hier gewesen ende\n");*/
    }
    
    for(i=1; i<pos1; ++i)
      marks[einbettungsreihenfolge[i]]=1;
    
    for(i=pos1; i<=flaechenzahl; ++i)
      marks[einbettungsreihenfolge[i]]=0;
    
    
    eingetr=pos1-1;
    if(nein_kante!=NULL){
      kante=nein_kante->next;
      while(kante!=nein_kante){
	if(!marks[kante->end]){
	  ++eingetr;
	  marks[kante->end]=1;
	  einbettungsreihenfolge[eingetr]=kante->end;
	  angeklebt[kante->end][0]=kante->start;
	  angeklebt[kante->end][1]=startpos;
	}
	kante=kante->next;
      }
      if(eingetr!=pos1-1)
	++startpos;
      else{
	fprintf(stderr, " dies sollte nicht passieren!\n");
	exit (1);
      }
    }
    
    for(i=startpos; i<=eingetr && eingetr<flaechenzahl; ++i){
      kante=startkante=map_d[einbettungsreihenfolge[i]];
      do{
	if(!marks[kante->end]){
	  ++eingetr;
	  marks[kante->end]=1;
	  einbettungsreihenfolge[eingetr]=kante->end;
	  angeklebt[kante->end][0]=kante->start;
	  angeklebt[kante->end][1]=i;
	}
	kante=kante->next;
      }
      while(kante!=startkante);
    }
    
    /*fprintf(stderr, " eingetr %d, startpos %d, pos1 %d\n", eingetr, startpos, pos1);*/
    
    if(startpos==pos1)
      pos1--;
    else{
      pos1=startpos;
    }
  }
  while(eingetr!=flaechenzahl);

  return pos1;
}

/*************************************************************************************/

int test_ueberschneidungsfreiheit(koordinaten_2d koord, EDGE_DUAL *map_d[], 
				  int flaechennummer,
				  SURFACE *flaechen_neu,
				  SURFACE *flaechen,
				  int *einbettungsreihenfolge, 
				  int angeklebt[Max_Flaechenzahl][2]){
  /* Prueft, ob der Graph ueberschneidungsfrei ist.
     Rueckgabewert: 1, falls der Graph ueberschneidungsfrei ist;
                    0, falls nicht, oder 'fremde' Knoten befinden sich zu nah
		       (sieh FAKTOR1, FAKTOR2) an anderen Kanten;*/
  int i, j, k, l, m, ende1, ende2, kl_fl, flaeche, flaeche2, endknoten, endknoten2;

  int i_alt, j_alt, ende1_alt, ende2_alt, geprueft;
  float laenge_i_j, laenge_e1_j, laenge_i_e2, laenge_e1_e2;
  float lange_i_e2_h2, laenge_e1_j_h2;

  EDGE_DUAL *kante1;
  float px1, px2, alfa, a, epsylon;
  
  float laenge_i_ende1, laenge_i_ende1_h2;

  float i0, i1, j0, j1, e10, e11, e20, e21;

  float  i0_m_e10, i1_m_e11, j0_m_e20, i0_m_e20, i1_m_e21, e10_m_e20, e11_m_e21, i0_m_j0, i1_m_j1, e10_m_j0, e11_m_j1;

 
  flaeche=einbettungsreihenfolge[flaechennummer];
  
  kl_fl=angeklebt[flaeche][0];
  kante1=map_d[flaeche];
  while((kante1->end)!=kl_fl)
    kante1=kante1->next;
      
  m=kante1->start_con[1];

  i=flaechen_neu[flaeche][m];
  i_alt=flaechen[flaeche][m];

  endknoten=flaechen_neu[flaeche][kante1->start_con[0]];
  
  /*fprintf(stderr, "uf a\n");*/
  
  while(i!=endknoten){
    ++m;
    /*fprintf(stderr, "uf m=%d\n", m);*/
    if(!flaechen_neu[flaeche][m])
      m=0;
    
    ende1=flaechen_neu[flaeche][m];
    ende1_alt=flaechen[flaeche][m];
    
    for(k=1; k<flaechennummer; ++k){
      /*fprintf(stderr, "uf k=%d\n", k);*/
      flaeche2=einbettungsreihenfolge[k];

      if(k==1){
	/* fuer k==1 muessen wir alle Kanten der flaeche2 auf Ueberschneidung
	   geprueft werden*/

	j=endknoten2=flaechen_neu[flaeche2][0];
	j_alt=flaechen[flaeche2][0];

	ende2=flaechen_neu[flaeche2][1];
	ende2_alt=flaechen[flaeche2][1];
	l=1;
      }
      else{
	kl_fl=angeklebt[flaeche2][0];
	kante1=map_d[flaeche2];
	while((kante1->end)!=kl_fl)
	  kante1=kante1->next;
      
	l=kante1->start_con[1];
	j=flaechen_neu[flaeche2][l];
	j_alt=flaechen[flaeche2][l];

	endknoten2=flaechen_neu[flaeche2][kante1->start_con[0]];
	
	++l;
	if(!flaechen_neu[flaeche2][l])
	  l=0;
	ende2=flaechen_neu[flaeche2][l];
	ende2_alt=flaechen[flaeche2][l];
      }

      do{

	i0=koord[i][0];
	i1=koord[i][1];
	
	j0=koord[j][0];
	j1=koord[j][1];
	
	e10=koord[ende1][0];
	e11=koord[ende1][1];
	
	e20=koord[ende2][0];
	e21=koord[ende2][1];

	i0_m_e10=i0-e10;
	i1_m_e11=i1-e11;

	laenge_i_ende1_h2=i0_m_e10*i0_m_e10+i1_m_e11*i1_m_e11;

	geprueft=0;
 
	
	if(i_alt==ende2_alt ){
	  /*fprintf(stderr, "hier gewesen(1.5)\n");*/
	  
	  i0_m_e20=i0-e20;
	  i1_m_e21=i1-e21;
	  
	  if((lange_i_e2_h2=(i0_m_e20*i0_m_e20+i1_m_e21*i1_m_e21)) < (epsylon=(laenge_i_ende1_h2/FAKTOR2_h2))){   /* d.h. i ist nah genug am ende2 */
	    
	    if(ende1_alt==j_alt){
	      e10_m_j0=e10-j0;
	      e11_m_j1=e11-j1;

	      if((laenge_e1_j_h2=(e10_m_j0*e10_m_j0+e11_m_j1*e11_m_j1)) < epsylon){
		/* d.h. "identische" Kanten liegen praktisch aufeinander,
		   weitere Pruefung dieser ist nicht erfordelich*/
		geprueft=1;
	      }
	      else{
		if(laenge_e1_j_h2 > laenge_i_ende1_h2/FAKTOR1_h2){
		  geprueft=1;
		}
		else{
		  /*fprintf(stderr, " i=e2, e1=j, j zu nah an e1\n");*/
		  return 0;
		}
	      }
	    }
	    else{
	      e10_m_j0=e10-j0;
	      e11_m_j1=e11-j1;
	      
	      if((e10_m_j0*e10_m_j0+e11_m_j1*e11_m_j1) > laenge_i_ende1_h2/FAKTOR1_h2){
		geprueft=1;
	      }
	      else{ /* ungleiche Knoten zu nah aneinander*/
		/*fprintf(stderr, " i=e2, e1!=j\n");*/
		return 0;
	      }
	    }
	  }
	  else{
	    if(ende1_alt==j_alt){
	      e10_m_j0=e10-j0;
	      e11_m_j1=e11-j1;
	      
	      if((e10_m_j0*e10_m_j0+e11_m_j1*e11_m_j1) < epsylon){

		if(lange_i_e2_h2 > laenge_i_ende1_h2/FAKTOR1_h2)
		  geprueft=1;
		else{
		  /*fprintf(stderr, " i=e2, e1=j, i zu nah an e2 \n");*/
		  return 0;
		}
	      }
	    }
	  }
	}
	else{
	  if(ende1_alt==j_alt){
	    e10_m_j0=e10-j0;
	    e11_m_j1=e11-j1;
	    
	    if((e10_m_j0*e10_m_j0+e11_m_j1*e11_m_j1) < laenge_i_ende1_h2/FAKTOR2_h2){
	      /* d.h. "identische" Knoten j und ende1 liegen praktisch 
		 aufeinander */      
	      i0_m_e20=i0-e20;
	      i1_m_e21=i1-e21;
	      
	      if((i0_m_e20*i0_m_e20+i1_m_e21*i1_m_e21) > laenge_i_ende1_h2/FAKTOR1_h2){
		geprueft=1;
	      }
	      else{   /* ungleiche Knoten zu nah aneinander*/
		/*fprintf(stderr, " e1=j1, e2!=i\n");*/
		return 0;
	      }
	    }
	  }
	  else{
	    if(i_alt==j_alt){
	      i0_m_j0=i0-j0;
	      i1_m_j1=i1-j1;

	      if((i0_m_j0*i0_m_j0+i1_m_j1*i1_m_j1) < laenge_i_ende1_h2/FAKTOR2_h2){
		e10_m_e20=e10-e20;
		e11_m_e21=e11-e21;
		
		if((e10_m_e20*e10_m_e20+e11_m_e21*e11_m_e21) > laenge_i_ende1_h2/FAKTOR1_h2){
		  geprueft=1;
		}
		else{
		  /*fprintf(stderr, " i=j\n");*/
		  return 0;
		}
	      }
	    }
	    else{
	      if(ende1_alt==ende2_alt){
		e10_m_e20=e10-e20;
		e11_m_e21=e11-e21;
		
		if((e10_m_e20*e10_m_e20+e11_m_e21*e11_m_e21) < laenge_i_ende1_h2/FAKTOR2_h2){
		  i0_m_j0=i0-j0;
		  i1_m_j1=i1-j1;

		  if((i0_m_j0*i0_m_j0+i1_m_j1*i1_m_j1) > laenge_i_ende1_h2/FAKTOR1_h2){    
		    geprueft=1;
		  }
		  else{
		    /*fprintf(stderr, " e1=e2\n");*/
		    return 0;
		  }
		}  
	      }
	    }
	  }
	}
	
	if((!geprueft) && (ende1!=j) && (ende1!=ende2) && (i!=j) && (ende2!=i)){

	  j0_m_e20=j0-e20;

	  /* pruefe, ob i und ende1 in verschiedenen (durch j-ende2 defi-
	     nierten) Halbraeumen  liegen */
	  if((j0_m_e20)==0.0){
	    /* wenn Steigung der Geraden ueber j und ende2 unendlich ist, 
	       reicht es nur die ersten Koordinaten zu vergleichen */
	    /*fprintf(stderr, " drehe(1)(%d->%d  %d->%d)\n", j, ende2, i, ende1);*/
	    alfa=j0;
	    px1=i0;
	    px2=e10;
	  }
	  else{
	    a=(j1-e21)/(j0_m_e20);
	    alfa=-a*j0+j1;
	    px1=-a*i0+i1;
	    px2=-a*e10+e11;
	  }
	  
	  if((px1<=alfa && px2>=alfa) || (px2<=alfa && px1>=alfa)){
	    
	    /* pruefe, ob j und ende2 in verschiedenen (durch i-ende1 defi-
	       nierten) Halbraeumen  liegen */

	    if((i0_m_e10)==0.0){
	      /* wenn Steigung der Geraden ueber i und ende1 unendlich, 
		 reicht es nur die ersten Koordinaten zu vergleichen*/
	      /*fprintf(stderr, " drehe(2)(%d->%d  %d->%d)\n", j, ende2, i, ende1);*/
	      alfa=i0;
	      px1=j0;
	      px2=e20;
	      
	    }
	    else{
	      a=(i1_m_e11)/(i0_m_e10);
	      alfa=-a*i0+i1;
	      px1=-a*j0+j1;
	      px2=-a*e20+e21;
	      
	    }
	    
	    if((px1<=alfa && px2>=alfa) || (px2<=alfa && px1>=alfa)){
	      if(px1==alfa && px2==alfa){
		/* das ist der Fall, wo alle Knoten (i, j, ende1, ende2)
		   auf einer Geraden liegen*/
		if(i0>e10){
		  if((j0<i0 && j0>e10) || (e20<i0 && e20>e10)){

		    /*fprintf(stderr, "%d->%d und %d->%d schneiden sich\n", i_alt, ende1_alt, j_alt, ende2_alt);*/
		    return 0;
		  }
		}
		else{
		  if(i0<e10){
		    if((j0>i0 && j0<e10) || (e20>i0 && e20<e10)){
		      
		      /*fprintf(stderr, "%d->%d und %d->%d schneiden sich\n", i_alt, ende1_alt, j_alt, ende2_alt);*/
		      return 0;
		    }
		  }
		  else{
		    if(i1>e11){
		      if((j1<i1 && j1>e11) || (e21<i1 && e21>e11)){
			/*fprintf(stderr, "%d->%d und %d->%d schneiden sich\n", i_alt, ende1_alt, j_alt, ende2_alt);*/
			return 0;
		      }
		    }
		    else{
		      if(i1<e11){
			if((j1>i1 && j1<e11) || (e21>i1 && e21<e11)){
			  /*fprintf(stderr, "%d->%d und %d->%d schneiden sich\n", i_alt, ende1_alt, j_alt, ende2_alt);*/
			  return 0;
			}
		      }
		      else{ /* eigentlich unmoeglich */
			fprintf(stderr, " Eingabefehler: Knoten %d und %d haben gleiche Koordinaten\n", i, ende1);
			exit (3);
		      }
		    }      
		    
		  }
		}
	      }
	      else{
		/*fprintf(stderr, " %d->%d und %d->%d schneiden sich\n", i_alt, ende1_alt, j_alt, ende2_alt);*/
		return 0;
	      }
	    }
	  } 

	  laenge_i_ende1=sqrt(laenge_i_ende1_h2);
	  
	  epsylon=laenge_i_ende1/FAKTOR1;
	  
	  /* in if(...) pruefen wir ob Knoten ende2 oder Knoten j zu nah
	     an der Kante i->ende1 liegt; wenn ja - return 0 */
	  
	  i0_m_e20=i0-e20;
	  i1_m_e21=i1-e21;
	  e10_m_e20=e10-e20;
	  e11_m_e21=e11-e21;

	  laenge_i_e2=sqrt(i0_m_e20*i0_m_e20+i1_m_e21*i1_m_e21);
	  laenge_e1_e2=sqrt(e10_m_e20*e10_m_e20+e11_m_e21*e11_m_e21);

	  if(laenge_i_e2+laenge_e1_e2-laenge_i_ende1 < epsylon){
	    /* ende2 zu nah an i->ende1*/
	    /*fprintf(stderr, "ende2 zu nah an i->ende1\n");*/
	    return 0;
	  }
	  else{
	    
	    i0_m_j0=i0-j0;
	    i1_m_j1=i1-j1;
	    e10_m_j0=e10-j0;
	    e11_m_j1=e11-j1;

	    laenge_i_j=sqrt(i0_m_j0*i0_m_j0+i1_m_j1*i1_m_j1);
	    laenge_e1_j=sqrt(e10_m_j0*e10_m_j0+e11_m_j1*e11_m_j1);

	    if(laenge_i_j+laenge_e1_j-laenge_i_ende1 < epsylon){
	      /* j zu nah an i->ende1*/
	      /*fprintf(stderr, "j zu nah an i->ende1\n");*/
	      return 0;
	    }
	  }

	}

	j=ende2;
	j_alt=ende2_alt;

	++l;
	if(!flaechen_neu[flaeche2][l])
	  l=0;
	
	ende2=flaechen_neu[flaeche2][l];
	ende2_alt=flaechen[flaeche2][l];
     }
      while(j!=endknoten2);
    }
    
    i=ende1;
    i_alt=ende1_alt;
  }

  return 1;
  
}


/***************************************************************************/

int einbettung_aufschnitt(koordinaten_3d koord3, koordinaten_2d *koord2, 
			  EDGE_DUAL *map_d[], SURFACE *flaechen, 
			  int flaechenzahl, int knotenzahl,
			  char graph_d[Max_Flaechenzahl][Max_Flaechenzahl], 
			  SURFACE **flaechen_neu, int *einbettungsreihenfolge){
  

  int *marks, *eingebettet;
  int angeklebt[Max_Flaechenzahl][2]; /* angeklebt[i][0] ist die Nummer der
					 Flaeche, von der die Flaeche i 
					 aufgebaut wird;
					 angeklebt[i][1] ist die Position von
					 angeklebt[i][0] in der Einbettungs-
					 reihenfolge */
  EDGE_DUAL *kante, *startkante;
  int eingetr, i, j, k, min_knotenzahl, min_flaeche;
  
  int aufruf=0;
  
  int knotenzahl_neu;

#ifndef NOTIMES
  time_t st_zeit, akt_zeit;
#endif //NOTIMES

  int zeit_test=0;

#ifndef NOTIMES
  time(&st_zeit);
  //  srand(time(NULL));
#endif //NOTIMES

  if((marks=(int*)malloc((flaechenzahl+1)*sizeof(int)))==NULL || (eingebettet=(int*)malloc((flaechenzahl+1)*sizeof(int)))==NULL || (*flaechen_neu=(SURFACE*)malloc((flaechenzahl+1)*sizeof(SURFACE)))==NULL){
    fprintf(stderr," Speicher reicht nicht aus(einb. aufschn.)(1) \n");
    exit(1);
  }

  
  memcpy(*flaechen_neu, flaechen, (size_t)(flaechenzahl+1)*sizeof(SURFACE));

  /*fprintf(stderr, " Flaechen davor\n");
    for(i=1; i<=(flaechenzahl); i++){
    fprintf(stderr, "%d:  ", i);
    for(k=0; k<FlaechenGroesse; k++){
    fprintf(stderr, "%d ", (*flaechen_neu)[i][k]);
    if((*flaechen_neu)[i][k]==0)
    break;
    }
    fprintf(stderr, "\n");
    }*/

  for(i=0; i<=flaechenzahl; ++i){
    marks[i]=0;
    angeklebt[i][0]=angeklebt[i][1]=0;
  }

  for(i=1; i<=flaechenzahl; ++i){
    for(j=0; flaechen[i][j]; ++j);
    if(i==1){
      min_knotenzahl=j;
      min_flaeche=1;
    }
    else{
      if(j<min_knotenzahl){
	min_knotenzahl=j;
	min_flaeche=i;
      }
    }
  }
  
  einbettungsreihenfolge[1]=min_flaeche;
  marks[min_flaeche]=marks[0]=1;
  eingetr=1;
  angeklebt[min_flaeche][0]=angeklebt[min_flaeche][1]=0;
  angeklebt[0][0]=angeklebt[0][1]=0;

  for(i=1; i<=eingetr && eingetr<flaechenzahl; ++i){
    kante=startkante=map_d[einbettungsreihenfolge[i]];
    do{
      if(!marks[kante->end]){
	++eingetr;
	marks[kante->end]=1;
	einbettungsreihenfolge[eingetr]=kante->end;
	angeklebt[kante->end][0]=kante->start;
	angeklebt[kante->end][1]=i;
      }
      kante=kante->next;
    }
    while(kante!=startkante);
  }


  
  if(eingetr!=flaechenzahl){
    fprintf(stderr, " Fehler Einbettungsreihenfolge eingetr=%d, flaechenzahl=%d\n", eingetr, flaechenzahl);
    exit(3);
  }

  /*fprintf(stderr, "\n");
    for(i=1; i<=flaechenzahl; i++){
    fprintf(stderr, "%d ", einbettungsreihenfolge[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "\n");*/

  /*for(i=1; i<=flaechenzahl; i++){
    fprintf(stderr, "%d angeklebt an %d\n ", i, angeklebt[i][0]);
    }
    fprintf(stderr, "\n");*/


  knotenzahl_neu=knotenzahl;

  knotenzahl_neu=korrektur_knotennummer(angeklebt, map_d, *flaechen_neu, koord2, einbettungsreihenfolge, knotenzahl_neu, 0, flaechenzahl);
  
  
  for(i=1; i<=flaechenzahl; ++i){
    j=einbettungsreihenfolge[i];

    /*fprintf(stderr, " (1) 11 \n");*/

    einbettung_flaeche(map_d, j, angeklebt[j][0], flaechen, *flaechen_neu, koord3, *koord2);

    /*fprintf(stderr, " (1) 12 \n");*/

    if(i>1){
      if(!test_ueberschneidungsfreiheit(*koord2, map_d, i, *flaechen_neu, flaechen, einbettungsreihenfolge, angeklebt)){
	++aufruf;

	/*fprintf(stderr, " ende aufruf %d \t flaechenzahl=%d\n", aufruf, flaechenzahl);*/
	
	//if(!(aufruf%200)){ entfernt von Gunnar Brinkmann -- warum benutzt er staendig srand()?
	//  srand(time(NULL)); Ich habe es einmal in main eingefuegt.
	//}

#ifndef NOTIMES
	if((st_zeit!=-1) && (!zeit_test) && (aufruf>100)){
	  time(&akt_zeit);
	  if(akt_zeit!=-1){
	    if(difftime(akt_zeit, st_zeit) > ZEITLIMIT){
	      /*fprintf(stderr, " bin hier\n");*/
		/*exit(2);*/
	      zeit_test=1;
	    }
	  }
	}
#else
	if (aufruf> 2000) zeit_test=1; // das ist nur geraten...
#endif //NOTIMES

	if(zeit_test){
	  while((angeklebt[j][0]!=min_flaeche) && ((rand()%100)<Wkeit)){
	    /* mit Wahrscheinlichkeit von 10% wird der Teilbaum mit Wurzel
	       in angeklebt[i][0] verworfen*/
	    /*fprintf(stderr, " \t bin hier2\n");*/
	    i=angeklebt[j][1];
	    j=einbettungsreihenfolge[i];
	  }
	}

	i=korrektur_einbettungsreihenfolge(angeklebt, map_d, i, flaechenzahl, einbettungsreihenfolge)-1;


	knotenzahl_neu=korrektur_knotennummer(angeklebt, map_d, *flaechen_neu, koord2, einbettungsreihenfolge, knotenzahl_neu, i, flaechenzahl);

      }
    }
  }
  
  /*fprintf(stderr, "reihenfolge:\n");
  
  for(i=1; i<=(flaechenzahl); i++){
    fprintf(stderr, "%d ", einbettungsreihenfolge[i]);
  }
  fprintf(stderr, "\n");
  
  fprintf(stderr, "neue Flaechen\n");
  for(i=1; i<=(flaechenzahl); i++){
    fprintf(stderr, "%d:  ", i);
    for(k=0; k<FlaechenGroesse; k++){
      fprintf(stderr, "%d ", (*flaechen_neu)[i][k]);
      if((*flaechen_neu)[i][k]==0)
	break;
    }
    fprintf(stderr, "\n");
    }*/
  
  angeklebt[min_flaeche][0]=angeklebt[min_flaeche][1]=0;

  for(i=1; i<=flaechenzahl; ++i){
    j=einbettungsreihenfolge[i];
    k=angeklebt[j][0];
    graph_d[j][k]=2;  /*wenn Flaeche j an die Flaeche k "angeklebt" wird*/
    graph_d[k][j]=1;
    
    /*fprintf(stderr, " %d klebt an %d\n", j, k);*/
    
  }

  return knotenzahl_neu;
}


/*******************************************************************/

int main(int argc, char *argv[]){

  EDGE **map;
  EDGE_DUAL **map_d;

  int test, knotenzahl, flaechenzahl;
  FILE *fil2;           /* Zeiger auf den Ausgabe-File (im Vega-Format)*/
  FILE *fil;            /* Zeiger auf Input-File (im Vega-Format)*/


  srand(0);

  koordinaten_3d koord3;
   
  SURFACE *flaechen;

  int knotenzahl_neu;
  koordinaten_2d koord2;

  char graph_d[Max_Flaechenzahl][Max_Flaechenzahl]={{0}};

  int *einbettungsreihenfolge;

  SURFACE *flaechen_neu;
  int groesste_fl;
  char *cmd;

  cmd=*argv;
  groesste_fl=FlaechenGroesse;

  print_header=1;
  pagelabel=pageno="1";

  for(++argv, --argc; argc>0; ++argv, --argc){
    if(strcmp(*argv, "-n")==0){
      print_header=0;
    }
    else if(strcmp(*argv, "-p")==0){
      pagelabel=argv[1];
      pageno=argv[2];
      argc-=2;
      argv+=2;
    }
    else if(strcmp(*argv, "-s")==0){
      int i;
      if (sscanf(argv[1], "%d", &i) == 1) {
	if (i < FlaechenGroesse) {
	  groesste_fl = i;
	} else {
	  fprintf (stderr, "Warnung: Zahl hinter -s muss kleiner als %d sein (ignoriert)\n", FlaechenGroesse);
	}
      }
      argc-=1;
      argv+=1;
    }
    else{
      break;
    }
  }

  fil = stdin;

  if(argc > 0) {
    if(strcmp (*argv, "-") != 0) {
      fil = fopen(*argv, "r");
      if(fil==NULL){
	fprintf(stderr, "%s: can't open '%s' for input\n", cmd, *argv);
	exit(2);
      }
      --argc;
      ++argv;
    }
  }    

  fil2 = stdout;

  if(argc>0) {
    if(strcmp(*argv, "-") != 0) {
      fil2 = fopen(*argv, "w");
      if (fil2 == NULL) {
	fprintf (stderr, "%s: can't open '%s' for output\n", cmd, *argv);
	exit (2);
      }
      --argc;
      ++argv;
    }
  }
  
  if((test=lese_vega(fil, &map, &knotenzahl, &koord3))==1 ){

    mache_flaechen(&map_d, map, &flaechenzahl, &flaechen, knotenzahl, groesste_fl);

    if(flaechenzahl>=Max_Flaechenzahl){
      fprintf(stderr," Der Graph hat zu viele Flaechen\n");
      exit (3);
    }

    if((einbettungsreihenfolge=(int*)malloc((flaechenzahl+1)*sizeof(int)))==NULL){
    
      fprintf(stderr," Speicher reicht nicht aus(aufschn.(main))(1) \n");
      exit(2);
    }  

    if((triangulierungen=(EDGE_Add***)malloc((flaechenzahl+1)*sizeof(EDGE_Add**)))==NULL){
      fprintf(stderr," Speicherplatz reicht nicht aus(main 2)\n");
    }
    
    trianguliere_flaechen(flaechen, koord3, flaechenzahl);

    /*fprintf(stderr,"  vor einbettung_aufschn\n");*/

    knotenzahl_neu=einbettung_aufschnitt(koord3, &koord2, map_d, flaechen, flaechenzahl, knotenzahl, graph_d, &flaechen_neu, einbettungsreihenfolge);

    /*fprintf(stderr,"  nach einbettung_aufschn\n");*/

    mache_vega2d(fil2, koord2, koord3, map, map_d, graph_d, knotenzahl_neu, flaechen_neu, flaechen, flaechenzahl, einbettungsreihenfolge);
  }

  fclose(fil);
  fclose(fil2);
  /*fclose(fil1);*/
  
  return 0;

}

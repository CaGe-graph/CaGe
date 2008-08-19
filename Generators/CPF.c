/*========================================================================*/

/* CPF.c (ehemals 3reggen.c) */
/* Version 1.3 */
/* Thomas Harmuth */
/* 14.November 1997 */

/* Dieses Programm generiert 3-regulaere, planare Graphen mit vorgegebenen
   Flaechen. Bedienung siehe Anleitung. */

/* Aenderung gegenueber Version 1.2:  Die Option "pathface_max" entfaellt
   und der Priority-default wird anders gesetzt. Beim Isomorphietest
   entfaellt das bisher wichtigste Kriterium, damit auf die Konstruktion
   von Graphen entlang von Bauchbinden manchmal verzichtet werden kann.
   Desweiteren wird die groesste Flaechenzahl in einem Zielgraphen errechnet,
   in dem ein Bauchbindenpfad der am besten bewertete Pfad sein kann.
   Falls bauchbindenkennung<3, so ist dies wie bisher die groesste gewuenschte
   Flaechenzahl. Falls bauchbindenkennung==3, so ist dies die groesste
   gewuenschte GERADE Flaechenzahl, fuer die theoretisch Graphen existieren
   (is_moeglich==True), da es in Graphen mit ungerader Flaechenzahl nicht
   nur Bauchbinden geben kann. Die Flaechenzahl, die man erhaelt, heisst
   "f_max_bb_best" und hat Auswirkungen auf das Vollstaendigkeitskriterium.
   Man koennte sie auch fuer Kantenkriterien einsetzen, aber diese sind
   sowieso schwaecher als das Vollstaendigkeitskriterium. */


/*************/
/* Includes: */
/*************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <memory.h>
#include<sys/times.h>
#include<sys/stat.h>
#include<malloc.h>


/******************************************/
/* User-modifizierbare define-Konstanten: */
/******************************************/

#define N_MAX    500       /* Maximal moegliche Anzahl der Knoten */
#define F_MAX    50        /* Maximale Groesse einer Flaeche */ 
#define V_MAX    10        /* Fuer einen Patch werden alle moeglichen Ver-
 knuepfungen gespeichert, jedoch maximal V_MAX gleichartige Verknuepfungen.
 V_MAX muss mindestens 4 sein, damit in der Funktion "generiere_bind_tabelle"
 keine Ueberlaeufe in den Faellen auftreten, wo "anz" nicht abgefragt wird */
#define FILENAMENLAENGE 255
#define MAXFTYPEN 50        /* maximale Anzahl verschiedener Flaechentypen */
#define MAXFTYPEN_FSTAT 15  /* maximale Anzahl verschiedener Flaechentypen,
                               wenn die Option "facestat" benutzt wird oder
                               Flaechenzahlen eingeschraenkt werden */
#define MEMBLOCKSIZE 32768L /* Groesse eines dynamischen Speicherblocks */ 
                      /* Mindestgroesse: Groesse des groessten Elements */


/*******************************************/
/* nicht modifizierbare define-Konstanten: */
/*******************************************/

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 0
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1
#endif

/*
#ifdef __sgi
#define ENDIAN_OUT BIG_ENDIAN
#define ENDIAN_IN  BIG_ENDIAN
#else
#ifdef sun
#define ENDIAN_OUT BIG_ENDIAN
#define ENDIAN_IN BIG_ENDIAN
#else
  |* BIG_ENDIAN list is probably not complete *|
#define ENDIAN_OUT LITTLE_ENDIAN
#define ENDIAN_IN  LITTLE_ENDIAN
#endif
#endif  
*/
static unsigned short word = (BIG_ENDIAN << 8) | LITTLE_ENDIAN;
# define ENDIAN_OUT ((int) * (char *) &word)

#define F_MAXANZ ((N_MAX>>1)+2) /* Maximal moegliche Anzahl der Flaechen */
#define aussen   (N_MAX+2)      /* so werden Kanten nach aussen markiert */
#define FL_MAX   UCHAR_MAX    /* absolutes Limit fuer eine Flaeche, wobei
                                 unsigned char der Typ einer Flaeche ist */
#define KN_MAX   USHRT_MAX/3  /* absolutes Limit fuer die Knotenzahl, wobei
                                  unsigned short der Typ eines Knotens ist. 
  - Eine Division durch 2 waere mindestens
    notwendig, damit beim Addieren zweier Zahlen vom Typ KNOTENTYP die
    hoechste erlaubte Zahl nicht ueberschritten wird (z.B. beim Zusammen-
    naehen von Patches, wenn die Knotenzahlen addiert werden). Allerdings
    ist dies nur von Belang, wenn das Ergebnis auch in einer Variablen vom
    Typ KNOTENTYP gespeichert werden soll.
  - Eine Division durch 2 waere mindestens notwendig, damit eine Variable
    vom Typ EULERTYP den Inhalt einer Variablen vom Typ KNOTENTYP uebernehmen
    koennte.
  - Eine Division durch 3 waere mindestens notwendig, weil die Variable "start"
    vom Typ KNOTENTYP ist und 3*N_MAX Kanten ansteuert.
  - Eine Division durch 3 ist mindestens notwendig, weil die Bordercodes,
    die eine Laenge von N_MAX haben koennen, dreimal hintereinander gespeichert
    werden. Dies ist allerdings nur von Belang, wenn die Variable, die die
    Eintraege ansteuert, vom Typ KNOTENTYP ist.
  - Eine Division durch 3 ist mindestens notwendig, weil die Variable
    "krit_max" im Extremfall direkt von der Knotenzahl der beteiligten
    Flaechen abhaengt, und die kann maximal 3*N_MAX sein (je drei Knoten
    werden miteinander identifiziert). 
  - Eine Subtraktion mit 2 waere mindestens notwendig wegen der Definition
    von "aussen". */
#define False    0
#define True     1
#define nil      0
#define PV_DEFAULT 20    /* default-Wert fuer Option "pv" */

#ifdef __osf__ /* OSF auf den alphas */
#define time_factor CLK_TCK
#endif
#ifdef __sgi /* Silicon Graphics */
#define time_factor CLK_TCK
#endif
#ifdef __linux   /* Linux */
#define time_factor CLK_TCK
#endif
#ifdef sun /* Sun */
#define time_factor 60
#endif
#ifdef __hpux /* Hewlet Packard */
#define time_factor CLK_TCK
#endif
#ifdef __ultrix /* DEC */
#define time_factor 60
#endif
#ifndef time_factor
#define time_factor CLK_TCK
#endif


/***********/
/* Makros: */
/***********/

#define CODESIZE(n) (((size_t)(n)<<2)+1)  
   /* Laenge eines planarcodes fuer 3-regulaere Graphen mit n Knoten. */
   /* Dies ist gleichzeitig eine obere Schranke fuer die Laenge des
      Dualen, denn die Anzahl der Eintraege im Dualen ist:
         1           (Flaechenzahl)
      +  f           (Anzahl der abschliessenden Nullen)
      +  \sum i*p_i  (Adjazenzen zwischen Flaechen)
      mit \sum i*p_i = \sum (i-6)*p_i + \sum 6*p_i 
                     =      -12       +     6f
      Also 1 + f + \sum i*p_i = 7f - 11 = 7/2*n + 3 <= CODESIZE(n) */

#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)<(b) ? (b) : (a))
#define ABS(a)   ((a)<0 ? -(a) : (a))


/**********************/
/* Typ-Deklarationen: */
/**********************/

typedef char BOOL;                 /* von 0 verschieden entspricht True, 
   es sollen aber nur die Werte 0 und 1 benutzt werden, um mit dem Wahr-
   heitswert rechnen zu koennen. */
typedef unsigned short KNOTENTYP;  /* fuer Knotennummern und Randcodierungen */
typedef unsigned char FLAECHENTYP;   /* fuer die Groesse einer Flaeche */
typedef signed short EULERTYP;       /* fuer Zahlen, die mit KNOTENTYP
                          assoziiert werden und negativ werden koennen */

typedef struct elem {          /* zur Speicherung eines Patches */ 
          struct elem *next;   /* naechster Patch mit demselben Bordercode */
          struct elem *prev1;  /* Zeiger auf den ersten Vorgaenger */
          struct elem *prev2;  /* Zeiger auf den zweiten Vorgaenger */
          KNOTENTYP *flaechenzahl;    /* Zeiger auf Array mit Flaechenzahlen */
          KNOTENTYP i;         /* falls prev1!=nil: Drehung des ersten Vorgaen-
            gers, falls prev1==nil: Die Struktur repraesentiert ein i-Eck */
          KNOTENTYP j;  /* falls prev1!=nil: Drehung des zweiten Vorgaengers */
          KNOTENTYP nahtlen;   /* Laenge der Naht bei Durchschnitten, Naht
                                  bis zum 3er-Treffpunkt bei Einschluessen */
          KNOTENTYP ziellen;   /* Weglaenge zwischen den beiden Nahtenden (von
                                  der Markierung entgegen des Uhrzeigersinns)
                                  bei Durchschnitten, Laenge der Schlinge bei
                                  Einschluessen */
          unsigned char art;   /* Art der Verknuepfung:
                                  0 = gerade Verknuepfung,
                                  1 = ungerade Verknuepfung,
                                  2 = Einschluss 1a,
                                  3 = Einschluss 1b,
                                  4 = Einschluss 2a,
                                  5 = Einschluss 2b           */
          char test; /* Bit 0 gesetzt:  Patch wurde mittelbar benutzt */
                     /* Bit 1 gesetzt:  Patch wurde unmittelbar benutzt */ 
        } ELEM;

typedef struct treenode {     /* Verzweigung im Baum mit Bruchkantenpatches */
          struct treenode *next;   
               /* fuer naechsten Zweig mit bis dahin gleichem Code */
          struct elem *firstpatch;  
               /* zugehoerige Patches (nil = noch keine gefunden) */
          struct treenode *nextlevel;  /* weitere Patches mit laengerem Code */
          KNOTENTYP code;  /* naechster Eintrag des Bordercodes */
          KNOTENTYP wh;    /* dieser Wert gibt an, nach wie vielen Eintraegen
               sich der Bordercode wiederholt (wenn er mit "code" endet)
               klar:  wh<=len(bordercode) */
        } TREENODE;

typedef struct bbtreenode {       /* Verzweigung im Baum mit den BB-Patches */
          struct bbtreenode *nextlevel;    /* Verzweigung von Level 0 auf 1 */
          struct bbtreenode *next;   /* naechster Zweig, selbe Flaechenzahl */
          KNOTENTYP code;    /* Bordercode, der zur folgenden Liste gehoert */
          struct elem *firstpatch;  /* Liste von Patches mit gegebenem Code */
        } BBTREENODE;

typedef struct kante {    /* Kante eines Graphen (wird gerichtet behandelt) */
          struct kante *prev;    /* vorherige Kante im Uhrzeigersinn */
          struct kante *next;    /* naechste Kante im Uhrzeigersinn */
          struct kante *invers;  /* inverse Kante (geht von "name" aus, falls
                                    name!=aussen, andernfalls invers==nil) */
          KNOTENTYP ursprung;    /* (ursprung->name) ist die Kante im Graph, */
          KNOTENTYP name;        /* wenn der Graph numeriert ist */
          KNOTENTYP ursprung2;   /* -> fuer alternative Numerierung */
          KNOTENTYP name2;       /* -> fuer alternative Numerierung */
          KNOTENTYP all;         /* Farbe der Kante (beim Isomorphietest) */
                                 /* Nummer der Flaeche rechts (bei Dual
                                    und bei Patchzusammenhang) */
          EULERTYP nr;           /* fuer die Numerierung der Kanten eines
                                    Petriepfades (bei Graphenkonstruktion),
                                    fuer die Markierung der Randknoten
                                    (bei Patchzusammenhang), wobei sich die
                                    Numerierung auf den Ursprung bezieht */
          FLAECHENTYP fl_links;  /* Groesse der Flaeche links von der Kante */
          FLAECHENTYP fl_rechts; /* dasselbe fuer die rechte Flaeche */
          BOOL pfadanfang;       /* True <=> die Kante soll als Anfang
                                    eines Petriepfades benutzt werden. */    
          BOOL sp_pfadanfang;    /* True <=> die Kante soll als Anfang
                  eines spiegelverkehrten Petriepfades benutzt werden. */   
        } KANTE;

typedef KANTE *PLANMAP[N_MAX][3];  /* fuer Graph, der ausgegeben werden soll
                                   oder fuer Patch, der bewertet werden soll */
typedef KANTE KANTENARRAY[N_MAX][3];     /* fuer die Umrandung eines
                                            mittleren Brillenpatches  */
typedef KANTE KANTENARRAY2[N_MAX*3];
  /* Fuer die Kanten eines Patches bzw. Graphen: ein Patch bzw. Graph kann
     maximal N_MAX*3 (gerichtete) Kanten enthalten. Jede Kante wird im Verlauf
     der Konstruktion nur einmal gezogen, deshalb ist das Array ausreichend
     dimensioniert fuer einen Patch. */ 

typedef struct mem {       /* Zeiger auf grossen Speicherblock */
  struct mem *next;        /* naechster Speicherblock */
  size_t used;             /* Anzahl der benutzten Bytes - immer durch 
                              sizeof(void *) teilbar */
  void *memory;            /* Zeiger auf Block */
} MEMORY;

   
/**********************/    
/* globale Variablen: */
/**********************/

/* Alle globalen Variablen, die mit "(-)" gekennzeichnet sind, werden einmal
   belegt und bleiben dann waehrend des ganzen Programmdurchlaufs konstant. */

/* ------------------ */
/* Erzeugungsgrenzen: */
/* ------------------ */

KNOTENTYP n_max;    /* maximale Knotenzahl eines brauchbaren Patches bei 
                       gegebener maximaler Knotenzahl eines Graphen (-)
                       (wird nur am Anfang benutzt) */
KNOTENTYP n_max_bb; /* dasselbe speziell fuer Bauchbindenpatches (diese
                       Zahl ist <= n_max) (-) */
KNOTENTYP f_max;    /* Flaechenzahl = maximale Flaechenzahl eines Graphen
                       bei gegebener maximaler Knotenzahl eines Graphen (-) */
KNOTENTYP f_anf;    /* Flaechenzahl bei gegebener minimaler Knotenzahl eines
                       Graphen (-) */
KNOTENTYP f_min1;   /* minimale Flaechenzahl eines Bauchbindenpatches bei
                       gegebenen Flaechentypen. Diese Zahl ist gleichzeitig die
                       minimale Flaechenzahl eines Einschlusspatches B. (-) */
KNOTENTYP f_max1;   /* maximale Flaechenzahl eines Bauchbindenpatches bei
                       gegebener maximaler Knotenzahl eines Graphen. Dies ist
                       gleichzeitig die maximale Flaechenzahl eines Patches
                       ueberhaupt. Bei Bruchkanten kann noch eine Flaeche
                abgezogen werden, die mindestens aus dem 3.Patch kommt. (-) */
KNOTENTYP f_max_bb_best=0;   /* diese Variable enthaelt die groesstmoegliche
                                Flaechenzahl eines Zielgraphen, in dem ein
                                Bauchbindenpfad am besten bewertet sein kann 
                                (falls bauchbindenkennung==3, so ist diese 
                                Flaechenzahl immer gerade)   (-) */
                               
/* ---------- */
/* Statistik: */
/* ---------- */

unsigned long graphenzahl[(N_MAX>>1)+1][3], 
              non_iso_graphenzahl[(N_MAX>>1)+1][3]; 
          /* graphenzahl[n][x] fuer Graphen vom Typ x mit 2*n Knoten */ 
unsigned long patches1=0;    /* Anzahl der gespeicherten Bauchbindenpatches */
unsigned long patches23=0;   /* Anzahl der gespeicherten Bruchkantenpatches */
unsigned long nahttyp[6] = {0,0,0,0,0,0};  /* gefundene Patches nach
                Nahttypen (siehe "ELEM.art") der Vorgaenger sortiert */
unsigned int *facestatarray = nil;         /* Array fuer Flaechenstatistik */
int fl1_anz[F_MAXANZ+1];  /* fuer jede Flaechenzahl:  Anzahl der gespeicherten
                             Bauchbindenpatches */
int fl2_anz[F_MAXANZ+1];  /* fuer jede Flaechenzahl:  Anzahl der gespeicherten
                             Bruchkantenpatches */
/* int unmittelbar=0, mittelbar=0; */  /* Anzahl der unmittelbar und mittelbar
                                          benutzten Patches */
/* int gute=0; */         /* Anzahl der brauchbaren Patches */
int gute_basen=0;         /* Anzahl der mittelbar brauchbaren Patches */
unsigned long connzahl[(N_MAX>>1)+1][3];   /* connzahl[n][x-1] fuer x-fach-
                                              zusammenhaengende Graphen */


/* ----------------- */
/* Benutzeroptionen: */
/* ----------------- */

KNOTENTYP n_anf=0,n_end=0;      /* erste und letzte gewuenschte Knotenzahl
                               bzw. Anzahl Pseudogeraden */  
FLAECHENTYP face[MAXFTYPEN];       /* gewuenschte Flaechentypen (-) */
KNOTENTYP facenum_min[MAXFTYPEN];  /* zugehoerige Minimalzahlen (-) */
KNOTENTYP facenum_max[MAXFTYPEN];  /* zugehoerige Maximalzahlen (-) */
FLAECHENTYP anz_face=0;            /* Anzahl der Flaechentypen (-) */
BOOL facestat = False;       /* True => Flaechenstatistik wird erstellt (-) */
BOOL facerestrict = False;   /* True => Minimal- und Maximalzahlen fuer
                                Flaechen eines Typs werden gesetzt */
BOOL facenumbers = False;    /* Flaechenzahlen werden errechnet
                                (fuer "facerestrict" und "facestat") (-) */
BOOL output=True;            /* False => keine Graphen ausgeben (-) */
BOOL do_bauchbinde = True;   /* Sollen Typ-1-Graphen erzeugt werden? (-) */
BOOL do_sandwich = True;     /* Sollen Typ-2-Graphen erzeugt werden? (-) */
BOOL do_brille = True;       /* Sollen Typ 3-Graphen erzeugt werden? (-) */
BOOL alternative = False;    /* False = Aufspaltung nach kritischen Flaechen,
                                True = keine Aufspaltung (-) */
BOOL dual = False;           /* True => Triangulierungen werden ausgegeben */
KNOTENTYP bauchbindenkennung=3, sandwichkennung=1, brillenkennung=2;
 /* Prioritaeten der drei Pfadtypen (je kleiner der Wert, desto groesser die
    Prioritaet) (-) */
unsigned char pv = PV_DEFAULT;   /* je hoeher dieser Wert, desto mehr 
                                 Patchvermeidungskriterien werden angewendet */
BOOL conn1 = False; /* True => 1-zusammenhaengende Graphen werden ausgegeben */
BOOL conn2 = False;
BOOL conn3 = False;
BOOL do_conn = False;     /* True => Zusammenhangsstatistik */
BOOL patchconn = False;   /* True => Zusammenhangszahl der Patches wird
                             beruecksichtigt, d.h. Patches, die zu Graphen mit
                             einer zu kleinen Zusammenhangszahl fuehren,
                             werden aussortiert. Dazu muss der Patch allerdings
                             konstruiert werden. */
BOOL barnette = False;    /* True (nur bei bipartiten Graphen) =>
                             kein Knoten darf an drei Vierecke grenzen
                             (siehe Hamiltonpaper ([5] in Diplomarbeit) */
BOOL PGA = False;    /* True => Programm wird zur Erzeugung von PGAs eingesetzt
                        (voellig neues Verhalten)
                        n = Anzahl der Pseudogeraden */


/* ------ */
/* Files: */
/* ------ */

FILE *outputfile[(N_MAX>>1)+1];  /* fuer jede Graphgroesse ein Outputfile,
                              outputfile[n] fuer Graphen mit 2*n Knoten (-) */
char logfilename[FILENAMENLAENGE];   /* Am Anfang auch fuer die anderen
                                        Filenamen (-) */

/* ------------------- */
/* Speicherverwaltung: */
/* ------------------- */

MEMORY *firstmem=nil;  /* Zeiger auf ersten dynamischen Speicherbereich (-) */
MEMORY *currmem=nil;         /* Zeiger auf aktuellen dynamischen
                                Speicherbereich (der noch nicht voll ist) */ 
struct mallinfo space_info;


/* ---------- */
/* Sonstiges: */
/* ---------- */

TREENODE *tree[F_MAXANZ+1];        /* fuer Baeume mit Bruchkantenpatches */
BBTREENODE *bbtree[F_MAXANZ+1];    /* fuer Baeume mit Bauchbindenpatches */
FLAECHENTYP big_face = 0;        /* Anzahl der Ecken im groessten n-Eck (-) */
                                 /* =0, damit Default fuer PGA gesetzt wird */
FLAECHENTYP small_face;          /* Anzahl der Ecken im kleinsten n-Eck (-) */
BOOL dreiecke = False;             /* True => Dreiecke sind erlaubt (-) */
BOOL vierecke = False;             /* True => Vierecke sind erlaubt (-) */
BOOL fuenfecke = False;            /* True => Fuenfecke sind erlaubt (-) */
BOOL sechsecke = False;            /* True => Sechsecke sind erlaubt (-) */
FLAECHENTYP gr6 = 0;               /* kleinste erlaubte Flaeche >= 6 (-) */
FLAECHENTYP kl6 = 0;               /* groesste erlaubte Flaeche < 6  (-) */
FLAECHENTYP g6 = 0;                /* kleinste erlaubte Flaeche > 6  (-) */
KNOTENTYP max6 = 0;                /* maximal erlaubte Anzahl Sechsecke (-) */
KNOTENTYP krit_min = 0;         /* Mindestanzahl der kritischen Punkte (-) */
KNOTENTYP krit_max = 0;         /* Maximalanzahl der kritischen Punkte (-) */
BOOL is_moeglich[N_MAX+1];     /* is_moeglich[i]==False => Graphen mit den
                          vorgegebenen Flaechen und i Knoten gibt es nicht
                          (PGA: mit i Pseudogeraden) */

KNOTENTYP bind[N_MAX+1][8][V_MAX];    /* Fuer den Patch 1 in der Verklebung
   wird festgehalten, welche Verknuepfungen moeglich sind. Es werden
   die Positionen im Bordercode gespeichert, an denen die Verknuepfung
   vorgenommen werden koennte. Sei bind[i][j][k] das betrachtete Element.
   i ist die Laenge der Verknuepfung (in Kanten),j die Art. Da mehrere
   gleichartige Verknuepfungen auftreten koennen, koennen pro Paar (i,j)
   mehrere Eintraege gespeichert werden, die mit Hilfe von k unterschieden
   werden. Insgesamt sind die Elemente [i][j][0] bis [i][j][anz[i][j]-1]
   belegt. Maximal V_MAX Eintraege pro Verknuepfungsart koennen gespeichert
   werden.

   0: linker Patch - keine Bauchbinde moeglich
   1: linker Patch - es muss Bauchbinde entstehen (sonst nicht kanonisch)
   2: rechter Patch - keine Bauchbinde moeglich
   3: rechter Patch - "halbe" Bauchbinde entsteht (ob komplette Bauchbinde
      entsteht, haengt vom linken Patch ab
   4: Einschluss Patch A, Fall 1a
   5: Einschluss Patch A, Fall 1b
   6: Einschluss Patch A, Fall 2a
   7: Einschluss Patch A, Fall 2b
   8: Einschluss Patch B, Faelle 1a und 2a
   9: Einschluss Patch B, Faelle 1b und 2b
   Bei den Faellen 4 bis 9 ist i nicht die Laenge der Naht, sondern der
   erste Eintrag im Bordercode des Patches B (bei 4 bis 7 also der erforder-
   liche und bei 8 und 9 der tatsaechliche Eintrag).
   Bei den Faellen 4 bis 9 ist es unwesentlich, ob eine Bauchbinde entsteht,
   da das Entstehen nicht vom Zusammenspiel zweier Patches (die dann entspre-
   chend zusammenpassen muessen), sondern nur vom Patch A abhaengt.
   Bei den Faellen j=8 und j=9 ist immer nur eine Position im Bordercode moeg-
   lich, die gespeichert werden koennte, naemlich Position 0. Deshalb wird die
   Position gar nicht gespeichert. Es wird lediglich der Wert anz[x][j] auf 1
   gesetzt, wenn der Patch den Bordercode (x) oder (x,0) hat. Man beachte,
   dass es die Arrayelemente bind[i][j][k] fuer j=8,9 deshalb nicht gibt. */ 
KNOTENTYP anz[N_MAX+1][10];     /* anz[i][j]<=V_MAX (siehe oben) */
 

/***************/
/* Prototypen: */
/***************/

BOOL gehe_patchbaum_durch_in_rekursion(KNOTENTYP *bordercode,KNOTENTYP len,
     KNOTENTYP fl,KNOTENTYP wh,KNOTENTYP f_min,KNOTENTYP krit); 
    /* wird von "Verknuepfung" und "Einschluss" aufgerufen */


/*****************************************/
/* Initialisierungsfunktionen:           */
/* (werden einmal von "main" aufgerufen) */
/*****************************************/

/***********************INITIALISIERE_PATCHBAEUME****************************/

void initialisiere_patchbaeume(void) {
  static KNOTENTYP i;
  for (i=0; i<=F_MAXANZ; i++) {tree[i] = nil;   bbtree[i] = nil; 
                               fl1_anz[i] = fl2_anz[i] = 0;}
}

/***********************INITIALISIERE_IS_MOEGLICH****************************/

void initialisiere_is_moeglich(void) {
  static KNOTENTYP i;
  for (i=0; i<=N_MAX; i++) {is_moeglich[i] = True;}
}

/***********************INITIALISIERE_ANZ_ARRAY******************************/
/*  Auch die nullte Reihe muss initialisiert werden, weil darauf zugegriffen
    werden kann. Das Programm merkt dann auf diese Weise, dass kein 
    Durchschnitt der Laenge 0 moeglich ist. */

void initialisiere_anz_array(void) {
  static KNOTENTYP i,j;
  for (i=0; i<=N_MAX; i++)
    {for (j=0; j<10; j++) {anz[i][j]=0;} }
}

  
/***************************************/
/* programminterne Speicherverwaltung: */
/***************************************/
 
/***********************GIB_SPEICHER_FREI************************************/
/*  Diese Funktion wird von der Funktion "schluss" aufgerufen.              */

void gib_speicher_frei(MEMORY *first) {
  static MEMORY *mem;
  while (first) {
    mem = first->next;
    free(first->memory);
    free(first);
    first = mem;
  }
}
 
/***********************SCHLUSS**********************************************/
/*  Diese Funktion wird bei Beendigung des Programms aufgerufen.            */

void schluss(int errornumber) {
  KNOTENTYP j;
  gib_speicher_frei(firstmem);
  for (j=0; j<=N_MAX; j+=2) {
    if (outputfile[j>>1]!=nil && outputfile[j>>1]!=stdout) 
       {fclose(outputfile[j>>1]);}
  }
  exit(errornumber);
}

/*******************LOGFILE_ENTRY********************************************/
/*  Schreibt einen String in das Logfile.                                   */

void logfile_entry(char *string) {
  static FILE *logfile;
  if (logfile = fopen(logfilename,"a")) {
    if (fprintf(logfile,"%s",string)<0) {
      fprintf(stderr,"Cannot write logfile entry '%s'!\n",string);
      perror((char *)"Error description:");
    }
    if (fclose(logfile)!=0) {
      fprintf(stderr,"Cannot close logfile!\n");
      perror((char *)"Error description:");
    } 
  }
  else {
    fprintf(stderr,"Cannot open logfile!\n");
    perror((char *)"Error description:");
  }
}

/******************SCHREIBFEHLER*********************************************/

void schreibfehler(size_t ist,size_t soll) {
  static FILE *logfile;
  fprintf(stderr,"Error while writing: Tried %d entries, wrote %d entries!\n",
          soll,ist);
  perror((char *)"Error description:");
  logfile_entry((char *)"Error while writing!\n");
  schluss(12);
}

/***********************HOLE_SPEICHER****************************************/
/*  Stellt len Bytes Speicher zur Verfuegung                                */
/*  len wird aufgerundet, so dass die Zahl durch sizeof(void *) teilbar ist */

void *hole_speicher(size_t len) {    
  static void *ptr;
  static FILE *logfile;
  if (len > MEMBLOCKSIZE)  
    {fprintf(stderr,"Required memblock too big (change MEMBLOCKSIZE)!\n");
     logfile = fopen(logfilename,"a");
     fprintf(logfile,"Required memblock too big (change MEMBLOCKSIZE)!\n");
     fclose(logfile);
     schluss(1);}
  if (currmem==nil) {            /* noch kein dynamischer Speicherbereich */
    if ((firstmem = currmem = (MEMORY *)malloc(sizeof(MEMORY)))==nil)
      {fprintf(stderr,"No memory for memorynode!\n"); 
       logfile = fopen(logfilename,"a");
       fprintf(logfile,"No memory for memorynode!\n");
       fclose(logfile);
       schluss(2);}
    currmem->next = nil;
    currmem->used = 0;
    if ((currmem->memory = malloc(MEMBLOCKSIZE))==nil)
      {fprintf(stderr,"No memory for memblock!\n");
       logfile = fopen(logfilename,"a");
       fprintf(logfile,"No memory for memblock!\n");
       fclose(logfile);
       schluss(3);}
  }
  if (MEMBLOCKSIZE - currmem->used < len) {
      /* Speicherbereich zu klein => naechsten Block bereitstellen */  
    if ((currmem->next = (MEMORY *)malloc(sizeof(MEMORY)))==nil) 
      {fprintf(stderr,"No memory for new memorynode!\n");
       logfile = fopen(logfilename,"a");
       fprintf(logfile,"No memory for new memorynode!\n");
       fclose(logfile);
       schluss(4);}
    currmem = currmem->next;
    currmem->next = nil;
    currmem->used = 0;
    if ((currmem->memory = malloc(MEMBLOCKSIZE))==nil)
      {fprintf(stderr,"No memory for new memblock!\n");
       logfile = fopen(logfilename,"a");
       fprintf(logfile,"No memory for new memblock!\n");
       fclose(logfile);
       schluss(5);}
  }
  ptr = (void *)((char *)(currmem->memory) + currmem->used);
  currmem->used += (len-1)/sizeof(void *)*sizeof(void *)+sizeof(void *);
  return(ptr);
}   


/************************************/
/* Low-Level-Umwandlungsfunktionen: */
/************************************/

/*************************MAP_2_PLANARCODE***********************************/
/*   Fuer komplette Graphen, die beliebig numeriert sind.                   */
/*   "code" muss ausreichend Speicherplatz bereithalten.                    */

void map_2_planarcode(PLANMAP m, KNOTENTYP *code, KNOTENTYP n) {
  static int i;
  static KNOTENTYP k,j;
  i = 0;
  code[i++] = n;
  for (k=0; k<n; k++) {
    for (j=0; j<3; j++) {code[i++] = m[k][j]->name;}
    code[i++] = 0;
  }
}

/*************************MAP_2_PLANARCODE2**********************************/
/*   wie "map_2_planarcode", aber fuer die alternative Numerierung          */

void map_2_planarcode2(PLANMAP m, KNOTENTYP *code, KNOTENTYP n) {
  static int i;
  static KNOTENTYP k,j;
  i = 0;
  code[i++] = n;
  for (k=0; k<n; k++) {
    for (j=0; j<3; j++) {code[i++] = m[k][j]->name2;}
    code[i++] = 0;
  }
}


/********************************/
/* Low-Level-Ausgabefunktionen: */
/********************************/

/*************************SCHREIBE_PLANARCODE*********************************/
/* Diese Funktion schreibt den Planarcode mit unsigned chars, wenn moeglich. */

void schreibe_planarcode(KNOTENTYP *planarcode,FILE *f,size_t size) {
  static unsigned char planarcode_c[CODESIZE(N_MAX)];
  static size_t i;
  if (sizeof(KNOTENTYP)==1 || planarcode[0]>252) {
    /* Inhalt von "planarcode" kann direkt kopiert werden */
    if (sizeof(KNOTENTYP)>1) {fprintf(f,"%c",0);}
       /* Kennzeichen fuer grossen Code */
    if ((i = fwrite(planarcode,sizeof(KNOTENTYP),size,f)) < size) 
       {schreibfehler(i,size);}
  }
  else {    /* Inhalt muss Eintrag fuer Eintrag umgewandelt werden */
    for (i=0; i<size; i++)
        {planarcode_c[i] = (unsigned char)planarcode[i];}
    if ((i = fwrite(planarcode_c,sizeof(unsigned char),size,f)) < size)
       {schreibfehler(i,size);}
  }
}

/*************************SCHREIBE_PATCHPLANARCODE****************************/
/* Diese Funktion schreibt den Planarcode mit unsigned chars
   und filtert dabei auch noch Aussenkanten heraus.                          */
/* Die Funktion wird nur zu Testzwecken eingesetzt.                          */

void schreibe_patchplanarcode(KNOTENTYP *planarcode,FILE *f) {
  static unsigned char planarcode_c[CODESIZE(N_MAX)];
  static int i,j;
  /* Inhalt muss Eintrag fuer Eintrag geprueft werden */
  j = 0; 
  for (i=0; i<CODESIZE(planarcode[0]); i++) {
    if (planarcode[i]!=aussen) 
       {planarcode_c[j++] = (unsigned char)planarcode[i];}
  }
  fwrite(planarcode_c,sizeof(unsigned char),(size_t)j,f);
}


/***************************************************************/
/* Low-Level-Funktionen zur Codierung von Graphen und Patches: */
/***************************************************************/

/************************BELEGEDUMMIES************************/
/* belegt k->all, k->invers->prev->all, ... mit Nummer
   -- einmal rund um die Flaeche (rechtsherum) */

void belegedummies(KANTE *k,KNOTENTYP nummer) {
  static KANTE *merke;
  merke = k;  k->all = nummer;
  k = k->invers->prev;
  while (k != merke) {k->all = nummer;  k = k->invers->prev;}
}

/************************DUALCODE*****************************/
/* berechnet das Dual und schreibt es als planarcode in code */

size_t dualcode(KNOTENTYP *code, PLANMAP m, KNOTENTYP knotenzahl) {
  static KNOTENTYP flaechenzahl,i,j,nextnumber;
  static size_t codelaenge;
  static KANTE *startedge[N_MAX/2+2];
  static KANTE *run, *merke;

  code[0] = flaechenzahl = 2 + knotenzahl/2;
  codelaenge = 1;  nextnumber = 1;

  for (i=0; i<knotenzahl; i++) {
    for (j=0; j<3; j++) {m[i][j]->all=0;}
  }
  /* "all" gibt die Nummer der Flaeche rechts der Kante an */

  belegedummies(m[0][0],1);
  startedge[0] = m[0][0];

  for (i=0; i<flaechenzahl; i++) { 
    merke = startedge[i]; 
    if (merke->invers->all) {code[codelaenge++] = merke->invers->all;}
    else {
      code[codelaenge++] = nextnumber+1;
      startedge[nextnumber] = merke->invers;
      belegedummies(merke->invers,nextnumber+1);
      nextnumber++;
    }
    for (run = merke->invers->prev; run != merke; run = run->invers->prev) {
      if (run->invers->all) {code[codelaenge++] = run->invers->all;}
      else {
        code[codelaenge++] = nextnumber+1;
	startedge[nextnumber] = run->invers;
	belegedummies(run->invers,nextnumber+1);
	nextnumber++; 
      }
    }
    code[codelaenge++] = 0;
  }
  return(codelaenge);
}

/**************************GETCONN*******************************************/
/*  Liefert die Zusammenhangszahl z von m. Diese wird mit Hilfe des Dualen
    ermittelt:
    z = 1  <=>  Flaeche mit sich selbst verklebt  <=>  Schleife im Dualen
    z = 2  <=>  Zwei Flaechen an zwei Kanten verklebt  <=>  Doppelkante
                                                            im Dualen       */

unsigned char getconn(PLANMAP m, KNOTENTYP n) {
  static KNOTENTYP dcode[CODESIZE(N_MAX)];   /* ausreichend gross */
  static BOOL adj[N_MAX+1];
  static size_t codesize;
  static int i,j,k;
  static unsigned char conn;
  codesize = dualcode(dcode,m,n);
  
  conn = 3;     /* solange nichts anderes festgestellt wird */
  k = 1;
  for (i=1; i<=(int)dcode[0] && conn>1; i++) {
    for (j=1; j<=(int)dcode[0]; j++) {adj[j]=False;}
    while (dcode[k]) {
      if (conn==3 && adj[dcode[k]]) {conn = 2;}
      if (dcode[k]==i)              {conn = 1;}
      adj[dcode[k]] = True;    /* Adjazenz registrieren */ 
      k++;
    }
    k++;
  }
  return(conn);
}   
 
/*************************NUMERIERE_GRAPH************************************/
/*  Findet Numerierung fuer Graphen, wobei der markierte Knoten die
    1 erhaelt. Es wird vorausgesetzt, dass im Graphen alle Knoten mit 0 
    numeriert sind (also alle "ursprung"-Eintraege und alle "name"-Eintraege).
    Das Ergebnis der Numerierung steht in m. Ueber m sind dann auch alle
    Kanten ansprechbar, denn jede (gerichtete) Kante besitzt genau einen
    Eintrag in m. Deshalb wird das Numerieren von Graphen oftmals nur benutzt,
    um das Array m zu erhalten.  
    Markiert wird der Knoten k->ursprung. Der gesamte Graph wird von k aus
    erreicht, so dass k als Zeiger auf den gesamten Graphen dient. */
/*  Die Funktion kann auch dann benutzt werden, wenn noch Kanten nach aussen
    existieren, wenn also "k" noch gar keinen Graphen, sondern nur einen
    Patch kennzeichnet. Das geht schneller als bei "numeriere_patch" (die
    Funktion gibt es nicht mehr), denn
    die Eintraege "ursprung" und "name" werden bei der Konstruktion eines
    Patches bereits initialisiert, die Eintraege "pursprung" und "pname"
    jedoch nicht. Falls "k" nur auf einen Patch zeigt, so muss k->prev nach
    aussen zeigen. */

void numeriere_graph(PLANMAP m,KANTE *k) {
  static KNOTENTYP i,j;

  /*  m erhaelt korrekte Numerierung: */ 
  k->ursprung = k->next->ursprung = k->prev->ursprung = 1;
  m[0][0] = k;  m[0][1] = k->next;  m[0][2] = k->prev;  
  m[1][0] = k->invers;
  k->name = 2; 
  k->invers->ursprung = k->invers->next->ursprung = 
                        k->invers->prev->ursprung = 2;
  m[2][0] = k->next->invers;
  k->next->name = 3;  
  k->next->invers->ursprung = k->next->invers->next->ursprung =
                              k->next->invers->prev->ursprung = 3;
  m[3][0] = k->prev->invers;
  if (k->prev->name==aussen) {j=3;}
  else {
    j = k->prev->name = 4;      /* j = hoechste bisher vergebene Nummer */
    k->prev->invers->ursprung = k->prev->invers->next->ursprung =
                                k->prev->invers->prev->ursprung = 4;
  }

  i = 1;
  while (i<j) {              /* es gibt noch unbelegte Knoten */
    k = m[i][0];             /* diese Kante ist bereits verknuepft worden */
    k->name = k->invers->ursprung;   /* darum existiert dieser Wert */  
    if (k->next->name!=aussen) {
      if (k->next->invers->ursprung==0) {
        k->next->name = ++j;   m[j-1][0] = k->next->invers;
        m[j-1][0]->ursprung = m[j-1][0]->prev->ursprung = 
        m[j-1][0]->next->ursprung = j;
      }
      else {k->next->name = k->next->invers->ursprung;}
    }
    if (k->prev->name!=aussen) {
      if (k->prev->invers->ursprung==0) {
        k->prev->name = ++j;   m[j-1][0] = k->prev->invers;
        m[j-1][0]->ursprung = m[j-1][0]->prev->ursprung = 
        m[j-1][0]->next->ursprung = j;
      }
      else {k->prev->name = k->prev->invers->ursprung;}
    }
    m[i][1] = k->next;
    m[i][2] = k->prev;
    i++;
  }
}

/*************************NUMERIERE_GRAPH2***********************************/
/*  Funktioniert wie "numeriere_graph", aber benutzt den alternativen
    Speicherplatz in der Struktur "kante". Die Funktion wird nur fuer
    komplette 3-regulaere Karten benutzt, deshalb entfaellt die Frage nach
    Aussenkanten.                                                           */

void numeriere_graph2(PLANMAP m,KANTE *k) {
  static KNOTENTYP i,j;

  /*  m erhaelt korrekte Numerierung: */ 
  k->ursprung2 = k->next->ursprung2 = k->prev->ursprung2 = 1;
  m[0][0] = k;  m[0][1] = k->next;  m[0][2] = k->prev;  
  m[1][0] = k->invers;
  k->name2 = 2; 
  k->invers->ursprung2 = k->invers->next->ursprung2 = 
                         k->invers->prev->ursprung2 = 2;
  m[2][0] = k->next->invers;
  k->next->name2 = 3;  
  k->next->invers->ursprung2 = k->next->invers->next->ursprung2 =
                               k->next->invers->prev->ursprung2 = 3;
  m[3][0] = k->prev->invers;
  j = k->prev->name2 = 4;      /* j = hoechste bisher vergebene Nummer */
  k->prev->invers->ursprung2 = k->prev->invers->next->ursprung2 =
                               k->prev->invers->prev->ursprung2 = 4;

  i = 1;
  while (i<j) {              /* es gibt noch unbelegte Knoten */
    k = m[i][0];             /* diese Kante ist bereits verknuepft worden */
    k->name2 = k->invers->ursprung2;   /* darum existiert dieser Wert */  
    if (k->next->invers->ursprung2==0) {
      k->next->name2 = ++j;   m[j-1][0] = k->next->invers;
      m[j-1][0]->ursprung2 = m[j-1][0]->prev->ursprung2 = 
      m[j-1][0]->next->ursprung2 = j;
    }
    else {k->next->name2 = k->next->invers->ursprung2;}
    if (k->prev->invers->ursprung2==0) {
      k->prev->name2 = ++j;   m[j-1][0] = k->prev->invers;
      m[j-1][0]->ursprung2 = m[j-1][0]->prev->ursprung2 = 
      m[j-1][0]->next->ursprung2 = j;
    }
    else {k->prev->name2 = k->prev->invers->ursprung2;}
    m[i][1] = k->next;
    m[i][2] = k->prev;
    i++;
  }
}

/*************************NUMERIERE_GRAPH_SP********************************/
/*  Funktioniert wie "numeriere_graph", aber numeriert spiegelverkehrt.
    Die Funktion wird nur fuer komplette 3-regulaere Karten benutzt, 
    deshalb entfaellt die Frage nach Aussenkanten.                         */

void numeriere_graph_sp(PLANMAP m,KANTE *k) {
  static KNOTENTYP i,j;

  /*  m erhaelt korrekte Numerierung: */ 
  k->ursprung = k->next->ursprung = k->prev->ursprung = 1;
  m[0][0] = k;  m[0][1] = k->prev;  m[0][2] = k->next;  
  m[1][0] = k->invers;
  k->name = 2; 
  k->invers->ursprung = k->invers->next->ursprung = 
                        k->invers->prev->ursprung = 2;
  m[2][0] = k->prev->invers;
  k->prev->name = 3;  
  k->prev->invers->ursprung = k->prev->invers->next->ursprung =
                              k->prev->invers->prev->ursprung = 3;
  m[3][0] = k->next->invers;
  j = k->next->name = 4;      /* j = hoechste bisher vergebene Nummer */
  k->next->invers->ursprung = k->next->invers->next->ursprung =
                              k->next->invers->prev->ursprung = 4;

  i = 1;
  while (i<j) {              /* es gibt noch unbelegte Knoten */
    k = m[i][0];             /* diese Kante ist bereits verknuepft worden */
    k->name = k->invers->ursprung;   /* darum existiert dieser Wert */  
    if (k->prev->invers->ursprung==0) {
      k->prev->name = ++j;   m[j-1][0] = k->prev->invers;
      m[j-1][0]->ursprung = m[j-1][0]->prev->ursprung = 
      m[j-1][0]->next->ursprung = j;
    }
    else {k->prev->name = k->prev->invers->ursprung;}
    if (k->next->invers->ursprung==0) {
      k->next->name = ++j;   m[j-1][0] = k->next->invers;
      m[j-1][0]->ursprung = m[j-1][0]->prev->ursprung = 
      m[j-1][0]->next->ursprung = j;
    }
    else {k->next->name = k->next->invers->ursprung;}
    m[i][1] = k->prev;
    m[i][2] = k->next;
    i++;
  }
}

/*************************NUMERIERE_GRAPH2_SP********************************/
/*  Funktioniert wie "numeriere_graph2", aber numeriert spiegelverkehrt.
    Die Funktion wird nur fuer komplette 3-regulaere Karten benutzt, 
    deshalb entfaellt die Frage nach Aussenkanten.                          */

void numeriere_graph2_sp(PLANMAP m,KANTE *k) {
  static KNOTENTYP i,j;

  /*  m erhaelt korrekte Numerierung: */ 
  k->ursprung2 = k->next->ursprung2 = k->prev->ursprung2 = 1;
  m[0][0] = k;  m[0][1] = k->prev;  m[0][2] = k->next;  
  m[1][0] = k->invers;
  k->name2 = 2; 
  k->invers->ursprung2 = k->invers->next->ursprung2 = 
                         k->invers->prev->ursprung2 = 2;
  m[2][0] = k->prev->invers;
  k->prev->name2 = 3;  
  k->prev->invers->ursprung2 = k->prev->invers->next->ursprung2 =
                               k->prev->invers->prev->ursprung2 = 3;
  m[3][0] = k->next->invers;
  j = k->next->name2 = 4;      /* j = hoechste bisher vergebene Nummer */
  k->next->invers->ursprung2 = k->next->invers->next->ursprung2 =
                               k->next->invers->prev->ursprung2 = 4;

  i = 1;
  while (i<j) {              /* es gibt noch unbelegte Knoten */
    k = m[i][0];             /* diese Kante ist bereits verknuepft worden */
    k->name2 = k->invers->ursprung2;   /* darum existiert dieser Wert */  
    if (k->prev->invers->ursprung2==0) {
      k->prev->name2 = ++j;   m[j-1][0] = k->prev->invers;
      m[j-1][0]->ursprung2 = m[j-1][0]->prev->ursprung2 = 
      m[j-1][0]->next->ursprung2 = j;
    }
    else {k->prev->name2 = k->prev->invers->ursprung2;}
    if (k->next->invers->ursprung2==0) {
      k->next->name2 = ++j;   m[j-1][0] = k->next->invers;
      m[j-1][0]->ursprung2 = m[j-1][0]->prev->ursprung2 = 
      m[j-1][0]->next->ursprung2 = j;
    }
    else {k->next->name2 = k->next->invers->ursprung2;}
    m[i][1] = k->prev;
    m[i][2] = k->next;
    i++;
  }
}

/*******************LOESCHE_NUMERIERUNG**************************************/
/*  Diese Funktion loescht die Originalnumerierung im fertigen Graphen m. 
    m muss ein fertiger Graph sein, Aussenkanten sind nicht erlaubt.        */

void loesche_numerierung(PLANMAP m,KNOTENTYP n) {
  static KNOTENTYP i,j;
  for (i=0; i<n; i++) {
    for (j=0; j<3; j++) {m[i][j]->ursprung = m[i][j]->name = 0;}
  }
}
 
/*******************LOESCHE_NUMERIERUNG2*************************************/
/*  Diese Funktion loescht die alternativen Numerierungen im fertigen 
    Graphen m. m muss ein fertiger Graph sein, Aussenkanten sind nicht
    erlaubt.                                                                */

void loesche_numerierung2(PLANMAP m,KNOTENTYP n) {
  static KNOTENTYP i,j;
  for (i=0; i<n; i++) {
    for (j=0; j<3; j++) {m[i][j]->ursprung2 = m[i][j]->name2 = 0;}
  }
}
 
/***************************VERGLEICHE_CODES*********************************/
/*  kein memcmp, um endian-Probleme zu vermeiden                            */
/*  (obwohl auch mit dem falschen endian keine Probleme auftreten duerften,
     da auch dann eine Halbordnung existiert)                               */

signed char vergleiche_codes(KNOTENTYP *code1,KNOTENTYP *code2,size_t len) {
  static KNOTENTYP i;
  for (i=0; i<len; i++) {
    if (*code1 > *code2) {return(1);}
    else if (*code1 < *code2) {return(-1);}
    code1++;  code2++;
  }
  return(0);
}


/*************************************************************/
/* Low-Level-Funktionen zur Ueberwachung der Flaechenzahlen: */
/*************************************************************/

/*****************FLAECHENZAHLEN2_OK***************************************/
/*  Prueft, ob die vom Benutzer geforderten Flaechenzahlen eingehalten
    werden. "e1" und "e2" sind die beiden Patches, die miteinander verklebt
    werden sollen. Falls min==False, so werden nur die Maximalzahlen
    geprueft, andernfalls auch die Mindestzahlen. */

BOOL flaechenzahlen2_ok(ELEM *e1,ELEM *e2,BOOL min) {
  static FLAECHENTYP f;
  static KNOTENTYP f_anz;
  for (f=0; f<anz_face; f++) {
    if ((f_anz = (e1->flaechenzahl)[f]+(e2->flaechenzahl)[f]) > facenum_max[f]
        || (min && f_anz<facenum_min[f]))
       {return(False);}
  }
  return(True);
}

/*****************FLAECHENZAHLEN3_OK***************************************/
/*  Prueft, ob die vom Benutzer geforderten Flaechenzahlen eingehalten
    werden. "e1", "e2" und "e3" sind die drei Patches, 
    die miteinander verklebt werden sollen.  */

BOOL flaechenzahlen3_ok(ELEM *e1,ELEM *e2,ELEM *e3) {
  static FLAECHENTYP f;
  static KNOTENTYP f_anz;
  for (f=0; f<anz_face; f++) {
    if ((f_anz = (e1->flaechenzahl)[f]+(e2->flaechenzahl)[f]+
        (e3->flaechenzahl)[f]) > facenum_max[f]  ||  f_anz<facenum_min[f])
      {return(False);}
  }
  return(True);
}

/**************FLAECHENVORAUSSCHAU_OK**************************************/
/*  Diese Funktion prueft fuer zwei Patches "e1" und "e2", die miteinander
    verklebt werden sollen, ob aus dem Ergebnispatch ueberhaupt noch etwas
    werden kann, wenn man die Mindestzahlen pro Flaechentyp, die im Graphen
    herauskommen sollen, mitberuecksichtigt. Beide Patches zusammen besitzen
    "fl" Flaechen und eine Flaechendifferenz "fd".                        */

BOOL flaechenvorausschau_ok(ELEM *e1,ELEM *e2,KNOTENTYP fl,KNOTENTYP fd) {
  static signed long fd_neu;    /* was mindestens hinzukommt */
  static KNOTENTYP fl_neu;      /* was mindestens hinzukommt */
  static FLAECHENTYP f;
  static EULERTYP f_anz;
  fl_neu = 0;  fd_neu = 0L;
  for (f=0; f<anz_face; f++) {
    if ((f_anz = facenum_min[f]-(EULERTYP)((e1->flaechenzahl)[f])
                               -(EULERTYP)((e2->flaechenzahl)[f]))>0)
      {fl_neu += f_anz;
       fd_neu += ((signed long)face[f]-6)*((signed long)f_anz);}
  }  
  return(fl+fl_neu <= f_max  &&
        (((signed long)fd+fd_neu) <= (signed long)
          (f_max-fl-fl_neu)*(signed long)(6-small_face)));
}
         

/*******************************************************/
/* Low-Level-Funktionen zur Ueberwachung der Struktur: */
/*******************************************************/

/****************DREI_VIERECKE************************************************/
/*  Diese Funktion uebergibt True, wenn im Patch bzw. Graphen, dessen 
    Numerierung in "m" enthalten ist und der "n" Knoten besitzt, ein (innerer)
    Knoten an drei Vierecke grenzt.                                          */
/*  Die Funktion funktioniert fuer Graphen nur, wenn alle nr-Eintraege der
    Kanten auf 0 gesetzt sind (wird interpretiert als: kein Randknoten).     */

BOOL drei_vierecke(PLANMAP m,KNOTENTYP n) {
  static KNOTENTYP i;
  for (i=0; i<n; i++) {     /* Knoten durchlaufen */
    if (m[i][0]->nr==0) {   /* kein Randknoten => Analyse sinnvoll */
      if (m[i][0]->fl_links==4 && m[i][1]->fl_links==4 && m[i][2]->fl_links==4)
  	 {return(True);}
    }
  }
  return(False);
}


/***************************************/
/* Funktionen fuer den Isomorphietest: */
/***************************************/

/*********************VERGLEICHE_ANGRENZENDE_FLAECHEN************************/
/*  Diese Funktion vergleicht die an zwei Petriepfaden angrenzenden Flaechen
    und gibt daraufhin eine Bewertung der Pfade ab.
    "pfad1" und "pfad2" sind die Anfangskanten der beiden Pfade. "sp1" und 
    "sp2" geben an, ob Pfad 1 bzw. Pfad 2 spiegelverkehrt zu durchlaufen ist.
    Die Pfade muessen gleichlang sein, naemlich beide "len" Kanten lang sein.
    Die Funktion gibt den Wert 1 zurueck, wenn Pfad 1 besser ist, im umge-
    kehrten Fall den Wert 2. Wenn beide Pfade gleich gut bewertet werden,
    gibt die Funktion den Wert 0 zurueck. */
/*  Konvention: erst werden die linken Flaechen, dann die rechten Flaechen
    miteinander verglichen (falls nicht spiegelverkehrt) */

char vergleiche_angrenzende_flaechen(KANTE *pfad1,KANTE *pfad2,BOOL sp1,
     BOOL sp2,KNOTENTYP len) {
  static KNOTENTYP i;     /* zurueckgelegte Wegstrecke */
  static FLAECHENTYP fl_l1,fl_l2,fl_r1,fl_r2;   /* die vier Flaechen, die
                                      jeweils am Vergleich beteiligt sind */
  i = 0;
  while (i<len) {
    if (sp1) {fl_l1 = pfad1->fl_rechts;   fl_r1 = pfad1->fl_links;}
    else     {fl_l1 = pfad1->fl_links;    fl_r1 = pfad1->fl_rechts;}
    if (sp2) {fl_l2 = pfad2->fl_rechts;   fl_r2 = pfad2->fl_links;}
    else     {fl_l2 = pfad2->fl_links;    fl_r2 = pfad2->fl_rechts;}
    if (fl_l1 != fl_l2)  {return(fl_l1<fl_l2 ? 1 : 2);}
    if (fl_r1 != fl_r2)  {return(fl_r1<fl_r2 ? 1 : 2);}    
    pfad1 = (i&1)!=sp1 ? pfad1->invers->next : pfad1->invers->prev;
    pfad2 = (i&1)!=sp2 ? pfad2->invers->next : pfad2->invers->prev;
    i++;
  }
  return(0);      /* keinen Unterschied festgestellt */
}

/***********************MARKIERE_PETRIEPFAD**********************************/
/*  Diese Funktion verfolgt von der Kante "k" ausgehend den Petriepfad im
    Graphen und numeriert alle auftretenden Kanten mit "farbe" (dies erfolgt im
    "all"-Element jeder Kante). Falls "links" den Wert True besitzt, geht
    die Funktion zuerst nach links ("spiegelverkehrt"), andernfalls nach rechts
    ("normal").
    An "pfadcode" gibt die Funktion die Bewertung des gefundenen Pfades
    zurueck und an "codelen" wird die Laenge dieses Bewertungscodes
    uebergeben.
    Die Funktion markiert alle Kanten, deren Fortsetzung zum selben Petriepfad
    fuehren wuerde wie der gerade markierte, als unbrauchbar. */
/*  An "first" uebergibt die Funktion einen Zeiger auf die erste Kante des
    Petriepfades, an "last" einen Zeiger auf das Inverse der letzten Kante
    (Startkante fuer die andere Richtung). */
/*  Die Richtung, die bei Kante i eingeschlagen wird (links oder rechts),
    ist die gleiche, die bei Kante -i in der entgegengesetzten Laufrichtung
    eingeschlagen wird. Das erspart Fallunterscheidungen, wenn die Lauf-
    richtung des Pfades umgedreht wird, um Kanonizitaet zu erreichen. */
/*  Der Rueckgabewert der Funktion ist "True", wenn die Richtung des Pfades
    allein aufgrund des Pfades selbst noch nicht eindeutig ist. */
/*  Falls der Pfad ein Sandwichpfad ist, so werden an "anfang" und "ende" die
    Werte von "anf" und "end" uebergeben. */
/*  Wichtig: Die nr-Eintraege in den Kanten des Graphen brauchen zu Beginn der
    Funktion NICHT gleich 0 zu sein (also kann z.B. die Funktion 
    "drei_vierecke" zuvor ohne Bedenken angewendet werden). */

BOOL markiere_petriepfad(KANTE *k,BOOL links,KNOTENTYP farbe,KNOTENTYP 
     *pfadcode,KNOTENTYP *codelen,KANTE **first,KANTE **last,KNOTENTYP
     *anfang,KNOTENTYP *ende) {
  static KANTE *k2;           /* Laufvariable */
  static KNOTENTYP h,i;       /* Hilfsvariable */
  static KNOTENTYP lv,lr; /* zaehlt die Schritte mit (vorwaerts/rueckwaerts) */
  static KNOTENTYP l1,l2,l3;  /* Laengen der einzelnen Pfadteile, falls es sich
                                 beim Petriepfad nicht um Bauchbinde handelt */
  static EULERTYP nr;         /* fuer die Numerierung der einzelnen Kanten von
                                 der Ausgangskante aus gesehen */
  static BOOL sp;             /* True => gesamter Pfad ist spiegelverkehrt */
  static KNOTENTYP anf,end;   /* Anzahl der Kanten, die vom Anfang bzw. vom
                                 Ende des Pfades vorwaerts bzw. rueckwarts
                       durchlaufen als Startkanten zum selben Pfad fuehren. */
  static BOOL erg;            /* Rueckgabewert */
                                
  /* zunaechst vorwaerts laufen: */
  k2 = k;   nr = 0;
  while (k2->all != farbe) {    /* Kreis ist noch nicht geschlossen */
    k2->all = k2->invers->all = farbe;
    k2->nr  = k2->invers->nr  = nr++;
    k2 = (nr&1)==links ? k2->invers->next : k2->invers->prev;
  }
  lv = (KNOTENTYP)nr;
  *last = (nr&1)==links ? k2->prev->invers : k2->next->invers;  
  *last = (*last)->invers;     /* Schritt zurueck und umdrehen */
  
  /* nun rueckwaerts laufen: */
  k2 = links ? k->next->invers : k->prev->invers;   nr = -1;
  while (k2->all != farbe) {
    k2->all = k2->invers->all = farbe;
    k2->nr  = k2->invers->nr  = nr--;
    k2 = (nr&1)==links ? k2->next->invers : k2->prev->invers;
  }
  lr = (KNOTENTYP)(-nr)-1;
  *first = (nr&1)==links ? k2->invers->prev : k2->invers->next;
  /* Schritt zurueck */
  

  /* Pfad analysieren: */
  if (lr==0 && (k->prev->all!=farbe || k->next->all!=farbe)) { 
    /* Bauchbinde gefunden */
    pfadcode[0] = bauchbindenkennung;      /* pfadcode fuer Bauchbinde */
    if (lv&1) {fprintf(stderr,"Logischer Fehler in markiere_petriepfad\n");
               schluss(8);}
    pfadcode[1] = lv;           /* Laenge der Bauchbinde */
    *codelen = 2;
    *last = (*first)->invers;
    anf = end = lv;
    erg = True;
  }
  else if ((*first)->prev->nr + (*first)->next->nr < (*last)->prev->nr 
           + (*last)->next->nr) {    /* Brillenpfad */
    /* Addition der beiden "nr"-Werte, um Gleichheit auszuschliessen 
       (falls eine Kante in beiden Summen vorkommt) */
    pfadcode[0] = brillenkennung;       /* Kennzeichnung fuer Brillenpfad */
    l1 = (KNOTENTYP)(MAX((*first)->prev->nr,(*first)->next->nr) - 
         (*first)->nr);    /* erstes Brillenglas */
    l3 = (KNOTENTYP)((*last)->nr - MIN((*last)->prev->nr,(*last)->next->nr));
         /* zweites Brillenglas */
    l2 = (KNOTENTYP)(MIN((*last)->prev->nr,(*last)->next->nr) 
         - MIN((*first)->prev->nr,(*first)->next->nr));    /* Mittelteil */
    if (l1>l3) {k2 = *last;  *last = *first;  *first = k2;  
                 h = l1;  l1 = l3;  l3 = h;}    
                /* der Pfad beginnt am kleineren Glas */
    pfadcode[1] = l2;           /* Mittelteil */
    pfadcode[2] = l1;          /* kleineres Glas */
    pfadcode[3] = l3;          /* groesseres Glas */
    *codelen = 4;
    anf = l1+l2+(l3&1);    end = l2+l3+(l1&1);
    erg = (l1==l3);  
  }
  else {                                              /* Sandwichpfad */
    pfadcode[0] = sandwichkennung;       /* Kennzeichnung fuer Sandwichpfad */
    l1 = (KNOTENTYP)(MAX((*last)->prev->nr,(*last)->next->nr) - (*first)->nr);
         /* erster Aussenteil (in Dokumentationsentwurf Laenge "3") */
    l2 = (KNOTENTYP)(MAX((*first)->prev->nr,(*first)->next->nr) - 
                  MAX((*last)->prev->nr,(*last)->next->nr)); /* Mittelteil */
    l3 = (KNOTENTYP)((*last)->nr - MIN((*first)->prev->nr,(*first)->next->nr));
         /* zweiter Aussenteil (in Dokumentationsentwurf Laenge "1") */
    if (l1>l3) {k2 = *last;  *last = *first;  *first = k2;
                h = l1;  l1 = l3;  l3 = h;}
               /* der Pfad beginnt am kuerzeren Aussenteil */
    pfadcode[1] = l2;                  /* Mittelteil */
    pfadcode[2] = l1;          /* kleinerer Aussenteil */
    pfadcode[3] = l3;          /* groesserer Aussenteil */
    *codelen = 4;
    *anfang = anf = l1+((l3&1)==0);   *ende = end = l3+((l1&1)==0);
    erg = (l1==l3);  
  }
 
  /* Nun werden "anf" bzw. "end" Kanten als unbrauchbar markiert: */
  sp = (links != ((*first)->nr & 1));    /* Ausrichtung des Pfades */
  k2 = *first;
  for (i=0; i<anf; i++) {
    if ((i&1)==sp) {k2->pfadanfang = False;     k2 = k2->invers->prev;}
    else           {k2->sp_pfadanfang = False;  k2 = k2->invers->next;}
  }
  sp = (links != ((*last)->nr & 1));    /* Ausrichtung des Pfades */
  k2 = *last;
  for (i=0; i<end; i++) {
    if ((i&1)==sp) {k2->pfadanfang = False;     k2 = k2->invers->prev;}
    else           {k2->sp_pfadanfang = False;  k2 = k2->invers->next;}
  }

  return(erg);
}

/**************NUMERIERE_GRAPH_VOM_PFAD**************************************/
/*  Diese Funktion numeriert einen Graphen von der Kante "first" ausgehend.
    Dies ist die Anfangskante eines Pfades. Die Numerierung wird im alter-
    nativen Speicherplatz von "m" festgehalten. Der zugehoerige Planarcode
    wird an "graph" zurueckgegeben (dort muss ausreichend Speicherplatz
    vorhanden sein. Falls "sp==True", so wird spiegelverkehrt numeriert.    */
 
void numeriere_graph_vom_pfad(KANTE *first,BOOL sp,KNOTENTYP *graph,
     PLANMAP m,KNOTENTYP n) {
  loesche_numerierung2(m,n);
  if (sp) {numeriere_graph2_sp(m,first);} else {numeriere_graph2(m,first);}
  map_2_planarcode2(m,graph,n);
}  
  
/*******************VERGLEICHE_PETRIEPFADE***********************************/
/*  Vergleicht zwei Pfade mit feststehenden Anfangskanten und gleichen
    Laengen. Auch die Pfadteile sind gleich lang. Wenn "beide_richtungen==
    True", so muss, wenn der Vergleich vom Anfang aus gesehen noch kein 
    eindeutiges Ergebnis bringt, auch noch der Vergleich mit dem
    Ende von Pfad 2 vorgenommen werden.                                     */
/*  "first1" ist die Anfangskante von Pfad 1, "first2" von Pfad 2. "last2"
    ist die Endkante von Pfad 2.                                            */
/*  "g1" ist der planar-Code des Graphen von "pfad1" aus gesehen.           */
/*  Der Graph ist in "m" gespeichert (die Referenznumerierung aus "g1" ist
    im Originalspeicherplatz gespeichert) und besitzt "n" Knoten.           */
/*  "is_g1" gibt an, ob "g1" sinnvolle Werte enthaelt oder erst noch
    berechnet werden muss. "len" ist die Laenge der Pfade.                  */
/*  "sp1" gibt an, ob Pfad 1 spiegelverkehrt verlaeuft, "sp2f" gibt an, ob
    Pfad 2 vom Anfang an durchlaufen spiegelverkehrt verlaeuft, "sp2l" gibt
    an, ob Pfad 2 vom Ende an durchlaufen spiegelverkehrt verlaeuft.        */
/*  "sp2l" und "last2" sind nur von Bedeutung, wenn beide_richtungen==True. */

BOOL vergleiche_petriepfade(KANTE *first1,KANTE *first2,KANTE *last2,BOOL sp1,
     BOOL sp2f,BOOL sp2l,KNOTENTYP *g1,PLANMAP m,KNOTENTYP n,
     BOOL beide_richtungen,KNOTENTYP len,BOOL *is_g1) {
  static KNOTENTYP g2[CODESIZE(N_MAX)];
  static signed char erg;                   /* Vergleichsergebnis */  

  if ((erg = vergleiche_angrenzende_flaechen(first1,first2,sp1,sp2f,len))==2)
    {return(False);}
  else if (erg==1) {return(True);}
  /* Falls beide_richtungen==True => Vergleich von "first1" und "last2"
     wuerde dasselbe Ergebnis bringen, da die Flaechensequenzen von "first2"
     und "last2" aus betrachtet dann gleichwertig sind. */

  /* ab hier gilt: erg==0 => Pfade mitsamt Sequenzen gleichwertig */
  if (!(*is_g1)) {map_2_planarcode(m,g1,n); *is_g1 = True;} 
  /* Der Wert von "beide_richtungen" passt auch bei pfad1, da pfad1 und 
         pfad2 die gleiche Flaechensequenz haben. */
  numeriere_graph_vom_pfad(first2,sp2f,g2,m,n);
  erg = vergleiche_codes(g1,g2,CODESIZE(n));
  if (erg > 0) {return(False);}
  if (beide_richtungen) {           /* andersherum auch codieren */
    numeriere_graph_vom_pfad(last2,sp2l,g2,m,n);
    erg = vergleiche_codes(g1,g2,CODESIZE(n));
    if (erg > 0) {return(False);}
  }
  return(True);                     /* nichts Besseres gefunden */   
}
  
/*******************VERGLEICHE_BAUCHBINDENMENGE******************************/
/*  Diese Funktion vergleicht die Bauchbinde "pfad2" und alle sich hieraus
    ergebenden Pfade mit der Referenzbauchbinde "pfad1". Wenn es eine bessere
    findet, uebergibt sie "False", sonst "True". Der Pfad "pfad2" ist normal
    zu durchlaufen. "pfad2" kann gleich "pfad1->invers->next" sein. 
    "m", "n", und "g1" siehe "vergleiche_bauchbinden". 
    "pfad1" ist spiegelverkehrt. Die Laengen der Referenzbauchbinde und der 
    Vergleichsbauchbinden muessen gleich sein.                              */

BOOL vergleiche_bauchbindenmenge(KANTE *pfad1,KANTE *pfad2,KNOTENTYP *g1,
     PLANMAP m,KNOTENTYP n,KNOTENTYP len,BOOL *is_g1) {
  static KANTE *p;
  static char erg;

  /* zunaechst pfad2 normal durchlaufen: */
  p = pfad2;
  do { 
    /* eindeutige Richtung im Pfad anhand der Flaechensequenzen festlegen: */
    erg = vergleiche_angrenzende_flaechen(p,p->invers,False,False,len);
    /* if (erg==2) {p = p->invers;} */ 
    /* Zeiger nicht umdrehen, sonst wird der Schleifendurchlauf unkorrekt */ 
    if (vergleiche_petriepfade(pfad1,erg==2 ? p->invers : p,erg==2 ? p : 
        p->invers,True,False,False,g1,m,n,erg==0,len,is_g1)==False)
          {return(False);}
    p = p->invers->prev->invers->next;
  } while (p!=pfad2);                /* einmal rundherum */  

  /* nun die spiegelverkehrten Pfade durchlaufen: */
  p = pfad2->invers->prev;
  do {
    if (p!=pfad1) {
      erg = vergleiche_angrenzende_flaechen(p,p->invers,True,True,len);
      if (vergleiche_petriepfade(pfad1,erg==2 ? p->invers : p,erg==2 ? p :
          p->invers,True,True,True,g1,m,n,erg==0,len,is_g1)==False)
            {return(False);}
    }
    p = p->invers->next->invers->prev;
  } while (p!=pfad2->invers->prev);   /* einmal rundherum */

  return(True);
}

/***************************BESTER_PETRIEPFAD********************************/
/*  Diese Funktion bildet aus allen Kanten des Graphen Petriepfade. 
    Diese Pfade werden mit dem Referenz-
    pfad, dessen Bewertung durch "pfadcode", "codelen" und dem Planarcode
    "g1" gegeben ist, verglichen. Falls sich ein besserer Pfad findet, wird 
    "False" zurueckgegeben, andernfalls "True". Die "all"-Werte in allen 
    Kanten des Graphen muessen <= 1 sein. "first" zeigt auf die erste Kante 
    des Originalpetriepfades, "sp" gibt an, ob der Pfad spiegelverkehrt 
    verlaeuft (muss bei Bauchbinde immer "True" sein). "pfl" gibt die 
    Petrieflaeche an. Wenn "is_g1==True", so enthaelt "g1" bereits den
    korrekten Code, andernfalls muss er bei Bedarf erst noch uebergeben
    werden, und zwar aus "m". */

BOOL bester_petriepfad(KNOTENTYP *pfadcode,KNOTENTYP codelen,KANTE *first,
     BOOL sp,PLANMAP m,KNOTENTYP n,KNOTENTYP *g1,BOOL *is_g1) {
  static KNOTENTYP farbe;        /* fuer die Markierung eines Pfades */
  static KNOTENTYP i,j;          /* fuer die betrachtete Kante */
  static KANTE *vglfirst, *vgllast, *h;    /* Endkanten eines Petriepfades */ 
  static KNOTENTYP vglcode[4];   /* fuer den betrachteten Petriepfad */
  static KNOTENTYP vgllen;       /* fuer den betrachteten Petriepfad */  
  static signed char erg;        /* Vergleichsergebnis               */
  static BOOL beide_richtungen;  /* Muss ein Pfad in beiden Richtungen 
                                    bewertet werden? */
  static KNOTENTYP dummy,len;    /* len = Pfadlaenge */
  static PLANMAP m2;             /* Sicherheitskopie der ANFAENGLICHEN
                                    Belegung */

  memcpy(m2,m,sizeof(KANTE *)*n*3L);   /* Es werden nacheinander die 
      Kanten der ANFAENGLICHEN Belegung durchgegangen. Auf diese Weise ist 
      sichergestellt, dass jede Kante genau einmal gewaehlt wird, denn
      "m" kann sich im Gegensatz zu "m2" veraendern. */

  farbe = 1;

  /* zunaechst nach rechts fortsetzen ("normal"): */
  for (i=0; i<n; i++) {
    for (j=0; j<3; j++) {
      if (m2[i][j]->pfadanfang==True) {
        /* Petriepfad wurde noch nicht erzeugt */
        farbe++;
        beide_richtungen = markiere_petriepfad(m2[i][j],False,farbe,vglcode,
                             &vgllen,&vglfirst,&vgllast,&dummy,&dummy);
        if ((erg = vergleiche_codes(pfadcode,vglcode,MIN(codelen,vgllen))) > 0)
           {return(False);}    /* besseren Petriepfad gefunden */
        else if (erg==0) {
          /* ab hier gilt:  Die Pfade sind vom Typ und von der Laenge gleich */

          if (pfadcode[0]==bauchbindenkennung) {            /* Bauchbinde */
            if (vergleiche_bauchbindenmenge(first,vglfirst,g1,m,n,
                pfadcode[1],is_g1)==False) {return(False);}
          }
          else {          /* Sandwich oder Brille */
            len = pfadcode[1]+pfadcode[2]+pfadcode[3];
            if (beide_richtungen) {
               /* neuer Versuch, die Richtung festzulegen */
              erg = vergleiche_angrenzende_flaechen(vglfirst,vgllast,
                    (vglfirst->nr & 1),(vgllast->nr & 1),len);
              if (erg==2) {h = vglfirst;  vglfirst = vgllast;  vgllast = h;}
                 /* pfadcode[2] und pfadcode[3] brauchen nicht vertauscht zu 
                    werden, denn sie sind gleich, wenn 
                    beide_richtungen==True. */
              beide_richtungen = (erg==0);
            }
            if (vergleiche_petriepfade(first,vglfirst,vgllast,sp,
                  (vglfirst->nr & 1),(vgllast->nr & 1),g1,m,n,beide_richtungen,
                  len,is_g1)==False)  {return(False);}
          }
        }
      }
    }
  }
   
  /* nun nach links fortsetzen ("spiegelverkehrt"): */
  for (i=0; i<n; i++) {
    for (j=0; j<3; j++) {
      if (m2[i][j]->sp_pfadanfang==True) {  
        /* Petriepfad wurde noch nicht erzeugt */
        farbe++;
        beide_richtungen = markiere_petriepfad(m2[i][j],True,farbe,vglcode,
                             &vgllen,&vglfirst,&vgllast,&dummy,&dummy);

        /* Bauchbinden sind alle abgegrast */
        /* if (vglcode[0]==bauchbindenkennung) {
          fprintf(stderr,"Logischer Fehler 2 in bester_pfad!\n"); exit(0);
        } */        

        if ((erg = vergleiche_codes(pfadcode,vglcode,MIN(codelen,vgllen))) > 0)
           {return(False);}    /* besseren Petriepfad gefunden */
        else if (erg==0) {

          len = pfadcode[1]+pfadcode[2]+pfadcode[3];
          if (beide_richtungen) { /* neuer Versuch, die Richtung festzulegen */
            erg = vergleiche_angrenzende_flaechen(vglfirst,vgllast,
                  (vglfirst->nr & 1)==0,(vgllast->nr & 1)==0,len); 
            if (erg==2) {h = vglfirst;  vglfirst = vgllast;  vgllast = h;}
            beide_richtungen = (erg==0);
          }
          if (vergleiche_petriepfade(first,vglfirst,vgllast,sp,
                (vglfirst->nr & 1)==0,(vgllast->nr & 1)==0,g1,m,n,
                beide_richtungen,len,is_g1)==False)  {return(False);}
        }
      }
    }
  }
  return(True);
}
         

/*************************************************/
/* Funktionen fuer die Konstruktion von Patches: */
/*************************************************/

/************************GEHE_NAHT_ENTLANG**********************************/
/*  Diese Funktion geht von der Kante "k" aus (die im Uhrzeigersinn
    zeigt) "len" Kanten rueckwaerts (gegen den Uhrzeigersinn) und uebergibt
    die Kante, die dann erreicht wird (Ausrichtung ebenfalls im Uhrzeiger-
    sinn). Wenn zu einer Kante der "next"-Nachfolger existiert (er zeigt nie
    nach aussen), so wird immer dieser bevorzugt. */
/*  Die Funktion ist ebenfalls dazu geeignet, einen frei "baumelnden" Pfad
    entlangzugehen. */

KANTE *gehe_naht_entlang(KANTE *k,KNOTENTYP len) {
  while (len>0) {
    if (k->next) {k = k->next->invers;}
    else         {k = k->prev->invers;}
    len--;
  }
  return(k);
}

/************************SUCHE_KANTE****************************************/
/*  Uebergibt die Kante "kk" eines Patches, deren Ursprung markiert ist und 
    bei der kk->prev nach aussen zeigt (sofern kk->prev existiert).
    "k" ist die Bruchkante, von der aus die Umrandung gegen den Uhrzeigersinn
    zu durchlaufen ist und "offset" die Anzahl der Bruchkanten, die ueber-
    sprungen werden muessen (einschliesslich "k"). 
    Das Innere des Patches darf nicht ausgefuellt sein!                    */

KANTE *suche_kante(KANTE *k,KNOTENTYP offset) {
  while (offset>0) {          /* einen Teil der Border ueberspringen */
    k = k->next->invers;
    while (k->next==nil)      /* "k" und "k->prev" sind keine Bruchkanten */
      {k = k->prev->invers->next->invers;}
    offset--;
  }
  return(k->invers->prev);
}

/*********************BAUE_NAHT*********************************************/
/*  Diese Funktion baut eine Zickzacknaht mit "len" Kanten, wobei die erste
    immer nach rechts fortgesetzt wird (falls len>1). "start" zeigt auf den
    Beginn des freien Bereichs in "map" und wird innerhalb der Funktion
    heraufgesetzt. An "anf" und "end" werden Zeiger auf die Enden der Naht
    uebergeben, wobei die Kanten immer vom Ende ins Innere der Naht
    gerichtet sind. */

void baue_naht(KNOTENTYP len,KANTE **anf,KANTE **end,
               KANTENARRAY2 map,KNOTENTYP *start) {
  static KANTE defaultkante = {nil,nil,nil,0,0,0,0,0,0,0,0,True,True};
  static KNOTENTYP i,j;

  j = *start;
  *anf = &map[j];
  map[j] = map[j+1] = defaultkante;
  map[j].invers = &map[j+1];   map[j+1].invers = &map[j];    
  j += 2;
  for (i=1; i<len; i++) {
    map[j] = map[j+1] = defaultkante;
    if (i&1) {map[j].next = &map[j-1];  map[j-1].prev = &map[j];}
    else     {map[j].prev = &map[j-1];  map[j-1].next = &map[j];}
    map[j].invers = &map[j+1];   map[j+1].invers = &map[j];
    j += 2;
  }
  *end = &map[j-1];
  *start = j;
  /* if (*start >= N_MAX*3) {fprintf(stderr,"LF in baue_naht\n"); exit(0);} */
}

/**********************SETZE_AUSSENKANTEN**********************************/
/*  Diese Funktion setzt die Aussenkanten um eine Patchumrandung.
    "anf" ist eine Kante, deren linker Nachbar nach aussen zeigt.         */

void setze_aussenkanten(KANTE *anf,KANTENARRAY2 map,KNOTENTYP *start) {
  static KANTE defaultkante = {nil,nil,nil,0,0,0,0,0,0,0,0,True,True};
  static KANTE *k;
  k = anf;
  do {
    if (k->prev==nil) {
      k->prev = k->next->next = &map[*start];
      map[*start] = defaultkante;
      map[*start].prev = k->next;
      map[*start].next = k;
      /* map[*start].invers = nil; */  /* bereits erledigt */ 
      map[*start].name = aussen;
      (*start)++;
      k = k->next->invers;    /* gegen den Uhrzeigersinn */
    }
    else {k = k->prev->invers;}   /* gegen den Uhrzeigersinn */
  } while (k!=anf);               /* einmal umranden */
}

/**********************BAUE_UMRANDUNG**************************************/
/*  Diese Funktion baut die komplette Umrandung eines BRUCHKANTENpatches.
    Die Kanten nach aussen werden ebenfalls erzeugt. Die Funktion uebergibt
    einen Zeiger auf die Kante, die vom markierten Knoten in Richtung
    Uhrzeigersinn zeigt. "start" zeigt auf  den Beginn des freien Bereichs
    in "map" und wird innerhalb der Funktion heraufgesetzt. */
    
KANTE *baue_umrandung(KNOTENTYP *code,KNOTENTYP len,KANTENARRAY2 map,
                      KNOTENTYP *start) {
  static KANTE *k,*k2,*k3,*anf;
  static KNOTENTYP i; 
  
  baue_naht((code[0]<<1)+1,&anf,&k,map,start);
  for (i=1; i<len; i++) {
    baue_naht((code[i]<<1)+1,&k2,&k3,map,start);
    k->prev = k2;    k2->next = k;
    k = k3;
  }
  k->prev = anf;   anf->next = k;

  /* nun Aussenkanten setzen (bislang alle ==nil): */
  setze_aussenkanten(anf,map,start);
  return(anf->invers->prev);
}

/***********************MACHE_MAXIMAL2**************************************/
/*  Verschiebt einen Bordercode so, dass er maximal wird. Genaugenommen wird
    nur ein Offset auf eine Stelle im Bordercode zurueckgegeben, so dass der
    Code von dort ab gelesen maximal ist. Deshalb muss der Bordercode zweimal
    hintereinander im Speicher stehen. 
    Der Unterschied zu "mache_maximal" ist, dass der Offset rueckwaerts
    verschoben wird. Er bleibt im Intervall [0,len-1] und ist als NEGATIVER
    OFFSET zu interpretieren (anf -> len-anf).                             */

KNOTENTYP mache_maximal2(KNOTENTYP *code,KNOTENTYP len) {
  static KNOTENTYP j;
  static KNOTENTYP i;          /* offset */
  static EULERTYP vgl;
  static KNOTENTYP anf;
  static KNOTENTYP len2;
  anf = 0;
  i = 1;
  len2 = len<<1;      /* zweimal "len" */
  while (i+anf<len) {
    j=len;
    while (j<len2) {  
      if ((vgl = (EULERTYP)(code[j]-code[j-i]))<0) 
        {j = len2;   code -= i;   anf += i;   i = 0;}
      else if (vgl>0) {j = len2;}
      else {j++; if (j==len2) {return(anf);} }
    }
    i++;
  }
  return(anf);
} 

/*********************BERECHNE_UMRANDUNGEN**********************************/
/*  Diese Funktion berechnet anhand der Informationen, die zu einem 
    BRUCHKANTENPatch "e" gegeben sind, die Bordercodes seiner beiden 
    Vorgaenger. Sie werden an "code1" und "code2" uebergeben. Der Ausgangs-
    bordercode steht in "bordercode". "bordercode", "code1" und "code2" sind
    Felder, in die der Bordercode zweimal hintereinander hineingeschrieben
    werden koennte. Das passiert nicht, aber der Code wird nicht vom ersten
    Arrayelement an geschrieben, sondern ab der Position "anf" bzw. "anf1"
    und "anf2", wobei "anf1" und "anf2" jeweils < N_MAX. Die letzten Eintraege
    sind "end", "end1" und "end2", so dass len=end-anf+1.
    Es wird nur vorausgesetzt, dass die Eintraege "anf" bis "end" korrekt
    gesetzt sind. Am Ende duerfen auch nur die Eintraege "anf1" bis "end1"
    bzw. "anf2" bis "end2" als korrekt vorausgesetzt werden.  */
/*  Die Offsets geben an, um wie viele Eintraege vom Nahtbeginn bzw. Ende
    gegen den Uhrzeigersinn gegangen werden muss, um zum neuen markierten
    Knoten (der also zum maximalen Code fuehrt) zu gelangen. Bei
    Einschluessen ist "offset2" bedeutungslos. */

void berechne_umrandungen(ELEM *e,KNOTENTYP *bordercode,KNOTENTYP anf,
     KNOTENTYP end,KNOTENTYP *code1,KNOTENTYP *anf1,KNOTENTYP *end1,
     KNOTENTYP *offset1,KNOTENTYP *code2,KNOTENTYP *anf2,KNOTENTYP *end2,
     KNOTENTYP *offset2) {
  static KNOTENTYP ziellen;  
  static KNOTENTYP pos,pos1,pos2,len1,len2,i;

  /* if (e->prev1==nil || e->prev2==nil) 
        {fprintf(stderr,"LF0 in berechne_umrandungen\n"); exit(0);} */

  if (e->art < 2) {       /* Durchschnitt */
    /* erst vom markierten Knoten rueckwaerts bis zum Nahtende laufen
       (konstanter Teil von Patch 1): */
    pos1 = N_MAX;     /* => pos1 kann nicht <0 werden */
 
    if (e->prev1->prev1) {     /* erster Vorgaenger hat mehr als 1 Flaeche */  
      ziellen = e->ziellen-1;
      pos = end;
      code1[--pos1] = 0;     /* weil markierter Knoten an Bruchkante haengt =>
                                Kante k ist unmittelbar folgende Bruchkante */
      while ((i=(bordercode[pos]<<1)+1) < ziellen) {
        /* Gleichheit ist unmoeglich, darum "<" dasselbe wie "<=" */
        code1[--pos1] = bordercode[pos];
        ziellen -= i;
        pos--;
      }
      code1[--pos1] = (ziellen-1)>>1;
    }
    else {code1[pos1] = 0;   pos = end - ((e->prev1->i)-3-e->nahtlen);
          *offset1 = 0;} 
       /* Bordercode ist uninteressant, nur der Eintrag code1[pos1] und die 
          Position "pos" sind fuer Patch 2 wichtig. "e->nahtlen" ist entweder
          1 oder 2. Falls e->nahtlen==1, so ist die abgespaltene Flaeche 
          mindestens ein 4-eck, bei e->nahtlen==2 mindestens
          ein 5-eck, denn sonst laege ein Bauchbindenpatch vor. */       

    /* nun vom Nahtende rueckwaerts bis zum markierten Knoten laufen  
       (konstanter Teil von Patch 2): */
    pos2 = N_MAX;             /* => pos2 kann nicht <0 werden */

    if (e->prev2->prev1) {     /* zweiter Vorgaenger hat mehr als 1 Flaeche */    
      code2[--pos2] = bordercode[pos] - code1[pos1] - 1 - (pos==anf);
      if (pos-- > anf) {     /* => nach der Reduzierung gilt noch pos>=anf  */
                             /* => Code noch nicht komplett durchlaufen     */
        memcpy(&code2[pos2 -= (pos-anf)],&bordercode[anf+1],
               sizeof(KNOTENTYP)*(pos-anf));
        code2[--pos2] = bordercode[anf] - 1;
      }     /* ab hier wird "pos" nicht mehr gebraucht */
    }
    else {*offset2 = 0;}
    
    /* nun Naht aufteilen (auch wenn Code uninteressant, wird nichts 
       zerstoert): */
    if (e->art==0) {     /* gerader Durchschnitt */
      code1[--pos1] = 0;
      code1[--pos1] = (code2[*anf2 = --pos2] = (e->nahtlen>>1)) - 1;
      code1[*anf1 = --pos1] = 0;
    }
    else {               /* ungerader Durchschnitt */
      code1[--pos1] = code2[--pos2] = e->nahtlen>>1;    
      code1[*anf1 = --pos1] = code2[*anf2 = --pos2] = 0;
    }
  }

  else {           /* Einschluss => beide Vorgaenger je mehr als 1 Flaeche =>
                      Codes sind immer interessant */
    /* vom markierten Knoten rueckwaerts bis zum Nahtende laufen
       Konstanter Teil von Patch 1): */ 
    code1[N_MAX-1] = 0;    /* da k unmittelbar auf Bruchkante folgt */
    memcpy(&code1[N_MAX-(end-anf)-1],
           &bordercode[anf+1],sizeof(KNOTENTYP)*(end-anf));
    /* gesamter alter Code bis auf den ersten Eintrag */
    pos1 = N_MAX-(end-anf)-1;
    code1[--pos1] = bordercode[anf]-1;    /* erster Eintrag */

    /* nun Naht aufteilen: */
    switch (e->art) {
      case 2: {  
        code2[*anf2 = N_MAX-1] = e->ziellen>>1;
        code1[*anf1 = pos1-4] = code1[pos1-2] = 0;
        /* if (e->nahtlen&1) 
              {fprintf(stderr,"LF in berechne_umrandung\n"); exit(0);} */
        code1[pos1-3] = (e->nahtlen>>1)-1;
        code1[pos1-1] = code1[pos1-3] + code2[*anf2] + 1;
        break;
      }
      case 3: {
        code2[*anf2 = N_MAX-2] = (e->ziellen>>1) - 1;
        code2[N_MAX-1] = 0;
        /* if (e->nahtlen&1) 
              {fprintf(stderr,"LF2 in berechne_umrandung\n"); exit(0);} */
        code1[*anf1 = pos1-3] = 0;
        code1[pos1-2] = (e->nahtlen>>1)-1;
        code1[pos1-1] = code1[pos1-2] + code2[*anf2] + 2;
        break;
      }
      case 4: {
        code2[*anf2 = N_MAX-1] = e->ziellen>>1;
        code1[*anf1 = pos1-4] = code1[pos1-2] = 0;
        /* if ((e->nahtlen&1)==0) 
              {fprintf(stderr,"LF3 in berechne_umrandung\n"); exit(0);} */
        code1[pos1-1] = e->nahtlen>>1;
        code1[pos1-3] = code1[pos1-1] + code2[*anf2];
        break;
      }
      case 5: {
        code2[*anf2 = N_MAX-2] = (e->ziellen>>1) - 1;
        code2[N_MAX-1] = 0;
        /* if ((e->nahtlen&1)==0) 
              {fprintf(stderr,"LF4 in berechne_umrandung\n"); exit(0);} */
        code1[*anf1 = pos1-3] = 0;
        code1[pos1-1] = e->nahtlen>>1;
        code1[pos1-2] = code1[pos1-1] + code2[*anf2] + 1;
        break;
      }
    }
  }

  /* nun Codes maximieren (fuer die Kopien ist immer Platz): */
  if (e->prev1->prev1) {    /* sonst Code uninteressant */
    len1 = N_MAX - *anf1; 
    memcpy(&code1[N_MAX],&code1[*anf1],sizeof(KNOTENTYP)*len1);
    *offset1 = mache_maximal2(&code1[*anf1],len1);
    *end1 = N_MAX-1 + len1 - *offset1;
    /* end1 von N_MAX-1 nach hinten verschieben */
    *anf1 += len1 - *offset1;
  }
  if (e->prev2->prev1) {    /* sonst Code uninteressant */
    len2 = N_MAX - *anf2;
    memcpy(&code2[N_MAX],&code2[*anf2],sizeof(KNOTENTYP)*len2);
    *offset2 = mache_maximal2(&code2[*anf2],len2);
    *end2 = N_MAX-1 + len2 - *offset2;
    *anf2 += len2 - *offset2;  
  }
}

/*******************MARKIERE_N_ECK******************************************/
/* Rechts von der Kante "k" befindet sich ein n-Eck. Diese Funktion geht ein-
   mal herum und markiert alle benachbarten Kanten. */

void markiere_n_eck(FLAECHENTYP n,KANTE *k) {
  static FLAECHENTYP i;
  for (i=0; i<n; i++) {
    k->fl_rechts = k->invers->fl_links = n;
    k = k->invers->prev;
  }
} 

void bestimme_gute_basen(ELEM *e) {
     if ((e->test&4)==0) {e->test |= 4;   gute_basen++;}
     if (e->prev1) {bestimme_gute_basen(e->prev1);}
     if (e->prev2) {bestimme_gute_basen(e->prev2);}
}
  
/*********************KONSTRUIERE_PATCH*************************************/
/* Konstruiert den Patch e rekursiv. Der Patch wird an m uebergeben.
   "k" ist die Kante, deren Ursprung markiert wird. "start" ist die
   Nummer des ersten freien Elements im Array m.                           */
/* k->ursprung ist markiert und k->prev wuerde aus dem Patch(-teil) heraus-
   zeigen, wenn k->prev existierte. */
/* WICHTIG: Die Funktion geht davon aus, dass e ein BRUCHKANTENPATCH ist.  */

void konstruiere_patch_rek(KANTENARRAY2 m,KNOTENTYP *start,
     KNOTENTYP *bordercode,KNOTENTYP anf,KNOTENTYP end,ELEM *e,KANTE *k) {
  static KANTE *k2;
  KANTE *anfang,*ende;                /* Enden der Naht (NICHT static) */      
  KNOTENTYP code1[2*N_MAX], anf1, end1, offset1, 
            code2[2*N_MAX], anf2, end2, offset2;       /* NICHT static */

  /* if ((e->test&1)==0 && doppelkantentest==False) 
        {mittelbar++;   e->test |= 1;} */

  if (e->prev1==nil) {                  /* einzelne Flaeche */
    markiere_n_eck((FLAECHENTYP)(e->i),k);
    /* Wegen des Flaechensequenzvergleichs ist die Markierung IMMER wichtig. */
    /* Auch fuer die Funktion "drei_vierecke", "ueberpruefe_wege_in_patch"
       etc. */
    return;
  }

  else {                               /* Patch aus mehreren Flaechen */
    berechne_umrandungen(e,bordercode,anf,end,code1,&anf1,&end1,&offset1,
                         code2,&anf2,&end2,&offset2);

    if (e->art > 1) {     /* Einschluss */
      baue_naht(e->ziellen + e->nahtlen,&anfang,&ende,m,start);

      anfang->prev = k->invers->next;       /* Anfang verknuepfen */
      anfang->next = k->invers;
      k->invers->prev = k->invers->next->next = anfang;

      k2 = gehe_naht_entlang(anfang->invers,e->nahtlen-1);
           /* Ende verknuepfen */
      if (k2->next) {    /* Anschluss links */ 
        /* if (e->nahtlen&1 || e->art>3) 
              {fprintf(stderr,"LF in konstruiere_patch_rek\n"); exit(0);} */
        k2->prev = k2->next->next = ende;
        ende->next = k2;
        ende->prev = k2->next;
      }
      else {             /* Anschluss rechts */
        /* if ((e->nahtlen&1)==0 || e->art<4)
	      {fprintf(stderr,"LF3 in konstruiere_patch_rek\n"); exit(0);} */
        k2->next = k2->prev->prev = ende;
        ende->next = k2->prev;
        ende->prev = k2;
      }

      if (e->art==4)                     /* Patch 2 konstruieren */
         {konstruiere_patch_rek(m,start,code2,anf2,end2,e->prev2,k2->next);}
      else {konstruiere_patch_rek(m,start,code2,anf2,end2,e->prev2,
            k2->next->invers->prev);}
    }

    else {             /* Durchschnitt */
      baue_naht(e->nahtlen,&anfang,&ende,m,start);

      anfang->prev = k->invers->next;          /* Anfang verknuepfen */
      anfang->next = k->invers;
      k->invers->prev = k->invers->next->next = anfang;
      
      k2 = gehe_naht_entlang(k,e->ziellen-1);    /* Ende verknuepfen */
      k2->next = k2->prev->prev = ende;
      ende->next = k2->prev;
      ende->prev = k2;

      k2 = suche_kante(ende->next->invers,offset2);  /* Patch 2 konstruieren */
      konstruiere_patch_rek(m,start,code2,anf2,end2,e->prev2,k2);
    }         

    /* Patch 1 konstruieren */
    k2 = suche_kante(anfang->next->invers,offset1);
    konstruiere_patch_rek(m,start,code1,anf1,end1,e->prev1,k2);
  }                
}           

KANTE *konstruiere_patch(KANTENARRAY2 m,KNOTENTYP *bordercode,KNOTENTYP len,
      ELEM *e) {
  static KANTE *k;
  static KNOTENTYP start;
  start = 0;
  k = baue_umrandung(bordercode,len,m,&start);
  konstruiere_patch_rek(m,&start,bordercode,0,len-1,e,k);
  return(k);
}  
          
/*********************KONSTRUIERE_BB_PATCH**********************************/
/*  Diese Funktion arbeitet wie "konstruiere_patch", laeuft aber nur fuer
    Bauchbindenpatches. Da Bauchbindenpatches immer nur in der obersten
    Rekursionsebene auftreten koennen, kann anschliessend "konstruiere_
    patch_rek" aufgerufen werden.  */
 
KANTE *konstruiere_bb_patch(KANTENARRAY2 m,KNOTENTYP bordercode,ELEM *e) {
  static KNOTENTYP start;
  static KANTE *k,*k2;
  static KANTE *anfang,*ende;            /* Enden der Naht */      
  static KNOTENTYP code1[2*5], anf1, end1, len1, offset1, 
                   code2[2*5], anf2, end2, len2, offset2;
  /* 5 = laengstmoegliche neue Umrandung */ 

  /* Umrandung bauen */
  start = 0;
  baue_naht(bordercode<<1,&anfang,&ende,m,&start);
  anfang->prev = ende;
  ende->next = anfang;
  setze_aussenkanten(anfang,m,&start);
  k = ende->invers;    /* k->ursprung ist markiert, k->prev aussen */

  /* neue Umrandungscodes berechnen */
  switch (e->art) {
    case 0: {
      code1[anf1 = 1] = code1[3] = 0;  
      code1[2] = (code2[anf2 = 3] = e->nahtlen>>1)-1;
      code1[4] = (e->ziellen>>1)-1;
      code2[4] = bordercode - code1[4] - 2;
      break;
    }
    case 1: {
      code1[anf1 = 2] = code2[anf2 = 2] = 0;
      code1[3] = code2[3] = e->nahtlen>>1;
      code1[4] = (e->ziellen>>1)-1;
      code2[4] = bordercode - code1[4] - 2;
      break;
    }
    case 2: {
      code2[anf2 = 4] = e->ziellen>>1;
      code1[anf1 = 0] = code1[2] = 0;
      code1[1] = (e->nahtlen>>1)-1;
      code1[3] = code1[1] + code2[4] + 1;
      code1[4] = bordercode - 1;
      break;
    }
    case 3: {
      code2[anf2 = 3] = (e->ziellen>>1)-1;
      code2[4] = 0;
      code1[anf1 = 1] = 0;
      code1[2] = (e->nahtlen>>1)-1;
      code1[3] = code1[2] + code2[3] + 2;
      code1[4] = bordercode - 1;
      break;
    }
    case 4: {
      code2[anf2 = 4] = e->ziellen>>1;
      code1[anf1 = 0] = code1[2] = 0;
      code1[3] = e->nahtlen>>1;
      code1[1] = code1[3] + code2[4];
      code1[4] = bordercode - 1;
      break;
    }
    case 5: {
      code2[anf2 = 3] = (e->ziellen>>1)-1;
      code2[4] = 0;
      code1[anf1 = 1] = 0;
      code1[3] = e->nahtlen>>1;
      code1[2] = code1[3] + code2[3] + 1;
      code1[4] = bordercode - 1;
      break;
    }
  }
  len1 = 5 - anf1;    len2 = 5 - anf2;
  memcpy(&code1[5],&code1[anf1],sizeof(KNOTENTYP)*len1);
  memcpy(&code2[5],&code2[anf2],sizeof(KNOTENTYP)*len2);
  /* fuer die obigen zwei Kopien ist auf jeden Fall Platz */
  offset1 = mache_maximal2(&code1[anf1],len1);
  offset2 = mache_maximal2(&code2[anf2],len2);
  end1 = 4 + len1 - offset1;   /* end1 von 4 nach hinten verschieben */
  end2 = 4 + len2 - offset2;
  anf1 += len1 - offset1;
  anf2 += len2 - offset2;  

  /* Naht bauen (wie bei "konstruiere_patch_rek") */
  if (e->art > 1) {     /* Einschluss */
    baue_naht(e->ziellen + e->nahtlen,&anfang,&ende,m,&start);
 
    anfang->prev = k->invers->next;       /* Anfang verknuepfen */
    anfang->next = k->invers;
    k->invers->prev = k->invers->next->next = anfang;

    k2 = gehe_naht_entlang(anfang->invers,e->nahtlen-1);   
         /* Ende verknuepfen */
    if (k2->next) {    /* Anschluss links */ 
      /* if (e->nahtlen&1 || e->art>3) 
            {fprintf(stderr,"LF in konstruiere_bb_patch_rek\n"); exit(0);} */
      k2->prev = k2->next->next = ende;
      ende->next = k2;
      ende->prev = k2->next;
    }
    else {             /* Anschluss rechts */
      /* if ((e->nahtlen&1)==0 || e->art<4)
	    {fprintf(stderr,"LF3 in konstruiere_bb_patch_rek\n"); exit(0);} */
      k2->next = k2->prev->prev = ende;
      ende->next = k2->prev;
      ende->prev = k2;
    }

    if (e->art==4)                     /* Patch 2 konstruieren */
       {konstruiere_patch_rek(m,&start,code2,anf2,end2,e->prev2,k2->next);}
    else {konstruiere_patch_rek(m,&start,code2,anf2,end2,e->prev2,
          k2->next->invers->prev);}
  }

  else {             /* Durchschnitt */
    baue_naht(e->nahtlen,&anfang,&ende,m,&start);

    anfang->prev = k->invers->next;          /* Anfang verknuepfen */
    anfang->next = k->invers;
    k->invers->prev = k->invers->next->next = anfang;
      
    k2 = gehe_naht_entlang(k,e->ziellen-1);    /* Ende verknuepfen */
    k2->next = k2->prev->prev = ende;
    ende->next = k2->prev;
    ende->prev = k2;

    k2 = suche_kante(ende->next->invers,offset2);   /* Patch 2 konstruieren */
    konstruiere_patch_rek(m,&start,code2,anf2,end2,e->prev2,k2);
  }         

  /* Patch 1 konstruieren */
  k2 = suche_kante(anfang->next->invers,offset1);
  konstruiere_patch_rek(m,&start,code1,anf1,end1,e->prev1,k2);             

  return(k);
}           


/**********************************************************/
/* Funktionen zur Konstruktion von Graphen (aus Patches): */
/**********************************************************/

/************************SUCHE_NAHT*****************************************/
/*  Uebergibt die Breakedge, die sich vor dem durch "pos" bestimmten Kanten-
    zug befindet. k ist die Kante, die vom markierten Knoten zum Beginn
    der (beabsichtigten) Naht fuehrt.                                      */

KANTE *suche_naht(KNOTENTYP *code,KNOTENTYP pos,KANTE *k) {
  static int i,j;        /* i = Teil der Border, der uebersprungen wird */
  i = 0;
  while (i<pos) {             /* einen Teil der Border ueberspringen */
    for (j=0; j<code[i]; j++) {k = k->invers->next->invers->prev;}
    k = k->invers->prev;      /* break-edge ueberspringen */
    /* if (k->prev->name!=aussen) {fprintf(stderr,"Logischer Fehler 1  %d\n",
                                   k->prev->name);
                                   schluss();} */
    i++;
  }
  k = k->next->invers;   /* einen Schritt zurueck */
  /* if (k->invers->next->name!=aussen) 
        {fprintf(stderr,"Logischer Fehler 2\n"); schluss();} */
  return(k);
}

/************************GUTER_GRAPH*****************************************/
/*  Diese Funktion verarbeitet einen guten Graphen m mit n Knoten: 
    Sie gibt ihn aus und sie erstellt eventuell die Flaechenstatistik (dann
    sind e1-e3 die Zeiger auf die Patches, e3==nil bei Bauchbindengraphen). */
/*  Falls "is_code==True", so enthaelt "graph" bereits den auszugebenden
    Planarcode, andernfalls wird er innerhalb der Funktion berechnet.       */

void guter_graph(KNOTENTYP n,PLANMAP m,ELEM *e1,ELEM *e2,ELEM *e3,
                 KNOTENTYP *graph,BOOL is_code) {
  static int i,j;
  static size_t size;
  static unsigned char conn;

  /* Zusammenhangsstatistik */
  if (do_conn) {conn = getconn(m,n); connzahl[n>>1][conn-1]++;}  

  if (output && (!do_conn ||
     (conn==1 && conn1) || (conn==2 && conn2) || (conn==3 && conn3))) {
    if (dual) {size = dualcode(graph,m,n);}  /* "graph" ist gross genug */
    else {
      size = CODESIZE(n);
      if (!is_code) {map_2_planarcode(m,graph,n);}
    }
    schreibe_planarcode(graph,outputfile[n>>1],size);
  }

  /* Flaechenstatistik */
  if (facestat) {
    j = 0;         /* Index auf passendes Arrayelement */
    for (i=0; i<anz_face; i++) {
      if ((e1->flaechenzahl)[i] || (e2->flaechenzahl)[i] || 
          (e3 && (e3->flaechenzahl)[i]))
        {j += (1<<i);}    /* i-te Flaeche ist im Graphen enthalten */
    }
    (facestatarray[j])++;
  }
}
          
/****************VERKNUEPFE_BAUCHBINDENPATCHES*******************************/
/* Verknuepft Bauchbindenpatches entlang der Grenzen, die 2*len Kanten lang
   sind.  k1 wird mit k2->invers identifiziert.                             */
/* Die Grenze von k2 wird aufgeloest.                                       */

void verknuepfe_bauchbindenpatches(KANTE *k1,KANTE *k2,KNOTENTYP len) {
  static KNOTENTYP i;
  k2 = k2->invers;
  for (i=len<<1; i>0; i--) {  
    k1->fl_links = k1->invers->fl_rechts = k2->fl_links;
    if (k1->prev->name==aussen) {
      k1->prev = k1->next->next = k2->prev;    /* Verknuepfung */
      k2->prev->next = k1;
      k2->prev->prev = k1->next;
      k1 = k1->next->invers;                   /* weiter gehen */
      k2 = k2->next->invers;                   /* weiter gehen */
    }
    else {
      k1 = k1->prev->invers;
      k2 = k2->prev->invers;
    }  
  }
}

/***********BILDE_GRAPHEN_AUS_BAUCHBINDENPATCHES*****************************/
/*  Bildet Graphen aus zwei Bauchbindenpatches. Die resultierenden Graphen
    besitzen "n" Knoten und "fl" Flaechen, wobei "fl" nicht "f_max" sein muss.
    Deshalb kann als obere Grenze fuer die Patches nicht "f_max1" benutzt
    werden, sondern "fl-x" mit x=2,3 oder 6.                                */

void bilde_graphen_aus_bauchbindenpatches(KNOTENTYP n) {
  static KNOTENTYP fl1,fl2;    /* Flaechenzahlen der beteiligten Patches */
  static KNOTENTYP fl;         /* Anzahl Flaechen im erzeugten Graphen */
  static BBTREENODE *t1,*t2,*t3,*t4;
  static ELEM *e1,*e2;         /* Zeiger auf die beteiligten Patches */  
  static KANTE *k1, *k2, *dummykante;
  static KANTENARRAY2 patchmap1,patchmap2;
  static PLANMAP m;                         /* fuer den Graphen */
  static KNOTENTYP graph[CODESIZE(N_MAX)];   /* fuer Originalnumerierung 
                                      (gleichzeitig Vergleichs-planarcode) */
  static KNOTENTYP dummy;
  static KNOTENTYP pfadcode[2];
  static BOOL is_num;  /* True => Patches vom Originalpfad wurden numeriert */

  fl = (n>>1)+2;

  for (fl1=f_min1; fl1<=MIN(fl-f_min1,fl>>1); fl1++) {    /* fl1<=fl2 */ 
    fl2 = fl-fl1;
    t1 = bbtree[fl1];
    while (t1) {
      t2 = bbtree[fl2];
      while (t2) {
        if (t1->code+t2->code>=krit_min && t1->code+t2->code<=krit_max) {
          /* Kritische Flaechen passen zusammen */
          t3 = t1->nextlevel;    /* t3 und t4 zeigen auf zweiten Level */
          t4 = t2->nextlevel;    /* Bordercodes aufeinander abstimmen */
          while (t3 && t4) {     /* naechstes Paar finden */
            while (t3 && t4 && t3->code!=t4->code) {
              if (t3->code < t4->code)  {t3 = t3->next;}
              else                      {t4 = t4->next;}
            }
            if (t3 && t4) {          /* => t3->code==t4->code */
              e1 = t3->firstpatch;
              while (e1) {
                e2 = t4->firstpatch;
                while (e2) {
                  if (!facerestrict || flaechenzahlen2_ok(e1,e2,True)) {
                    /* if ((e1->test&2)==0) {unmittelbar++; e1->test |= 2;}
                       if ((e2->test&2)==0) {unmittelbar++; e2->test |= 2;}
                       if ((e1->test&1)==0) {mittelbar++; e1->test |= 1;}
                       if ((e2->test&1)==0) {mittelbar++; e2->test |= 1;} */
                    k1 = konstruiere_bb_patch(patchmap1,t3->code,e1);
                    k2 = konstruiere_bb_patch(patchmap2,t4->code,e2);
                    bestimme_gute_basen(e1);
                    bestimme_gute_basen(e2);
                    graphenzahl[n>>1][0]++;
                    verknuepfe_bauchbindenpatches(k1,k2,t3->code);
                    numeriere_graph_sp(m,k1);  /* k1 gilt als erste Kante */
                      
                    if (!(barnette && drei_vierecke(m,n))) {
                      /* Wird Graph gebraucht?  Wichtig: Vor Aufruf der
                         Funktion "drei_vierecke" sind alle nr-Eintraege 0 */

                      /* Minimalitaetstest: */
                      dummy = markiere_petriepfad(k1,True,1,pfadcode,&dummy,
                                           &dummykante,&dummykante,nil,nil); 
                      is_num = False;
                      /* Richtung festlegen (falls e1==e2 => symmetrisch): */
                      if (e1!=e2 && vergleiche_petriepfade(k1,k1->invers,nil,
                          True,True,True,graph,m,n,False,pfadcode[1],&is_num)
                          ==False) {
                        /* Richtung umdrehen und besseren Code nehmen. Dass der
                           schlechtere Code noch als Originalnumerierung in 
                           "m" ist, ist egal, denn "is_num" ist jetzt "True" 
                           und "graph" erhaelt den besseren Code. */
                        k1 = k1->invers;
                        if (is_num)    /* Code ist bereits vorhanden */
                           {map_2_planarcode2(m,graph,n);}
                        else {         /* Code ist noch nicht vorhanden */
                          numeriere_graph2_sp(m,k1);
                          map_2_planarcode2(m,graph,n);
                          is_num = True;
                        }
                      }
                      /* mit anderen Petriepfaden vergleichen: */
                      if (vergleiche_bauchbindenmenge(k1,k1->invers->next,
                          graph,m,n,pfadcode[1],&is_num)) { 
                        /* Vergleich hat noch keinen besseren Pfad ergeben */
                        if (bester_petriepfad(pfadcode,2,k1,True,m,n,graph,
                           &is_num)) {   /* kanonisch erzeugt */
                          non_iso_graphenzahl[n>>1][0]++;
                          guter_graph(n,m,e1,e2,nil,graph,is_num);
                        } 
                      }
                    }
                  }
 
                  if (e1==e2) {e2 = nil;} else {e2 = e2->next;}
                  /* Patches aus gleicher Liste nicht zweimal verknuepfen */
                }
                e1 = e1->next;
              }
              t3 = t3->next;  t4 = t4->next;
            }
          }
        }      /* if (in krit-intervall) */
        if (t1==t2) {t2 = nil;} else {t2 = t2->next;}
        /* Patches aus gleichem Baum nicht zweimal verknuepfen */
      }        /* while t2 */
      t1 = t1->next;
    }          /* while t1 */
  }            /* for */
}

/***********************MACHE_MAXIMAL***************************************/
/*  Verschiebt einen Bordercode so, dass er maximal wird. Genaugenommen wird
    nur ein Offset auf eine Stelle im Bordercode zurueckgegeben, so dass der
    Code von dort ab gelesen maximal ist. Deshalb muss der Bordercode zweimal
    hintereinander im Speicher stehen. */

KNOTENTYP mache_maximal(KNOTENTYP *code,KNOTENTYP len) {
  static KNOTENTYP i,j;       /* i = offset */
  static EULERTYP vgl;
  static KNOTENTYP anf;
  anf = 0;
  i = 1;
  while (i+anf<len) {
    j=0;
    while (j<len) {  
      if ((vgl = (EULERTYP)(code[j]-code[i+j]))<0) 
        {j = len;   code += i;   anf += i;   i = 0;}
      else if (vgl>0) {j = len;}
      else {j++; if (j==len) {return(anf);} }
    }
    i++;
  }
  return(anf);
} 

/**********************SUCHE_BORDERCODE*************************************/
/*  Diese Funktion ist zugeschnitten auf die Suche nach einem vorgegebenen
    Bordercode mit mindestens 2 und maximal 4 Elementen. "len" ist die 
    Laenge des Bordercodes,
    "code" enthaelt den Code selbst. Der Code kann in einzelnen Positionen
    veraendert werden. In "aend" ist Bit i gesetzt, wenn die i-te Position
    des Bordercodes veraendert werden darf. Die einzige erlaubte Aenderung
    ist jedoch folgende: Alle Positionen, in denen eine Aenderung vorgenommen
    werden darf, duerfen um den gleichen Wert erhoeht werden.
    Beispiel:  Code (4,0,2,0) ist vorgegeben und die Bits 0 und 2 sind
    gesetzt. Dann wird nach allen Codes der Form (4+k,0,2+k,0) mit k>=0
    gesucht. Wenn ueberhaupt ein code-Eintrag veraendert werden darf, so muss
    es unter anderem der erste sein, denn sonst ist der erste Eintrag und
    damit der gesuchte Code irgendwann nicht mehr maximal. 
    Die Funktion sucht also nach mehreren Codes. Damit sie nicht immer von
    vorn suchen muss, ist es sinnvoll, ihr die Position des letzten Fundes
    zu uebergeben. Dies geschieht durch das Array "t3". Die Werte t3[0] bis
    t3[3] sind die aktuellen Zeiger im Baum, und zwar fuer die Baumtiefen 0
    bis 3.  Gleichzeitig enthaelt "code" den aktuellen Code. 
    Dieser wird waehrend der Suche veraendert, genauso wie die Werte in "t3".
    "code" und "t3" erhalten also immer den aktuellen Zwischenstand der Suche,
    so dass beim naechsten Aufruf der Funktion an der betreffenden Stelle
    fortgesetzt werden kann.
    Das Suchen von einem Fundort zum naechsten ist moeglich, weil die Fundorte
    geordnet sind.
    Vor dem ersten Aufruf der Funktion fuer einen bestimmten Code muss t3[0]
    auf den ersten Knoten des (Teil-)Baums zeigen und t3[1] muss den Wert 
    "nil" haben. An t3[1]==nil erkennt die Funktion, dass sie zum ersten Mal
    aufgerufen wurde. Deshalb muss der Bordercode mindestens eine Laenge von 
    2 haben (siehe oben).
    Die Funktion liefert den Zeiger auf einen Knoten, der einen passenden
    Bordercode repraesentiert, oder "nil", falls es keinen (mehr) gibt.
    Zusaetzlich wird an "k" der Wert uebergeben, um den die variablen Eintraege
    insgesamt erhoeht wurden. Auch hier gilt: Falls die Funktion nicht zum
    ersten Mal aufgerufen wird, muss der alte Wert an die Funktion uebergeben
    werden. */
 
TREENODE *suche_bordercode(KNOTENTYP *code,KNOTENTYP len,unsigned char aend,
  TREENODE **t3,KNOTENTYP *k) {
  static KNOTENTYP pos;     /* pos = Tiefe im Patchbaum */
  static EULERTYP i;   
  static BOOL erhoehen;     /* True => variable Werte erhoehen */ 
  if (t3[1]) {erhoehen = True;}  /* nicht erster Aufruf mit aktuellen Werten */
  else {pos = 0;  erhoehen = False;}
  /* Nun ist "pos" die Tiefe im Patchbaum, in der der Code verglichen werden
     muss. */
  while (t3[0]) {           /* Baum nicht komplett durchsucht */
    if (erhoehen) {
      for (i=(EULERTYP)len-1; i>=0; i--) /* code-Eintraege simultan erhoehen */
        {if (aend&(1<<i)) {pos = (KNOTENTYP)i;   code[i]++;} }
      (*k)++;   erhoehen = False;
    }
    while (t3[pos] && t3[pos]->code<code[pos]) {t3[pos] = t3[pos]->next;}
    if (t3[pos] && t3[pos]->code==code[pos]) {       /* Codeeintrag passt */
      if (pos+1==len) {                              /* Code vollstaendig */
        if (t3[pos]->firstpatch) {return(t3[pos]);}  /* Es gibt Patch */
        else {erhoehen = True;}   /* kein Patch => naechsten Code probieren */
      }
      else {            /* Code noch nicht vollstaendig */
        if (t3[pos]->nextlevel) {t3[pos+1] = t3[pos]->nextlevel;  pos++;}
        else {erhoehen = True;}   /* Code kann nicht vervollstaendigt werden */
      }
    }
    else {erhoehen = True;}     /* "pos"-ten Codeeintrag nicht gefunden */
  }
  return(nil);
}
  
/*************VERKNUEPFE_SANDWICHPATCHES************************************/
/* Diese Funktion verknuepft die Patches, die durch k1, k2 und k3 gegeben
   sind, wobei k1 und k3 die "Fleischeinlagen" und k2 die Umrandung vertritt.
   len1, len2 und len3 sind die Laengen der drei Nahtteile, code2 ist der
   Bordercode von Patch 2, "i" gibt an, wo der Bordercode von Patch 2
   fuer die Verknuepfung beginnt (wie bei der Erzeugung von Patches).
   "fall" gibt den Fall 1-4 an.                                            */
/* Da die Kante "k2" bei der Verknuepfung verschwindet, waere es ein
   Fehler, sie spaeter zum Ansprechen des Graphen zu verwenden. Deshalb gibt
   die Funktion einen Zeiger auf eine Kante zurueck, die wirklich im Graphen
   bleibt, und zwar die Kante "k4". */ 

KANTE *verknuepfe_sandwichpatches(KANTE *k1, KANTE *k2, KANTE *k3, KNOTENTYP
     len1, KNOTENTYP len2, KNOTENTYP len3, KNOTENTYP *code2, KNOTENTYP i,
     char fall) {
  static KANTE *k4;           /* Beginn der Umrandungsnaht */
  static KANTE *returnkante;
  k2 = (returnkante = k4 = suche_naht(code2,i,k2))->next->invers;
  if (fall<3) {k1 = k1->next;} else {k1 = k1->invers;}
  /* nun zeigen k1 und k2 parallel zum ersten 3er-Verknuepfungspunkt aus 
     Richtung Naht zwischen Patch 1 und 2 */

  /* Patches 1 und 2 vernetzen (Border von Patch 2 wird aufgeloest): */
  while (len3>1) {
    k1->fl_rechts = k1->invers->fl_links = k2->fl_rechts; 
    if (k1->next->name==aussen) {
      k1->next = k1->prev->prev = k2->next;
      k2->next->prev = k1;
      k2->next->next = k1->prev;
      k1 = k1->prev->invers;   k2 = k2->prev->invers;
    }
    else {k1 = k1->next->invers;  k2 = k2->next->invers;}
    len3--;
  }

  /* zweiten 3er-Verknuepfungspunkt vernetzen: */
  k1->fl_rechts = k1->invers->fl_links = k2->fl_rechts;
  k1->next = k1->prev->prev = k2->next;   k2->next->prev = k1;
  k2->next->next = k1->prev;

  /* Patches 1 und 3 vernetzen (Border von Patch 3 wird aufgeloest): */
  k1 = k1->prev->invers;
  if (fall==1 || fall==3) {k3 = k3->next->invers;}
  else      {k3 = k3->next->invers->next->invers;}
  while (len2>1) {
    k1->fl_rechts = k1->invers->fl_links = k3->fl_rechts;
    if (k1->next->name==aussen) {
      k1->next = k1->prev->prev = k3->next;
      k3->next->prev = k1;
      k3->next->next = k1->prev;
      k1 = k1->prev->invers;   k3 = k3->prev->invers;
    }
    else {k1 = k1->next->invers;   k3 = k3->next->invers;}
    len2--;
  }

  /* ersten 3er-Verknuepfungspunkt vernetzen: */
  k1->fl_rechts = k1->invers->fl_links = k3->fl_rechts;
  k4->prev = k1;   k4->next = k1->prev;   k1->prev->prev = k1->next = k4;

  /* Patches 2 und 3 vernetzen (Border von Patch 3 wird aufgeloest): */
  k3 = k3->next->invers;    k4 = k4->invers;
  while (len1>1) {
    k4->fl_rechts = k4->invers->fl_links = k3->fl_rechts;
    if (k4->next->name==aussen) {
      k4->next = k4->prev->prev = k3->next;
      k3->next->prev = k4;
      k3->next->next = k4->prev;
      k4 = k4->prev->invers;   k3 = k3->prev->invers;
    }
    else {k4 = k4->next->invers;   k3 = k3->next->invers;}
    len1--;
  }
  k4->fl_rechts = k4->invers->fl_links = k3->fl_rechts;
  return(returnkante);
}
  
/***********BILDE_GRAPHEN_AUS_SANDWICHPATCHES*******************************/
/*  Bildet Graphen aus drei Sandwichpatches. Die resultierenden Graphen
    besitzen "n" Knoten und "fl" Flaechen, wobei "fl" nicht "f_max" sein muss.
    Deshalb kann als obere Grenze fuer die Patches nicht "f_max1" benutzt
    werden, sondern "fl-x1-x2" mit x1=1 (minimale Groesse des mittleren 
    Patches 2) und x2=2 (minimale Groesse eines Fleischeinlagepatches
    1 oder 3) bzw. "fl-2*x2" fuer den mittleren Patch.                     */
/*  Weitere Kommentare siehe "bilde_graphen_aus_bauchbindenpatches".       */
  
void bilde_graphen_aus_sandwichpatches(KNOTENTYP n) {
  static KNOTENTYP fl1,fl2,fl3;  /* Flaechenzahlen der beteiligten Patches */
  static KNOTENTYP fl;           /* Anzahl Flaechen im erzeugten Graphen */
  static TREENODE *t1[3],*t2[5],*t3[3];
         /* fuer jeden Patch und jede benoetigte Baumtiefe ein Zeiger, so dass
            Rueckspruenge moeglich sind (incl. kritische Flaechenzahl) */
  static TREENODE *t2e; /* Zeiger fuer Patch 2 (je nach Laenge des Bordercodes
                           ist t2e gleich t2[i] fuer ein i) */
  static ELEM *e1,*e2,*e3;       /* Zeiger auf die beteiligten Patches */  
  static KNOTENTYP fl_min13;     /* kleinster Fleischeinlagepatch */
  static KNOTENTYP fl_min2;      /* kleinster mittlerer Patch */
  static KANTE *k1, *k2, *k3;
  static KNOTENTYP code1[2]={0,0}, code3[2]={0,0};  
         /* Bordercodes von 1,3 (zweite Eintraege bleiben konstant) */
  static KNOTENTYP code2[8];      /* fuer Bordercode von 2 (zweimal hinter-
                                     einander) */
  static KNOTENTYP i;      /* Offset in code2, so dass der Code maximal ist */
  static KNOTENTYP j;      /* Zaehlvariable */
  static BOOL gerade1, gerade3;  /* True => 1 bzw. 3 hat gerade Borderlaenge */
                                 /* => code ist (x,0) statt (x) */ 
  static KNOTENTYP dummylen;               /* fuer die Erzeugung der Patches */
  static KNOTENTYP codelen;      /* Codelaenge bei der Suche nach Patches */
  static PLANMAP m;
  static KNOTENTYP len2;   /* Laenge von Nahtteil 2 (Verknuepfung von Patch 1
                              und Patch 3) */
  static KNOTENTYP k;      /* Anzahl der Winkel in Nahtteil 2 (aus
                              dieser errechnet sich der Wert von "len") */
  static char fall;        /* "fall" gibt an, welcher der Faelle 1-4 beim 
    Verknuepfen angewendet werden muss */
  static unsigned char aend;  /* Erklaerung siehe "suche_bordercode" */
  static KNOTENTYP len12, len23;   /* Laengen der Nahtteile 1 und 2 bzw.
                                      2 und 3 zusammen */
  static KNOTENTYP pfadcode[4];           /* fuer die Codierung des Pfades */
  static KANTE *first,*last,*h;        /* Anfang und Ende des Petriepfades */
  static KANTENARRAY2 patchmap1,patchmap2,patchmap3;
  static KNOTENTYP graph[CODESIZE(N_MAX)];  /* Originalcodierung */
  static BOOL beide_richtungen;   /* Muss ein Pfad in beiden Richtungen
                                     bewertet werden? */
  static BOOL sp;   /* sp-Wert, nachdem evtl. die Laufrichtung des
                                Pfades umgedreht wurde */
  static KNOTENTYP anfang,ende;   /* siehe "markiere_petriepfad" */
  static BOOL erlaubt;            /* True => Pfad hat richtige Richtung */
  static BOOL is_num;
  static BOOL unbrauchbar;      /* fuer Strukturtest */

  fl = (n>>1)+2;
  fl_min13 = f_min1;   fl_min2 = 1;
  for (fl3=fl_min13; fl3<=fl-fl_min13-fl_min2; fl3++) {    /* Patch 3 */
    t3[0] = tree[fl3];
    while (t3[0]) {        /* erster Level */
      t3[1] = t3[0]->nextlevel;  gerade3 = False;  /* Zeiger t3 auf code (x) */
      while (t3[1]) {

        /* naechsten Patch 3 aussuchen: */
        while (t3[1] && t3[1+gerade3]->firstpatch==nil) {
          if (gerade3 || t3[1]->nextlevel==nil || t3[1]->nextlevel->code!=0)
             {t3[1] = t3[1]->next;   gerade3 = False;}         /* (x)->(x+1) */
          else {t3[2] = t3[1]->nextlevel;  gerade3 = True;}    /* (x)->(x,0) */
        }   /* while */
        
        if (t3[1]) {                   /* Patch 3 gefunden */
          for (fl1=fl_min13; fl1<=MIN(fl3,fl-fl3-fl_min2); fl1++) { 
            /* Patch 1 aussuchen (wie bei Patch 3) */
            fl2 = fl-fl1-fl3;
            t1[0] = tree[fl1];
            while (t1[0] && t1[0]->code+t3[0]->code<=krit_max) {
              /* Patch 1 und 3 zusammen nicht zu viele kritische Flaechen */
              t1[1] = t1[0]->nextlevel;  gerade1 = False;
              while (t1[1]) {

                /* naechsten Patch 1 aussuchen (siehe Patch 3): */
                while (t1[1] && t1[1+gerade1]->firstpatch==nil) {              
                  if (gerade1 || t1[1]->nextlevel==nil || t1[1]->nextlevel->
                      code) {t1[1] = t1[1]->next;   gerade1 = False;}
                  else {t1[2] = t1[1]->nextlevel;  gerade1 = True;}
                }   /* while */
        
                if (t1[1] && (fl1!=fl3 || t3[1]->code>t1[1]->code ||
                    (t3[1]->code==t1[1]->code && (gerade3 || !gerade1)))) {
                    /* falls die nach "t1[1] &&" folgenden Bedingungen nicht
                       erfuellt sind, hat t1 in der urspruenglichen Ordnung
                       t3 ueberholt */ 
                  /* Patch 1 gefunden - Patch 2 zuordnen */
                  t2[0] = tree[fl2];
                  while (t2[0] && 
                         t1[0]->code+t2[0]->code+t3[0]->code<=krit_max) {
                    if (t1[0]->code+t2[0]->code+t3[0]->code>=krit_min) {
                       /* kritische Flaechen im erforderlichen Intervall */
              

              /* diesen Block eigentlich um 8 Zeichen einruecken: */
              t2[1] = t2[0]->nextlevel;     t2[2] = nil;     k = 0;
              code1[0] = t1[1]->code;   code3[0] = t3[1]->code;
              len12 = (code3[0]<<1)+1+gerade3;
              len23 = (code1[0]<<1)+1+gerade1;
              len2 = MIN(len23,len12);
              len2 -= (1+(len2&1));   /* groesstmoegliche Laenge fuer Naht 2 */
              /* len2 ist immer ungerade => man kann ausrechnen, ob len1 und 
                 len3 (immer) gerade oder ungerade sind: len12 ist ungerade,
                 wenn gerade3==False. len1 ist dann, da len2 ungerade ist, 
                 gerade. Entsprechend fuer len3. */
              fall = 3 + gerade3 - (gerade1<<1);       /* Faelle 1-4 */
              if (t2[1]) {        
                switch (fall) {  
                  /* Faelle 1-4:  Codes vorbereiten */
                  case 1: {           /* len12-len2 = len1 (kleinstmoeglich) */
                    code2[0] = code2[3] = ((len12-len2)-1)>>1;  
                    code2[1] = code2[4] = 0;
                    code2[2] = code2[5] = ((len23-len2)-1)>>1;
                    i = (code2[0] || code2[2]) ? mache_maximal(code2,3) : 2;
                    /* falls Startcode == (0,0,0), dann beginnt der Code bei
                       Eintrag 2, da (k,0,k) nicht mehr maximal waere */  
                    aend = i ? 3 : 5;  codelen = 3;
                    break;
                  }
                  case 2: {
                    code2[0] = code2[2] = ((len12-len2)-1)>>1;
                    code2[1] = code2[3] = ((len23-len2)-1)>>1;
                    i = mache_maximal(code2,2);  aend = 3;  codelen = 2;
                    break;
                  }
                  case 3: {
                    code2[0] = code2[4] = ((len12-len2)-1)>>1;
                    code2[2] = code2[6] = ((len23-len2)-1)>>1;
                    code2[1] = code2[3] = code2[5] = code2[7] = 0;
                    i = mache_maximal(code2,4);  aend = 5;  codelen = 4;
                    break;
                  }
                  case 4: {
                    code2[0] = code2[3] = ((len12-len2)-1)>>1;
                    code2[1] = code2[4] = ((len23-len2)-1)>>1;
                    code2[2] = code2[5] = 0;
                    i = mache_maximal(code2,3); aend = i ? 5 : 3; codelen = 3;
                    break;
                  }  
                }
                /* Patches mit passenden Bordercodes suchen:  (beachte:
                   nach dem Aufruf der Funktion "suche_bordercode" sind nur 
                   noch die Eintraege code2[i] bis code2[i+codelen-1] 
                   korrekt) */
                while (t2e = suche_bordercode(&code2[i],codelen,aend,&t2[1],
                       &k)) {   /* Patch 2 gefunden */ 
                  len2 = MIN(len23,len12);
                  len2 -= (1+(len2&1));    /* groesste Laenge fuer Naht 2 */ 
                  if (len2 > (k<<1)) {     /* k ist klein genug */
                    len2 -= (k<<1);        /* aktuelle Laenge von Nahtteil 2 */
                    e3 = t3[1+gerade3]->firstpatch;
                    while (e3) {
                      e1 = t1[1+gerade1]->firstpatch;
                      while (e1) {
                        e2 = t2e->firstpatch;
                        while (e2) {
                          if (!facerestrict || flaechenzahlen3_ok(e1,e2,e3)) {
                            /* if ((e1->test&2)==0) 
                                  {unmittelbar++;  e1->test |= 2;}
                               if ((e2->test&2)==0)
                                  {unmittelbar++;  e2->test |= 2;}
                               if ((e3->test&2)==0)
                                  {unmittelbar++;  e3->test |= 2;}
                               if ((e1->test&1)==0)
                                  {mittelbar++;  e1->test |= 1;}
                               if ((e2->test&1)==0) 
                                  {mittelbar++;  e2->test |= 1;}
                               if ((e3->test&1)==0)
                                  {mittelbar++;  e3->test |= 1;} */
                            k1 = konstruiere_patch(patchmap1,code1,1+gerade1,
                                                   e1);
                            k2 = konstruiere_patch(patchmap2,&code2[i],codelen,
                                                   e2);
                            k3 = konstruiere_patch(patchmap3,code3,1+gerade3,
                                                   e3);
                            bestimme_gute_basen(e1);
                            bestimme_gute_basen(e2);
                            bestimme_gute_basen(e3);
                            graphenzahl[n>>1][1]++;
                            k2 = verknuepfe_sandwichpatches(k1,k2,k3,
                                 len12-len2,len2,len23-len2,&code2[i],
                                 i ? 1+(fall>2) : 0,fall);
  
                            /* Minimalitaetstest: */
                            beide_richtungen = markiere_petriepfad(k2->next,
                                 fall>2,1,pfadcode,&dummylen,&first,&last,
                                 &anfang,&ende);
                            /* WICHTIG:  k2->next ist die 1. Kante des Pfades
                               (sonst wuerde u.U. ein anderer Pfad markiert) */
                            sp = (first->nr&1) != (fall>2);
                            if (sp) {numeriere_graph_sp(m,first);}
                            else    {numeriere_graph(m,first);}

                            /* Strukturtest: */
                            /* hier muessen im Gegensatz zur Bauchbinde und
                               Brille die nr-Eintraege geloescht werden */
                            unbrauchbar = False;
                            if (barnette) {
                              for (j=0; j<n; j++) { /*nr-Eintraege loeschen*/
                                m[j][0]->nr = m[j][1]->nr = m[j][2]->nr = 0;
                              }
                              if (drei_vierecke(m,n)) {unbrauchbar = True;}
                            }

                            if (!unbrauchbar) {
                              is_num = False;   erlaubt = True;
                              if (beide_richtungen) { 
                                /* Richtung festlegen: nicht erlaubt, wenn
                                   fl1==fl3. Dann muss verglichen werden. */
                                /* Es gilt sp(first)=sp(last). */
                                if (vergleiche_petriepfade(first,last,nil,
                                    sp,sp,True,graph,m,n,False,pfadcode[1]+
                                    pfadcode[2]+pfadcode[3],&is_num)==False) {
                                  if (fl1==fl3) {erlaubt = False;} 
                                              /* falsche Richtung */ 
                                  else {  /* Richtung umdrehen und besseren
                                             Code uebernehmen */
                                    h = first;  first = last;  last = h;
                                    /* pfadcode[2] und pfadcode[3] nicht
                                       vertauschen: sie sind gleich */
                                    if (is_num)    /* Code bereits vorhanden */
                                       {map_2_planarcode2(m,graph,n);}
                                    else {      /* Code noch nicht vorhanden */
                                      if (sp) {numeriere_graph2_sp(m,first);}
                                      else    {numeriere_graph2(m,first);}
                                      map_2_planarcode2(m,graph,n);
                                      is_num = True;
                                    }
                                  }
                                }
                              }
                              if (erlaubt) {   /* richtige Richtung */
                                if (bester_petriepfad(pfadcode,
                                    4,first,sp,m,n,graph,&is_num)) {
                                  non_iso_graphenzahl[n>>1][1]++;
                                  guter_graph(n,m,e1,e2,e3,graph,is_num);
                                } 
                              }
                            }
                          }
                          e2 = e2->next;
                        }   /* while e2 */ 
                        /* if (e1==e3) {e1=nil;} else {*/ e1 = e1->next; /*}*/
                        /* Patches aus gleicher Liste 2mal verknuepfen */
                      }    /* while e1 */
                      e3 = e3->next;
                    }    /* while e3 */
                  }     /* if len2>k*2 */
                }      /* while t2e */
              }       /* if t2[1] */
              /* Ende des einzurueckenden Blocks */

 
                    }        /* if (krit-Intervall) */
                    t2[0] = t2[0]->next;
  	          }         /* while (t2[0]) */
	    
                  /* naechsten Knoten fuer 1 ansteuern (ob der sinnvoll ist, 
                     wird erst beim naechsten Schleifendurchlauf geprueft) */ 
                  if (gerade1 || !(t1[1]->nextlevel) || t1[1]->nextlevel->
                      code) {gerade1 = False;  t1[1] = t1[1]->next;}
                  else {gerade1 = True;  t1[2] = t1[1]->nextlevel;}
                }      /* if t1[1] && ... */
                else {t1[1] = nil;}      /* damit Schleife beendet wird */
              }        /* while t1[1] */
              t1[0] = t1[0]->next;
            }          /* while t1[0] */
          }          /* for (fl1) */

          /* naechsten Knoten fuer 3 ansteuern */
          if (gerade3 || t3[1]->nextlevel==nil || t3[1]->nextlevel->code!=0)
             {gerade3 = False;  t3[1] = t3[1]->next;}
          else {gerade3 = True;  t3[2] = t3[1]->nextlevel;}
        }            /* if t3[1] */
      }              /* while t3[1] */
      t3[0] = t3[0]->next;
    }              /* while t3[0] */
  }                /* for (fl3) */    
}

/*************VERKNUEPFE_BRILLENPATCHES*************************************/
/* Diese Funktion verknuepft die Patches, die durch k1, k2 und k3 gegeben 
   sind, wobei k1 und k2 die Brillenglaeser und k3 den Mittelteil vertritt.
   len1, len2 und len3 sind die Laengen der drei Nahtteile, code3 ist der
   Bordercode von Patch 3, "i" gibt an, wo der Bordercode von Patch 3
   fuer die Verknuepfung beginnt (wie bei der Erzeugung von Patches).   
   "fall" gibt den Fall 1-8 an.                                            */
/* Da die Kante "k3" bei der Verknuepfung verschwinden kann, waere es ein
   Fehler, sie spaeter zum Ansprechen des Graphen zu verwenden. Deshalb gibt
   die Funktion einen Zeiger auf eine Kante zurueck, die wirklich im Graphen
   bleibt, und zwar die Kante "k4". */ 

KANTE *verknuepfe_brillenpatches(KANTE *k1, KANTE *k2, KANTE *k3, KNOTENTYP
     len1, KNOTENTYP len2, KNOTENTYP len3, KNOTENTYP *code3, KNOTENTYP i,
     char fall) {
  static KANTE *k4;            /* Beginn der mittleren Naht */
  static KANTE *returnkante;
  k3 = (returnkante = k4 = suche_naht(code3,i,k3))->next->invers;
  if (fall==1 || fall==2 || fall==5 || fall==6) {k1 = k1->next;}
  else                                          {k1 = k1->invers;}
  /* nun zeigen k1 und k3 parallel zum 3er-Verknuepfungspunkt aus Richtung
     Uhrzeigersinn */
  
  /* Patch 1 vernetzen (Border von Patch 1 wird aufgeloest): */
  while (len1>1) {
    k3->fl_links = k3->invers->fl_rechts = k1->fl_links;
    if (k3->prev->name==aussen) {
      k3->prev = k3->next->next = k1->prev;
      k1->prev->next = k3;
      k1->prev->prev = k3->next;
      k1 = k1->next->invers;     k3 = k3->next->invers;
    }
    else {k1 = k1->prev->invers;   k3 = k3->prev->invers;}
    len1--;
  }  
  
  /* 3er-Treffpunkt von Patch 1 und 3 vernetzen: */
  k3->fl_links = k3->invers->fl_rechts = k1->fl_links;
  k3->prev = k4->next;  k4->next->next = k4->prev = k3;
  k3 = k3->next;      /* wichtig, da altes k3 anschliessend veraendert wird */
  k3->prev->next = k4;  /* naemlich hier */
  
  /* Patch 3 mit sich selbst vernetzen (die Kanten k3 verschwinden): */
  k3 = k3->invers;   k4 = k4->invers;
  while (len3>1) {
    k4->fl_rechts = k4->invers->fl_links = k3->fl_rechts;  
    if (k4->next->name==aussen) {
      k4->next = k4->prev->prev = k3->next;
      k3->next->prev = k4;
      k3->next->next = k4->prev;
      k3 = k3->prev->invers;   k4 = k4->prev->invers;
    }
    else {k3 = k3->next->invers;   k4 = k4->next->invers;}
    len3--;
  }
  
  /* 3er-Treffpunkt von Patch 2 und 3 vernetzen: */
  k4->fl_rechts = k4->invers->fl_links = k3->fl_rechts;
  k3 = k3->next;
  k4->next = k4->prev->prev = k3;
  k3->prev = k4;   k3->next = k4->prev;
  
  /* Patch 2 vernetzen (Border von Patch 2 wird aufgeloest): */
  if (fall==2 || fall==4) {k2 = k2->invers;}
  else                    {k2 = k2->next;}
  k3 = k3->invers; 
  while (len2>1) {
    k3->fl_links = k3->invers->fl_rechts = k2->fl_links;
    if (k3->prev->name==aussen) {
      k3->prev = k3->next->next = k2->prev;
      k2->prev->next = k3;
      k2->prev->prev = k3->next;
      k2 = k2->next->invers;     k3 = k3->next->invers;
    }
    else {k2 = k2->prev->invers;   k3 = k3->prev->invers;}
    len2--;
  }
  k3->fl_links = k3->invers->fl_rechts = k2->fl_links;  
  return(returnkante);
}
   
/*********************GIBT_ES_DOPPELKANTE_IN_GRAPH***************************/
/*  Diese Funktion prueft, ob der Graph g eine Doppelkante enthaelt.
    g liegt im planar_code vor.                                             */

BOOL gibt_es_doppelkante_in_graph(KNOTENTYP *g) {
  static KNOTENTYP n;
  static unsigned long i; 

  n = g[0];
  for (i=0L; i<(unsigned long)n; i++) {
    if (g[(i<<2)+1]==g[(i<<2)+2] || g[(i<<2)+1]==g[(i<<2)+3] ||
	  g[(i<<2)+2]==g[(i<<2)+3]) {return(True);}
  }
  return(False);
}    
   
/***********BILDE_GRAPHEN_AUS_BRILLENPATCHES*********************************/
/*  Bildet Graphen aus drei Brillenpatches. Die resultierenden Graphen
    besitzen "n" Knoten und "fl" Flaechen, wobei "fl" nicht "f_max" sein muss.
    Deshalb kann als obere Grenze fuer die Patches nicht "f_max1" benutzt
    werden, sondern "fl-2*x" mit x=2 (minimale Groesse des mittleren 
    Patches 3 und eines Brillenglaspatches 1 oder 2).                       */
/*  Weitere Kommentare siehe "bilde_graphen_aus_bauchbinden/sandwichpatches */

void bilde_graphen_aus_brillenpatches(KNOTENTYP n) {
  static KNOTENTYP fl1,fl2,fl3;  /* Flaechenzahlen der beteiligten Patches */
  static KNOTENTYP fl;           /* Anzahl Flaechen im erzeugten Graphen */
  static TREENODE *t1[3],*t2[3],*t3[5];
  static TREENODE *t3e;
  static ELEM *e1,*e2,*e3;       /* Zeiger auf die beteiligten Patches */  
  static KNOTENTYP fl_min12;     /* kleinster Brillenglaspatch */
  static KNOTENTYP fl_min3;      /* kleinster mittlerer Patch */
  static KANTE *k1, *k2, *k3;
  static KNOTENTYP code1[2]={0,0}, code2[2]={0,0};  /* Bordercodes von 1,2
         (zweite Eintraege bleiben unveraendert) */
  static KNOTENTYP code3[8];      /* fuer Bordercode von 3 (zweimal hinter-
                                     einander) */
  static KNOTENTYP i;       /* Offset in code3, so dass der Code maximal ist */
  static BOOL gerade1, gerade2;  /* True => 1 bzw. 2 hat gerade Borderlaenge */
                                 /* => code ist (x,0) statt (x) */ 
  static KNOTENTYP dummylen;               /* fuer die Erzeugung der Patches */
  static KNOTENTYP codelen;      /* Codelaenge bei der Suche nach Patches */
  static KNOTENTYP graph[CODESIZE(N_MAX)];   /* Originalnumerierung */
  static PLANMAP m;
  static KNOTENTYP len;    /* Laenge der Naht, an der Patch 3 mit sich selbst
                              verknuepft ist */
  static KNOTENTYP k;      /* Anzahl der Winkel in der mittleren Naht (aus
                              dieser errechnet sich der Wert von "len") */
  static char fall;    /* "fall" gibt an, welcher der Faelle 1-8 beim 
    Verknuepfen angewendet werden muss */
  static unsigned char aend;        /* Erklaerung siehe "suche_bordercode" */
  static KNOTENTYP pfadcode[4];           /* fuer die Codierung des Pfades */
  static KANTE *first,*last, *h;       /* Anfang und Ende des Petriepfades */
  static KANTENARRAY2 patchmap1,patchmap2,patchmap3;
  static BOOL beide_richtungen;
  static BOOL erlaubt;
  static BOOL is_num;

  fl = (n>>1)+2;
  fl_min12 = f_min1;  fl_min3 = 2;
  for (fl2=fl_min12; fl2<=fl-fl_min12-fl_min3; fl2++) {    /* Patch 2 */
    t2[0] = tree[fl2];
    while (t2[0]) {        /* erster Level */
      t2[1] = t2[0]->nextlevel;  gerade2 = False;  /* Zeiger t2 auf code (x) */
      while (t2[1]) {

        /* naechsten Patch 2 aussuchen: */
        while (t2[1] && t2[1+gerade2]->firstpatch==nil) {
          if (gerade2 || t2[1]->nextlevel==nil || t2[1]->nextlevel->code!=0)
             {t2[1] = t2[1]->next;   gerade2 = False;}         /* (x)->(x+1) */
          else {t2[2] = t2[1]->nextlevel;  gerade2 = True;}    /* (x)->(x,0) */
        }   /* while */
        
        if (t2[1]) {                   /* Patch 2 gefunden */
          for (fl1=fl_min12; fl1<=MIN(fl2,fl-fl2-fl_min3); fl1++) { 
            /* Patch 1 aussuchen (wie bei Patch 2) */
            fl3 = fl-fl1-fl2;
            t1[0] = tree[fl1];
            while (t1[0] && t1[0]->code+t2[0]->code<=krit_max) {
              /* Patch 1 und 2 zusammen nicht zu viele kritische Flaechen */
              t1[1] = t1[0]->nextlevel;  gerade1 = False;
              while (t1[1]) {

                /* naechsten Patch 1 aussuchen (siehe Patch 2): */
                while (t1[1] && t1[1+gerade1]->firstpatch==nil) {              
                  if (gerade1 || t1[1]->nextlevel==nil || t1[1]->nextlevel->
                      code) {t1[1] = t1[1]->next;   gerade1 = False;}
                  else {t1[2] = t1[1]->nextlevel;  gerade1 = True;}
                }   /* while */
        
                if (t1[1] && (fl1!=fl2 || t2[1]->code>t1[1]->code ||
                    (t2[1]->code==t1[1]->code && (gerade2 || !gerade1)))) {
                    /* falls die nach "t1[1] &&" folgenden Bedingungen nicht
                       erfuellt sind, hat t1 in der urspruenglichen Ordnung
                       t2 ueberholt */ 
                  /* Patch 1 gefunden - Patch 3 zuordnen */
                  t3[0] = tree[fl3];
                  while (t3[0] && 
                         t1[0]->code+t2[0]->code+t3[0]->code<=krit_max) {
                    if (t1[0]->code+t2[0]->code+t3[0]->code>=krit_min) {
                       /* kritische Flaechen im erforderlichen Intervall */


              /* diesen Block eigentlich um 8 Zeichen einruecken */
              t3[1] = t3[0]->nextlevel;     t3[2] = nil;     k = 0;
              fall = 4 - ((gerade1<<1) + gerade2);   /* zunaechst Faelle 1-4 */
              code1[0] = t1[1]->code;   code2[0] = t2[1]->code;
              while (t3[1]) {        
                switch (fall) {  
                  /* Faelle 1-8:  Codes vorbereiten */
                  case 1: {
                    code3[0] = code3[2] = code2[0] + 1;  
                    code3[1] = code3[3] = code1[0] + 1;
                    i = mache_maximal(code3,2);   aend = 3;   codelen = 2;   
                    break;
                  }
                  case 2: {
                    code3[0] = code3[3] = code2[0];
                    code3[1] = code3[4] = 0;
                    code3[2] = code3[5] = code1[0] + 1;
                    i = mache_maximal(code3,3);  aend = i ? 3 : 5;  codelen=3;
                    break;
                  }
                  case 3: {
                    code3[0] = code3[3] = code2[0] + 1;
                    code3[1] = code3[4] = code1[0];
                    code3[2] = code3[5] = 0;
                    i = mache_maximal(code3,3);  aend = i ? 5 : 3;  codelen=3;
                    break;
                  }
                  case 4: {
                    code3[0] = code3[4] = code2[0];
                    code3[2] = code3[6] = code1[0];
                    code3[1] = code3[3] = code3[5] = code3[7] = 0;
                    i = mache_maximal(code3,4);  aend = 5;  codelen = 4;
                    break;
                  }  
                  case 5: {
                    code3[0] = code3[2] = 0;
                    code3[1] = code3[3] = code1[0] + code2[0] + 3; 
                    i = 1;   aend = 3;   codelen = 2;
                    break;
                  }
                  case 6: {
                    code3[0] = code3[3] = code3[1] = code3[4] = 0;
                    code3[2] = code3[5] = code1[0] + code2[0] + 2;
                    i = 2;   aend = 3;   codelen = 3;
                    break;
                  }
                  case 7: {
                    code3[0] = code3[3] = code3[2] = code3[5] = 0;
                    code3[1] = code3[4] = code1[0] + code2[0] + 2;
                    i = 1;   aend = 5;   codelen = 3;
                    break;
                  }
                  case 8: {
                    code3[0] = code3[4] = code3[1] = code3[5] = code3[3] =
                    code3[7] = 0;
                    code3[2] = code3[6] = code1[0] + code2[0] + 1;
                    i = 2;   aend = 5;   codelen = 4;
                    break;
                  }                    
                }
                /* Patches mit passenden Bordercodes suchen:  (beachte:
                   nach dem Aufruf der Funktion "suche_bordercode" sind nur 
                   noch die Eintraege code3[i] bis code3[i+codelen-1] 
                   korrekt) */
                while (t3e = suche_bordercode(&code3[i],codelen,aend,&t3[1],
                       &k)) {
                  /* Patch 3 gefunden */ 
                  len = (k<<1)+(fall<5);             /* mittlere Naht */
                  e2 = t2[1+gerade2]->firstpatch;
                  while (e2) {   
                    e1 = t1[1+gerade1]->firstpatch;
                    while (e1) {
                      e3 = t3e->firstpatch;
                      while (e3) {
                        if (!facerestrict || flaechenzahlen3_ok(e1,e2,e3)) {
                          /* if ((e1->test&2)==0) 
                                {unmittelbar++;  e1->test |= 2;} 
                             if ((e1->test&1)==0) 
                                {mittelbar++;    e1->test |= 1;}
                             if ((e2->test&2)==0)
                                {unmittelbar++;  e2->test |= 2;}
                             if ((e2->test&1)==0)
                                {mittelbar++;    e2->test |= 1;}
                             if ((e3->test&2)==0)
                                {unmittelbar++;  e3->test |= 2;}
                             if ((e3->test&1)==0)
                                {mittelbar++;    e3->test |= 1;} */
                          k1 = konstruiere_patch(patchmap1,code1,1+gerade1,e1);
                          k2 = konstruiere_patch(patchmap2,code2,1+gerade2,e2);
                          k3 = konstruiere_patch(patchmap3,&code3[i],codelen,
                                                 e3);
                          bestimme_gute_basen(e1);
                          bestimme_gute_basen(e2);
                          bestimme_gute_basen(e3);
                          k3 = verknuepfe_brillenpatches(k1,k2,k3,(code1[0]<<1)
                               +1+gerade1,(code2[0]<<1)+1+gerade2,len,
                               &code3[i],i ? 2-gerade1 : 0,fall);
                          graphenzahl[n>>1][2]++;
                          numeriere_graph(m,k3);
                          map_2_planarcode(m,graph,n);
                          if (!(gibt_es_doppelkante_in_graph(graph) ||
                               (barnette && drei_vierecke(m,n)))) {
                                /* fuer "drei_vierecke": alle nr-Eintraege sind
                                   zu Beginn bereits auf 0 */

                            /* Minimalitaetstest: */
                            beide_richtungen = markiere_petriepfad(k3,False,1,
                              pfadcode,&dummylen,&first,&last,nil,nil); 
                              /* WICHTIG: k3 fuehrt zum Originalpfad, obwohl es
                                 nicht die Anfangskante des Pfades ist */
                            loesche_numerierung(m,n);  /* die alte Numerie-
                              rung ging nicht von der Kante "first" aus */
                            if (first->nr&1) {numeriere_graph_sp(m,first);}
                            else             {numeriere_graph(m,first);}
                            is_num = False;   erlaubt = True;
                            if (beide_richtungen) { 
                              /* Richtung festlegen: nicht erlaubt, wenn
                                 fl1==fl2. Dann muss verglichen werden. */
                              if (vergleiche_petriepfade(first,last,nil,
                                  first->nr&1,last->nr&1,True,graph,m,n,
                                  False,pfadcode[1]+pfadcode[2]+pfadcode[3],
                                  &is_num)==False) {
                                if (fl1==fl2) {erlaubt = False;} 
                                            /* falsche Richtung */ 
                                else {  /* Richtung umdrehen und besseren
                                           Code uebernehmen */
                                  h = first;  first = last;  last = h;
                                  /* pfadcode[2] und pfadcode[3] nicht
                                     vertauschen: sie sind gleich */
                                  if (is_num)    /* Code bereits vorhanden */
                                     {map_2_planarcode2(m,graph,n);}
                                  else {      /* Code noch nicht vorhanden */
                                    if (first->nr&1) 
                                         {numeriere_graph2_sp(m,first);}
                                    else {numeriere_graph2(m,first);}
                                    map_2_planarcode2(m,graph,n);
                                    is_num = True;
                                  }
                                }
                              }
                            }
                            if (erlaubt) {   /* richtige Richtung */
                              if (bester_petriepfad(pfadcode,
                                  4,first,first->nr&1,m,n,graph,&is_num)) {
                                non_iso_graphenzahl[n>>1][2]++;
                                guter_graph(n,m,e1,e2,e3,graph,is_num);
                              } 
                            }
			  }
                        }
                        e3 = e3->next;
                      }   /* while e3 */ 
                      /*if (e1==e2) {e1 = nil;} else {*/ e1 = e1->next; /*}*/
                    }
                    e2 = e2->next;
                  }    /* while e2 */
                }      /* while t3e */

                if (fall<5)      /* Faelle 5-8 starten (k ist mindestens 1) */
                  {fall += 4;  t3[1] = t3[0]->nextlevel;  t3[2] = nil;  k = 1;}
                else {t3[1] = nil;}    /* Schleife beenden */
              }      /* while t3[1] */
              /* Ende des einzurueckenden Blocks */

 
                    }       /* if (krit-Intervall) */
                    t3[0] = t3[0]->next;
  	          }         /* while (t3[0]) */
	    
                  /* naechsten Knoten fuer 1 ansteuern (ob der sinnvoll ist, 
                     wird erst beim naechsten Schleifendurchlauf geprueft) */ 
                  if (gerade1 || !(t1[1]->nextlevel) || t1[1]->nextlevel->
                      code) {gerade1 = False;  t1[1] = t1[1]->next;}
                  else {gerade1 = True;  t1[2] = t1[1]->nextlevel;}
                }      /* if t1[1] && ... */
                else {t1[1] = nil;}      /* damit Schleife beendet wird */
              }        /* while t1[1] */
              t1[0] = t1[0]->next;
            }          /* while t1[0] */
          }          /* for (fl1) */

          /* naechsten Knoten fuer 2 ansteuern */
          if (gerade2 || t2[1]->nextlevel==nil || t2[1]->nextlevel->code!=0)
             {gerade2 = False;  t2[1] = t2[1]->next;}
          else {gerade2 = True;  t2[2] = t2[1]->nextlevel;}
        }            /* if t2[1] */
      }              /* while t2[1] */
      t2[0] = t2[0]->next;
    }              /* while t2[0] */
  }                /* for (fl2) */    
}
  

/**********************************************************************/
/* Funktionen zum Testen eines generierten Patches auf Brauchbarkeit: */
/**********************************************************************/

/*********************ERMITTLE_VAL_2_KNOTEN*********************************/
/*  Ermittelt die Anzahl von Valenz-2-Knoten in einem Bruchkantenpatch.    */

KNOTENTYP ermittle_val_2_knoten(KNOTENTYP *bordercode,KNOTENTYP len) {
  static KNOTENTYP erg,i;
  erg = 0; 
  for (i=0; i<len; i++) {erg += bordercode[i];}
  return(erg+len);
}     
  
/*********************ERMITTLE_BORDERLAENGE*********************************/
/*  Ermittelt Laenge einer Patchumrandung eines Bruchkantenpatches.        */

KNOTENTYP ermittle_borderlaenge(KNOTENTYP *bordercode,KNOTENTYP len) {
  return((ermittle_val_2_knoten(bordercode,len)<<1)-len);
}     

/***************************IST_NULL_IM_CODE********************************/
/*  Die Funktion ueberprueft, ob der Bordercode eine 0 enthaelt.           */

BOOL ist_null_im_code(KNOTENTYP *bordercode,KNOTENTYP len) {
  while (len-- > 0) {if (bordercode[len]==0) {return(True);}}
  return(False);
}

/*****************SUCHE_BORDERCODE2*****************************************/
/*  Diese Funktion sucht nach einem konkreten Bordercode "code" im Baum
    fuer "fl" Flaechen und "krit1" bis "krit2" kritischen Flaechen. Sie
    uebergibt "True",  wenn sie einen Patch mit diesem Bordercode gefunden 
    hat. */

BOOL suche_bordercode2(KNOTENTYP *code,KNOTENTYP len,KNOTENTYP fl,
     KNOTENTYP krit1,KNOTENTYP krit2) {
  static KNOTENTYP pos;     /* pos = Tiefe im Patchbaum */
  static TREENODE *t,*t2;

  t = tree[fl];
  while (t && t->code<=krit2) {       /* erster Level */
    if (t && t->code>=krit1) {
      t2 = t->nextlevel;
      pos = 0;
      while (t2) {           /* Baum nicht komplett durchsucht */
        while (t2 && t2->code < code[pos])  {t2 = t2->next;}
        if (t2!=nil && t2->code==code[pos]) {
          pos++;
          if (pos==len) {
            if (t2->firstpatch!=nil) {return(True);}   /* gefunden */
            else {t2 = nil;}     /* naechster Schleifendurchlauf */
          }
          else {t2 = t2->nextlevel;}
        }
        else {t2 = nil;}         /* naechster Schleifendurchlauf */
      }
    }
    t = t->next;
  }
  return(False);
}  

/***********************GIBT_ES_GEGENSTUECK**********************************/
/*  Diese Funktion prueft, ob es ein Gegenstueck zu einem Bauchbindenpatch
    mit fl Flaechen und Bordercode c und krit kritischen Flaechen gibt.     */
/*  alternative  =>  krit_max==0==krit==t->code==krit_min, so dass
    auch dann die richtige Verzweigung angesteuert wird.                    */

BOOL gibt_es_gegenstueck(KNOTENTYP fl, KNOTENTYP c, KNOTENTYP krit) {
  static KNOTENTYP f;
  static BBTREENODE *t,*t2;
  for (f=(f_anf>fl) ? f_anf-fl : 1; f<=f_max_bb_best-fl; f++) {
    /* erster Level: */
    if (is_moeglich[((f+fl)<<1)-4] && (bauchbindenkennung<3 || (f+fl)%2==0)) {
      /* sonst Gegenstueck uninteressant */
      t = bbtree[f];
      while (t && t->code+krit<=krit_max) {
        if (t && t->code+krit>=krit_min) {
          /* zweiter Level: */
          t2 = t->nextlevel;
          while (t2 && t2->code < c)  {t2 = t2->next;}
          if (t2 && t2->code==c)  {return(True);}
        }
        t = t->next;
      }
    }
  }
  return(False);   /* endgueltig nichts gefunden */
}
  
/******************SUCHE_GEGENSTUECKE***************************************/
/*  Sucht Gegenstuecke zu einem brauchbaren Bruchkantenpatch "code3" mit "fl"
    Flaechen, davon "krit3" kritische (=0, falls keine Aufspaltung).       */
/*  Falls es welche gibt, gibt die Funktion "True" zurueck.                */
/*  Prinzip: Ein Patch wird gesucht. Sobald er gefunden wird, wird der
    Code des dritten Patches ausgerechnet und gesucht. Es gilt: Unabhaengig vom
    uebergebenen Patch wird mindestens noch ein weiterer Einschlusspatch
    benoetigt. Deshalb werden zunaechst alle Einschlusspatches gesucht,
    auch wenn die gefundene Borderlaenge eventuell nicht mit der benoetigten
    Borderlaenge uebereinstimmt.                                           */
/*  Der gefundene Patch braucht nicht auf zwei verschiedene Weisen 
    zugeordnet zu werden (z.B. Brillenglas 1 und 2). Denn wenn es bei der
    zweiten Zuordnung ein passendes Gegenstueck gibt, so wuerde das
    Gegenstueck ja auch gefunden und der gefundene Patch als Gegenstueck
    zugeordnet.                                                            */ 

BOOL suche_gegenstuecke(KNOTENTYP fl,KNOTENTYP *code3,KNOTENTYP len3,KNOTENTYP
     krit3) {
  static TREENODE *t[5];                   /* fuer gesuchten Mittelteil */
  static TREENODE *t1[3];                  /* fuer Patch 1 (Einschlusspatch) */
  static KNOTENTYP code1, code2[8];     /* fuer die gesuchten Patches
                                           (Patch 2 zweimal hintereinander) */
  static codelen2;
  static KNOTENTYP i;           /* Offset in code2, so dass Code maximal ist */
  static KNOTENTYP k;           /* fuer Sandwichmittelteile */
  static BOOL gerade1;
  static char aend2;
  static KNOTENTYP len12,len2,len23;  
         /* Sandwichpfadteile (s. "bilde_graphen_aus_sandwichpatches") */
  static KNOTENTYP f;          /* Anzahl Flaechen, die aufzuteilen sind */
  static KNOTENTYP fl1,fl2;    /* f = fl1+fl2 */
  static KNOTENTYP dummy;
  static KNOTENTYP krit2min,krit2max;  /* Mindest- und Hoechstwert an
                                          kritischen Flaechen fuer Patch 2 */

  for (f=(f_anf>fl+1) ? f_anf-fl : 2; f<=f_max-fl; f++) {
    for (fl1=1; fl1<f; fl1++) {      /* Groesse der ersten Flaeche */
      fl2 = f-fl1;     /* Wichtig: Paare (x,y) UND (y,x) werden gebildet,
                          denn das heisst, dass ein gefundener Patch nicht
                          auf zwei verschiedene Weisen zugeordnet werden muss, 
                          sondern es reicht eine Zuordnung. */ 

      t1[0] = tree[fl1];  
      while (t1[0] && t1[0]->code+krit3<=krit_max) {
        krit2max = krit_max - t1[0]->code - krit3;
        krit2min = t1[0]->code+krit3 < krit_min ? 
                   krit_min - t1[0]->code - krit3 : 0;  
        t1[1] = t1[0]->nextlevel;
        gerade1 = False;             /* Zeiger t1 auf code (x) */
        while (t1[1]) {

          /* naechsten Patch 1 aussuchen: */
          while (t1[1] && t1[1+gerade1]->firstpatch==nil) {              
            if (gerade1 || t1[1]->nextlevel==nil || t1[1]->nextlevel->code!=0)
               {t1[1] = t1[1]->next;   gerade1 = False;}      /* (x)->(x+1) */
            else {t1[2] = t1[1]->nextlevel;  gerade1 = True;} /* (x)->(x,0) */
          }   /* while */

          if (t1[1]) {             /* Patch 1 gefunden */
            /* Patch 2 ausrechnen und aufsuchen */
            code1 = t1[1]->code;
            
            switch (len3) {

  	      case 4: {    /* Mittelteil eines Sandwiches oder einer Brille */
                if (!gerade1) {    /* sonst unbrauchbar */
                  /* Mittelteil einer ungeraden Brille */
                  if (code1 <= code3[2]) {     /* OBdA kleines Glas */
                    code2[0] = code3[0] - (code3[2] - code1);
                    if (suche_bordercode2(code2,1,fl2,krit2min,krit2max)) 
                       {return(True);}
                  }
                  /* Mittelteil einer geraden Brille */
                  if (code3[0] > code3[2] + 1 + code1) {
                    code2[0] = code3[0] - (code3[2] + 1 + code1);
                    if (suche_bordercode2(code2,1,fl2,krit2min,krit2max))  
                       {return(True);}
                  }
                  /* Mittelteil eines Sandwiches */
                  if (code1 > code3[0]) {        /* OBdA grosse Einlage */
                    code2[0] = code3[2] + (code1 - code3[0]);
                    if (suche_bordercode2(code2,1,fl2,krit2min,krit2max))
                       {return(True);}
                  }
                }
                break;
              }
             
              case 3: {   /* Mittelteil eines Sandwiches oder einer Brille */
                if (gerade1) {            /* sonst OBdA unbrauchbar */
                  /* OBdA wird immer der ungerade Patch dem geraden 
                     zugeordnet */
 
                  /* Mittelteil einer ungeraden Brille */
                  if (code3[1]==0 && code3[2] > code1) {
                    code2[0] = code3[0] - (code3[2] - code1 - 1);
                    if (suche_bordercode2(code2,1,fl2,krit2min,krit2max))
                       {return(True);}
                  }
                  else if (code3[2]==0 && code3[0]>code1 &&
                           code3[1] > (code3[0] - code1 - 1)) {
                    code2[0] = code3[1] - (code3[0] - code1 - 1);
                    if (suche_bordercode2(code2,1,fl2,krit2min,krit2max))
                       {return(True);}
                  }
                  /* Mittelteil einer geraden Brille */
                  if (code3[1]==0 && code3[0] > code1 + 2 + code3[2]) {
                    code2[0] = code3[0] - (code1 + 2 + code3[2]);
                    if (suche_bordercode2(code2,1,fl2,krit2min,krit2max))
                       {return(True);}
                  }
                  else if (code3[2]==0 && code3[0] > code1 + 2 + code3[1]) {
                    code2[0] = code3[0] - (code1 + 2 + code3[1]);
                    if (suche_bordercode2(code2,1,fl2,krit2min,krit2max))
                       {return(True);}
                  }
                  /* Mittelteil eines Sandwiches */
                  if (code3[1]==0 && code1 >= code3[2]) {
                    code2[0] = code3[0] + 1 + (code1 - code3[2]);
                    if (suche_bordercode2(code2,1,fl2,krit2min,krit2max))
                       {return(True);} 
                  }
                  else if (code3[2]==0 && code1 >= code3[0]) {
                    code2[0] = code3[1] + 1 + (code1 - code3[0]);
                    if (suche_bordercode2(code2,1,fl2,krit2min,krit2max))
                       {return(True);}
                  }
                }
                break;
              }

              case 2: {     /* Mittelteil oder Einschlusspatch */
                if (gerade1) {     /* sonst fuer Mittelteile unbrauchbar */
                  /* Mittelteil einer ungeraden Brille */                 
                  if (code1 < code3[1]) {      /* OBdA kleineres Glas */
                    code2[0] = code3[0] - (code3[1] - code1);
                    code2[1] = 0;
                    if (suche_bordercode2(code2,2,fl2,krit2min,krit2max))
                       {return(True);}
                  }
                  /* Mittelteil einer geraden Brille */
                  if (code3[0] > code3[1] + 3 + code1) {
                    code2[0] = code3[0] - (code3[1] + 3 + code1);
                    code2[1] = 0;
                    if (suche_bordercode2(code2,2,fl2,krit2min,krit2max))
                       {return(True);}
                  }
                  /* Mittelteil eines Sandwiches */
                  if (code1 >= code3[0]) {     /* OBdA groessere Einlage */
                    code2[0] = code3[1] + (code1 - code3[0]);
                    code2[1] = 0;
                    if (suche_bordercode2(code2,2,fl2,krit2min,krit2max))
                       {return(True);}
                  }
                }

                if (code3[1]==0) {    /* 3 ist gerader Einschlusspatch */
                  /* Einschlusspatch fuer ungerade Brille */
                  t[0] = tree[fl2];
                  while (t[0] && t[0]->code<=krit2max) {
                    if (t[0]->code>=krit2min) { 
                      if (gerade1) {
                        code2[0] = code2[2] = code3[0]+1;
                        code2[1] = code2[3] = code1 + 1; 
                        i = mache_maximal(code2,2);  codelen2 = 2;  aend2 = 3; 
                      }
                      else {
                        code2[0] = code2[3] = code1;
                        code2[1] = code2[4] = 0;
                        code2[2] = code2[5] = code3[0]+1;
                        i = mache_maximal(code2,3);  codelen2=3; 
                        aend2 = i ? 3 : 5;
                      }
                      t[1] = t[0]->nextlevel;  t[2] = nil;
                      if (suche_bordercode(&code2[i],codelen2,aend2,&t[1],
                          &dummy)) {return(True);}

                      /* Einschlusspatch fuer gerade Brille */
                      if (gerade1) {
                        code2[0] = code3[0]+code1+3;
                        code2[1] = 0; 
                        t[1] = t[0]->nextlevel;  t[2] = nil;
                        if (suche_bordercode(&code2[0],2,3,&t[1],&dummy)) 
                           {return(True);}
                      }
                      else {          /* zwei Moeglichkeiten */
                        code2[0] = code1 + code3[0] + 2;
                        code2[1] = code2[2] = 0; 
                        t[1] = t[0]->nextlevel;  t[2] = nil;
                        if (suche_bordercode(&code2[0],3,3,&t[1],&dummy))
                           {return(True);}
                        code2[0] = code1 + code3[0] + 2;
                        code2[1] = code2[2] = 0; 
                        t[1] = t[0]->nextlevel;  t[2] = nil;
                        if (suche_bordercode(&code2[0],3,5,&t[1],&dummy))
                           {return(True);}
                      }
 
                      /* Einschlusspatch fuer Sandwich 
                         (s. "bilde_graphen...") */
                      if (gerade1) {                  /* Fall 2 */
                        len12 = (code3[0]<<1)+2;
                        len23 = (code1<<1)+2;
                        len2 = MIN(len23,len12);
                        len2 -= (1+(len2&1));        /* maximale Laenge l2 */
                        code2[0] = code2[2] = ((len12-len2)-1)>>1;
                        code2[1] = code2[3] = ((len23-len2)-1)>>1;
                        i = mache_maximal(code2,2);  aend2 = 3;  codelen2 = 2;
                      }                
                      else {                          /* Fall 4 */
                        len12 = (code3[0]<<1)+2;
                        len23 = (code1<<1)+1;
                        len2 = MIN(len23,len12);
                        len2 -= (1+(len2&1));
                        code2[0] = code2[3] = ((len12-len2)-1)>>1;
                        code2[1] = code2[4] = ((len23-len2)-1)>>1;
                        code2[2] = code2[5] = 0;
                        i = mache_maximal(code2,3);  aend2 = i ? 5 : 3; 
                        codelen2=3;
                      }  
                      t[1] = t[0]->nextlevel;     t[2] = nil;     k = 0;
                      if (suche_bordercode(&code2[i],codelen2,aend2,&t[1],
                          &k)) {    /* Patch 2 gefunden */ 
                        len2 = MIN(len23,len12);
                        len2 -= (1+(len2&1)); /* groesste Laenge fuer Naht 2 */
                        if (len2 > (k<<1)) {return(True);}  /* k klein genug */
                      }
                    }
                    t[0] = t[0]->next;
                  }
                }     /* if code3[1]==0 */                       
                break;
              }

              case 1: {     /* 3 ist ungerader Einschlusspatch */
                /* Einschlusspatch fuer ungerade Brille */
                t[0] = tree[fl2];
                while (t[0] && t[0]->code<=krit2max) {
                  if (t[0]->code>=krit2min) { 
                    if (gerade1) {
                      code2[0] = code2[3] = code1 + 1;
                      code2[1] = code2[4] = code3[0];
                      code2[2] = code2[5] = 0;
                      i = mache_maximal(code2,3);  codelen2 = 3;  
                      aend2 = i ? 5 : 3;
                    }
                    else {
                      code2[0] = code2[4] = code1;
                      code2[2] = code2[6] = code3[0];
                      code2[1] = code2[3] = code2[5] = code2[7] = 0;
                      i = mache_maximal(code2,4);  codelen2 = 4;  aend2 = 5;
                    }
                    t[1] = t[0]->nextlevel;  t[2] = nil;
                    if (suche_bordercode(&code2[i],codelen2,aend2,&t[1],
                        &dummy)) {return(True);}

                    /* Einschlusspatch fuer gerade Brille */
                    if (gerade1) {    /* zwei Moeglichkeiten */
                      code2[0] = code1 + code3[0] + 2;
                      code2[1] = code2[2] = 0;
                      t[1] = t[0]->nextlevel;  t[2] = nil;
                      if (suche_bordercode(&code2[0],3,3,&t[1],&dummy)) 
                         {return(True);}
                      code2[0] = code1 + code3[0] + 2;
                      code2[1] = code2[2] = 0;
                      t[1] = t[0]->nextlevel;  t[2] = nil;
                      if (suche_bordercode(&code2[0],3,5,&t[1],&dummy))
                         {return(True);}
                    }
                    else {
                      code2[0] = code1 + code3[0] + 1;
                      code2[1] = code2[2] = code2[3] = 0;
                      t[1] = t[0]->nextlevel;  t[2] = nil;
                      if (suche_bordercode(&code2[0],4,5,&t[1],&dummy))
                         {return(True);}
                    }

                    /* Einschlusspatch fuer Sandwich
                       (siehe "bilde_graphen...") */
                    if (gerade1) {          /* Fall 1 */
                      len12 = (code3[0]<<1)+1;
                      len23 = (code1<<1)+2;
                      len2 = MIN(len23,len12);
                      len2 -= (1+(len2&1));        /* maximale Laenge l2 */
                      code2[0] = code2[3] = ((len12-len2)-1)>>1;  
                      code2[1] = code2[4] = 0;
                      code2[2] = code2[5] = ((len23-len2)-1)>>1;
                      i = (code2[0] || code2[2]) ? mache_maximal(code2,3) : 2;
                      /* falls Startcode == (0,0,0), dann beginnt der Code bei
                         Eintrag 2, da (k,0,k) nicht mehr maximal waere */  
                      aend2 = i ? 3 : 5;  codelen2 = 3;
                    }                
                    else {                       /* Fall 3 */                
                      len12 = (code3[0]<<1)+1;
                      len23 = (code1<<1)+1;
                      len2 = MIN(len23,len12);
                      len2 -= (1+(len2&1));        /* maximale Laenge l2 */
                      code2[0] = code2[4] = ((len12-len2)-1)>>1;
                      code2[2] = code2[6] = ((len23-len2)-1)>>1;
                      code2[1] = code2[3] = code2[5] = code2[7] = 0;
                      i = mache_maximal(code2,4);  aend2 = 5;  codelen2 = 4;
                    }
                    t[1] = t[0]->nextlevel;  t[2] = nil;   k = 0;
                    if (suche_bordercode(&code2[i],codelen2,aend2,&t[1],&k)) {
                      /* Patch 2 gefunden */ 
                      len2 = MIN(len23,len12);
                      len2 -= (1+(len2&1));  /* groesste Laenge fuer Naht 2 */ 
                      if (len2 > (k<<1)) {return(True);}   /* k klein genug */
                    }
                  }
                  t[0] = t[0]->next;
                }    /* while t[0] */
                break;
              }
	    }       /* switch */
            if (gerade1 || !(t1[1]->nextlevel) || t1[1]->nextlevel->code!=0)
               {gerade1 = False;  t1[1] = t1[1]->next;}
            else {gerade1 = True;  t1[2] = t1[1]->nextlevel;}
          }         /* if t1[1] */
        }           /* while t1[1] */
        t1[0] = t1[0]->next;
      }
    }             /* for fl1 */
  }               /* for f */
  return(False);
}
  

/*******************VERSAGT_PGA_PATCHKRITERIUM*******************************/
/*  Diese Funktion stellt zu einem Patch bestimmte Akzeptanzkriterien bereit
    und uebergibt "True", wenn eines von ihnen versagt.
    Falls der Patch ein BB-Patch ist und durch einen geraden Durchschnitt
    erhalten wurde, so zeigen "vg1" und "vg2" auf die Codes der direkten
    Vorgaengerpatches. "vg1" hat die Laenge 4 und "vg2" die Laenge 2.
    Falls der Patch ein BB-Patch ist und durch einen ungeraden Durchschnitt
    erhalten wurde, so wird die vorliegende Funktion gar nicht erst aufgerufen.
*/

BOOL versagt_pga_patchkriterium(KNOTENTYP *bordercode,KNOTENTYP len,BOOL bb,
     KNOTENTYP fl,KNOTENTYP *vg1,KNOTENTYP *vg2) {
  static KNOTENTYP i;
  static BOOL brauchbar;

  if (bb) {   
    /* Knotenkriterium fuer BB-Patches: */
    if (n_max_bb < bordercode[0]+(fl<<1)-2) {return(True);}
    brauchbar = False;
    /* Flaechenkriterium + passende Umrandung fuer BB-Patches: */
    for (i=n_anf; i<=n_end; i+=2) {  
      /* "n_anf" und "n_end" sind globale Variablen */
      if (is_moeglich[i] && i*(i-1)/6+1==fl) {
        /* Passende Umrandung, passende Vorgaenger ueberpruefen */
        if (bordercode[0]==i-1 && vg1[0]==(i>>1)-2 && vg1[2]==(i>>1)-2 &&
            vg2[0]==(i>>1)-1 && vg2[1]==(i>>1)-1) {brauchbar = True;}
        else {i=n_end;}
      }
    }
    if (!brauchbar) {return(True);}   
       /* BB-Patch und/oder Rand hat nicht gewuenschte Groesse */
  }
  else {   /* Knotenkriterium fuer BK-Patches: */
    if (n_max < (fl<<1) - 2 + ermittle_val_2_knoten(bordercode,len))
       {return(True);}
  }
  return(False);     /* kein Kriterium versagt */
}


/*******************VERSAGT_PATCHKRITERIUM*********************************/
/*  Diese Funktion stellt zu einem Patch bestimmte Akzeptanzkriterien bereit
    und uebergibt "True", wenn eines von ihnen versagt. */

BOOL versagt_patchkriterium(KNOTENTYP *bordercode,KNOTENTYP len,BOOL bb,
     KNOTENTYP fl) {
  static KNOTENTYP val_2_knoten;
  static KNOTENTYP i;
  static EULERTYP j;
  static BOOL gefunden;

  val_2_knoten = bb ? bordercode[0] : 0;       /* 0 => noch nicht berechnet */

  /* Kantenkriterium fuer Patches, die nur noch mit einem Flaechentyp ergaenzt
     werden koennen (1): */
  if (pv>3 && len+6 == (f_max-fl)*(6-small_face) && /* nur noch 1 Flaechentyp */
     (((val_2_knoten ? val_2_knoten : (val_2_knoten = 
      ermittle_val_2_knoten(bordercode,len)))-1)>>1)+2+fl > f_max)
     {return(True);}    

  /* Kantenkriterium fuer Patches, die keine 0 im Bordercode haben und bei
     denen auch keine 0 mehr hinzukommen kann, da nur noch 3- und 4- und 5-Ecke
     hinzukommen duerfen (=> kein Einschluss moeglich) (3). 
     Wenn der Code keine 0 hat, so ist er nur dann als Brillenpatch zu 
     gebrauchen, wenn seine Bordercodelaenge 2 ist. Bis es dazu kommt, muessen
     len-2 Flaechen hinzukommen. Ist der Patch dann zu gross fuer einen
     Brillenpatch, so kann das Kantenkriterium angewendet werden. */
  if (pv>6 && !bb && len>2 && len+gr6>(f_max-fl-1)*(6-small_face) &&
      !ist_null_im_code(bordercode,len) &&
      (!fuenfecke || len>4 || (len==4 &&   /* len==3 => abgedeckt durch (5) */
      (bordercode[1]!=1 || bordercode[3]!=1 ||
      (fl+(len-2)>f_max-(f_min1<<1) &&  
      (((val_2_knoten ? val_2_knoten : (val_2_knoten = 
       ermittle_val_2_knoten(bordercode,len)))-1)>>1)+2+fl+(len-2) > f_max)))))
       {return(True);}
  
  /* Kantenkriterium fuer nicht brauchbare Patches, bei denen aber kein
     Einschluss mehr hinzukommen kann, weil dann die Flaechenzahl des Patches
     zu gross wuerde: (4) */
  /* Das Kriterium klappt auch, wenn der Einschluss gar nicht im Patch selbst
     vorkommt:  Wenn der Patch als Mittelteil einer Brille verwendet wird, so
     muessen zwei Brillenglaeser mit je >=f_min1 Flaechen hinzukommen. Also
     muss gelten:  fl<=f_max-2*f_min1 = f_max1-f_min1. Falls der Patch 
     verworfen wird, so ist diese Bedingung also nicht erfuellt. */ 
  if (pv>4 && !bb && fl+f_min1>f_max1 && 
     (((val_2_knoten ? val_2_knoten : (val_2_knoten = 
      ermittle_val_2_knoten(bordercode,len)))-1)>>1)+2+fl > f_max) 
        {return(True);}
  
  /* Kantenkriterium fuer nicht brauchbare Patches, bei denen kein
     Einschluss vorgenommen werden kann, weil keine 0 im Bordercode ist und wo
     nach einer zusaetzlichen Flaeche kein Einschluss mehr vorgenommen werden
     kann, weil dann der Patch zu gross wuerde: (5) */
  /* Das Kriterium ist nicht korrekt, wenn der Einschluss gar nicht im Patch
     selbst vorgenommen wird, denn beim Bau einer Brille muss im Bordercode
     keine 0 vorkommen (code (x y)). Um den Patch fuer den Bau einer Brille 
     auszuschliessen, wird die Codelaenge ueberprueft. Nach Einbau einer
     zusaetzlichen Flaeche in den Patch gilt Kriterium (4). */
  if (pv>5 && !bb && fl+f_min1==f_max1 && len!=2 && 
      !ist_null_im_code(bordercode,len) 
      && (((val_2_knoten ? val_2_knoten : (val_2_knoten = 
      ermittle_val_2_knoten(bordercode,len)))-1)>>1)+2+fl > f_max) 
        {return(True);}

  /* Kantenkriterium fuer nicht brauchbare Patches, bei denen kein Einschluss
     vorgenommen werden kann, weil der Bordercode allgemein nicht passt, und 
     bei denen nach einer zusaetzlichen Flaeche kein Einschluss mehr vorgenom-
     men werden kann, weil dann der Patch zu gross wuerde: (6)
     Es wird geprueft, ob nur noch 3-, 4- und 5-Ecke hinzukommen duerfen.
     Ist dies der Fall, so gibt es nur vier Patches, die eingeschlossen werden
     koennen, und so kann man den erforderlichen Codeausschnitt sehr genau
     bestimmen. Wenn ein Einschluss vorgenommen wird, muss ein Bauchbindenpatch
     entstehen, da dann f_max1 Flaechen erreicht werden. Ferner muss dann die
     Randcodierung auf den zugehoerigen BB-Patch mit f_min1 Flaechen abgestimmt
     sein. */
  /* Wieder muss abgeklaert werden, dass der Patch selbst nicht fuer eine 
     Brille benutzt werden kann (wird er groesser, so wird er zu gross fuer
     eine Brille). */
  if (pv>7 && !bb && fl+f_min1==f_max1 && (len>4 || (len==4 && (bordercode[1] 
      || bordercode[3]))) && len+gr6>(f_max-fl-1)*(6-small_face) && 
        /* eigentlich len+6+(gr6-6) */  
      (((val_2_knoten ? val_2_knoten : (val_2_knoten = 
      ermittle_val_2_knoten(bordercode,len)))-1)>>1)+2+fl > f_max) {
    /* das Kantenkriterium passt, nun muss noch geprueft werden, ob ein
       Einschluss moeglich ist: */
    if (len>5 || small_face==5) {return(True);}  /* kein BB-Einschluss bzw.
                                   kein Einschluss nur aus 5-Ecken moeglich */
    gefunden = False;
    memcpy(&bordercode[len],bordercode,sizeof(KNOTENTYP)*len);
    for (i=0; i<len && !gefunden; i++) {
      if (bordercode[i]==0) {
        if (len==5 && bordercode[i+2]==0) {     /* Fall 1a oder 2a */
          j = (EULERTYP)(bordercode[i+3])-(EULERTYP)(bordercode[i+1])-1;
          if (bordercode[i+4]==f_min1-1 &&
             ((j==2 && dreiecke && vierecke) || 
              (j==3 && vierecke && fuenfecke && f_min1==3) /* ||
              (j==4 && vierecke && fuenfecke && fl+4<=f_max1)*/)) 
                {gefunden=True;}
          j = (EULERTYP)(bordercode[i+1])-(EULERTYP)(bordercode[i+3]);
          if (bordercode[i+4]==f_min1-1 &&
             ((j==2 && dreiecke && vierecke) ||
              (j==3 && vierecke && fuenfecke && f_min1==3) /* ||
              (j==4 && vierecke && fuenfecke && fl+4<=f_max1)*/))
                {gefunden=True;}
          /* falls j==4 erfuellt und "vierecke" erfuellt => f_min1<=3 =>
             fl+4>f_max1, denn fl+f_min1==f_max1 */
        }
        if (len==4) {    /* Fall 1b oder 2b: */
          j = (EULERTYP)(bordercode[i+2])-(EULERTYP)(bordercode[i+1])-2;
          if (j==2 && dreiecke && fuenfecke && bordercode[i+3]==1)
             {gefunden=True;}
          j = (EULERTYP)(bordercode[i+1])-(EULERTYP)(bordercode[i+2])-1;
          if (j==2 && dreiecke && fuenfecke && bordercode[i+3]==1) 
             {gefunden=True;}
          /* falls bordercode[i+3]!=1 => entsteht Bauchbinde !=2 => schlecht */
        }
      }
    }
    if (!gefunden) {return(True);}
  }

  /* Knotenkriterium fuer BB-Patches: */
  if (bb && n_max_bb < bordercode[0]+(fl<<1)-2 && pv>2) {return(True);}

  return(False);     /* kein Kriterium versagt */
}


/******************************************/
/* Funktionen zum Generieren der Patches: */
/******************************************/

/***********************ORDNE_PATCH_IN_BAUM**********************************/
/*   Nimmt einen Patch in den zugehoerigen Patchbaum auf.                   */
/*   bb = True  <=>  Bauchbindenpatch                                       */
/*   len = Laenge des Bordercodes, falls nicht BB-Patch                     */
/*   fl = Anzahl der Flaechen im Patch                                      */
/*   prev1, prev2, i, j, art: Informationen ueber die Vorgaenger            */
/*   facearray:  Anzahlen der einzelnen Flaechen                            */
/*   krit:  Anzahl der kritischen Flaechen im Patch (0, falls alternativ)   */

void ordne_patch_in_baum(ELEM *prev1, ELEM *prev2, KNOTENTYP i, KNOTENTYP j,
  unsigned char art, KNOTENTYP *bordercode, KNOTENTYP len, KNOTENTYP wh, 
  KNOTENTYP fl, BOOL bb,KNOTENTYP *facearray,KNOTENTYP nahtlen,
  KNOTENTYP ziellen,KNOTENTYP krit) {
  static int l;       /* Zeiger auf aktuelle Stelle im Bordercode */
  static TREENODE *t, *t2, *t3, *t4;   /* t-t4 siehe b */  
  static BBTREENODE *b, *b2, *b3, *b4;  /* b = aktuell, b2 = Vorgaenger, 
                                           b3 = neu, b4 = letzte Wurzel */
  static ELEM *e;
  
  if (bb) {                 /* Typ-1-Patch einsortieren */
    /* erster Level: Anzahl der kritischen Flaechen */
    b = bbtree[fl];  b2 = nil;
    while (b && b->code<krit) {b2=b; b=b->next;}
    if (b==nil || b->code!=krit) {    /* neuen Baumknoten einrichten */
      b3 = (BBTREENODE *)hole_speicher(sizeof(BBTREENODE));
      if (b2==nil) {bbtree[fl] = b3;} else {b2->next = b3;}
      b3->next = b;   b3->firstpatch = nil;   b3->code = krit;
      b3->nextlevel = nil;
      b = b3;        /* von hier aus weitergehen */
    }
    /* zweiter Level: Bordercode */
    b4 = b;   b2 = nil;   b = b->nextlevel;
    while (b && b->code<bordercode[0]) {b2=b; b=b->next;}
    if (b==nil || b->code!=bordercode[0]) {  /* neuen Baumknoten einrichten */
      b3 = (BBTREENODE *)hole_speicher(sizeof(BBTREENODE));
      if (b2==nil) {b4->nextlevel = b3;} else {b2->next = b3;}
      b3->next = b;   b3->firstpatch = nil;   b3->code = bordercode[0];
      b3->nextlevel = nil;
      b = b3;        /* b ist Anfang der Liste, in die eingeordnet wird */
    }      /* if */
  }         /* if (bb) */   

  else {                    /* Typ 2 oder 3 einsortieren */
    /* erster Level: Anzahl der kritischen Flaechen */
    t = tree[fl];   t2 = nil;
    while (t && t->code<krit) {t2=t; t=t->next;}
    if (t==nil || t->code!=krit) {   /* neuen Baumknoten einrichten */
      t3 = (TREENODE *)hole_speicher(sizeof(TREENODE));
      if (t2==nil) {tree[fl] = t3;} else {t2->next = t3;}
      t3->next = t;   t3->firstpatch = nil;   t3->code = krit;
      t3->nextlevel = nil;
      t = t3;       /* von hier aus weitergehen */
    }
    /* weitere Level: Bordercode */
    l = 0;   
    while (l<len) {         /* noch nicht am Ende des Codes */
      t4 = t;  t = t->nextlevel;  t2 = nil;
      while (t && t->code<bordercode[l]) {t2=t; t=t->next;}
      if (t==nil || t->code!=bordercode[l]) { /* neuen Baumknoten einrichten */
        t3 = (TREENODE *)hole_speicher(sizeof(TREENODE));
        if (t2==nil) {t4->nextlevel = t3;} else {t2->next = t3;}
        t3->next = t;   t3->firstpatch = nil;
        /* t3->wh bleibt zunaechst unbesetzt, denn solange die Liste keinen
           Patch erhaelt, ist der Wert sowieso uninteressant */
        t3->code = bordercode[l];   t3->nextlevel = nil;
        t = t3;        /* t ist Anfang der Liste, in die eingeordnet wird */
      }      /* if */
      l++;
    }        /* while */
  }         /* else (bb) */   

  e = (ELEM *)hole_speicher(sizeof(ELEM));
  e->prev1 = prev1;  e->prev2 = prev2;  e->i = i;  e->j = j;
  e->art = art;
  e->next = (bb) ? b->firstpatch : t->firstpatch;
  e->flaechenzahl = facearray;
  e->nahtlen = nahtlen;
  e->ziellen = ziellen;
  e->test = 0;
  if (prev1) {nahttyp[art]++;}                         /* fuer Statistik */
  if (bb) {b->firstpatch = e;  patches1++;  fl1_anz[fl]++;} 
  else    {t->firstpatch = e;  t->wh = wh;  patches23++;  fl2_anz[fl]++;}
}      
   
/***********************IS_MAXIMAL******************************************/
/*  Prueft einen Bordercode auf Maximalitaet.                              */
/*  Der Bordercode muss zweimal hintereinander im Speicher stehen.         */
/*  Falls der Bordercode maximal ist, wird die Wiederholungszahl zurueck-
    gegeben, andernfalls eine Null.                                        */

KNOTENTYP is_maximal(KNOTENTYP *code,KNOTENTYP len) {
  static KNOTENTYP i,j;       /* i = offset */
  static EULERTYP vgl;
  i = 1;
  while (i<len) {
    j=0;
    while (j<len) {  
      if ((vgl = (EULERTYP)(code[j]-code[i+j]))<0) {return(0);}
      else if (vgl>0) {j=len;}
      else {j++; if (j==len) {return(i);} }
    }
    i++;
  }
  return(len);
} 

/*********************VERKNUEPFE_BORDERCODES**********************************/
/*  Ermittelt aus zwei Bordercodes einen neuen und prueft auf Maximalitaet,
    falls test==True (andernfalls ist Testergebnis unwichtig)                */
/*  Voraussetzung:  i1!=i2                                                   */
/*  Der Ergebniscode wird zweimal hintereinander in den Speicher geschrieben */

KNOTENTYP verknuepfe_bordercodes(KNOTENTYP i1,KNOTENTYP i2,KNOTENTYP j1,
     KNOTENTYP j2,KNOTENTYP *ergcode,KNOTENTYP *erglen,KNOTENTYP *code1,
     KNOTENTYP *code2,KNOTENTYP len1,KNOTENTYP len2,BOOL test) {
  ergcode[0] = (j1+len2==j2) ? 2+code2[j2]+code1[i2] : 1+code2[j2];
  *erglen=1;
  if (j1+len2!=j2) {
    j2++;
    if (j2<j1+len2) {memcpy(&ergcode[1],&code2[j2],
                            sizeof(KNOTENTYP)*(size_t)(j1+len2-j2));}
    *erglen += j1+len2-j2;
    ergcode[(*erglen)++] = code2[j1]+1+code1[i2];
  }
  i2++;
  if (i2<i1+len1) {memcpy(&ergcode[*erglen],&code1[i2],
                          sizeof(KNOTENTYP)*(size_t)(i1+len1-i2));}
  *erglen += i1+len1-i2;
  memcpy(&ergcode[*erglen],ergcode,sizeof(KNOTENTYP)*(*erglen));
  return(test ? is_maximal(ergcode,*erglen) : 1);
}

/*********************VERKNUEPFE_BORDERCODESE*******************************/
/*  ermittelt bei Einschluss einen neuen Bordercode und prueft auf
    Maximalitaet, falls test==True.   Voraussetzung:  i1!=i2               */

KNOTENTYP verknuepfe_bordercodesE(KNOTENTYP i1,KNOTENTYP i2,KNOTENTYP *ergcode,
     KNOTENTYP *erglen,KNOTENTYP *code,KNOTENTYP len,BOOL test) {
  ergcode[0] = 1+code[i2];
  *erglen=1;
  i2++;
  if (i2<i1+len) {memcpy(&ergcode[*erglen],&code[i2],
                         sizeof(KNOTENTYP)*(size_t)(i1+len-i2));}
  *erglen += i1+len-i2;
  memcpy(&ergcode[*erglen],ergcode,sizeof(KNOTENTYP)*(*erglen));
  return(test ? is_maximal(ergcode,*erglen) : 1);
}

/*********************BILDE_BORDERCODEE*************************************/
/*  bildet den neuen Bordercode bei bevorstehendem Einschluss eines
    Patches und prueft, ob das Ergebnis kanonisch ist.
    i1,i2:  Positionen im einschliessenden Code an den Raendern der geloeschten
            Werte
    Rueckgabewerte:
    erglen = Laenge des erzeugten Bordercodes
    Funktionswert enthaelt Wiederholungszahl, falls Beginn der Naht kanonisch
                           0 sonst
    bb == True    <=>  der entstehende Patch ist ein Bauchbindenpatch      */

KNOTENTYP bilde_bordercodeE(KNOTENTYP *ergcode,KNOTENTYP *erglen,
     KNOTENTYP *code,KNOTENTYP i1,KNOTENTYP i2,KNOTENTYP len,BOOL *bb) {
  if (i1+len==i2)        /* es entsteht Bauchbinde */
    {*erglen=1;  ergcode[0]=code[i1]+1;  *bb = True;  return(1);}
 
  /* normale Faelle: */
  *bb = False;  
  if (code[i1]!=0) {return(0);}
  else {return(verknuepfe_bordercodesE(i1,i2,ergcode,erglen,code,len,True));}
}        


/**********************MARKIERE_RANDKNOTEN***********************************/
/*  Diese Funktion markiert die Randknoten des Patches, dessen Randkante k
    ist und dessen Numerierung in m festgehalten ist. Dafuer wird der
    Eintrag "nr" benutzt. "nr==1" <=> Randknoten                            */

void markiere_randknoten(PLANMAP m,KANTE *k) {
  static KANTE *k2;
  k2 = k;
  do {
    m[k2->ursprung-1][0]->nr = 1;     /* m[][1] und m[][2] brauchen nicht
                           belegt zu werden, da sie nicht abgefragt werden */
    k2 = k2->invers->next;
    if (k2->name==aussen) {k2 = k2->next;}
  } while (k2!=k);
}  

/****************FINDE_PATCHZUSAMMENHANG**************************************/
/*  Diese Funktion uebergibt die maximale Zusammenhangszahl jedes Graphen,
    an dem der Patch beteiligt ist, dessen Numerierung in "m" enthalten ist.
    Der Patch besitzt "n" Knoten.                                            */

unsigned char finde_patchzusammenhang(PLANMAP m,KNOTENTYP n) {
  static KNOTENTYP i,j,l,y,z,con;
  static KANTE *k;
  con = 3;         /* solange nichts anderes gezeigt ist */
  j = 0;           /* naechste zu vergebende Nummer (fuer eine Flaeche) */
  for (i=0; i<n && con>1; i++) {     /* Knoten durchlaufen */
    if (m[i][0]->nr==0) {   /* kein Randknoten => Analyse sinnvoll */
      for (l=0; l<3 && con>1; l++) {   /* die drei adjazenten Kanten 
                                          durchlaufen */
        if (m[i][l]->all==0)  /* rechte Flaeche noch nicht numeriert */
          {belegedummies(m[i][l],++j);  y=j;}       /* Flaeche numerieren */
        else {y = m[i][l]->all;}    /* y = Nummer der rechten Flaeche */
        k = m[i][l];   z = 0;
        do {       /* linke Flaeche durchlaufen */
          if (k->invers->all==y) {con = 1;}
          else if (k->all==y) {z++;}   /* z = Anzahl gemeinsamer Kanten
                                        zwischen linker und rechter Flaeche */
          k = k->invers->next;
        } while (con>1 && k!=m[i][l]);
        if (z>1 && con>2) {con=2;}     /* >=2 gemeinsame Kanten */
      }
    }
  }
  return(con);
}  

/**************UEBERPRUEFE_WEGE_IN_PATCH*************************************/
/* Diese Funktion ist fuer beliebige Patches m mit mehr als einer Flaeche 
   und n Knoten. Sie ueberprueft, ob alle
   Zickzackpfade den Patch durchqueren oder ob zwei Zickzackpfade zwei oder
   mehr Schnittkanten besitzen (vgl. Funktion "ueberpruefe_wege_in_BBpatch").
   Ist das der Fall, so wird "False" zurueckgegeben. Fuer p3-PGAs ist ein
   solcher Patch unbrauchbar, da auch jeder BB-Patch, der aus dem ueberprueften
   Patch hervorgeht, den Test in der Funktion "ueberpruefe_wege_in_BBpatch" 
   nicht
   bestehen wuerde (die Schleifen- oder Schnittkanteneigenschaft wird vererbt).
   Im Patch muessen alle all-Variablen den Wert 0 besitzen. */
/* "start" ist die Kante, VOR der eine Naht beginnt. */
/* Es erfolgen wie bei der Funktion "ueberpruefe_wege_in_BBpatch" zwei
   Schleifendurchlaeufe. Zusaetzlich erfolgt ein dritter Schleifendurchlauf. */
/* Innerhalb dieser Funktion wird das Element "pfadanfang" jeder Kante 
   zweckentfremdet. Zu Beginn muss es den Wert "True" enthalten (wie bei der
   Konstruktion des Patches).                                                */
  
BOOL ueberpruefe_wege_in_patch(PLANMAP m,KNOTENTYP n,KANTE *start) {
  static KANTE *k,*kr;   /* kr = Randkante, nach der der Petriepfad beginnt
                            (der Wert von "start" muss erhalten bleiben) */
  static KNOTENTYP i,j,pfad;  
              /* "pfad" = Nummer des aktuell betrachteten Pfades */
              /* Maximum fuer "pfad":  N_MAX, denn von jedem zweiten Randknoten
                 koennen 2 verschiedene Pfade ausgehen */
  static BOOL vergleich[N_MAX+1];   /* zum Abhaken der Schnittkanten */
  static KNOTENTYP anz_pfad;  /* Anzahl der verschiedenen Pfade */  

  /* erster Durchlauf: Pfade markieren -> Schleifen ueberpruefen */
  pfad = 0;  kr = start;
  /* "kr" zeigt immer vom Inneren des Pfades weg */
  do {
    pfad++;  j=0;
    if (kr->all==0) {   /* Pfad wurde noch nicht durchlaufen */
                        /* eventuell entstehen Luecken in der Numerierung */
      kr->all = kr->invers->all = pfad;
      k = (pfad&1) ? kr->next : kr->prev;
      while (k->name!=aussen) {      /* Pfad markieren */
        if (k->all == pfad) {return(False);}       /* Schleife durchlaufen */
        if (k->all) {k->pfadanfang = k->invers->pfadanfang = False;}  /* Kante
                                   wird bereits zum zweiten Mal durchlaufen */
        k->all = k->invers->all = pfad;
        j++;
        if ((j&1)==(pfad&1)) {k = k->invers->prev;} else {k = k->invers->next;}
      }
    }
    if (pfad&1) {         /* Bruchkanten ueberspringen */
      kr = kr->invers;
      do {kr = kr->prev->invers;} while (kr->next->name==aussen);
    }
    else {kr = kr->next;}
  } while (kr!=start);

  /* zweiter Durchlauf: */
  /* Eine Kante ist Schnittkante von hoechstens zwei verschiedenen 
     Zickzackpfaden (linke Fortsetzung, rechte Fortsetzung). Also gilt: Wenn
     Pfad Nummer i durchlaufen wird und eine Nummer j>i gefunden wird, so ist
     die zugehoerige Kante eine Schnittkante zwischen den Pfaden i und j und
     zwischen keinen anderen Pfaden. Deshalb erhaelt man alle Schnittkanten,
     wenn man (wie bei der Funktion "ueberpruefe_wege_in_BBpatch") zuerst
     Pfad 1, dann Pfad 2, dann Pfad 3 usw. entlanggeht, denn kleine Nummern
     werden durch grosse ueberschrieben, aber nicht umgekehrt. */

  anz_pfad = pfad;      /* eventuell Luecken in Numerierung */
  kr = start;   pfad=0;
  do {
    for (i=1; i<=anz_pfad; i++) {vergleich[i] = False;}
    pfad++;  j=0;
    if (kr->all==pfad) {    /* sonst Pfad bereits durchlaufen */
      k = (pfad&1) ? kr->next : kr->prev;
      while (k->name!=aussen) {  /* Pfad Nr. "pfad" durchlaufen */
        if (k->all!=pfad) {
          if (vergleich[k->all]) {return(False);}  /* zwei Schnittkanten */
          else {vergleich[k->all] = True;}
        }
        j++;
        if ((j&1)==(pfad&1)) {k = k->invers->prev;} else {k = k->invers->next;}
      }
    }
    if (pfad&1) {         /* Bruchkanten ueberspringen */
      kr = kr->invers;
      do {kr = kr->prev->invers;} while (kr->next->name==aussen);
    }
    else {kr = kr->next;}
  } while (kr!=start);

  /* dritter Durchlauf: */
  /* Gibt es innere Kanten, die nicht oder nur einmal numeriert sind (also
     k->pfadanfang==True)? Wenn ja, so wird auch 
     diese Eigenschaft vererbt, so dass im resultierenden BB-Patch Kanten sind,
     die in keinem oder nur in einem Zickzackpfad enthalten sind, der von Rand
     zu Rand laeuft. 
     Dann kann dieser BB-Patch kein Kandidat fuer ein p3-maximales PGA sein. */
  /* In der Praxis zeigte sich, dass dieser Durchlauf nicht erfolgversprechend
     ist, denn eine derartige innere Kante muesste schon Teil eines 
     Bauchbindenpfades sein, der komplett im Inneren des Patches verlaeuft. */
  for (i=0; i<n; i++) {
    for (j=0; j<=2; j++) {
      if (m[i][j]->pfadanfang==True &&   /* maximal einmal numerierte Kante */
          m[i][j]->fl_links && m[i][j]->fl_rechts)       /* => innere Kante */
         {fprintf(stderr,"3"); return(False);}
    }
  }
  return(True);
}

/*********************UEBERPRUEFE_WEGE_IN_BBPATCH****************************/
/* Diese Funktion ist nur fuer Bauchbinden-Patches (p3-PGA-Kandidaten).
   Falls nicht alle Petriepfade den Patch durchqueren, wird False zurueckgege-
   ben. "start" ist die Kante, VOR der die Naht beginnt, und "ziel" die Kante,
   nach der die Naht endet. Ferner muessen alle Petriepfade auch noch gleich
   lang sein, und zwar so viele Kanten, wie "laenge" angibt. Der Wert bezieht
   sich auf die Anzahl der inneren Kanten der Pfade und ist immer gerade.
   Desweiteren muessen dann
   fuer jeden Petriepfad die Aussenwege zwischen Start- und Zielkante genauso
   lang sein wie der Petrie-Pfad zwischen diesen Kanten.  
   Im Patch muessen alle all-Variablen den Wert 0 besitzen.                 */
/* Nach dem ersten Schleifendurchlauf werden alle Petriepfade noch einmal
   durchlaufen. Dabei wird ueberprueft, ob zwei Petriepfade mehr als eine
   Kante gemeinsam haben.
   Durch die ganzen Vorbedingungen, die erfuellt sein muessen, ist dieser
   zweite Durchlauf wahrscheinlich ueberfluessig, aber was soll`s, die
   Funktion wird sowieso nicht oft erfolgreich durchlaufen, so dass der
   zweite Durchlauf nur selten erreicht wird. */

BOOL ueberpruefe_wege_in_BBpatch(KANTE *start,KNOTENTYP laenge) {
  static KANTE *k,*kr;   /* kr = Randkante, nach der der Petriepfad beginnt
                            (der Wert von "start" muss erhalten bleiben) */
  static KANTE *ziel;
  static KNOTENTYP i,j,pfad;  
              /* "pfad" = Nummer des aktuell betrachteten Pfades */
              /* Maximum fuer "pfad":  N_MAX, denn von jedem zweiten Randknoten
                 koennen 2 verschiedene Pfade ausgehen */
  static BOOL vergleich[N_MAX+1];   /* zum Abhaken der Schnittkanten */
  static KNOTENTYP anz_pfad;  /* Anzahl der verschiedenen Pfade */  

  /* "ziel" bestimmen: Es wird derjenige Pfad entlanggegangen, durch den
     der Patch zusammengeklebt wurde */
  k = start;
  for (i=0; i<laenge; i++) {
    if (i&1) {k = k->prev->invers;} else {k = k->next->invers;} 
  }
  ziel = k->next->invers;
  if (ziel->prev->name!=aussen) {fprintf(stderr,"Logischer Fehler in "
                                 "ueberpruefe Petriepfade!\n"); exit(100);}
      
  /* erster Durchlauf: Laenge der Pfade und Schleifen ueberpruefen */
  pfad = 0;
  kr = start;  ziel = ziel->invers;
  /* "kr" und "ziel" zeigen immer vom Inneren des Pfades weg */
  do {
    pfad++;  j=0;
    if (pfad>laenge+1) {
      if (kr->all!=pfad-laenge-1)
         {fprintf(stderr,"Reihenfolge passt nicht\n"); return(False);}
      /* Den Pfad abzugehen eruebrigt sich, denn da "laenge" immer gerade ist,
         ist klar, dass man den Pfad mit der Nummer "pfad-laenge-1" 
         rueckwaerts gehen wuerde. */
    }
    else {
      kr->all = kr->invers->all = pfad;
      k = (pfad&1) ? kr->next : kr->prev;
      while (k->name!=aussen) {   /* Pfad markieren */
        if (k->all == pfad) {return(False);}
        k->all = k->invers->all = pfad;
        j++;
        if ((j&1)==(pfad&1)) {k = k->invers->prev;} else {k = k->invers->next;}
      }
      if (j-1!=laenge) {return(False);}
      if ((pfad&1 ? k->next->invers : k->prev->invers) != ziel) 
         {return(False);}
    }
    if (pfad&1) {kr = kr->invers->prev->invers;  ziel = ziel->next;}
    else        {kr = kr->next;  ziel = ziel->invers->prev->invers;}
  } while (kr!=start);

  /* zweiter Durchlauf: */
  /* Wenn dieser Durchlauf erreicht wird, so steht fest:
     Wenn man die Anfaenge und Enden der VERSCHIEDENEN Petriepfade gegen den 
     Uhrzeigersinn jeweils mit 1,...,anz numeriert, so durchlaeuft man, wenn 
     man den gesamten Rand des Patches betrachtet, die Sequenz
     1,...,anz,1,...,anz .
     Also haben je zwei Petriepfade mindestens eine Schnittkante. Desweiteren
     hat man alle verschiedenen Petriepfade, wenn man "anz" (gegen den 
     Uhrzeigersinn) aufeinanderfolgende Petriepfade betrachtet. */
  /* Der zweite Durchlauf funktioniert folgendermassen:
     Da zuerst Pfad 1 markiert wurde, dann Pfad 2, Pfad 3 usw., so muss, wenn
     Pfad 1 mit jedem anderen Pfad genau eine Schnittkante besitzt, jede
     Pfadnummer (ausser 1) genau einmal auftreten, wenn man die inneren Kanten
     von Pfad 1 ablaeuft. Wenn man die inneren Kanten von Pfad i ablaeuft, so
     muss jede Nummer, die hoeher ist als i, genau einmal auftreten (= eine
     Schnittkante), jede kleinere Nummer darf nicht auftreten, da diese durch
     i ueberschrieben wurde. Per Induktion weiss man aber, dass Pfad i mit 
     allen kleiner numerierten Pfaden genau eine Schnittkante hat, so dass
     Pfad i mit ALLEN anderen Pfaden je genau eine Schnittkante hat. */

  anz_pfad = pfad/2;   kr = start;   pfad=0;
  if (anz_pfad != laenge+1) {fprintf(stderr,"Logischer Fehler 2 in "
                                     "ueberpruefe_petriepfade\n"); exit(100);}
  do {
    for (i=1; i<=anz_pfad; i++) {vergleich[i] = False;}
    pfad++;
    k = (pfad&1) ? kr->next : kr->prev;
    for (j=0; j<laenge; j++) {
      /* der Pfad mit der Nummer "pfad" wird abgelaufen (innere Kanten) */
      if (vergleich[k->all]) {if (k->all!=pfad) {return(False);}}
         /* zwei Schnittkanten der Pfade "pfad" und "k->all" */ 
      else {vergleich[k->all]=True;}    /* erste Schnittkante gefunden */
      if ((j&1)!=(pfad&1)) {k = k->invers->prev;} else {k = k->invers->next;}
    }
    for (i=1; i<=anz_pfad; i++) {
      if (i!=pfad && vergleich[i]!=(i>=pfad)) {return(False);}
    }
    if (pfad&1) {kr = kr->invers->prev->invers;} else {kr = kr->next;}
  } while (pfad<anz_pfad);
  return(True);
}

/*********************MUSS_PATCH_KONSTRUIERT_WERDEN**************************/
/*  Diese Funktion prueft, ob der generierte Patch konstruiert werden muss,
    und konstruiert ihn gegebenenfalls. 
    Es gibt folgende Anlaesse:
    Bei S3PZ-Graphen:
    - Pruefung auf Doppelkante
    - Patchconn
    - spezielle Strukturen ueberpruefen  (z.B. drei Vierecke um einen Knoten)
    Bei p3-PGAs:
    - Patchconn
    - spezielle Strukturen ueberpruefen  (z.B. zwei benachbarte Fuenfecke)
    - BB-Patches pruefen, ob sie p3-PGA repraesentieren
    Der Rueckgabewert gibt an, ob der Patch die durchgefuehrten Pruefungen
    bestanden hat (dann "True").    
    Der Patch ist in "e", besitzt "fl" Flaechen, hat die Umrandung "bordercode"
    der Laenge "len" und ist ein Bauchbindenpatch, falls "bb"==True. Falls
    "einschl"==True, so ist er durch einen Einschluss entstanden.           */
/*  "e.next" und "e.fd" muessen nicht mit korrekten Werten gefuellt sein.   */
/*  Reihenfolge der einzelnen Abfragen nicht vertauschen, sonst muss die
    Ueberpruefung der Flags "numeriert" und "markiert" neugestaltet werden. */ 

BOOL muss_patch_konstruiert_werden(ELEM *e,KNOTENTYP fl,KNOTENTYP *bordercode,
     KNOTENTYP len,BOOL bb,BOOL einschl) {
  static KANTENARRAY2 map;
  static PLANMAP m;
  static KANTE *k;  
  static KNOTENTYP i;
  static KNOTENTYP n;         /* Knotenzahl (bei Bedarf berechnet) */ 
  static unsigned char con;
  static BOOL numeriert;      /* True => Patch ist numeriert */
  static BOOL markiert;       /* True => Randknoten sind markiert */
  static BOOL all_benutzt;    /* True => all-Eintraege sind nicht gleich 0 */ 

  /* Muss konstruiert werden? */
  if ((einschl && big_face>=7)     /* Doppelkante ueberpruefen */         ||
      (patchconn && conn1==False)  /* Patchzusammenhang pruefen */        ||
      barnette                     /* siehe Hamiltonpaper (3 Vierecke) */ ||
      (PGA && fl>1)) {
    if (bb) {k = konstruiere_bb_patch(map,bordercode[0],e);}
    else    {k = konstruiere_patch(map,bordercode,len,e);}
    numeriert = markiert = False;  /* numeriert/markiert wird nur bei Bedarf */
    all_benutzt = False;    

    /* 1. Muss auf Doppelkanten getestet werden? */
    if (einschl && big_face>=7) {
      /* Bei PGA ist die erste Bedingung immer falsch. */
      numeriere_graph(m,k);  
      n = bb ? bordercode[0] + (fl<<1) - 2 : 
               ermittle_val_2_knoten(bordercode,len) + (fl<<1) - 2;
      numeriert = True;
      for (i=0; i<n; i++) {                   /* Kanten durchgehen */
        if (m[i][0]->name == m[i][1]->name  ||
            m[i][0]->name == m[i][2]->name  ||
            m[i][1]->name == m[i][2]->name) {return(False);}
      }
    }

    /* 2. Muss auf Patchzusammenhang getestet werden? (auch bei PGA) */
    /* conn1==True => minimaler Zusammenhang erlaubt => keine Einschraenkung */
    if (patchconn && conn1==False) {
      if (!numeriert)  {
        numeriere_graph(m,k);
        n = bb ? bordercode[0] + (fl<<1) - 2 : 
                 ermittle_val_2_knoten(bordercode,len) + (fl<<1) - 2;
        numeriert = True;
      }      
      markiere_randknoten(m,k);  markiert = True;
      con = finde_patchzusammenhang(m,n);  all_benutzt = True;
      if (con<2 || (con==2 && conn2==False)) {return(False);}
    }       
 
    /* 3. Ist die Option "barnette" eingestellt? */
    if (barnette) {
      if (!numeriert)  {
        numeriere_graph(m,k);
        n = bb ? bordercode[0] + (fl<<1) - 2 : 
                 ermittle_val_2_knoten(bordercode,len) + (fl<<1) - 2;
        numeriert = True;
      }      
      if (!markiert) {markiere_randknoten(m,k);  markiert = True;}
      if (drei_vierecke(m,n)) {return(False);}
      all_benutzt = True;
    }

    /* 4. Ist ein p3-PGA-Kandidat vorhanden? */
    if (PGA && bb) {
      if (!numeriert)  {
        numeriere_graph(m,k);
        n = bordercode[0] + (fl<<1) - 2;
        numeriert = True;
      }      
      if (all_benutzt) { 
        /* => Punkt 2 oder 3 aufgerufen => all-Eintraege loeschen */
        for (i=0; i<n; i++) {m[i][0]->all = m[i][1]->all = m[i][2]->all = 0;}
      }
      if (ueberpruefe_wege_in_BBpatch(k->invers->next,bordercode[0]-1)==False) 
         {return(False);}
      all_benutzt = True;
    }

    /* 5. Scheidet Patch als p3-PGA-Vorgaenger aus? */
    if (PGA && !bb && fl>1) {
      if (!numeriert)  {
        numeriere_graph(m,k);
        n = ermittle_val_2_knoten(bordercode,len) + (fl<<1) - 2;
        numeriert = True;
      }      
      if (all_benutzt) { 
        /* => Punkt 2 oder 3 aufgerufen => all-Eintraege loeschen */
        for (i=0; i<n; i++) {m[i][0]->all = m[i][1]->all = m[i][2]->all = 0;}
      }
      if (ueberpruefe_wege_in_patch(m,n,k->invers->next)==False) 
         {return(False);}
      all_benutzt = True;
    }
  }
  return(True);    /* alle Tests ueberstanden */
}
    
/*************************SCHREIBE_PATCH**************************************/
/*  Diese Funktion schreibt den Patch e in ein File. Dabei werden Aussenkanten
    nicht geschrieben. Es handelt sich also um gueltigen Planarcode.         */

void schreibe_patch(ELEM *e,KNOTENTYP *bordercode,KNOTENTYP len,BOOL bb,
                    KNOTENTYP fl,FILE *f) {
  static KNOTENTYP planarcode_c[CODESIZE(N_MAX)];
  static KNOTENTYP i;
  static size_t j;
  static PLANMAP m;
  static KANTE *k;
  static KNOTENTYP patch[CODESIZE(N_MAX)];
  static KANTENARRAY2 map;
  static KNOTENTYP n;

  if (bb) {k = konstruiere_bb_patch(map,bordercode[0],e);}
  else    {k = konstruiere_patch(map,bordercode,len,e);}
  n = (bb ? bordercode[0] : ermittle_val_2_knoten(bordercode,len))+2*fl-2;
  numeriere_graph(m,k);
  map_2_planarcode(m,patch,n);
  j = 0;
  for (i=0; i<CODESIZE(n); i++) {
    if (patch[i]!=aussen) {planarcode_c[j++] = patch[i];}
  }
  schreibe_planarcode(planarcode_c,f,j);
}
 
/**********************VERKNUEPFUNG******************************************/
/* Verknuepft zwei Patches entlang eines Durchschnittpfades. Deshalb waere
   "Durchschnitt" ein geeigneterer Name fuer diese Funktion.
   art==0 => gerader Durchschnitt,  art==1 => ungerader Durchschnitt        
   Variablen siehe "verknuepfe Patches"                                     */
/* Rueckgabewert: - unbedeutend im normalen Konstruktionsprozess
                  - in Rekursion: True <=> brauchbarer Patch erzeugt        */
/* Rekursion wird fuer PGAs nicht durchgefuehrt.                            */
/* gedreht==True => urspruenglicher Patch 2 ist hier links. Falls l1==l2,
   so wird die vorliegende Funktion zweimal mit denselben Parametern aufge-
   rufen, einmal mit gedreht==False und einmal mit gedreht==True. Damit die
   Listen nur einmal benutzt werden, wird dieser Wert zu Beginn abgefragt.  */
/* Wenn die Funktion aus einer Rekursion aufgerufen wird, so erhalten die
   Parameter "l1" und "l2" den Wert "nil" und die Parameter "nahtlen" und
   "ziellen" den Wert 0. Sie werden allesamt nicht gebraucht, ausser um
   abzufragen, ob die Funktion aus einer Rekursion aufgerufen wird. Ferner
   ist der Wert "no_krit2" unwichtig.                                       */
/* Falls notest==True => die Rekursion kann ohne Test aufgerufen werden,
   da die Funktion bereits aus der Rekursion kommt und der Test somit 
   bereits einmal richtig war. notest==True <=> Funktion wird aus Rekursion
   aufgerufen.                                                              */ 
/* "f_min" ist nur von Bedeutung, wenn krit<<1==krit_max. Das ist 
   insbesondere dann der Fall, wenn keine Aufspaltung gewuenscht wird, denn
   dann gilt krit<<1==0==krit_max. Falls notest==False, so ist f_min=fl-1,
   andernfalls f_min<fl-1.                                                  */
/* vg1, vg2: siehe "versagt_pga_patchkriterium" */

BOOL verknuepfung(KNOTENTYP i,TREENODE *l1,KNOTENTYP j,TREENODE *l2,
     BOOL gedreht,BOOL bb,KNOTENTYP fl,KNOTENTYP wh,
     KNOTENTYP *ergcode,KNOTENTYP erglen,unsigned char art,KNOTENTYP f_min,
     KNOTENTYP nahtlen,KNOTENTYP ziellen,BOOL notest,KNOTENTYP krit,
     KNOTENTYP *vg1,KNOTENTYP *vg2) {
  static ELEM *e1,*e2,e;
  static KNOTENTYP *facearray;    /* fuer die neuen Flaechenzahlen */
  static FLAECHENTYP f;
  static KNOTENTYP fd_neu;
  BOOL gegenstueck;
 
  if (PGA) {
    if ((bb && art==1)    /* ungerader Durchschnitt => kein p3-PGA */  ||
         versagt_pga_patchkriterium(ergcode,erglen,bb,fl,vg1,vg2)) 
       {return(False);}
  }
  else {
    if (bb && do_bauchbinde==False) {return(False);}
    if (pv>2 && versagt_patchkriterium(ergcode,erglen,bb,fl)) {return(False);}
  
    /* Gegenstuecke zu brauchbaren Patches suchen und
       rekursiv weitere Verklebungsmoeglichkeiten suchen:   */
    fd_neu = !alternative || anz_face==1 ? (krit_max-krit) :
             (big_face-6)*((6-small_face)*(f_max-fl)-(bb ? 6 : erglen+6)) / 
             (big_face-small_face);
    if (pv>8 && (notest  ||  krit<<1 > krit_max  ||  (krit<<1==krit_max && 
       (f_min>=f_max1-fl || 
       (pv>10 && (fd_neu+5)/(6-kl6)+max6+fd_neu/(g6-6) <= f_min))))) {
      /* man beachte:  alternative==TRUE => krit<<1==krit_max */
      /* man beachte:  falls alternative==FALSE, so kann die letzte Bedingung 
         nicht wahr sein, denn wo kaeme sonst der aktuelle Patch her? */
      /* Gegenstuecke FUER PATCHES stehen fest, Rekursion */
      if (bb) {gegenstueck = (krit<<1 > krit_max  ||  
               f_max_bb_best-fl <= f_min  || 
        (pv>10 && (fd_neu+6)/(6-kl6)+max6+fd_neu/(g6-6) <= f_min)) ?
               gibt_es_gegenstueck(fl,ergcode[0],krit) :  True;}
               /* Gegenstuecke bekannt/nicht bekannt */
      else if ((erglen<3 || (erglen==3 && (ergcode[1]==0 || ergcode[2]==0)) ||
               (erglen==4 && ergcode[1]==0 && ergcode[3]==0)) &&
               (do_sandwich || do_brille))     /* brauchbarer Patch */
        {gegenstueck = (krit<<1 > krit_max  ||  f_max-fl-1-(erglen>2) <= f_min 
           || (pv>10 && (fd_neu+5)/(6-kl6)+max6+fd_neu/(g6-6) <= f_min)) ? 
                       suche_gegenstuecke(fl,ergcode,erglen,krit) : True;}
      else {gegenstueck = False;}      /* kein brauchbarer Patch */

      if (!gegenstueck &&
         (bb || !gehe_patchbaum_durch_in_rekursion(ergcode,erglen,fl,wh,f_min,
          krit))) {return(False);}    /* wichtig: lazy-Eval. bei || */
    }
  }

  /* ab hier gilt: die Patches sind brauchbar oder mittelbar brauchbar
     oder nicht eindeutig unbrauchbar */
  if (l1==nil) {return(True);}   /* Vorgaengerpatches existieren noch nicht */

  /* Bis hierhin kommt die Funktion nicht innerhalb einer Rekursion, zudem 
     wird von hier aus keine weitere Rekursion eingeleitet
     => die zugehoerigen Variablen koennen "static" sein */

  if (l1!=l2 || !gedreht) {     /* Bedingung nicht erfuellt => doppelt */
    e1 = l1->firstpatch;
    while (e1) {
      e2 = l2->firstpatch;
      while (e2) {
        if (!facerestrict || (flaechenzahlen2_ok(e1,e2,False) &&
            (pv<2 || flaechenvorausschau_ok(e1,e2,fl,bb ? 6 : erglen+6)))) {
          e.prev1 = e1;  e.prev2 = e2;  e.i = i;  e.j = j;  e.art = art; 
          e.nahtlen = nahtlen;   e.ziellen = ziellen; 
          /* e.next ist unwichtig fuer "muss_patch_konstruiert_werden" */
          if (muss_patch_konstruiert_werden(&e,fl,ergcode,erglen,bb,False)) {
            if (facenumbers) {
              facearray =
                (KNOTENTYP *)hole_speicher(sizeof(KNOTENTYP)*(size_t)anz_face);
              for (f=0; f<anz_face; f++) 
                {facearray[f] = (e1->flaechenzahl)[f] + (e2->flaechenzahl)[f];}
            }
            else {facearray = nil;} 
            if (PGA && bb) {     /* Patch ausgeben */
              graphenzahl[(ergcode[0]+1)>>1][0]++;
              schreibe_patch(&e,ergcode,erglen,bb,fl,
                             outputfile[(ergcode[0]+1)>>1]);
            }
            else {               /* Patch speichern */
              ordne_patch_in_baum(e1,e2,i,j,art,ergcode,erglen,wh,fl,bb,
                                  facearray,nahtlen,ziellen,krit);
            }
          }
        }
        e2 = e2->next;
      }
      e1 = e1->next;
    }
  }
  return(True);    /* unwichtig, da nicht von Rekursion abgefangen */
}

/**********************EINSCHLUSS********************************************/
/* nimmt Einschluss vor
   Variablen siehe "verknuepfe Patches"                                     */
/* "art": Art des Einschlusses (2-5)                                        */
/* Immer: Ein Element aus l1 schliesst ein Element aus l2 ein               */
/* wird fuer PGAs nicht gebraucht                                           */

BOOL einschluss(KNOTENTYP i,TREENODE *l1,TREENODE *l2,BOOL bb,KNOTENTYP fl,
     KNOTENTYP wh,KNOTENTYP *ergcode,KNOTENTYP erglen,unsigned char art,
     KNOTENTYP f_min,KNOTENTYP nahtlen,KNOTENTYP ziellen,BOOL notest,
     KNOTENTYP krit) {
  static ELEM *e1,*e2,e;
  static KNOTENTYP *facearray;       /* fuer die neuen Flaechenzahlen */
  static FLAECHENTYP f;
  static KNOTENTYP fd_neu;
  BOOL gegenstueck;

  if (bb && (do_bauchbinde==False || ergcode[0]==1)) {return(False);}
  if (versagt_patchkriterium(ergcode,erglen,bb,fl))  {return(False);}

  /* Gegenstuecke zu brauchbaren Patches suchen und
     rekursiv weitere Verklebungsmoeglichkeiten suchen:   */
  fd_neu = !alternative || anz_face==1 ? (krit_max-krit) :
           (big_face-6)*((6-small_face)*(f_max-fl)-(bb ? 6 : erglen+6)) / 
           (big_face-small_face);
  if (pv>8 && (notest  ||  krit<<1 > krit_max  ||  (krit<<1==krit_max && 
     (f_min>=f_max1-fl || 
     (pv>10 && (fd_neu+5)/(6-kl6)+max6+fd_neu/(g6-6) <= f_min))))) {
    /* man beachte:  alternative==TRUE => krit<<1==krit_max */
    /* Gegenstuecke FUER PATCHES stehen fest, Rekursion */
    if (bb) {gegenstueck = (krit<<1 > krit_max  ||  
             f_max_bb_best-fl <= f_min  || 
            (pv>10 && (fd_neu+6)/(6-kl6)+max6+fd_neu/(g6-6) <= f_min)) ? 
             gibt_es_gegenstueck(fl,ergcode[0],krit) : True;}
    else if ((erglen<3 || (erglen==3 && (ergcode[1]==0 || ergcode[2]==0)) ||
             (erglen==4 && ergcode[1]==0 && ergcode[3]==0)) &&
             (do_sandwich || do_brille))    /* brauchbarer Patch */
      {gegenstueck = (krit<<1 > krit_max || f_max-fl-1-(erglen>2) <= f_min ||
         (pv>10 && (fd_neu+5)/(6-kl6)+max6+fd_neu/(g6-6) <= f_min)) ? 
                     suche_gegenstuecke(fl,ergcode,erglen,krit) : True;}
    else {gegenstueck = False;}   /* kein brauchbarer Patch */

    if (!gegenstueck &&
       (bb || !gehe_patchbaum_durch_in_rekursion(ergcode,erglen,fl,wh,f_min,
              krit))) {return(False);}   /* wichtig: lazy-Eval. bei || */
  }

  /* ab hier gilt: die Patches sind brauchbar oder mittelbar brauchbar
     oder nicht eindeutig unbrauchbar */
  if (l1==nil) {return(True);}   /* Vorgaengerpatches existieren noch nicht */

  e1 = l1->firstpatch;
  while (e1) {
    e2 = l2->firstpatch;
    while (e2) {
      if (!facerestrict || (flaechenzahlen2_ok(e1,e2,False) &&
          (pv<2 || flaechenvorausschau_ok(e1,e2,fl,bb ? 6 : erglen+6)))) {
        e.prev1 = e1;  e.prev2 = e2;  e.i = i;  e.j = 0;  e.art = art; 
        e.nahtlen = nahtlen;   e.ziellen = ziellen; 
        /* e.next ist unwichtig fuer "muss_patch_konstruiert_werden" */
        if (muss_patch_konstruiert_werden(&e,fl,ergcode,erglen,bb,True)) {
          if (facenumbers) {
            facearray = 
              (KNOTENTYP *)hole_speicher(sizeof(KNOTENTYP)*(size_t)anz_face);
            for (f=0; f<anz_face; f++) 
              {facearray[f] = (e1->flaechenzahl)[f] + (e2->flaechenzahl)[f];}
          }
          else {facearray = nil;} 
          ordne_patch_in_baum(e1,e2,i,0,art,ergcode,erglen,wh,fl,bb,facearray,
                              nahtlen,ziellen,krit);
        }
      }
      e2 = e2->next;
    }
    e1 = e1->next;
  }
  return(True);     /* unwichtig, da nicht von Rekursion abgefangen */
}

/*********************VERKNUEPFE_PATCHES_IN_REKURSION***********************/
/* Prueft, ob die uebergebenen Patches aus den Listen miteinander verbunden
   werden koennen (alle Patches einer Liste haben denselben Bordercode)    */
/* Diese Funktion funktioniert wie die Funktion "verknuepfe_patches",
   benutzt jedoch nicht den Inhalt der Arrays "bind" und "anz", so dass 
   sie rekursiv eingesetzt werden kann. Deshalb sind die Variablen nicht
   static.                                                                 */
/* Die Codes muessen dreimal hintereinander im Speicher stehen.            */
/* Die Funktion uebergibt "True", wenn ein brauchbarer Patch erzeugt
   wurde oder ein mittelbar brauchbarer Patch erzeugt wurde.               */
/* krit = Anzahl kritischer Flaechen in beiden Patches zusammen.           */
/* Die Funktion wird fuer PGAs nicht benutzt.                              */

BOOL verknuepfe_patches_in_rekursion(KNOTENTYP *code1,KNOTENTYP len1,
     KNOTENTYP *code2,KNOTENTYP len2,KNOTENTYP fl,KNOTENTYP f_min,
     KNOTENTYP wh1,KNOTENTYP wh2,KNOTENTYP krit) {
  KNOTENTYP i,j;       /* Verschiebungen im Bordercode */
  KNOTENTYP l;
  KNOTENTYP ergcode[3*N_MAX];   /* laenger als N_MAX kann ein erlaubter
       Code nie werden, da pro Eintrag mindestens ein Knoten verbraucht wird */
    /* In der Funktion selbst werden nur 2*N_MAX Eintraege verwendet, jedoch
       in der aufgerufenen Funktion "gehe_patchbaum_durch_in_rekursion" 3*N_MAX
       Eintraege. */
  KNOTENTYP wh;      /* wh!=0 => kanonische Verknuepfung gefunden */
  BOOL bb;                /* True => Bauchbindenpatch entsteht */
  KNOTENTYP erglen;

  if (len1>1 && len2>1) {             /* Durchschnitte moeglich */
    for (j=0; j<wh2; j++) {

      /* ungerade Verknuepfung testen: */
      if (len2>2 && code2[j]==0 && len1>2) { 
        for (i=0; i<wh1; i++) {
          if (code1[i]==0 && code1[i+1]==code2[j+1]) {

            /* Patch 1 links */
            if (len1==3 && len2==3) {      /* => Bauchbinde */
              ergcode[0] = code1[i+len1-1] + code2[len2+j-1] + 2;
              if (verknuepfung(i,nil,j,nil,False,True,fl,1,ergcode,1,1,
                               f_min,0,0,True,krit,nil,nil)) {return(True);}
            }
            else if (len1>3 && code1[len1+i-1]==0) {   
              wh = verknuepfe_bordercodes(i+len1-1,i+len1+2,len2+j-1,len2+j+2,
                   ergcode,&erglen,code1,code2,len1,len2,True);
              if (wh && verknuepfung(i,nil,j,nil,False,False,fl,wh,ergcode,
                  erglen,1,f_min,0,0,True,krit,nil,nil)) {return(True);}
            }

            /* Patch 1 rechts */
            if (len1==3 && len2==3) {    /* => Bauchbinde */
              ergcode[0] = code1[i+len1-1] + code2[len2+j-1] + 2;
              if (verknuepfung(j,nil,i,nil,True,True,fl,1,ergcode,1,1,
                               f_min,0,0,True,krit,nil,nil)) {return(True);}
            }
            else if (len2>3 && code2[len2+j-1]==0) {   
              wh = verknuepfe_bordercodes(j+len2-1,j+len2+2,len1+i-1,len1+i+2,
                   ergcode,&erglen,code2,code1,len2,len1,True);
              if (wh && verknuepfung(j,nil,i,nil,True,False,fl,wh,ergcode,
                  erglen,1,f_min,0,0,True,krit,nil,nil)) {return(True);}
            }
          }
        }
      }       /* ungerade Verknuepfung testen */
           
      /* gerade Verknuepfung testen, wobei Patch 2 = links: */
      if (((len2>4 && code2[len2+j-1]==0) || 
          (len2==4 && len1==2)) && code2[j]==0 && code2[j+2]==0) {
        for (i=0; i<wh1; i++) {
          if (code1[i]==code2[j+1]+1) {
            if (len2==4) {    /* => len1==2 => Bauchbinde */
              ergcode[0] = code2[len2+j-1] + code1[i+len1-1] + 2;
              if (verknuepfung(j,nil,i,nil,True,True,fl,1,ergcode,1,0,f_min,0,
                           0,True,krit,code2,code1)) {return(True);}
            }
            else {            /* keine Bauchbinde */
              wh = verknuepfe_bordercodes(len2+j-1,len2+j+3,i+len1-1,i+len1+1,
                   ergcode,&erglen,code2,code1,len2,len1,True);
              if (wh && verknuepfung(j,nil,i,nil,True,False,fl,wh,ergcode,
                        erglen,0,f_min,0,0,True,krit,nil,nil)) {return(True);}
            }
          }
        }
      }              /* gerade Verknuepfung mit Patch 2 links testen */
  
      /* gerade Verknuepfung testen, wobei Patch 2 = rechts: */
      if (((len1==4 && len2==2) || len1>4) && code2[j]) {
        for (i=0; i<wh1; i++) {
          if ((len1==4 || code1[i+len1-1]==0) && code1[i]==0 && 
              code1[i+2]==0 && code1[i+1]+1==code2[j]) {
            if (len1==4) {    /* => len2==2 => Bauchbinde */
              ergcode[0] = code2[len2+j-1] + code1[i+len1-1] + 2;
              if (verknuepfung(i,nil,j,nil,False,True,fl,1,ergcode,1,0,f_min,0,
                               0,True,krit,code1,code2)) {return(True);}
            }
            else {              /* keine Bauchbinde */
               wh = verknuepfe_bordercodes(i+len1-1,i+len1+3,len2+j-1,len2+j+1,
                    ergcode,&erglen,code1,code2,len1,len2,True);
               if (wh && verknuepfung(i,nil,j,nil,False,False,fl,wh,ergcode,
                         erglen,0,f_min,0,0,True,krit,nil,nil)) {return(True);}
	    }
          }
        }
      }           /* gerade Verknuepfung mit Patch 2 rechts testen */
    }             /* for j */
  }               /* if len1>1 && len2>1 */

  /* Im folgenden werden die Einschluesse getestet, bei denen Patch 2 als
     Patch A fungiert. Diese Tests sind in einer eigenen Schleife 
     untergebracht, da meistens die ganze Schleife entfallen kann. */
  if ((len1==1 || (len1==2 && code1[1]==0)) && len2>3) { 
    /* Patch 1 kann Patch B und Patch 2 kann Patch A sein */
    l = code1[0];
    for (j=0; j<wh2; j++) {
      if (len1==1 && len2>4 && code2[j]==0 && code2[j+2]==0) {

        if (code2[j+3]==code2[j+1]+1+l) {                          /* 1a */
          wh = bilde_bordercodeE(ergcode,&erglen,code2,len2+j-1,
                                 len2+j+4,len2,&bb);
          if (wh && einschluss(j,nil,nil,bb,fl,wh,ergcode,erglen,2,f_min,0,0,
                               True,krit)) {return(True);}
        }

        else if (code2[j+1]==code2[j+3]+l) {                       /* 2a */
          wh = bilde_bordercodeE(ergcode,&erglen,code2,len2+j-1,
                                 len2+j+4,len2,&bb);
          if (wh && einschluss(j,nil,nil,bb,fl,wh,ergcode,erglen,4,f_min,0,0,
                               True,krit)) {return(True);}
        }
      }

      if (len1==2 /* => code1[1]==0 && len2>3 */ && code2[j]==0) {

        if (code2[j+2]==code2[j+1]+2+l) {                          /* 1b */
          wh = bilde_bordercodeE(ergcode,&erglen,code2,len2+j-1,
                                 len2+j+3,len2,&bb);
          if (wh && einschluss(j,nil,nil,bb,fl,wh,ergcode,erglen,3,f_min,0,0,
                               True,krit)) {return(True);}
        }
        
        else if (code2[j+1]==code2[j+2]+1+l) {                     /* 2b */
          wh = bilde_bordercodeE(ergcode,&erglen,code2,len2+j-1,
                                 len2+j+3,len2,&bb);
          if (wh && einschluss(j,nil,nil,bb,fl,wh,ergcode,erglen,5,f_min,0,0,
                               True,krit)) {return(True);}
        }
      }
    }    /* for j */
  }      /* Patch 1 als Patch B und Patch 2 als Patch A */
          
  /* Im folgenden werden die Einschluesse getestet, bei denen Patch 2 als 
     Patch B fungiert. Schleifen ueber j sind nicht notwendig, da die Drehung 
     von Patch 2 immer 0 sein muss. */ 
  l = code2[0];

  if (len2==1 && len1>4) {           /* => Einschluss 1a oder 2a */
    for (i=0; i<wh1; i++) {                            
      if ((len1==5 || code1[len1+i-1]==0) && code1[i]==0 && code1[i+2]==0) {
           
        if (code1[i+3]==code1[i+1]+1+l) {                  /* 1a */
          wh = bilde_bordercodeE(ergcode,&erglen,code1,len1+i-1,
                                 len1+i+4,len1,&bb);
          if (wh && einschluss(i,nil,nil,bb,fl,wh,ergcode,erglen,2,f_min,0,0,
                               True,krit)) {return(True);}
        }

        else if (code1[i+1]==code1[i+3]+l) {               /* 2a */
          wh = bilde_bordercodeE(ergcode,&erglen,code1,len1+i-1,
                                 len1+i+4,len1,&bb);
          if (wh && einschluss(i,nil,nil,bb,fl,wh,ergcode,erglen,4,f_min,0,0,
                               True,krit)) {return(True);}
        }
      }
    }
  }
      
  if (len2==2 && code2[1]==0 && len1>3) {    /* => Einschluss 1b oder 2b */
    for (i=0; i<wh1; i++) {                            
      if ((len1==4 || code1[len1+i-1]==0) && code1[i]==0) {

        if (code1[i+2]==code1[i+1]+2+l) {                  /* 1b */
          wh = bilde_bordercodeE(ergcode,&erglen,code1,len1+i-1,
                                 len1+i+3,len1,&bb);
          if (wh && einschluss(i,nil,nil,bb,fl,wh,ergcode,erglen,3,f_min,0,0,
                               True,krit)) {return(True);}
        }

        else if (code1[i+1]==code1[i+2]+1+l) {             /* 2b */
          wh = bilde_bordercodeE(ergcode,&erglen,code1,len1+i-1,
                                 len1+i+3,len1,&bb);
          if (wh && einschluss(i,nil,nil,bb,fl,wh,ergcode,erglen,5,f_min,0,0,
                               True,krit)) {return(True);}
        }
      }
    }
  }
  return(False);    /* keinen (mittelbar) brauchbaren Patch generiert */
}

/**************SUCHE_PASSENDE_PATCHES_IN_REKURSION***************************/
/*  Sucht zu einem Bordercode alle Patches, die sich mit den zum Bordercode
    gehoerenden Patches zusammennaehen lassen.                              */
/*  Falls ein brauchbarer Patch erzeugt wird, gibt die Funktion "True"
    zurueck, andernfalls "False".                                           */
/*  Es kann nicht passieren, dass Patch 2 den Patch 1 erreicht. Deshalb
    ist kein "ende"-Flag notwendig.                                         */
/*  krit = kritische Flaechen in Patch 1 und Patch 2 zusammen               */
/*  Die Reihenfolge, in der die Patches zugeordnet werden, ist anders als
    bei "suche_passende_patches", aber das ist egal, da ja alle in Frage
    kommenden Patches feststehen.                                           */

BOOL suche_passende_patches_rek_in_rekursion(KNOTENTYP *bordercode1,
     KNOTENTYP len1,KNOTENTYP fl1,TREENODE *root2,KNOTENTYP *bordercode2,
     KNOTENTYP len2,KNOTENTYP fl2,KNOTENTYP wh1,KNOTENTYP f_min,
     KNOTENTYP krit) {
  /* Variablen nicht static wegen Rekursion */
  KNOTENTYP flaechen_neu = fl1 + fl2 + (len1+len2)/(6-small_face);
  /* Flaechenzahl unter Beruecksichtigung der Eulerformel */
  /* genaugenommen len1+(len2+1)-1, da "len2" noch den alten Wert hat */
  /* len1+len2 ist die kleinstmoegliche Flaechendifferenz in den
     Ergebnispatches */
  if (flaechen_neu<f_max) {
    /* eigentlich "<= f_max", aber "flaechen_neu" ist um 1 niedriger, als es
       von der Bedeutung her sein muesste */ 
    while (root2) {                           /* einen Level durchgehen */
      bordercode2[len2++] = root2->code;
      if (root2->nextlevel) {
        if (suche_passende_patches_rek_in_rekursion(bordercode1,len1,fl1,
           root2->nextlevel,bordercode2,len2,fl2,wh1,f_min,krit))
           {return(True);}
      }
      if (root2->firstpatch) {         /* Patchpaar gefunden */
        memcpy(&bordercode2[len2],bordercode2,sizeof(KNOTENTYP)*(size_t)len2);
        memcpy(&bordercode2[len2<<1],bordercode2,sizeof(KNOTENTYP)*
              (size_t)len2);
        if (verknuepfe_patches_in_rekursion(bordercode1,len1,bordercode2,
            len2,fl1+fl2,f_min,wh1,root2->wh,krit)) {return(True);}
      }
      root2 = root2->next;
      len2--;
    }  /* while */
  }
  return(False);
}

BOOL suche_passende_patches_in_rekursion(KNOTENTYP *bordercode1,
     KNOTENTYP len1,KNOTENTYP fl1,KNOTENTYP wh1,KNOTENTYP f_min,
     KNOTENTYP krit) {
  KNOTENTYP bordercode2[3*N_MAX];
  KNOTENTYP i;
  TREENODE *t;
  for (i=1; i<=f_max1-fl1; i++) {   
    /* Falls krit<<1==krit_max: Nach Voraussetzung gilt f_max1-fl1 <= f_min
       < fl1. Deshalb ist als Returnwert auch nicht die Information darueber 
       notwendig, ob Patch 2 den Patch 1 erreicht hat. */
    t = tree[i];
    while (t && t->code<=krit_max-krit) {
      if (suche_passende_patches_rek_in_rekursion(bordercode1,len1,fl1,
          t->nextlevel,bordercode2,0,i,wh1,f_min,krit + t->code)) 
          {return(True);}           /* brauchbaren Patch gefunden */ 
      t = t->next;
    }
  }                             
  return(False);
}
      
/******************GEHE_PATCHBAUM_DURCH_IN_REKURSION************************/
/*   Loest dasselbe aus wie "gehe_patchbaum_durch", aber den Patch 1
     erhaelt man durch Rekursion anstatt durch systematisches Durchforsten
     des Baums.                                                            */
/*   Als Rueckgabewert liefert die Funktion "True", wenn von "root" aus
     ein brauchbarer Patch erzeugt werden kann, andernfalls "False".       */
/*   Im Gegensatz zur Funktion "gehe_patchbaum_durch" muss der Bordercode
     nicht aktualisiert werden, sondern ist bereits aktuell. Die Summe wird
     in der Funktion selbst ausgerechnet und ein TREENODE eingerichtet, der
     die Information ueber die Vorgaengerknoten enthaelt.                  */ 

BOOL gehe_patchbaum_durch_in_rekursion(KNOTENTYP *bordercode,
     KNOTENTYP len,KNOTENTYP fl,KNOTENTYP wh,KNOTENTYP f_min,KNOTENTYP krit) {
  memcpy(&bordercode[len],bordercode,sizeof(KNOTENTYP)*(size_t)len);
  memcpy(&bordercode[len<<1],bordercode,sizeof(KNOTENTYP)*(size_t)len);
  return(suche_passende_patches_in_rekursion(bordercode,len,fl,wh,f_min,krit));
}

/*********************VERKNUEPFE_PATCHES************************************/
/* Prueft, ob die uebergebenen Patches aus den Listen miteinander verbunden
   werden koennen (alle Patches einer Liste haben denselben Bordercode)    */
/* lenx = Laenge des Bordercodes x                                         */
/* Die Patches sind immer vom Typ 2 oder 3                                 */
/* l = Nahtlaenge (Anzahl identifizierter Kanten)                          */
/* fl = Anzahl Flaechen in den potentiellen Ergebnispatches                */
/* "f_min" ist die Mindestzahl an Flaechen in ALLEN Patches, die in
   Zukunft noch konstruiert werden (wird bei "gibt_es_gegenstueck" 
   gebraucht), minus 1.                                                    */

void verknuepfe_patches(TREENODE *liste1,KNOTENTYP *code1,KNOTENTYP len1,
     TREENODE *liste2,KNOTENTYP *code2,KNOTENTYP len2,KNOTENTYP fl,
     unsigned char expand,KNOTENTYP f_min,KNOTENTYP sum1,KNOTENTYP krit) {
  static EULERTYP ii;         /* Index fuer Verschiebung im Bordercode */
  static KNOTENTYP i,j;       /* Verschiebungen im Bordercode */
  static KNOTENTYP l;
  static KNOTENTYP ergcode[3*N_MAX];   /* laenger als N_MAX kann ein erlaubter
       Code nie werden, da pro Eintrag mindestens ein Knoten verbraucht wird */
  static KNOTENTYP wh;      /* wh!=0 => kanonische Verknuepfung gefunden */
  static BOOL bb;                /* True => Bauchbindenpatch entsteht */
  static KNOTENTYP erglen;
  static BOOL dummy;

  /* Es folgen die Tests auf gerade oder ungerade Verknuepfung: */
  /* Im folgenden kann man sich "if (anz[l][x]>0)"-Abfragen sparen. Falls die
   Bedingung nicht zutrifft, wird die zugehoerige Schleife nicht durchlaufen */
  for (j=0; j<liste2->wh; j++) {

    /* ungerade Verknuepfung testen: */
    if (code2[j]==0 && len2>2) { 
      l = (code2[j+1]<<1)+1;           /* anz[l][x] groesser ODER GLEICH 0 */

      if (len2==3) {
        for (ii=(EULERTYP)anz[l][1]-1; ii>=0; ii--) { /* Typ 1 => Bauchbinde */
          i = bind[l][1][ii];
          ergcode[0] = code1[i+len1-1] + code2[len2+j-1] + 2;
          dummy = verknuepfung(i,liste1,j,liste2,False,True,
                  fl,1,ergcode,1,1,f_min,l,sum1-l,False,krit,nil,nil);
        }
        for (ii=(EULERTYP)anz[l][3]-1; ii>=0; ii--) {  /* Typ 3 - Bauchbinde */
          i = bind[l][3][ii];
          ergcode[0] = code1[i+len1-1] + code2[len2+j-1] + 2;
          dummy = verknuepfung(j,liste2,i,liste1,True,True,fl,1,ergcode,1,1,
                  f_min,l,ermittle_borderlaenge(code2,len2)-l,False,krit,nil,
                  nil);
        }
      }

      for (ii=(EULERTYP)anz[l][0]-1; ii>=0; ii--) {    /* Typ 0 => keine BB */
        i = bind[l][0][ii];
        wh = verknuepfe_bordercodes(i+len1-1,i+len1+2,
             len2+j-1,len2+j+2,ergcode,&erglen,code1,code2,len1,len2,True);
        if (wh) {dummy = verknuepfung(i,liste1,j,liste2,False,False,fl,wh,
                         ergcode,erglen,1,f_min,l,sum1-l,False,krit,nil,nil);}
      }

      if (len2>3 && code2[len2+j-1]==0) {     /* Typ 2/3 - keine Bauchbinde */
        for (ii=(EULERTYP)anz[l][2]-1; ii>=0; ii--) {   /* Typ 2 => keine BB */
          i = bind[l][2][ii];
          wh = verknuepfe_bordercodes(len2+j-1,len2+j+2,i+len1-1,
               i+len1+2,ergcode,&erglen,code2,code1,len2,len1,True);
          if (wh) {dummy = verknuepfung(j,liste2,i,liste1,True,False,
                           fl,wh,ergcode,erglen,1,f_min,l,
                           ermittle_borderlaenge(code2,len2)-l,False,krit,
                           nil,nil);}
        }
        for (ii=(EULERTYP)anz[l][3]-1; ii>=0; ii--) {    /* Typ 3 - keine BB */
          i = bind[l][3][ii];
          wh = verknuepfe_bordercodes(len2+j-1,len2+j+2,i+len1-1,
               i+len1+2,ergcode,&erglen,code2,code1,len2,len1,True);
          if (wh) {dummy = verknuepfung(j,liste2,i,liste1,True,False,
                           fl,wh,ergcode,erglen,1,f_min,l,
                           ermittle_borderlaenge(code2,len2)-l,False,krit,
                           nil,nil);}
        }
      }    
    }       /* ungerade Verknuepfung testen */
           
    /* gerade Verknuepfung testen, wobei Patch 2 = links: */
    if (code2[j]==0 && code2[j+2]==0 && len2>3) {
      l = (code2[j+1]<<1)+2;
      
      if (len2==4) {
        for (ii=(EULERTYP)anz[l][3]-1; ii>=0; ii--) {  /* Typ 3 - Bauchbinde */
          i = bind[l][3][ii];
          ergcode[0] = code1[i+len1-1] + code2[len2+j-1] + 2;
          dummy = verknuepfung(j,liste2,i,liste1,True,True,fl,1,ergcode,1,0,
                  f_min,l,ermittle_borderlaenge(code2,len2)-l,False,krit,
                  code2,code1);
        }
      }

      if (len2>4 && code2[len2+j-1]==0) {     /* Typ 2/3 - keine Bauchbinde */
        for (ii=(EULERTYP)anz[l][2]-1; ii>=0; ii--) {  /* Typ 2 => keine BB */
          i = bind[l][2][ii];
          wh = verknuepfe_bordercodes(len2+j-1,len2+j+3,i+len1-1,
               i+len1+1,ergcode,&erglen,code2,code1,len2,len1,True);
          if (wh) {dummy = verknuepfung(j,liste2,i,liste1,True,False,
                           fl,wh,ergcode,erglen,0,f_min,l,
                           ermittle_borderlaenge(code2,len2)-l,False,krit,
                           nil,nil);}
        }
        for (ii=(EULERTYP)anz[l][3]-1; ii>=0; ii--) {   /* Typ 3 - keine BB */
          i = bind[l][3][ii];
          wh = verknuepfe_bordercodes(len2+j-1,len2+j+3,i+len1-1,
               i+len1+1,ergcode,&erglen,code2,code1,len2,len1,True);
          if (wh) {dummy = verknuepfung(j,liste2,i,liste1,True,False,
                           fl,wh,ergcode,erglen,0,f_min,l,
                           ermittle_borderlaenge(code2,len2)-l,False,krit,
                           nil,nil);}
        }
      }      
    }             /* gerade Verknuepfung mit Patch 2 links testen */
  
    /* gerade Verknuepfung testen, wobei Patch 2 = rechts: */
    if (len2>1) {
      l = code2[j]<<1;

      for (ii=(EULERTYP)anz[l][0]-1; ii>=0; ii--) {    /* Typ 0 => keine BB */
        i = bind[l][0][ii];
        wh = verknuepfe_bordercodes(i+len1-1,i+len1+3,
             len2+j-1,len2+j+1,ergcode,&erglen,code1,code2,len1,len2,True);
        if (wh) {dummy = verknuepfung(i,liste1,j,liste2,False,False,fl,wh,
                         ergcode,erglen,0,f_min,l,sum1-l,False,krit,nil,nil);}
      }

      if (len2==2) {
        for (ii=(EULERTYP)anz[l][1]-1; ii>=0; ii--) { /* Typ 1 => Bauchbinde */
          i = bind[l][1][ii];
          ergcode[0] = code1[i+len1-1] + code2[len2+j-1] + 2;
          dummy = verknuepfung(i,liste1,j,liste2,False,True,fl,1,ergcode,1,0,
                               f_min,l,sum1-l,False,krit,code1,code2);
        }
      }
    }           /* gerade Verknuepfung mit Patch 2 rechts testen */
  }     /* for j */

  /* Im folgenden werden die Einschluesse getestet, bei denen Patch 2 als
     Patch A fungiert. Diese Tests sind in einer eigenen Schleife 
     untergebracht, da meistens die ganze Schleife entfallen kann. */
  if (!PGA && (len1==1 || (len1==2 && code1[1]==0)) && len2>3) { 
    /* Patch 1 kann Patch B und Patch 2 kann Patch A sein */
    for (j=0; j<liste2->wh; j++) {

      if (len2>4 && code2[j]==0 && code2[j+2]==0 && code2[j+3]>code2[j+1]+1 &&
          anz[l = code2[j+3]-code2[j+1]-1][8]) {                   /* Typ 8 */
        wh = bilde_bordercodeE(ergcode,&erglen,code2,len2+j-1,
                               len2+j+4,len2,&bb);
        if (wh) {dummy = einschluss(j,liste2,liste1,bb,fl,wh,ergcode,erglen,2,
                         f_min,(code2[j+1]+1)<<1,(l<<1)+1,False,krit);}
      }

      if (code2[j]==0 && code2[j+2]>code2[j+1]+2
          && anz[l = code2[j+2]-code2[j+1]-2][9]) {                /* Typ 9 */
        wh = bilde_bordercodeE(ergcode,&erglen,code2,len2+j-1,
                               len2+j+3,len2,&bb);
        if (wh) {dummy = einschluss(j,liste2,liste1,bb,fl,wh,ergcode,erglen,3,
                         f_min,(code2[j+1]+1)<<1,(l<<1)+2,False,krit);}
      }

      if (len2>4 && code2[j]==0 && code2[j+2]==0 && code2[j+1]>code2[j+3] &&
          anz[l = code2[j+1]-code2[j+3]][8]) {                    /* Typ 8 */
        wh = bilde_bordercodeE(ergcode,&erglen,code2,len2+j-1,
                               len2+j+4,len2,&bb);
        if (wh) {dummy = einschluss(j,liste2,liste1,bb,fl,wh,ergcode,erglen,4,
                         f_min,(code2[j+3]<<1)+1,(l<<1)+1,False,krit);}
      }

      if (code2[j]==0 && code2[j+1]>code2[j+2]+1
          && anz[l = code2[j+1]-code2[j+2]-1][9]) {                /* Typ 9 */
        wh = bilde_bordercodeE(ergcode,&erglen,code2,len2+j-1,
                               len2+j+3,len2,&bb);
        if (wh) {dummy = einschluss(j,liste2,liste1,bb,fl,wh,ergcode,erglen,5,
                         f_min,(code2[j+2]<<1)+1,(l<<1)+2,False,krit);}
      }
    }    /* for j */
  }      /* Patch 1 als Patch B und Patch 2 als Patch A */
          
  /* Im folgenden werden die Einschluesse getestet, bei denen Patch 2 als 
     Patch B fungiert. Schleifen ueber j sind nicht notwendig, da die Drehung 
     von Patch 2 immer 0 sein muss. */
  if (!PGA && len1>3) { 
    l = code2[0];

    if (len2==1 && len1>4) {                   /* => Einschluss 1a oder 2a */
      for (ii=(EULERTYP)anz[l][4]-1; ii>=0; ii--) {         /* Typ 4 => 1a */
        i = bind[l][4][ii];
        wh = bilde_bordercodeE(ergcode,&erglen,code1,len1+i-1,len1+i+4,
                               len1,&bb);    /* wh ist hier immer > 0 */
        dummy = einschluss(i,liste1,liste2,bb,fl,wh,ergcode,erglen,2,f_min,
                           (code1[i+1]+1)<<1,(l<<1)+1,False,krit);
      }
      for (ii=(EULERTYP)anz[l][6]-1; ii>=0; ii--) {         /* Typ 6 => 2a */  
        i = bind[l][6][ii];
        wh = bilde_bordercodeE(ergcode,&erglen,code1,len1+i-1,
                               len1+i+4,len1,&bb);
        dummy = einschluss(i,liste1,liste2,bb,fl,wh,ergcode,erglen,4,f_min,
                           (code1[i+3]<<1)+1,(l<<1)+1,False,krit);
      }
    }        /* if (len2==1) */

    if (len2==2 && code2[1]==0) {  /* => Einschluss 1b oder 2b (len1>3 s.o.) */
      for (ii=(EULERTYP)anz[l][5]-1; ii>=0; ii--) {          /* Typ 5 => 1b */
         i = bind[l][5][ii];
        wh = bilde_bordercodeE(ergcode,&erglen,code1,len1+i-1,
                               len1+i+3,len1,&bb);
        dummy = einschluss(i,liste1,liste2,bb,fl,wh,ergcode,erglen,3,f_min,
                           (code1[i+1]+1)<<1,(l<<1)+2,False,krit);
      }
      for (ii=(EULERTYP)anz[l][7]-1; ii>=0; ii--) {          /* Typ 7 => 2b */
        i = bind[l][7][ii];
        wh = bilde_bordercodeE(ergcode,&erglen,code1,len1+i-1,
                               len1+i+3,len1,&bb);
        dummy = einschluss(i,liste1,liste2,bb,fl,wh,ergcode,erglen,5,f_min,
                           (code1[i+2]<<1)+1,(l<<1)+2,False,krit);
      }
    }        /* if (len2==2) */
  }          /* if len1>3 */

  if (expand==0) {return;}      /* kein Ueberlauf: fertig */

  /* Es folgen dieselben Tests nochmal, und zwar fuer den Fall, dass ein
     Ueberlauf aufgetreten ist. Wichtig ist, dass i gross genug ist, wenn
     eine Verknuepfung durchgefuehrt wird, denn sonst ist die Verknuepfung
     bereits oben vorgenommen worden. Deshalb laeuft i nicht von 0 bis len1. */
  /* Bei geraden und ungeraden Verknuepfungen koennen hier keine Bauchbinden
     entstehen, da hier kein Ueberlauf aufgetreten sein kann. */

  for (j=0; j<liste2->wh; j++) {

    /* ungerade Verknuepfung testen: */
    if (len2>2 && code2[j]==0) { 
      l = (code2[j+1]<<1)+1;           /* anz[l][x] groesser ODER GLEICH 0 */  

      if (anz[l][0]>=V_MAX) {           /* Ueberlauf aufgetreten */
        for (i=bind[l][0][V_MAX-1]+1; i<liste1->wh; i++) {         /* Typ 0 */
          if (code1[i]==0 && code1[i-1]==0 && code1[i+1]==code2[j+1]) {
            /* len1>3, da sonst kein Ueberlauf */   
            wh = verknuepfe_bordercodes(i+len1-1,i+len1+2,len2+j-1,len2+j+2,
                 ergcode,&erglen,code1,code2,len1,len2,True);
            if (wh) {dummy = verknuepfung(i,liste1,j,liste2,False,False,fl,
                             wh,ergcode,erglen,1,f_min,l,sum1-l,False,krit,
                             nil,nil);}
          }
        }
      }

      if (len2>3 && code2[len2+j-1]==0) {    /* Typ 2/3 - keine Bauchbinde */
        if (anz[l][2]>=V_MAX) {
          for (i=bind[l][2][V_MAX-1]+1; i<liste1->wh; i++) {       /* Typ 2 */
            if (code1[i]==0 && code1[i+1]==code2[j+1]) {
              /* len1>2, da sonst kein Ueberlauf */        
              wh = verknuepfe_bordercodes(len2+j-1,len2+j+2,i+len1-1,
                   i+len1+2,ergcode,&erglen,code2,code1,len2,len1,True);
              if (wh) {dummy = verknuepfung(j,liste2,i,liste1,True,False,
                               fl,wh,ergcode,erglen,1,f_min,l,
                               ermittle_borderlaenge(code2,len2)-l,False,
                               krit,nil,nil);}
            }
          }
        }
      }      
    }       /* ungerade Verknuepfung testen */
           
    /* gerade Verknuepfung testen, wobei Patch 2 = links: */
    if (len2>4 && code2[j]==0 && code2[j+2]==0 && code2[len2+j-1]==0) {
      l = (code2[j+1]<<1)+2;
 
      if (anz[l][2]>=V_MAX) {        
        for (i=bind[l][2][V_MAX-1]+1; i<liste1->wh; i++) {     /* Typ 2 */
          if (code1[i]==code2[j+1]+1) {
            /* len1>2, da sonst kein Ueberlauf */
            wh = verknuepfe_bordercodes(len2+j-1,len2+j+3,i+len1-1,i+len1+1,
                 ergcode,&erglen,code2,code1,len2,len1,True);
            if (wh) {dummy = verknuepfung(j,liste2,i,liste1,True,False,
                             fl,wh,ergcode,erglen,0,f_min,l,
                             ermittle_borderlaenge(code2,len2)-l,False,krit,
                             nil,nil);}
          }
        }
      }
    }             /* gerade Verknuepfung mit Patch 2 links testen */
  
    /* gerade Verknuepfung testen, wobei Patch 2 = rechts: */
    if (len2>1) {
      l = code2[j]<<1;
      
      if (anz[l][0]>=V_MAX) { 
        for (i=bind[l][0][V_MAX-1]+1; i<liste1->wh; i++) {     /* Typ 0 */
          if (code1[i]==0 && code1[i+2]==0 && code1[i-1]==0 && 
              code1[i+1]+1==code2[j]) {
             /* len1>4, da sonst kein Ueberlauf */ 
             wh = verknuepfe_bordercodes(i+len1-1,i+len1+3,len2+j-1,len2+j+1,
                  ergcode,&erglen,code1,code2,len1,len2,True);
             if (wh) {dummy = verknuepfung(i,liste1,j,liste2,False,False,fl,
                              wh,ergcode,erglen,0,f_min,l,sum1-l,False,krit,
                              nil,nil);}
          }
        }
      }
    }           /* gerade Verknuepfung mit Patch 2 rechts testen */
  }             /* for j */

  /* Im folgenden werden die Einschluesse getestet, bei denen Patch 2 als 
     Patch B fungiert. Schleifen ueber j sind nicht notwendig, da die Drehung 
     von Patch 2 immer 0 sein muss. */
  if (!PGA && len1>3) { 
    l = code2[0];

    if (len2==1 && len1>4) {           /* => Einschluss 1a oder 2a */
      if (anz[l][4]>=V_MAX) {          /* Einschluss 1a */
        for (i=bind[l][4][V_MAX-1]+1; i<liste1->wh; i++) {      /* Typ 4 */
          if ((len1==5 || code1[i-1]==0) && 
              code1[i]==0 && code1[i+2]==0 && code1[i+3]==code1[i+1]+1+l) { 
            wh = bilde_bordercodeE(ergcode,&erglen,code1,len1+i-1,
                                   len1+i+4,len1,&bb);
            if (wh) {dummy = einschluss(i,liste1,liste2,bb,fl,wh,ergcode,
                             erglen,2,f_min,(code1[i+1]+1)<<1,(l<<1)+1,False,
                             krit);}
          }
        }
      }
      
      if (anz[l][6]>=V_MAX) {          /* Einschluss 2a */
        for (i=bind[l][6][V_MAX-1]+1; i<liste1->wh; i++) {      /* Typ 6 */
          if ((len1==5 || code1[i-1]==0) && 
              code1[i]==0 && code1[i+2]==0 && code1[i+1]==code1[i+3]+l) { 
            wh = bilde_bordercodeE(ergcode,&erglen,code1,len1+i-1,
                                   len1+i+4,len1,&bb);
            if (wh) {dummy = einschluss(i,liste1,liste2,bb,fl,wh,ergcode,
                             erglen,4,f_min,(code1[i+3]<<1)+1,(l<<1)+1,False,
                             krit);}
          }
        }
      }
    }        /* if (len2==1) */

    if (len2==2 && code2[1]==0) {  /* => Einschluss 1b oder 2b (len1>3 s.o.) */
      if (anz[l][5]>=V_MAX) {
        for (i=bind[l][5][V_MAX-1]+1; i<liste1->wh; i++) {      /* Typ 5 */
          if ((len1==4 || code1[i-1]==0) && 
              code1[i]==0 && code1[i+2]==code1[i+1]+2+l) { 
            wh = bilde_bordercodeE(ergcode,&erglen,code1,len1+i-1,
                                   len1+i+3,len1,&bb);
            if (wh) {dummy = einschluss(i,liste1,liste2,bb,fl,wh,ergcode,
                             erglen,3,f_min,(code1[i+1]+1)<<1,(l<<1)+2,False,
                             krit);}
          }
        }
      }
  
      if (anz[l][7]>=V_MAX) {
        for (i=bind[l][7][V_MAX-1]+1; i<liste1->wh; i++) {      /* Typ 7 */
          if ((len1==4 || code1[i-1]==0) && 
              code1[i]==0 && code1[i+1]==code1[i+2]+1+l) { 
            wh = bilde_bordercodeE(ergcode,&erglen,code1,len1+i-1,
                                   len1+i+3,len1,&bb);
            if (wh) {dummy = einschluss(i,liste1,liste2,bb,fl,wh,ergcode,
                             erglen,5,f_min,(code1[i+2]<<1)+1,(l<<1)+2,False,
                             krit);}
          }
        }
      }
    }        /* if (len2==2) */
  }          /* if len1>3 */
}

/**********************GENERIERE_BIND_TABELLE********************************/
/*  Sucht alle moeglichen Verknuepfungen des Patches heraus, dessen Border-
    code uebergeben wird. Voraussetzung: alle anz-Elemente sind auf 0.      */
/*  Da V_MAX >= 4, kann in allen Faellen, wo eine Abfrage "if (len==x)"
    erfolgt, die Abfrage "if (anz<V_MAX)" entfallen. Es gilt naemlich
    4 >= x == len >= anz, so dass kein Ueberlauf auftreten kann.            */
/*  Sobald die Bedingung "if (anz<V_MAX)" einmal nicht zutrifft, wird in der
    Variablen "expand" ein Bit gesetzt, naemlich das dem Typ zugehoerige.
    Wichtig ist nur, dass anschliessend expand!=0 gilt.                     */
/*  An der Stelle, wo ein Ueberlauf auftritt, wird der zugehoerige anz-Wert
    auf V_MAX+1 gesetzt, auch wenn natuerlich nur V_MAX Arrayelemente besetzt
    sind, */
/*  Fuer die Faelle 4-7 (Patch A bei Einschluss) kann der resultierende
    Bordercode unabhaengig von Patch B bereits berechnet werden. Somit kann
    auch schon an dieser Stelle geprueft werden, ob der resultierende Code
    kanonisch ist. */

void generiere_bind_tabelle(KNOTENTYP *code,KNOTENTYP wh,KNOTENTYP len,
          unsigned char *expand) {
  static KNOTENTYP i,l;
  static BOOL bb;                       /* fuer Faelle 4-7 (dummy) */
  static KNOTENTYP ergcode[3*N_MAX];    /* fuer Faelle 4-7 (dummy) */
  static KNOTENTYP erglen;              /* fuer Faelle 4-7 (dummy) */
  *expand = 0;
  if (!PGA) {
    if (len==1) {anz[code[0]][8]++;}                              /* Typ 8 */
    if (len==2 && code[1]==0) {anz[code[0]][9]++;}                /* Typ 9 */
  }
  for (i=len; i<wh+len; i++) {   /* => i-1 ist immer noch >=0 */
    if (code[i]) {     /* Unterteilung lohnt, da code[i] immer wichtig ist */
      if (len>2) {                                              /* Typ 2 */ 
        l = code[i]<<1;
        if (anz[l][2]<V_MAX)  {bind[l][2][anz[l][2]++] = i-len;}
        else {*expand |= 4;}
      }
      if (len==2)                                               /* Typ 3 */
         {l = code[i]<<1;      bind[l][3][anz[l][3]++] = i-len;}
    }
    else {           /* code[i]==0 */
      if (len>4 && code[i+2]==0 && code[i-1]==0) {              /* Typ 0 */
        l = (code[i+1]<<1)+2; 
        if (anz[l][0]<V_MAX) {bind[l][0][anz[l][0]++] = i-len;}
        else {*expand |= 1;}
      }
      if (len==4 && code[i+2]==0)                               /* Typ 1 */
         {l = (code[i+1]<<1)+2;  bind[l][1][anz[l][1]++] = i-len;}
      if (len>3 && code[i-1]==0) {                              /* Typ 0 */
        l = (code[i+1]<<1)+1;
        if (anz[l][0]<V_MAX) {bind[l][0][anz[l][0]++] = i-len;}
        else {*expand |= 1;}
      }
      if (len==3)                                               /* Typ 1/3 */
         {l = (code[i+1]<<1)+1;  bind[l][1][anz[l][1]++] = i-len;
                                 bind[l][3][anz[l][3]++] = i-len;}
      if (len>3) {                                              /* Typ 2 */
        l = (code[i+1]<<1)+1;
        if (anz[l][2]<V_MAX) {bind[l][2][anz[l][2]++] = i-len;}
        else {*expand |= 4;}
      }
      if (!PGA) {
        if (len>4 && code[i+2]==0 && code[i+3]>code[i+1]+1 &&     /* Typ 4 */
            bilde_bordercodeE(ergcode,&erglen,code,i-1,i+4,len,&bb)) {
          l = code[i+3]-code[i+1]-1;
          if (anz[l][4]<V_MAX) {bind[l][4][anz[l][4]++] = i-len;}
          else {*expand |= 16;}
        }
        if (len>3 && code[i+2]>code[i+1]+2 &&                     /* Typ 5 */
            bilde_bordercodeE(ergcode,&erglen,code,i-1,i+3,len,&bb)) {  
          l = code[i+2]-code[i+1]-2;
          if (anz[l][5]<V_MAX) {bind[l][5][anz[l][5]++] = i-len;}
          else {*expand |= 32;}
        }
        if (len>4 && code[i+2]==0 && code[i+1]>code[i+3] &&       /* Typ 6 */
            bilde_bordercodeE(ergcode,&erglen,code,i-1,i+4,len,&bb)) {
          l = code[i+1]-code[i+3];
          if (anz[l][6]<V_MAX) {bind[l][6][anz[l][6]++] = i-len;}
          else {*expand |= 64;}
        }
        if (len>3 && code[i+1]>code[i+2]+1 &&                     /* Typ 7 */
            bilde_bordercodeE(ergcode,&erglen,code,i-1,i+3,len,&bb)) {
          l = code[i+1]-code[i+2]-1;
          if (anz[l][7]<V_MAX) {bind[l][7][anz[l][7]++] = i-len;}
          else {*expand |= 128;}
        }
      }
    }
  }      
} 

/**********************SUCHE_PASSENDE_PATCHES********************************/
/*  Sucht zu einem Bordercode alle Patches, die kleiner oder gleich sind
    (wobei als Ordnung die Reihenfolge, in der ein Patch im Baum gefunden
    wird, gilt) und sich mit den zum Bordercode gehoerenden Patches
    zusammennaehen lassen.                                                  */

BOOL suche_passende_patches_rek(TREENODE *root1,KNOTENTYP *bordercode1,
     KNOTENTYP len1,TREENODE *root2,KNOTENTYP *bordercode2,
     KNOTENTYP len2,KNOTENTYP fl,unsigned char expand,KNOTENTYP sum1,
     KNOTENTYP krit1,KNOTENTYP krit2,BOOL abbruch_mgl) {
  /* Variablen nicht static wegen Rekursion */
  BOOL ende = False; 
  KNOTENTYP flaechen_neu = fl + (len1+len2-6*PGA)/(6-small_face);
  /* Flaechenzahl unter Beruecksichtigung der Eulerformel */
  /* genaugenommen len1+(len2+1)-1, da "len2" noch den alten Wert hat */
  /* len1+len2 ist die kleinstmoegliche Flaechendifferenz in den
     Ergebnispatches */
  if (pv<1 || flaechen_neu<f_max) {
    /* eigentlich "<= f_max", aber "flaechen_neu" ist um 1 niedriger, als es
       von der Bedeutung her sein muesste */ 
    while (root2 && !ende) {                      /* einen Level durchgehen */
      bordercode2[len2++] = root2->code;
      if (root2->nextlevel) 
         {ende = suche_passende_patches_rek(root1,bordercode1,len1,
                 root2->nextlevel,bordercode2,len2,fl,expand,sum1,krit1,
                 krit2,abbruch_mgl);}
      if (!ende && root2->firstpatch)   /* Patchpaar gefunden */
        {memcpy(&bordercode2[len2],bordercode2,sizeof(KNOTENTYP)*(size_t)len2);
         memcpy(&bordercode2[len2<<1],bordercode2,
                sizeof(KNOTENTYP)*(size_t)len2);
         verknuepfe_patches(root1,bordercode1,len1,root2,bordercode2,
                   len2,fl,expand,fl-1,sum1,krit1+krit2);}
      if (root2==root1) {ende=True;} else {root2 = root2->next;}
      /* zweite Liste erreicht erste => alle gesuchten Codepaare gebildet */  
      len2--;
    }  /* while */
  }
  else {ende = (abbruch_mgl && len2<len1 && 
                memcmp(bordercode1,bordercode2,
                sizeof(KNOTENTYP)*(size_t)len2)==0);}
  return(ende);
}

void suche_passende_patches(TREENODE *root1,TREENODE *root2,KNOTENTYP
     *bordercode1,KNOTENTYP len1,KNOTENTYP fl,unsigned char expand,
     KNOTENTYP sum1,KNOTENTYP krit1,KNOTENTYP krit2,BOOL abbruch_mgl) {
  static KNOTENTYP bordercode2[3*N_MAX];
  static BOOL dummy;
  dummy = suche_passende_patches_rek(root1,bordercode1,len1,
          root2,bordercode2,0,fl,expand,sum1,krit1,krit2,abbruch_mgl);
}
      
/******************GEHE_PATCHBAUM_DURCH*************************************/
/*   Vorgehensweise:  Patch 1 ist immer der groessere, alle kleineren
     oder gleichen werden herausgesucht und zusammengenaeht                */
/*   fl = Flaechenzahl jedes Ergebnispatches                               */
/*   sum = Laenge der Umrandung                                            */
/*   krit1, krit2 = Anzahl der kritischen Flaechen in den Patches 1 und 2  */
/*   abbruch_mgl==True  =>  eventuell muss fuer Patch 2 nicht der gesamte
     Baum durchgegangen werden                                             */

void gehe_patchbaum_durch(TREENODE *root1,TREENODE *root2,
     KNOTENTYP *bordercode,KNOTENTYP len,KNOTENTYP fl,KNOTENTYP sum,
     KNOTENTYP krit1,KNOTENTYP krit2,BOOL abbruch_mgl) {
  KNOTENTYP i;
  unsigned char expand; 
  while (root1) {                                 /* einen Level durchgehen */
    bordercode[len] = root1->code;  len++;
    if (root1->nextlevel)
       {gehe_patchbaum_durch(root1->nextlevel,root2,bordercode,len,fl,
        sum+(root1->code<<1)+1,krit1,krit2,abbruch_mgl);}
    if (root1->firstpatch) { 
      memcpy(&bordercode[len],bordercode,sizeof(KNOTENTYP)*(size_t)len);
      memcpy(&bordercode[len<<1],bordercode,sizeof(KNOTENTYP)*(size_t)len);
      generiere_bind_tabelle(bordercode,root1->wh,len,&expand);
      suche_passende_patches(root1,root2,bordercode,len,fl,expand,
        sum+(root1->code<<1)+1,krit1,krit2,abbruch_mgl);
      for (i=((1+bordercode[0])<<1); i>0; i--)
        {anz[i][0] = anz[i][1] = anz[i][2] = anz[i][3] = anz[i][4] =
         anz[i][5] = anz[i][6] = anz[i][7] = anz[i][8] = anz[i][9] = 0;}
         /* 2*(bordercode[0]+1) ist hoechstmoegliche Nahtlaenge bzw. mehr als
            hoechstmoeglicher erster Bordercode-Eintrag (bei Einschluss) */ 
    }
    root1 = root1->next;  len--;
  }
}

/*********************GENERIERE_PATCHES*************************************/
/*   n:  Obergrenze fuer resultierende Graphen                             */ 
/*   Reihenfolge, in der die Patches generiert werden:
     1.  Kritische Punkte steigen monoton
     2.  Bei konstanter kritischer Punktzahl steigt die Flaechenzahl
         monoton
     3.  Bei konstanter kritischer Punktzahl und konstanter Flaechenzahl
         werden die kritischen Punkte aus 1. in zwei Summanden aufgeteilt
     4.  Anschliessend wird die Flaechenzahl aus 2. in zwei Summanden
         aufgeteilt
     =>  Bei einer festen kritischen Punktzahl und einer bestimmten
         Flaechenzahl stehen alle Patches mit kleinerer kritischer
         Punktzahl oder gleicher Punktzahl und kleinerer Flaechenzahl
         fest (3. und 4. koennten auch vertauscht werden).                 */
   
void generiere_patches(void) {
  static KNOTENTYP bordercode[3*N_MAX],i,j,fl1,fl2,fl;
  static EULERTYP krit1,krit2; 
  static KNOTENTYP *facearray;
  static TREENODE *t1,*t2;
 
  /* Ausgangsflaechen erzeugen: */
  for (i=0; i<anz_face; i++) {
    for (j=0; j<face[i]; j++) {bordercode[j]=0;}  /* Border des face[i]-Ecks */
    if (facenumbers) {
      facearray = (KNOTENTYP *)
                  hole_speicher((size_t)anz_face*sizeof(KNOTENTYP));
      for (j=0; j<anz_face; j++) {facearray[j] = 0;}
      facearray[i] = 1;    /* i-te Flaeche wurde erzeugt */
    }
    else {facearray = nil;}
    ordne_patch_in_baum(nil,nil,face[i],0,0,bordercode,face[i],1,1,
      False,facearray,0,0,(!alternative && face[i]>6) ? face[i]-6 : 0);
      /* falls alternative  =>  keine Trennung */
  }

  /* Patches zusammennaehen: */
  for (j=0; j<=krit_max; j++) {     /* 1.) Patches mit j kritischen Punkten */
    for (fl=2; fl<=f_max1; fl++) {    /* 2.) Patches mit fl Flaechen */
      for (krit2=(EULERTYP)j/2; krit2>=0; krit2--) {    /* 3.) */
        /* wichtig: krit1>=krit2 */
        krit1 = j-krit2;
        for (fl1=(krit1==krit2 ? (fl+1)>>1 : 1); fl1<fl; fl1++) {   /* 4.) */
          /* fl1 = Flaechen in Patch 1 (>=fl2, wenn krit1==krit2, damit Patch 1
                                        niemals kleiner als Patch 2 ist) */
          fl2 = fl-fl1;                 /* fl2 = Flaechen in Patch 2 */
          t1 = tree[fl1];
          while (t1 && t1->code<krit1) {t1 = t1->next;}
          if (t1 && t1->code==krit1) {
            t2 = tree[fl2];
            while (t2 && t2->code<krit2) {t2 = t2->next;}
            if (t2 && t2->code==krit2) { 
              /* es gibt Patches mit fl1 Flaechen und krit1 kritischen Punkten
                 sowie fl2 Flaechen und krit2 kritischen Punkten */
              {gehe_patchbaum_durch(t1->nextlevel,t2->nextlevel,bordercode,0,
                              fl,0,(KNOTENTYP)krit1,(KNOTENTYP)krit2,
                              krit1==krit2 && fl1==fl2);}
            }
          }
        }
      }
    }
  }
}   


/**************************************************************/
/* Sonstige Funktionen, die nur von "main" aufgerufen werden: */
/**************************************************************/

/**************************SORTIERE_FLAECHEN********************************/
/*  Sortiert das Array "face" in aufsteigender Reihenfolge                 */
/*  Die Arrays "facenum" und "facenum2" werden gleich mitsortiert          */

void sortiere_flaechen(FLAECHENTYP *face,FLAECHENTYP anz,KNOTENTYP *facenum,
                       KNOTENTYP *facenum2) {
  static FLAECHENTYP i,h;
  static KNOTENTYP hh;
  static BOOL getauscht;
  getauscht = True;              /* Schleife einmal durchlaufen */
  while (anz>1 && getauscht) {
    i = 1;   getauscht = False;
    while (i<anz) {
      if (face[i]<face[i-1]) {
        getauscht=True; 
        h = face[i];       face[i] = face[i-1];          face[i-1] = h;
        hh = facenum[i];   facenum[i] = facenum[i-1];    facenum[i-1] = hh;
        hh = facenum2[i];  facenum2[i] = facenum2[i-1];  facenum2[i-1] = hh;
      }
      i++;
    }
    anz--;    /* nun steht das groesste Element am Ende der Liste */
  }         
}

/***********ERRECHNE_KOMBINATIONEN**************************************/
/* Diese Funktion errechnet erlaubte Kombinationen von Flaechenzahlen.
   Sie arbeitet rekursiv. "i" ist der Index der aktuellen Flaeche.
   "fl", "anz_face", "facenum", "facenum_min", "facenum_max", "euler-
   facenum_min" und "euler_facenum_max" siehe "preufe_flaechen-
   kombination". "sum" ist die Summe der bisher genommenen Flaechen.   */

BOOL errechne_kombinationen(FLAECHENTYP i,FLAECHENTYP anz_face,KNOTENTYP sum,
     KNOTENTYP *facenum,KNOTENTYP *facenum_min,KNOTENTYP *facenum_max,
     KNOTENTYP fl,KNOTENTYP *euler_facenum_min,KNOTENTYP *euler_facenum_max) {
  FLAECHENTYP j;
  signed long eulersumme;
  KNOTENTYP kritische_punkte;   /* fuer Aufspaltung nach kritischen Fl. */
  BOOL erg = False;
  if (i+1==anz_face) {     /* Eulerformel berechnen */
    if (facenum_min[i]<=fl-sum && facenum_max[i]>=fl-sum) { 
      /* sonst unmoeglich */
      facenum[i] = fl-sum;   /* mit Flaechen des letzten Typs auffuellen */
      eulersumme = 0L;           /* Eulersumme berechnen */
      kritische_punkte = 0L;
      for (j=0; j<anz_face; j++) {
        eulersumme += (signed long)facenum[j]*(6-(signed long)face[j]);
        if (face[j]>6) {kritische_punkte += facenum[j]*(KNOTENTYP)(face[j]-6);}
      }
      if (eulersumme==12) {    /* Kombination gefunden */
        /* KNOTENTYP h;
           fprintf(stderr,"erlaubte Kombination: ");
           for (h=0; h<anz_face; h++) {fprintf(stderr,"%d ",facenum[h]);}
           fprintf(stderr,"\n"); */
        /* Grenzen aendern: */
        for (j=0; j<anz_face; j++) {
          if (facenum[j] < euler_facenum_min[j]) 
             {euler_facenum_min[j] = facenum[j];}
          if (facenum[j] > euler_facenum_max[j])
	     {euler_facenum_max[j] = facenum[j];}
          if (face[j]==6 && max6<facenum[j]) {max6 = facenum[j];}
        }
        if (!alternative) {    /* kritische Punkte angleichen */
          if (kritische_punkte < krit_min) {krit_min = kritische_punkte;}
          if (kritische_punkte > krit_max) {krit_max = kritische_punkte;}
        } 
        return(True);
      }
    }    
  }
  else {
    if (sum + facenum_min[i] <= fl) {
      sum += (facenum[i] = facenum_min[i]);
      while (sum<=fl && facenum[i]<=facenum_max[i]) {
        if (errechne_kombinationen(i+1,anz_face,sum,facenum,facenum_min,
            facenum_max,fl,euler_facenum_min,euler_facenum_max)==True)
           {erg = True;}  /* Moeglichkeit gefunden */
        facenum[i]++;  sum++;
      }
    }
  }       
  return(erg);
}

/*********************PRUEFE_FLAECHENKOMBINATION*****************************/
/* Diese Funktion prueft, ob die vom Benutzer gewuenschte Flaechenkombination
   ueberhaupt moeglich ist und wenn ja, ob sich irgendwelche Einschraenkungen
   an Flaechenzahlen eines bestimmten Typs ergeben.                         */
/* Falls ueberhaupt keine Graphen erzeugt werden koennen, gibt die Funktion
   "False" zurueck. Falls die Funktion "True" zurueckgibt, enthaelt das Array
   "moeglich" fuer jede Flaechenanzahl getrennt die Information, ob Graphen
   mit dieser Flaechenzahl produziert werden koennen. "moeglich[i]" enthaelt
   die Information fuer "i+anf" Flaechen.                                   */
/* "anf" und "end" sind die kleinste bzw. groesste gewuenschte FLAECHENzahl
   in den fertigen Graphen. "face", "facenum_min" und "facenum_max" zeigen
   auf die Arrays, in denen der Benutzer die gewuenschten Flaechen und deren
   Zahlen spezifiziert hat. Die Arrays enthalten jeweils "anz_face" Ein-
   traege. "face" ist aufsteigend sortiert.                                 */
/* Falls die Funktion eine Verbesserung erreichen konnte, so wird "change"
   auf True gesetzt.                                                        */

BOOL pruefe_flaechenkombination(KNOTENTYP anf,KNOTENTYP end,FLAECHENTYP *face,
     KNOTENTYP *facenum_min,KNOTENTYP *facenum_max,FLAECHENTYP anz_face,
     BOOL *change) {
  static KNOTENTYP trivial_facenum_min[MAXFTYPEN],
                   trivial_facenum_max[MAXFTYPEN]; /* fuer triviale Grenzen */
  static KNOTENTYP euler_facenum_min[MAXFTYPEN], euler_facenum_max[MAXFTYPEN];
                   /* durch Eulerformel ermittelte Grenzen */
  static KNOTENTYP facenum[MAXFTYPEN];   /* fuer konkrete Flaechenzahlen */
  static KNOTENTYP sum;                  /* Summe dieser Flaechenzahlen */
  static int i;               /* Index des betrachteten Flaechentyps */ 
  static KNOTENTYP fl;
  static FILE *logfile;

  /* Triviale Grenzen ermitteln: */
  logfile = fopen(logfilename,"a");
  fprintf(logfile,"\nTrivial intervals for face numbers (face/min/max):\n");
  fprintf(stderr,"\nTrivial intervals for face numbers (face/min/max):\n");
  for (i=0; i<(int)anz_face; i++) {
    /* kleinste Anzahl */
    if (face[i]<6 && dreiecke+vierecke+fuenfecke==1) { /* einzige kleine Fl. */
      trivial_facenum_min[i] = 12/(6-face[i]);
      if (12/(6-face[i])<anf && gr6>6) {trivial_facenum_min[i] += 
         (anf - 12/(6-face[i])) * (gr6-6) / (gr6-face[i]);}
    }
    else if (i+1==anz_face && face[i]>=6  /* => i>0 wegen small_face<6 */ && 
             face[i-1]<6 && anf>12/(6-kl6))
         /* einzige grosse Flaeche, und sie kommt mit Sicherheit hinzu */
      {trivial_facenum_min[i] = face[i]==6 ? anf - 12/(6-kl6) :
         (anf - 12/(6-kl6)) * (6-kl6) / (face[i]-kl6);}
    else {trivial_facenum_min[i] = 0;}
    
    /* groesste Anzahl */
    if (face[i]<6) {
      if (12/(6-face[i]) > end) {trivial_facenum_max[i] = end;}
      else if (big_face<=6)     {trivial_facenum_max[i] = 12/(6-face[i]);}
      else {trivial_facenum_max[i] = 12/(6-face[i]) + 
            (end - 12/(6-face[i])) * (big_face-6) / (big_face-face[i]);}
    } 
    else {trivial_facenum_max[i] = (end - 12/(6-small_face)) * 
          (6-small_face) / (face[i]-small_face);}

    fprintf(stderr,"%3d : %3d %3d\n",face[i],trivial_facenum_min[i],
            trivial_facenum_max[i]);
    fprintf(logfile,"%3d : %3d %3d\n",face[i],trivial_facenum_min[i],
            trivial_facenum_max[i]); 
  }
  fprintf(stderr,"\n");
  fprintf(logfile,"\n");
  fclose(logfile);

  /* Werden Zahlen verlangt, die die (trivialen) Grenzen sprengen? */
  for (i=0; i<(int)anz_face; i++) {
    if (facenum_min[i] > facenum_max[i]) {
      logfile = fopen(logfilename,"a");
      fprintf(stderr,"\nError: User defined %d-face number interval is "
              "empty!\n",face[i]);
      fprintf(logfile,"\nError: User defined %d-face number interval is "
              "empty!\n",face[i]);
      fclose(logfile);
      return(False);
    }
    else if (trivial_facenum_min[i] > trivial_facenum_max[i]) {
      logfile = fopen(logfilename,"a");
      fprintf(stderr,"\nNote: Face combination is impossible because trivial "
              "%d-face interval is empty!\n",face[i]);
      fprintf(logfile,"\nNote: Face combination is impossible because trivial "
              "%d-face interval is empty!\n",face[i]);
      fclose(logfile);
      return(False);
    }
    else if (facenum_min[i] > trivial_facenum_max[i]  ||
             facenum_max[i] < trivial_facenum_min[i]) {
      logfile = fopen(logfilename,"a");
      fprintf(stderr,"\nNote: User defined %d-face number interval exceeds\n"
              "      trivial face number interval!\n",face[i]);
      fprintf(logfile,"\nNote: User defined %d-face number interval exceeds\n"
              "      trivial face number interval!\n",face[i]);
      fclose(logfile);
      return(False);
    }
    else if (facenum_min[i] < trivial_facenum_min[i]  ||
             facenum_max[i] > trivial_facenum_max[i]) {
      if (facerestrict) {
        logfile = fopen(logfilename,"a");
        fprintf(stderr,"Note: Changed user defined %d-face number interval\n "
            "     corresponding to trivial face number interval.\n",face[i]);
        fprintf(logfile,"Note: Changed user defined %d-face number interval\n "
            "     corresponding to trivial face number interval.\n",face[i]);
        fclose(logfile);
      }
      if (facenum_min[i] < trivial_facenum_min[i]) 
         {facenum_min[i] = trivial_facenum_min[i];}
      if (facenum_max[i] > trivial_facenum_max[i])
   	 {facenum_max[i] = trivial_facenum_max[i];}
    }
  }
      
  /* nun konkrete Kombinationen ausrechnen: */ 
  for (i=0; i<anz_face; i++) {euler_facenum_min[i] = trivial_facenum_max[i];
                              euler_facenum_max[i] = trivial_facenum_min[i];}
  for (fl=anf; fl<=end; fl++) {
    if (is_moeglich[(fl<<1)-4]) {
      is_moeglich[(fl<<1)-4] = 
        errechne_kombinationen(0,anz_face,0,facenum,facenum_min,facenum_max,fl,
                               euler_facenum_min,euler_facenum_max);
      if (is_moeglich[(fl<<1)-4]==False) {
        logfile = fopen(logfilename,"a");
        fprintf(stderr,"Result by Euler formula: No graphs with %d"
                  " vertices possible!\n",(fl<<1)-4);
          fprintf(logfile,"Result by Euler formula: No graphs with %d"
                  " vertices possible!\n",(fl<<1)-4);
          fclose(logfile);
          *change = True;
      }
    }
  }  

  /* obere Grenze der Patcherzeugung einschraenken: */
  fl = end;
  while (fl>=anf && !is_moeglich[(fl<<1)-4]) {fl--;}
  if (fl<anf) {          /* gar keine Knotenzahl passt */
    logfile = fopen(logfilename,"a");
    fprintf(logfile,"Result computed with Euler formula:\n"
                    "No graphs with the given face number(s) and the given "
                    "vertex number(s) exist!\n");
    fprintf(stderr,"Result computed with Euler formula:\n"
                   "No graphs with the given face number(s) and the given "
                   "vertex number(s) exist!\n");
    fclose(logfile);
    return(False);
  }
  else if (fl<end) {     /* Einschraenkung erreicht */
    /* Erzeugungsgrenzen neu festlegen */
    *change  = True;
    end = (fl<<1)-4;              /* nun ist "end" eine Knotenzahl */
    n_max = 2*end-3;              /* theoretische Obergrenze fuer Patches */
    n_max_bb = end;               /* theoretische Obergrenze fuer BB-Patches */
    f_max = end/2 + 2;            /* reale Obergrenze fuer Graphen */
    f_min1 = (dreiecke ? 2 : (vierecke ? 3 : 6));
    f_max1 = f_max-f_min1;  
  }

  /*  Flaechenzahlen an die durch die Eulerformel errechneten Zahlen anpassen.
      Ob die Zahlen auch genutzt werden, haengt davon ab, ob der Benutzer 
      Flaechenzahlen eingeschraenkt hat oder nicht, also ob 
      facerestrict==True.  */
  for (i=0; i<anz_face; i++) {
    if (euler_facenum_min[i] > facenum_min[i] ||
        euler_facenum_max[i] < facenum_max[i]) {
      logfile = fopen(logfilename,"a");
      fprintf(stderr,"Note: Face number interval for %d-faces can be "
            "reduced\n      from [%d,%d] to [%d,%d].\n",face[i],facenum_min[i],
            facenum_max[i],euler_facenum_min[i],euler_facenum_max[i]);
      fprintf(logfile,"Note: Face number interval for %d-faces can be "
            "reduced\n      from [%d,%d] to [%d,%d].\n",face[i],facenum_min[i],
            facenum_max[i],euler_facenum_min[i],euler_facenum_max[i]);
      fclose(logfile);
      if (euler_facenum_min[i] > facenum_min[i]) 
         {facenum_min[i] = euler_facenum_min[i];  *change = True;}
      if (euler_facenum_max[i] < facenum_max[i]) 
         {facenum_max[i] = euler_facenum_max[i];  *change = True;}
    }
  }
  return(True);
}

/******************PRUEFE_GRUENBAUM_SAETZE***********************************/
/* Diese Funktion prueft, ob der Benutzer Flaechenkombinationen aufgerufen
   hat, die laut Gruenbaum (siehe Buch "Convex Polytopes") nicht realisierbar
   sind.                                                                    */
/* "anf" und "end" sind die kleinste bzw. groesste gewuenschte FLAECHENzahl
   in den fertigen Graphen. "face", "facenum_min" und "facenum_max" zeigen
   auf die Arrays, in denen der Benutzer die gewuenschten Flaechen und deren
   Zahlen spezifiziert hat. Die Arrays enthalten jeweils "anz_face" Ein-
   traege. "face" ist aufsteigend sortiert.                                 */

BOOL pruefe_gruenbaum_saetze(KNOTENTYP anf,KNOTENTYP end,FLAECHENTYP *face,
     KNOTENTYP *facenum_min,KNOTENTYP *facenum_max,FLAECHENTYP *anz_face,
     BOOL *change) {
  FLAECHENTYP k,j,i;
  KNOTENTYP fl, minsum, maxsum;
  static FILE *logfile;
  
  /*  Satz 13.4.2(k):  */
  for (k=2; k<=5; k++) {
    minsum = maxsum = 0;            /* Summen fuer n bei G(k,n) */
    for (i=0; i<*anz_face; i++) {
      if (face[i]%k!=0) {minsum += facenum_min[i];  maxsum += facenum_max[i];}
    }
    if (maxsum==1) {                /* unmoeglich */
      if (minsum==1) {              /* nur unmoegliche Kombinationen */
        logfile = fopen(logfilename,"a");
        fprintf(stderr,"Result given by Gruenbaum: No graphs possible!\n");
        fprintf(logfile,"Result given by Gruenbaum: No graphs possible!\n");
        fclose(logfile);
        return(False);
      }
      else {       /* minsum==0  =>  einschraenken */
        i=0;
        while (i<*anz_face) {
          if (face[i]%k!=0 && facenum_max[i]==1) {   /* Flaeche gefunden */
            logfile = fopen(logfilename,"a");
            fprintf(stderr,"Note: %d-faces will not occur (result by Gruenbaum)"
                    ".\n",face[i]);
            fprintf(logfile,"Note: %d-faces will not occur (result by "
                    "Gruenbaum).\n",face[i]);
            fclose(logfile);
            /* Flaeche aus Array entfernen: */
            if (face[i]==3) {dreiecke = False;}
            if (face[i]==4) {vierecke = False;}
            if (face[i]==5) {fuenfecke = False;}
            if (face[i]==6) {sechsecke = False;}
            while (i<*anz_face-1) {
              face[i] = face[i+1];  facenum_min[i] = facenum_min[i+1];
              facenum_max[i] = facenum_max[i+1];     i++;
            }
            (*anz_face)--;
            /* Flaechenvariablen neu setzen und pruefen: */
            if (*anz_face==0) {
              logfile = fopen(logfilename,"a");
              fprintf(stderr,"No faces left.\n");
              fprintf(logfile,"No faces left.\n");
              fclose(logfile);
              return(False);
            }
            small_face = face[0];
            big_face = face[*anz_face-1];
            if (small_face>5) {
              logfile = fopen(logfilename,"a");
              fprintf(stderr,"Error: no face type smaller than a hexagon.\n");
              fprintf(logfile,"Error: no face type smaller than a hexagon.\n");
              fclose(logfile);
              return(False);
            }
            j = 0;
            while (j<*anz_face && face[j]<6) {j++;}
            kl6 = face[j-1];
            if (j==*anz_face) {g6 = gr6 = 0;}
            else if (sechsecke) {
              gr6 = 6;  
              if (j+1<*anz_face) {g6 = face[j+1];} else {g6 = 0;} 
            }
            else {g6 = gr6 = face[j];}
            *change = True;
          }
          else {i++;}
        }
      }
    }
  }

  /*  Satz 13.4.4(k):  */
  for (fl=anf; fl<=end; fl++) {
    if (is_moeglich[(fl<<1)-4]) {    /* noch kein Widerspruch entdeckt */ 
      for (k=3; k<=5; k++) {
        minsum = maxsum = 0;            /* Summen fuer n bei G(k,n) */
        for (i=0; i<*anz_face; i++) {
          if (face[i]%k!=0) 
            {minsum += facenum_min[i];  maxsum += facenum_max[i];}
        }
        if ((maxsum==0 && ((k==3 && fl%2!=0) || (k==4 && fl%4!=2) || 
            (k==5 && fl%10!=2))) || (maxsum==2 && k==3 && fl%2!=0)) {
          /* man beachte: falls maxsum==2, so gilt entweder:
             minsum==2  =>  Satz 13.4.4(3) nicht erfuellt,  oder:
             minsum==1  =>  Satz 13.4.2(3) nicht erfuellt,  oder:
             minsum==0  =>  Satz 13.4.4(3) nicht erfuellt */
          logfile = fopen(logfilename,"a");
          fprintf(stderr,"Result given by Gruenbaum: No graphs with %d"
                  " vertices possible!\n",(fl<<1)-4);
          fprintf(logfile,"Result given by Gruenbaum: No graphs with %d"
                  " vertices possible!\n",(fl<<1)-4);
          fclose(logfile);
          is_moeglich[(fl<<1)-4] = False;
          *change = True;
        }
      }
    }
  }      
    
  /* obere Grenze der Patcherzeugung einschraenken
     (wie bei "pruefe_flaechenkombination"): */
  fl = end;
  while (fl>=anf && !is_moeglich[(fl<<1)-4]) {fl--;}
  if (fl<anf) {          /* gar keine Knotenzahl passt */
    logfile = fopen(logfilename,"a");
    fprintf(logfile,"Result according to Gruenbaum:\n"
                    "No graphs with the given face number(s) and the given "
                    "vertex number(s) exist!\n");
    fprintf(stderr,"Result according to Gruenbaum:\n"
                   "No graphs with the given face number(s) and the given "
                   "vertex number(s) exist!\n");
    fclose(logfile);
    return(False);
  }
  else if (fl<end) {     /* Einschraenkung erreicht */
    /* Erzeugungsgrenzen neu festlegen */
    end = (fl<<1)-4;              /* nun ist "end" eine Knotenzahl */
    n_max = 2*end-3;              /* theoretische Obergrenze fuer Patches */
    n_max_bb = end;               /* theoretische Obergrenze fuer BB-Patches */
    f_max = end/2 + 2;            /* reale Obergrenze fuer Graphen */
    f_min1 = (dreiecke ? 2 : (vierecke ? 3 : 6));
    f_max1 = f_max-f_min1;
    *change = True;  
  }
  return(True);
}

 
/*************************SCHREIBE_PATCH2*************************************/
/*  Diese Funktion kann bei Bedarf in das Programm eingebaut werden.
    Sie dient Testzwecken. Sie schreibt alle Patches mit "fl" Flaechen
    nach stdout. Aufgerufen werden muss sie mit ausreichend Speicherplatz
    an der Stelle "bordercode" und "len==0" sowie "root==tree[fl]"           */

void schreibe_patch2(TREENODE *root,KNOTENTYP *bordercode,KNOTENTYP len,
                    KNOTENTYP fl) {
  static unsigned char planarcode_c[CODESIZE(N_MAX)];
  static KNOTENTYP i,j;
  static ELEM *e;
  static PLANMAP m;
  static KANTE *k;
  static KNOTENTYP patch[CODESIZE(N_MAX)];
  static KANTENARRAY2 map;
  static KNOTENTYP n;

  while (root) {                         /* einen Level durchgehen */
    bordercode[len] = root->code;  len++;
    if (root->nextlevel)
       {schreibe_patch2(root->nextlevel,bordercode,len,fl);}
    if (e = root->firstpatch) { 
      while (e) {
        k = konstruiere_patch(map,bordercode,len,e);
        n = ermittle_val_2_knoten(bordercode,len)+2*fl-2;
        numeriere_graph(m,k);
        map_2_planarcode(m,patch,n);
        j = 0;
        for (i=0; i<CODESIZE(n); i++) {
          if (patch[i]!=aussen) 
             {planarcode_c[j++] = (unsigned char)(patch[i]);}
        }
        fwrite(planarcode_c,sizeof(unsigned char),(size_t)j,stdout);
        e = e->next;
      }
    }
    root = root->next;  len--;
  }
}
 
    
/******************************MAIN*****************************************/

void main(int argc,char *argv[]) {
  int i,j;
  clock_t savetime=0, buffertime;
  struct tms TMS;
  int l=0;                  /* >0 => l-ter Parameter ist expliziter Filename */
  BOOL standardout = False;   /* True => Graphen mit hoechster Knotenzahl auf
                                 stdout ausgeben */
  BOOL standardout_all = False;  /* True => alle Graphen auf stdout ausgeben */
  BOOL unknown_option = False;  /* True => unbekannte Option entdeckt */
  BOOL patchstat = False;       /* True => entsprechende Option benutzt */
  int prio = 0;                 /* !=0  => Option "priority" benutzt */
  char strpuf[10];               /* fuer Zahlenstrings */   
  char filenamenteil[FILENAMENLAENGE];   /* gemeinsamer Teil aller Filenamen */
  char *fileindex = nil;         /* fuer individuelle Filenamen */
  FILE *logfile;
  BOOL change;                   /* fuer Vorabpruefungen */

  /* Wurden die Konstanten unzulaessig manipuliert? */
  if (N_MAX>KN_MAX)           /* N_MAX wurde manipuliert */
    {fprintf(stderr,"Internal error: Constant N_MAX too high!\n"); exit(10);}
  if (F_MAX>FL_MAX || F_MAX>N_MAX)       /* F_MAX wurde manipuliert */
    {fprintf(stderr,"Internal error: Constant F_MAX too high!\n"); exit(11);}

  /* Argumente auswerten */
  if (argc<2) {fprintf(stderr,"Usage: see manual.\n"); exit(12);}
  for (i=1; i<argc; i++) {
    switch (argv[i][0]) {
      case 'n': {
        if (strcmp(argv[i],(char *)"n")==0) 
          {n_end = ++i<argc ? atoi(argv[i]) : 0;}       /* letzte Knotenzahl */
        else if (strcmp(argv[i],(char *)"no_output")==0)
          {output = False;} 
        else if (strcmp(argv[i],(char *)"no_1")==0)
          {do_bauchbinde = False;}
        else if (strcmp(argv[i],(char *)"no_2")==0)
          {do_sandwich = False;}
        else if (strcmp(argv[i],(char *)"no_3")==0)
          {do_brille = False;}
        else {unknown_option = True;}
        break;
      }
      case 'c': {
        if (strcmp(argv[i],(char *)"con")==0) {
          if (++i<argc) {
            if (strcmp(argv[i],(char *)"1")==0)      {do_conn = conn1 = True;}
            else if (strcmp(argv[i],(char *)"2")==0) {do_conn = conn2 = True;}
            else if (strcmp(argv[i],(char *)"3")==0) {do_conn = conn3 = True;}
            else {
              fprintf(stderr,"Error: Invalid argument after option 'con'!\n");
              exit(31);
            }
          }
          else {
            fprintf(stderr,"Error: Missing argument after option 'con'!\n");
            exit(32);
          }
        }
        else {unknown_option = True;}
        break;
      }
      case 's': {
        if (strcmp(argv[i],(char *)"s")==0)
          {n_anf = ++i<argc ? atoi(argv[i]) : 0;}       /* erste Knotenzahl */
        else if (strcmp(argv[i],(char *)"stdout_max")==0) 
          {standardout = True;}
        else if (strcmp(argv[i],(char *)"stdout")==0) 
          {standardout_all = True;}
        else {unknown_option = True;}
        break;
      }
      case 'f': {
        if (strcmp(argv[i],(char *)"f")==0) {        /* Flaechentyp */
          if (anz_face<MAXFTYPEN) { 
            if (++i<argc && atoi(argv[i])>=3 && atoi(argv[i])<=F_MAX) {
              face[anz_face]=atoi(argv[i]);
              if (face[anz_face]==3) {dreiecke = True;}
              if (face[anz_face]==4) {vierecke = True;}
              if (face[anz_face]==5) {fuenfecke = True;}
              if (face[anz_face]==6) {sechsecke = True;}
              if (face[anz_face]>=6 && (gr6==0 || gr6>face[anz_face]))
		 {gr6 = face[anz_face];}
              if (face[anz_face]>6 && (g6==0 || g6>face[anz_face]))
		 {g6 = face[anz_face];}
            }
            else {fprintf(stderr,"Error: Face size %d exceeds interval [3,%d]"
                          "!\n",i<argc ? atoi(argv[i]) : 0,F_MAX); exit(13);}
            for (j=0; j<anz_face; j++) {
              if (face[j]==face[anz_face])
                {fprintf(stderr,"Error: Face %d defined twice!\n",
                 face[anz_face]); exit(14);}
            }
            facenum_max[anz_face] = F_MAXANZ;
            facenum_min[anz_face] = 0;         /* falls facenumbers==True
                  durch eine andere Flaeche oder durch "facestat" und fuer die
                  korrekte Erstellung des Filenamens erforderlich */
            if (i+1<argc && (argv[i+1][0]=='-' || argv[i+1][0]=='+')) {
               /* Minimalzahl oder Maximalzahl einlesen */
              if (argv[i+1][0]=='+')      /* Minimalzahl */
                   {facenum_min[anz_face] = atoi(&argv[++i][1]);}
              else {facenum_max[anz_face] = atoi(&argv[++i][1]);}
              facenumbers = True;  facerestrict = True;
              if (i+1<argc && (argv[i+1][0]=='-' || argv[i+1][0]=='+')) {
                 /* noch 'ne Minimalzahl oder Maximalzahl einlesen */
                if (argv[i+1][0]=='+')      /* Minimalzahl */
                     {facenum_min[anz_face] = atoi(&argv[++i][1]);}
                else {facenum_max[anz_face] = atoi(&argv[++i][1]);}
              }
            }
            if (facenum_min[anz_face] > facenum_max[anz_face]) 
	      {fprintf(stderr,"Error: Minimum face number is bigger than "
                              "maximum number!\n");  exit(15);}
            anz_face++;
          }
          else {
            fprintf(stderr,"Error: Number of face types exceeds maximum %d!\n",
            MAXFTYPEN); exit(16);
          }
        }
        else if (strcmp(argv[i],(char *)"fileindex")==0) {   /* Fileindex */
          if (++i<argc) {fileindex = argv[i];}
          else {fprintf(stderr,"Error: Missing file index!\n"); exit(17);}
        }
        else if (strcmp(argv[i],(char *)"facestat")==0) 
          {facestat = facenumbers = True;}
          /* Flaechenstatistik erstellen => Flaechenzahlen speichern */  
        else {unknown_option = True;}
        break;
      }              /* case f */
      case 'p': {    /* Flaeche, an der jeder Pfad liegen muss */
        if (strcmp(argv[i],(char *)"pv")==0) {
          if (++i<argc) {pv = (char)atoi(argv[i]);}
          else {fprintf(stderr,"Error: Missing pv parameter!\n"); exit(30);}
        } 
        else if (strcmp(argv[i],(char *)"patchstat")==0) {patchstat = True;}
        else if (strcmp(argv[i],(char *)"pid")==0) 
          {fprintf(stdout,"%d\n",getpid());  fflush(stdout);}
        else if (strcmp(argv[i],(char *)"priority")==0) {
          if (++i<argc) {
            switch (prio = atoi(argv[i])) {
	      case 123:  {bauchbindenkennung=1;  sandwichkennung=2;
                          brillenkennung=3;      break;}
	      case 132:  {bauchbindenkennung=1;  sandwichkennung=3;
                          brillenkennung=2;      break;}
	      case 213:  {bauchbindenkennung=2;  sandwichkennung=1;
                          brillenkennung=3;      break;}
	      case 231:  {bauchbindenkennung=3;  sandwichkennung=1;
                          brillenkennung=2;      break;}
	      case 312:  {bauchbindenkennung=2;  sandwichkennung=3;
                          brillenkennung=1;      break;}
	      case 321:  {bauchbindenkennung=3;  sandwichkennung=2;
                          brillenkennung=1;      break;}
              default:   {fprintf(stderr,"Error: Invalid priority %d!\n",
                          atoi(argv[i]));  exit(18);}
	    }
          }
          else {fprintf(stderr,"Error: Missing priority!\n"); exit(19);}
        }
        else if (strcmp(argv[i],(char *)"patchcon")==0) 
          {patchconn = True;}            /* Zusammenhangszahl bei Patches */
        else {unknown_option = True;}
        break;
      }          
      case 'm': {
        if (strcmp(argv[i],(char *)"maxfsize")==0) {
          if (++i<argc) {
            big_face = atoi(argv[i]);
            if (big_face<5) {
              fprintf(stderr,"Error: Invalid argument after option "
                             "'maxfsize'!\n");  exit(43);
            }
          }
          else {
            fprintf(stderr,"Error: Missing argument after option 'con'!\n");
            exit(42);
          }
        }
        else {unknown_option = True;}
        break;
      }
      case 'P': {    /* p3-maximale Pseudogeradenarrangements */
        if (strcmp(argv[i],(char *)"PLA")==0) {PGA = True;}
        else {unknown_option = True;}
        break;
      }
      case 'a': {    /* Alternative Patcherzeugung ohne kritische Flaeche */
        if (strcmp(argv[i],(char *)"alt")==0) {alternative = True;}
        else {unknown_option = True;}
        break;
      }
      case 'd': {    /* Duale Ausgabe => Triangulierungen */
        if (strcmp(argv[i],(char *)"dual")==0) {dual = True;}
        else {unknown_option = True;}
        break;
      }
      case 'b': {    /* Barnettes Vermutung fuer C3CBP-Graphen */
        if (strcmp(argv[i],(char *)"barnette")==0) {barnette = True;}
        else {unknown_option = True;}
        break;
      }
      case 'l': {    /* eigener Name fuer das Logfile */
        if (strcmp(argv[i],(char *)"l")==0) {
          if (++i<argc && strlen(argv[i])<=FILENAMENLAENGE) {l=i;}
          else {fprintf(stderr,"Error option l: missing filename or filename"
                        " too long!\n");  exit(20);}
        }
        else {unknown_option = True;}    
        break;
      }
      default: {unknown_option = True; break;}
    }     /* switch */
    if (unknown_option) {fprintf(stderr,"Error: Unknown option %s!\n",argv[i]);
                         exit(21);}
  }       /* for */


  /* Argumente ueberpruefen */
  if (n_anf==0) {n_anf=n_end;}
  if (n_end==0) {fprintf(stderr,"Error: Missing parameter n!\n");  exit(22);}

  if (PGA) {      /* p3-maximale Pseudogeradenarrangements erzeugen */
    if (do_bauchbinde==False || do_sandwich==False || do_brille==False ||
        do_conn || anz_face>0 || pv!=PV_DEFAULT || prio!=0 ||
        patchconn || alternative || barnette || facestat || dual)  
      /* nicht erlaubte Option benutzt */
      {fprintf(stderr,"Error using option 'PLA': Invalid option used"
                      " simultaneously!\n");  exit(34);}
    if (n_end%2 || n_anf%2) 
       {fprintf(stderr,"Error: Pseudoline number is odd!\n"); exit(35);}
    if (n_anf<6)
       {fprintf(stderr,"Error: Pseudoline number to start is too small!\n");
        exit(36);}
    if (n_end%6!=0 && (n_end+2)%6!=0) 
       {fprintf(stderr,"Error: Invalid pseudoline number to end!\n");
        exit(37);}
    if (n_anf%6!=0 && (n_anf+2)%6!=0) 
       {fprintf(stderr,"Error: Invalid pseudoline number to start!\n");
        exit(38);}
    if (n_end>MIN(MAXFTYPEN+4,F_MAX)) 
       {fprintf(stderr,"Error: Pseudoline number to end exceeds maximum"
                " (%d)!\n",MIN(MAXFTYPEN+4,F_MAX));  exit(39);}
    n_max_bb = n_end*(n_end-1)/3 + n_end-1;   /* Obergrenze fuer BB-Patches */
    n_max = n_max_bb-2;    /* bei jeder Verklebung kommen mindestens 2 neue
                              Knoten hinzu, da kein Einschluss und da keine
                              Drei- und Vierecke erlaubt sind */
    f_max = f_max1 = n_end*(n_end-1)/6 + 1;    /* reale Obergrenze fuer PGAs */

    /* immer gesetzte Optionen: */
    alternative = True;
    do_conn = True;   conn1 = False;   conn2 = False;   conn3 = True;
    patchconn = True;
    if (big_face==0) {big_face=(FLAECHENTYP)n_end;}
    for (i=5; i<=big_face; i++) {     /* Flaechen festlegen */
      face[i-5] = i; 
      facenum_max[i-5] = F_MAXANZ;
      facenum_min[i-5] = 0;    
    }
    anz_face = big_face-4;
    small_face = 5;   fuenfecke = True;
    kl6 = 5;  g6 = big_face>6 ? big_face : 0;
    gr6 = big_face>=6 ? 6 : 0;
    sechsecke = (gr6==6);
  }
  else {              /* S3PZ--Graphen erzeugen */
    if (big_face>0) {fprintf(stderr,"Error using option 'maxfsize': This "
                     "option is allowed only together with 'PLA'!\n"); 
                     exit(41);}
    if (n_end%2 || n_anf%2) 
       {fprintf(stderr,"Error: Vertex number is odd!\n"); exit(23);}
    if (n_anf<4)
       {fprintf(stderr,"Error: Vertex number to start is too small!\n");
        exit(24);}
    n_max = 2*n_end-3;            /* theoretische Obergrenze fuer Patches */
    n_max_bb = n_end;             /* theoretische Obergrenze fuer BB-Patches */
    f_max = n_end/2 + 2;          /* reale Obergrenze fuer Graphen */
    f_anf = n_anf/2 + 2;          /* reale Untergrenze fuer Graphen */
    f_min1 = (dreiecke ? 2 : (vierecke ? 3 : 6));
    f_max1 = f_max-f_min1;  
             /* der andere BB-Patch hat mindestens f_min1 Flaechen */

    if (anz_face==0) 
       {fprintf(stderr,"Error: No face types specified!\n"); exit(27);}  
    sortiere_flaechen(face,anz_face,facenum_min,facenum_max);
    big_face = face[anz_face-1];
    small_face = face[0];
    if (small_face > 5) 
      {fprintf(stderr,"Error: No face type smaller than a hexagon!\n"); 
       exit(28);}
    if (barnette) {  /* nur bipartite Graphen erlaubt => nur gerade Flaechen */
      for (j=0; j<(int)anz_face; j++) {
        if (face[j]%2==1) {
          fprintf(stderr,"Error using option 'barnette':"
                         " This option is only for bipartite graphs!\n");
          exit(33);
        }
      }
    }
    kl6 = fuenfecke ? 5 : (vierecke ? 4 : 3);
    if (!do_conn) {conn1 = conn2 = conn3 = True;}
  }
       
  if (n_max>N_MAX)
     {fprintf(stderr,"Error: Vertex number to end is too big (maximum %d)!\n",
              N_MAX); exit(25);}
  if (f_max>F_MAXANZ)
     {fprintf(stderr,"Error: Face number %d is too big (maximum %d)!\n",
              f_max,F_MAXANZ); exit(26);}
  if (facestat) {
    if (anz_face>MAXFTYPEN_FSTAT) 
       {fprintf(stderr,"Error using option 'facestat':"
                       " Too many face types specified!\n");  exit(29);} 
    facestatarray = (unsigned int *)
		    hole_speicher(sizeof(unsigned int)*(1<<anz_face));
    for (j=0; j<(1<<anz_face); j++) {facestatarray[j]=0;}
  }

  for (i=n_anf; i<=n_end; i+=2) {  /* Achtung: Bedeutung von i unterschiedlich
                                  fuer S3PZ und PGA */
    graphenzahl[i>>1][0] = non_iso_graphenzahl[i>>1][0] = connzahl[i>>1][0] =  
    graphenzahl[i>>1][1] = non_iso_graphenzahl[i>>1][1] = connzahl[i>>1][1] = 
    graphenzahl[i>>1][2] = non_iso_graphenzahl[i>>1][2] = connzahl[i>>1][2] 
    = 0;
  }
  
  /* gemeinsamen Teil aller Filenamen erstellen: */
  filenamenteil[0] = 0;     /* leeren */
  if (PGA) {
    if (big_face!=n_end) {
      sprintf(strpuf,"_f%d-%d",small_face,big_face);
      strcat(filenamenteil,strpuf);
    }
  }
  else {
    for (i=0; i<MIN(12,anz_face); i++) {
      sprintf(strpuf,"_f%d",face[i]);
      strcat(filenamenteil,strpuf);
      if (facenum_min[i]>0) {
        sprintf(strpuf,"+%d",facenum_min[i]);
        strcat(filenamenteil,strpuf);
      }
      if (facenum_max[i]<F_MAXANZ) {
        sprintf(strpuf,"-%d",facenum_max[i]);
        strcat(filenamenteil,strpuf);
      }
    }
    if (!do_bauchbinde || !do_sandwich || !do_brille) {
      strcat(filenamenteil,(char *)"_t");
      if (do_bauchbinde) {strcat(filenamenteil,(char *)"1");}
      if (do_sandwich)   {strcat(filenamenteil,(char *)"2");}
      if (do_brille)     {strcat(filenamenteil,(char *)"3");}
    }
    if (prio)         {sprintf(strpuf,"_p%d",prio);
                       strcat(filenamenteil,strpuf);}
    if (barnette)     {strcat(filenamenteil,(char *)"_barn");}
    if (alternative)  {strcat(filenamenteil,(char *)"_alt");}
    if (!conn1 || !conn2 || !conn3) {
      strcat(filenamenteil,(char *)"_c");
      if (conn1) {strcat(filenamenteil,(char *)"1");}
      if (conn2) {strcat(filenamenteil,(char *)"2");}
      if (conn3) {strcat(filenamenteil,(char *)"3");}
    }
    if (patchconn) {strcat(filenamenteil,(char *)"_pac");}
  }
  if (dual) {strcat(filenamenteil,(char *)"_dual");}
  if (fileindex)    {strcat(filenamenteil,(char *)"_");
                     strcat(filenamenteil,fileindex);}
  if (pv<PV_DEFAULT) {sprintf(strpuf,"_pv%d",pv);
                      strcat(filenamenteil,strpuf);}

  /* Outputfiles oeffnen: */
  for (j=0; j<=N_MAX; j+=2) {outputfile[j>>1] = nil;}   
      /* um zu kennzeichnen, welche Files nicht geschlossen werden muessen */
  if (output) {
    for (j=(int)n_anf; j<=(int)n_end; j+=2) {
      if (!PGA || (j-2)%6!=0) {    /* sonst keine PGAs moeglich */
        if (PGA) {sprintf(logfilename,"PLA_n%d",j);}
        else     {sprintf(logfilename,"3reg_n%d",j);}
        strcat(logfilename,filenamenteil);
        strcat(logfilename,(char *)".plc");
        if ((j==(int)n_end && standardout) || standardout_all) 
           {outputfile[j>>1] = stdout;}
        else {
          if ((outputfile[j>>1] = fopen(logfilename,"w"))==nil)
            {fprintf(stderr,"Cannot open file %s!\n",logfilename); schluss(6);}
          fprintf(outputfile[j>>1],">>planar_code %s<<",
                  ENDIAN_OUT==BIG_ENDIAN ? (char *)"be" : (char *)"le");
        }
      }
    }   /* for j */
    if (standardout || standardout_all)
       {fprintf(stdout,">>planar_code %s<<",
                ENDIAN_OUT==BIG_ENDIAN ? (char *)"be" : (char *)"le");}
       /* nur einmal fuer alle stdout-Kanaele, da sonst Ausgabe unbrauchbar
          wuerde */
  }     /* if output */

  /* Logfile oeffnen und erstellen: */
  if (!l) {     /* Logfilenamen ermitteln */
    if (PGA) {
      if (n_anf!=n_end) {sprintf(logfilename,"PLA_n%d_s%d",n_end,n_anf);}
      else          {sprintf(logfilename,"PLA_n%d",n_end);}
    }
    else {
      if (n_anf!=n_end) {sprintf(logfilename,"3reg_n%d_s%d",n_end,n_anf);}
      else          {sprintf(logfilename,"3reg_n%d",n_end);}
    }
    strcat(logfilename,filenamenteil);
    strcat(logfilename,(char *)".log");    
  }
  else {strcpy(logfilename,argv[l]);}
  /* Ab hier darf der Inhalt von "logfilename" nicht mehr veraendert werden,
     da das Logfile einige Male geoeffnet und geschlossen werden wird. */

  logfile = fopen(logfilename,"w");
  if (logfile==nil) {fprintf(stderr,"Cannot open file %s!\n",logfilename);
                     schluss(7);}
  fprintf(stderr,"Program call:  ");
  for (i=0; i<argc; i++) {fprintf(stderr,"%s ",argv[i]);}
  fprintf(stderr,"\n\n");
  fprintf(logfile,"Program call:  ");
  for (i=0; i<argc; i++) {fprintf(logfile,"%s ",argv[i]);}
  fprintf(logfile,"\n\n");
  fprintf(logfile,"Minimal %s number: %d\n",PGA ? (char *)"pseudoline" :
                  (char *)"vertex",n_anf);
  fprintf(logfile,"Maximal %s number: %d\n",PGA ? (char *)"pseudoline" :
                  (char *)"vertex",n_end);
  fprintf(logfile,"Face types to use: ");
  for (i=0; i<anz_face; i++) {fprintf(logfile,"%d ",face[i]);}
  fprintf(logfile,"\n");
  fclose(logfile);
  fprintf(stderr,"Minimal %s number: %d\n",PGA ? (char *)"pseudoline" :
                  (char *)"vertex",n_anf);
  fprintf(stderr,"Maximal %s number: %d\n",PGA ? (char *)"pseudoline" :
                  (char *)"vertex",n_end);
  fprintf(stderr,"Face types to use: ");
  for (i=0; i<anz_face; i++) {fprintf(stderr,"%d ",face[i]);}
  fprintf(stderr,"\n");

  /* entscheiden, ob normale oder alternative Patcherzeugung: */
  if (big_face<7 || pv<10) {alternative = True;}

  /* Flaechenkombination vorpruefen: */ 
  if (alternative) {krit_min = 0;          krit_max = 0;}
  else             {krit_min = USHRT_MAX;  krit_max = 0;}             
  initialisiere_is_moeglich();
  
  if (PGA) {
    for (j=n_anf; j<=n_end; j++) 
      {if ((j-2)%6==0 || j%2==1) {is_moeglich[j]=False;} }
  }
  else {  
    do {
      if (pruefe_flaechenkombination(f_anf,f_max,face,
          facenum_min,facenum_max,anz_face,&change)==False) {schluss(0);}
      change = False;
      if (pruefe_gruenbaum_saetze(f_anf,f_max,face,
          facenum_min,facenum_max,&anz_face,&change)==False) {schluss(0);}
      if (change==True) {     /* nochmal Euler-Vorpruefung anwenden */
        logfile = fopen(logfilename,"a");
        fprintf(stderr,"\nOne more pass:\n");
        fprintf(logfile,"\nOne more pass:\n");
        fclose(logfile);
      }
    } while (change==True);

    fprintf(stderr,"\nIntervals for face numbers (face/min/max):\n");
    for (i=0; i<anz_face; i++)
      {fprintf(stderr,"%3d : %3d %3d\n",face[i],facenum_min[i],
                      facenum_max[i]);}
    fprintf(stderr,"\n");   
    logfile = fopen(logfilename,"a");
    fprintf(logfile,"\nIntervals for face numbers (face/min/max):\n");
    for (i=0; i<anz_face; i++)
      {fprintf(logfile,"%3d : %3d %3d\n",face[i],facenum_min[i],
                       facenum_max[i]);}
    fprintf(logfile,"\n");   
    fclose(logfile);
 
    if (do_bauchbinde && bauchbindenkennung==3) {  
        /* Bauchbinde niedrigwertig => kann Bauchbinde vergessen werden? */
      do_bauchbinde = False;     /* sofern kein Widerspruch eintritt */
      for (j=(int)n_anf; !do_bauchbinde && j<=(int)n_end; j+=2) {
        if (j%4==0 && is_moeglich[j]) {do_bauchbinde = True;}
           /* es gibt theoretisch Bauchbindengraphen */
      }
      if (do_bauchbinde==False) {
        fprintf(stderr,"\nGood news: No type-1-patches need to be "
                "generated.\n");
        fprintf(logfile,"\nGood news: No type-1-patches need to be "
                "generated.\n");
      }
    }

    if (do_bauchbinde) {
      for (j=(int)n_anf; j<=(int)n_end; j+=2) {
        if (is_moeglich[j]) {   /* evtl. neues Maximum fuer "f_max_bb_best" */ 
          if (bauchbindenkennung<3 || j%4==0) 
             {f_max_bb_best = (KNOTENTYP)(j/2+2);}
              /* bestbewertete BB mit j Knoten moeglich */
        }
      }
    }
  }


  /* Jetzt geht's los: */
  /* Patches erzeugen */
  initialisiere_patchbaeume();
  initialisiere_anz_array();
  generiere_patches();
  times(&TMS);
  savetime = TMS.tms_utime;
  fprintf(stderr,"Time for generating the patches: %.1f seconds\n",
          (double)savetime/time_factor);
  space_info = mallinfo();
  fprintf(stderr,"Memory consumption for storing the patches: %ld bytes\n",
          (long)space_info.usmblks+space_info.uordblks);
  if (PGA) {
    fprintf(stderr,"Number of patches: %ld\n",patches23);
  }
  else {  
    fprintf(stderr,"Number of type-1-patches: %ld\n",patches1);
    fprintf(stderr,"Number of other patches: %ld\n",patches23);
  }
  logfile = fopen(logfilename,"a");
  fprintf(logfile,"Time for generating the patches: %.1f seconds\n",
          (double)savetime/time_factor);
  fprintf(logfile,"Memory consumption for storing the patches: %ld bytes\n",
          (long)space_info.usmblks+space_info.uordblks);
  if (PGA) {
    fprintf(logfile,"Number of patches: %ld\n",patches23);
  }
  else {  
    fprintf(logfile,"Number of type-1-patches: %ld\n",patches1);
    fprintf(logfile,"Number of other patches: %ld\n",patches23);
  }

  if (patchstat) {
    fprintf(stderr,"\nPatches sorted by groups:");
    fprintf(logfile,"\nPatches sorted by groups:");
    for (i=0; i<(PGA ? 2 : 6); i++) {
      fprintf(stderr,"  %ld",nahttyp[i]);
      fprintf(logfile,"  %ld",nahttyp[i]);
    }

    fprintf(logfile,"\n\nPatches sorted by faces (#faces/#type1/#type23):\n");
    fprintf(stderr,"\n\nPatches sorted by faces (#faces/#type1/#type23):\n");
    for (i=1; i<=f_max1; i++) {
      fprintf(logfile,"%3d : %6d  %6d\n",i,fl1_anz[i],fl2_anz[i]);
      fprintf(stderr,"%3d : %6d  %6d\n",i,fl1_anz[i],fl2_anz[i]);
    }
    fprintf(stderr,"\n");
    fprintf(logfile,"\n");
  }
  fclose(logfile);

  if (PGA) {
    for (i=n_anf; i<=n_end; i++) {
      if (is_moeglich[i]) {
        fprintf(stderr,"Wrote %d p3-maximal PLAs with %d pseudolines.\n",
                       graphenzahl[i>>1][0],i);
        fprintf(logfile,"Wrote %d p3-maximal PLAs with %d pseudolines.\n",
                        graphenzahl[i>>1][0],i);
      }
    }
    schluss(0);
  }                  /* hier ENDE fuer PGA */

  /* Graphen erzeugen */
  for (j=(int)n_anf; j<=(int)n_end; j+=2) {
    if (is_moeglich[j]) {     /* es gibt theoretisch Graphen */
      if (do_bauchbinde && (bauchbindenkennung<3 || j%4==0)) {
        /* 2.Bedingung falsch => es kann keine idealen BB-Patches geben */ 
        bilde_graphen_aus_bauchbindenpatches((KNOTENTYP)j);
        times(&TMS);
        buffertime = TMS.tms_utime;
        fprintf(stderr,"Generated %d type-1-graphs with %d vertices in %.1f"
                " seconds, took %d.\n",graphenzahl[j>>1][0],j,
                (double)(buffertime-savetime)/time_factor,
                non_iso_graphenzahl[j>>1][0]);
        logfile = fopen(logfilename,"a");
        fprintf(logfile,"Generated %d type-1-graphs with %d vertices in"
                " %.1f seconds, took %d.\n",graphenzahl[j>>1][0],j,
                (double)(buffertime-savetime)/time_factor,
                non_iso_graphenzahl[j>>1][0]);
        fclose(logfile);
        savetime = buffertime;
      }
      if (do_sandwich) {
        bilde_graphen_aus_sandwichpatches((KNOTENTYP)j);
        times(&TMS);
        buffertime = TMS.tms_utime; 
        fprintf(stderr,"Generated %d type-2-graphs with %d vertices in %.1f"
                " seconds, took %d.\n",graphenzahl[j>>1][1],j,
                (double)(buffertime-savetime)/time_factor,
                non_iso_graphenzahl[j>>1][1]);
        logfile = fopen(logfilename,"a");
        fprintf(logfile,"Generated %d type-2-graphs with %d vertices in"
                " %.1f seconds, took %d.\n",graphenzahl[j>>1][1],j,
                (double)(buffertime-savetime)/time_factor,
                non_iso_graphenzahl[j>>1][1]);
        fclose(logfile);
        savetime = buffertime;
      }
      if (do_brille) {
        bilde_graphen_aus_brillenpatches((KNOTENTYP)j);
        times(&TMS);
        buffertime = TMS.tms_utime; 
        fprintf(stderr,"Generated %d type-3-graphs with %d vertices in %.1f"
                " seconds, took %d.\n",graphenzahl[j>>1][2],j,
                (double)(buffertime-savetime)/time_factor,
                non_iso_graphenzahl[j>>1][2]);
        logfile = fopen(logfilename,"a");
        fprintf(logfile,"Generated %d type-3-graphs with %d vertices in"
                " %.1f seconds, took %d.\n",graphenzahl[j>>1][2],j,
                (double)(buffertime-savetime)/time_factor,
                non_iso_graphenzahl[j>>1][2]);
        fclose(logfile);
        savetime = buffertime;
      }
    }
    else {     /* keine Graphen mit j Knoten moeglich */
      fprintf(stderr,"Euler and Gruenbaum formulas: No graphs with %d vertices"
                     " can be generated.\n",j);
      logfile = fopen(logfilename,"a");
      fprintf(logfile,"Euler and Gruenbaum formulas: No graphs with %d "
                      "vertices can be generated.\n",j);
      fclose(logfile);
    }
  }    /* for j */

  logfile = fopen(logfilename,"a");
  for (j=(int)n_anf; j<=(int)n_end; j+=2) {
    fprintf(stderr,"\nTotal: Generated %d non-isomorphic graphs with %d "
      "vertices.",
       non_iso_graphenzahl[j>>1][0] + non_iso_graphenzahl[j>>1][1] +
       non_iso_graphenzahl[j>>1][2], j);
    fprintf(logfile,"\nTotal: Generated %d non-isomorphic graphs with %d "
      "vertices.",
       non_iso_graphenzahl[j>>1][0] + non_iso_graphenzahl[j>>1][1] +
       non_iso_graphenzahl[j>>1][2], j);
    if (do_conn) {
      if (patchconn) {   /* falls patchconn==True, dann nur die gewuenschten
                       Faelle anzeigen, denn die anderen Faelle sind eh falsch
                       dadurch, dass Patches nicht beruecksichtigt wurden */
        fprintf(logfile,"\n(");  fprintf(stderr,"\n(");
        if (conn1) {fprintf(stderr,"1-connected: %d   ",connzahl[j>>1][0]);
                    fprintf(logfile,"1-connected: %d   ",connzahl[j>>1][0]);}
        if (conn2) {fprintf(stderr,"2-connected: %d   ",connzahl[j>>1][1]);
                    fprintf(logfile,"2-connected: %d   ",connzahl[j>>1][1]);}
        if (conn3) {fprintf(stderr,"3-connected: %d",connzahl[j>>1][2]);
                    fprintf(logfile,"3-connected: %d",connzahl[j>>1][2]);}
        fprintf(logfile,")");    fprintf(stderr,")");
      }
      else {             /* alle Faelle anzeigen, denn alle sind korrekt */
        fprintf(stderr,"\n(1-connected: %d   2-connected: %d   "
                       "3-connected: %d)",
          connzahl[j>>1][0],connzahl[j>>1][1],connzahl[j>>1][2]);
        fprintf(logfile,"\n(1-connected: %d   2-connected: %d "
          "  3-connected: %d)",connzahl[j>>1][0],connzahl[j>>1][1],
          connzahl[j>>1][2]);
      }
    }    
  }
  fprintf(stderr,"\nTotal generation time: %.1f seconds.\n",
          (double)buffertime/time_factor);
  fprintf(logfile,"\nTotal generation time: %.1f seconds.\n",
          (double)buffertime/time_factor);

  if (facestat) {       /* Flaechenstatistik */
    fprintf(stderr,"\nGraphs with certain face types:\n");
    fprintf(logfile,"\nGraphs with certain face types:\n");
    for (j=0; j<(1<<anz_face); j++) {
      if (facestatarray[j]) {    /* Zahl ungleich 0 ausgeben */
        for (i=0; i<anz_face; i++) {
          if ((j>>i)&1) {fprintf(stderr,"%2d ",face[i]);
                         fprintf(logfile,"%2d ",face[i]);}
          else          {fprintf(stderr,"   ");  fprintf(logfile,"   ");}
        }
        fprintf(stderr," : %5d\n",facestatarray[j]);
        fprintf(logfile," : %5d\n",facestatarray[j]);
      }
    }
  }
  
  if (pv!=20) {
    fprintf(logfile,"Used patches: %d\n",gute_basen);
    fprintf(stderr,"Used patches: %d\n",gute_basen);
  }

  /* fprintf(logfile,"Unmittelbar benutzte Patches: %d\n",unmittelbar); */
  /* fprintf(logfile,"Gute Patches: %d\n",gute);
     fprintf(logfile,"Gute + Basen: %d\n",gute_basen);
     fprintf(logfile,"Kurze: %d\n",kurze);
     fprintf(stderr,"Unmittelbar benutzte Patches: %d\n",unmittelbar); */
 /*  fprintf(stderr,"Gute Patches: %d\n",gute);
     fprintf(stderr,"Gute + Basen: %d\n",gute_basen);
     fprintf(stderr,"Kurze: %d\n",kurze); */
  
  fclose(logfile);
  schluss(0);
}


/******************************************************************/
/* Anhang: Tests, die vielleicht manchmal eingebaut werden sollen */
/******************************************************************/

/* hier sollen alle Patches mit 10 Flaechen ausgegeben werden, und zwar
   ohne Aussenkanten */
/*  KNOTENTYP code[3*N_MAX],len=0;
    schreibe_patch(tree[10]->nextlevel,code,len,10);
*/

/* der auszugebende Patch findet sich hier bei "root->firstpatch" */
/*     if (gib_einen_patch_aus) {
          ELEM *e;
          KANTE *k1;
          KNOTENTYP j=0,n;
          KNOTENTYP graph[CODESIZE(N_MAX)];
          FLAECHENTYP pfl1;
          PLANMAP m;
          KANTENARRAY2 patchmap1;
          e = root->firstpatch;
          while (e) {
            j++; 
            k1 = konstruiere_patch(patchmap1,bordercode,len,e,&pfl1);
            numeriere_graph(m,k1);
            map_2_planarcode(m,graph,n = ermittle_val_2_knoten(bordercode,len)
                             +2*fl-2);
            schreibe_planarcode(graph,stdout);
            e=e->next;
          }
       }
*/






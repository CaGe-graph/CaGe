/******************************HORLOGE.C*******************************/

#include "horloge.h"
#include <stdlib.h>

/*---------------------------------------------------------------------*\

Module:       horloge.c

Date:         12.04.97

Auteur:       G. CAPOROSSI

Description: Fonctions d'allocation et utilisation de l'horloge

\*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*\
Fonction:            AllocHorloge

Parametres Entree: Horloge *clock

Parametres Sortie:  int 1 initialisation reussie
                        0 si problemes de memoire

Description:        alloue la memoire d'une horloge

\*---------------------------------------------------------------------*/

Horloge *
AllocHorloge(void)
{
	Horloge *clock;

	/* allocation de la memoire de l'horloge */
	clock = (Horloge *) malloc(sizeof(Horloge));
	clock->ucpu = 0.0;
	clock->scpu = 0.0;
	clock->clock_ticks = sysconf(_SC_CLK_TCK);
	clock->time_struct = (struct tms *) malloc(sizeof(struct tms));

	if (!clock->time_struct) {
		fprintf(stderr,"Allocation Error in Horloge.");
		exit(0);
	}
	return(clock);
}

/*---------------------------------------------------------------------*\
Fonction:            InitHorloge

Parametres Entree: Horloge *clock

Parametres Sortie:  int 1 initialisation reussie
                        0 si problemes de memoire

Description:        initialise une horloge (debut a l'heure courante)

\*---------------------------------------------------------------------*/

int
InitHorloge(Horloge *clock)
{
	if(clock && clock->time_struct) {
		times(clock->time_struct);
		clock->start_utime = clock->time_struct->tms_utime;
		clock->start_stime = clock->time_struct->tms_stime;
		clock->ucpu = 0.0;
		clock->scpu = 0.0;
	} else {
		printf("InitHorloge: Unallocated structure!!\n");
		exit(0);
	}
	return(1);
}

/*---------------------------------------------------------------------*\
Fonction:            EvalHorloge

Parametres Entree: Horloge *clock

Parametres Sortie:  int 1 initialisation reussie
                        0 si problemes de memoire

Description:        met a jour l'horloge

\*---------------------------------------------------------------------*/

int
EvalHorloge(Horloge *clock)
{
	if (!clock || !clock->time_struct) {
		fprintf(stderr, "Clock not allocated\n");
		return 0;
	}

	times(clock->time_struct);
	clock->lap_utime = clock->time_struct->tms_utime;
	clock->lap_stime = clock->time_struct->tms_stime;
	clock->ucpu = (double) (clock->lap_utime-clock->start_utime) / (double) clock->clock_ticks;
	clock->scpu = (double) (clock->lap_stime-clock->start_stime) / (double) clock->clock_ticks;

	return(1);
}

/*---------------------------------------------------------------------*\
Fonction:            PrintHorloge

Parametres Entree: Horloge *clock
                   FILE    *out

Parametres Sortie:  int 1 initialisation reussie
                        0 si problemes de memoire

Description:        affiche les temps cpu, sys et reel

\*---------------------------------------------------------------------*/


int
PrintHorloge(FILE *out,Horloge *clock)
{
	if (clock->ucpu > 1e-6) {
		fprintf(out,"\tCPU Time:    %.2f\n",clock->ucpu);
		fprintf(out,"\tSystem Time: %.2f\n",clock->scpu);
		//fprintf(out,"\tReal time: %.2f\n",clock->dure);
	} else {
		fprintf(out,"\tCPU Time:    Too small to calculate\n");
		fprintf(out,"\tSystem time: Too small to calculate\n");
		//fprintf(out,"\tReal Time: Too small to calculate\n");
	}

	return(0);
}


/*---------------------------------------------------------------------*\
Fonction:            PrintCPU

Parametres Entree: Horloge *clock
                   FILE    *out

Parametres Sortie:  int 1 initialisation reussie
                        0 si problemes de memoire

Description:        affiche les temps cpu,

\*---------------------------------------------------------------------*/


int
PrintCPU(FILE *out,Horloge *clock)
{
	if(clock && clock->ucpu > 1e-6)
		fprintf(out," CPU Time: %f\n",clock->ucpu);
	else
		fprintf(out," (CPU Time NA)\n");

	return(0);
}

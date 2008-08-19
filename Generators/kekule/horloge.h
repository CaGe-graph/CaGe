/**************************** HORLOGE.H   *******************************/

/* Sorry to use the french word horloge, but the aim is to avoid
   conflicts with clock */

#include <sys/times.h>
#include <stdio.h>
#include <unistd.h>

typedef struct horloge {
	clock_t		start_utime;
	clock_t		start_stime;
	clock_t		lap_utime;
	clock_t		lap_stime;
	struct tms	*time_struct;
	//int		is_started;
	clock_t		clock_ticks;
	double		ucpu, scpu;
} Horloge;

/* allocation of the structure */
extern Horloge *AllocHorloge(void);

/* initialisation (start of count ) */
extern int InitHorloge(Horloge *clock);

/* compute elapsed time since InitHorloge */
extern int EvalHorloge(Horloge *clock);

/* print time to file *out : CPU, sys et reel */
extern int PrintHorloge(FILE *out, Horloge *clock);

/* print CPU time to file *out */
extern int PrintCPU(FILE *out, Horloge *clock);

/* gives CPU time *
extern double getCPU(Horloge *clock);
 gives system CPU time
extern double getSCPU(Horloge *clock);*/

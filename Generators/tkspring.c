/*
 *	tkspring.c		Fruchterman/Reingold type spring embedder
 *				for graphs. Works best with planar maps.
 *
 *	Based on a program by Egon Zakrajsek, Ljubljana 1994
 *
 *	ODF (Olaf Delgado Friedrichs) 1998/03/25
 */


#ifndef USE_TCL
#define USE_TCL 1		/* if 0, graphics output is disabled	*/
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#if USE_TCL
#include<tcl.h>
#include<tk.h>
#endif /*USE_TCL*/


#define MAX_LINE_LENGTH 3000


struct point {
  double	x;
  double	y;
  double	z;
};


struct edge {
  struct point p;
  struct point q;
  int shade;
};


struct vertex {			   /* vertex */
  struct point  saved_pos;
  struct point	pos;
  struct point	disp;
  int valency;
  int *adj;
  int fixed;
};

int iter [3];			/* max iteration counts			*/
int gstep[3];			/* flags for graphics output		*/
int vstep[3];			/* flags for informational output	*/

int flat = 0;
double bestangle = 2.0 / 3.0 * M_PI;

double maxdist = 1000.0;
int frames_begin = 0;
int frames_end = 0;
int maxstep = 0;
int print_frame_count = 0;
int count_only = 0;
double height = 4.0;
int sleep_ms = 0;

struct vertex *vertices = NULL;	/* list of vertices			*/
int nvertices = 0;
int nedges = 0;

static char tcl_command_buffer[1024];


double squarednorm(struct point);
double norm(struct point);
double product(struct point p, struct point q);
struct point difference(struct point p, struct point q);
struct point crossproduct(struct point p, struct point q);
void normalize(struct point *p);

int readgraph(void);
int writegraph(void);
int draw_graph(int);

void zero_out_positions();
void random_positions();
void initial_positions_planar(int);

int position(void);
void show_statistics(void);


/* ------------------------------------------------------------------------ */

#if USE_TCL

Tcl_Interp *interp;		/* Interpreter for this application. */


int
init_tk()
{
  char val[20];
  int code;

  interp = Tcl_CreateInterp();

  code = Tcl_Init(interp);
  if (code != TCL_OK) {
    fprintf(stderr, "in Tcl_Init: %s\n", interp->result);
    return code;
  }

  code = Tk_Init(interp);
  if (code != TCL_OK) {
    fprintf(stderr, "in Tk_Init: %s\n", interp->result);
    return code;
  }

  Tcl_StaticPackage(interp, "Tk", Tk_Init, (Tcl_PackageInitProc *) NULL);

  sprintf(val, "%d", maxstep);
  if (Tcl_SetVar(interp, "maxstep", val, TCL_LEAVE_ERR_MSG) == NULL) {
    fprintf(stderr, "in Tcl_SetVar: %s\n", interp->result);
    return code;
  }

  code = Tcl_Eval(interp, "source $env(EMBED_LIB)/tkembed.tcl");
  if (code != TCL_OK) {
    fprintf(stderr, "in Tcl_Eval: %s\n", interp->result);
    return code;
  }

  return TCL_OK;
}


int
exit_tk()
{
  int code;

  if (Tk_GetNumMainWindows() > 0) {
    sprintf(tcl_command_buffer, "done_calculation");
    code = Tcl_Eval(interp, tcl_command_buffer);
    if (code != TCL_OK) {
      fprintf(stderr, "in Tcl_Eval: %s\n", interp->result);
      return code;
    }
  }

  while (Tk_GetNumMainWindows() > 0) {
    sprintf(tcl_command_buffer, "tkwait variable window_changed");
    code = Tcl_Eval(interp, tcl_command_buffer);
    if (code != TCL_OK) {
      fprintf(stderr, "in Tcl_Eval: %s\n", interp->result);
      return code;
    }
    draw_graph(maxstep);
  }

  sprintf(tcl_command_buffer, "exit");
  Tcl_Eval(interp, tcl_command_buffer);
  Tcl_DeleteInterp(interp);
}

#endif /*USE_TCL*/

/* ------------------------------------------------------------------------ */


void
main(int argc, char *argv[])
{
  int c, i;
  int statistics = 0;
  int draw = 0;
  char initialize = 'p';

  extern char *optarg;

#ifndef NOTIMES
  srand48((long)time(NULL));
#else
  srand48(0);
#endif //NOTIMES

  if (readgraph())
    exit(1);

  if (nvertices == 0) {
    fprintf(stderr, "Empty graph\n");
    exit(1);
  }

  if (nvertices < 200)
    iter[1] = 200;
  else
    iter[1] = nvertices;

  iter[0] = iter[1] / 20;
  iter[2] = iter[1] / 10;

  height = 0.65 * sqrt((double)nvertices);

  while ((c = getopt(argc, argv, "fg:i:n:osv:w:B:D:E:H:NOW:")) != EOF)
  {
    switch (c) {
    case 'f':
      flat = 1;
      break;
    case 'g':
      sscanf( optarg, "%d,%d,%d", &(gstep[0]), &(gstep[1]), &(gstep[2]) );
      draw = 1;
      break;
    case 'i':
      initialize = optarg[0];
      break;
      break;
    case 'n':
      sscanf( optarg, "%d,%d,%d", &(iter[0]), &(iter[1]), &(iter[2]) );
      break;
    case 'o':
      fprintf(stdout,"%d\n",getpid());  fflush(stdout);
      break;
    case 's':
      statistics = 1;
      break;
    case 'v':
      sscanf( optarg, "%d,%d,%d", &(vstep[0]), &(vstep[1]), &(vstep[2]) );
      break;
    case 'w':
      bestangle = atof( optarg ) / 180.0 * M_PI;
      break;
    case 'B':
      frames_begin = atoi( optarg );
      break;
    case 'D':
      maxdist = atof( optarg );
      break;
    case 'E':
      frames_end = atoi( optarg );
      break;
    case 'H':
      height = atof( optarg );
      break;
    case 'N':
      print_frame_count = 1;
      break;
    case 'O':
      count_only = print_frame_count = 1;
      break;
    case 'W':
      sleep_ms = atoi(optarg);
      break;
    case '?':
      fprintf(stderr, "%s: unknown option -%c\n", argv[0], c);
      exit(1);
    }
  }

  switch(initialize) {
  case 'r':
    random_positions();
    break;
  case 's':
    initial_positions_planar(1);
    break;
  case 'z':
    zero_out_positions();
    break;
  case 'p':
    initial_positions_planar(0);
    break;
  default:
    break;
  }

  for (i = 1; i <= nvertices; i++) {
    vertices[i].saved_pos.x = vertices[i].pos.x;
    vertices[i].saved_pos.y = vertices[i].pos.y;
    vertices[i].saved_pos.z = vertices[i].pos.z;
  }

  maxstep = iter[0] + iter[1] + iter[2];

#if USE_TCL
  if (draw > 0) {
    init_tk();
    draw_graph(0);
    sprintf(tcl_command_buffer, "stop_go");
    c = Tcl_Eval(interp, tcl_command_buffer);
    if (c != TCL_OK) {
      fprintf(stderr, "in Tcl_Eval: %s\n", interp->result);
      exit(c);
    }
  }
#endif /*USE_TCL*/

  if (position())
    exit(1);

  if (writegraph())
    exit(1);

  if (statistics)
    show_statistics();

#if USE_TCL
  if (draw > 0)
    exit_tk();
#endif /*USE_TCL*/
}


/* ------------------------------------------------------------------------ */

double
  squarednorm(struct point p)
{
  return p.x*p.x + p.y*p.y + p.z*p.z;
}


double
  norm(struct point p)
{
  return sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
}


void
normalize(struct point *p)
{
  double f = sqrt(p->x * p->x + p->y * p->y + p->z * p->z);

  p->x /= f;
  p->y /= f;
  p->z /= f;
}


double
  product(struct point p, struct point q)
{
  return p.x*q.x + p.y*q.y + p.z*q.z;
}


struct point
  difference(struct point p, struct point q)
{
  struct point t;
  t.x = p.x - q.x;
  t.y = p.y - q.y;
  t.z = p.z - q.z;
  return t;
}


struct point
  crossproduct(struct point p, struct point q)
{
  struct point t;
  t.x = p.y * q.z - p.z * q.y;
  t.y = p.z * q.x - p.x * q.z;
  t.z = p.x * q.y - p.y * q.x;
  return t;
}


/* ------------------------------------------------------------------------ */


int
readgraph(void)
{
  char line[MAX_LINE_LENGTH];  /* one line from file                   */
  char *remain_line, *p;       /* remaining line                       */
  int  read_char;              /* number of read characters            */
  int  vert;                   /* read vertex index                    */
  int  tnei;                   /* read neighbour index (vertex index)  */
  int  i;                      /* counter                              */
  int  alloc;		       /* number of vertices allocated	       */
  double x, y, z;	       /* read coordinates                     */
  struct vertex *v;	       /* pointer to current vertex	       */
  int  dim, lineno, tmp;

  nvertices = 0;
  nedges = 0;

  alloc = 10;
  vertices = (struct vertex *) malloc( alloc * sizeof(struct vertex) );
  if (vertices == NULL) {
    fprintf(stderr, "Not enough memory at start");
    return 4;
  }

  /* First vertices and edges are read from std input */

  dim = 0;
  lineno = 0;
  while(fgets(line, MAX_LINE_LENGTH, stdin) == line)
  {
    lineno++;
    if (lineno == 1) {
      sscanf(line, " %n", &read_char);
      if (line[read_char] == '>') {
	if (strncmp(line+read_char, ">>writegraph3d", 14) == 0)
	  dim = 3;
	else if (strncmp(line+read_char, ">>writegraph2d", 14) == 0)
	  dim = 2;
	else {
	  fprintf(stderr, "Unknown format %s.\n", line+read_char);
	  return 1;
	}
	continue;
      }
    }

    if(sscanf(line,"%d %n", &vert, &read_char) !=1 )
      continue;

    if(vert==0)
      break;			/* End of graph data */

    if(vert != ++nvertices) {
      fprintf(stderr,
	      "At line %d: vertex numbers must be sequential.\n", lineno);
      return 2;
    }
    remain_line = &line[read_char];

    if(sscanf(remain_line,"%lf %lf %n", &x, &y, &read_char) < 2) {
      fprintf(stderr,"Coordinates format error in vertex %d!\n",nvertices);
      return 3;
    }
    remain_line += read_char;

    if (dim == 0) {
      if(sscanf(remain_line,"%d %n", &tmp, &read_char) < 1)
	dim = 2;
      else if (tmp <= 0 || strchr(".eE", remain_line[read_char]))
	dim = 3;
      else
	dim = 2;
    }

    if (dim == 3) {
      if(sscanf(remain_line,"%lf %n", &z, &read_char) < 1) {
	fprintf(stderr,"Missing z coordinate at vertex %d!\n",nvertices);
	return 3;
      }
      remain_line += read_char;
    }

    if (nvertices >= alloc) {
      alloc = alloc * 2;
      vertices =
	(struct vertex *) realloc( vertices, alloc * sizeof(struct vertex) );
      if (vertices == NULL) {
	fprintf(stderr, "Not enough memory at vertex %d", nvertices);
	return 4;
      }
    }

    v = &(vertices[nvertices]);

    v->saved_pos.x = v->pos.x = x;
    v->saved_pos.y = v->pos.y = y;
    if (flat)
      v->saved_pos.z = v->pos.z = 0.0;
    else
      v->saved_pos.z = v->pos.z = z;
    v->fixed = 0;

    i = 0;
    p = remain_line;
    while(sscanf(p, "%d%n", &tnei, &read_char) == 1) {
      i++;
      p += read_char;
    }

    v->valency = i;
    v->adj = (int *) malloc( i * sizeof(int) );
    if (v->adj == NULL) {
      fprintf(stderr, "Not enough memory at vertex %d", nvertices);
      return 4;
    }

    i = 0;
    p = remain_line;
    while(sscanf(p, "%d%n", &tnei, &read_char) == 1) {
      v->adj[i] = tnei;
      i++;
      p += read_char;
      if (tnei > nvertices)
	nedges++;
    }
  }

  return 0;
}


/* ------------------------------------------------------------------------ */


int
writegraph(void)
{
  int i, j;

  printf(">>writegraph3d<<\n");
  for (i = 1; i <= nvertices; i++) {
    printf("%3d %8.3lf %8.3lf %8.3lf",
	   i, vertices[i].pos.x, vertices[i].pos.y, vertices[i].pos.z);
    for (j = 0; j < vertices[i].valency; j++)
      printf( " %3d", vertices[i].adj[j] );
    printf("\n");
  }
  return 0;
}


/* ------------------------------------------------------------------------ */

double
compute_bondlength_forces(double bondlength)
{
  int i, j;
  struct vertex *v, *w;
  struct point delta;
  double fact, sumlength;

  sumlength = 0.0;

  for (i = 1; i <= nvertices; i++) {
    v = &(vertices[i]);

    for (j = 0; j < v->valency; j++) {
      if (v->adj[j] > i) {
	w = &(vertices[v->adj[j]]);

	delta = difference(v->pos, w->pos);
	fact = norm(delta);
	sumlength += fact;
	if (fact < 0.01)
	  continue;

	if (v->valency == 1 || w->valency == 1)
	  fact = (fact - 0.78 * bondlength) / fact;
	else
	  fact = (fact - bondlength) / fact;

	v->disp.x -= delta.x * fact;
	v->disp.y -= delta.y * fact;
	v->disp.z -= delta.z * fact;
	w->disp.x += delta.x * fact;
	w->disp.y += delta.y * fact;
	w->disp.z += delta.z * fact;
      }
    }
  }

  return sumlength;
}


void
compute_angular_forces()
{
  int i, j, k;
  struct vertex *u, *v, *w;
  struct point dvu, dvw, normal;
  double fact, s;

  s = 0.5 * sin((M_PI - bestangle) / 2.0);

  for (i = 1; i <= nvertices; i++) {
    u = &(vertices[i]);

    for (j = 0; j + 1 < u->valency; j++) {
      v = &(vertices[u->adj[j]]);

      for (k = j + 1; k < u->valency; k++) {
	w = &(vertices[u->adj[k]]);

	dvu = difference(u->pos, v->pos);
	dvw = difference(w->pos, v->pos);
	normal = crossproduct(dvw, dvu);
	normal = crossproduct(normal, dvw);
	fact = s * norm(dvw) / norm(normal);
	u->disp.x += 0.05 * (0.5 * dvw.x + fact * normal.x  - dvu.x);
	u->disp.y += 0.05 * (0.5 * dvw.y + fact * normal.y  - dvu.y);
	u->disp.z += 0.05 * (0.5 * dvw.z + fact * normal.z  - dvu.z);
      }
    }
  }
}


void
compute_local_repulsive_forces()
{
  int i, j, k;
  struct vertex *u, *v, *w;
  struct point delta;
  double fact;

  for (i = 1; i <= nvertices; i++) {
    w = &(vertices[i]);

    for (j = 0; j + 1 < w->valency; j++) {
      v = &(vertices[w->adj[j]]);

      for (k = j + 1; k < w->valency; k++) {
	u = &(vertices[w->adj[k]]);
	
	delta = difference(v->pos, u->pos);
	
	if (delta.x == 0.0) delta.x = 0.1 * (drand48() - 0.5);
	if (delta.y == 0.0) delta.y = 0.1 * (drand48() - 0.5);
	if (delta.z == 0.0 && !flat) delta.z = 0.1 * (drand48() - 0.5);

	fact = - 1.0 / squarednorm(delta);

	v->disp.x -= delta.x * fact;
	v->disp.y -= delta.y * fact;
	v->disp.z -= delta.z * fact;
	u->disp.x += delta.x * fact;
	u->disp.y += delta.y * fact;
	u->disp.z += delta.z * fact;
      }
    }
  }
}


void
compute_central_repulsion_forces()
{
  int i;
  struct vertex *v;
  struct point sum, avg, delta;
  double fact;

  sum.x = sum.y = sum.z = 0.0;

  for (i = 1; i + 1 <= nvertices; i++) {
    v = &(vertices[i]);

    sum.x += v->pos.x;
    sum.y += v->pos.y;
    sum.z += v->pos.z;
  }

  avg.x = sum.x / nvertices;
  avg.y = sum.y / nvertices;
  avg.z = sum.z / nvertices;

  for (i = 1; i + 1 <= nvertices; i++) {
    v = &(vertices[i]);

    delta = difference(v->pos, avg);

    if (delta.x == 0.0) delta.x = 0.1 * (drand48() - 0.5);
    if (delta.y == 0.0) delta.y = 0.1 * (drand48() - 0.5);
    if (delta.z == 0.0 && !flat) delta.z = 0.1 * (drand48() - 0.5);

    fact = 0.5 * sqrt((double)nvertices) / squarednorm(delta);

    v->disp.x += delta.x * fact;
    v->disp.y += delta.y * fact;
    v->disp.z += delta.z * fact;
  }
}


void
perform_displacement(double temp, double *sumdisp, double *maxdisp)
{
  int i;
  double disp, fact, lambda;
  struct vertex *v;
  struct point d;

  *maxdisp = 0.0;
  *sumdisp = 0.0;

  for (i = 1; i <= nvertices; i++) {
    disp = norm(vertices[i].disp);
    if (disp > *maxdisp)
      *maxdisp = disp;
    *sumdisp += disp;
  }

  fact = *maxdisp > temp ? temp / *maxdisp : 1.0;

  for (i = 1; i <= nvertices; i++) {
    v = &(vertices[i]);
    v->pos.x += v->disp.x * fact;
    v->pos.y += v->disp.y * fact;
    v->pos.z += v->disp.z * fact;
  }
}


/* ------------------------------------------------------------------------ */


void
zero_out_positions()
{
  int i;

  for (i = 1; i <= nvertices; i++)
    vertices[i].pos.x = vertices[i].pos.y = vertices[i].pos.z = 1.0;
}


void
random_positions()
{
  int i;

  for (i = 1; i <= nvertices; i++) {
    vertices[i].pos.x = 2.0 * (drand48() - 0.5);
    vertices[i].pos.y = 2.0 * (drand48() - 0.5);
    vertices[i].pos.z = 2.0 * (drand48() - 0.5);
  }
}


void
initial_positions_planar(int spherical)
{
  int front, rear;
  int *index_to_bfs_rank;
  int *bfs_rank_to_index;
  int *bfs_rank_to_level;
  int *level_to_bfs_rank;
  double *bfs_rank_to_lo_angle;
  double *bfs_rank_to_angle_wd;
  double *bfs_rank_to_pos_angle;
  int i, j, k, k1, k2, s, n;
  int deg, mindeg;
  double wd, sumwd, avgwd;
  double angle, angle1, angle2;
  double dr, height, radius, oldradius;
  int level, maxlevel;
  struct vertex *v, *w;
  int best_height, best_vertex, startvertex;


  /* --- allocate memory for the various arrays --- */

  index_to_bfs_rank = (int *) malloc(sizeof(int) * (nvertices + 1));
  bfs_rank_to_index = (int *) malloc(sizeof(int) * (nvertices + 1));
  bfs_rank_to_level = (int *) malloc(sizeof(int) * (nvertices + 2));
  level_to_bfs_rank = (int *) malloc(sizeof(int) * (nvertices + 1));
  bfs_rank_to_lo_angle = (double *) malloc(sizeof(double) * (nvertices + 1 ));
  bfs_rank_to_angle_wd = (double *) malloc(sizeof(double) * (nvertices + 1 ));
  bfs_rank_to_pos_angle = (double *) malloc(sizeof(double) * (nvertices + 1 ));

  if (   index_to_bfs_rank == NULL
      || bfs_rank_to_index == NULL
      || bfs_rank_to_level == NULL
      || level_to_bfs_rank == NULL
      || bfs_rank_to_lo_angle == NULL
      || bfs_rank_to_angle_wd == NULL
      || bfs_rank_to_pos_angle == NULL)
  {
    fprintf(stderr, "Not enough memory for constructing initial positions");
    exit(4);
  }


  /* --- find a good starting vertex --- */

  best_height = 0;
  best_vertex = 0;

  for (startvertex = 1; startvertex <= nvertices; startvertex++) {

    for (i = 1; i <= nvertices; i++)
      index_to_bfs_rank[i] = 0;

    front = rear = 1;

    index_to_bfs_rank[startvertex] = front;
    bfs_rank_to_index[front] = startvertex;
    bfs_rank_to_level[front] = 0;
    front++;

    while (rear < front) {
      i = bfs_rank_to_index[rear];
      v = &(vertices[i]);

      for (j = 0; j < v->valency; j++) {
	k = v->adj[j];
	if (index_to_bfs_rank[k] == 0) {
	  index_to_bfs_rank[k] = front;
	  bfs_rank_to_index[front] = k;
	  bfs_rank_to_level[front] = bfs_rank_to_level[rear] + 1;
	  front++;
	}
      }
      rear++;
    }

    maxlevel = bfs_rank_to_level[--front];
    if (maxlevel > best_height) {
      best_height = maxlevel;
      best_vertex = startvertex;
    }
  }

  startvertex = best_vertex;

  /* --- initialize some arrays --- */

  for (i = 1; i <= nvertices; i++) {
    index_to_bfs_rank[i] = 0;
    bfs_rank_to_lo_angle[i] = -1.0;
    bfs_rank_to_angle_wd[i] = 0.0;
  }


  /* --- make first level (either one vertex or a smallest face) --- */

  level_to_bfs_rank[0] = 1;
  front = rear = 1;

  index_to_bfs_rank[startvertex] = front;
  bfs_rank_to_index[front] = startvertex;
  bfs_rank_to_level[front] = 0;
  front++;

  /* --- assign breadth first search ranks to the remaining vertices --- */

  while (rear < front) {
    i = bfs_rank_to_index[rear];
    v = &(vertices[i]);

    /* --- look for the first child of v in circular order --- */

    for (k = 0; k < v->valency; k++) {
      j = (k+1) % (v->valency);
      if (index_to_bfs_rank[v->adj[k]] != 0
	  &&
	  index_to_bfs_rank[v->adj[j]] == 0)
	break;
    }

    if (k >= v->valency)
      j = 0;

    /* --- now assign new numbers to all of the children --- */

    while (1) {
      k = v->adj[j];
      if (index_to_bfs_rank[k] == 0) {
	index_to_bfs_rank[k] = front;
	bfs_rank_to_index[front] = k;
	bfs_rank_to_level[front] = bfs_rank_to_level[rear] + 1;
	if (bfs_rank_to_level[front] > bfs_rank_to_level[front-1])
	  level_to_bfs_rank[bfs_rank_to_level[front]] = front;
	front++;
      }
      else
	break;

      j = (j+1) % (v->valency);
    }
    rear++;
  }
  front--;

  if (front < nvertices)
    fprintf(stderr, "warning: graph is not connected");


  /* --- we are ready to assign positions to the vertices --- */

  height = radius = 0.0;
  maxlevel = bfs_rank_to_level[front];
  bfs_rank_to_level[front + 1] = maxlevel+1;

  for (level = 0; level <= maxlevel; level++)
  {
    /* --- assign angle ranges to vertices in current level --- */

    if (level == 0) {
      wd = 2.0 * M_PI / (level_to_bfs_rank[1] - 1);
      for (i = 1; i < level_to_bfs_rank[1]; i++) {
	bfs_rank_to_lo_angle[i] = (i-1) * wd;
	bfs_rank_to_angle_wd[i] = wd;
      }
      avgwd = wd;
    }
    else
    {
      for (i = level_to_bfs_rank[level-1];
	   i < level_to_bfs_rank[level];
	   i++)
      {
	v = &(vertices[bfs_rank_to_index[i]]);
	for (k = 0; k < v->valency; k++) {
	  k1 = (k+1) % (v->valency);
	  if (bfs_rank_to_level[index_to_bfs_rank[v->adj[k]]] < level
	      &&
	      bfs_rank_to_level[index_to_bfs_rank[v->adj[k1]]] == level)
	    break;
	}
	if (k >= v->valency) {
	  if (bfs_rank_to_level[index_to_bfs_rank[v->adj[0]]] == level) {
	    if (i > 1) {
	      fprintf(stderr, "Oops! Inconsistent graph!");
	      abort();
	    }
	    else
	      k1 = 0;
	  }
	  else
	    continue;
	}

	n = 0;
	k = k1;
	do {
	  n++;
	  k = (k+1) % (v->valency);
	}
	while(k != k1 &&
	      bfs_rank_to_level[index_to_bfs_rank[v->adj[k]]] == level);

	wd = bfs_rank_to_angle_wd[i] / n;

	n = 0;
	k = k1;
	j = index_to_bfs_rank[v->adj[k]];
	do {
	  if (bfs_rank_to_lo_angle[j] < 0.0 || k != k1) {
	    bfs_rank_to_lo_angle[j] = bfs_rank_to_lo_angle[i] + wd * n;
	  }
	  bfs_rank_to_angle_wd[j] += wd;
	  n++;
	  k = (k+1) % (v->valency);
	  j = index_to_bfs_rank[v->adj[k]];
	}
	while(k != k1 && bfs_rank_to_level[j] == level);
      }

      sumwd = 0.0;
      for (k = level_to_bfs_rank[level];
	   bfs_rank_to_level[k] == level;
	   k++)
      {
	  sumwd += bfs_rank_to_angle_wd[k];
      }
      avgwd = sumwd / (k - level_to_bfs_rank[level]);
    }

    /* --- determine radius and height of supporting circle --- */

    if (flat)
      radius += 2.0;
    else if (spherical) {
      angle = - 0.5 * M_PI + (level + 0.5) * M_PI / (maxlevel + 1.0);
      radius = cos(angle) * sqrt((double)nvertices);
      height = sin(angle) * sqrt((double)nvertices);
    }
    else {
      oldradius = radius;
      radius = 2.0 / avgwd;
      dr = radius - oldradius;
      if (dr * dr < 1.99)
	height += sqrt(2.0 - (dr * dr));
      else
	height += 0.1;
    }

    /* --- now assign positions to vertices in current level --- */

    for (k = level_to_bfs_rank[level];
	 bfs_rank_to_level[k] == level;
	 k++)
    {
      v = &(vertices[bfs_rank_to_index[k]]);
      angle = bfs_rank_to_lo_angle[k] + 0.5 * bfs_rank_to_angle_wd[k];
      while (angle >= 2.0 * M_PI)
	angle -= 2.0 * M_PI;
      bfs_rank_to_pos_angle[k] = angle;
      v->pos.x = cos(angle) * radius;
      v->pos.y = sin(angle) * radius;
      v->pos.z = height;
    }

    /* --- adjust angle ranges according to point positions --- */

    for (k = level_to_bfs_rank[level];
	 bfs_rank_to_level[k] == level;
	 k++)
    {
      angle = bfs_rank_to_pos_angle[k];

      if (bfs_rank_to_level[k+1] > level)
	k1 = level_to_bfs_rank[level];
      else
	k1 = k + 1;
      angle1 = bfs_rank_to_pos_angle[k1];

      if (bfs_rank_to_level[k1+1] > level)
	k2 = level_to_bfs_rank[level];
      else
	k2 = k1 + 1;
      angle2 = bfs_rank_to_pos_angle[k2];

      if (angle1 <= angle) {
	angle1 += 2.0 * M_PI;
	angle2 += 2.0 * M_PI;
      }
      if (angle2 <= angle1)
	angle2 += 2.0 * M_PI;

      bfs_rank_to_angle_wd[k1] = (angle2 - angle) / 2.0;
      angle = (angle + angle1) / 2.0;
      while (angle >= 2.0 * M_PI)
	angle -= 2.0 * M_PI;
      bfs_rank_to_lo_angle[k1] = angle;
    }
  }

  /* --- cleaning up: --- */

  free(index_to_bfs_rank);
  free(bfs_rank_to_index);
  free(bfs_rank_to_level);
  free(level_to_bfs_rank);
  free(bfs_rank_to_lo_angle);
  free(bfs_rank_to_angle_wd);
  free(bfs_rank_to_pos_angle);
}


/* ------------------------------------------------------------------------ */

int
  position(void)
{
  FILE *pipe;
  static char letter[] = { 'a', 'b', 'c' };
  int it, phase;
  int do_local, do_central, do_bondlength, do_angular;
  int i;
  struct vertex *v;
  double arg, temp;
  double maxdisp, sumdisp, avgdisp, sumlength, avglength;
  double bondlength;
  int frame_count;
  int step;

  for (i = 1; i <= nvertices; i++) {
    v = &vertices[i];
    v->saved_pos.x = v->pos.x;
    v->saved_pos.y = v->pos.y;
    v->saved_pos.z = v->pos.z;
  }

  step = 0;

#if USE_TCL
  if (gstep[0] > 0)
    for (it = 0; it < frames_begin; it++)
      frame_count = draw_graph(step);
#endif /*USE_TCL*/

  for (phase = 0; phase < 3; phase++) {
#if USE_TCL
    if (gstep[phase] > 0)
      frame_count = draw_graph(step);
#endif /*USE_TCL*/

    if (vstep[phase] > 0)
      fprintf(stderr, "Phase %c:\n", letter[phase]);

    for (it = 0; it < iter[phase]; ++it) {
      ++step;

      for (i = 1; i <= nvertices; i++)
	vertices[i].disp.x = vertices[i].disp.y = vertices[i].disp.z = 0.0;

      do_bondlength = 1;

      arg = 1.0 - (double)it / (double)iter[phase];

      switch (phase) {
      case 0:
	temp = arg * arg;
	bondlength = 0.0;

	do_central = 1;
	do_local = 0;
	do_angular = 0;
	break;
      case 1:
	temp = arg * arg * arg * arg;
	bondlength = 0.0;

	do_local = 1;
	do_central = 0;
	do_angular = 0;
	break;
      case 2:
	temp = 0.4 * arg * arg * arg;
	bondlength = 1.414;

	do_local = 0;
	do_central = 0;
	do_angular = 1;
	break;
      }

      if (do_bondlength)
	sumlength = compute_bondlength_forces(bondlength);

      if (do_local)
	compute_local_repulsive_forces();

      if (do_central)
	compute_central_repulsion_forces();

      if (do_angular)
	compute_angular_forces();

      perform_displacement(temp, &sumdisp, &maxdisp);

#if USE_TCL
      if ( gstep[phase] > 0
	   && ( it == iter[phase] - 1 || it % gstep[phase] == 0) )
      {
	frame_count = draw_graph(step);
      }
#endif /*USE_TCL*/

      if ( vstep[phase] > 0
	   && ( it == iter[phase] - 1 || it % vstep[phase] == 0) )
      {
	avgdisp = sumdisp / (double)nvertices;
	avglength = sumlength / (double)nedges;

	fprintf( stderr, "i = %3d, temp = %8.6lf, maxd = %8.6lf, ",
		 it, temp, maxdisp );
	fprintf( stderr, "avgd = %8.6lf, avgl = %8.6lf\n",
		 avgdisp, avglength );
      }
    }
  }

#if USE_TCL
  for (it = 0; it < frames_end; it++)
    frame_count = draw_graph(step);
#endif /*USE_TCL*/

  if (print_frame_count)
    fprintf( stderr, "Frame # = %d\n", frame_count );

  maxstep = step;

  return 0;
}


/* ------------------------------------------------------------------------ */

#if USE_TCL

int
cmp_edges(const void* lhs, const void* rhs)
{
  struct edge *e1, *e2;
  double c1, c2;

  e1 = (struct edge *)lhs;
  e2 = (struct edge *)rhs;

  c1 = (e1->p.z + e1->q.z) / 2.0;
  c2 = (e2->p.z + e2->q.z) / 2.0;

  if (c1 < c2)
    return -1;
  else if (c1 > c2)
    return 1;
  else
    return 0;
}


int
  draw_graph(int step)
{
  int code;
  double display_width, display_height;
  double xscale, yscale;
  double xoff, yoff;

  struct vertex *v, *w;
  struct point t;
  int n, i, j;
  static int angle = 90;
  static int count = 0;
  int steps;
  double s, c;
  double xc, yc, zc;
  double d, dmax, avglen;
  double f1, f2;
  double xmin, xmax, ymin, ymax, zmin, zmax;
  double elevation;

  struct edge *edges;
  int e;

  edges = (struct edge *) malloc( sizeof(struct edge) * nedges );
  if (edges == NULL) {
    fprintf(stderr, "Not enough memory");
    return;
  }

  /* --- determine number of interpolation steps  --- */

  dmax = 0.0;
  avglen = 0.0;

  for (i = 1; i <= nvertices; i++) {
    v = &(vertices[i]);
    d = norm( difference( v->pos, v->saved_pos ) );
    if (d > dmax)
      dmax = d;
    for (j = 0; j < v->valency; j++) {
      if (v->adj[j] > i) {
	w = &(vertices[v->adj[j]]);
	avglen += norm( difference( v->saved_pos, w->saved_pos ) );
      }
    }
  }

  avglen /= nedges;
  steps = (int) ( dmax / maxdist ) + 1;


  /* --- interpolate and display --- */

  if (count_only)
    count += steps;
  else {
    for (n = 1; n <= steps; n++) {
      /* --- initialize Tk stuff --- */

      if (Tk_GetNumMainWindows() <= 0) {
	free(edges);
	return TCL_RETURN;
      }

      sprintf(tcl_command_buffer, "update; set step %d", step);
      code = Tcl_Eval(interp, tcl_command_buffer);
      if (code != TCL_OK) {
	fprintf(stderr, "in Tcl_Eval: %s\n", interp->result);
	free(edges);
	return code;
      }

      if (Tk_GetNumMainWindows() <= 0) {
	free(edges);
	return TCL_RETURN;
      }

      sprintf(tcl_command_buffer, "get_canvas_size");
      code = Tcl_Eval(interp, tcl_command_buffer);
      if (code != TCL_OK) {
	fprintf(stderr, "in Tcl_Eval: %s\n", interp->result);
	free(edges);
	return code;
      }
      sscanf(interp->result, "%lf %lf", &display_width, &display_height);

      xscale = (display_width  / 2.0 - 10.0) / height;
      yscale = (display_height / 2.0 - 10.0) / height;

      if (xscale <= yscale)
	yscale = xscale;
      else
	xscale = yscale;

      yscale *= -1;

      xoff = display_width / 2.0;
      yoff = display_height - 10.0;

      sprintf(tcl_command_buffer, "clear_canvas");
      code = Tcl_Eval(interp, tcl_command_buffer);
      if (code != TCL_OK) {
	fprintf(stderr, "in Tcl_Eval: %s\n", interp->result);
	free(edges);
	return code;
      }


      /* --- compute current transformation parameters --- */

      f2 = (double)n / (double)steps;
      f1 = 1.0 - f2;

      s = sin( angle * M_PI / 180.0);
      c = cos( angle * M_PI / 180.0);
      angle = ( angle - 1 ) % 360;

      /* --- compute center of gravity to rotate about --- */

      xc = yc = zc = 0.0;

      for (i = 1; i <= nvertices; i++) {
	v = &(vertices[i]);
	xc += f1 * v->saved_pos.x + f2 * v->pos.x;
	yc += f1 * v->saved_pos.y + f2 * v->pos.y;
	zc += f1 * v->saved_pos.z + f2 * v->pos.z;
      }
      xc /= nvertices;
      yc /= nvertices;
      zc /= nvertices;


      /* --- compute endpoints of all the edges --- */

      e = 0;
      for (i = 1; i <= nvertices; i++) {
	v = &(vertices[i]);

	for (j = 0; j < v->valency; j++) {
	  if (v->adj[j] > i) {
	    w = &(vertices[v->adj[j]]);

	    t.x = f1 * w->saved_pos.x + f2 * w->pos.x - xc;
	    t.y = f1 * w->saved_pos.y + f2 * w->pos.y - yc;
	    t.z = f1 * w->saved_pos.z + f2 * w->pos.z - zc;

	    edges[e].p.x = t.x * c - t.z * s;
	    edges[e].p.y = t.y;
	    edges[e].p.z = t.x * s + t.z * c;

	    t.x = f1 * v->saved_pos.x + f2 * v->pos.x - xc;
	    t.y = f1 * v->saved_pos.y + f2 * v->pos.y - yc;
	    t.z = f1 * v->saved_pos.z + f2 * v->pos.z - zc;

	    edges[e].q.x = t.x * c - t.z * s;
	    edges[e].q.y = t.y;
	    edges[e].q.z = t.x * s + t.z * c;

	    ++e;
	  }
	}
      }

      nedges = e;

      /* --- compute bounding box --- */

      for (e = 0; e < nedges; e++) {
	if (e == 0) {
	  xmin = xmax = edges[e].p.x;
	  ymin = ymax = edges[e].p.y;
	  zmin = zmax = edges[e].p.z;
	}

	if (edges[e].p.x < xmin) xmin = edges[e].p.x;
	if (edges[e].p.x > xmax) xmax = edges[e].p.x;
	if (edges[e].p.y < ymin) ymin = edges[e].p.y;
	if (edges[e].p.y > ymax) ymax = edges[e].p.y;
	if (edges[e].p.z < zmin) zmin = edges[e].p.z;
	if (edges[e].p.z > zmax) zmax = edges[e].p.z;

	if (edges[e].q.x < xmin) xmin = edges[e].q.x;
	if (edges[e].q.x > xmax) xmax = edges[e].q.x;
	if (edges[e].q.y < ymin) ymin = edges[e].q.y;
	if (edges[e].q.y > ymax) ymax = edges[e].q.y;
	if (edges[e].q.z < zmin) zmin = edges[e].q.z;
	if (edges[e].q.z > zmax) zmax = edges[e].q.z;
      }


      /* --- compute elevation --- */

      if ( - ymin > 0.9 * height)
	elevation = - ymin + 0.1 * height;
      else
	elevation = height;

      for (e = 0; e < nedges; e++) {
	edges[e].p.y += elevation;
	edges[e].q.y += elevation;
      }

      /* --- now do the drawing --- */

      qsort((void *)edges, nedges, sizeof(struct edge), cmp_edges);

      for (e = 0; e < nedges; e++) {
	edges[e].shade = 
	  (int) (0.5 + 14.0 * ((zmax - (edges[e].p.z + edges[e].q.z) / 2.0)
			       / (zmax - zmin)));

	sprintf(tcl_command_buffer,
		"add_closer_line %lf %lf %lf %lf #%x%xf",
		edges[e].p.x * xscale + xoff,
		edges[e].p.y * yscale + yoff,
		edges[e].q.x * xscale + xoff,
		edges[e].q.y * yscale + yoff,
		edges[e].shade, edges[e].shade
		);
	code = Tcl_Eval(interp, tcl_command_buffer);
	if (code != TCL_OK) {
	  fprintf(stderr, "in Tcl_VarEval: %s\n", interp->result);
	  free(edges);
	  return code;
	}
      }

      /* --- update screen and pause if requested --- */

      sprintf(tcl_command_buffer, "update idletasks");
      code = Tcl_Eval(interp, tcl_command_buffer);
      if (code != TCL_OK) {
	fprintf(stderr, "in Tcl_Eval: %s\n", interp->result);
	free(edges);
	return code;
      }

      Tcl_Sleep(sleep_ms);
    }
  }

  for (i = 1; i <= nvertices; i++) {
    v = &vertices[i];
    v->saved_pos.x = v->pos.x;
    v->saved_pos.y = v->pos.y;
    v->saved_pos.z = v->pos.z;
  }

  free(edges);
  return count;
}

#endif /*USE_TCL*/

/* ------------------------------------------------------------------------ */

void
show_statistics()
{
  int i, j, k, n;
  struct vertex *u, *v, *w;
  struct point delta, dv, dw;
  double min, max, sum, length, fact, angle;

  max = 0.0;
  min = 10.0;
  sum = 0.0;
  n = 0;

  for (i = 1; i <= nvertices; i++) {
    v = &(vertices[i]);
    for (j = 0; j < v->valency; j++) {
      if (v->adj[j] > i) {
	w = &(vertices[v->adj[j]]);
	
	delta = difference(v->pos, w->pos);
	length = norm(delta);
	if (length > max)
	  max = length;
	if (length < min)
	  min = length;
	sum += length;
	n++;
      }
    }
  }

  fprintf( stderr, "Bondlength (min, max, avg): %lf, %lf, %lf\n",
	   min, max, sum/n );

  min = 360.0;
  max = 0.0;
  sum = 0.0;
  n = 0;

  for (i = 1; i <= nvertices; i++) {
    u = &(vertices[i]);
    for (j = 0; j + 1 < u->valency; j++) {
      v = &(vertices[u->adj[j]]);
      for (k = j + 1; k < u->valency; k++) {
	w = &(vertices[u->adj[k]]);
	
	dv = difference(v->pos, u->pos);
	dw = difference(w->pos, u->pos);
	fact = norm(dv) * norm(dw);
	if (fact <= 1e-3)
	  continue;

	angle = acos(product(dv, dw) / fact) / M_PI * 180.0;

	if (angle > max)
	  max = angle;
	if (angle < min)
	  min = angle;
	sum += angle;
	n++;
      }
    }
  }

  fprintf( stderr, "Angle (min, max, avg): %lf, %lf, %lf\n",
	   min, max, sum/n );
  fflush(stderr);
}


/*
** Local Variables:
** compile-command: "make -k tkspring"
** End:
*/

/* -- EOF tkspring.c -- */

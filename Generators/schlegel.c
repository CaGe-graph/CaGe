/*
 *	schlegel.c		Calculation of Schlegel diagrams for
 *				2-connected, not necessarily 3-connected,
 *				planar maps.
 *
 *	Based on a Schlegel program by Bor Plestenjak, Ljubljana 1995
 *
 *	ODF (Olaf Delgado Friedrichs) 1998/02/24
 */

#include<stdio.h>
#include<stdlib.h>
#include<math.h>


#define MAX_LINE_LENGTH 3000
#define MAX_ITERATIONS   500


struct coor { double x,y; };	   /* 2D coordinates */

struct vertex {			   /* vertex */
  int fixed;			   /* in outer cycle? */
  struct coor v,disp;
  int valency, orig_valency, alloc_valency;
  int *adj;
  int *rev;
};

struct triangle { int a, b, c; };


struct vertex *v = NULL;	   /* vertices */
struct triangle *tri = NULL;
int nvertices = 0;
int ntriangles = 0;
int cycle_length;
int n_orig_vertices;
int n_alloc_vertices;

void normalize_coordinates(void);  /* normalizes and centralizes vertices */
void subdivide_faces(void);	   /* subdivide each internal face */
void main_loop(int);              /* main loop of the algorithm */
int read_graph(void);              /* reads graph */
int write_graph(int);       	   /* writes graph */
double norm(struct coor c);        /* euclidean norm */


/* ------------------------------------------------------------------------ */


main( int argc, char *argv[] )
{
  int c;
  int pull = 1;
  int subdivide = 1;
  int write_all_vertices = 0;

  while ((c = getopt(argc, argv, "aops")) != EOF) {
    switch (c) {
    case 'a':
      write_all_vertices = !write_all_vertices;
      break;
    case 'p':
      pull = !pull;
      break;
    case 'o':
      fprintf(stdout,"%d\n",getpid());
      fflush(stdout);
      break;
    case 's':
      subdivide = !subdivide;
      break;
    default:
      break;
    }
  }

  if (read_graph() != 0)
    return 1;

  if (subdivide)
    subdivide_faces();

  main_loop(pull);
  write_graph(write_all_vertices);
  return 0;
}

/* ------------------------------------------------------------------------ */


int
read_graph(void)
{
  char line[MAX_LINE_LENGTH];  /* one line from file                   */
  char *remain_line, *p;       /* remaining line                       */
  int  read_char;              /* number of read characters            */
  int  vert;                   /* read vertex index                    */
  int  tnei;                   /* read neighbour index (vertex index)  */
  int  i;                      /* counter                              */
  int  alloc;		       /* number of vertices allocated	       */
  double tmpx, tmpy;           /* read coordinates                     */

  alloc = 10;
  v = (struct vertex *) malloc( alloc * sizeof(struct vertex) );
  if (v == NULL) {
    fprintf(stderr, "Not enough memory at start");
    return 4;
  }

  /* First vertices and edges are read from std input */

  while(fgets(line,MAX_LINE_LENGTH,stdin)==line) {
    if(sscanf(line,"%d %n",&vert,&read_char)!=1) {
      fprintf(stderr,"Data format error in line %d",nvertices);
      return 1;
    }

    if(vert==0)
      break;			/* End of graph data */

    if(vert != ++nvertices) {
      fprintf(stderr,"Data format error in line %d",nvertices);
      return 2;
    }
    remain_line = &line[read_char];

    if(sscanf(remain_line,"%lf %lf %n",&tmpx,&tmpy,&read_char)!=2) {
      fprintf(stderr,"Coordinates format error in vertex %d",nvertices);
      return 3;
    }
    remain_line += read_char;

    if (nvertices >= alloc) {
      alloc = alloc * 2;
      v = (struct vertex *) realloc( v, alloc * sizeof(struct vertex) );
      if (v == NULL) {
	fprintf(stderr, "Not enough memory at vertex %d", nvertices);
	return 4;
      }
    }
      
    v[nvertices].v.x=tmpx;
    v[nvertices].v.y=tmpy;
    v[nvertices].fixed = 0;

    i = 0;
    p = remain_line;
    while(sscanf(p, "%d%n", &tnei, &read_char) == 1) {
      i++;
      p += read_char;
    }

    v[nvertices].valency = i;
    v[nvertices].orig_valency = i;
    v[nvertices].alloc_valency = 2 * i;
    v[nvertices].adj =
      (int *) malloc( v[nvertices].alloc_valency * sizeof(int) );
    if (v[nvertices].adj == NULL) {
      fprintf(stderr, "Not enough memory at vertex %d", nvertices);
      return 4;
    }

    i = 0;
    p = remain_line;
    while(sscanf(p, "%d%n", &tnei, &read_char) == 1) {
      v[nvertices].adj[i] = tnei;
      i++;
      p += read_char;
    }
  }

  n_orig_vertices = nvertices;
  n_alloc_vertices = alloc;

  normalize_coordinates();

  if (fscanf(stdin,"%d",&cycle_length)!=1) {
    fprintf(stderr,"Outer cycle format error");
    return 5;
  }
  if (fgets(line,MAX_LINE_LENGTH,stdin)!=line) {
    fprintf(stderr,"Outer cycle format error 1");
    return 7;
  }

  remain_line=line;
  for(i=1; i <= cycle_length; i++) {
    if(sscanf(remain_line,"%d%n",&vert,&read_char)!=1) {
      fprintf(stderr,"Outer cycle format error 2 in %d step",i);
      return 7;
    }
    v[vert].fixed=1;
    v[vert].v.x=1000*cos(2*i*M_PI/cycle_length);
    v[vert].v.y=1000*sin(2*i*M_PI/cycle_length);
    remain_line += read_char;
  }

  return 0;
}

/* ------------------------------------------------------------------------ */


int
write_graph(all)
{
  int i, j, w;

  if (all) {
    for (i = 1; i <= nvertices; i++) {
      printf("%3d %8.3lf %8.3lf", i, v[i].v.x, v[i].v.y);
      for (j = 0; j < v[i].valency; j++)
	printf( " %3d", v[i].adj[j] );
      printf("\n");
    }
  }
  else {
    for (i = 1; i <= n_orig_vertices; i++) {
      printf("%3d %8.3lf %8.3lf", i, v[i].v.x, v[i].v.y);
      for (j = 0; j < v[i].orig_valency; j++)
	printf( " %3d", v[i].adj[j] );
      printf("\n");
    }
  }
  return 0;
}


/* ------------------------------------------------------------------------ */


double
norm(struct coor c)
{
  return sqrt(c.x*c.x+c.y*c.y);
}

/* ------------------------------------------------------------------------ */


void
normalize_coordinates(void)
{
  int i;
  double max = 0, sumx = 0 ,sumy = 0, r;

  for(i = 1; i <= nvertices; i++) {
    sumx += v[i].v.x;
    sumy += v[i].v.y;
  }
  sumx /= nvertices;
  sumy /= nvertices;

  for(i = 1; i <= nvertices; i++) {
    v[i].v.x -= sumx;
    v[i].v.y -= sumy;
  }

  for(i = 1; i <= nvertices; i++) {
    r = norm(v[i].v);
    if (r > max)
      max = r;
  }
  max /= 1000;

  if (max > 0) {
    for(i = 1; i <= nvertices; i++) {
      v[i].v.x /= max;
      v[i].v.y /= max;
    }
  }
}

/* ------------------------------------------------------------------------ */

void
make_reverses(void)
{
  int i, j, k;
  struct vertex *w;

  for (i = 1; i <= nvertices; i++) {
    v[i].rev = (int *)malloc(sizeof(int) * v[i].valency);
    if (v[i].rev == NULL) {
      fprintf(stderr, "Not enough memory");
      return;
    }
    
    for (j = 0; j < v[i].valency; j++) {
      w = &(v[v[i].adj[j]]);
      for (k = 0; k < w->valency; k++)
	if (w->adj[k] == i)
	  break;
      if (k >= w->valency) {
	fprintf(stderr,
		"Oops! Graph inconsistent at vertices %d and %d\n",
		i, v[i].adj[j]);
	abort();
      }
      v[i].rev[j] = k;
    }
  }
}


void
subdivide_faces(void)
{
  int **seen;
  int i, j, k, t;

  make_reverses();

  seen = (int **) malloc( sizeof(int *) * (n_orig_vertices+1) );
  if (seen == NULL) {
    fprintf(stderr, "Not enough memory");
    return;
  }

  ntriangles = 0;

  for (i = 1; i <= n_orig_vertices; i++) {
    seen[i] = (int *) malloc( sizeof(int) * v[i].orig_valency );
    if (seen[i] == NULL) {
      fprintf(stderr, "Not enough memory");
      return;
    }
    for (j = 0; j < v[i].orig_valency; j++)
      seen[i][j] = 0;
    ntriangles += v[i].orig_valency;
  }
  
  ntriangles -= cycle_length;
  tri = (struct triangle *) malloc( sizeof(struct triangle) * ntriangles );
  if (tri == NULL) {
    fprintf(stderr, "Not enough memory");
    return;
  }
  t = 0;

  for (i = 1; i <= n_orig_vertices; i++) {
    for (j = 0; j < v[i].orig_valency; j++) {
      if (!seen[i][j]) {
	int outer = 1;
	int fdeg = 0;
	int i1 = i, j1 = j;
	int tmp;
	do {
	  if (!v[i1].fixed)
	    outer = 0;
	  fdeg++;
	  seen[i1][j1] = 1;
	  tmp = v[i1].adj[j1];
	  j1 = (v[i1].rev[j1]+1)%v[tmp].orig_valency;
	  i1 = tmp;
	} while (i1 != i || j1 != j);

	if (!outer) {
	  if (++nvertices >= n_alloc_vertices) {
	    n_alloc_vertices = n_alloc_vertices * 2;
	    v = (struct vertex *)
	      realloc( v, n_alloc_vertices * sizeof(struct vertex) );
	    if (v == NULL) {
	      fprintf(stderr,
		      "Not enough memory at temporary vertex %d", nvertices);
	      return;
	    }
	  }

	  v[nvertices].fixed = 0;
	  v[nvertices].valency = 0;
	  v[nvertices].orig_valency = 0;
	  v[nvertices].alloc_valency = fdeg;
	  v[nvertices].v.x = v[nvertices].v.y = 0.0;
	  v[nvertices].adj =
	    (int *) malloc( v[nvertices].alloc_valency * sizeof(int) );
	  if (v[nvertices].adj == NULL) {
	    fprintf(stderr, "Not enough memory at vertex %d", nvertices);
	    return;
	  }

	  do {
	    v[i1].adj[v[i1].valency++] = nvertices;
	    v[nvertices].adj[v[nvertices].valency++] = i1;
	    tmp = v[i1].adj[j1];
	    j1 = (v[i1].rev[j1]+1)%v[tmp].orig_valency;
	    i1 = tmp;
	  } while (i1 != i || j1 != j);

	  for (k = 0; k < fdeg; k++) {
	    tri[t].a = nvertices;
	    tri[t].b = v[nvertices].adj[k];
	    tri[t].c = v[nvertices].adj[(k+1)%fdeg];
	    t++;
	    if (t > ntriangles) {
	      fprintf(stderr, "Too many triangles\n" );
	      exit(1);
	    }
	  }
	}
      }
    }
  }
  if (t < ntriangles) {
    fprintf( stderr, "Only %d triangles instead of expected %d\n",
	     t, ntriangles );
    ntriangles = t;
  }
}


/* ------------------------------------------------------------------------ */

void
main_loop(int pull)
{
  int step=0;
  int i, j, k, z;
  double koef, temp, d;
  struct vertex *a, *b, *c;
  struct coor center, delta;

  for (step = 1; step <= MAX_ITERATIONS; step++) {
    for(i = 1; i <= nvertices; i++)
      v[i].disp.x = v[i].disp.y = 0.0;

    for (i = 0; i < ntriangles; i++) {
      a = &(v[tri[i].a]);
      b = &(v[tri[i].b]);
      c = &(v[tri[i].c]);

      koef = 0.5 * (   ( b->v.y - a->v.y ) * ( c->v.x - a->v.x )
		     - ( b->v.x - a->v.x ) * ( c->v.y - a->v.y ) );
      koef *= koef;
      /*
      koef *= koef;
      koef *= koef;
      koef *= koef;
      */

      center.x = ( a->v.x + b->v.x + c->v.x ) / 3.0;
      center.y = ( a->v.y + b->v.y + c->v.y ) / 3.0;

      a->disp.x += koef * (center.x - a->v.x);
      a->disp.y += koef * (center.y - a->v.y);
      b->disp.x += koef * (center.x - b->v.x);
      b->disp.y += koef * (center.y - b->v.y);
      c->disp.x += koef * (center.x - c->v.x);
      c->disp.y += koef * (center.y - c->v.y);
    }

    temp = 40.0 / exp( 4.0 * (double)step / ( (step<250) ? 250 : (step+1) ) );

    for(i = 1; i <= nvertices; i++) {
      if (v[i].fixed == 0) {
	d = norm(v[i].disp);
	if (d > temp)
	  koef = temp / d;
	else
	  koef = 1.0;

	v[i].v.x += v[i].disp.x * koef;
	v[i].v.y += v[i].disp.y * koef;
      }
    }
  }
}


void
crossproduct( struct coor *res, struct coor *v1, struct coor *v2 )
{
  res->x = v1->y - v2->y;
  res->y = v2->x - v1->x;
}


/*
** Local Variables:
** compile-command: "cc -O2 -o schlegel schlegel.c -lm"
** End:
*/

/* -- EOF schlegel.c -- */


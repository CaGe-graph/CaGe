#include <stdio.h>	
#include <stdlib.h>

#ifdef __alpha
#define LONGTYPE unsigned long int
#else
#define LONGTYPE unsigned long long int
#endif

#define MAXNV 30		/* the maximum number of vertices -- at most
				 * CHAR_MAX-1 */
#define MAXE (6*MAXNV - 12)	/* the maximum number of oriented edges */
#define MAXF (2*MAXNV-4)	/* the maximum number of faces */
#define MAXAUTS 12		/* the maximum number of automorphisms */
#define SPLITLEVEL 16		/* which is the level (number of hexagons)
				 * where the option m shall split it into
				 * parts */
#define EMPTY		(MAXNV+1)
#define UNNAMED		0	/* EMPTY */
#define OCCUPIED	1

#define Cs	0
#define C2v	1
#define C2h	2
#define D2h	3
#define C3h	4
#define D3h	5
#define C6h	6
#define D6h	7

#define MAX_NUM_MODES		35
/*
 * Why do we set MAX_NUM_MODES to 35?
 *
 * Let C be a two-connected component of an inner dual G and u be the number of
 * unsaturated connection edges of C. Further let M be a maximum matching of C*
 * and w be the number of white and b be the number of black vertices that are
 * unsaturated by M. We then have at most
 *
 *            /u\   /u\
 *        x = \w/ = \b/
 *
 * possibilities to distribute the unsaturated vertices on the connection
 * edges of C*. The largest value for x is reached for w = [u/2].
 * If |G| = 30, a component can have at most 7 unsaturated connection
 * edges (try it out!), so we define
 *
 *                         /7\
 *        MAX_NUM_MODES := \3/ = 35.
 *
 */

#define MAX_NUM_VAR_MODES	16	/* 16 is enough for a component with 4 connection edges
					 * with mode SINGLE (the first time this occurs for
					 * maxnv == 32) */

typedef struct e {		/* The data type used for edges */
	int             start;	/* vertex where the edge starts */
	int             end;	/* vertex where the edge ends */
	struct e       *prev;	/* previous edge in clockwise direction */
	struct e       *next;	/* previous edge in clockwise direction */
	struct e       *invers;	/* the edge that is inverse to this one */
	int             dummy1, dummy2;	/* ints for temporary use. Every
					 * function may use them but may
					 * NEVER rely on them not having
					 * changed after another function has
					 * been called */
	int             fwdlbl, bwdlbl;	/* for the canon_label function */

	char on_boundary;	/* the number of times the undirected edge occurs in the boundary */
	unsigned char bound_vertex;	/* if e = (v, w) is an edge on the boundary of the inner
					 * dual, and e^ = {x, y} is the i-dual edge of e with
					 * vertex x on the boundary of the fusen, then e->bound_vertex
					 * is set to x */
	struct e	*opposite;
	unsigned char length;
	char match;		/* != 0 if the edge is in the "matching" */
	char mode[MAX_NUM_MODES];
	char type;
	unsigned char component;	/* */
	char connection_edge;	/* */
} EDGE;

typedef struct e2 {		/* The shortened data type used for edges in
				 * the triangular net */
	char           *start;	/* char pointer to where the edge starts. The
				 * real name in the net is never needed. This
				 * is the name of the vertex mapped here and
				 * 0 in case no vertex was already mapped
				 * here. It is used as an integer pointer
				 * with all edges around one vertex pointing
				 * to the same integer, so that modification
				 * of the value can be done faster */
	char           *end;
	struct e2      *next;	/* previous edge in clockwise direction */
	struct e2      *nextnext;	/* = ->next->next */
	struct e2      *invers;	/* the edge that is inverse to this one */
} EDGE2;

typedef EDGE   *PLANMAP[MAXNV];	/* Planmap[i] is an arbitrary edge starting
				 * at vertex i. WHICH edge is chosen is
				 * arbitrary. No function can rely on this
				 * edge not changing after something "has
				 * been done to the graph" -- when returning,
				 * it might be another arbitrary edge. */

#define FALSE 0
#define TRUE  1

/* Global variables */

int             maxnv, nv;	/* The vertex number of the final graph --
				 * that is: The facenumber of the the dual
				 * (maxnv) and the temporary vertex number
				 * during the construction */

int             ne;		/* number of ORIENTED edges -- twice the
				 * number of unoriented edges */

int             C, H;		/* the number of C-atoms and H-atoms -- are
				 * computed evry time a skeleton is given to
				 * next_step */
int             CHdifference;	/* always equal to 2 times the number of
				 * hexagons -2 */
int             Cconstant;	/* always equal to 5*maxnv+1 */

EDGE           *boundary_edge;	/* This is always an edge with the boundary
				 * on the left side (prev-direction). It may
				 * only be altered in delete-vertex and
				 * add-vertex. It is always the edge leaving
				 * the last vertex and having the outer face
				 * on the left. */

EDGE           *boundary_edges[MAXNV];	/* Here the old boundary edges are
					 * stored in order to repair the
					 * boundary edge value after deletion
					 * of a vertex */

LONGTYPE        number_of_skeletons;
LONGTYPE        triv_skeletons;
LONGTYPE        number_of_labellings;
LONGTYPE        counter;	/* The counter how many structures are
				 * accepted (helicenes or benzenoids) */

LONGTYPE        formula[4 * MAXNV + 3][2 * MAXNV + 5];
/* formula[x][y] gives the number of structures with chemical formula C_x H_y */
LONGTYPE        groupformula[8][4 * MAXNV + 3][2 * MAXNV + 5];
/* The chemical formulas sorted with respect to their groups */

LONGTYPE        catas;	/* the number of catacondensed structures */
int             group;

char            catacondensed;	/* Is the structure catacondensed ? */
#define catanumber CHdifference
/*
 * the number of oriented edges in the dual of a catacondensed structure:
 * 2*maxnv-2 -- the same as Chdifference
 */

PLANMAP         map;		/* What it is all about: This contains the
				 * graph to be constructed */

int             degree[MAXNV];	/* The valency of the vertices */

int             components[MAXNV];	/* The number of boundary components
					 * of the vertex = the number of
					 * times it occurs in the boundary.
					 * For degree 6 it is 0, for degree 5
					 * or 1 it is one, for 2,4 it is one
					 * or two and for 3 it is one, two or
					 * three. The entry components[i]
					 * gives the number of components of
					 * vertex i for 0<=i<=nv. For larger
					 * i it is undefined. */

EDGE           *can_numberings[MAXAUTS][MAXE];	/* the canonical numberings */
int             number_can_numberings, number_can_numb_or_pres;
/*
 * The number of canonical numberings and orientation preserving canonical
 * numberings
 */

EDGE           *new_vertex[MAXNV][6];
/*
 * When a new vertex is added to the boundary, it is taken from this list.
 * The list contains (pointers to) edges leaving a vertex with some
 * pre-initialised edges and their invers. new_vertex[a][b] contains an edge
 * starting at vertex a and being pre-initialised to form a star with b
 * edges. new_vertex[x][0] is not used.
 */

char            possible_labels[7][4];	/* possible_labels[i][j] contains the
					 * number of labels for a vertex with
					 * degree i and j components. Fields
					 * corresponding to impossible
					 * combinations like
					 * possible_labels[6][2] are not
					 * initialized, so they contain a
					 * random value */

char            label[MAXNV];	/* The labels of the skeleton describing the
				 * helicene (1,2 or maximally 3) interpreted
				 * as: 1,2 or 3 boundary edges when occuring
				 * on the boundary for the first time after
				 * "boundary_edge" */

int             variable_positions[MAXNV];	/* list of vertices, where
						 * more than one label is
						 * possible */
int             number_of_possibilities[MAXNV];	/* how many possibilities are
						 * there ? This field is used
						 * differently in case of
						 * benzenoids and helicenes.
						 * For helicenes it gives the
						 * number of possibilities at
						 * the [i]th variable position
						 * -- for benzenoids it gives
						 * the number at vertex [i] */
int             maxlevel;	/* number of variable positions minus 1 */
/*
 * These variables are global in order not to pass them in the recursive call
 * of construct_labels
 */

int             global_init;	/* a marker whether the canon_label() routine
				 * is called for a new skeleton or the same
				 * one as before */
int             just_skeletons;		/* shall just skeletons be generated */
int             modulo, part;		/* shall the generation be split --
					 * which part is to be generated */
int             modulocounter;
int             pl_code_out, bec_out;		/* shall planarcode or BEC be
						 * written to outfile (stdout) ? */
int             pl_map_out;
char            *charp, prgname[50], logfilename[100];
int             just_count;	/* by default all isomers are really formed
				 * in the memory of the computer -- this can
				 * be switched off */
int             benzenoids;	/* shall just benzenoid structures be generated ? */
int		kekule;		/* shall just kekulean structures be generated? */
int             detailed;	/* shall additional data like the group and
				 * the formula becomputed and displayed ?
				 * Option: d */
FILE *outfile;			/* The file to write data to -- default:
				 * stdout */

/*
 * for the embedding in the net in case of benzenoids, the following
 * variables are used:
 */

EDGE2          *edgenet[MAXNV];	/* This is an edge in the net that has the
				 * outer face of the embedding on the right
				 * and is the first edge of the vertex
				 * [number] when running around the boundary
				 * starting from boundary_edge in clockwise
				 * direction. So the direction of the edge is
				 * against the running direction. */

int             embed_from_here[MAXNV + 1][2];	/* which vertices are
						 * embedded from here -- at
						 * most two they are listed
						 * in clockwise direction */

int             checkedges[MAXNV + 1];	/* how many edges must(may) be
					 * checked when the vertex is
					 * embedded. The checking starts at
					 * the second edge in ->next
					 * direction after the referenece
					 * edge for the embedding. This is
					 * sometimes more than just the
					 * number of boundary edges */

EDGE2          *startnet;	/* an edge in the center of the net used for
				 * embedding the helicenes when testing for
				 * being benzenoid */

int             jump[MAXNV];	/* How many edges are BETWEEN the first edge
				 * with boundary on the left leaving this
				 * vertex and the next one in clockwise
				 * direction ? This field is only valid for
				 * vertices with 2 boundary components. */
int             jumpname[MAXNV];/* What is the name of the vertex at the end
				 * of that edge ? */


void next_step(void);
void next_step_benzenoids(void);
int canon_label(EDGE *[], int, int, int);
void write_helicene_pl(FILE *);
void write_bec(FILE *);


/****************************** Christian: ******************************/

#define MAX_NUM_COMPS	14	/* The maximum number of components of the skeleton,
				 * i.e. the number of 2-edge-connected components
				 * plus the number of vertices v with degree[v] == 3
				 * and components[v] == 3 */

#define MAX_NUM_PATHS	(MAXNV - 4)		/* maximum number of connection paths */
#define MAX_NUM_CE	(MAX_NUM_COMPS - 1)	/* maximum number of connection edges
						 * of a component */

#define MAX_BOUNDARY	(3 * MAXNV + 3) 	/* maximum number of edges in the
						 * boundary of a (fusene) component */

/****************************** kekule.c ******************************/

int num_comps;			/* the number of "components" of the inner dual,
				 * i.e. 2-edge-connected components and vertices v
				 * with degree[v] == 3 and components[v] == 3 */

char isTcomp[MAX_NUM_COMPS];	/* whether a component consists only of a single vertex */

EDGE *comp_boundary_edge[MAX_NUM_COMPS];	/* comp_boundary_edge[comp] is an edge in
						 * the boundary of component comp */

int num_paths;
int path_length[MAX_NUM_PATHS];
EDGE *path_start_edge[MAX_NUM_PATHS];
int path_index[MAX_NUM_PATHS];

char cv_zero_label[MAXNV];
EDGE *conn_edge[MAX_NUM_COMPS][MAX_NUM_COMPS];
int num_conn_edges[MAX_NUM_COMPS];

char symmetric_case;

void write_up_kekule(void);
void find_conn_edges(int);
void next_step_kekule(void);
void next_step_kekule_benz(void);


/****************************** match.c ******************************/

int boundary_length;			/* number of vertices/edges in the boundary of a
					 * component of the fusene */

char bound_sat[MAX_BOUNDARY];		/* bound_sat[v] != 0 means, vertex v on the boundary
					 * is saturated */

char bound_match[MAX_BOUNDARY];		/* bound_match[e] != 0 means, edge e on the boundary
					 * is in the matching */
EDGE *bound_edge[MAX_BOUNDARY];

int first_bound_vertex[MAXNV];		/* for a connection vertex a, first_bound_vertex[a]
					 * is the first vertex of the boundary cycle of the
					 * fusene component, which lies in the face a^ */

char unsat[2];			/* unsat[0/1] = number of even/odd unsaturated vertexes */
char vis[MAXNV];		/* whether a vertex has been visited (used in DFS) */

#define PREV_EDGE(e)	(((e) == 0) ? (boundary_length - 1) : ((e) - 1))
#define NEXT_VERTEX(v)	(((v) == boundary_length - 1) ? 0 : ((v) + 1))

int init_match(int);
int match(void);

/****************************** modes.c ******************************/

#define ODD		1
#define EVEN		2
#define FORBIDDEN	(-1)

#define LENGTH(edge)		(edge->length)
#define TYPE(edge)		(edge->type)
#define MODE(edge, num)		(edge->mode[num])

#define SINGLE			0x01
#define DOUBLE			0x02
#define SINGLE_OR_DOUBLE	0x03
#define	LEFT			0x04
#define RIGHT			0x08
#define LEFT_OR_RIGHT		0x0c

#define VARIABLE		0x10

int num_modes[MAX_NUM_COMPS];

char var_modes[MAXNV][MAX_NUM_MODES][MAX_NUM_VAR_MODES];	/* */
int num_var_modes[MAX_NUM_COMPS][MAX_NUM_MODES];

int vm_comps[MAX_NUM_COMPS], num_vm_comps;
int is_vm_comp[MAX_NUM_COMPS];

EDGE *ce_var[MAX_NUM_COMPS][MAX_NUM_CE][MAX_NUM_MODES];
int num_ce_var[MAX_NUM_COMPS][MAX_NUM_MODES];

int modes(int);
int T_modes(int);

/****************************** label.c ******************************/

/* the label types for the connection paths: */
#define SUM_EVEN	0
#define SUM_ODD		1	/* important: SUM_EVEN has to be even, SUM_ODD to be odd */
#define ANY		2
#define ONE_ODD_LABEL	3
#define ALL_TWO		4

char label_type[MAX_NUM_PATHS];	/* how a path has to be labelled: either SUM_ODD, SUM_EVEN,
				   ONE_ODD_LABEL, ALL_TWO or ANY */

int current_mode[MAX_NUM_COMPS];

void label_sum(int, int, int, char, int);
void label_any(int, int, int);
void label_one_odd(int, int, int, char);
void label_all_two(int);

int split_paths[MAX_NUM_PATHS], num_split_paths;
char first_labelling;

#define MAX_NUM_SPLIT_PATHS	8

char have_labelled[1 << MAX_NUM_SPLIT_PATHS];
char must_split;

char get_mode(EDGE *);
int get_label_types(void);
void select_connection_edges(int, int);


/**************************** label_benz.c ***************************/

#define START_OF_PATH	1
#define LAST_ON_PATH	2

int path_of[MAXNV];		/* path[v] is the number of the path v lies on,
				 * or EMPTY_PATH */
char path_flag[MAXNV];		/* */

EDGE *ce_of[MAXNV];		/* if v is the start vertex of a connection edge e then
				 * ce_of[v] == e */

void select_connection_edges_benz(int, int);
void init_embedding(void);

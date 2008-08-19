#include "fusgen.h"

#define COUNTERCLOCKWISE	(-1)
#define CLOCKWISE		1

static char used[MAX_BOUNDARY];
static char found;
static void alt_path_intern(EDGE *);
static void alt_path_extern(int, char);

static EDGE *comp_edges[MAXE];
static int num_comp_edges;

static void
init_match_int(EDGE *e, int ori)
{
	EDGE *t;
	int next_ori;

	vis[e->start]++;

	t = e;
	do {
		if (t->on_boundary < 2) {
			comp_edges[num_comp_edges++] = t;
			if (ori % 3 == 0)
				t->match = t->invers->match = 1;

			if (!vis[t->end]) {
				if (t->dummy1) { /* edge is in clockwise direction on the boudary */
					switch (degree[t->end]) {
					case 2:
					case 5:
						next_ori = ori - 1;
						break;
					case 3:
						next_ori = ori + components[t->end];
						break;
					case 4:
						if (components[t->end] == 2) {
							if (t->invers->next->on_boundary == 2)
								next_ori = ori + 1;
							else
								next_ori = ori - 1;
						} else
							next_ori = ori;
						break;
					default:
						fprintf(stderr, "impossible: degree %d on boundary\n",
							degree[t->end]);
						exit(1);
					}
				} else
					next_ori = ori + 1;

				init_match_int(t->invers->next, next_ori);
			}

			if (t->invers->dummy1) { /* t is in counterclockwise direction on the boudary */
				switch (degree[t->start]) {
				case 2:
				case 5:
					ori--;
					break;
				case 3:
					ori += components[t->start];
					break;
				case 4:
					if (components[t->start] == 2) {
						if (t->next->on_boundary == 2)
							ori++;
						else
							ori--;
					}
					break;
				default:
					fprintf(stderr, "impossible: degree %d on boundary\n",
						degree[t->start]);
					exit(1);
				}
			} else
				ori++;
		}
	} while ((t = t->next) != e);
}

static void
init_match_ext(EDGE *b)
{
	EDGE *t, *last;
	char m, last_m;
	int i, n;

	unsat[0] = unsat[1] = 0;
	last = (b->prev->on_boundary < 2) ? b->prev->invers : b->prev->prev->invers;
	last->bound_vertex = last->invers->bound_vertex = 0;
	bound_edge[0] = last;
	last_m = 0;
	i = 0;
	t = b;
	while (1) {
		first_bound_vertex[t->start] = i;

		if (degree[t->start] == 4) {
			if (components[t->start] == 2) {
				n = (t->prev->on_boundary == 2) ? 3 : 1;
			} else {
				n = 2;
			}
		} else
			n = 5 - degree[t->start] + components[t->start];

		m = !last_m;
		while (--n > 0) {
			bound_match[i] = m;
			bound_sat[i] = 1;
			last_m = m;
			m = !m;
			i++;
			bound_edge[i] = NULL;
		}

		if (t != last) {
			bound_match[i] = !(last_m || t->match);
			bound_sat[i] = last_m || bound_match[i];
			last_m = bound_match[i] || t->match;
			unsat[i % 2] += !bound_sat[i];
			i++;
			t->bound_vertex = t->invers->bound_vertex = i;
			bound_edge[i] = t;
			t = t->invers->next;
			if (t->on_boundary == 2)
				t = t->next;
		} else {
			bound_match[i] = 0;
			bound_sat[i] = last_m;
			unsat[1] += !last_m;
			boundary_length = i + 1;
			return;
		}
	}
}

static void
alt_path_intern(EDGE *e)
{
	char m;
	int be;

	e->dummy2 = e->invers->dummy2 = 1;	/* dummy2 == 1 if e is used in DFS */
	m = !e->match;

	if (e->dummy1 || e->on_boundary == 0) { /* e is internal edge or in clockwise direction
						 * on boundary */
		if (!found && e->next->match == m && !e->next->dummy2)
			alt_path_intern(e->next);
		if (!found && e->invers->prev->match == m && !e->invers->prev->dummy2)
			alt_path_intern(e->invers->prev->invers);
	} else {			 /* e is in counterclockwise direction on boundary */
		if (!bound_sat[e->bound_vertex]) {
			found++;
			bound_sat[e->bound_vertex]++;
		} else {
			be = e->bound_vertex;
			if (!found && bound_match[be] == m && !used[be])
				alt_path_extern(be, CLOCKWISE); /* search clockwise around boundary */
			be = PREV_EDGE(e->bound_vertex);
			if (!found && bound_match[be] == m && !used[be])
				alt_path_extern(be, COUNTERCLOCKWISE); /* search counterclockwise */
		}
	}
	if (found)
		e->match = e->invers->match = !e->match;
}

static void
alt_path_extern(int e, char dir)
{
	int end_v, next_edge;
	char m;

	used[e]++;

	if (dir == COUNTERCLOCKWISE) {
		end_v = e;
		next_edge = PREV_EDGE(e);
	} else {
		end_v = NEXT_VERTEX(e);
		next_edge = end_v;
	}

	if (!bound_sat[end_v]) {
		found++;
		bound_sat[end_v]++;
		bound_match[e]++;
		return;
	}

	m = !bound_match[e];
	if (!found && !used[next_edge] && bound_match[next_edge] == m)
		alt_path_extern(next_edge, dir);

	if (!found && bound_edge[end_v] != NULL)
		if (bound_edge[end_v]->match == m && !bound_edge[end_v]->dummy2)
			alt_path_intern(bound_edge[end_v]);

	if (found)
		bound_match[e] = !bound_match[e];
}

int
match(void)
/* constructs a maximum matching; returns the number of unsaturated vertices */
{
	int i, j;
	char must_search;

	i = 0;
	must_search = unsat[0] && unsat[1];
	while (must_search) {
		while (bound_sat[i] && (i < boundary_length))
			i++;
		if (i == boundary_length)
			return unsat[0] + unsat[1];

		for (j = 0; j < num_comp_edges; j++)
			comp_edges[j]->dummy2 = 0;

		for (j = 0; j < boundary_length; j++)
			used[j] = 0;

		found = 0;
		if (bound_edge[i] != NULL) /* i has degree 3, start searching at the internal edge */
			alt_path_intern(bound_edge[i]);
		if (!found && bound_match[i] != FORBIDDEN)
			alt_path_extern(i, CLOCKWISE);
		if (!found && bound_match[PREV_EDGE(i)] != FORBIDDEN)
			alt_path_extern(PREV_EDGE(i), COUNTERCLOCKWISE);
		if (found) {
			bound_sat[i]++;
			unsat[0]--; unsat[1]--;
			must_search = unsat[0] && unsat[1];
		} else { /* are there still unsaturated vertices in both color classes without i? */
			if (i % 2)
				must_search = unsat[0] && (unsat[1] - 1);
			else
				must_search = (unsat[0] - 1) && unsat[1];
		}
		i++;
	}
	return unsat[0] + unsat[1];
}

int
init_match(int comp)
{
	EDGE *b;
	int i;

	b = comp_boundary_edge[comp];
	for (i = 0; i < nv; i++)	/* initialize vis[] for init_match_int() */
		vis[i] = 0;
	num_comp_edges = 0;

	/* match all edges in component whose 'orientation' is 0, i.e. that of b */
	init_match_int(b, 0);	
	/* now try to match the "edges" on the boundary as good as possible */
	init_match_ext(b);

	if (unsat[0] && unsat[1])
		return match();
	else
		return unsat[0] + unsat[1];
}


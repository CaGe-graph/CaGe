#include "fusgen.h"

static char must_label;

void
find_components_benz(void)
{
	EDGE *s, *t;
	int v, len=0;
        s = NULL;

	num_comps = num_paths = 0;
	must_label = 0;
	t = boundary_edge;
	do {
		t->dummy1++;	/* dummy1 != 0 if t is in clockwise direction on the boundary */
		t->on_boundary++; t->invers->on_boundary++;	/* t->on_boundary is the number of times
								 * the undirected edge t occurs in the
								 * boudary cycle */
		if (t->on_boundary == 2) {
			v = t->start;
			switch (degree[v]) {
			case 1:
				TYPE(t) = DOUBLE;
				/* we are at the start of new a path */
				s = t;
				len = 0;
				break;
			case 2:
				/* we are on a path */
				len++;
				path_of[v] = num_paths;
				if (!must_label) must_label++;
				break;
			case 3:
				if (components[v] == 3) {
					/* v is a "single vertex component" */
					isTcomp[num_comps]++;
					cv_zero_label[v] = 1;
					comp_boundary_edge[num_comps] = t;
					t->component = t->prev->component = t->next->component = num_comps;
					conn_edge[num_comps][0] = t;
					conn_edge[num_comps][1] = t->next;
					conn_edge[num_comps][2] = t->prev;
					num_conn_edges[num_comps] = 3;
					num_comps++;
					TYPE(t) = TYPE(t->next) = TYPE(t->prev) = 0;
				} else {
					TYPE(t) = 0;
					cv_zero_label[v] = 0;
					if(!must_label) must_label++;
					/* we have surrounded a component */
					comp_boundary_edge[num_comps++] = t->next;
					ce_of[v] = t;
				}
				/* also, here starts a new path */
				s = t;
				len = 0;
				break;
			case 4:
				TYPE(t) = 0;
				cv_zero_label[v] = 1;
				/* here too, we have surrounded a component... */
				comp_boundary_edge[num_comps++] = t->next;
				/* ...and are at the start of new a path */
				s = t;
				len = 0;
			}

			v = t->end;
			switch (degree[v]) {
			/* in cases 3 and 4, we are at the end of a path */
			case 3:
				if (components[v] != 3) {
					TYPE(t->invers) = 0;
					cv_zero_label[v] = 0;
					if (!must_label) must_label++;
				} else {
					cv_zero_label[v] = 1;
				}
				t->invers->opposite = s;
				s->opposite = t->invers;
				LENGTH(t->invers) = LENGTH(s) = len;
				path_start_edge[num_paths] = s;
				path_length[num_paths] = len;
				if (len > 0)
					path_flag[s->end] = LAST_ON_PATH;
				path_flag[v] = START_OF_PATH;
				ce_of[v] = t->invers;
				num_paths++;
				break;
			case 4:
				TYPE(t->invers) = 0;
				t->invers->opposite = s;
				s->opposite = t->invers;
				LENGTH(t->invers) = LENGTH(s) = len;
				cv_zero_label[v] = 1;
				if (len || degree[s->start] != 1) {
					path_start_edge[num_paths] = s;
					path_length[num_paths] = len;
					if (len > 0)
						path_flag[s->end] = LAST_ON_PATH;
					num_paths++;
				}			
			}
		}
	} while ((t = t->invers->next) != boundary_edge);

	if (degree[boundary_edge->start] == 1) {
		TYPE(boundary_edge) = DOUBLE;
		boundary_edge->opposite = s;
		s->opposite = boundary_edge;
		LENGTH(boundary_edge) = LENGTH(s) = len;
		if (len || (degree[s->start] != 4 && components[s->start] == 2)) {
			path_start_edge[num_paths] = s;
			path_length[num_paths] = len;
			if (len)
				path_flag[s->end] = LAST_ON_PATH;
			num_paths++;				
		}
	} else { /* boundary_edge is in the boundary of the last component */
		comp_boundary_edge[num_comps++] = boundary_edge;
	}
}

void
next_step_kekule_benz()
{
	int i, comp, nmodes;
	EDGE *t;

	if (ne == catanumber) {
		next_step_benzenoids();
		return;
	}
	if (number_can_numberings == 1)
		triv_skeletons++;
	number_of_skeletons++;

	if (detailed) {
		C = Cconstant - (ne >> 1);
		H = C - CHdifference;;
	}

	if ((nv + (ne >> 1)) % 2 == 0)	/* The resulting benzenoid had an odd number of vertices and */
		return; 		/* would therefore be non-kelulean */

	/* Initialisations */
	for (i = 0; i < nv; i++) {
		number_of_possibilities[i] = possible_labels[degree[i]][components[i]];
		path_flag[i] = 0;
		label[i] = 0;
		t = map[i];
		do {
			t->dummy1 = t->on_boundary = t->match = 0;
		} while ((t = t->next) != map[i]);
	}

	for (i = 0; i < MAX_NUM_COMPS; i++) {
		isTcomp[i] = 0;
	}

	find_components_benz();

	if (num_comps == 1) {
		if(init_match(0)) {
			return;
		} else {
			find_conn_edges(0);
			for (i = 0; i < num_conn_edges[0]; i++) {
				MODE(conn_edge[0][i], 0) = DOUBLE;
			}
			num_modes[0] = 1;
		}
	} else {
		/* compute the modes for the components: */
		num_vm_comps = 0;
		for (comp = 0; comp < num_comps; comp++) {
			if (isTcomp[comp]) {
				nmodes = T_modes(comp);
			} else {
				find_conn_edges(comp);
				nmodes = modes(comp);
			}
			if(nmodes == 0) {
				return;
			}
		}
	}
	if ((number_can_numberings == 1) || !must_label) {
		symmetric_case = 0;
	} else {
		symmetric_case = 1;
		global_init = 1;	/* next time the canon_label routine is called,
					 * it must be initialised for a new skeleton */
	}

	if (num_vm_comps) { /* determine the paths for which we have to split the labellings */
		num_split_paths = 0;
		for (i = 0; i < num_paths; i++) {
			t = path_start_edge[i];
			if (MODE(t, 0) == VARIABLE || MODE(t->opposite, 0) == VARIABLE) {
				split_paths[num_split_paths++] = i;
			}
		}
	}

	/* prepare for the embedding */
	init_embedding();

	/* initiate the labelling process */
	select_connection_edges_benz(0, 0);

	/* restore the net after the embedding: */
	*(startnet->start) = *(startnet->end) = UNNAMED;
}

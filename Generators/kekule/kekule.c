#include "fusgen.h"

static char must_label;

void
write_up_kekule(void)
/* like write_up(), but with test for symmetric case */
{
	static EDGE *starts[MAXAUTS];
	int i, j;
	char is_canon;

	if (symmetric_case) {
		if (global_init) {
			for (i = 0; can_numberings[0][i] != boundary_edge; i++);
			for (j = 0; j < number_can_numberings; j++)
				starts[j] = can_numberings[j][i];
		}

		is_canon = canon_label(starts, number_can_numb_or_pres, number_can_numberings,
							global_init);
		global_init = 0;
		if (!is_canon) {
			return;
		}
	}
	counter++;

	if (detailed)
		groupformula[group][C][H]++;

	if (just_count)
		return;

	if (pl_code_out)
		write_helicene_pl(outfile);
	else if (bec_out)
		write_bec(outfile);
}

void
find_conn_edges(int comp)
{
	EDGE *t;
	int i;

	t = comp_boundary_edge[comp];
	i = 0;
	do {
		t = t->invers->next;
		if (t->on_boundary == 2) {
			conn_edge[comp][i++] = t;
			t->component = comp;
			t = t->next;
		}
	} while (t != comp_boundary_edge[comp]);
	num_conn_edges[comp] = i;	/* number of connection edges of the component */
}

void
find_components(void)
{
	EDGE *s, *t;
	int v, len, i;

	num_comps = num_paths = 0;
	must_label = 0;
	i = 0;
	t = boundary_edge;
	do {
		t->dummy1++;	/* dummy1 != 0 if t is in clockwise direction on the boundary */
		t->on_boundary++; t->invers->on_boundary++;
		/* t->on_boundary is the number of times the undirected edge t occurs
		 * in the boudary cycle */

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
				variable_positions[i + len++] = v;
				if (!must_label) must_label++;
				break;
			case 3:
				if (components[v] == 3) {
					/* v is a "single vertex component" */
					isTcomp[num_comps]++;
					cv_zero_label[v] = 1;
					comp_boundary_edge[num_comps] = t;
					t->component = t->prev->component =
						t->next->component = num_comps;
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
				t->invers->opposite = s;
				s->opposite = t->invers;
				LENGTH(t->invers) = LENGTH(s) = len;
				if (components[v] == 3) {
					cv_zero_label[v] = 1;
					if (len || degree[s->start] != 1) {
						path_start_edge[num_paths] = s;
						path_length[num_paths] = len;
						path_index[num_paths] = i;
						i += len;
						num_paths++;
					}
				} else {
					TYPE(t->invers) = 0;
					cv_zero_label[v] = 0;
					if (!must_label) must_label++;
					path_start_edge[num_paths] = s;
					path_length[num_paths] = len;
					path_index[num_paths] = i;
					i += len;
					num_paths++;
				}
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
					path_index[num_paths] = i;
					i += len;
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
			path_index[num_paths] = i;
			num_paths++;				
		}
	} else { /* boundary_edge is in the boundary of the last component */
		comp_boundary_edge[num_comps++] = boundary_edge;
	}
}

void
next_step_kekule()
{
	int i, comp, nmodes;
	EDGE *t;

	if (ne == catanumber) {	/* The fusenes will be catacondensed, therefore kekulean */
		next_step();
		return;
	}

	if (number_can_numberings == 1)
		triv_skeletons++;
	number_of_skeletons++;

	if (detailed) {
		C = Cconstant - (ne >> 1);
		H = C - CHdifference;;
	}

	if ((nv + (ne >> 1)) % 2 == 0)	/* The resulting fusene had an odd number of vertices */
		return; 		/* and would therefore be non-kelulean */

	/* Initialisations */
	for (i = 0; i < nv; i++) {
		label[i] = 0;
		t = map[i];
		do {
			t->dummy1 = t->on_boundary = t->match = 0;
		} while ((t = t->next) != map[i]);
	}

	for (i = 0; i < MAX_NUM_COMPS; i++) {
		isTcomp[i] = 0;
	}

	find_components();

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

	/* start labelling routine or write up, if all labels are zero: */
	symmetric_case = 0;
	if (must_label) {
		if (number_can_numberings > 1) {
			symmetric_case = 1;
			global_init = 1;	/* next time the canon_label routine is called,
						 * it must be initialised for a new skeleton */
		}

		if (num_vm_comps) {
			/* determine the paths for which we have to split the labellings */
			num_split_paths = 0;
			for (i = 0; i < num_paths; i++) {
				t = path_start_edge[i];
				if (MODE(t, 0) == VARIABLE || MODE(t->opposite, 0) == VARIABLE) {
					split_paths[num_split_paths++] = i;
				}
			}
		}
		/* initiate the labelling process */
		select_connection_edges(0, 0);
	} else {
		write_up_kekule();
	}
}

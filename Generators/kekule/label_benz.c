#include "fusgen.h"

void embed_benzenoid_kekule(int *);

void embed_sum(int *, char, int);
void embed_one_odd(int *, char);
void embed_all_two(int *);

static int to_place[MAXNV + 1];

/*
 * The code of the following embedding routines is stolen from Gunnar Brinkmann's
 * construct_labels_benzenoid() and embed_benzenoid() routines!
 */

void
embed_any(int *which)
{
	int	vertex, fixvertex;
	EDGE2	*run_net, *buffer, *run2;
	char	*charp;

	vertex = *which;

	run_net = edgenet[vertex]->nextnext;
	fixvertex = embed_from_here[vertex][0];

	if (*run_net->invers->nextnext->nextnext->end != UNNAMED) {
		/* At a position neighbouring both the faces where
		 * the hexagon is placed for label 1 and 2 a forbidden
		 * neighbour was detected, so only 3 is possible */
		run_net = run_net->nextnext;
		buffer = run_net->invers;
		run2 = buffer->nextnext;
		if (*run2->end == UNNAMED) {
			run2 = run2->next;
			if (*run2->end == UNNAMED) {
				run2 = run2->next;
				if (*run2->end == UNNAMED) {
					label[vertex] = 3;
					edgenet[fixvertex] = buffer;
					charp = buffer->start;
					*charp = OCCUPIED /* +fixvertex */ ;
					if (path_flag[vertex]) {
						embed_benzenoid_kekule(which + 1);
					} else {
						embed_any(which + 1);
					}
					*charp = UNNAMED;
				}
			}
		}
	} else { /* that is: this face is OK -- and need not be checked again */
		/* OK first try label 1: */
		buffer = run_net->invers;
		run2 = buffer->nextnext;
		if (*run2->end == UNNAMED) {
			run2 = run2->next;
			if (*run2->end == UNNAMED) {
				label[vertex] = 1;
				edgenet[fixvertex] = buffer;
				charp = buffer->start;
				*charp = OCCUPIED /* +fixvertex */ ;
				if (path_flag[vertex]) {
					embed_benzenoid_kekule(which + 1);
				} else {
					embed_any(which + 1);
				}
				*charp = UNNAMED;
			}
		}
		run_net = run_net->next;
		buffer = run_net->invers;
		if (*buffer->nextnext->nextnext->end == UNNAMED) {
			/* otherwise this time the face between
			 * position 2 and 3 made a problem */
			buffer = run_net->invers;
			run2 = buffer->nextnext->next;
			if (*run2->end == UNNAMED) {
				label[vertex] = 2;
				edgenet[fixvertex] = buffer;
				charp = buffer->start;
				*charp = OCCUPIED /* +fixvertex */ ;
				if (path_flag[vertex]) {
					embed_benzenoid_kekule(which + 1);
				} else {
					embed_any(which + 1);
				}
				*charp = UNNAMED;
			}
			run_net = run_net->next;
			buffer = run_net->invers;
			run2 = buffer->nextnext->next;
			if (*run2->end == UNNAMED) {
				run2 = run2->next;
				if (*run2->end == UNNAMED) {
					label[vertex] = 3;
					edgenet[fixvertex] = buffer;
					charp = buffer->start;
					*charp = OCCUPIED /* +fixvertex */ ;
					if (path_flag[vertex]) {
						embed_benzenoid_kekule(which + 1);
					} else {
						embed_any(which + 1);
					}
					*charp = UNNAMED;
				}
			}
		}
	}
}

void
embed_sum(int *which, char how, int sum)
{
	int	vertex, fixvertex;
	EDGE2	*run_net, *buffer, *run2;
	char	*charp;

	vertex = *which;

	run_net = edgenet[vertex]->nextnext;
	fixvertex = embed_from_here[vertex][0];

	if (*run_net->invers->nextnext->nextnext->end != UNNAMED) {
		/* At a position neighbouring both the faces where
		 * the hexagon is placed for label 1 and 2 a forbidden
		 * neighbour was detected, so only 3 is possible */
		run_net = run_net->nextnext;
		buffer = run_net->invers;
		run2 = buffer->nextnext;
		if (*run2->end == UNNAMED) {
			run2 = run2->next;
			if (*run2->end == UNNAMED) {
				run2 = run2->next;
				if (*run2->end == UNNAMED) {
					label[vertex] = 3;
					edgenet[fixvertex] = buffer;
					charp = buffer->start;
					*charp = OCCUPIED /* +fixvertex */ ;
					if (path_flag[vertex]) {
						if ((sum + 3) % 2 == how)
							embed_benzenoid_kekule(which + 1);
					} else {
						embed_sum(which + 1, how, sum + 3);
					}
					*charp = UNNAMED;
				}
			}
		}
	} else { /* that is: this face is OK -- and need not be checked again */
		/* OK first try label 1: */
		buffer = run_net->invers;
		run2 = buffer->nextnext;
		if (*run2->end == UNNAMED) {
			run2 = run2->next;
			if (*run2->end == UNNAMED) {
				label[vertex] = 1;
				edgenet[fixvertex] = buffer;
				charp = buffer->start;
				*charp = OCCUPIED /* +fixvertex */ ;
				if (path_flag[vertex]) {
					if ((sum + 1) % 2 == how)
						embed_benzenoid_kekule(which + 1);
				} else {
					embed_sum(which + 1, how, sum + 1);
				}
				*charp = UNNAMED;
			}
		}
		run_net = run_net->next;
		buffer = run_net->invers;
		if (*buffer->nextnext->nextnext->end == UNNAMED) {
			/* otherwise this time the face between
			 * position 2 and 3 made a problem */
			buffer = run_net->invers;
			run2 = buffer->nextnext->next;
			if (*run2->end == UNNAMED) {
				label[vertex] = 2;
				edgenet[fixvertex] = buffer;
				charp = buffer->start;
				*charp = OCCUPIED /* +fixvertex */ ;
				if (path_flag[vertex]) {
					if ((sum + 2) % 2 == how)
						embed_benzenoid_kekule(which + 1);
				} else {
					embed_sum(which + 1, how, sum + 2);
				}
				*charp = UNNAMED;
			}
			run_net = run_net->next;
			buffer = run_net->invers;
			run2 = buffer->nextnext->next;
			if (*run2->end == UNNAMED) {
				run2 = run2->next;
				if (*run2->end == UNNAMED) {
					label[vertex] = 3;
					edgenet[fixvertex] = buffer;
					charp = buffer->start;
					*charp = OCCUPIED /* +fixvertex */ ;
					if (path_flag[vertex]) {
						if ((sum + 3) % 2 == how)
							embed_benzenoid_kekule(which + 1);
					} else {
						embed_sum(which + 1, how, sum + 3);
					}
					*charp = UNNAMED;
				}
			}
		}
	}
}

void
embed_one_odd(int *which, char has_odd)
{
	int	vertex, fixvertex;
	EDGE2	*run_net, *buffer, *run2;
	char	*charp;

	vertex = *which;

	run_net = edgenet[vertex]->nextnext;
	fixvertex = embed_from_here[vertex][0];

	if (*run_net->invers->nextnext->nextnext->end != UNNAMED) {
		/* At a position neighbouring both the faces where
		 * the hexagon is placed for label 1 and 2 a forbidden
		 * neighbour was detected, so only 3 is possible */
		run_net = run_net->nextnext;
		buffer = run_net->invers;
		run2 = buffer->nextnext;
		if (*run2->end == UNNAMED) {
			run2 = run2->next;
			if (*run2->end == UNNAMED) {
				run2 = run2->next;
				if (*run2->end == UNNAMED) {
					label[vertex] = 3;
					edgenet[fixvertex] = buffer;
					charp = buffer->start;
					*charp = OCCUPIED /* +fixvertex */ ;
					if (path_flag[vertex]) {
						/* ok, this label is odd */
						embed_benzenoid_kekule(which + 1);
					} else {
						embed_one_odd(which + 1, 1);
					}
					*charp = UNNAMED;
				}
			}
		}
	} else { /* that is: this face is OK -- and need not be checked again */
		/* OK first try label 1: */
		buffer = run_net->invers;
		run2 = buffer->nextnext;
		if (*run2->end == UNNAMED) {
			run2 = run2->next;
			if (*run2->end == UNNAMED) {
				label[vertex] = 1;
				edgenet[fixvertex] = buffer;
				charp = buffer->start;
				*charp = OCCUPIED /* +fixvertex */ ;
				if (path_flag[vertex]) {
					/* label is odd */
					embed_benzenoid_kekule(which + 1);
				} else {
					embed_one_odd(which + 1, 1);
				}
				*charp = UNNAMED;
			}
		}
		run_net = run_net->next;
		buffer = run_net->invers;
		if (*buffer->nextnext->nextnext->end == UNNAMED) {
			/* otherwise this time the face between
			 * position 2 and 3 made a problem */
			buffer = run_net->invers;
			run2 = buffer->nextnext->next;
			if (*run2->end == UNNAMED) {
				label[vertex] = 2;
				edgenet[fixvertex] = buffer;
				charp = buffer->start;
				*charp = OCCUPIED /* +fixvertex */ ;
				if (path_flag[vertex]) {
					if (has_odd)
						embed_benzenoid_kekule(which + 1);
				} else {
					embed_one_odd(which + 1, has_odd);
				}
				*charp = UNNAMED;
			}
			run_net = run_net->next;
			buffer = run_net->invers;
			run2 = buffer->nextnext->next;
			if (*run2->end == UNNAMED) {
				run2 = run2->next;
				if (*run2->end == UNNAMED) {
					label[vertex] = 3;
					edgenet[fixvertex] = buffer;
					charp = buffer->start;
					*charp = OCCUPIED /* +fixvertex */ ;
					if (path_flag[vertex]) {
						embed_benzenoid_kekule(which + 1);
					} else {
						embed_one_odd(which + 1, 1);
					}
					*charp = UNNAMED;
				}
			}
		}
	}
}

void
embed_all_two(int *which)
{
	int	vertex, fixvertex;
	EDGE2	*run_net, *buffer, *run2;
	char	*charp;

	vertex = *which;

	run_net = edgenet[vertex]->nextnext;
	fixvertex = embed_from_here[vertex][0];

	if (*run_net->invers->nextnext->nextnext->end == UNNAMED) {
		run_net = run_net->next;
		buffer = run_net->invers;
		if (*buffer->nextnext->nextnext->end == UNNAMED) {
			/* otherwise this time the face between
			 * position 2 and 3 made a problem */
			buffer = run_net->invers;
			run2 = buffer->nextnext->next;
			if (*run2->end == UNNAMED) {
				label[vertex] = 2;
				edgenet[fixvertex] = buffer;
				charp = buffer->start;
				*charp = OCCUPIED /* +fixvertex */ ;
				if (path_flag[vertex]) {
					embed_benzenoid_kekule(which + 1);
				} else {
					embed_all_two(which + 1);
				}
				*charp = UNNAMED;
			}
		}
	}
}

void
embed_benzenoid_kekule(int *which)
{
	int             vertex, fixvertex, jumpvertex, i, j, test, poss;
	EDGE2          *startedge, *startedgenn, *run_net, *buffer, *run2;
	char           *delete;	/* is there an entry that must be reset to UNNAMED */
	char           *charp, lt, gen_label;

	vertex = *which;

	if (vertex == EMPTY) {
		write_up_kekule();
		return;
	}

	startedge = edgenet[vertex];

	delete = NULL;

	startedgenn = startedge->nextnext;

	/* OK -- then let's embed the vertex */
	if (number_of_possibilities[vertex] == 1) {
		/* some edgenet[] entries have to be initialized */
		if (components[vertex] == 1) {	/* One vertex has to be embedded from here: */
			fixvertex = embed_from_here[vertex][0];
			run_net = startedgenn;
			for (i = checkedges[vertex]; i != 1; i--)
				run_net = run_net->next;
			/* to find the place to insert the vertex */
			buffer = run_net->invers;
			poss = checkedges[fixvertex];
			run2 = buffer->nextnext;
			if (poss == 3) {
				if (*run2->end == UNNAMED) {
					run2 = run2->next;
					if (*run2->end == UNNAMED) {
						run2 = run2->next;
						if (*run2->end == UNNAMED) {
							edgenet[fixvertex] = buffer;
							delete = buffer->start;
							*delete = OCCUPIED /* +fixvertex */ ;
						} else
							return;
					} else
						return;
				} else
					return;
			} else if (poss == 4) {
				if (*run2->end == UNNAMED) {
					run2 = run2->next;
					if (*run2->end == UNNAMED) {
						run2 = run2->next;
						if (*run2->end == UNNAMED) {
							run2 = run2->next;
							if (*run2->end == UNNAMED) {
								edgenet[fixvertex] = buffer;
								delete = buffer->start;
								*delete = OCCUPIED /* +fixvertex */ ;
							} else
								return;
						} else
							return;
					} else
						return;
				} else
					return;
			} else if (poss == 2) {
				if (*run2->end == UNNAMED) {
					run2 = run2->next;
					if (*run2->end == UNNAMED) {
						edgenet[fixvertex] = buffer;
						delete = buffer->start;
						*delete = OCCUPIED /* +fixvertex */ ;
					} else
						return;
				} else
					return;
			} else if (poss == 1) {
				if (*run2->end == UNNAMED) {
					edgenet[fixvertex] = buffer;
					delete = buffer->start;
					*delete = OCCUPIED /* +fixvertex */ ;
				} else
					return;
			} else if (poss == 0) {
				edgenet[fixvertex] = buffer;
				delete = buffer->start;
				*delete = OCCUPIED /* +fixvertex */ ;
			}
			embed_benzenoid_kekule(which + 1);
		} else if (components[vertex] == 2) {
			if ((jumpvertex = embed_from_here[vertex][1]) >= 0) {
				run_net = startedgenn;
				for (i = checkedges[vertex]; i != 1; i--)
					run_net = run_net->next;
				buffer = run_net->invers;
				test = 1;
				run2 = buffer->nextnext;
				for (j = checkedges[jumpvertex]; j; j--) {
					if (*run2->end != UNNAMED)
						test = 0;
					run2 = run2->next;
				}
				if (test) {
					edgenet[jumpvertex] = buffer;
					delete = buffer->start;
					*delete = OCCUPIED /* +jumpvertex */ ;
				} else
					return;
			}
			fixvertex = embed_from_here[vertex][0]; /* This one is ALWAYS embedded after vertex */
			buffer = startedgenn->invers;
			test = 1;
			run2 = buffer->nextnext;
			for (j = checkedges[fixvertex]; j; j--) {
				if (*run2->end != UNNAMED)
					test = 0;
				run2=run2->next;
			}
			if (test) {
				edgenet[fixvertex] = buffer;
				charp = buffer->start;
				*charp = OCCUPIED /* +fixvertex */ ;
				embed_benzenoid_kekule(which + 1);
				*charp = UNNAMED;
			}
		} else {	/* that is: (components[vertex] == 3) -- the
				 * other vertices MUST be embedded later and
				 * we know that 3 places must be checked */
			buffer = startedgenn->invers;
			fixvertex = embed_from_here[vertex][0];
			run2 = buffer->nextnext;
			if (*run2->end == UNNAMED) {
				run2 = run2->next;
				if (*run2->end == UNNAMED) {
					run2 = run2->next;
					if (*run2->end == UNNAMED) {
						delete = buffer->start;
						edgenet[fixvertex] = buffer;
						*delete = OCCUPIED /* +fixvertex */ ;
					} else
						return;
				} else
					return;
			} else
				return;

			fixvertex = embed_from_here[vertex][1];
			buffer = startedgenn->nextnext->invers;

			run2 = buffer->nextnext;
			if (*run2->end == UNNAMED) {
				run2 = run2->next;
				if (*run2->end == UNNAMED) {
					run2 = run2->next;
					if (*run2->end == UNNAMED) {
						edgenet[fixvertex] = buffer;
						charp = buffer->start;
						*charp = OCCUPIED /* +fixvertex */ ;
						embed_benzenoid_kekule(which + 1);
						*charp = UNNAMED;
					}
				}
			}
		}		/* end 3 components */
	/* end 1 possibility */
	} else {		/* number of components is always 2 */
		poss = number_of_possibilities[vertex];
		if (poss == 3) {
			/* we are on a path and invoke the appropriate labelling routine */
			lt = label_type[path_of[vertex]];
			switch (lt) {
			case SUM_EVEN:
			case SUM_ODD:
				embed_sum(which, lt, 0);
				break;
			case ANY:
				embed_any(which);
				break;
			case ONE_ODD_LABEL:
				embed_one_odd(which, 0);
				break;
			case ALL_TWO:
				embed_all_two(which);
			}
		} else {	/* that is: labels 1 and 2 possible */
			if ((jumpvertex = embed_from_here[vertex][1]) >= 0) {
				/* First fix the edgenet entry that will stay constant */
				for (i = checkedges[vertex], run_net = startedgenn; i != 1;
					i--, run_net = run_net->next);
				buffer = run_net->invers;
				test = 1;
				run2 = buffer->nextnext;
				for (j = checkedges[jumpvertex]; j; j--) {
					if (*run2->end != UNNAMED)
						test = 0;
					run2 = run2->next;
				}
				if (test) {
					edgenet[jumpvertex] = buffer;
					delete = buffer->start;
					*delete = OCCUPIED /* +jumpvertex */ ;
				} else
					return;
			}
			/* Now the other one: */
			fixvertex = embed_from_here[vertex][0]; /* fixvertex must ALWAYS be embedded
								 * after vertex */

			/* This vertex is a connection vertex with degree 3. We generate only the
			 * label that corresponds to the currently selected connection edge! */
			if (path_flag[vertex] == START_OF_PATH) {
				if (ce_of[vertex]->connection_edge == 0)
					gen_label = 1;
				else
					gen_label = 2;
			} else {
				if (ce_of[vertex]->connection_edge == 0)
					gen_label = 2;
				else
					gen_label = 1;
			}
			if (gen_label == 1) {
				buffer = startedgenn->invers;
				test = 1;
				run2 = buffer->nextnext;
				for (j = checkedges[fixvertex]; j; j--) {
					if (*run2->end != UNNAMED)
						test = 0;
					run2 = run2->next;
				}
				if (test) {
					label[vertex] = 1;
					edgenet[fixvertex] = buffer;
					charp = buffer->start;
					*charp = OCCUPIED /* +fixvertex */ ;
					embed_benzenoid_kekule(which + 1);
					*charp = UNNAMED;
				}
			} else {
				buffer = startedgenn->next->invers;
				test = 1;
				run2 = buffer->nextnext;
				for (j = checkedges[fixvertex]; j; j--) {
					if (*run2->end != UNNAMED)
						test = 0;
					run2 = run2->next;
				}
				if (test) {
					label[vertex] = 2;
					edgenet[fixvertex] = buffer;
					charp = buffer->start;
					*charp = OCCUPIED /* +fixvertex */ ;
					embed_benzenoid_kekule(which + 1);
					*charp = UNNAMED;
				}
			}
		}
	}

	if (delete)
		*delete = UNNAMED;
}

void
init_embedding(void)
{
	int             i, j, vertex;
	EDGE           *run, *run2;
	int             first[MAXNV];
	int             number_on_boundary[MAXNV + 1];	/* The inverse of to_place */
	int             to_place_counter = 0;

	/* First the order in that the vertices have to be placed is determined.
	 * They are just taken by running in clockwise direction around the inner dual: */

	vertex = boundary_edge->start;

	for (i = 0; i < maxnv; i++) {
		first[i] = 1;
		run = map[i];
		do {
			run->dummy1 = 0;
			run = run->next;
		} while (run != map[i]);
	}
	number_on_boundary[vertex] = 0;
	number_on_boundary[boundary_edge->end] = 1;
	first[vertex] = first[boundary_edge->end] = 0;

	/* First mark the edges with the outer face on the left and determine
	 * when a vertex occurs first on the boundary: */
	boundary_edge->dummy1 = 1;
	j = 2;
	for (run = boundary_edge->invers->next; run != boundary_edge; run = run->invers->next) {
		run->dummy1 = 1;/* This tells that the outer face is on the left. */
		if (first[run->end]) {
			number_on_boundary[run->end] = j;
			first[run->end] = 0;
			j++;
		}
	}

	first[vertex] = 1;

	/* the number of boundary components of vertex must be 1, since boundary_edge starts
	 * at the last vertex, so the inner dual can not be disconnected, when it is removed */

	embed_from_here[vertex][0] = boundary_edge->end;

	for (run = boundary_edge->invers->next; run != boundary_edge; run = run->invers->next) {
		j = run->start;
		if (first[j] == 0) {	/* This means that it wasn't visited
					 * in this second run before */

			/* A vertex must be added to the to_place list only
			 * if some vertices have to be embedded from there */
			first[j] = 1;
			if (components[j] == 2) {	/* This means that it wasn't visited in
							 * this second run before */
				to_place[to_place_counter] = j;
				to_place_counter++;
				embed_from_here[j][0] = run->end;
				for (run2 = run->next, i = 0; run2->dummy1 != 1; run2 = run2->next, i++);
				jump[j] = i;
				if (degree[j] - jump[j] == 2)
					checkedges[j] = 3;	/* comes from a bridge */
				else
					checkedges[j] = 6 - degree[j] + jump[j];
				if (number_on_boundary[run2->end] > number_on_boundary[j]) {
					embed_from_here[j][1] = run2->end;
					checkedges[j]++;	/* The place where the second vertex must
								 * be placed, must/may also be checked */
				} else
					embed_from_here[j][1] = -1;
			} else if (components[j] == 3) {
				checkedges[j] = 3;
				embed_from_here[j][0] = run->end;
				embed_from_here[j][1] = run->next->end;
				to_place[to_place_counter] = j;
				to_place_counter++;
			} else {	/* 1 component */
				if (degree[j] == 1)
					checkedges[j] = 3;
				else {
					checkedges[j] = 5 - degree[j];
					if (number_on_boundary[run->end] > number_on_boundary[j]) {
						embed_from_here[j][0] = run->end;
						checkedges[j]++;
						to_place[to_place_counter] = j;
						to_place_counter++;
					}
				}
			}
		}
	}

	to_place[to_place_counter] = EMPTY;	/* as a mark that there isn't more to embed */

	/* Now really embed the first vertex: */
	edgenet[boundary_edge->end] = startnet->invers;
	edgenet[vertex] = NULL;
	*(startnet->start) = OCCUPIED /* +vertex */ ;
	*(startnet->end) = OCCUPIED /* +boundary_edge->end */ ;

}

void
split_labellings_benz(void)
{
	int split_path_num, path_num, mask, lts;
	char labelling_possible;

	if (first_labelling) { /* this is called once for every selection of connection edges */
		for (lts = 0; lts < (1 << num_split_paths); lts++)
			have_labelled[lts] = 0;
		first_labelling = 0;
	}

	mask = 0;
	for (split_path_num = 0; split_path_num < num_split_paths; split_path_num++) {
		if (label_type[split_paths[split_path_num]] == ANY) {
			mask |= (1 << split_path_num);
		}
	}
	for (lts = 0; lts < (1 << num_split_paths); lts++) {
		if (have_labelled[lts & mask])
			continue;

		labelling_possible = 1;
		for (split_path_num = 0; split_path_num < num_split_paths; split_path_num++) {
			path_num = split_paths[split_path_num];
			if (lts & (1 << split_path_num)) {
				label_type[path_num] = ALL_TWO;
			} else {
				label_type[path_num] = ONE_ODD_LABEL;
				if (path_length[path_num] == 0) {
					labelling_possible = 0;
					break;
				}
			}
		}
		have_labelled[lts & mask]++;
		if (labelling_possible) {
			embed_benzenoid_kekule(to_place);
		}
	}
}

void
select_var_modes_benz(int vm_comp_num)
{
	int comp, curr_mode, vm_num, i, n;
	EDGE *ce;

	if (vm_comp_num < num_vm_comps) {
		comp = vm_comps[vm_comp_num];
		curr_mode = current_mode[comp];
		n = num_var_modes[comp][curr_mode];
		if (n > 1)
			must_split = 1;
		for (vm_num = 0; vm_num < n; vm_num++) {
			for (i = 0; i < num_ce_var[comp][curr_mode]; i++) {
				ce = ce_var[comp][i][curr_mode];
				MODE(ce, curr_mode) = var_modes[ce->start][curr_mode][vm_num];
			}
			select_var_modes_benz(vm_comp_num + 1);
		}
	} else {
		if (get_label_types()) {
			if (must_split) {
				split_labellings_benz();
			} else {
				embed_benzenoid_kekule(to_place);
			}
		}
	}
}

void
select_modes_benz(int comp)
/* to select all possible combinations of modes */
{
	int mode;

	if (comp < num_comps) {
		for (mode = 0; mode < num_modes[comp]; mode++) {
			current_mode[comp] = mode;
			select_modes_benz(comp + 1);
		}
	} else {
		if (num_vm_comps > 0) {
			first_labelling = 1;
			must_split = 0;
			select_var_modes_benz(0);
		} else {
			if (get_label_types())
				embed_benzenoid_kekule(to_place);
		}
	}
}

void
select_connection_edges_benz(int comp, int ce_num)
{
	EDGE *ce;

	if (ce_num < num_conn_edges[comp]) {
		ce = conn_edge[comp][ce_num];
		ce->connection_edge = 0;
		select_connection_edges_benz(comp, ce_num + 1);
		if (degree[ce->start] == 3 && components[ce->start] != 3) {
			ce->connection_edge = 1;
			select_connection_edges_benz(comp, ce_num + 1);
		}
	} else if (comp < num_comps - 1) {
		select_connection_edges_benz(comp + 1, 0);
	} else {
		select_modes_benz(1);
	}
}

#include "fusgen.h"

EDGE *ce_unsat[MAX_NUM_CE], *ce_sat[MAX_NUM_CE], *ce_test[MAX_NUM_CE];
int num_ce_unsat, num_ce_sat, num_ce_test;

char can_remove_from[MAX_NUM_CE];
char comb[MAX_NUM_CE];

char var_comb[MAX_NUM_CE], all_var_combs[MAX_NUM_CE][MAX_NUM_VAR_MODES];
int num_var_combs, num_comb_conn_edges, num_doubles, modes_with_all_double;

char has_var_modes, more_than_one_vm;

void
remove_bound_vertex(EDGE *ce, char oe)
/* "removes" an odd or even numbered boundary vertex v from the face corresponding
 *  to ce->start, i.e. the edges adjacent to vertex v are marked "FORBIDDEN" */
{
	int f, r, prev_r;
	f = first_bound_vertex[ce->start];

	/* r = vertex to be removed */
	if (f % 2)	/* first_bound_vertex[a] is odd */
		r = f + 1 + (oe % 2);
	else
		r = f + 2 - (oe % 2);

	/* check whether vertex r is already removed */
	prev_r = -1;
	if (bound_match[f + 1] == FORBIDDEN) {
		if (bound_match[f] == FORBIDDEN)
			prev_r = f + 1;
		else if (bound_match[f + 2] == FORBIDDEN)
			prev_r = f + 2;
	}
	if (r == prev_r)
		return;

	if (prev_r != -1) {
		if (!bound_sat[prev_r - 1]) {
			bound_match[prev_r - 1] = 1;
			bound_match[prev_r] = 0;
			bound_sat[prev_r - 1] = 1;
			unsat[(prev_r - 1) % 2]--;
		} else if (!bound_sat[NEXT_VERTEX(prev_r)]) {
			bound_match[prev_r - 1] = 0;
			bound_match[prev_r] = 1;
			bound_sat[NEXT_VERTEX(prev_r)] = 1;
			unsat[NEXT_VERTEX(prev_r) % 2]--;
		} else {
			bound_match[prev_r - 1] = 0;
			bound_match[prev_r] = 0;
			bound_sat[prev_r] = 0;
			unsat[prev_r % 2]++;
		}
	}

	/* remove vertex r */
	if (!bound_sat[r]) {
		bound_sat[r] = 1;	/* we have to set this to 1, because match() must not
					   start to search at this vertex */
		unsat[r % 2]--;
	}

	if (bound_match[r - 1]) {
		bound_sat[r - 1] = 0;
		unsat[(r - 1) % 2]++;
	}
	if (bound_match[r]) {
		bound_sat[NEXT_VERTEX(r)] = 0;	/* if deg(a) = 3 then possibly r + 1 = 0,
						   therefore we use NEXT_VERTEX(r) */
		unsat[NEXT_VERTEX(r) % 2]++;
	}
	bound_match[r - 1] = bound_match[r] = FORBIDDEN;
}

void
remove_bound_edge(EDGE *ce, int which)
/* which = 0 or 1; removes the first or second edge (that occurs in the boundary cycle)
 * in the face corresponding to ce->start */
{
	int e = first_bound_vertex[ce->start] + which;

	bound_sat[e] = bound_sat[e + 1] = !bound_match[e];
	unsat[0] += bound_match[e];
	unsat[1] += bound_match[e];
	bound_match[e] = FORBIDDEN;
}

void
restore_bound_edge(EDGE *ce, int which)
{
	int e = first_bound_vertex[ce->start] + which;

	if (bound_sat[e] == 0 && bound_sat[e + 1] == 0) {
		bound_match[e] = 1;
		bound_sat[e] = bound_sat[e + 1] = 1;
		unsat[0]--; unsat[1]--;
	} else
		bound_match[e] = 0;
}

void
check_var_combs(int ce_num)
{
	EDGE *ce;
	int i, comb_num, all_double;

	if (ce_num < num_ce_test) {
		ce = ce_test[ce_num];
		ce->connection_edge = 0;
		check_var_combs(ce_num + 1);
		if (degree[ce->start] == 3) {
			ce->connection_edge = 1;
			check_var_combs(ce_num + 1);
		}
	} else {
		num_comb_conn_edges++;
		/* now check whether there is a mode in which all connection edges are DOUBLEs */
		for (comb_num = 0; comb_num < num_var_combs; comb_num++) {
			all_double = 1;
			for (i = 0; i < num_ce_test; i++) {
				if (all_var_combs[i][comb_num] ^ ce_test[i]->connection_edge) {
					/* i.e. the connection edge is SINGLE */
					all_double = 0;
					break;
				}
			}
			if (all_double) {
				modes_with_all_double++;
				break;
			}
		}
	}
}

void
find_var_combs(int k)
{
	int i;

	if (k < num_ce_test) {
		var_comb[k] = 0;	/* stands for the first edge in the face
					 * corresponding to the k-th connection vertex */
		find_var_combs(k + 1);
		var_comb[k] = 1;	/* the second edge... */
		find_var_combs(k + 1);
	} else {
		for (i = 0; i < num_ce_test; i++) {
			remove_bound_edge(ce_test[i], var_comb[i]);
		}
		if (match() == 0) {
			for (i = 0; i < num_ce_test; i++) {
				all_var_combs[i][num_var_combs] = var_comb[i];
			}
			num_var_combs++;
		}
		for (i = 0; i < num_ce_test; i++) {
			restore_bound_edge(ce_test[i], var_comb[i]);
		}
		if (unsat[0]) /* if neccesary, call match() again to restore matching */
			match();
	}
}

void
sat_modes(int comp)
{
	int rm_edge, ce_num, num_ce_ok, comb_num, num_vm, curr_mode;
	char m, conn_type[MAX_NUM_CE];
	EDGE *ce;
	int doubles[MAX_NUM_CE], doubles_required, ce_double[MAX_NUM_CE], num_ce_double;

	curr_mode = num_modes[comp];

	/* first set the modes for the saturated connection edges: */
	for (ce_num = 0; ce_num < num_ce_sat; ce_num++) {
		ce = ce_sat[ce_num];
		MODE(ce, curr_mode) = SINGLE_OR_DOUBLE;
		TYPE(ce) = SINGLE_OR_DOUBLE;
	}

	/* now the edges in ce_test[]: */
	if (num_ce_test == 1) {
		ce = ce_test[0];
		/* test whether ce is fixed */
		m = bound_match[first_bound_vertex[ce->start]];
		rm_edge = m ? 0 : 1;
		remove_bound_edge(ce, rm_edge);
		if (match()) {	/* edge is fixed */
			if (m) {
				MODE(ce, curr_mode) = SINGLE;
				TYPE(ce) |= SINGLE;
			} else {
				MODE(ce, curr_mode) = DOUBLE;
				TYPE(ce) |= DOUBLE;
			}
		} else {	/* not fixed */
			MODE(ce, curr_mode) = SINGLE_OR_DOUBLE;
			TYPE(ce) = SINGLE_OR_DOUBLE;
		}
		restore_bound_edge(ce, rm_edge);
	} else if (num_ce_test > 1) {
		num_var_combs = 0;
		find_var_combs(0);
		has_var_modes++;
		num_ce_var[comp][curr_mode] = 0;

		/* In this case, we have variable modes for the edges in ce_test[].
		 * Caution: Even when we discover that all edges ce in ce_test[] have
		 * type SINGLE_OR_DOUBLE, we must not set the MODE(ce, ...) = SINGLE_OR_DOUBLE,
		 * since when there are unsaturated connection edges, then for another
		 * combination of unsaturated vertices, some of the edges in ce_test[]
		 * may become fixed single or double! */

		if (num_var_combs == (1 << num_ce_test)) {
			/* i.e. every connection edge in ce_test[] is SINGLE_OR_DOUBLE */
			for (ce_num = 0; ce_num < num_ce_test; ce_num++) {
				ce = ce_test[ce_num];
				ce_var[comp][num_ce_var[comp][curr_mode]++][curr_mode] = ce;
				var_modes[ce->start][curr_mode][0] = SINGLE_OR_DOUBLE;
				MODE(ce, curr_mode) = VARIABLE;
				TYPE(ce) = SINGLE_OR_DOUBLE;
			}
			num_var_modes[comp][curr_mode] = 1;
		} else {
			num_comb_conn_edges = modes_with_all_double = 0;
			check_var_combs(0);

			if (modes_with_all_double == num_comb_conn_edges) {
				/* for every combination of connection edges there is a mode
				   in which all the edges are DOUBLE => we have only one mode */
				for (ce_num = 0; ce_num < num_ce_test; ce_num++) {
					ce = ce_test[ce_num];
					ce_var[comp][num_ce_var[comp][curr_mode]++][curr_mode] = ce;
					/* set all modes to SINGLE_OR_DOUBLE, even if deg(ce->start) = 4
					 * (then the edge may actually have only mode DOUBLE, but we
					 * don't need to care about that...) */
					var_modes[ce->start][curr_mode][0] = SINGLE_OR_DOUBLE;
					MODE(ce, curr_mode) = VARIABLE;
					TYPE(ce) = SINGLE_OR_DOUBLE;
				}
				num_var_modes[comp][curr_mode] = 1;
			} else if (num_var_combs % 2 == 0) {
				/* sort out the SINGLE connection edges (there may be connection edges
				 * which are SINGLE_OR_DOUBLE) */
				for (ce_num = 0; ce_num < num_ce_test; ce_num++) {
					doubles[ce_num] = 0;
				}
				num_ce_double = 0;
				for (comb_num = 0; comb_num < num_var_combs; comb_num++) {
					for (ce_num = 0; ce_num < num_ce_test; ce_num++) {
						if (all_var_combs[ce_num][comb_num] == 0) {
							/* the first connection edge is DOUBLE */
							doubles[ce_num]++;
						}
					}
				}
				doubles_required = num_var_combs >> 1;
				for (ce_num = 0; ce_num < num_ce_test; ce_num++) {
					if (doubles[ce_num] == doubles_required) {
						ce_double[num_ce_double++] = ce_num;
					} else {
						ce_var[comp][num_ce_var[comp][curr_mode]++][curr_mode] =
									ce_test[ce_num];
					}
				}

				/* now collect the modes for the remaining single edges */
				num_vm = 0;
				for (comb_num = 0; comb_num < num_var_combs; comb_num++) {
					/* check whether in this combination the values for the edges in
					   ce_double[] are zero -- if so, we select this combination
					   to construct a mode for the single edges */
					num_ce_ok = 0;
					for (ce_num = 0; ce_num < num_ce_double; ce_num++) {
						if (all_var_combs[ce_double[ce_num]][comb_num] == 0) {
							num_ce_ok++;
						}
					}
					if (num_ce_ok != num_ce_double) /* skip this combination */
						continue;

					/* now, out of this combination, construct a mode
					 * for the single edges */
					for (ce_num = 0; ce_num < num_ce_test; ce_num++) {
						if (doubles[ce_num] == doubles_required)
							continue;
						ce = ce_test[ce_num];
						if (all_var_combs[ce_num][comb_num] == 0)
							var_modes[ce->start][curr_mode][num_vm] = DOUBLE;
						else
							var_modes[ce->start][curr_mode][num_vm] = SINGLE;
					}
					num_vm++;
				}
				for (ce_num = 0; ce_num < num_ce_test; ce_num++) {
					ce = ce_test[ce_num];
					if (doubles[ce_num] == doubles_required) {
						MODE(ce, curr_mode) = SINGLE_OR_DOUBLE;
						TYPE(ce) = SINGLE_OR_DOUBLE;
					} else {
						MODE(ce, curr_mode) = VARIABLE;
						TYPE(ce) = SINGLE;
					}
				}
				num_var_modes[comp][curr_mode] = num_vm;
				if (num_vm > 1)
					more_than_one_vm = 1;
			} else {	/* all the edges in ce_test[] are possibly SINGLE */
				for (ce_num = 0; ce_num < num_ce_test; ce_num++)
					conn_type[ce_num] = 0;
				num_vm = 0;
				for (comb_num = 0; comb_num < num_var_combs; comb_num++) {
					for (ce_num = 0; ce_num < num_ce_test; ce_num++) {
						ce = ce_test[ce_num];
						if (all_var_combs[ce_num][comb_num] == 0) {
							var_modes[ce->start][curr_mode][num_vm] = DOUBLE;
							conn_type[ce_num] |= DOUBLE;
						} else {
							var_modes[ce->start][curr_mode][num_vm] = SINGLE;
							conn_type[ce_num] |= SINGLE;
						}
					}
					num_vm++;
				}
				for (ce_num = 0; ce_num < num_ce_test; ce_num++) {
					ce = ce_test[ce_num];
					ce_var[comp][num_ce_var[comp][curr_mode]++][curr_mode] = ce;

					MODE(ce, curr_mode) = VARIABLE;
					TYPE(ce) = conn_type[ce_num];
				}
				num_var_modes[comp][curr_mode] = num_vm;
				if (num_vm > 1)
					more_than_one_vm = 1;
			}
		}
	}
	num_modes[comp]++;
}

void
unsat_modes(int comp, int k, char u0, char u1)
{
	int i;
	char m;

	if (k < num_ce_unsat) {
		if (u0 > 0 && can_remove_from[k] & EVEN) {
			comb[k] = EVEN;
			unsat_modes(comp, k + 1, u0 - 1, u1);
		}
		if (u1 > 0 && can_remove_from[k] & ODD) {
			comb[k] = ODD;
			unsat_modes(comp, k + 1, u0, u1 - 1);
		}
	} else {
		for (i = 0; i < num_ce_unsat; i++) {
			remove_bound_vertex(ce_unsat[i], comb[i]);
		}
		if (match() == 0) {
			/* this combination is possible, adjust modes */
			for (i = 0; i < num_ce_unsat; i++) {
				/* determine the mode of ce_unsat[i] */
				if (first_bound_vertex[ce_unsat[i]->start] % 2)
					m = (comb[i] == ODD) ? LEFT : RIGHT;
				else
					m = (comb[i] == ODD) ? RIGHT : LEFT;

				MODE(ce_unsat[i], num_modes[comp]) = m;
				TYPE(ce_unsat[i]) |= m;
			}
			sat_modes(comp);
		}
	}
}

int
modes(int comp)
/* computes the modes of a component, returns number of modes */
{
	int i, j, k, num_ce, u0, u1, num_unsat, opp_comp;
	char has_unknown_ce, u_unsat, m;
	EDGE *ce, *u;
        u = NULL;

	num_ce = num_conn_edges[comp];
	num_unsat = init_match(comp);

	num_modes[comp] = 0;
	has_var_modes = 0;
	more_than_one_vm = 0;
	has_unknown_ce = u_unsat = 0;

	/* determine the saturated and unsaturated connection edges of this component */
	num_ce_unsat = num_ce_sat = num_ce_test = 0;
	for (i = 0; i < num_ce; i++) {
		/* except for the last component, the type of the last edge in conn_edge[comp][]
		   is undetermined */
		ce = conn_edge[comp][i];
		switch (TYPE(ce->opposite)) {
		case 0:
			u = ce;
			has_unknown_ce++;
			break;
		case LEFT:
		case RIGHT:
		case LEFT_OR_RIGHT:
			ce_unsat[num_ce_unsat++] = ce;
			break;
		case DOUBLE:
			if (degree[ce->opposite->start] == 3 && components[ce->opposite->start] == 2) {
				ce_test[num_ce_test++] = ce;
			} else {
				ce_sat[num_ce_sat++] = ce;
			}
			break;
		case SINGLE_OR_DOUBLE:
			opp_comp = ce->opposite->component;
			if (num_modes[opp_comp] == 1 && !is_vm_comp[opp_comp]) {
				ce_sat[num_ce_sat++] = ce;

			} else {
				ce_test[num_ce_test++] = ce;
			}
			break;
		case SINGLE:
			ce_test[num_ce_test++] = ce;
			break;
		default:
			fprintf(stderr, "modes(%d): cannot handle TYPE(%d, %d) = 0x%02x\n",
				comp, ce->opposite->start, ce->opposite->end, TYPE(ce->opposite));
			exit(1);
			return 0;	/* avoids warning */
		}
	}

	if (num_unsat > num_ce_unsat + 1)
		return 0;

	if (has_unknown_ce) {
		if ((num_ce_unsat - num_unsat) % 2) { /* determine type of u */
			ce_unsat[num_ce_unsat++] = u;
			u_unsat++;
		} else {
			ce_test[num_ce_test++] = u;
		}
	} else {
		if ((num_ce_unsat - num_unsat) % 2) {
			return 0;
		}
	}

	/* check all combinations for the unsaturated connection vertices */
	if(num_ce_unsat) {
		if (has_unknown_ce && u_unsat) {
			k = num_ce_unsat - 1;
			can_remove_from[k] = EVEN | ODD;
		} else {
			k = num_ce_unsat;
		}

		for (i = 0; i < k; i++) {
			ce = ce_unsat[i];

			if (LENGTH(ce) == 0 && cv_zero_label[ce->start] &&
						cv_zero_label[ce->opposite->start]) {
				if (TYPE(ce->opposite) == LEFT_OR_RIGHT) {
					can_remove_from[i] = EVEN | ODD;
				} else {
					if (first_bound_vertex[ce->start] % 2) {
						if (TYPE(ce->opposite) == LEFT) {
							can_remove_from[i] = ODD;
						} else {
							can_remove_from[i] = EVEN;
						}
					} else {
						if (TYPE(ce->opposite) == LEFT) {
							can_remove_from[i] = EVEN;
						} else {
							can_remove_from[i] = ODD;
						}
					}
				}
			} else {
				can_remove_from[i] = EVEN | ODD;
			}
		}

		k = (num_ce_unsat - unsat[0] - unsat[1]) / 2;
		u0 = unsat[0] + k;
		u1 = unsat[1] + k;
		unsat_modes(comp, 0, u0, u1);
	} else {
		sat_modes(comp);
	}

	if (has_var_modes) {
		if (!more_than_one_vm) {
			/* if all modes have just one single mode, we copy
			 * var_modes[ce->start][i][0] to MODE(ce, i) in order
			 * to speed up labelling */
			for (i = 0; i < num_modes[comp]; i++) {
				for (j = 0; j < num_ce_var[comp][i]; j++) {
					ce = ce_var[comp][j][i];
					m = var_modes[ce->start][i][0];
					MODE(ce, i) = m;
					TYPE(ce) |= m;
				}
			}
			is_vm_comp[comp] = 0;
		} else {
			vm_comps[num_vm_comps++] = comp;
			is_vm_comp[comp] = 1;
		}
	} else {
		is_vm_comp[comp] = 0;
	}
	return num_modes[comp];
}

int
T_modes(int comp)
/* computes the modes of the "component" that consists of a single vertex */
{
	EDGE *c0, *c1, *c2, *oc1, *oc2;
	char m1, m2;

	c0 = comp_boundary_edge[comp];
	c1 = c0->next;
	c2 = c0->prev;

	oc1 = c1->opposite;
	oc2 = c2->opposite;

	m1 = TYPE(oc1);
	m2 = TYPE(oc2);

	if (LENGTH(c1) == 0 && cv_zero_label[oc1->start]) {
		if (m1 == SINGLE_OR_DOUBLE || (m1 == SINGLE && is_vm_comp[oc1->component]))
			m1 = DOUBLE;
		/* else leave m1 as it is */
	} else {
		if (m1 & LEFT_OR_RIGHT)
			m1 = LEFT_OR_RIGHT;
		else if (m1 & SINGLE_OR_DOUBLE)
			m1 = DOUBLE;
		else {
			fprintf(stderr, "T_modes(%d): problem m1 = 0x%02x", comp, m1);
			exit(1);
		}
	}

	if (LENGTH(c2) == 0 && cv_zero_label[oc2->start]) {
		if (m2 == SINGLE_OR_DOUBLE || (m2 == SINGLE && is_vm_comp[oc2->component]))
			m2 = DOUBLE;
		/* else leave m2 as it is */
	} else {
		if (m2 & LEFT_OR_RIGHT)
			m2 = LEFT_OR_RIGHT;
		else if (m2 & SINGLE_OR_DOUBLE)
			m2 = DOUBLE;
		else {
			fprintf(stderr, "T_modes(%d): problem m2 = 0x%02x", comp, m2);
			exit(1);
		}
	}

	if (m1 == LEFT_OR_RIGHT) {
		if (m2 == LEFT_OR_RIGHT) {
			MODE(c1, 0) = LEFT; MODE(c2, 0) = RIGHT; MODE(c0, 0) = SINGLE;
			MODE(c1, 1) = RIGHT; MODE(c2, 1) = LEFT; MODE(c0, 1) = DOUBLE;
			TYPE(c0) = SINGLE_OR_DOUBLE;
			num_modes[comp] = 2;
		} else if (m2 == LEFT) {
			MODE(c1, 0) = RIGHT; MODE(c2, 0) = LEFT; MODE(c0, 0) = DOUBLE;
			TYPE(c0) = DOUBLE;
			num_modes[comp] = 1;
		} else if (m2 == RIGHT) {
			MODE(c1, 0) = LEFT; MODE(c2, 0) = RIGHT; MODE(c0, 0) = SINGLE;
			TYPE(c0) = SINGLE;
			num_modes[comp] = 1;
		} else if (m2 == DOUBLE) {
			MODE(c1, 0) = LEFT; MODE(c2, 0) = DOUBLE; MODE(c0, 0) = RIGHT;
			MODE(c1, 1) = RIGHT; MODE(c2, 1) = SINGLE; MODE(c0, 1) = LEFT;
			TYPE(c0) = LEFT_OR_RIGHT;
			num_modes[comp] = 2;
		} else if (m2 == SINGLE) {
			MODE(c1, 0) = LEFT; MODE(c2, 0) = DOUBLE; MODE(c0, 0) = RIGHT;
			TYPE(c0) = RIGHT;
			num_modes[comp] = 1;
		} else {
			fprintf(stderr, "error: can't happen: m1 = 0x%02x, m2 = 0x%02x\n", m1, m2);
			exit(1);
		}
	} else if (m1 == LEFT) {
		if (m2 == LEFT_OR_RIGHT) {
			MODE(c1, 0) = LEFT; MODE(c2, 0) = RIGHT; MODE(c0, 0) = SINGLE;
			TYPE(c0) = SINGLE;
			num_modes[comp] = 1;
		} else if (m2 == LEFT) {
			return 0;
		} else if (m2 == RIGHT) {
			MODE(c1, 0) = LEFT; MODE(c2, 0) = RIGHT; MODE(c0, 0) = SINGLE;
			TYPE(c0) = SINGLE;
			num_modes[comp] = 1;
		} else if (m2 == DOUBLE) {
			MODE(c1, 0) = LEFT; MODE(c2, 0) = DOUBLE; MODE(c0, 0) = RIGHT;
			TYPE(c0) = RIGHT;
			num_modes[comp] = 1;
		} else if (m2 == SINGLE) {
			MODE(c1, 0) = LEFT; MODE(c2, 0) = DOUBLE; MODE(c0, 0) = RIGHT;
			TYPE(c0) = RIGHT;
			num_modes[comp] = 1;
		} else {
			fprintf(stderr, "error: can't happen: m1 = 0x%02x, m2 = 0x%02x\n", m1, m2);
			exit(1);
		}
	} else if (m1 == RIGHT) {
		if (m2 == LEFT_OR_RIGHT) {
			MODE(c1, 0) = RIGHT; MODE(c2, 0) = LEFT; MODE(c0, 0) = DOUBLE;
			TYPE(c0) = DOUBLE;
			num_modes[comp] = 1;
		} else if (m2 == LEFT) {
			MODE(c1, 0) = RIGHT; MODE(c2, 0) = LEFT; MODE(c0, 0) = DOUBLE;
			TYPE(c0) = DOUBLE;
			num_modes[comp] = 1;
		} else if (m2 == RIGHT) {
			return 0;
		} else if (m2 == DOUBLE) {
			MODE(c1, 0) = RIGHT; MODE(c2, 0) = SINGLE; MODE(c0, 0) = LEFT;
			TYPE(c0) = LEFT;
			num_modes[comp] = 1;
		} else if (m2 == SINGLE) {
			return 0;
		} else {
			fprintf(stderr, "error: can't happen: m1 = 0x%02x, m2 = 0x%02x\n", m1, m2);
			exit(1);
		}
	} else if (m1 == DOUBLE) {
		if (m2 == LEFT_OR_RIGHT) {
			MODE(c1, 0) = SINGLE; MODE(c2, 0) = LEFT; MODE(c0, 0) = RIGHT;
			MODE(c1, 1) = DOUBLE; MODE(c2, 1) = RIGHT; MODE(c0, 1) = LEFT;
			TYPE(c0) = LEFT_OR_RIGHT;
			num_modes[comp] = 2;
		} else if (m2 == LEFT) {
			MODE(c1, 0) = SINGLE; MODE(c2, 0) = LEFT; MODE(c0, 0) = RIGHT;
			TYPE(c0) = RIGHT;
			num_modes[comp] = 1;
		} else if (m2 == RIGHT) {
			MODE(c1, 0) = DOUBLE; MODE(c2, 0) = RIGHT; MODE(c0, 0) = LEFT;
			TYPE(c0) = LEFT;
			num_modes[comp] = 1;
		} else if (m2 == DOUBLE) {
			MODE(c1, 0) = DOUBLE; MODE(c2, 0) = DOUBLE; MODE(c0, 0) = DOUBLE;
			TYPE(c0) = DOUBLE;
			num_modes[comp] = 1;
		} else if (m2 == SINGLE) {
			MODE(c1, 0) = DOUBLE; MODE(c2, 0) = DOUBLE; MODE(c0, 0) = DOUBLE;
			TYPE(c0) = DOUBLE;
			num_modes[comp] = 1;
		} else {
			fprintf(stderr, "error: can't happen: m1 = 0x%02x, m2 = 0x%02x\n", m1, m2);
			exit(1);
		}
	} else if (m1 == SINGLE) {
		if (m2 == LEFT_OR_RIGHT) {
			MODE(c1, 0) = DOUBLE; MODE(c2, 0) = RIGHT; MODE(c0, 0) = LEFT;
			TYPE(c0) = LEFT;
			num_modes[comp] = 1;
		} else if (m2 == LEFT) {
			return 0;
		} else if (m2 == RIGHT) {
			MODE(c1, 0) = DOUBLE; MODE(c2, 0) = RIGHT; MODE(c0, 0) = LEFT;
			TYPE(c0) = LEFT;
			num_modes[comp] = 1;
		} else if (m2 == DOUBLE) {
			MODE(c1, 0) = DOUBLE; MODE(c2, 0) = DOUBLE; MODE(c0, 0) = DOUBLE;
			TYPE(c0) = DOUBLE;
			num_modes[comp] = 1;
		} else if (m2 == SINGLE) {
			MODE(c1, 0) = DOUBLE; MODE(c2, 0) = DOUBLE; MODE(c0, 0) = DOUBLE;
			TYPE(c0) = DOUBLE;
			num_modes[comp] = 1;
		} else {
			fprintf(stderr, "error: can't happen: m1 = 0x%02x, m2 = 0x%02x\n", m1, m2);
			exit(1);
		}
	}
	is_vm_comp[comp] = 0;

	return num_modes[comp];
}

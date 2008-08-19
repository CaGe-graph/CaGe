#include "fusgen.h"

void
label_paths(int path_num)
{
	int num_vertices;
	EDGE *s, *t;

	if (path_num < num_paths) {
		num_vertices = path_length[path_num];
		s = path_start_edge[path_num];
		t = s->opposite;

		/* label the first and the last vertex on the path */
		if (degree[s->start] == 3 && components[s->start] == 2)
			label[s->start] = 2 - s->connection_edge;

		if (degree[t->start] == 3 && components[t->start] == 2)
			label[t->start] = t->connection_edge + 1;

		if (num_vertices > 0) {		/* there is a vertex to label on the path */
			switch (label_type[path_num]) {
			case SUM_EVEN:
			case SUM_ODD:
				label_sum(path_num, 0, num_vertices, label_type[path_num], 0);
				break;
			case ANY:
				label_any(path_num, 0, num_vertices);
				break;
			case ONE_ODD_LABEL:
				label_one_odd(path_num, 0, num_vertices, 0);
				break;
			case ALL_TWO:
				label_all_two(path_num);
				break;
			}
		} else {
			label_paths(path_num + 1);
		}
	} else {
		write_up_kekule();
	}
}

void
label_sum(int path_num, int pos, int len, char how, int sum)
{
	int i, p;

	if (pos < len) {
		p = variable_positions[path_index[path_num] + pos];
		for (i = 3; i != 0; i--) {
			label[p] = i;
			label_sum(path_num, pos + 1, len, how, sum + i);
		}
	} else if (sum % 2 == how) {
		if (path_num < num_paths) {
			label_paths(path_num + 1);
		} else {
			write_up_kekule();
		}
	}
}

void
label_one_odd(int path_num, int pos, int len, char has_odd)
{
	int i, p;

	if (pos < len) {
		p = variable_positions[path_index[path_num] + pos];
		for (i = 3; i != 0; i--) {
			label[p] = i;
			label_one_odd(path_num, pos + 1, len, i % 2 || has_odd);
		}
	} else if (has_odd) {
		if (path_num < num_paths) {
			label_paths(path_num + 1);
		} else {
			write_up_kekule();
		}
	}
}

void
label_any(int path_num, int pos, int len)
{
	int i, p;

	if (pos < len) {
		p = variable_positions[path_index[path_num] + pos];
		for (i = 3; i != 0; i--) {
			label[p] = i;
			label_any(path_num, pos + 1, len);
		}
	} else {
		if (path_num < num_paths) {
			label_paths(path_num + 1);
		} else {
			write_up_kekule();
		}
	}
}

void
label_all_two(int path_num)
{
	int pos;

	for (pos = 0; pos < path_length[path_num]; pos++) {
		label[variable_positions[path_index[path_num] + pos]] = 2;
	}

	if (path_num < num_paths) {
		label_paths(path_num + 1);
	} else {
		write_up_kekule();
	}
}

char
get_mode(EDGE *ce)
{
	char m;

	if (degree[ce->start] == 1) return DOUBLE;

	m = MODE(ce, current_mode[ce->component]);

	if (ce->connection_edge == 0)
		return m;

	switch (m) {
	case LEFT:
		return RIGHT;
	case RIGHT:
		return LEFT;
	case SINGLE:
	case SINGLE_OR_DOUBLE:
		return DOUBLE;
	case DOUBLE:
		return SINGLE;
	default:
		fprintf(stderr, "error: unknown mode 0x%02x\n", m);
		fprintf(stderr, "ce = (%d, %d), current_mode[%d] = %d\n", ce->start, ce->end,
			ce->component, current_mode[ce->component]);
		exit(1);
		return 0; /* not reached */
	}
}

void
count_labellings(void)
{
	int path_num, len, l;
	LONGTYPE k = 1, p;

	for (path_num = 0; path_num < num_paths; path_num++) {
		len = path_length[path_num];
		if (len > 0) {
			switch (label_type[path_num]) {
			case SUM_EVEN:
				p = 3; l = len; while (--l) p *= 3;
				k *= ((p + 1) >> 1) - (len % 2);
				break;
			case SUM_ODD:
				p = 3; l = len; while (--l) p *= 3;
				k *= ((p - 1) >> 1) + (len % 2);
				break;
			case ANY:
				p = 3; l = len; while (--l) p *= 3;
				k *= p;
				break;
			case ONE_ODD_LABEL:
				p = 3; l = len; while (--l) p *= 3;
				k *= p - 1;
				break;
			case ALL_TWO:
				/* here we have k *= 1 */
				break;
			default:
				fprintf(stderr, "error: unknown label type %d (path %d)\n",
					label_type[path_num], path_num);
				exit(1);
			}
		}
	}
	counter += k;
	if (detailed) {
		groupformula[Cs][C][H] += k;
	}

}

int
get_label_types()
{
	char m0, m1;
	int path_num;

	//for (path_num = 0; path_num < num_paths; path_num++) {
	//for (path_num = num_paths - 1; path_num >= 0; path_num--) {
	path_num = num_paths;
	while (path_num--) {
		m0 = get_mode(path_start_edge[path_num]);
		if (m0 & DOUBLE) {
			label_type[path_num] = ANY;
			continue;
		}
		m1 = get_mode(path_start_edge[path_num]->opposite);
		if (m1 & DOUBLE) {
			label_type[path_num] = ANY;
			continue;
		}
		switch (m0 | m1) {
		case LEFT:
		case RIGHT:
			label_type[path_num] = SUM_EVEN;
			break;
		case LEFT_OR_RIGHT:
			if (path_length[path_num] == 0) {
				return 0;
			}
			label_type[path_num] = SUM_ODD;
			break;
		case SINGLE:
			if (path_length[path_num] == 0) {
				return 0;
			}
			label_type[path_num] = ONE_ODD_LABEL;
			break;
		default:
			fprintf(stderr, "error: no label type for modes 0x%02x, 0x%02x\n", m0, m1);
			fprintf(stderr, "path_num = %d\n", path_num);
			exit(1);
		}

	}
	return 1;
}

void
split_labellings(void)
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
			if (just_count && !symmetric_case)
				count_labellings();
			else
				label_paths(0);
		}
	}
}

void
select_var_modes(int vm_comp_num)
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
			select_var_modes(vm_comp_num + 1);
		}
	} else {
		if (get_label_types()) {
			if (must_split) {
				split_labellings();
			} else {
				if (just_count && !symmetric_case)
					count_labellings();
				else
					label_paths(0);
			}
		}
	}
}

void
select_modes(int comp)
/* to select all possible combinations of modes */
{
	int mode;

	if (comp < num_comps) {
		for (mode = 0; mode < num_modes[comp]; mode++) {
			current_mode[comp] = mode;
			select_modes(comp + 1);
		}
	} else {
		if (num_vm_comps > 0) {
			first_labelling = 1;
			must_split = 0;
			select_var_modes(0);
		} else {
			if (get_label_types()) {
				if (just_count && !symmetric_case) {
					count_labellings();
				} else {
					label_paths(0);
				}
			}
		}
	}
}


void
select_connection_edges(int comp, int ce_num)
{
	EDGE *ce;

	if (ce_num < num_conn_edges[comp]) {
		ce = conn_edge[comp][ce_num];
		ce->connection_edge = 0;
		select_connection_edges(comp, ce_num + 1);
		if (degree[ce->start] == 3 && components[ce->start] != 3) {
			ce->connection_edge = 1;
			select_connection_edges(comp, ce_num + 1);
		}
	} else if (comp < num_comps - 1) {
		select_connection_edges(comp + 1, 0);
	} else {
		select_modes(1);
	}
}

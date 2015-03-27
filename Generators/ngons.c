#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

typedef struct e {
  int start; /* start vertex of edge */
  int end; /* end vertex of edge */

  struct e *prev; /* previous edge in clockwise direction */
  struct e *next; /* next edge in clockwise direction */
  struct e *inverse; /* inverse edge */

  int label; /* label of the angle starting with this edge */
  int leftface;
  int rightface;

  int matching;
} EDGE;

typedef struct {
  int maxdeg; /* maximum degree of verticex */
  int maxsize; /* maximum number of vertices */
  int maxedges; /* maximum number of directed edges */

  int size; /* number of vertices */
  int edges; /* number of directed edges */
  int boundary_length; /* boundary length */
  int faces; /* number of faces */
  int *counter; /* access only via COUNT, INCR COUNT, DECR_COUNT */

  int *deg; /* deg[v] = degree of vertex v */
  int *outer; /* outer[v] = number of occurences in outer face of vertex v */
  int *label; /* label[v] = label of vertex v */
  EDGE **firstedge; /* *firstedge[v] = edge with start v */

  int matching;
} GRAPH;

#define COUNT(G, deg, outer) G->counter[(outer) * (G->maxdeg + 1) + (deg)]
#define INCR_COUNT(G, deg, outer) G->counter[(outer) * (G->maxdeg + 1) + (deg)]++
#define DECR_COUNT(G, deg, outer) G->counter[(outer) * (G->maxdeg + 1) + (deg)]--

#define NUMBER(G, numberings, numb, i) numberings[numb * G->boundary_length + i]

static int DUALS, KEKULE, OUTPUT, BIPARTITE;

static unsigned long int MODULO, INDEX, dual_index;

static unsigned long int global_count, dual_count, labeled_count;

static unsigned long int dual_trivial, labeled_trivial;

static FILE *OUTFILE;

static int *vertexcode, *anglecode, *labeled, *restlabel, *filtered_numbs;

static int *saturated, *face_to_extra, *distance, *type;
static EDGE **firstedge, **path;

static void compute_code(GRAPH *G, unsigned char *code) {
  register EDGE *run;
  register int vertex;
  EDGE *temp, *startedge[G->size];
  int number[G->size], i;
  int last_number, actual_number;

  for (i = 0; i < G->size; i++) number[i] = 0;

  *code = G->size; code++;

  if (G->size < 2) {
    *code = 0; code++;
    return;
  }

  temp = G->firstedge[0];
  number[temp->start] = 1;
  number[temp->end] = 2;
  last_number = 2;
  startedge[1] = temp->inverse;

  actual_number = 1;

  while (actual_number <= G->size) {
    *code = number[temp->end]; code++;
    for (run = temp->next; run != temp; run = run->next) {
      vertex = run->end;
      if (!number[vertex]) {
        startedge[last_number++] = run->inverse;
        number[vertex] = last_number;
      }
      *code = number[vertex]; code++;
    }
    *code = 0; code++;
    temp = startedge[actual_number++];
  }
}

static void compute_dual_code(GRAPH* G, unsigned char *code) {
  unsigned char *edge_code;
  int dual_size = G->maxedges - (G->edges / 2) - G->maxsize + 1;
  EDGE *temp, *startedge[G->faces];
  int number[G->faces], prev[G->faces], next[G->faces], i;
  int last_number, actual_number, last_edge_number;

  for (i = 0; i < G->faces; i++) {
    number[i] = 0;
    prev[i] = 0;
    next[i] = 0;
  }

  edge_code = code + 4 * G->faces + 1;
  *code = dual_size; code++;

  temp = G->firstedge[0];
  number[temp->leftface] = 1;
  number[temp->rightface] = 2;
  last_number = 2;
  startedge[1] = temp->inverse;

  actual_number = 1;
  last_edge_number = G->faces;

  while (actual_number <= G->faces) {
    if (!number[temp->rightface]) {
      startedge[last_number++] = temp->inverse;
      number[temp->rightface] = last_number;
    }
    *code = number[temp->rightface]; code++;

    if (!temp->label) {
      /* actual number is an inner vertex */
      if (!number[temp->prev->leftface]) {
        startedge[last_number++] = temp->prev;
        number[temp->prev->leftface] = last_number;
      }
      *code = number[temp->prev->leftface]; code++;
      if (!number[temp->inverse->next->rightface]) {
        startedge[last_number++] = temp->inverse->next->inverse;
        number[temp->inverse->next->rightface] = last_number;
      }
      *code = number[temp->inverse->next->rightface]; code++;
    } else {
      /* actual_number is an outer vertex of degree 3 */
      if (!prev[temp->leftface]) {
        if (!number[temp->prev->rightface]) {
          startedge[last_number++] = temp->prev->inverse;
          number[temp->prev->rightface] = last_number;
        }

        if (temp->prev->inverse->label > 1) {
          *edge_code = actual_number; edge_code++;
          prev[temp->leftface] = ++last_edge_number;
          for (i = 0; i < temp->prev->inverse->label - 2; i++) {
            *edge_code = last_edge_number + 1; edge_code++;
            *edge_code = 0; edge_code++;
            *edge_code = last_edge_number; edge_code++;
            last_edge_number++;
          }
          *edge_code = number[temp->prev->rightface]; edge_code++;
          *edge_code = 0; edge_code++;
          next[temp->prev->rightface] = last_edge_number;
        } else {
          prev[temp->leftface] = number[temp->prev->rightface];
          next[temp->prev->rightface] = actual_number;
        }
      }
      *code = prev[temp->leftface]; code++;

      if (!next[temp->leftface]) {
        if (!number[temp->inverse->next->leftface]) {
          startedge[last_number++] = temp->inverse->next;
          number[temp->inverse->next->leftface] = last_number;
        }

        if (temp->label > 1) {
          *edge_code = actual_number; edge_code++;
          next[temp->leftface] = ++last_edge_number;
          for (i = 0; i < temp->label - 2; i++) {
            *edge_code = last_edge_number + 1; edge_code++;
            *edge_code = 0; edge_code++;
            *edge_code = last_edge_number; edge_code++;
            last_edge_number++;
          }
          *edge_code = number[temp->inverse->next->leftface]; edge_code++;
          *edge_code = 0; edge_code++;
          prev[temp->inverse->next->leftface] = last_edge_number;
        } else {
          next[temp->leftface] = number[temp->inverse->next->leftface];
          prev[temp->inverse->next->leftface] = actual_number;
        }
      }
      *code = next[temp->leftface]; code++;
    }
    *code = 0; code++;
    temp = startedge[actual_number++];
  }
}

static void write_header(FILE *fp) {
  unsigned char header[15] = ">>planar_code<<";
  fwrite(header, sizeof(unsigned char), 15, fp);
}

static void write_planar_code(GRAPH *G) {
  int size = G->size + G->edges + 1;
  unsigned char code[size];

  compute_code(G, code);
  fwrite(code, sizeof(unsigned char), size, OUTFILE);
}

static void write_dual_planar_code(GRAPH *G) {
  int size = 3 * G->maxedges - 3 * (G->edges / 2) - G->maxsize + 2;
  unsigned char code[size];

  compute_dual_code(G, code);
  fwrite(code, sizeof(unsigned char), size, OUTFILE);
}


static void kekule_greedy(GRAPH *G, int *nsat) {
  register EDGE *edge;
  int i, vertex, left, right, extra = G->faces;

  for (vertex = 0; vertex < G->size; vertex++) {
    /* Check all edges */
    edge = G->firstedge[vertex];
    for (i = 0; i < G->deg[vertex]; i++) {
      if (G->matching != edge->matching / 2) {
        /* edge is not checked yet */
        left = edge->leftface;
        right = edge->rightface;

        /* save edge as firstedge */
        firstedge[left] = edge;
        firstedge[right] = edge->inverse;

        /* add edge to matching if possible */
        if (!saturated[left] && !saturated[right]) {
          edge->matching = edge->inverse->matching = 2 * G->matching + 1;
          saturated[left] = saturated[right] = 1;
          *nsat += 2;
        } else {
          edge->matching = edge->inverse->matching = 2 * G->matching;
        }
      }

      if (edge->label) {
        left = edge->leftface;
        right = edge->inverse->next->leftface;

        firstedge[extra] = edge;
        face_to_extra[left] = extra;

        /* add boundary to matching if possible */
        if (edge->label % 2 == 0) {
          if (!saturated[left]) {
            saturated[left] = 1;
            saturated[extra] = 1;
            *nsat += 2;
          } else if (!saturated[right]) {
            saturated[right] = 1;
            saturated[extra] = 2;
            *nsat += 2;
          }
        } else {
          if (!saturated[left] && !saturated[right]) {
            saturated[left] = saturated[right] = 1;
            saturated[extra] = 1;
            *nsat += 3;
          } else {
            saturated[extra] = 2;
            *nsat += 1;
          }
        }

        extra++;
      }

      edge = edge->next;
    }
  }
}

#define TYPE_EDGE 0
#define TYPE_OUTER_LEFT 1
#define TYPE_OUTER_RIGHT 2
#define TYPE_EXTRA_LEFT 3
#define TYPE_EXTRA_RIGHT 4
#define TYPE_BORDER_LEFT 5
#define TYPE_BORDER_RIGHT 6

static int kekule_augmenting(GRAPH *G, int end) {
  EDGE *edge;

  if (!saturated[end]) {
    saturated[end] = 1;
    while (distance[end] > 0) {
      edge = path[end];
      switch(type[end]) {
        case TYPE_EDGE:
          edge->matching = edge->inverse->matching = G->matching ? 5 - edge->matching : 1 - edge->matching;
          end = edge->leftface;
          break;
        case TYPE_OUTER_LEFT:
          saturated[face_to_extra[end]] = 1;
          return 1;
        case TYPE_OUTER_RIGHT:
          saturated[face_to_extra[edge->leftface]] = 2;
          return 1;
        case TYPE_EXTRA_LEFT:
          saturated[end] = 2;
          end = edge->inverse->next->leftface;
          break;
        case TYPE_EXTRA_RIGHT:
          saturated[end] = 1;
          end = edge->leftface;
          break;
        case TYPE_BORDER_LEFT:
          end = face_to_extra[end];
          saturated[end] = 3 - saturated[end];
          end = edge->inverse->next->leftface;
          break;
        case TYPE_BORDER_RIGHT:
          end = face_to_extra[edge->leftface];
          saturated[end] = 3 - saturated[end];
          end = edge->leftface;
          break;
      }
    }
    saturated[end] = 1;
    return 1;
  } else {
    return 0;
  }
}

static int kekule_add_edge(GRAPH *G, int list[], int *listsize, EDGE *edge) {
  int previous = edge->leftface;
  int end = edge->rightface;

  if (distance[end] == -1 && edge->matching - 2 * G->matching == distance[previous] % 2) {
    distance[end] = distance[previous] + 1;
    path[end] = edge;
    type[end] = TYPE_EDGE;
    list[(*listsize)++] = end;

    return 1;
  }

  return 0;
}

static int kekule_add_outer_left(GRAPH *G, int list[], int *listsize, int previous, EDGE *edge) {
  int end = edge->leftface;

  if (distance[end] == -1 && saturated[previous] % 2 == distance[previous] % 2) {
    distance[end] = distance[previous] + 1;
    path[end] = edge;
    type[end] = TYPE_OUTER_LEFT;
    list[(*listsize)++] = end;

    return 1;
  }

  return 0;
}

static int kekule_add_outer_right(GRAPH *G, int list[], int *listsize, int previous, EDGE *edge) {
  int end = edge->inverse->next->leftface;

  if (distance[end] == -1 && saturated[previous] / 2 == distance[previous] % 2) {
    distance[end] = distance[previous] + 1;
    path[end] = edge;
    type[end] = TYPE_OUTER_RIGHT;
    list[(*listsize)++] = end;

    return 1;
  }

  return 0;
}

static int kekule_add_extra_right(GRAPH *G, int list[], int *listsize, EDGE *edge) {
  int previous = edge->leftface;
  int extra = face_to_extra[previous];

  if (saturated[extra] == 0) {
    if (distance[extra] == -1 && distance[previous] % 2 == 0) {
      distance[extra] = distance[previous] + 1;
      path[extra] = edge;
      type[extra] = TYPE_EXTRA_RIGHT;
      list[(*listsize)++] = extra;

      return 1;
    } else {
      return 0;
    }
  }

  int end = edge->inverse->next->leftface;

  if (distance[end] == -1 && saturated[extra] % 2 == distance[previous] % 2) {
    distance[end] = distance[previous] + edge->label;
    path[end] = edge;
    type[end] = TYPE_BORDER_RIGHT;
    list[(*listsize)++] = end;

    return 1;
  }

  return 0;
}

static int kekule_add_extra_left(GRAPH *G, int list[], int *listsize, EDGE *edge) {
  int previous = edge->inverse->next->leftface;
  int end = edge->leftface;
  int extra = face_to_extra[end];

  if (saturated[extra] == 0) {
    if (distance[extra] == -1 && distance[previous] % 2 == 0) {
      distance[extra] = distance[previous] + 1;
      path[extra] = edge;
      type[extra] = TYPE_EXTRA_LEFT;
      list[(*listsize)++] = extra;

      return 1;
    } else {
      return 0;
    }
  }

  if (distance[end] == -1 &&
      ((edge->label % 2 == 0 && saturated[extra] / 2 == distance[previous] % 2) ||
       (edge->label % 2 == 1 && saturated[extra] % 2 == distance[previous] % 2))) {
    distance[end] = distance[previous] + edge->label;
    path[end] = edge;
    type[end] = TYPE_BORDER_LEFT;
    list[(*listsize)++] = end;

    return 1;
  }

  return 0;
}

static int kekule_step(GRAPH *G, int *nsat, int list[], int *listsize, int face) {
  EDGE *edge = firstedge[face];

  if (face >= G->faces) {

    /* extra */
    if (
      (kekule_add_outer_left(G, list, listsize, face, edge) && kekule_augmenting(G, list[*listsize - 1])) ||
      (kekule_add_outer_right(G, list, listsize, face, edge) && kekule_augmenting(G, list[*listsize - 1]))
    ) {
      *nsat += 2;
      return 1;
    }

  } else if (!edge->label) {

    /* inner face */
    if (
      (kekule_add_edge(G, list, listsize, edge) && kekule_augmenting(G, list[*listsize - 1])) ||
      (kekule_add_edge(G, list, listsize, edge->prev->inverse) && kekule_augmenting(G, list[*listsize - 1])) ||
      (kekule_add_edge(G, list, listsize, edge->inverse->next) && kekule_augmenting(G, list[*listsize - 1]))
    ) {
      *nsat += 2;
      return 1;
    }

  } else {

    /* outer face */
    if (
      (kekule_add_edge(G, list, listsize, edge) && kekule_augmenting(G, list[*listsize - 1])) ||
      (kekule_add_extra_right(G, list, listsize, edge) && kekule_augmenting(G, list[*listsize - 1])) ||
      (kekule_add_extra_left(G, list, listsize, edge->prev->inverse) && kekule_augmenting(G, list[*listsize - 1]))
    ) {
      *nsat += 2;
      return 1;
    }

  }

  return 0;
}

static void kekule_bipartite(GRAPH *G, int *nsat) {
  int start, i;
  int list[G->faces + G->boundary_length], listsize, index;

  for (start = 0; start < G->faces + G->boundary_length; start++) {
    if (!saturated[start]) {
      for (i = 0; i < G->faces + G->boundary_length; i++) distance[i] = -1;

      distance[start] = 0;
      list[0] = start;
      listsize = 1;

      for (index = 0; index < listsize; index++) {
        if (kekule_step(G, nsat, list, &listsize, list[index])) break;
      }
    }
  }
}

static int kekule_exhaustive_step(GRAPH *G, int *nsat, int face);

static int kekule_exhaustive_augmenting(GRAPH *G, int *nsat, int end, int *success) {
  *success = 0;
  if (kekule_augmenting(G, end)) {
    *nsat += 2;
    distance[end] = -1;
    return 1;
  } else if (kekule_exhaustive_step(G, nsat, end)) {
    distance[end] = -1;
    return 1;
  } else {
    distance[end] = -1;
    return 0;
  }
}

static int kekule_exhaustive_step(GRAPH *G, int *nsat, int face) {
  EDGE *edge = firstedge[face];
  int end, success = 0;

  if (face >= G->faces) {

    /* extra */
    if (
      (kekule_add_outer_left(G, &end, &success, face, edge) && kekule_exhaustive_augmenting(G, nsat, end, &success)) ||
      (kekule_add_outer_right(G, &end, &success, face, edge) && kekule_exhaustive_augmenting(G, nsat, end, &success))
    ) return 1;

  } else if (!edge->label) {

    /* inner face */
    if (
      (kekule_add_edge(G, &end, &success, edge) && kekule_exhaustive_augmenting(G, nsat, end, &success)) ||
      (kekule_add_edge(G, &end, &success, edge->prev->inverse) && kekule_exhaustive_augmenting(G, nsat, end, &success)) ||
      (kekule_add_edge(G, &end, &success, edge->inverse->next) && kekule_exhaustive_augmenting(G, nsat, end, &success))
    ) return 1;

  } else {

    /* outer face */
    if (
      (kekule_add_edge(G, &end, &success, edge) && kekule_exhaustive_augmenting(G, nsat, end, &success)) ||
      (kekule_add_extra_right(G, &end, &success, edge) && kekule_exhaustive_augmenting(G, nsat, end, &success)) ||
      (kekule_add_extra_left(G, &end, &success, edge->prev->inverse) && kekule_exhaustive_augmenting(G, nsat, end, &success))
    ) return 1;

  }

  return 0;
}

static void kekule_exhaustive(GRAPH *G, int *nsat) {
  int start, i;

  for (i = 0; i < G->faces + G->boundary_length; i++) distance[i] = -1;

  for (start = 0; start < G->faces + G->boundary_length; start++) {
    if (!saturated[start]) {
      distance[start] = 0;

      kekule_exhaustive_step(G, nsat, start);

      distance[start] = -1;
    }
  }
}

static int kekule(GRAPH *G) {
  int i, nsat = 0, total = G->faces + G->boundary_length;

  G->matching = 1 - G->matching;

  for (i = 0; i < total; i++) saturated[i] = 0;

  kekule_greedy(G, &nsat);

  if (nsat == total) return 1;

  if (BIPARTITE) {
    kekule_bipartite(G, &nsat);
  } else {
    kekule_exhaustive(G, &nsat);
  }

  return (nsat == total);
}


static int canon_angle_labeling(GRAPH *G, EDGE **numberings, int nbop, int nbf) {
  int fn, numb, i, c;

  for (fn = 0; fn < nbf; fn++) {
    numb = filtered_numbs[fn];
    for (i = 0; i < G->boundary_length; i++) {
      if (anglecode[i] == 0) continue;
      if (numb < nbop) {
        c = NUMBER(G, numberings, numb, i)->label;
      } else {
        c = NUMBER(G, numberings, numb, i)->inverse->prev->inverse->label;
      }
      if (c > anglecode[i]) {
        break;
      } else if (c < anglecode[i]) {
        return 0;
      }
    }
  }

  return 1;
}

static void label_angles(GRAPH* G, EDGE**numberings, int nbop, int nbf, int n) {
  EDGE *edge;
  int vertex, label;

  for (; n < G->boundary_length; n++) {
    edge = numberings[n];
    vertex = edge->end;
    if (labeled[vertex] == G->outer[vertex] - 1) {
      edge->label = restlabel[vertex] + 1;
    } else {
      labeled[vertex]++;
      for (label = 0; label <= restlabel[vertex]; label++) {
        edge->label = label + 1;
        restlabel[vertex] -= label;
        anglecode[n] = label + 1;
        label_angles(G, numberings, nbop, nbf, n + 1);
        restlabel[vertex] += label;
      }
      anglecode[n] = 0;
      labeled[vertex]--;
      return;
    }
  }

  if (canon_angle_labeling(G, numberings, nbop, nbf)) {
    if (!KEKULE || kekule(G)) {
      global_count++;
      if (OUTPUT) write_dual_planar_code(G);
    }
  }
}


static int canon_vertex_labeling(GRAPH *G, EDGE **numberings, int nbtot, int *nbf) {
  int numb, i, c, fn = 0;

  for (numb = 1; numb < nbtot; numb++) {
    for (i = 0; i < G->boundary_length; i++) {
      if (vertexcode[i] == 0) continue;
      c = G->label[NUMBER(G, numberings, numb, i)->end];
      if (c > vertexcode[i]) {
        break;
      } else if (c < vertexcode[i]) {
        return 0;
      }
    }
    if (i == G->boundary_length) filtered_numbs[fn++] = numb;
  }

  *nbf = fn;

  return 1;
}

static void label_vertices(GRAPH *G, int *facecount, EDGE **numberings, int nbtot, int nbop, int n) {
  int vertex, label, nbf;

  for (; n < G->boundary_length; n++) {
    vertex = numberings[n]->end;
    if (G->label[vertex] == 0) {
      for (label = G->deg[vertex] + G->outer[vertex]; label <= G->maxdeg; label++) {
        if (facecount[label]) {
          G->label[vertex] = vertexcode[n] = label;
          restlabel[vertex] = label - G->deg[vertex] - G->outer[vertex];
          facecount[label]--;
          label_vertices(G, facecount, numberings, nbtot, nbop, n + 1);
          G->label[vertex] = 0;
          facecount[label]++;
        }
      }
      vertexcode[n] = 0;
      return;
    }
  }

  if (canon_vertex_labeling(G, numberings, nbtot, &nbf)) {
    labeled_count++;
    if (nbf == 0) labeled_trivial++;
    label_angles(G, numberings, nbop, nbf, 0);
  }
}


static int testcanon(GRAPH *G, EDGE *givenedge, int representation[], int mirror) {
  register EDGE *run;
  register int vertex;
  EDGE *temp, *startedge[G->size];
  int number[G->size], i;
  int last_number, actual_number;

  for (i = 0; i < G->size; i++) number[i] = 0;

  number[givenedge->start] = 1;
  number[givenedge->end] = 2;
  last_number = 2;
  startedge[1] = givenedge->inverse;

  actual_number = 1;
  temp = givenedge;

  while (actual_number <= G->size) {
    if ((mirror) ? temp->prev->inverse->label : temp->inverse->label) {
      if (-1 < *representation) return 2;
      representation++;
    }
    for (run = (mirror) ? temp->prev : temp->next;
         run != temp;
         run = (mirror) ? run->prev : run->next) {
      vertex = run->end;
      if (!number[vertex]) {
        startedge[last_number++] = run->inverse;
        number[vertex] = last_number;
        vertex = G->deg[vertex] + G->size;
      } else {
        vertex = number[vertex];
      }
      if (vertex > *representation) return 0;
      if (vertex < *representation) return 2;
      representation++;

      if ((mirror) ? run->prev->inverse->label : run->inverse->label) {
        if (-1 < *representation) return 2;
        representation++;
      }
    }
    if (0 > *representation) return 0;
    if (0 < *representation) return 2;
    representation++;
    temp = startedge[actual_number++];
  }

  return 1;
}

static int testcanon_init(GRAPH *G, EDGE *givenedge, int representation[], int mirror) {
  register EDGE *run;
  register int vertex;
  EDGE *temp, *startedge[G->size];
  int number[G->size], i;
  int last_number, actual_number, better = 0;

  for (i = 0; i < G->size; i++) number[i] = 0;

  number[givenedge->start] = 1;
  number[givenedge->end] = 2;
  last_number = 2;
  startedge[1] = givenedge->inverse;

  actual_number = 1;
  temp = givenedge;

  while (actual_number <= G->size) {
    if ((mirror) ? temp->prev->inverse->label : temp->inverse->label) {
      if (better) {
        *representation = -1;
      } else {
        if (-1 < *representation) {
          better = 1;
          *representation = -1;
        }
      }
      representation++;
    }
    for (run = (mirror) ? temp->prev : temp->next;
         run != temp;
         run = (mirror) ? run->prev : run->next) {
      vertex = run->end;
      if (!number[vertex]) {
        startedge[last_number++] = run->inverse;
        number[vertex] = last_number;
        vertex = G->deg[vertex] + G->size;
      } else {
        vertex = number[vertex];
      }
      if (better) {
        *representation = vertex;
      } else {
        if (vertex > *representation) return 0;
        if (vertex < *representation) {
          better = 1;
          *representation = vertex;
        }
      }
      representation++;

      if ((mirror) ? run->prev->inverse->label : run->inverse->label) {
        if (better) {
          *representation = -1;
        } else {
          if (-1 < *representation) {
            better = 1;
            *representation = -1;
          }
        }
        representation++;
      }
    }
    if (better) {
      *representation = 0;
    } else {
      if (0 > *representation) return 0;
      if (0 < *representation) {
        better = 1;
        *representation = 0;
      }
    }
    representation++;
    temp = startedge[actual_number++];
  }

  if (better) return 2;
  else return 1;
}

static void construct_numb(GRAPH *G, EDGE *givenedge, EDGE **numberings, int mirror) {
  register EDGE *run;
  EDGE *end;

  run = end = givenedge;
  do {
    *numberings = run;
    numberings++;
    run = (mirror) ? run->inverse->prev : run->inverse->next;
  } while (run != end);
}

static int canon(GRAPH *G, EDGE **numberings, int *nbtot, int *nbop) {
  int last_vertex, minstart, maxend, list_length_last = 0;
  EDGE *startlist_last[G->maxdeg], *run, *end;

  last_vertex = G->size - 1;
  minstart = G->deg[last_vertex];
  maxend = 0;

  /* Find the edges starting in last_vertex with end of maximal degree. */
  run = end = G->firstedge[last_vertex];
  do {
    if (run->label > 0 || run->inverse->label > 0) {
      if (G->deg[run->end] > maxend) {
        list_length_last = 1;
        startlist_last[0] = run;
        maxend = G->deg[run->end];
      } else if (G->deg[run->end] == maxend) {
        startlist_last[list_length_last++] = run;
      }
    }
    run = run->next;
  } while (run != end);

  int i, list_length;
  EDGE *startlist[G->maxsize * G->maxdeg];

  /* Find all boundary edges with start of degree minstart and outer 1, and end
   * of degree maxend. If a better edge is found, last_vertex is not canonical */
  list_length = 0;
  for (i = 0; i < last_vertex; i++) {
    if (G->outer[i] != 1) continue;
    if (G->deg[i] < minstart) return 0;
    if (G->deg[i] == minstart) {
      run = end = G->firstedge[i];
      do {
        if (run->label > 0 || run->inverse->label > 0) {
          if (G->deg[run->end] > maxend) return 0;
          if (G->deg[run->end] == maxend) {
            startlist[list_length++] = run;
          }
        }
        run = run->next;
      } while (run != end);
    }
  }

  int representation[G->size * G->maxdeg];
  EDGE *numblist[G->size * G->maxdeg], *numblist_mirror[G->size * G->maxdeg];
  int test, numbs = 0, numbs_mirror = 0;

  /* Determine the smallest representation around last_vertex */
  representation[0] = G->size + G->maxdeg + 1;
  for (i = 0; i < list_length_last; i++) {
    test = testcanon_init(G, startlist_last[i], representation, 0);
    if (test == 1) {
      numblist[numbs++] = startlist_last[i];
    } else if (test == 2) {
      numblist[0] = startlist_last[i];
      numbs = 1; numbs_mirror = 0;
    }
    test = testcanon_init(G, startlist_last[i], representation, 1);
    if (test == 1) {
      numblist_mirror[numbs_mirror++] = startlist_last[i];
    } else if (test == 2) {
      numblist_mirror[0] = startlist_last[i];
      numbs = 0; numbs_mirror = 1;
    }
  }

  /* Check wether there are smaller representations for other edges */
  for (i = 0; i < list_length; i++) {
    test = testcanon(G, startlist[i], representation, 0);
    if (test == 1) {
      numblist[numbs++] = startlist[i];
    } else if (test == 2) return 0;
    test = testcanon(G, startlist[i], representation, 1);
    if (test == 1) {
      numblist_mirror[numbs_mirror++] = startlist[i];
    } else if (test == 2) return 0;
  }

  *nbop = numbs;
  *nbtot = numbs + numbs_mirror;

  /* Construct numberings of boundary edges */
  if (numbs == 0) {
    *nbop = numbs_mirror;
    for (i = 0; i < numbs_mirror; i++) {
      construct_numb(G, numblist_mirror[i], numberings + i * G->boundary_length, 0);
    }
  } else if (numblist[0]->label > 0) {
    for (i = 0; i < numbs; i++) {
      construct_numb(G, numblist[i], numberings + i * G->boundary_length, 0);
    }
    for (i = 0; i < numbs_mirror; i++, numbs++) {
      construct_numb(G, numblist_mirror[i], numberings + numbs * G->boundary_length, 1);
    }
  } else {
    for (i = 0; i < numbs; i++) {
      construct_numb(G, numblist[i]->inverse, numberings + i * G->boundary_length, 0);
    }
    for (i = 0; i < numbs_mirror; i++, numbs++) {
      construct_numb(G, numblist_mirror[i]->inverse, numberings + numbs * G->boundary_length, 1);
    }
  }

  return 1;
}


static int is_augmenting(GRAPH* G, int *facecount) {
  int i, j, msum = 0, nsum = G->size - G->maxsize + 1;

  /* Property 2 */
  for (i = 2; i <= G->maxdeg; i++) {
    for (j = 1; j < i; j++) {
      msum += COUNT(G, j, i - j);
    }
    nsum += facecount[i];
    if (msum < nsum) return 0;
  }

  return 1;
}

static void add_vertex(GRAPH* G, EDGE* edge, int l) {
  int i, vertex, new_vertex = G->size;
  EDGE *temp = edge, *next_temp, *new_edge = 0, *new_inverse = 0, *first_inverse = 0;

  for (i = 0; i <= l; i++) {
    vertex = temp->end;
    next_temp = temp->inverse->next;

    /* Add new_edge and new_inverse */
    new_edge = malloc(sizeof(EDGE));
    new_inverse = malloc(sizeof(EDGE));
    new_edge->start = new_inverse->end = vertex;
    new_edge->end = new_inverse->start = new_vertex;
    new_edge->inverse = new_inverse; new_inverse->inverse = new_edge;
    new_edge->matching = new_inverse->matching = 2 * G->matching;

    /* Connect with other edges */
    new_edge->prev = temp->inverse; new_edge->prev->next = new_edge;
    new_edge->next = next_temp; new_edge->next->prev = new_edge;
    if (i == 0) {
      first_inverse = new_inverse;
      new_edge->label = 1;
      new_edge->leftface = new_inverse->rightface = G->faces++;
    } else {
      new_inverse->next = temp->prev->inverse; new_inverse->next->prev = new_inverse;
      new_edge->label = 0;
      temp->label = 0;
      new_edge->leftface = new_inverse->rightface = temp->leftface;
      new_inverse->next->leftface = temp->prev->rightface = temp->leftface;
    }
    new_inverse->label = 0;
    temp = next_temp;
  }
  new_inverse->prev = first_inverse; first_inverse->next = new_inverse;
  new_inverse->label = 1;
  new_inverse->leftface = new_edge->rightface = G->faces++;

  /* Update graph */
  G->size++;
  G->edges += 2 * (l + 1);
  G->boundary_length = G->boundary_length - l + 2;
  G->deg[new_vertex] = l + 1;
  G->outer[new_vertex] = 1;
  G->label[new_vertex] = 0;
  G->firstedge[new_vertex] = new_inverse;
}

static void remove_vertex(GRAPH* G) {
  int last_vertex = G->size - 1, deg = G->deg[last_vertex];
  EDGE *end, *run, *temp, *temp_inverse;

  end = run = G->firstedge[last_vertex];
  do {
    temp = run;
    temp_inverse = run->inverse;
    temp_inverse->next->prev = temp_inverse->prev;
    temp_inverse->prev->next = temp_inverse->next;
    temp_inverse->next->label = 1;
    run = run->next;
    free(temp); free(temp_inverse);
  } while (run != end);

  /* Update graph */
  G->size--;
  G->edges -= 2 * deg;
  G->boundary_length = G->boundary_length + deg - 3;
  G->faces -= 2;
}

static void construct_graphs(GRAPH *G, int *facecount, EDGE **numberings, int nbtot, int nbop) {
  int number, number2, i, cont;
  EDGE *edge, *temp;
  int maxlength, l, length, vertex, prev_vertex;
  int new_nbtot, new_nbop;
  EDGE *new_numberings[2 * (G->boundary_length + 2) * (G->boundary_length + 2)];

  for (number = 0; number < G->boundary_length; number++) {
    edge = numberings[number];

    /* Check wether edge is smallest in orientation preserving orbit */
    cont = 0;
    for (i = 1; i < nbop; i++) {
      temp = NUMBER(G, numberings, i, number);
      if (temp->start < edge->start || (temp->start == edge->start && temp->end < edge->end)) {
        cont = 1;
        break;
      }
    }
    if (cont) continue;

    /* Determine maxlength of boundary segment */
    maxlength = G->maxdeg - 1;
    while (maxlength > 0 && facecount[maxlength + 1] == 0) maxlength--;
    if (G->boundary_length + 1 < maxlength) maxlength = G->boundary_length + 1;

    length = 0;
    for (l = 0; l < maxlength; l++) {
      length = l + 1;
      number2 = (number + length) % G->boundary_length;
      prev_vertex = vertex;
      vertex = numberings[number2]->start;

      /* Update counter, deg, outer for boundary segment (edge, l) */
      if (l == 0) {
        /* Update first vertex of boundary segment */
        DECR_COUNT(G, G->deg[vertex], G->outer[vertex]);
        G->deg[vertex]++; G->outer[vertex]++;
        INCR_COUNT(G, G->deg[vertex], G->outer[vertex]);
        /* Update new vertex */
        INCR_COUNT(G, 1, 1);
      } else {
        /* Update previous vertex of boundary segment */
        DECR_COUNT(G, G->deg[prev_vertex], G->outer[prev_vertex]);
        G->outer[prev_vertex]--;
        INCR_COUNT(G, G->deg[prev_vertex], G->outer[prev_vertex]);

        /* Update last vertex of boundary segment */
        DECR_COUNT(G, G->deg[vertex], G->outer[vertex]);
        G->deg[vertex]++;
        INCR_COUNT(G, G->deg[vertex], G->outer[vertex]);
        /* Update new vertex */
        DECR_COUNT(G, l, 1);
        INCR_COUNT(G, l + 1, 1);

        /* Update facecount */
        if (G->outer[prev_vertex] == 0) {
          facecount[G->deg[prev_vertex]]--;
          if (facecount[G->deg[prev_vertex]] < 0) break;
        }
      }

      /* Filter kekule */
      if (KEKULE && (G->size + 1 == G->maxsize) &&
          (G->maxedges - G->boundary_length + l) % 2 != 0) {
        continue;
      }

      /* Check wether (edge, length) is smallest in orbit */
      cont = 0;
      for (i = nbop; i < nbtot; i++) {
        temp = NUMBER(G, numberings, i, number2);
        if (temp->end < edge->start || (temp->end == edge->start && temp->start < edge->end)) {
          cont = 1;
          break;
        }
      }
      if (cont) continue;

      /* Optimalization: the degree of the new vertex is at most
       * the minimum of the degrees of vertices with outer 1. */
      i = 0;
      while (COUNT(G, i, 1) == 0) i++;
      if (length > i) continue;

      /* Check wether boundary segment (edge, length) is augmenting */
      if (!is_augmenting(G, facecount)) continue;

      /* Add new vertex */
      add_vertex(G, edge, l);

      /* Check wether new vertex is canonical */
      if (canon(G, new_numberings, &new_nbtot, &new_nbop)) {
        if (G->size == G->maxsize) {
          cont = 0;
          if (G->boundary_length == 2) {
            temp = new_numberings[0];
            if (G->deg[temp->start] == G->deg[temp->end]) {
              if (facecount[G->deg[temp->start] + 1] == 2) cont = 1;
            } else {
              if (facecount[G->deg[temp->start] + 1] && facecount[G->deg[temp->end] + 1]) cont = 1;
            }
          }
          if (!cont) {
            dual_index++;
            if (MODULO && dual_index == MODULO) dual_index = 0;
            if ((!MODULO && !INDEX) || dual_index == INDEX) {
              dual_count++;
              if (new_nbtot == 1) dual_trivial++;
              if (DUALS) {
                if (OUTPUT) write_planar_code(G);
              } else {
                /* Label vertices, then angles, and output dual graphs */
                label_vertices(G, facecount, new_numberings, new_nbtot, new_nbop, 0);
              }
            }
          }
        } else {
          /* Construct descendants */
          construct_graphs(G, facecount, new_numberings, new_nbtot, new_nbop);
        }
      }

      /* Remove vertex */
      remove_vertex(G);
    }

    /* Undo changes to counter, deg, outer */
    if (length == 1) {
      /* Update only vertex of boundary segment */
      vertex = edge->end;
      DECR_COUNT(G, G->deg[vertex], G->outer[vertex]);
      G->deg[vertex]--; G->outer[vertex]--;
      INCR_COUNT(G, G->deg[vertex], G->outer[vertex]);
      /* Update removed vertex */
      DECR_COUNT(G, 1, 1);
    } else {
      /* Update first vertex of boundary segment */
      vertex = edge->end;
      DECR_COUNT(G, G->deg[vertex], G->outer[vertex]);
      G->deg[vertex]--;
      INCR_COUNT(G, G->deg[vertex], G->outer[vertex]);
      /* Update boundary segment */
      for (i = 1; i < length - 1; i++) {
        vertex = numberings[(number + i) % G->boundary_length]->end;
        if (G->outer[vertex] == 0) facecount[G->deg[vertex]]++;
        DECR_COUNT(G, G->deg[vertex], G->outer[vertex]);
        G->deg[vertex]--; G->outer[vertex]++;
        INCR_COUNT(G, G->deg[vertex], G->outer[vertex]);
      }
      /* Update last vertex of boundary segment */
      vertex = numberings[(number + length - 1) % G->boundary_length]->end;
      DECR_COUNT(G, G->deg[vertex], G->outer[vertex]);
      G->deg[vertex]--;
      INCR_COUNT(G, G->deg[vertex], G->outer[vertex]);
      /* Update removed vertex */
      DECR_COUNT(G, length, 1);
    }
  }
}


static void write_help() {
  fprintf(stdout, "Usage: ngons [-p] [-d] [-o OUTFILE] [-m M] [-i I] SPECS\n\n");
  fprintf(stdout, " -p,--planarcode write planar code to stdout or outfile\n");
  fprintf(stdout, " -d,--duals      generate inner duals\n");
  fprintf(stdout, " -k,--kekule     filter kekule structures\n");
  fprintf(stdout, " -o,--output     write to OUTFILE instead of stdout\n");
  fprintf(stdout, " -m,--modulo\n");
  fprintf(stdout, " -i,--index      only use inner dual with index I (modulo M)\n");
  fprintf(stdout, " SPECS           sequence of pairs \"n:m\", meaning there are m n-gons\n");
}


int main(int argc, char *argv[]) {
  int c, option_index;
  char *charp;
  clock_t start, end;
  double cpu_time;

  DUALS = 0;
  KEKULE = 0;
  OUTPUT = 0;
  OUTFILE = stdout;
  BIPARTITE = 1;
  MODULO = 0;
  INDEX = 0;

  /* Process command line options */
  static struct option long_options[] = {
    {"planarcode", no_argument,       0, 'p'},
    {"duals",       no_argument,       0, 'd'},
    {"kekule",      no_argument,       0, 'k'},
    {"output",      required_argument, 0, 'o'},
    {"modulo",      required_argument, 0, 'm'},
    {"index",       required_argument, 0, 'i'},
    {"help",        no_argument,       0, 'h'},
  };

  while (1) {
    c = getopt_long(argc, argv, "pdko:m:i:h", long_options, &option_index);
    if (c == -1) break;
    switch (c) {
      case 'p':
        OUTPUT = 1;
        break;
      case 'd':
        DUALS = 1;
        break;
      case 'k':
        KEKULE = 1;
        break;
      case 'o':
        OUTFILE = fopen(optarg, "w");
        break;
      case 'm':
        MODULO = strtoul(optarg, &charp, 10);
        if (*charp != '\0') {
          fprintf(stderr, "\"%s\" is no numeric value.\n", optarg);
          return 1;
        }
        if (MODULO == 1) {
          fprintf(stderr, "Modulo 1 is impossible.\n");
          return 1;
        }
        break;
      case 'i':
        INDEX = strtoul(optarg, &charp, 10);
        if (*charp != '\0') {
          fprintf(stderr, "\"%s\" is no numeric value.\n", optarg);
          return 1;
        }
        break;
      default:
        write_help();
        return 1;
    }
  }

  if (MODULO && INDEX >= MODULO) {
    fprintf(stderr, "Index %ld is impossible modulo %ld.\n", INDEX, MODULO);
    return 1;
  }

  if (optind == argc) {
    write_help();
    return 1;
  }

  int i, face, count, countsize;
  GRAPH *G = malloc(sizeof(GRAPH));

  /* Determine maxdeg, maxsize and maxedges */
  G->maxdeg = G->maxsize = G->maxedges = 0;
  for (i = optind; i < argc; i++) {
    sscanf(argv[i], "%d:%d", &face, &count);
    if (face > G->maxdeg) G->maxdeg = face;
    if (face % 2) BIPARTITE = 0;
    G->maxsize += count;
    G->maxedges += count * face;
  }

  if (G->maxsize == 1) {
    fprintf(stderr, "ERROR: The graph should have at least two faces.");
    return 1;
  }

  /* Initialize facecount */
  int facecount[G->maxdeg + 1];
  for (i = 0; i <= G->maxdeg; i++) facecount[i] = 0;
  for (i = optind; i < argc; i++) {
    sscanf(argv[i], "%d:%d", &face, &count);
    facecount[face] = count;
  }

  /* Initialize graph */
  countsize = (G->maxdeg + 1) * (G->maxdeg + 2); /* (G->maxdeg + 1) * (G->maxdeg / 2 + 2) */
  G->counter = malloc(countsize * sizeof(int));
  for (i = 0; i < countsize; i++) G->counter[i] = 0;
  G->deg = malloc(G->maxsize * sizeof(int));
  G->outer = malloc(G->maxsize * sizeof(int));
  G->label = malloc(G->maxsize * sizeof(int));
  G->firstedge = malloc(G->maxsize * sizeof(EDGE*));
  G->matching = 0;

  /* Initialize two vertices and two directed edge */
  EDGE *edge = malloc(sizeof(EDGE));
  EDGE *inverse = malloc(sizeof(EDGE));
  edge->start = inverse->end = 0;
  edge->end = inverse->start = 1;
  edge->prev = edge->next = inverse->inverse = edge;
  inverse->prev = inverse->next = edge->inverse = inverse;
  edge->label = inverse->label = 1;
  edge->leftface = inverse->rightface = 0;
  edge->leftface = inverse->rightface = 1;
  edge->matching = inverse->matching = 2 * G->matching;

  G->size = 2;
  G->edges = 2;
  G->boundary_length = 2;
  G->faces = 2;

  INCR_COUNT(G, 1, 1); INCR_COUNT(G, 1, 1);

  G->deg[0] = G->deg[1] = 1;
  G->outer[0] = G->outer[1] = 1;
  G->label[0] = G->label[1] = 0;
  G->firstedge[0] = edge; G->firstedge[1] = inverse;

  /* Initialize numberings */
  int nbtot = 4, nbop = 2;
  EDGE *numberings[2 * G->boundary_length * G->boundary_length * sizeof(EDGE*)];
  NUMBER(G, numberings, 0, 0) = edge; NUMBER(G, numberings, 0, 1) = inverse;
  NUMBER(G, numberings, 1, 0) = inverse; NUMBER(G, numberings, 1, 1) = edge;
  NUMBER(G, numberings, 2, 0) = edge; NUMBER(G, numberings, 2, 1) = inverse;
  NUMBER(G, numberings, 3, 0) = inverse; NUMBER(G, numberings, 3, 1) = edge;

  /* Initialize global variables */
  global_count = dual_count = dual_index = labeled_count = 0;
  dual_trivial = labeled_trivial = 0;
  if (!DUALS) {
    vertexcode = malloc(G->maxedges * sizeof(int));
    anglecode = malloc(G->maxedges * sizeof(int));
    for (i = 0; i < G->maxedges; i++) anglecode[i] = 0;
    labeled = malloc(G->maxsize * sizeof(int));
    for (i = 0; i < G->size; i++) labeled[i] = 0;
    restlabel = malloc(G->maxsize * sizeof(int));
    filtered_numbs = malloc(2 * G->maxedges * sizeof(int));
  }
  if (KEKULE) {
    face_to_extra = malloc((G->maxedges / 2) * sizeof(int));
    saturated = malloc(G->maxedges * sizeof(int));
    distance = malloc(G->maxedges * sizeof(int));
    type = malloc(G->maxedges * sizeof(int));
    firstedge = malloc(G->maxedges * sizeof(EDGE*));
    path = malloc(G->maxedges * sizeof(EDGE*));
  }

  /* Start Construction */
  if (OUTPUT) write_header(OUTFILE);
  start = clock();
  if (G->size == G->maxsize) {
    dual_count = 1;
    if (DUALS) {
      if (OUTPUT) write_planar_code(G);
    } else {
      label_vertices(G, facecount, numberings, nbtot, nbop, 0);
    }
  } else {
    construct_graphs(G, facecount, numberings, nbtot, nbop);
  }
  end = clock();
  cpu_time = ((double) (end - start)) / CLOCKS_PER_SEC;

  /* Print results */
  for (i = 0; i < argc; i++) fprintf(stderr, "%s ", argv[i]);
  fprintf(stderr, "\n\n");
  if (DUALS) {
    fprintf(stderr, "inner duals:    %ld (%ld trivial)\n\n",
            dual_count, dual_trivial);
    fprintf(stderr, "CPU time:       %.2fs\n"
                    "graphs/s:       %.0f\n",
            cpu_time, dual_count / cpu_time);
  } else {
    if (KEKULE) {
      fprintf(stderr, "kekulean graphs: %ld\n", global_count);
    } else {
      fprintf(stderr, "graphs:          %ld\n", global_count);
    }
    fprintf(stderr, "inner duals:     %ld (%ld trivial)\n"
                    "vertex-labeled:  %ld (%ld trivial)\n\n",
            dual_count, dual_trivial, labeled_count, labeled_trivial);
    fprintf(stderr, "CPU time:        %.2fs\n"
                    "graphs/s:        %.0f\n",
            cpu_time, global_count / cpu_time);
  }

  /* Free memory */
  if (!DUALS) {
    free(vertexcode); free(anglecode);
    free(labeled); free(restlabel);
    free(filtered_numbs);
  }
  if (KEKULE) {
    free(saturated); free(face_to_extra);
    free(distance); free(type);
    free(firstedge); free(path);
  }
  free(edge); free(inverse);
  free(G->deg); free(G->outer); free(G->label);
  free(G->counter); free(G->firstedge);
  free(G);

  return 0;
}


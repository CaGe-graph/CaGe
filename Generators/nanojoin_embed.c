#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <unistd.h>
#include <errno.h>

/* DATA STRUCTURES */

typedef struct vertex {
    int vertexnr;
    int degree;

    float co[3];
    unsigned char partoftube;

    unsigned char embedded;
} vertex;

typedef struct edge 
{
    vertex* startvertex;
    vertex* endvertex;

    struct edge* next;
    struct edge* prev;
    struct edge* inv;
} edge;

typedef struct graph {
    int nrofvertices;
    vertex **vertices;
    edge **firstedges;
    int nrofrings;
    struct tube **tubes;
} graph;

typedef struct tube {
    int parameters[2];
    int length;
    edge *firstedge;
} tube;



float UNITLENGTH = 1.42;

/*
GRAPH FUNCTIONS
*/

void init_vertex(int nr, vertex** v) {
        (*v)->vertexnr = nr;
        (*v)->co[0] = (*v)->co[1] = (*v)->co[2] = 0;
        (*v)->degree = 0;
        (*v)->embedded = 0;
        (*v)->partoftube = 0;
}

edge* findInverse(int i, edge* startedge) {
        edge* inverse;
        inverse = startedge;
        while (inverse->endvertex->vertexnr != i) {
            inverse = inverse->next;

        }
        return inverse;
}

graph* readNextGraphVega(FILE* file) {
    int vertexnumber;
    int r;
    float x, y, z;
    int degree;
    int neighbour;
    int neighbours[3];
    char* string = malloc(128*sizeof(char));
    char* rfgets;
    edge *currentedge;
    edge *previous;
    edge *inverse;

    int verticesarraysize = 10;
    graph *g;

    g = malloc(sizeof(graph));
    g->nrofvertices = 0;
    g->vertices = calloc(verticesarraysize, sizeof(vertex));
    g->firstedges = malloc(verticesarraysize*sizeof(edge));

    rfgets = fgets(string, 128, stdin);
    r = sscanf(string, "%d %f %f %f %d %d %d", &vertexnumber, &x, &y, &z, &(neighbours[0]), &(neighbours[1]), &(neighbours[2]));
    /* read vertex by vertex */
    while (!feof(stdin) && vertexnumber != 0) {
        degree = r - 4;
        
        /* extend vertices array if necessary */
        while (vertexnumber - 1 >= verticesarraysize) {
            //printf("Extending array to %d\n", verticesarraysize * 2);
            g->vertices = realloc(g->vertices, 2*verticesarraysize*sizeof(vertex));
            for (int i=verticesarraysize; i < 2 * verticesarraysize; i++) {
                g->vertices[i] = NULL;
            }
            verticesarraysize *= 2;
            g->firstedges = realloc(g->firstedges, verticesarraysize*sizeof(edge));
        }

        if (g->vertices[vertexnumber - 1] == NULL) {
            g->vertices[vertexnumber - 1] = malloc(sizeof(vertex));
            init_vertex(vertexnumber - 1, &g->vertices[vertexnumber - 1]);
        }

        g->vertices[vertexnumber-1]->degree = degree;

        previous = NULL;
        for (int i=0; i < degree; i++) {
            neighbour = neighbours[i];

             /* extend vertices array if necessary */
            while (neighbour - 1 >= verticesarraysize) {
                g->vertices = realloc(g->vertices, 2*verticesarraysize*sizeof(vertex));
                for (int i=verticesarraysize; i < 2 * verticesarraysize; i++) {
                    g->vertices[i] = NULL;
                }
                verticesarraysize *= 2;
                g->firstedges = realloc(g->firstedges, verticesarraysize*sizeof(edge));
            }

            /* neighbour does not exist yet */
            if (g->vertices[neighbour - 1] == NULL) {
                g->vertices[neighbour - 1] = malloc(sizeof(vertex));
                init_vertex(neighbour - 1, &g->vertices[neighbour - 1]);
            }

            currentedge = malloc(sizeof(edge));
            currentedge->startvertex = g->vertices[vertexnumber - 1];
            currentedge->endvertex = g->vertices[neighbour - 1];

            if (previous == NULL) {
                /* first edge */
                g->firstedges[vertexnumber - 1] = currentedge;
            } else {
                previous->next = currentedge;
                currentedge->prev = previous;
            }

            /* the inverse edge exists */
            if (neighbour < vertexnumber) {
                inverse = findInverse(vertexnumber - 1, g->firstedges[neighbour - 1]);
                currentedge->inv = inverse;
                inverse->inv = currentedge;
            }
            previous = currentedge;
        }

        previous->next = g->firstedges[vertexnumber - 1];
        g->firstedges[vertexnumber - 1]->prev = previous;

        g->nrofvertices += 1;
        
        rfgets = fgets(string, 128, stdin);
        r = sscanf(string, "%d %f %f %f %d %d %d", &vertexnumber, &x, &y, &z, &(neighbours[0]), &(neighbours[1]), &(neighbours[2]));
    }
    free(string);
    return g;
}

graph* readNextGraphPlanar(FILE* file) {
    int numberofvertices;
    unsigned char current;
    edge *inverse, *last;
    graph *g;


    numberofvertices = (int) fgetc(file);
    if (numberofvertices < 0)
        return NULL;

    g = malloc(sizeof(graph));
    g->nrofvertices = numberofvertices;

    g->vertices = calloc(numberofvertices, sizeof(vertex));
    for (int i=0; i < numberofvertices; i++) {
        g->vertices[i] = malloc(sizeof(vertex));
        init_vertex(i, &(g->vertices[i]));
    }

    g->firstedges = calloc(numberofvertices, sizeof(edge));
    
    for (int i=0; i < numberofvertices; i++) {
        current = fgetc(file) - 1;
        g->firstedges[i] = malloc(sizeof(edge));
        g->firstedges[i]->startvertex = g->vertices[i];
        g->firstedges[i]->endvertex = g->vertices[current];

        if (current < i) {
            inverse = findInverse(i, g->firstedges[current]);
            inverse->inv = g->firstedges[i];
            g->firstedges[i]->inv = inverse;
        }

        last = g->firstedges[i];
        g->vertices[i]->degree += 1;

        current = fgetc(file);
        while (current != 0) {
            current -= 1;
            g->vertices[i]->degree += 1;

            last->next = malloc(sizeof(edge));
            last->next->prev = last;
            last = last->next;
            last->startvertex = g->vertices[i];
            last->endvertex = g->vertices[current];

            if (current < i) {
                inverse = findInverse(i, g->firstedges[current]);
                inverse->inv = last;
                last->inv = inverse;
            }

            current = fgetc(file);
        }

        last->next = g->firstedges[i];
        g->firstedges[i]->prev = last;
    }

    return g;
}

void free_graph(graph* g) {
    struct edge *e, *first;


    for (int i=0; i < g->nrofvertices; i++) {
        first = g->firstedges[i];
        e = first->next;
        do {
            e = e->next;
            free(e->prev);
        } while ( e != first);
        free(first);
    }
    free(g->firstedges);
    for (int i=0; i < g->nrofvertices; i++) {
        free(g->vertices[i]);
    }
    free(g->vertices);
    for (int i=0; i < g->nrofrings; i++) {
        free(g->tubes[i]);
    }
    free(g->tubes);
    free(g);
}

/*
JOIN
*/

void fillw3d(graph *g, int inputDescriptor, int* vertexTranslation) {
    char nChar;
    char lChar;
    int index;
    int *reverseTranslation;
    int rread;
    int vertexindex;

    int vertexnumber;
    float x, y, z;

    char* string = malloc(128*sizeof(char));

    reverseTranslation = malloc((g->nrofvertices + 1) * sizeof(int));

    for (int i=0; i < g->nrofvertices; i++) {
        reverseTranslation[vertexTranslation[i]] = i;
    }

    do {
        rread = read(inputDescriptor, &nChar, 1);
        printf("%c", nChar);
    } while (nChar != '\n');

    while (read(inputDescriptor, &nChar, 1) == 1) {
        index = 0;
        lChar = ' ';
        do {
            if (nChar != '\t' && (nChar != ' ' || lChar != ' '))
                string[index++] = nChar;
            lChar = nChar;
            rread = read(inputDescriptor, &nChar, 1);
        } while (nChar != '\n');
        string[index] = 0;
        if (string[0] == '0')
            break;

        sscanf(string, "%d %f %f %f ", &vertexnumber, &x, &y, &z);

        vertexindex = reverseTranslation[vertexnumber];
        g->vertices[vertexindex]->co[0] = x;
        g->vertices[vertexindex]->co[1] = y;
        g->vertices[vertexindex]->co[2] = z;
        g->vertices[vertexindex]->embedded = 1;
    }
    free(string);
    free(reverseTranslation);
}

void printw3d(graph *g) {
    edge *e;
    float* co;
    for (int i=0; i < g->nrofvertices; i++) {
        printf("%3d", i + 1);
        //printf("%d\t\t", i+1);
        if (g->vertices[i]->embedded == 1) {
            co = g->vertices[i]->co;
            for (int j=0; j < 3; j++) {
                printf(" %8.3f", co[j]);
                //printf("%f\t", co[j]);
            }
        } else {
            printf(" %8.3f %8.3f %8.3f", NAN, NAN, NAN);
            //printf("%f\t%f\t%f\t", NAN, NAN, NAN);
        }
        //printf("\t");
        e = g->firstedges[i];
        printf(" %3d", e->endvertex->vertexnr + 1);
        e = e->next;
        while (e != g->firstedges[i]) {
            printf(" %3d", e->endvertex->vertexnr + 1);
            e = e->next;
        }
        printf("\n");
    }
    printf("  0\n\n");
    fflush(stdout);
}

void printPartialw3d(graph *g, int descriptor, int **vertexTranslation) {
    int current;
    edge *e;

    dprintf(descriptor, ">>writegraph3d<<\n");
    current = 1;
    for (int i=0; i < g->nrofvertices; i++) {
        if (g->vertices[i]->partoftube == 0) {
            (*vertexTranslation)[i] = current++;
            g->vertices[i]->embedded = 1;
        } else {
            (*vertexTranslation)[i] = 0;
        }
    }
    for (int i=0; i < g->nrofvertices; i++) {
        if ((*vertexTranslation)[i] != 0) {
            current = 0;
            dprintf(descriptor, "%*d\t\t %.3f\t %.3f\t %.3f\t\t", 4, (*vertexTranslation)[i], 0.0, 0.0, 0.0);


            e = g->firstedges[i];
            do {
                if ((*vertexTranslation)[e->endvertex->vertexnr] != 0)
                    dprintf(descriptor, "  %d", (*vertexTranslation)[e->endvertex->vertexnr]);
                current++;
                e = e->next;
            } while (e != g->firstedges[i]);

            dprintf(descriptor, "\n");
        }
        
    }
    dprintf(descriptor, "0\n");

}

void embedJoin(graph *g) {
    int stdinPipe[2];
    int stdoutPipe[2];

    int nChild;
    int nResult;

    int (*vertexTranslation) = malloc(g->nrofvertices * sizeof(int));


    if (pipe(stdinPipe) < 0) {
        perror("allocating pipe for child input redirect");
        return;
    }

    if (pipe(stdoutPipe) < 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        perror("allocating pipe for child output redirect");
        return;
    }

    nChild = fork();

    if (0 == nChild) {
        //child process

        if (dup2(stdinPipe[0], STDIN_FILENO) == -1) {
          exit(errno);
        }

        if (dup2(stdoutPipe[1], STDOUT_FILENO) == -1) {
          exit(errno);
        }

        // redirect stderr
        if (dup2(stdoutPipe[1], STDERR_FILENO) == -1) {
          exit(errno);
        }

        // all these are for use by parent only
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);

        char *args[]={"Generators/embed", "-d3", "-is", NULL};
        nResult = execvp(args[0],args);

        exit(nResult);
    } else if (nChild > 0) {
         // parent continues here

        // close unused file descriptors, these are for child only
        close(stdinPipe[0]);
        close(stdoutPipe[1]); 

        printPartialw3d(g, stdinPipe[1], &vertexTranslation);

        // Just a char by char read here, you can change it accordingly
        fillw3d(g, stdoutPipe[0], vertexTranslation);

        // done with these in this example program, you would normally keep these
        // open of course as long as you want to talk to the child
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
    }else {
        // failed to create child
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
  }

  free(vertexTranslation);

}

/*
TUBE FUNCTIONS
*/

int markTube(edge* e, unsigned char **processed, tube **r) {
    int cyclesize = 0, length = 0, parameterindex = 0;
    int parameters[2];
    int index1, index2;
    int degree3;
    unsigned char encounteredtwo;
    edge **startcycle, **previouscycle, **newcycle, **tmp;
    edge *current, *start, *reference;
    edge *doublethree = NULL;

    /* set e vertex so degree sequence is (3,2)^l (2,3)^m */
    current = e;
    do {
        if (current->startvertex->degree == current->endvertex->degree && current->endvertex->degree == 3) {
            doublethree = current;
        }
        current = current->inv->next;
        cyclesize++;
    } while (current != e);


    if (doublethree != NULL) {
        e = doublethree->inv->next;
    }
    else if (e->startvertex->degree != 3) {
        e = e->inv->next;
    }

    
    /* save start cycle */
    parameters[0] = parameters[1] = 0;
    startcycle = malloc(cyclesize*sizeof(edge*));
    current = e;
    for(int i=0; i < cyclesize; i++) {
        startcycle[i] = current;

        parameters[parameterindex]++;
        if (current->startvertex->degree == current->endvertex->degree && current->endvertex->degree == 2)
            parameterindex = 1;

        current = current->inv->next;
        (*processed)[current->startvertex->vertexnr] = 1;
    }

    previouscycle = malloc(cyclesize*sizeof(edge*));
    newcycle = malloc(cyclesize*sizeof(edge*));

    memcpy(newcycle, startcycle, cyclesize*sizeof(edge*));

    start = startcycle[0];
    current = start;

    encounteredtwo = 0;
    while (current == start && encounteredtwo == 0) {
        tmp = newcycle;
        newcycle = previouscycle;
        previouscycle = tmp;
        reference = startcycle[0];
        current = current->next->inv->prev->inv;
        start = current;

        /* check if cycle */
        for (int i = 0; i < cyclesize; i++) {
            if (current->endvertex->degree == 2)
                encounteredtwo = 1;


            newcycle[i] = current;
            if (reference->endvertex->degree == 3) {
                current = current->inv->next;
            } else {
                current = current->inv->prev;
            }
            reference = reference->inv->next;
            previouscycle[i]->startvertex->partoftube = 1;
        }

        index1 = 0;
        index2 = 0;
        /* check if hexagons -> only degree three vertices*/
        degree3 = 0;
        while (2*degree3 < cyclesize && current != NULL) {
            while (startcycle[index1]->startvertex->degree != 3) {
                index1++;
            }

            while(startcycle[index2]->endvertex->degree != 2) {
                index2++;
            }

            if (previouscycle[index1]->next->inv != newcycle[index2]->inv->next) {
                current = NULL;
            }
            index1++;
            index2++;
            degree3++;

        }
        length++;
    }

    length--;

    (*r) = malloc(sizeof(tube));
    (*r)->firstedge = previouscycle[0];
    (*r)->length = length;
    for (int i=0; i < 2; i++) {
        (*r)->parameters[i] = parameters[i] / 2;
    }

    for (int i=0; i < cyclesize; i++)
        previouscycle[i]->startvertex->partoftube = 0;

    free(startcycle);
    free(previouscycle);
    free(newcycle);
    return cyclesize;
}

void findSeparationVertices(graph *g) {
    vertex **degreetwo;
    int cyclesize;
    int vertexnr;
    int nroftubes = 0;
    int nrofdegreetwo = 0;
    int nextvertex;
    unsigned char* processed;

    g->tubes = malloc(4*sizeof(tube*));

    edge* try;

    processed = calloc(g->nrofvertices, sizeof(unsigned char));
    degreetwo = malloc(g->nrofvertices * sizeof(vertex*));

    /* find degree 2 vertices */
    for (int i=0; i < g->nrofvertices; i++) {
        if (g->vertices[i]->degree == 2)
            degreetwo[nrofdegreetwo++] = g->vertices[i];
    }
    nextvertex = g->nrofvertices;
    for(int i=0; i < nrofdegreetwo; i++) {
        vertexnr = degreetwo[i]->vertexnr;
        if (processed[vertexnr] == 0) {
            try = g->firstedges[vertexnr];
            for (int j=0; j < 2; j++) {
                if (!processed[try->startvertex->vertexnr] && 
                    (try->inv->next->endvertex->degree == 2 || try->inv->next->inv->next->inv->next->endvertex->vertexnr == try->startvertex->vertexnr)) {
                    cyclesize = markTube(try, &processed, &(g->tubes[nroftubes++]));
                }
                try = try->next;
            }
        }
    }
    free(processed);
    free(degreetwo);
    g->nrofrings = nroftubes;
}

void idealTube(tube *t, float (***vertices)[3]) {
    float x, y, x2, y2, x3, y3, z3;
    float p, q;
    float sind, cosd, distance, multiplier, beta, radius;
    int l, m, count;
    edge *e;

    l = t->parameters[0];
    m = t->parameters[1];

    (*vertices) = malloc((t->length + 1)*sizeof(float (*)[3]));

    for (int ringnr = 0; ringnr < t->length + 1; ringnr++) {
        x = ringnr * sqrt(3) * UNITLENGTH/2;
        y = ringnr * (1.5 * UNITLENGTH);
        (*vertices)[ringnr] = malloc(2*(l+m)*sizeof(float[3]));

        count = 0;
        for (int i=0; i < 2 * l; i++) {
            x += sqrt(3) * UNITLENGTH/2;
            if (count % 2 == 1)
                y += 0.5 * UNITLENGTH;
            else
                y -= 0.5 * UNITLENGTH;
            (*vertices)[ringnr][count][0] = x;
            (*vertices)[ringnr][count][1] = y;
            count++;
        }

        for (int i=0; i < 2 * m; i++) {
            if (count % 2 == 0) {
                x += sqrt(3) * UNITLENGTH/2;
                y -= UNITLENGTH/2;
            } else {
                y -= UNITLENGTH;
            }
            (*vertices)[ringnr][count][0] = x;
            (*vertices)[ringnr][count][1] = y;
            count++;
        }
    }
    
    p = (*vertices)[0][count-1][0];
    q = (*vertices)[0][count-1][1];

    distance = sqrt(p*p + q*q);

    sind = fabs(q/distance);
    cosd = fabs(p/distance);

    multiplier = (2 * M_PI) / (p*cosd - q * sind);
    radius = (p*cosd - q * sind)/(2 * M_PI);

    for (int ringnr = 0; ringnr < t->length + 1; ringnr++) {
        for (int i=0; i < 2*(l+m); i++) {
            x = (*vertices)[ringnr][i][0];
            y = (*vertices)[ringnr][i][1];

            x2 = cosd*x - sind * y;
            y2 = sind*x + cosd * y;

            beta = multiplier * x2;

            x3 = radius * sin(beta);
            y3 = y2;
            z3 = -radius * cos(beta) + radius;

            (*vertices)[ringnr][i][0] = x3;
            (*vertices)[ringnr][i][1] = y3;
            (*vertices)[ringnr][i][2] = z3;
        }
    }
}

/*
ATTACH FUNCTIONS
*/

void applyRotation(float rotationmatrix[3][3], float source[3], float **dest) {
    float sum;
    for (int j=0; j < 3; j++) {
        sum = 0;
        for (int k=0; k < 3; k++) {
            sum += rotationmatrix[j][k] * source[k];
        }
        (*dest)[j] = sum;
    }
}

void rotateAndTranslateTube(tube *t, float (***ideal)[3], float rotationmatrix[3][3], float *translationvector) {
    int count;
    float *dest;
    int l, m, size;
    edge *e;

    l = t->parameters[0];
    m = t->parameters[1];

    size = 2*(l+m);

    dest = malloc(3*sizeof(float));

    for (int ringnr = 0; ringnr < t->length + 1; ringnr++) {
        for (int i = 0; i < size; i++) {
            applyRotation(rotationmatrix, (*ideal)[ringnr][i], &dest);
            for (int co=0; co < 3; co++) {
                (*ideal)[ringnr][i][co] = dest[co] - translationvector[co];
            }
        }
    }
}

void matrixmultiplication(float (*first)[3][3], float second[3][3], float ***result) {
    for (int i=0; i < 3; i++) {
        for (int j=0; j < 3; j++) {
            (*result)[i][j] = 0;
            for (int k=0; k < 3; k++) {
                (*result)[i][j] += (*first)[i][k] * second[k][j];
            }
        }
    }

    for (int i=0; i < 3; i++) {
        for (int j=0; j < 3; j++) {
            (*first)[i][j] = (*result)[i][j];
        }
    }
}

float fitness(int length, float (*ideal)[3], float (*real)[3], float rotationmatrix[3][3], float (*transformed)[3], float **translatevector) {
    float sum;
    float result;
    float totalsum[3];

    totalsum[0] = totalsum[1] = totalsum[2] = 0;
    for (int i=0; i < length; i++) {
        for (int j=0; j < 3; j++) {
            sum = 0;
            for (int k=0; k < 3; k++) {
                sum += rotationmatrix[j][k] * ideal[i][k];
            }
            transformed[i][j] = sum;
            totalsum[j] += sum - real[i][j];
        }
    }

    for (int i=0; i < 3; i++)
        totalsum[i] /= length;

    result = 0;
    for (int i=0; i < length; i++) {
        for (int j=0; j < 3; j++) {
            result += fabs(transformed[i][j] - real[i][j] - totalsum[j]);
        }
    }

    if (translatevector != NULL) {
        for (int i=0; i < 3; i++) {
            (*translatevector)[i] = totalsum[i];
        }
    }

    return result;
}

void rotationmatrices(float angles[3], float (*rotationmatrix)[3][3][3]) {
    for (int axis = 0; axis < 3; axis++) {
        for (int i=0; i < 3; i++) {
            (*rotationmatrix)[axis][i][i] = cos(angles[axis]);
        }
        for (int i=0; i < 3; i++) {
            (*rotationmatrix)[axis][axis][i] = (*rotationmatrix)[axis][i][axis] = 0;
        }
        (*rotationmatrix)[axis][axis][axis] = 1;
    }

    (*rotationmatrix)[0][1][2] = -sin(angles[0]);
    (*rotationmatrix)[1][2][0] = -sin(angles[1]);
    (*rotationmatrix)[2][0][1] = -sin(angles[2]);

    (*rotationmatrix)[0][2][1] = sin(angles[0]);
    (*rotationmatrix)[1][0][2] = sin(angles[1]);
    (*rotationmatrix)[2][1][0] = sin(angles[2]);   
}

void attachTube(graph *g, tube *t) {
    float (**ideal)[3];
    float (*real)[3];
    float (*transformed)[3];

    float currentrotationmatrix[3][3];
    float bestrotationmatrix[3][3];
    float *translatevector;


    float rotation10[3][3][3];
    float rotation1[3][3][3];
    float rotationneg1[3][3][3];

    float **result;
    float angles[3];
    float (*difvectors)[3];

    float bestfitness, f;
    int bestaxis, bestdirection;

    int l, m, count, length;
    edge *e, *start;

    l = t->parameters[0];
    m = t->parameters[1];

    transformed = malloc(2*(l+m)*sizeof(float[3]));
    result = malloc(3*sizeof(float*));
    for (int i=0; i < 3; i++) {
        result[i] = malloc(3*sizeof(float));
    }

    length = 2*(l+m);
    idealTube(t, &ideal);

    /* fill real */
    real = malloc(2*(l+m)*sizeof(float[3]));
    count = 0;
    e = t->firstedge;
    do {
        for (int i=0; i < 3; i++)
            real[count][i] = e->startvertex->co[i];
        if ( (count + 1) % length < 2 * l && count % 2 == 1) {
            e = e->inv->next;
        } else if ((count + 1) % length > 2 * l && count % 2 == 0) {
            e = e->inv->next;
        } else {
            e = e->inv->prev;
        }
        count++;
    } while (e != t->firstedge);

    /*
    general rotation matrices
    */
    
    angles[0] = angles[1] = angles[2] = M_PI/9;
    rotationmatrices(angles, &rotation10);
    angles[0] = angles[1] = angles[2] = M_PI/180;
    rotationmatrices(angles, &rotation1);
    angles[0] = angles[1] = angles[2] = -M_PI/180;
    rotationmatrices(angles, &rotationneg1);

    /* optimizing -> find approximation */
    for (int reflection = 1; reflection >= -1; reflection -= 2) {
                
        /* initial currentrotationmatrix */
        for (int i=0; i < 3; i++) {
            for (int j=0; j < 3; j++) {
                currentrotationmatrix[i][j] = 0;
            }
        }

        currentrotationmatrix[0][0] = reflection;
        currentrotationmatrix[1][1] = reflection;
        currentrotationmatrix[2][2] = reflection;

        bestfitness = fitness(length, ideal[0], real, currentrotationmatrix, transformed, NULL);

        for (int xrot=0; xrot < 360; xrot += 20) {
            matrixmultiplication(&currentrotationmatrix, rotation10[0], &result);
            for (int yrot=0; yrot < 360; yrot += 20) {
                matrixmultiplication(&currentrotationmatrix, rotation10[1], &result);
                for (int zrot=0; zrot < 360; zrot += 20) {
                    matrixmultiplication(&currentrotationmatrix, rotation10[2], &result);
                    f = fitness(length, ideal[0], real, currentrotationmatrix, transformed, NULL);
                    if (f < bestfitness) {
                        bestfitness = f;
                        for (int i=0; i < 3; i++) {
                            for (int j=0; j < 3; j++) {
                                bestrotationmatrix[i][j] = currentrotationmatrix[i][j];
                            }
                        }
                    }
                }
            }
        }
    }

    /* improve approximation */   
    for (int iteration=0; iteration < 50; iteration++) {
        bestdirection = 0;
        for (int axis=0; axis < 3; axis++) {

            matrixmultiplication(&bestrotationmatrix, rotation1[axis], &result);
            f = fitness(length, ideal[0], real, bestrotationmatrix, transformed, NULL);
            if (f < bestfitness) {
                bestfitness = f;
                bestaxis = axis;
                bestdirection = 1;
            }

            matrixmultiplication(&bestrotationmatrix, rotationneg1[axis], &result);


            matrixmultiplication(&bestrotationmatrix, rotationneg1[axis], &result);
            f = fitness(length, ideal[0], real, bestrotationmatrix, transformed, NULL);

            if (f < bestfitness) {
                bestfitness = f;
                bestaxis = axis;
                bestdirection = -1;
            }
            matrixmultiplication(&bestrotationmatrix, rotation1[axis], &result);

        }

        if (bestdirection == 1) {
            matrixmultiplication(&bestrotationmatrix, rotation1[bestaxis], &result);
        } else if (bestdirection == -1) {
            matrixmultiplication(&bestrotationmatrix, rotationneg1[bestaxis], &result);
        }
    }

    /* calculate translation vector */
    translatevector = malloc(3*sizeof(float));
    fitness(length, ideal[0], real, bestrotationmatrix, transformed, &translatevector);

    /* transoform ideal tube */
    rotateAndTranslateTube(t, &ideal, bestrotationmatrix, translatevector);

    /* set coordinates of tube vertices */
    
    /*difvectors = malloc(length*sizeof(float[3]));
    count = 0;
    e = t->firstedge;
    do {
        for (int i=0; i < 3; i++)
            difvectors[count][i] = e->startvertex->co[i] - ideal[0][count][i];
        if ( (count + 1) % length < 2 * l && count % 2 == 1) {
            e = e->inv->next;
        } else if ((count + 1) % length > 2 * l && count % 2 == 0) {
            e = e->inv->next;
        } else {
            e = e->inv->prev;
        }
        count++;
    } while (e != t->firstedge);*/

    e = t->firstedge->inv->next->inv->prev;
    for (int ringnr = 1; ringnr < t->length + 1; ringnr++) {
        count = 0;
        start = e;
        do {
            e->startvertex->embedded = 1;

            for (int i=0; i < 3; i++)
                e->startvertex->co[i] = ideal[ringnr][count][i];// + difvectors[count][i];

            if ( (count + 1) % length < 2 * l && count % 2 == 1) {
                e = e->inv->next;
            } else if ((count + 1) % length > 2 * l && count % 2 == 0) {
                e = e->inv->next;
            } else {
                e = e->inv->prev;
            }
            count++;
        } while (e != start);
        e = e->inv->next->inv->prev;
    }


}

void read_stdin(FILE* output) {
    char ch;
    while(read(STDIN_FILENO, &ch, 1) > 0)
    {
     fprintf(output, "%c", ch);
    }
    fclose(output);
}

int main( int argc, const char* argv[] )
{
    graph *g;
    float (**vertices)[3];
    int r;
    char* string = malloc(128*sizeof(char));
    char *fgetsr;

    fgetsr = fgets(string, 128, stdin);

    g = readNextGraphVega(stdin);

    while (!feof(stdin)) {
        findSeparationVertices(g);
        embedJoin(g);
        for (int i=0; i < g->nrofrings; i++) {
            attachTube(g, g->tubes[i]);
        }
        printw3d(g);
        free_graph(g);
        g = readNextGraphVega(stdin);
    }

    return 0;
}
/**
 *
 * NanotubeEmbedder.java by Simon Buelens
 *          on Aug 24, 2009
 */

package cage.embedder;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.Scanner;

/**
 *
 * @author Simon Buelens
 */
public class NanotubeEmbedder {

    private int [][] currentGraph;
    private double [][] graphCoords;
    private int [][] vertDepth;
    private int graphStartColumns;
    private int graphStartRows;
    private int nrVertices;
    // Number of vertices on 1 level
    private int length = 0;
    private int firstPentagon;
    // find the next vertex on the same level before or after the current
    private int next = 1;
    private IO io;
    // The distance between 2 C-atoms in graphite (in nm)
    private double l = 1.42;
    // distance between 2 vertices with common neighbour for a hexagon (x = 2 * l * sin(Pi/3));
    private double x = l * 1.73205081;
    // distance between 2 levels (from outer level orthogonal on x)
    private double a = l * 0.5;
    private final double epsilon = 1e-15;
    private boolean print;
    // boundary parameters
    private int phases;
    private double [] factors;
    private int b1;
    private int b2;


    public static void main (String [] args) {


        if (args.length == 1) {
            if (args[0].equals("--help"))
                usage();
        }

        Scanner sc = new Scanner(System.in);
        String input = "";
        while(sc.hasNext()) {
            input += sc.nextLine() + '\n';
        }

        NanotubeEmbedder embedder = new NanotubeEmbedder(input);

        int p;
        for (int i=0; i<args.length; i++) {
            if (args[i].equals("-p")) {
                if (args.length < i + 2)
                    usage();
                p = 3;
                try {
                    p = Integer.parseInt(args[i + 1]);
                } catch (NumberFormatException e) {
                    usage();
                }
                if (p < 0 || p > 4)
                    usage();
                embedder.setPhases(p);
            }
            else if (args[i].equals("-f")) {
                if (args.length < i + 2)
                    usage();
                String [] fs = args[i+1].split(",");
                int l = fs.length;
                if (l > 4)
                    usage();
                double [] factors = new double [l];
                for (int j=0; j<l; j++) {
                    try {
                        factors[j] = Double.parseDouble(fs[j]);
                    } catch(NumberFormatException e) {
                        usage();
                    }
                }
                embedder.setFactors(factors);
            }
        }
        embedder.startEmbedding();
    }

    /**
     * This class can pre-embed nanocones for CaGe. It will search the depth of the first pentagon itself.
     * @param input A string representing the graphs from CaGe
     */
    public NanotubeEmbedder(String input) {
        this.graphStartRows = 100;
        this.graphStartColumns = 3;
        this.io = new IO(input, graphStartRows, graphStartColumns);
        this.print = true;
        this.phases = 3;
        this.factors = new double [3];
        factors[0] = 0.05;
        factors[1] = 0.15;
        factors[2] = 0.25;
        // Set the factors for the different phases
    }

    public void startEmbedding() {
        while ((nrVertices = io.findNextGraph()) > 0) {
            this.currentGraph = io.getCurrentGraph();
            // Currently skip 2d
            if (io.getDimension() == 3) {
                this.graphCoords = io.getGraphCoords();
                this.calculateDepthsBFS();
                firstPentagon = this.findFirstPentagon();
                this.calculateBoundaryParameters();
                this.calculateEmbeddingForGraph();
            }
            if (print)
                io.printGraph(System.out);
        }
    }

    /**
     * Calculate the depths of all the vertices from <code>currentGraph</code>.
     * We start by searching the outer vertices and give them depth 0.
     * We continue our search to all vertices with BFS starting from the outer vertices.
     * We also keep track of the order we found the vertices in.
     * We fill in the global variable <code>vertDepth</code> as a matrix containing 2 arrays.
     * The first is the order we found the vertices in. The second are the corresponding
     * depths we found the vertices from the first array on.
     * TODO: fix this method. If b2 is 1, all the vertices starting from level 1 are shifted 1 too the right.
     * TODO: make better use of the fact that the neighbours are given IN ORDER.
     */
    private void calculateDepthsBFS () {
        int start = 0, current, previous, temp, index = 0;
        vertDepth = new int [2][nrVertices];
        int [] seen = new int [nrVertices];
        for (int i=0; i<nrVertices; i++) {
            // The neighbours are filled in order. So if the third neighbour
            // is not filled in, we have a vertex with degree 2.
            // note: there are no vertices with degree 1 in a nanotube.
            if (currentGraph[i][2] == 0) {
                start = i;
                break;
            }
        }

        // Init next: we look after or before the current vertex to find the next
        if (currentGraph[currentGraph[start][0] - 1][2] == 0) {
            // First neighbour is on same depth, we look at the next neighbour
            current = currentGraph[start][0] - 1;
            previous = start;
            if (currentGraph[currentGraph[start][1] - 1][0] == start + 1 && currentGraph[currentGraph[currentGraph[start][1] - 1][1] - 1][2] == 0)
                    next = -1;
            else if (currentGraph[currentGraph[start][1] - 1][1] == start + 1 && currentGraph[currentGraph[currentGraph[start][1] - 1][2] - 1][2] == 0)
                    next = -1;
            else if (currentGraph[currentGraph[currentGraph[start][1] - 1][0] - 1][2] == 0)
                    next = -1;
        }
        else {
            previous = currentGraph[start][0] - 1;
            if (currentGraph[currentGraph[start][0] - 1][0] == start + 1) {
                if (currentGraph[currentGraph[currentGraph[start][0] - 1][2] - 1][2] == 0) {
                    next = -1;
                    current = currentGraph[currentGraph[start][0] - 1][2] - 1;
                }
                else
                    current = currentGraph[currentGraph[start][0] - 1][1] - 1;
            }
            else if (currentGraph[currentGraph[start][0] - 1][1] == start + 1) {
                if (currentGraph[currentGraph[currentGraph[start][0] - 1][0] - 1][2] == 0) {
                    next = -1;
                    current = currentGraph[currentGraph[start][0] - 1][0] - 1;
                }
                else
                    current = currentGraph[currentGraph[start][0] - 1][2] - 1;
            }
            else if (currentGraph[currentGraph[currentGraph[start][0] - 1][1] - 1][2] == 0) {
                next = -1;
                current = currentGraph[currentGraph[start][0] - 1][1] - 1;
            }
            else
                current = currentGraph[currentGraph[start][0] - 1][0] - 1;
        }

        // Start by adding the outer vertices to the array
        vertDepth[0][index] = current;
        seen[current] = 1;
        vertDepth[1][index++] = 0;
        while (current != start) {
            temp = findPlaceInNeighbours(current, previous + 1) + next + 3;
            temp %= 3;
            previous = current;
            current = currentGraph[current][temp] == 0 ? (next == 1 ? currentGraph[current][0] - 1 : currentGraph[current][1] - 1) : currentGraph[current][temp] - 1;
            while (currentGraph[current][2] != 0) {
                temp = findPlaceInNeighbours(current, previous + 1) + next + 3;
                temp %= 3;
                previous = current;
                current = currentGraph[current][temp] - 1;
            }
            vertDepth[0][index] = current;
            seen[current] = 1;
            vertDepth[1][index++] = 0;
        }

        start = 0;
        int buffer = -1;
        while (start < nrVertices) {
            for (int i=2; i>=0; i--) {
                if (currentGraph[vertDepth[0][start]][i] != 0) {
                    current = currentGraph[vertDepth[0][start]][i] - 1;
                    if (seen[current] == 0) {
                        if (buffer != -1 && getLengthBetween(current, vertDepth[0][index - 1]) < 3) {
                            vertDepth[0][index] = buffer;
                            vertDepth[1][index++] = vertDepth[1][start] + 1;
                            seen[buffer] = 1;
                            buffer = -1;
                        }
                        if (vertDepth[1][start] + 1 == vertDepth[1][index - 1] && getLengthBetween(current, vertDepth[0][index - 1]) > 2) {
                            if (buffer != -1) {
                                temp = vertDepth[0][index - 2];
                                vertDepth[0][index - 2] = vertDepth[0][index - 1];
                                vertDepth[0][index - 1] = temp;
                                vertDepth[0][index] = buffer;
                                vertDepth[1][index++] = vertDepth[1][start] + 1;
                                seen[buffer] = 1;
                                buffer = -1;
                                i+=1;
                            }
                            else
                                buffer = current;
                        }
                        else {
                            vertDepth[0][index] = current;
                            vertDepth[1][index++] = vertDepth[1][start] + 1;
                            seen[current] = 1;
                        }
                    }
                }
            }
            start++;
        }

    }
    
    private int findPlaceInNeighbours(int vertex, int neighbour) {
        for (int i=0; i<3; i++) {
            if (currentGraph[vertex][i] == neighbour)
                return i;
        }
        return -1;
    }


    /**
     * Search the first pentagon in the current graph. Make sure <code>vertDepth</code> is initialised
     * by calling <code>calculateDepthsBFS()</code> first.
     * @return the number of the vertex containing the first pentagon in the order given by <code>vertDepth</code>.
     */
    private int findFirstPentagon() {
        int t1 = -1, t2 = 0, count;
        for (int i=0; i<nrVertices; i++) {
            for (int j=0; j<3; j++) {
                t2 = vertDepth[0][i];
                if (currentGraph[t2][j] == 0)
                    break;
                t1 = currentGraph[t2][j] - 1;
                count = 1;
                do {
                    for (int k=0; k<3; k++) {
                        if (currentGraph[t1][k] == t2 + 1) {
                            t2 = t1;
                            t1 = currentGraph[t2][(k+2)%3] - 1;
                            if (t1 == -1)
                                t1 = currentGraph[t2][1] - 1;
                            break;
                        }
                    }
                    count++;
                } while (t1 != vertDepth[0][i] && count < 6);
                if (count == 5)
                    return i;
            }
        }
        return -1;
    }

    private void calculateEmbeddingForGraph() {


        /**************     Embedding Phase 1 : Pre-Embedding      *************/

        int temp, start = 0, offset = 0, i=0, begin = 0;
        double prevX = -x, prevZ = 0, beginX = 0, beginZ = 0;
        boolean horizontal = true;


        /**
         * We first calculate how we would place the hexagon layers in 2D
         **/
        while (vertDepth[1][i] == 0) {
            temp = (i + 1 >= nrVertices || vertDepth[1][i + 1] != 0) ? vertDepth[0][0] : vertDepth[0][i+1];
            if (areNeighbours(vertDepth[0][i], temp + 1))
                start = i;
            i++;
            length = i;
        }

        i = 0;
        while (vertDepth[1][i + 1] == 0) {
            if (areNeighbours(vertDepth[0][i], vertDepth[0][i + 1] + 1)) {
                horizontal = true;
                beginX = -x;
                beginZ = 0;
                break;
            }
            else if (!haveCommonNeighbour(vertDepth[0][i], vertDepth[0][i + 1])) {
                horizontal = false;
                beginX = - x / 2;
                beginZ = l + a;
                break;
            }
            i++;
        }

        i = 0;
        if (areNeighbours(vertDepth[0][0], vertDepth[0][length - 1] + 1)) {
            i++;
            offset = -1;
            begin = 1;
        }

        if (!haveCommonNeighbour(vertDepth[0][0], vertDepth[0][length - 1] + 1)) {
            beginX = - x;
            beginZ = l;
        }

        while (vertDepth[1][i] == 0) {
            temp = (i + 1 >= nrVertices || vertDepth[1][i + 1] != 0) ? vertDepth[0][0] : vertDepth[0][i+1];

            if (horizontal) {
                prevX += x;
            }
            else {
                prevX += x / 2;
                prevZ -= l + a;
            }
            graphCoords[vertDepth[0][i]][0] = prevX;
            graphCoords[vertDepth[0][i]][2] = prevZ;
            graphCoords[vertDepth[0][i + length + offset]][0] = prevX - x/2;
            graphCoords[vertDepth[0][i + length + offset]][2] = prevZ - (horizontal ? a : -a);

            if (areNeighbours(vertDepth[0][i], temp + 1)) {
                horizontal = false;
                prevZ -= a;
                prevX += x/2;
                graphCoords[temp][0] = prevX;
                graphCoords[temp][2] = prevZ;
                i++;
                offset -= 1;
            }
            else if (!haveCommonNeighbour(vertDepth[0][i], temp)) {
                horizontal = true;
                prevZ -= l;
                offset += 1;
                graphCoords[vertDepth[0][i + length + offset]][0] = prevX;
                graphCoords[vertDepth[0][i + length + offset]][2] = prevZ;
            }
            i++;
        }

        temp = 0;
        for (i = 2; i <= vertDepth[1][firstPentagon]; i += 2) {
            for (int j = 0; j < length; j++) {
                temp = i * length + j;
                graphCoords[vertDepth[0][temp]][0] = graphCoords[vertDepth[0][temp - 2 * length]][0] - x / 2;
                graphCoords[vertDepth[0][temp]][2] = graphCoords[vertDepth[0][temp - 2 * length]][2] - l - a;
                graphCoords[vertDepth[0][temp + length]][0] = graphCoords[vertDepth[0][temp - length]][0] - x / 2;
                graphCoords[vertDepth[0][temp + length]][2] = graphCoords[vertDepth[0][temp - length]][2] - l - a;
            }
        }

        i = 0;
        while (vertDepth[1][i] <= vertDepth[1][firstPentagon] + 1)
            i++;
        int [] amount = new int [vertDepth[1][nrVertices - 1] + 1];
        while (i < nrVertices) {
            amount[vertDepth[1][i]]++;
            i++;
        }

        
        /**** ROTATE AXES ****/

        double distance = Math.sqrt(Math.pow(b2 * x / 2 + b1 * x, 2)
                + Math.pow(b2 * (a + l), 2));
        double distOnX = b2 * x / 2 + b1 * x;

        double angle = Math.acos(distOnX / distance);
        double aCos = Math.cos(angle);
        double aSin = Math.sin(angle);
        double xPrev;

        for (i = 0; i < nrVertices; i ++) {
                temp = vertDepth[0][i];
                xPrev = graphCoords[temp][0];
                graphCoords[temp][0] = xPrev * aCos - graphCoords[temp][2] * aSin;
                graphCoords[temp][2] = xPrev * aSin + graphCoords[temp][2] * aCos;
        }



        /**** Place the top ****/

        
        i = 0;
        while (vertDepth[1][i] <= vertDepth[1][firstPentagon])
            i++;
        prevZ = graphCoords[vertDepth[0][i]][2];
        while (vertDepth[1][i] <= vertDepth[1][firstPentagon] + 1) {
            if (graphCoords[vertDepth[0][i]][2] < prevZ)
                prevZ = graphCoords[vertDepth[0][i]][2];
            i++;
        }

        beginX = graphCoords[vertDepth[0][2*length]][0] - graphCoords[vertDepth[0][0]][0];
        while (i < nrVertices) {
            start = i;
            distOnX = distance / amount[vertDepth[1][start]];
            prevZ -= l;
            prevX = beginX   * (vertDepth[1][start] / 2);
            while (i < nrVertices && vertDepth[1][start] == vertDepth[1][i]) {
                graphCoords[vertDepth[0][i]][0] = distOnX * (i - start) + prevX;
                graphCoords[vertDepth[0][i]][2] = prevZ;
                i++;
            }
        }

        /**** 2D -> 3D ****/

        i = 0;
        double radius = distance / (2.0*Math.PI);
        beginX = distance;
        while (i < nrVertices) {
            angle = graphCoords[vertDepth[0][i]][0] * Math.PI * 2 / distance;
            graphCoords[vertDepth[0][i]][0] = radius * Math.cos(angle);
            graphCoords[vertDepth[0][i]][1] = radius * Math.sin(angle);
            i++;
        }

        /**************             Embedding Phase 2              *************/

        /**
         * In this second phase, we will begin with trying to get the levels containing pentagons
         * in a better embedding.
         */


        Random r = new Random();
        int maxStep;
        int vertex;
        double t;

        i = 0;
        while (vertDepth[1][i] < vertDepth[1][firstPentagon])
            i++;
        start = i;
        begin = 0;
        i = start;
        beginX = l * 1.2;

        /*
         Some edges are way too long, especially at the start from the cap.
         We try to fix this locally.
         **/
        while (i < nrVertices) {
            //code in comments is Java 5, when switching to this version these lines should be used (nvcleemp)
            //for (int n : currentGraph[vertDepth[0][i]]) {
            for (int j=0; j < currentGraph[vertDepth[0][i]].length; j++) {
                int n = currentGraph[vertDepth[0][i]][j];
                if (n != 0 && getDistance(vertDepth[0][i], n - 1) > x) {
                    moveTowards(vertDepth[0][i], n - 1, beginX);
                }
            }
            i++;
        }
        i = start;
        /*
         * The edges on the top levels may still be too long. Move those
         * vertices towards eachother.
         */
        while (i < nrVertices) {
            //code in comments is Java 5, when switching to this version these lines should be used (nvcleemp)
            //for (int n : currentGraph[vertDepth[0][i]]) {
            for (int j=0; j < currentGraph[vertDepth[0][i]].length; j++) {
                int n = currentGraph[vertDepth[0][i]][j];
                if (n != 0 && getDistance(vertDepth[0][i], n - 1) > x) {
                    moveTowardsEachother(vertDepth[0][i], n - 1, l);
                }
            }
            i++;
        }

        /*
         * Start with a advanced kind of local search.
         * The factor for which the neighbours's neighbours count is still small.
         */
        t = 0.1;
        maxStep = (nrVertices - firstPentagon)*50;
        for (int step=0; step<maxStep; step++) {
            vertex = firstPentagon + (step % (nrVertices - firstPentagon));
            if (step % (nrVertices - firstPentagon) == 0 ) {
                t = (1.0 - step/maxStep);
                t = 0.5 * t * t;
            }
            findLocalOptimum(vertDepth[0][vertex], t, factors[0], 0, false);
        }

        maxStep = (nrVertices - firstPentagon)*150;
        for (int step=0; step<maxStep; step++) {
            vertex = firstPentagon + (step % (nrVertices - firstPentagon));
            if (step % (nrVertices - firstPentagon) == 0 ) {
                t = (1.0 - step/maxStep);
                t = 0.1 * t;
            }
            findLocalOptimum(vertDepth[0][vertex], t, factors[1], 0.1, false);
        }
        

        /**************             Embedding Phase 3              *************/

        /**
         * In this last phase, we rearrange all the vertices to become a local minimum.
         */

        maxStep = (nrVertices)*250;
        for (int step=0; step<maxStep; step++) {
            vertex = step % nrVertices;
            t = (1.0 - step/maxStep);
            t = 0.1 * t * t * t;
            findLocalOptimum(vertDepth[0][vertex], t, factors[2], 0, false);
        }

    }

    /**
     * Try to find a local optimum for the current <code>vertex</code>.
     * We will do this by the use of forces, applied from it's neighbours and the neighbours of it's neighbours.
     * We know the distance they should be at, so if they are farther away, they will pull the
     * vertex towards them, if they are closer by, they will try to push the vertex away. If they are at the exact
     * distance they should be at, they will do nothing at all.
     * The parameter t can be used to make the length of the movement shorter
     * (eg. use this to decrease the influence of the forces by time).
     * The last parameter can be used to stress the fact that the graph should be convex.
     * This method will then add an additional force performed from a point in the middle.
     * @param vertex the vertex we want to find a local optimum for
     * @param t the percentage of the distance of the force vector we should actually move the vertex by.
     * @param factor this is the factor the neighbours's neighbours count for
     * @param forceConvex use this to stress the fact that you want the graph to be convex.
     */
    private void findLocalOptimum(int vertex, double t, double factor, double forceConvex, boolean quadratic) {
        double x = 0.0, y = 0.0, z = 0.0, d;

        // Calculate forces applied to the vertex by it's neighbours and it's neighbours' neighbours.

        //code in comments is Java 5, when switching to this version these lines should be used (nvcleemp)
        //for (int n : currentGraph[vertex]) {
        for (int i = 0; i < currentGraph[vertex].length; i++) {
            int n = currentGraph[vertex][i];
            if (n != 0) {
                d = getDistance(vertex, n - 1);
                if (d > epsilon) {
                    if (!quadratic)
                        d = (l - d) / d;
                    else if (l - d < 0)
                        d = - Math.pow(l - d, 2) / d;
                    else
                        d = Math.pow(l - d, 2) / d;
                    x += d*(graphCoords[vertex][0] - graphCoords[n - 1][0]);
                    y += d*(graphCoords[vertex][1] - graphCoords[n - 1][1]);
                    z += d*(graphCoords[vertex][2] - graphCoords[n - 1][2]);
                    //code in comments is Java 5, when switching to this version these lines should be used (nvcleemp)
                    //for (int nn : currentGraph[n-1]) {
                    for (int j = 0; j < currentGraph[n-1].length; j++) {
                        int nn = currentGraph[n-1][j];
                        if (nn != 0 && nn - 1 != n) {
                            d = getDistance(vertex, nn - 1);
                            if (d > epsilon) {
                                if (!quadratic)
                                    d = (this.x - d) / d;
                                else if (this.x - d < 0)
                                    d = - Math.pow(this.x - d, 2) / d;
                                else
                                    d = Math.pow(this.x - d, 2) / d;
                                x += factor*d*(graphCoords[vertex][0] - graphCoords[n - 1][0]);
                                y += factor*d*(graphCoords[vertex][1] - graphCoords[n - 1][1]);
                                z += factor*d*(graphCoords[vertex][2] - graphCoords[n - 1][2]);
                            }
                        }
                    }
                }
            }
        }

        // Add a force from the inside of the graph.
        if (forceConvex > 0) {
            d = Math.sqrt(Math.pow(graphCoords[vertex][0], 2) + Math.pow(graphCoords[vertex][1], 2));
            d *= (1 + forceConvex);
            x *= d;
            y *= d;
        }

        graphCoords[vertex][0] += t*x;
        graphCoords[vertex][1] += t*y;
        graphCoords[vertex][2] += t*z;
    }
    
    /**
     * Calculate the distance between two given vertices
     * @param from the first vertex
     * @param to the second vertex
     * @return the distance between the two vertices
     */
    private double getDistance(int from, int to) {
        double d = 0.0, temp;
        for (int i=0; i<3; i++) {
            temp = graphCoords[from][i] - graphCoords[to][i];
            d += temp * temp;
        }
        return Math.sqrt(d);
    }

    /**
     * Calculate the boundary parameters which define the nanotube.
     * They will be stored in b1 and b2, b1 >= b2.
     */
    private void calculateBoundaryParameters() {
        int i = 0, temp = 0, start = -1;
        while (vertDepth[1][i] == 0) {
            temp = (i + 1 >= nrVertices || vertDepth[1][i + 1] != 0) ? vertDepth[0][0] : vertDepth[0][i+1];
            if (areNeighbours(vertDepth[0][i], temp + 1))
                start = i;
            i++;
        }
        int length = i;
        i = start;
        if (start == -1) {
            b1 = length;
            b2 = 0;
        }
        else {
            if (vertDepth[1][i + 1] == 1)
                i = 0;
            else
                i++;
            temp = (i + 1 >= nrVertices || vertDepth[1][i + 1] != 0) ? vertDepth[0][0] : vertDepth[0][i+1];
            while (haveCommonNeighbour(vertDepth[0][i], temp)) {
                if (vertDepth[1][i + 1] == 1)
                    i = 0;
                else
                    i++;
                temp = (i + 1 >= nrVertices || vertDepth[1][i + 1] != 0) ? vertDepth[0][0] : vertDepth[0][i+1];
            }
            if (start > i) {
                // We somewhere reached the end
                b2 = length - start + i;
            }
            else
                b2 = i - start + 1;
            start = i;
            while (!areNeighbours(vertDepth[0][i], temp + 1)) {
                if (vertDepth[1][i + 1] == 1)
                    i = 0;
                else
                    i++;
                temp = (i + 1 >= nrVertices || vertDepth[1][i + 1] != 0) ? vertDepth[0][0] : vertDepth[0][i+1];
            }
            if (start > i) {
                // We somewhere reached the end
                b1 = length - start + i;
            }
            else
                b1 = i - start;
        }
        if (b1 < b2) {
            temp = b2;
            b2 = b1;
            b1 = b2;
        }
    }

    /**
     * Check wether the given vertices are neighbours in the current graph
     */
    private boolean areNeighbours(int vertex, int neighbour) {
        //code in comments is Java 5, when switching to this version these lines should be used (nvcleemp)
        //for (int n : currentGraph[vertex])
        //    if (n == neighbour)
        //        return true;
        for (int i = 0; i < currentGraph[vertex].length; i++)
            if (currentGraph[vertex][i] == neighbour)
                return true;
        return false;
    }

    /**
     * Check wether the given vertices have a common neighbour in the current graph
     */
    private boolean haveCommonNeighbour(int v1, int v2) {
        //code in comments is Java 5, when switching to this version these lines should be used (nvcleemp)
        //for (int n : currentGraph[v1]) {
        //    for (int j : currentGraph[v2])
        //        if (n != 0 && n == j)
        //            return true;
        //}
        for (int i = 0; i < currentGraph[v1].length; i++) {
            for (int j = 0; j < currentGraph[v2].length; j++)
                if (currentGraph[v1][i] != 0 && currentGraph[v1][i] == currentGraph[v2][j])
                    return true;
        }
        return false;
    }

    private static void usage() {
        String output = "usage: PreEmbedderNanocones [options] <input >output\n" +
                "\n" +
                "The input must be in writegraph format as specified by the VEGA project\n\n" +
                "Options: (n stands for integer value, d for double value)\n" +
                "   --help      Will show this information.\n" +
                "   -p n        the amount of phases to be executed. Must be between 0 and 3.\n" +
                "               Phase 1 is the pre-embedding. All vertices are divided over circles \n" +
                "               in different layers. In phase 2 the top (the levels containing pentagons)\n" +
                "               are positioned better with a kind of local search.\n" +
                "               In the 3rd phase the local search is executed over the whole graph, \n" +
                "               repositioning vertices just that little bit better.\n" +
                "   -f d[,d[,d]]\n" +
                "               The factors for which the neighbours's neighbours count for in the different\n" +
                "               phases. You can specify upto 3 different factors.\n" +
                "               If you specify less then 3, the phases that follow will use the same factor\n" +
                "               as the last one specified. They should be doubles (x.xxx), seperated by comma's.\n" +
                "\n" +
                "The default is:    -p 3 -f 0.05,0.15,0.25";
        System.err.println(output);
        System.exit(1);
    }

    /**
     * The the amount of vertices on the shortest path between the two given vertices v1 and v2.
     * This method is very time-consuming and should therefor be avoided.
     * @param v1 the first vertex
     * @param v2 the second vertex
     * @return the amount of vertices between the two given vertices
     */
    private int getLengthBetween(int v1, int v2) {
        //code in comments is Java 5, when switching to this version these lines should be used (nvcleemp)

        //List<Integer> list = new ArrayList<Integer>();
        List list = new ArrayList();
        int [] notSeen = new int [nrVertices];
        int [] length = new int [nrVertices];
        //for (int n : currentGraph[v1]) {
        for (int j = 0; j < currentGraph[v1].length; j++) {
            int n = currentGraph[v1][j];
            if (n == 0)
                continue;
            if (n - 1 == v2)
                return 0;
            //list.add(n-1);
            list.add(Integer.valueOf(n - 1));
        }
        notSeen[v1] = 1;
        int i = 0;
        while (i < list.size()) {
            //for (int n : currentGraph[list.get(i)]) {
            for (int j = 0; j < currentGraph[((Integer)list.get(i)).intValue()].length; j++) {
                int n = currentGraph[((Integer)list.get(i)).intValue()][j];
                if (n == 0)
                    continue;
                if (n - 1 == v2)
                    //return length[list.get(i)] + 1;
                    return length[((Integer)list.get(i)).intValue()] + 1;
                if (notSeen[n - 1] == 0) {
                    //list.add(n - 1);
                    list.add(Integer.valueOf(n - 1));
                    //length[n - 1] = length[list.get(i)] + 1;
                    length[n - 1] = length[((Integer)list.get(i)).intValue()] + 1;
                }
            }
            //notSeen[list.get(i)] = 1;
            notSeen[((Integer)list.get(i)).intValue()] = 1;
            i++;
        }
        return -1;
    }

    /**
     * Move the second vertex (v2) towards the first vertex (v1) on the vector between these two.
     * The distance after the movement should be d.
     * @param v1 the vector to move to
     * @param v2 the vector to be moved
     * @param d the distance between the two vertices after the movement
     */
    private void moveTowards(int v1, int v2, double d) {
        double distance = getDistance(v1, v2);
        distance = (-d) / distance;
        for (int i=0; i<3; i++)
            graphCoords[v2][i] = graphCoords[v1][i] + (graphCoords[v1][i] - graphCoords[v2][i]) * distance;
    }

    /**
     * Move the second vertex towards the first on the vector between these two. The movement is
     * proportional to the distance between the two.
     * @param v1 the vertex to move to
     * @param v2 the vertex to be moved
     * @param d
     */
    private void moveTowardsEachother(int v1, int v2, double d) {
        double distance = getDistance(v1, v2);
        distance = (d - distance) / distance;
        for (int i=0; i<3; i++)
            graphCoords[v2][i] = graphCoords[v1][i] + (graphCoords[v1][i] - graphCoords[v2][i]) * distance;
    }

    private void setPhases(int p) {
        this.phases = p;
    }

    //code in comments is Java 5, when switching to this version these lines should be used (nvcleemp)
    //private void setFactors(double ... factors) {
    private void setFactors(double[] factors) {
        for (int i=0; i<factors.length; i++)
            this.factors[i] = factors[i];
        if (factors.length < this.factors.length)
            for (int i=factors.length; i<this.factors.length; i++)
                this.factors[i] = factors[factors.length - 1];
    }

}

/**
 *
 * NanoconeEmbedder.java by Simon Buelens
 *          on Aug 5, 2009
 */

package cage.embedder;

import java.util.Random;
import java.util.Scanner;

/**
 *
 * @author Simon Buelens
 */
public class NanoconeEmbedder {

    private int [][] currentGraph;
    private double [][] graphCoords;
    private int [][] vertDepth;
    private int graphStartColumns;
    private int graphStartRows;
    private int nrVertices;
    private int firstPentagon;
    private int originalNrVertices = 0;
    // find the next vertex on the same level before or after the current
    private int next = 1;
    private IO io;
    // The distance between 2 C-atoms in graphite (in nm)
    private double l = 1.42;
    // distance between 2 vertices with common neighbour for a hexagon (x = 2 * l * sin(Pi/3));
    private double vertexDistanceInHexagon = l * 1.73205081;
    // distance between 2 levels (from outer level orthogonal on x)
    private double a = l * 0.5;
    private double zeroZ;
    private final double epsilon = 1e-15;
    private double [] factors = new double [4];
    private int [] steps = new int [3];
    private int [] show = new int [3];
    // The amount of phases (out of 4) to show
    private int phases;
    private boolean print;
    private int minLayers;
    private boolean solo = false;

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {

        if (args.length == 1) {
            if (args[0].equals("--help"))
                usage();
        }

        Scanner sc = new Scanner(System.in);
        String input = "";
        while(sc.hasNext()) {
            input += sc.nextLine() + '\n';
        }

        NanoconeEmbedder embedder = new NanoconeEmbedder(input);

        int p, print, layers;
        for (int i=0; i<args.length; i++) {
            if (args[i].equals("-solo"))
                embedder.setSolo(true);
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
            else if (args[i].equals("-s")) {
                if (args.length < i + 2)
                    usage();
                String [] s = args[i+1].split(",");
                int l = s.length;
                if (l > 3)
                    usage();
                int [] steps = new int [l];
                for (int j=0; j<l; j++) {
                    try {
                        steps[j] = Integer.parseInt(s[j]);
                    } catch(NumberFormatException e) {
                        usage();
                    }
                }
                embedder.setMaxSteps(steps);
            }
            else if (args[i].equals("-sp")) {
                if (args.length < i + 2)
                    usage();
                String [] s = args[i+1].split(",");
                int l = s.length;
                if (l > 3)
                    usage();
                int [] show = new int [l];
                for (int j=0; j<l; j++) {
                    try {
                        show[j] = Integer.parseInt(s[j]);
                    } catch(NumberFormatException e) {
                        usage();
                    }
                }
                embedder.setShowPerPhase(show);
            }
            else if (args[i].equals("-print")) {
                if (args.length < i + 2)
                    usage();
                print = 1;
                try {
                    print = Integer.parseInt(args[i + 1]);
                } catch (NumberFormatException e) {
                    usage();
                }
                if (print == 1)
                    embedder.setPrint(true);
                else if (print == 0)
                    embedder.setPrint(false);
                else
                    usage();
            }
            else if (args[i].equals("-l")) {
                double l = 0.0;
                if (args.length < i + 2)
                    usage();
                try {
                    l = Double.parseDouble(args[i + 1]);
                } catch (NumberFormatException e) {
                    usage();
                }
                if (l == 0.0) {
                    usage();
                }
                embedder.setLengthBetweenAtoms(l);
            }
            else if (args[i].equals("-layers")) {
                if (args.length < i + 2)
                    usage();
                layers = 3;
                try {
                    layers = Integer.parseInt(args[i + 1]);
                } catch (NumberFormatException e) {
                    usage();
                }
                if (layers < 0)
                    usage();
                embedder.setMinimumLayers(layers);
            }
        }
        embedder.startEmbedding();
    }
    
    /**
     * This class can pre-embed nanocones for CaGe. It will search the depth of the first pentagon itself.
     * @param input A string representing the graphs from CaGe
     */
    public NanoconeEmbedder(String input) {
        this.graphStartRows = 100;
        this.graphStartColumns = 3;
        this.io = new IO(input, graphStartRows, graphStartColumns);
        this.phases = 4;
        this.print = true;
        this.minLayers = 3;
        this.print = true;
        // Set the factors for the different phases
        factors[0] = 0.25;
        factors[1] = 0.15;
        factors[2] = 0.22;
        factors[3] = 0.35;
        steps[0] = 50;
        steps[1] = 500;
        steps[2] = 500;
        show[0] = 0;
        show[1] = 0;
        show[2] = 0;
    }

    public void startEmbedding() {
        while ((nrVertices = io.findNextGraph()) > 0) {
            this.currentGraph = io.getCurrentGraph();
            // Currently skip 2d
            if (io.getDimension() == 3) {
                this.graphCoords = io.getGraphCoords();
                this.calculateDepthsBFS();
                firstPentagon = this.findFirstPentagon();
                if (vertDepth[1][firstPentagon] < this.minLayers * 2)
                    addLayersOfHexagons((this.minLayers * 2-vertDepth[1][firstPentagon])/2);
                firstPentagon = this.findFirstPentagon();
                this.calculateEmbeddingForGraph();
                if (originalNrVertices != 0 && originalNrVertices < nrVertices) {
                    removeAddedLayers();
                }
            }
            if (print)
                io.printGraph(System.out);
            if (solo)
                return;
        }
    }

    private void removeAddedLayers() {
        for (int i=0; i<originalNrVertices; i++) {
            for (int j=0; j<3; j++) {
                if (currentGraph[i][j] > originalNrVertices) {
                    if (j != 2)
                        currentGraph[i][j] = currentGraph[i][2];
                    currentGraph[i][2] = 0;
                }
            }
        }
        // Make sure the printing of the graph doesn't read the added vertices.
        currentGraph[originalNrVertices][0] = 0;
        currentGraph[originalNrVertices][1] = 0;
        currentGraph[originalNrVertices][2] = 0;
    }

    private void addLayersOfHexagons(int layers) {
        int start, current, startNrVertices;
        this.originalNrVertices = nrVertices;
        while (layers-- > 0) {
            // find first point
            start = 0;
            while (vertDepth[1][start + 1] == 0)
                start++;
            // Current vertex;
            current = 0;
            startNrVertices = nrVertices;
            addVerticesToGraph(1, vertDepth[0][start]);
            if (this.areNeighbours(vertDepth[0][start], vertDepth[0][current] + 1)) {
                // Add 4 points
                addVerticesToGraph(3, vertDepth[0][current]);
            }
            else if (this.haveCommonNeighbour(vertDepth[0][start], vertDepth[0][current])) {
                // Add 3 points
                addVerticesToGraph(2, vertDepth[0][current]);
            }
            else
                // Something odd happened.
                throw new RuntimeException("Given graph isn't a nanocone.");
            while (current < start - 1) {
                if (this.areNeighbours(vertDepth[0][current], vertDepth[0][current + 1] + 1)) {
                    // Add 4 points
                    addVerticesToGraph(3, vertDepth[0][current + 1]);
                }
                else if (this.haveCommonNeighbour(vertDepth[0][current], vertDepth[0][current + 1])) {
                    // Add 3 points
                    addVerticesToGraph(2, vertDepth[0][current + 1]);
                }
                else
                    // Something odd happened.
                    throw new RuntimeException("Given graph isn't a nanocone.");
                current++;
            }

            // We still have to close the gap between the last and the first point added
            if (this.areNeighbours(vertDepth[0][current], vertDepth[0][start] + 1)) {
                // Add 2 points
                currentGraph[nrVertices - 1][2] = nrVertices + 1;
                currentGraph[nrVertices][next == 1 ? 1 : 0] = nrVertices;
                currentGraph[nrVertices][next == 1 ? 0 : 1] = nrVertices + 2;
                currentGraph[nrVertices + 1][next == 1 ? 1 : 0] = nrVertices + 1;
                currentGraph[nrVertices + 1][next == 1 ? 0 : 1] = startNrVertices + 1;
                currentGraph[startNrVertices][1] = nrVertices + 2;
                nrVertices += 2;
            }
            else if (this.haveCommonNeighbour(vertDepth[0][current], vertDepth[0][start])) {
                // Add 1 points
                currentGraph[nrVertices - 1][2] = nrVertices + 1;
                currentGraph[nrVertices][next == 1 ? 1 : 0] = nrVertices;
                currentGraph[nrVertices][next == 1 ? 0 : 1] = startNrVertices + 1;
                currentGraph[startNrVertices][1] = nrVertices + 1;
                nrVertices++;
            }
            else
                // Something odd happened.
                throw new RuntimeException("Given graph isn't a nanocone.");
            this.calculateDepthsBFS();
        }
    }

    private void addVerticesToGraph(int amount, int vertexToConnect) {
        if (nrVertices + amount > currentGraph.length) {
            doubleCurrentGraph(currentGraph);
            doubleGraphCoords(graphCoords);
        }

        // Hexagons with degree 2
        for (int i=0; i<amount - 1; i++) {
            currentGraph[nrVertices + i] = new int [3];
            currentGraph[nrVertices + i][0] = nrVertices + 1 + i + next;
            currentGraph[nrVertices + i][1] = nrVertices + 1 + i - next;
        }

        // The last hexagon should be connected with vertexToConnect
        currentGraph[nrVertices + amount - 1] = new int [3];
        currentGraph[nrVertices + amount - 1][next == 1 ? 0 : 1] = vertexToConnect + 1;
        // amount == 1 is a special case with just 1 edge.
        if (amount > 1) {
            // The first hexagon is a neighbour of the hexagon we last added the previous time
            currentGraph[nrVertices - 1][2] = nrVertices + 1;
            currentGraph[nrVertices + amount - 1][next == 1 ? 1 : 0] = nrVertices + amount - 1;
        }

        // Update neighbours for vertexToConnect
        if (next == -1) {
            currentGraph[vertexToConnect][2] = currentGraph[vertexToConnect][1] + 1;
            currentGraph[vertexToConnect][1] = nrVertices + amount;
        }
        else
            currentGraph[vertexToConnect][2] = nrVertices + amount;
        
        nrVertices += amount;
    }

    private void doubleCurrentGraph(int[][] matrix) {
        currentGraph = new int [matrix.length*2][this.graphStartColumns];
        System.arraycopy(matrix, 0, currentGraph, 0, matrix.length);
    }

    private void doubleGraphCoords(double[][] matrix) {
        graphCoords = new double [matrix.length*2][3];
        System.arraycopy(matrix, 0, graphCoords, 0, matrix.length);
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

    /**
     * Check wether the given vertices are neighbours in the current graph
     */
    private boolean areNeighbours(int vertex, int neighbour) {
        //code in comments is Java 5, when switching to this version these lines should be used (nvcleemp)

        //for (int n : currentGraph[vertex])
        //    if (n == neighbour)
        //        return true;
        for (int i = 0; i < currentGraph[vertex].length; i++){
            int n = currentGraph[vertex][i];
            if (n == neighbour)
                return true;
        }
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
        for(int i=0; i<currentGraph[v1].length; i++){
            for (int j = 0; j < currentGraph[v2].length; j++) {
                if (currentGraph[v1][i] != 0 && currentGraph[v1][i] == currentGraph[v2][j])
                    return true;
            }
        }
        return false;
    }


    /**
     * The actual calculation of the embedding of graph.
     * This will be done in several phases.
     * More comment can be read underneath the announcement of each phase.
     */
    private void calculateEmbeddingForGraph() {

        /**************     Embedding Phase 1 : Pre-Embedding      *************/

        if (phases > 0)
        this.calculateEmbeddingPhase1();

        

        /**************             Embedding Phase 2              *************/

        /**
         * In this second phase, we will begin with trying to get the levels containing pentagons
         * in a better embedding.
         */


        if (phases == 1) {
            return;
        }
        
        Random r = new Random();
        int maxStep;
        int vertex;
        double t;


        maxStep = (nrVertices - firstPentagon)*this.steps[0];
        for (int step=0; step<maxStep; step++) {
            vertex = r.nextInt(nrVertices - firstPentagon) + firstPentagon;
            t = (1.0 - step/maxStep);
            t = 0.1 * t * t * t;
            findLocalOptimum(vertDepth[0][vertex], t, factors[0], 0.2, true);
            if (this.show[0] != 0 && (step + 1)%(maxStep/this.show[0]) == 0)
                io.printGraph(System.out);
        }

        // Blow the top with the pentagons a bit up, to get it more convex.
        for (int step=0; step<nrVertices - firstPentagon; step++) {
            findLocalOptimum(vertDepth[0][firstPentagon + step], 0.2, factors[1], 10, true);
        }
        if (this.show[0] > 0)io.printGraph(System.out);


        /**************             Embedding Phase 3              *************/

        /**
         * Final phase.
         * This will finalize the whole graph. The whole graph will be changed just a bit,
         * as the levels with the hexagons were already pretty good from start and the pentagons
         * were
         */

        if (phases == 2)
            return;

        maxStep = nrVertices*this.steps[1];
        for (int step=0; step<maxStep; step++) {
            vertex = step % nrVertices;
            t = (1.0 - step/maxStep);
            t = 0.05 * t * t * t;
            findLocalOptimum(vertex, t, factors[2], 0, true);
            if (this.show[1] != 0  && (step + 1)%(maxStep/this.show[1]) == 0)
                io.printGraph(System.out);
        }


        /**************             Embedding Phase 4              *************/

        /**
         * Final phase.
         * This will finalize the whole graph. The whole graph will be changed just a bit,
         * as the levels with the hexagons were already pretty good from start and the pentagons
         * were
         */

        if (phases == 3)
            return;
        
        maxStep = (nrVertices - firstPentagon)*this.steps[2];
        for (int step=0; step<maxStep; step++) {
//            vertex = step%(nrVertices - firstPentagon) + firstPentagon;
            vertex = r.nextInt(nrVertices - firstPentagon) + firstPentagon;
            t = (1.0 - step/maxStep);
            t = 0.1 * t * t * t;
            findLocalOptimum(vertDepth[0][vertex], t, factors[3], 0, true);
            if (this.show[2] != 0  && (step + 1)%(maxStep/this.show[2]) == 0)
                io.printGraph(System.out);
        }

    }



    /**
     *******************             PreEmbedding Phase 1              **********************
     * 
     * We search the vertices on the same level (same distance from the border BFS).
     * These vertices will all have the same value for the z coordinate, starting from 0.
     * We calculate the distances between these vertices, add them all up.
     * This will be the outline of the circle we will place the vertices on.
     * We calculate for each vertex the angle we have to move over the circle
     * and place them in the correct order. We continue this for all the vertices on each level.
     *
     * Note: this might (and most of the times will) make a bad embedding for the levels
     * where we find the pentagons, but this will be tackled in phase 2.
     **/
    private void calculateEmbeddingPhase1() {
        double outline, z = 0, diff = 0, theta, thetaIncreaseX, thetaIncreaseL, radius;
        int current, temp, temp2, temp3, amountOfL = 0, amountOfX = 0;
        boolean evenDepth;

        current = 0;

        /**
         * Count the amount of times we find 2 vertices who are neighbours and on the
         * same level (amountOfL) and the vertices who have 1 common neighbour (amountOfX).
         */
        for (int i=current; vertDepth[1][i] == 0; i++) {
            temp2 = (i + 1 >= nrVertices || vertDepth[1][i + 1] != 0) ? vertDepth[0][current] : vertDepth[0][i+1];
            if (areNeighbours(vertDepth[0][i], temp2 + 1))
                amountOfL++;
            else
                amountOfX++;
        }

        /**
         * The outline of the circle for level 0
         */
        outline = amountOfL * l + amountOfX * vertexDistanceInHexagon;

        /**
         * We first set the levels containing only hexagons correct.
         * This will form a good first embedding in the shape of a cone.
         */
        while (current < firstPentagon) {

            temp = vertDepth[1][current];
            evenDepth = temp%2 == 0;

            theta = 0;
            thetaIncreaseX = 2.0*Math.PI*(vertexDistanceInHexagon)/outline;
            thetaIncreaseL = 2.0*Math.PI*((evenDepth?l:2*l))/outline;
            radius = outline / (2.0*Math.PI);


            // we save current's initial value for reference in the loop
            temp3 = current;

            /**
             * Every other level we have to start a bit farther in the circle.
             * This value has been noticed as a good reference point.
             */
            if (!evenDepth)
                theta += thetaIncreaseX/3;

            while (current < nrVertices && vertDepth[1][current] == temp) {
                temp2 = vertDepth[0][current];
                graphCoords[temp2][0] = radius * Math.cos(theta);
                graphCoords[temp2][1] = radius * Math.sin(theta);
                graphCoords[temp2][2] = z;

                temp2 = (current + 1 >= nrVertices || vertDepth[1][current + 1] != vertDepth[1][current]) ? vertDepth[0][temp3] : vertDepth[0][current+1];
                if ((evenDepth && areNeighbours(vertDepth[0][current], temp2 + 1))  || (!evenDepth && !this.haveCommonNeighbour(vertDepth[0][current], temp2)))
                    theta += thetaIncreaseL;
                else
                    theta += thetaIncreaseX;
                current++;
            }

            /**
             * The outline decreases linearly every 2 levels in the exact same pattern.
             */

            // Update outline
            if (evenDepth) {
                diff = amountOfL * (vertexDistanceInHexagon - l);
                outline -= diff;
                amountOfX -= amountOfL;
            }
            else {
                diff = amountOfL * l;
                outline -= diff;
            }

            // Update z for next round
            if (!evenDepth && amountOfX > 0) {
                diff /= 2.0*Math.PI;
                z += Math.sqrt(l*l - diff*diff) - (l - vertexDistanceInHexagon/2)*amountOfL/amountOfX;
            }
            else
                z += a;
        }

        /**
         * This is the first vertex from the first level that contains at least 1 pentagon.
         */
        int verticesPentagon = current;

        /**
         * We have to set the next levels in a usable pre-embedding. Since these levels
         * may contain pentagons, there are more uncertainties.
         * The outline doesn't decrease linearly anymore,
         * there are more then 2 possible lengths between vertices from the same level, etc.
         * This is a simplistic pre-embedding which will give our algorithm a starting point
         * for phase 2.
         */
        while (current < nrVertices) {

            temp = vertDepth[1][current];
            evenDepth = temp%2 == 0;

            amountOfX = 0;
            for (int i=current; i < nrVertices && vertDepth[1][i] == temp; i++) {
                amountOfX++;
            }

            outline = amountOfX * vertexDistanceInHexagon;

            theta = 0;
            thetaIncreaseX = 2.0*Math.PI/amountOfX;
            radius = outline / (2.0*Math.PI);

            if (!evenDepth)
                theta += thetaIncreaseX/3;

            while (current < nrVertices && vertDepth[1][current] == temp) {
                temp2 = vertDepth[0][current];
                graphCoords[temp2][0] = radius * Math.cos(theta);
                graphCoords[temp2][1] = radius * Math.sin(theta);
                graphCoords[temp2][2] = z;
                theta += thetaIncreaseX;
                current++;
            }

            z += a;
        }

        zeroZ = graphCoords[vertDepth[0][verticesPentagon]][2];
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
        for (int i = 0; i< currentGraph[vertex].length; i++) {
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
                    //for (int nn : currentGraph[n-1]) {
                    for (int j = 0; j < currentGraph[n-1].length; j++) {
                        int nn = currentGraph[n-1][j];
                        if (nn != 0 && nn - 1 != n) {
                            d = getDistance(vertex, nn - 1);
                            if (d > epsilon) {
                                if (!quadratic)
                                    d = (vertexDistanceInHexagon - d) / d;
                                else if (vertexDistanceInHexagon - d < 0)
                                    d = - Math.pow(vertexDistanceInHexagon - d, 2) / d;
                                else
                                    d = Math.pow(vertexDistanceInHexagon - d, 2) / d;
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
            double [] zero = {0, 0, zeroZ};
            d = getDistance(vertex, zero);
            d = 1 + forceConvex*(1/d);
            x *= d;
            y *= d;
            z *= d;
        }

        graphCoords[vertex][0] += t*x;
        graphCoords[vertex][1] += t*y;
        graphCoords[vertex][2] += t*z;
    }

    /**
     * Calculate the distance between a vertex and a point in 3d
     * @param from the vertex to start from
     * @param to the point we want to know the distance to the vertex from
     * @return the distance between the vertex and the point in 3d
     */
    private double getDistance(int from, double [] to) {
        double d = 0.0, temp;
        for (int i=0; i<3; i++) {
            temp = graphCoords[from][i] - to[i];
            d += temp * temp;
        }
        return Math.sqrt(d);
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
     * Calculate the distance between two vertices, but don't
     * take the square root. This can be used if the actual scalar value of
     * the distance doesn't matter, but we need a indication.
     * @param from the first vertex
     * @param to the second vertex
     * @return the distance between the two vertices
     */
    private double squaredDistPositions(int from, int to) {
        double d = 0.0, temp;
        for (int i=0; i<3; i++) {
            temp = graphCoords[from][i] - graphCoords[to][i];
            d += temp * temp;
        }
        return d;
    }

    private int findPlaceInNeighbours(int vertex, int neighbour) {
        for (int i=0; i<3; i++) {
            if (currentGraph[vertex][i] == neighbour)
                return i;
        }
        return -1;
    }

    /**
     * Calculate the depths of all the vertices from <code>currentGraph</code>.
     * We start by searching the outer vertices and give them depth 0.
     * We continue our search to all vertices with BFS starting from the outer vertices.
     * We also keep track of the order we found the vertices in.
     * We fill in the global variable <code>vertDepth</code> as a matrix containing 2 arrays.
     * The first is the order we found the vertices in. The second are the corresponding
     * depths we found the vertices from the first array on.
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
            if (currentGraph[currentGraph[start][1] - 1][0] == start + 1)
                if (currentGraph[currentGraph[currentGraph[start][1] - 1][1] - 1][2] == 0)
                    next = -1;
            else if (currentGraph[currentGraph[start][1] - 1][1] == start + 1)
                if (currentGraph[currentGraph[currentGraph[start][1] - 1][2] - 1][2] == 0)
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
            current = currentGraph[current][temp] == 0 ? currentGraph[current][0] - 1 : currentGraph[current][temp] - 1;
            if (currentGraph[current][2] != 0) {
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
        while (start < nrVertices) {
            for (int i=2; i>=0; i--) {
                if (currentGraph[vertDepth[0][start]][i] != 0) {
                    current = currentGraph[vertDepth[0][start]][i] - 1;
                    if (seen[current] == 0) {
                        vertDepth[0][index] = current;
                        vertDepth[1][index++] = vertDepth[1][start] + 1;
                        seen[current] = 1;
                        break;
                    }
                }
            }
            start++;
        }
    }

    private static void usage() {
        String output = "usage: PreEmbedderNanocones [options] <input >output\n" +
                "\n" +
                "The input must be in writegraph format as specified by the VEGA project\n\n" +
                "Options: (n stands for integer value, d for double value)\n" +
                "   --help      Will show this information.\n" +
                "   -p n        the amount of phases to be executed. Must be between 0 and 4.\n" +
                "               Phase 1 is the pre-embedding. All vertices are divided over circles \n" +
                "               in different layers. In phase 2 the top (the levels containing pentagons)\n" +
                "               are positioned better with a kind of local search.\n" +
                "               In the 3rd phase the local search is executed over the whole graph, \n" +
                "               repositioning vertices just that little bit better.\n" +
                "               In the last phase (4th) the top is again better looked for the last time.\n" +
                "   -f d[,d[,d[,d]]]\n" +
                "               The factors for which the neighbours's neighbours count for in the different\n" +
                "               phases. You can specify upto 4 different factors, one for each phase.\n" +
                "               If you specify less then 4, the phases that follow will use the same factor\n" +
                "               as the last one specified. They should be doubles (x.xxx), seperated by comma's.\n" +
                "   -s n[,n[,n]]\n" +
                "               The amount of times we should visit each vertex in phase 2 till 4 \n" +
                "               (phase 1 is constant). You can specify up to 3 amounts, seperated by comma's.\n" +
                "               If you specify less then 3, the phases that follow will use the same amount\n" +
                "               as the last phase specified.\n" +
                "   -sp n[,n[,n]]\n" +
                "               The amount of times a graph should be printed in phases 2, 3 or 4.\n" +
                "               If you specify less then 3, the following phases will print as many graphs\n" +
                "               as specified by the last number." +
                "   -l d        the value for the length between 2 atoms, in double format (x.xxx)\n" +
                "   -print n    decide wether the final graph should be printed to the output, should be 0 or 1\n" +
                "   -layers n   the minimum amount of layers needed to calculate the embedding.\n" +
                "               If the graph has less layers, layers will be added. \n" +
                "               Then the embedding will be calculated.\n" +
                "               Afterwards the added layers are removed again. Should be at least 0.\n" +
                "\n" +
                "The default is:    -p 4 -f 0.25,0.15,0.22,0.35 -s 50,500,500 -sp 0 -l 1.42 -print 1 -layers 3";
        System.err.println(output);
        System.exit(1);
    }

    private void setPhases(int p) {
        this.phases = p;
    }

    private void setPrint(boolean b) {
        this.print = b;
    }

    private void setLengthBetweenAtoms(double l) {
        this.l = l;
        vertexDistanceInHexagon = l * 1.73205081;
        a = l * 0.5;
    }

    private void setMinimumLayers(int layers) {
        this.minLayers = layers;
    }

    //code in comments is Java 5, when switching to this version these lines should be used (nvcleemp)
    //private void setFactors(double ... factors) {
    private void setFactors(double[] factors) {
        System.arraycopy(factors, 0, this.factors, 0, factors.length);
        if (factors.length < this.factors.length)
            for (int i=factors.length; i<this.factors.length; i++)
                this.factors[i] = factors[factors.length - 1];
    }

    //code in comments is Java 5, when switching to this version these lines should be used (nvcleemp)
    //private void setMaxSteps(int ... steps) {
    private void setMaxSteps(int[] steps) {
        System.arraycopy(steps, 0, this.steps, 0, steps.length);
        if (steps.length < this.steps.length)
            for (int i=steps.length; i<this.steps.length; i++)
                this.steps[i] = steps[steps.length - 1];
    }

    //code in comments is Java 5, when switching to this version these lines should be used (nvcleemp)
    //private void setShowPerPhase(int ... show) {
    private void setShowPerPhase(int[] show) {
        System.arraycopy(show, 0, this.show, 0, show.length);
        if (show.length < this.show.length)
            for (int i=show.length; i<this.show.length; i++)
                this.show[i] = show[show.length - 1];
    }

    private void setSolo(boolean b) {
        this.solo = b;
    }


}

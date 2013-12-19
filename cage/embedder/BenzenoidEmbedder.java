/* BenzenoidEmbedder.java
 *
 * Based on the benzenoid generator in Grinvin.
 */
package cage.embedder;

import java.io.InputStream;
import java.util.Scanner;

/**
 * Embedder for benzenoids.
 */
public class BenzenoidEmbedder {
    
    private static final int[][] VECTORS = {{1, 0}, {1, 1}, {0, 1}, {-1, 0}, {-1, -1}, {0, -1}}; //run through a hexagon clockwise
    
    private final IO io;
    private int [][] currentGraph;
    private double [][] graphCoords;
    private int nrOfVertices;

    /**
     * This class embeds benzenoids for CaGe.
     * @param input A string representing the graphs from CaGe
     */
    public BenzenoidEmbedder(String input) {
        this.io = new IO(input, 100, 3);
    }
    
    public void startEmbedding() {
        while ((nrOfVertices = io.findNextGraph()) > 0) {
            this.currentGraph = io.getCurrentGraph();
            this.graphCoords = io.getGraphCoords();
            embedGraph();
            io.printGraph(System.out);
        }
    }

    private void embedGraph() {
        int[][] benzene = new int[nrOfVertices][];
        int firstDegreeThree = -1;
        for (int vertex = 0; vertex < nrOfVertices; vertex++) {
            if (currentGraph[vertex][2]>0) { //degree 3
                if (firstDegreeThree == -1) {
                    firstDegreeThree = vertex;
                }
                benzene[vertex] = new int[3];
                benzene[vertex][0] = currentGraph[vertex][0] - 1;
                benzene[vertex][1] = currentGraph[vertex][1] - 1;
                benzene[vertex][2] = currentGraph[vertex][2] - 1;
            } else { //degree 2
                benzene[vertex] = new int[2];
                benzene[vertex][0] = currentGraph[vertex][0] - 1;
                benzene[vertex][1] = currentGraph[vertex][0] - 1;
            }
        }
        int[][] coordinates = new int[nrOfVertices][];
        coordinates[firstDegreeThree] = new int[]{0, 0};
        coordinates[benzene[firstDegreeThree][0]] = new int[]{1, 0};
        coordinates[benzene[firstDegreeThree][1]] = new int[]{0, 1};
        coordinates[benzene[firstDegreeThree][2]] = new int[]{-1, -1};

        giveCoordinates(firstDegreeThree, benzene[firstDegreeThree][0], 0, coordinates, benzene);
        giveCoordinates(firstDegreeThree, benzene[firstDegreeThree][1], 2, coordinates, benzene);
        giveCoordinates(firstDegreeThree, benzene[firstDegreeThree][2], 4, coordinates, benzene);

        //TODO: find correct rotation for display
        int order = nrOfVertices;
        double[][] realCoordinates = new double[order][2];
        double minX = Double.POSITIVE_INFINITY;
        double maxX = Double.NEGATIVE_INFINITY;
        double minY = Double.POSITIVE_INFINITY;
        double maxY = Double.NEGATIVE_INFINITY;
        for (int i = 0; i < order; i++) {
            realCoordinates[i][0] = coordinates[i][1] * Math.cos(Math.PI / 6);
            if (realCoordinates[i][0] < minX) {
                minX = realCoordinates[i][0];
            }
            if (realCoordinates[i][0] > maxX) {
                maxX = realCoordinates[i][0];
            }
            realCoordinates[i][1] = coordinates[i][0] - coordinates[i][1] * Math.sin(Math.PI / 6);
            if (realCoordinates[i][1] < minY) {
                minY = realCoordinates[i][1];
            }
            if (realCoordinates[i][1] > maxY) {
                maxY = realCoordinates[i][1];
            }
        }
        double transX = -(minX + maxX) / 2;
        double transY = -(minY + maxY) / 2;
        double scale = (maxX - minX) > (maxY - minY) ? 2 / (maxX - minX) : 2 / (maxY - minY);
        for (int i = 0; i < order; i++) {
            graphCoords[i][0] = (realCoordinates[i][0] + transX) * scale;
            graphCoords[i][1] = (realCoordinates[i][1] + transY) * scale;
        }
    }

    private InputStream in;
    
    //vertex[previous] has degree 2
    private boolean checkClockwise(int previouspreviousVertex, int previousVertex, int previousStep, int[][] coordinates, int[][] benzene) {
        int step = (previousStep + 1) % 6;
        int[] tempCoords = addVector(coordinates[previousVertex], step);
        int vertex;
        if (benzene[previousVertex][0] == previouspreviousVertex) {
            vertex = benzene[previousVertex][1];
        } else {
            vertex = benzene[previousVertex][0];
        }
        int nodes = 2; //make sure we only check a hexagon
        while (coordinates[vertex] == null && nodes < 6) {
            previouspreviousVertex = previousVertex;
            previousVertex = vertex;
            int previousIndex = -1;
            for (int i = 0; i < benzene[previousVertex].length; i++) {
                if (benzene[previousVertex][i] == previouspreviousVertex) {
                    previousIndex = i;
                }
            }
            vertex = benzene[previousVertex][(previousIndex - 1 + benzene[previousVertex].length) % benzene[previousVertex].length];
            step = (step + 1) % 6;
            tempCoords = addVector(tempCoords, step);
            nodes++;
        }

        return (coordinates[vertex] != null) && (coordinates[vertex][0] == tempCoords[0]) && (coordinates[vertex][1] == tempCoords[1]);
    }

    private int[] addVector(int[] vector, int vectorToAdd) {
        int[] newVector = new int[2];
        newVector[0] = vector[0] + VECTORS[vectorToAdd][0];
        newVector[1] = vector[1] + VECTORS[vectorToAdd][1];
        return newVector;
    }

    private void giveCoordinates(int previousVertex, int vertex, int step, int[][] coordinates, int[][] benzene) {
        coordinates[vertex] = addVector(coordinates[previousVertex], step);
        if (benzene[vertex].length == 2) {
            int index = (benzene[vertex][0] == previousVertex) ? 1 : 0;
            if (coordinates[benzene[vertex][index]] == null) {
                if (this.checkClockwise(previousVertex, vertex, step, coordinates, benzene)) {
                    giveCoordinates(vertex, benzene[vertex][index], (step + 1) % 6, coordinates, benzene);
                } else {
                    giveCoordinates(vertex, benzene[vertex][index], (step - 1 + 6) % 6, coordinates, benzene);
                }
            }
        } else {
            int previousIndex = -1;
            for (int i = 0; i < 3; i++) {
                if (benzene[vertex][i] == previousVertex) {
                    previousIndex = i;
                }
            }
            if (coordinates[benzene[vertex][(previousIndex - 1 + 3) % 3]] == null) {
                giveCoordinates(vertex, benzene[vertex][(previousIndex - 1 + 3) % 3], (step + 1) % 6, coordinates, benzene);
            }
            if (coordinates[benzene[vertex][(previousIndex + 1) % 3]] == null) {
                giveCoordinates(vertex, benzene[vertex][(previousIndex + 1) % 3], (step - 1 + 6) % 6, coordinates, benzene);
            }
        }
    }
    
        private static void usage() {
        String output = "usage: BenzenoidEmbedder [options] <input >output\n" +
                "\n" +
                "The input must be in writegraph format as specified by the VEGA project\n\n" +
                "Options: (n stands for integer value, d for double value)\n" +
                "   --help      Will show this information.\n";
        System.err.println(output);
        System.exit(1);
    }

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

        BenzenoidEmbedder embedder = new BenzenoidEmbedder(input);

        embedder.startEmbedding();
    }

}

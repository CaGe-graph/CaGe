/**
 *
 * IO.java by Simon Buelens
 *          on Aug 4, 2009
 */

package cage.embedder;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.PrintStream;
import java.io.StringReader;
import java.text.MessageFormat;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author Simon Buelens
 */
public class IO {

    private String input;
    private int [][] currentGraph;
    private double [][] graphCoords;
    private int graphStartColumns;
    private int graphStartRows;
    private int dimension = 0;
    private int nrVertices = 0;

    public IO(String input, int graphStartRows, int graphStartColumns) {
        this.input = input;
        this.graphStartRows = graphStartRows;
        this.graphStartColumns = graphStartColumns;
    }

    /**
     * Searches the next graph in the input string. If a graph is found, the amount of vertices
     * is returned and the vertices can be requested with <code>getCurrentGraph()</code>.
     * If the input code contained coordinates for the vertices, they can be requested with
     * <code>getGraphCoords()</code>.
     * @return the amount of vertices in the graph just found or 0 if no graph is found.
     */
    public int findNextGraph() {
        // Find in what kind of code the graph is written
        String header = "";
        int nl = 0;
        while (header.trim().equals("")) {
            nl = input.indexOf('\n');
            if (nl == -1) {
                input = "";
                return 0;
            }
            header = input.substring(0,nl);
            this.input = input.substring(nl + 1);
        }
        if (header.equalsIgnoreCase(">>writegraph2d<<")) {
            this.dimension = 2;
        }
        else if (header.equalsIgnoreCase(">>writegraph3d<<")) {
            this.dimension = 3;
        }
        else {
            if (dimension == 0) {
                throw new RuntimeException("Graph should start with >>writegraph2d<< or >>writegraph3d<<.\n" +
                        "Instead given input was found:\n" +
                        "\t\t" + ((nl==-1)?input.substring(0,nl):input));
            }
        }

        currentGraph = new int [this.graphStartRows][this.graphStartColumns];
        graphCoords = new double [currentGraph.length][dimension];
        BufferedReader reader = new BufferedReader(new StringReader(input));
        String line = "";
        nrVertices = 0;
        try {
            while ((line = reader.readLine()) != null) {
                if (line.equals(""))
                    continue;
                if (line.trim().charAt(0) == '0') {
                    // End of graph reached
                    input = input.substring(input.indexOf("0")+1);
                    if (nrVertices > 0)
                        return nrVertices;
                }
                if ((line = line.trim()).equals(""))
                    continue;
                String[] vertices = line.replaceAll("\t", "").split(" +");
                if (vertices.length > 0) {
                    int vertex = Integer.parseInt(vertices[0]) - 1;
                    if (vertex >= currentGraph.length) {
                        doubleCurrentGraph(currentGraph);
                        doubleGraphCoords(getGraphCoords());
                    }
                    // If the amount of neighbours exceeds the start length
                    if (vertices.length - 1 - dimension> this.graphStartColumns) {
                        currentGraph[vertex] = new int[vertices.length - 1 - dimension];
                    }
                    for (int i = 0; i<dimension; i++) {
                        graphCoords[vertex][i] = Double.parseDouble(vertices[i+1]);
                    }
                    for (int i = dimension + 1; i < vertices.length; i++) {
                        currentGraph[vertex][i - 1 - dimension] = Integer.parseInt(vertices[i]);
                    }
                    nrVertices++;
                }
                nl = input.indexOf("\n");
                if (nl == -1)
                    input = "";
                else
                    this.input = input.substring(input.indexOf("\n") + 1);
            }
        } catch (IOException ex) {
            Logger.getLogger(IO.class.getName()).log(Level.SEVERE, null, ex);
            return 0;
        }
        if (nrVertices == 0)
            return this.findNextGraph();
        return nrVertices;
    }

    private void doubleCurrentGraph(int[][] matrix) {
        currentGraph = new int [matrix.length*2][this.graphStartColumns];
        System.arraycopy(matrix, 0, currentGraph, 0, matrix.length);
    }

    private void doubleGraphCoords(double[][] matrix) {
        graphCoords = new double [matrix.length*2][dimension];
        System.arraycopy(matrix, 0, graphCoords, 0, matrix.length);
    }

    /**
     * Print out the graph in writegraph format
     * @param output the stream to print the graph on
     */
    public void printGraph(PrintStream output) {
        //code in comments is Java 5, when switching to this version these lines should be used (nvcleemp)
        
        //output.format(">>writegraph%dd<<\n", dimension);
        output.println(MessageFormat.format(">>writegraph{0,number,integer}d<<\n", new Object[]{Integer.valueOf(dimension)}));
        for (int i=0; i<currentGraph.length; i++) {
            // If the first neighbour of a vertex is vertex 0, we know
            // we have reached the end of this graph.
            if (currentGraph[i][0] == 0) {
                System.out.println("0");
                return;
            }
            //output.format("%4d % 8.3f % 8.3f % 8.3f", i+1, getGraphCoords()[i][0], getGraphCoords()[i][1], getGraphCoords()[i][2]);
            output.print(MessageFormat.format("{0} {1,number,########.###} {2,number,########.###} {3,number,########.###} ",
                    new Object[]{Integer.toString(i+1),
                        Double.valueOf(getGraphCoords()[i][0]),
                        Double.valueOf(getGraphCoords()[i][1]),
                        Double.valueOf(getGraphCoords()[i][2])}));
            for (int j=0; j<currentGraph[i].length; j++) {
                // There are no neighbours with vertex number 0,
                // don't print this and go to the next vertex.
                if (currentGraph[i][j] == 0)
                    break;
                //output.format("%4d",currentGraph[i][j]);
                output.print(MessageFormat.format(" {0}", new Object[]{Integer.toString(currentGraph[i][j])}));
            }
            System.out.println();
        }
        System.out.println("0");
    }

    public int [][] getCurrentGraph() {
        return this.currentGraph;
    }

    public int getDimension() {
        return dimension;
    }

    /**
     * @return the graphCoords
     */
    public double[][] getGraphCoords() {
        return graphCoords;
    }

    /**
     * @return the number of vertices in last graph
     */
    public int getNumberOfVertices() {
        return this.nrVertices;
    }

}

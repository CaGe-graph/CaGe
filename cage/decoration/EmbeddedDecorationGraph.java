package cage.decoration;

import java.util.Arrays;

/**
 * A <code>EmbeddedDecorationGraph</code> represent a <code>{@link DecorationGraph}</code>
 * together with coordinates for each vertex. The coordinates are stored as
 * barycentric coordinates in function of the V, E, and F corner of the decoration.
 * 
 * @author nvcleemp
 */
public class EmbeddedDecorationGraph {
    
    private final DecorationGraph graph;
    private final double[][] barycentricCoordinates;

    /**
     * Creates an <code>EmbeddedDecorationGraph</code> based on the given
     * <code>{@link DecorationGraph}</code> and the specified coordinates.
     * 
     * @param graph
     * @param barycentricCoordinates 
     */
    public EmbeddedDecorationGraph(DecorationGraph graph, double[][] barycentricCoordinates) {
        this.graph = graph;
        this.barycentricCoordinates = barycentricCoordinates;
    }
    
    /**
     * Creates an <code>EmbeddedDecorationGraph</code> based on the given
     * <code>{@link DecorationGraph}</code> and all coordinates set to zero.
     * 
     * @param graph
     */
    public EmbeddedDecorationGraph(DecorationGraph graph) {
        this.graph = graph;
        this.barycentricCoordinates = new double[graph.getOrder()][3];
    }

    /**
     * Returns the underlying <code>{@link DecorationGraph}</code>.
     * @return 
     */
    public DecorationGraph getGraph() {
        return graph;
    }

    /**
     * Returns the coordinates for the specified vertex. The returned array is a
     * copy of the data in this object, so it can safely be modified.
     * 
     * @param v
     * @return 
     */
    public double[] getBarycentricCoordinatesFor(int v) {
        return Arrays.copyOf(barycentricCoordinates[v], 3);
    }

    /**
     * Returns the coordinates for the specified vertex. The returned array is bot
     * a copy of the data in this object, so it should be treated as read-only.
     * 
     * @param v
     * @return 
     */
    public double[] nocopy_getBarycentricCoordinatesFor(int v) {
        return barycentricCoordinates[v];
    }
}

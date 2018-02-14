package cage.decoration;

/**
 * A <code>DecorationGraph</code> represent the graph formed by the vertex nodes
 * in a decoration.
 * 
 * @author nvcleemp
 */
public class DecorationGraph {
    
    private final int order;
    private final Neighbour[][] neighbours;

    /**
     * Creates a decoration graph with the specified order and neighbours for
     * each vertex.
     * 
     * @param order
     * @param neighbours 
     */
    public DecorationGraph(int order, Neighbour[][] neighbours) {
        this.order = order;
        this.neighbours = neighbours;
    }

    /**
     * Returns the order of this graph. The order is the number of vertices.
     * 
     * @return 
     */
    public int getOrder() {
        return order;
    }
    
    /**
     * Returns the neighbours for the specified vertex.
     * 
     * @param v
     * @return 
     */
    public Neighbour[] getNeighboursFor(int v){
        return neighbours[v];
    }
}

package cage;

/**
 * An ElementRule object can be used to deduce the chemical element of an
 * atom corresponding with a certain vertex in a graph.
 */
public interface ElementRule {

    /**
     * Returns the chemical symbol of the chemical element of the
     * atom corresponding with <tt>vertex</tt> in <tt>graph</tt>.
     *
     * @param graph The graph that contains the vertex
     * @param vertex The vertex for which the element is requested
     * @return The symbol of the element of <tt>vertex</tt>
     */
    public String getElement(EmbeddableGraph graph, int vertex);
}


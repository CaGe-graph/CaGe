package cage;

public interface EmbeddableGraph {

    final public static char embedAlways = 'a';
    final public static char embedIfRequired = 'i';

    public String getComment();

    public void setComment(String comment);

    /**
     * Adds a vertex to this graph.
     */
    public void addVertex();

    /**
     * Adds an edge from the last added vertex to a <code>to</code>.
     * @param to The destination of the edge to add.
     */
    public void addEdge(int to);

    /**
     * Returns the order of the graph, i.e. the number of vertices.
     * @return The number of vertices in this graph.
     */
    public int getSize();

    /**
     * Returns the valency of <code>vertex</code> i.e. The number of edges that
     * are incident with this vertex.
     * @param vertex The vertex for which the valency should be returned
     * @return The number of edges that are incident with <code>vertex</code>.
     */
    public int getValency(int vertex);

    public EdgeIterator getEdgeIterator(int vertex);

    public boolean has2DCoordinates();

    public float[] get2DCoordinates(int vertex);

    public float[][] get2DCoordinates();

    public void set2DCoordinates(int vertex, float[] coords);

    public boolean has3DCoordinates();

    public float[] get3DCoordinates(int vertex);

    public float[][] get3DCoordinates();

    public void set3DCoordinates(int vertex, float[] coords);
}

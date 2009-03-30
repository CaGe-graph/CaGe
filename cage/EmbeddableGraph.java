
package cage;


public abstract class EmbeddableGraph
{
  final public static char embedAlways = 'a';
  final public static char embedIfRequired = 'i';

  public abstract String getComment();
  public abstract void setComment(String comment);

  /**
   * Adds a vertex to this graph.
   */
  public abstract void addVertex();

  /**
   * Adds an edge from the last added vertex to a <code>to</code>.
   * @param to The destination of the edge to add.
   */
  public abstract void addEdge(int to);

  /**
   * Returns the order of the graph, i.e. the number of vertices.
   * @return The number of vertices in this graph.
   */
  public abstract int getSize();

  /**
   * Returns the valency of <code>vertex</code> i.e. The number of edges that
   * are incident with this vertex.
   * @param vertex The vertex for which the valency should be returned
   * @return The number of edges that are incident with <code>vertex</code>.
   */
  public abstract int getValency(int vertex);

  public abstract EdgeIterator getEdgeIterator (int vertex);

  public abstract boolean has2DCoordinates();
  public abstract float[] get2DCoordinates(int vertex);
  public abstract float[][] get2DCoordinates();
  public abstract void set2DCoordinates(int vertex, float[] coords);

  public abstract boolean has3DCoordinates();
  public abstract float[] get3DCoordinates(int vertex);
  public abstract float[][] get3DCoordinates();
  public abstract void set3DCoordinates(int vertex, float[] coords);
}

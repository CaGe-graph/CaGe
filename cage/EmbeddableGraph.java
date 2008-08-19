
package cage;


public abstract class EmbeddableGraph
{
  final public static char embedAlways = 'a';
  final public static char embedIfRequired = 'i';

  public abstract String getComment();
  public abstract void setComment(String comment);

  public abstract void addVertex();

  public abstract void addEdge(int to);

  public abstract int getSize();

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

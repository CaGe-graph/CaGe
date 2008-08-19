
package cage;


import lisken.systoolbox.*;
import com.sun.java.util.collections.*;


public class NativeEmbeddableGraph extends EmbeddableGraph
{
  protected long nGraph;

  private native long newNGraph();
  private native byte[] nGetComment(long nGraph);
  private native void nSetComment(long nGraph, byte[] comment);
  private native int nGetFormat(long nGraph);
  private native void nSetFormat(long nGraph, int format);
  private native void nAddVertex(long nGraph);
  private native void nAddEdge(long nGraph, int to);
  private native int nGetSize(long nGraph);
  private native int nGetValency(long nGraph, int vertex);
  private native NativeEdgeIterator nGetEdgeIterator(long nGraph, int vertex);
  private native boolean nHas2DCoordinates(long nGraph);
  private native float[] nGet2DCoordinates(long nGraph, int vertex);
  private native float[][] nGetAll2DCoordinates(long nGraph);
  private native void
   nSet2DCoordinates(long nGraph, int vertex, float[] coords);
  private native boolean nHas3DCoordinates(long nGraph);
  private native float[] nGet3DCoordinates(long nGraph, int vertex);
  private native float[][] nGetAll3DCoordinates(long nGraph);
  private native void
   nSet3DCoordinates(long nGraph, int vertex, float[] coords);
  private native byte[] toBytes(long nGraph);
  private native void nFinalize(long nGraph);

  public NativeEmbeddableGraph()
  {
    nGraph = newNGraph();
  }
  public NativeEmbeddableGraph(long nGraph)
  {
    this.nGraph = nGraph;
  }

  public String getComment()
  {
    byte[] bytes = nGetComment(nGraph);
    return bytes == null ? null : new String(bytes);
  }

  public void setComment(String comment)
  {
    nSetComment(nGraph, comment.getBytes());
  }

  public int getFormat()
  {
    return nGetFormat(nGraph);
  }

  public void setFormat(int format)
  {
    nSetFormat(nGraph, format);
  }

  public void addVertex()
  {
    nAddVertex(nGraph);
  }

  public void addEdge(int to)
  {
    nAddEdge(nGraph, to);
  }

  public int getSize()
  {
    return nGetSize(nGraph);
  }

  public int getValency(int vertex)
  {
    return nGetValency(nGraph, vertex);
  }

  public EdgeIterator getEdgeIterator (int vertex)
  {
    return nGetEdgeIterator(nGraph, vertex);
  }

  public boolean has2DCoordinates()
  {
    return nHas2DCoordinates(nGraph);
  }

  public float[] get2DCoordinates(int vertex)
  {
    return nGet2DCoordinates(nGraph, vertex);
  }

  public float[][] get2DCoordinates()
  {
    return nGetAll2DCoordinates(nGraph);
  }

  public void set2DCoordinates(int vertex, float[] coords)
  {
    nSet2DCoordinates(nGraph, vertex, coords);
  }

  public boolean has3DCoordinates()
  {
    return nHas3DCoordinates(nGraph);
  }

  public float[] get3DCoordinates(int vertex)
  {
    return nGet3DCoordinates(nGraph, vertex);
  }

  public float[][] get3DCoordinates()
  {
    return nGetAll3DCoordinates(nGraph);
  }

  public void set3DCoordinates(int vertex, float[] coords)
  {
    nSet3DCoordinates(nGraph, vertex, coords);
  }

  public String toString()
  {
    return new String(toBytes(nGraph));
  }

  protected void finalize() throws Throwable
  {
    nFinalize(nGraph);
    super.finalize();
  }
}


class NativeEdgeIterator extends EdgeIterator
{
  private long nIter;

  private native boolean nHasNextEdge(long nIter);
  private native int nGetNextEdge(long nIter);
  private native void nFinalize(long nIter);

  public boolean hasNext()
  {
    return nHasNextEdge(nIter);
  }

  public Object next()
   throws NoSuchElementException
  {
    return new Integer2(nGetNextEdge(nIter));
  }

  public int nextEdge()
   throws NoSuchElementException
  {
    return nGetNextEdge(nIter);
  }

  public void remove()
   throws com.sun.java.util.collections.UnsupportedOperationException
  {
    throw new com.sun.java.util.collections.UnsupportedOperationException("Graph edges can't be removed");
  }

  protected void finalize() throws Throwable
  {
    nFinalize(nIter);
    super.finalize();
  }
}


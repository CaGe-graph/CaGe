
package cage;


import com.sun.java.util.collections.*;


public abstract class EdgeIterator implements Iterator
{
  public abstract int nextEdge() throws NoSuchElementException;
}


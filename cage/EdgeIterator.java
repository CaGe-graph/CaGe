package cage;

import java.util.Iterator;
import java.util.NoSuchElementException;

public interface EdgeIterator extends Iterator {

    public abstract int nextEdge() throws NoSuchElementException;
}


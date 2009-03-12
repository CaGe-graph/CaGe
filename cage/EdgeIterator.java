package cage;

import java.util.Iterator;
import java.util.NoSuchElementException;

public abstract class EdgeIterator implements Iterator {

    public abstract int nextEdge() throws NoSuchElementException;
}


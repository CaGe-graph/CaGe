package cage;

import java.util.Iterator;
import java.util.NoSuchElementException;

/**
 * Extension of the interface <code>Iterator</code> with an extra method to
 * iterate over edges directly.
 */
public interface EdgeIterator extends Iterator {

    /**
     * Returns the next edge in the iteration.  Calling this method
     * repeatedly until the {@link #hasNext()} method returns false will
     * return each element in the underlying collection exactly once.
     *
     * @return the next edge in the iteration as an <code>int</code>.
     * @exception NoSuchElementException iteration has no more elements.
     */
    public int nextEdge() throws NoSuchElementException;
}


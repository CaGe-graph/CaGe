package cage;

import java.util.ArrayList;
import java.util.List;
import java.util.NoSuchElementException;

/**
 * A result "collection" of CaGe's production process, with
 * some methods to traverse it.
 *
 * A CaGeResultList maintains a "cursor" that points at the
 * "current" element in the list (as long as it is not empty).
 * The cursor is moved by the <code>next()</code> and
 * <code>previous()</code> methods.
 * <code>addGraph</code> will set the cursor to the graph
 * that is added (i.e. to the end of the list).
 *
 * The class implements several methods that look like those
 * of the ListIterator interface in terms of name and signature.
 * But we do not keep to the (semantic) contract of that
 * interface because of the different way backward/forward
 * movement is handled.  Alternating calls to next() and
 * previous() will not return the same element repeatedly,
 * as stipulated in the contract.  Instead, if both calls succeed,
 * the same two elements will be returned in alternation.
 */
public class CaGeResultList {

    private List<CaGeResult> results = new ArrayList<>();
    private CaGeResult result = null;
    private int cursor = 0,  found,  highestGraphNo = 0,  highestGraphNoIndex = -1;

    public CaGeResultList() {
    }

    public void addGraph(EmbeddableGraph graph, int graphNo) {
        addResult(new CaGeResult(graph, graphNo));
    }

    public void addResult(CaGeResult result) {
        cursor = results.size();
        if (result.getGraphNo() > highestGraphNo) {
            highestGraphNo = result.getGraphNo();
            highestGraphNoIndex = cursor;
        }
        this.result = result;
        results.add(result);
    }

    public EmbeddableGraph getGraph() {
        return result == null ? null : result.getGraph();
    }

    public int getGraphNo() {
        return result == null ? 0 : result.getGraphNo();
    }

    public CaGeResult getResult() {
        try {
            result = results.get(cursor);
        } catch (IndexOutOfBoundsException e) {
            return null;
        }
        return result;
    }

    public boolean findGraphNo(int no) {
        int n;
        n = results.size();
        for (found = 0; found < n; ++found) {
            if (results.get(found).getGraphNo() == no) {
                return true;
            }
        }
        return false;
    }

    public void gotoFound() {
        cursor = found;
        getResult();
    }

    public int nextIndex() {
        return cursor + 1;
    }

    public boolean hasNext() {
        return nextIndex() < results.size();
    }

    public Object next()
            throws NoSuchElementException {
        try {
            cursor = nextIndex();
            return getResult();
        } catch (Exception e) {
            throw new NoSuchElementException();
        }
    }

    public int nextGraphNo() {
        try {
            return results.get(nextIndex()).getGraphNo();
        } catch (Exception e) {
            throw new NoSuchElementException();
        }
    }

    public int previousIndex() {
        return cursor - 1;
    }

    public boolean hasPrevious() {
        return previousIndex() >= 0;
    }

    public Object previous()
            throws NoSuchElementException {
        try {
            cursor = previousIndex();
            return getResult();
        } catch (Exception e) {
            throw new NoSuchElementException();
        }
    }

    public int previousGraphNo() {
        try {
            return results.get(previousIndex()).getGraphNo();
        } catch (Exception e) {
            throw new NoSuchElementException();
        }
    }

    public int highestGraphNo() {
        return highestGraphNo;
    }

    public void gotoHighest() {
        cursor = highestGraphNoIndex;
        getResult();
    }

    /*
    public void remove()
    throws UnsupportedOperationException
    {
    throw new UnsupportedOperationException("CaGe result lists can't be modified");
    }

    public void add(Object o)
    throws UnsupportedOperationException
    {
    throw new UnsupportedOperationException("CaGe result lists can't be modified");
    }

    public void set(Object o)
    throws UnsupportedOperationException
    {
    throw new UnsupportedOperationException("CaGe result lists can't be modified");
    }
     */
}

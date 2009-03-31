package cage;

/**
 * An implementation of <code>ElementRule</code> that always returns the same
 * element.
 *
 * @author nvcleemp
 */
public class SingleElementRule implements ElementRule{

    private final String element;

    /**
     * Create a new <code>SingleElementRule</code> object that always returns
     * <tt>element</tt>.
     *
     * @param element The element which this rule should return.
     */
    public SingleElementRule(String element) {
        this.element = element;
    }

    /**
     * Returns the chemical symbol of the chemical element of the
     * atom which will always be the element set on construction of
     * this rule.
     *
     * @param graph The graph that contains the vertex
     * @param vertex The vertex for which the element is requested
     * @return The symbol of the element of <tt>vertex</tt>
     */
    public String getElement(EmbeddableGraph graph, int vertex) {
        return element;
    }

}

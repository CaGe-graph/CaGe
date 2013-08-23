package cage;

import java.util.Hashtable;

import lisken.systoolbox.MutableInteger;
import lisken.systoolbox.Systoolbox;

/**
 * An implementation of <code>ElementRule</code> that deduces the element of
 * atom based on the degree of the corresponding vertex.
 */
public class ValencyElementRule implements ElementRule {

    //The table containing the elements
    private Hashtable elements = new Hashtable();

    /**
     * Create a new <code>ValencyElementRule</code> object based on
     * <tt>elementRule</tt>. This string is a set of rules seperated by
     * whitespaces. Each rule is formatted as follows: [degree:]element.
     * The degree is optional in this format. If no degree is given, then the
     * element corresponds with the first free degree starting from the last
     * degree for which a rule was added. Only one element per degree is
     * possible and in case more than one element is given for the same degree,
     * only the last one will be remembered.
     *
     * @param elementRule A string containing the rules for this object.
     */
    public ValencyElementRule(String elementRule) {
        String[] rule = Systoolbox.stringToArray(elementRule);
        int n = 0;
        for (int i = 0; i < rule.length; ++i) {
            String entry = rule[i];
            try {
                int p = entry.indexOf(':');
                int k = Integer.parseInt(entry.substring(0, p));
                entry = entry.substring(p + 1);
                if (entry.length() == 0) {
                    throw new Exception();
                }
                n = k;
            } catch (Exception e) {
                while (elements.containsKey(new MutableInteger(++n))) {
                }
            }
            elements.put(new MutableInteger(n), entry);
        }
    }

    /**
     * Returns the chemical symbol of the chemical element of the
     * atom corresponding with <tt>vertex</tt> in <tt>graph</tt>
     * by looking up the element for the degree of <tt>vertex</tt>.
     *
     * @param graph The graph that contains the vertex
     * @param vertex The vertex for which the element is requested
     * @return The symbol of the element of <tt>vertex</tt>
     */
    @Override
    public String getElement(EmbeddableGraph graph, int vertex) {
        int valency = graph.getValency(vertex);
        return (String) elements.get(new MutableInteger(valency));
    }

    static public void main(String[] argv) {
        System.out.println(new ValencyElementRule(argv[0]).elements);
    }
}


package cage.decoration;

/**
 * A <code>Neighbour</code> represent the neighbour of a vertex in a 
 * <code>{@link DecorationGraph}</code>. It combines the identifier of the
 * neighbouring vertex together with the relation between the chamber of the
 * original vertex and the chamber of the neighbouring vertex.
 * 
 * @author nvcleemp
 */
public class Neighbour {
    
    private final int vertex;
    private final FacetType type;

    public Neighbour(int vertex, FacetType type) {
        this.vertex = vertex;
        this.type = type;
    }

    public int getVertex() {
        return vertex;
    }

    public FacetType getType() {
        return type;
    }
    
    
}

package cage.decoration;

/**
 * Class to represent a vertex in a decoration. Each vertex has an id, a position
 * in the decoration, and a type. For INTERNAL vertices only those of type VERTEX
 * need to be specified.
 * 
 * @author nvcleemp
 */
public class DecorationVertex {
    
    private final int id;
    private final VertexPosition position;
    private final FacetType type;

    public DecorationVertex(int id, VertexPosition position, FacetType type) {
        this.id = id;
        this.position = position;
        this.type = type;
    }

    public int getId() {
        return id;
    }

    public VertexPosition getPosition() {
        return position;
    }

    public FacetType getType() {
        return type;
    }
}

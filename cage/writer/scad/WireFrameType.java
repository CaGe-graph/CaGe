package cage.writer.scad;

/**
 * ScadType that outputs the graphs in a wire frame representation.
 * @author nvcleemp
 */
public class WireFrameType extends VertexEdgeType {

    @Override
    protected int getVertexRadius() {
        return 1;
    }

    @Override
    protected int getEdgeRadius() {
        return 1;
    }

    @Override
    public String getName() {
        return "wire frame";
    }
    
}

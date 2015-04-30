package cage.writer.scad;

/**
 * ScadType that outputs the graphs in a ball-stick representation.
 * @author nvcleemp
 */
public class BallStickType extends VertexEdgeType {

    @Override
    protected int getVertexRadius() {
        return 2;
    }

    @Override
    protected int getEdgeRadius() {
        return 1;
    }

    @Override
    public String getName() {
        return "ball-stick";
    }
    
}

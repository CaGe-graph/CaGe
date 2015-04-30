package cage.writer.scad;

/**
 *
 * @author nvcleemp
 */
public abstract class AbstractScadResolution implements ScadResolution{
    
    private final int minAngle;
    private final int minSize;

    public AbstractScadResolution(int minAngle, int minSize) {
        this.minAngle = minAngle;
        this.minSize = minSize;
    }

    @Override
    public ScadType configure(ScadType type) {
        type.setMinimumAngle(minAngle);
        type.setMinimumSize(minSize);
        return type;
    }
    
}

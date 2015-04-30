package cage.writer.scad;

/**
 *
 * @author nvcleemp
 */
public class LowScadResolution extends AbstractScadResolution {

    public LowScadResolution() {
        super(60, 100);
    }

    @Override
    public String getName() {
        return "low";
    }
    
}

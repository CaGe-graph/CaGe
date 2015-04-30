package cage.writer.scad;

/**
 * Interface for the different supported resolutions of some SCAD outputs.
 * 
 * @author nvcleemp
 */
public interface ScadResolution {
    /**
     * Returns the name of this resolution.
     * @return the name of this resolution
     */
    String getName();
    
    /**
     * Configure the given ScadType to use this resolution
     * @param type the ScadType that should be configured
     * @return the configured ScadType (usually the same object as the one supplied)
     */
    ScadType configure(ScadType type);
}

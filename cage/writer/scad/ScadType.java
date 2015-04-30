package cage.writer.scad;

import cage.CaGeResult;

/**
 * Interface for the different types of SCAD outputs.
 * 
 * @author nvcleemp
 */
public interface ScadType {
    /**
     * Returns the name of this type.
     * @return the name of this type
     */
    String getName();
    
    /**
     * Process a graph and return the corresponding SCAD code.
     * @param result the graph that needs to be output
     * @return the SCAD code as a single string
     */
    String processResult(CaGeResult result);
    
    /**
     * Does this type require resolution setting.
     * @return false if this type ignores resolution settings
     */
    boolean hasResolution();
    
    /**
     * Set the minimum angle of a fragment.
     * @param minAngle 
     */
    void setMinimumAngle(int minAngle);
    
    /**
     * Set the minimum size of a fragment.
     * @param minSize the minimum size times 100 
     */
    void setMinimumSize(int minSize);
}

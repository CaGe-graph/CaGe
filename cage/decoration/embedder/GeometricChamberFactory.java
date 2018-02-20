package cage.decoration.embedder;

import cage.decoration.FacetType;
import java.util.Arrays;
import java.util.EnumMap;

/**
 * Factory which can be used to construct a <code>{@link GeometricChamber}</code>.
 * 
 * @author nvcleemp
 */
public class GeometricChamberFactory {
    
    private final EnumMap<FacetType, Reflection> boundingReflections = new EnumMap<>(FacetType.class);
    private final EnumMap<FacetType, double[]> corners = new EnumMap<>(FacetType.class);
    
    public GeometricChamberFactory setBoundingReflectionFor(FacetType type, Reflection reflection){
        boundingReflections.put(type, reflection);
        return this;
    }
    
    public GeometricChamberFactory setCoordinatesFor(FacetType type, double[] coords){
        corners.put(type, Arrays.copyOf(coords, coords.length));
        return this;
    }
    
    /**
     * Constructs the <code>{@link GeometricChamber}</code> using the current
     * state of the factory.
     * 
     * @return 
     */
    public GeometricChamber build(){
        return new GeometricChamber(
                boundingReflections.get(FacetType.VERTEX),
                boundingReflections.get(FacetType.EDGE),
                boundingReflections.get(FacetType.FACE),
                corners.get(FacetType.VERTEX),
                corners.get(FacetType.EDGE),
                corners.get(FacetType.FACE)
        );
    }
}

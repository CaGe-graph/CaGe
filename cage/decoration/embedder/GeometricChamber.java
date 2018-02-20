package cage.decoration.embedder;

import static cage.decoration.DoubleVectorUtility.diff;
import static cage.decoration.DoubleVectorUtility.dotProduct;
import cage.decoration.FacetType;

import java.util.EnumMap;
import java.util.stream.Stream;


/**
 * A chamber in the chamber system of some maximally symmetric tiling.
 * @author nvcleemp
 */
public class GeometricChamber {
    
    private final EnumMap<FacetType, Reflection> boundingReflections = new EnumMap<>(FacetType.class);
    private final EnumMap<FacetType, double[]> corners = new EnumMap<>(FacetType.class);

    /**
     * Creates a chamber with the specified reflections and corners.
     * 
     * @param vMirror
     * @param eMirror
     * @param fMirror
     * @param v
     * @param e
     * @param f 
     */
    GeometricChamber(Reflection vMirror, Reflection eMirror, Reflection fMirror, double[] v, double[] e, double[] f) {
        boundingReflections.put(FacetType.VERTEX, vMirror);
        boundingReflections.put(FacetType.EDGE, eMirror);
        boundingReflections.put(FacetType.FACE, fMirror);
        corners.put(FacetType.VERTEX, v);
        corners.put(FacetType.EDGE, e);
        corners.put(FacetType.FACE, f);
    }
    
    /**
     * Returns the cartesian coordinates for the specified corner.
     * @param type
     * @return 
     */
    public double[] getCoordinatesForCorner(FacetType type){
        return corners.get(type);
    }
    
    /**
     * Returns the reflection opposite of the corner of the specified type.
     * @param type
     * @return 
     */
    public Reflection getReflectionOppositeOf(FacetType type){
        return boundingReflections.get(type);
    }
    
    /**
     * Returns the barycentric coordinates within this chamber corresponding to
     * the specified cartesian coordinates.
     * 
     * @param coords
     * @return 
     */
    public double[] getBarycentricCoordinatesFor(double... coords){
        if(coords.length != 2){
            throw new IllegalArgumentException("Wrong number of coordinates.");
        }
        
        double[] v0 = diff(corners.get(FacetType.EDGE), corners.get(FacetType.VERTEX));
        double[] v1 = diff(corners.get(FacetType.FACE), corners.get(FacetType.VERTEX));
        double[] v2 = diff(coords, corners.get(FacetType.VERTEX));
        
        //express v2 in function of v0 and v1 (i.e. v2 = a*v0 + b*v1)
        double d00 = dotProduct(v0, v0);
        double d01 = dotProduct(v0, v1);
        double d11 = dotProduct(v1, v1);
        double d20 = dotProduct(v2, v0);
        double d21 = dotProduct(v2, v1);
        double denom = d00 * d11 - d01 * d01;
        double a = (d11 * d20 - d01 * d21) / denom;
        double b = (d00 * d21 - d01 * d20) / denom;
        
        return new double[]{1.0 - a - b, a, b};
    }
    
    /**
     * Returns the directional vector corresponding to the specified edge.
     * @param type
     * @return 
     */
    public double[] getVectorForEdge(FacetType type){
        double[][] cornerVectors = Stream.of(FacetType.values()).filter(t->!t.equals(type)).map(corners::get).toArray(double[][]::new);
        return diff(cornerVectors[0], cornerVectors[1]);
    }
    
    /**
     * Returns a chamber corresponding to a chamber in a hexagon tiling.
     * @return 
     */
    public static GeometricChamber hexagonTilingChamber(){
        return new GeometricChamber(
                Reflection.alongYAxis(),
                Reflection.alongLine(-Math.sqrt(3), Math.sqrt(3)/2),
                Reflection.alongXAxis(),
                new double[]{0.5,0},
                new double[]{0,0},
                new double[]{0,Math.sqrt(3)/2}
        );
    }
    
    /**
     * Returns a chamber corresponding to a chamber in a square tiling.
     * @return 
     */
    public static GeometricChamber squareTilingChamber(){
        return new GeometricChamber(
                Reflection.alongYAxis(),
                Reflection.alongLine(-1, 0.5),
                Reflection.alongXAxis(),
                new double[]{0.5,0},
                new double[]{0,0},
                new double[]{0,0.5}
        );
    }
}

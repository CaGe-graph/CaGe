package cage.decoration.viewer;

import cage.decoration.FacetType;
import java.util.EnumMap;
import java.util.Map;

/**
 * This class represent a chamber to which a decoration can be applied for the
 * purpose of drawing it in a <code>{@link DecorationViewer}</code>.
 * 
 * @author nvcleemp
 */
class GeometricChamberCoordinates {
    private final int number;
    private final double[] v;
    private final double[] e;
    private final double[] f;
    
    private final Map<FacetType, GeometricChamberCoordinates> neighbouringChambers = new EnumMap<>(FacetType.class);

    public GeometricChamberCoordinates(int number, double[] v, double[] e, double[] f) {
        this.number = number;
        this.v = v;
        this.e = e;
        this.f = f;
    }

    public int getNumber() {
        return number;
    }

    public double[] getV() {
        return v;
    }

    public double[] getE() {
        return e;
    }

    public double[] getF() {
        return f;
    }
    
    public void setNeighbouringChamber(FacetType type, GeometricChamberCoordinates chamber){
        neighbouringChambers.put(type, chamber);
    }
    
    public GeometricChamberCoordinates getNeighbouringChamber(FacetType type){
        return neighbouringChambers.get(type);
    }

    @Override
    public String toString() {
        return "GeometricChamberCoordinates{" + number + ", v=" + v + ", e=" + e + ", f=" + f + '}';
    }
}

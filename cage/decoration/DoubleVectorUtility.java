package cage.decoration;

/**
 * Several static utility function for vectors of doubles.
 * 
 * @author nvcleemp
 */
public class DoubleVectorUtility {

    //should not be instantiated
    private DoubleVectorUtility() {
    }
    
    public static double dotProduct(double[] a, double[] b){
        if(a.length != b.length){
            throw new IllegalArgumentException("Vectors should have same length");
        }
        double sum = 0.0;
        for (int i = 0; i < a.length; i++) {
            sum += a[i]*b[i];
        }
        return sum;
    }
    
    public static double[] scalarProduct(double[] a, double s){
        double[] product = new double[a.length];
        for (int i = 0; i < a.length; i++) {
            product[i] = s*a[i];
        }
        return product;
    }
    
    public static double[] sum(double[] a, double[] b){
        if(a.length != b.length){
            throw new IllegalArgumentException("Vectors should have same length");
        }
        double[] sum = new double[a.length];
        for (int i = 0; i < a.length; i++) {
            sum[i] = a[i] + b[i];
        }
        return sum;
    }
    
    public static double[] diff(double[] a, double[] b){
        if(a.length != b.length){
            throw new IllegalArgumentException("Vectors should have same length");
        }
        double[] diff = new double[a.length];
        for (int i = 0; i < a.length; i++) {
            diff[i] = a[i] - b[i];
        }
        return diff;
    }
}

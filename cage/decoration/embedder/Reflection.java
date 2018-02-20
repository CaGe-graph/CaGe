package cage.decoration.embedder;

import java.io.PrintStream;
import java.util.Arrays;

/**
 * A 2D reflection.
 * 
 * @author nvcleemp
 */
public class Reflection {
    
    public static Reflection alongXAxis(){
        return new Reflection(new double[][] {
            {1, 0, 0},
            {0,-1, 0}
        });
    }
    
    public static Reflection alongYAxis(){
        return new Reflection(new double[][] {
            {-1, 0, 0},
            { 0, 1, 0}
        });
    }
    
    public static Reflection alongLine(double slope, double intercept){
        //just some shorter names
        double m = slope;
        double b = intercept;
        
        double denom = (1+m*m);
        return new Reflection(new double[][] {
            {(1-m*m)/denom, 2*m/denom,       -2*m*b/denom},
            {2*m/denom,     (m*m - 1)/denom, 2*b/denom}
        });
    }
    
    private final double[][] matrix;

    public Reflection(double[][] matrix) {
        if(matrix.length!=2){
            throw new IllegalArgumentException("Reflection requires a 2x3 matrix.");
        }
        this.matrix = new double[2][];
        for (int i = 0; i < 2; i++) {
            if(matrix[i].length!=3){
                throw new IllegalArgumentException("Reflection requires a 2x3 matrix.");
            }
            this.matrix[i] = Arrays.copyOf(matrix[i], matrix[i].length);
        }
    }
    
    public double[] apply(double... coordinates){
        if(coordinates.length != 2){
            throw new IllegalArgumentException("apply requires exactly two coordinates");
        }
        return new double[]{
            coordinates[0]*matrix[0][0] + coordinates[1]*matrix[0][1] + matrix[0][2],
            coordinates[0]*matrix[1][0] + coordinates[1]*matrix[1][1] + matrix[1][2],
        };
    }
    
    public void printAsMatrix(PrintStream out){
        final String format = "% 2.5f, % 2.5f, % 2.5f%n";
        for (int i = 0; i < matrix.length; i++) {
            out.format(format, matrix[i][0], matrix[i][1], matrix[i][2]);
        }
        out.format(format, 0.0, 0.0, 1.0);
    }
}

package cage.utility;

/**
 * Simple extension of Throwable that can be used to print the stack trace
 * at the position it is created.
 */
public class StackTrace extends Throwable {

    public StackTrace(String message) {
        super(message);
    }
    
}

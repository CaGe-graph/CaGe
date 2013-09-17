package cage.utility;

import cage.CaGe;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Static class to facilitate CaGe Debug Mode.
 *
 * @author nvcleemp
 */
public class Debug {
    public static void print(String message){
        if(CaGe.debugMode){
            try {
                StackTraceElement ste = new StackTrace("").getStackTrace()[1];
                System.err.println("[" + ste + "] " + message);
            } catch (Exception e) {
                System.err.println("[UNKNOWN POSITION] " + message);
            }
        }
    }

    @SuppressWarnings("CallToThreadDumpStack")
    public static void reportException(Throwable e){
        Logger.getLogger(Debug.class.getName()).log(Level.CONFIG, "Exception reported", e);
        if(CaGe.debugMode){
            e.printStackTrace();
        }
    }
}

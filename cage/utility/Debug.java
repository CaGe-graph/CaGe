package cage.utility;

import cage.CaGe;

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
        if(CaGe.debugMode){
            e.printStackTrace();
        }
    }
}

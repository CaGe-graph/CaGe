package lisken.systoolbox;

import java.io.PrintStream;
import java.io.PrintWriter;
import java.util.Enumeration;
import java.util.Vector;

public class ExceptionGroup extends Exception {

    private static final String EMPTY_MSG = "empty ExceptionGroup";
    Vector exceptionV = new Vector();

    public void add(Exception ex) {
        exceptionV.addElement(ex);
    }

    @Override
    public String getMessage() {
        if (exceptionV.size() > 0) {
            StringBuilder messages = new StringBuilder();
            Enumeration exceptions = exceptionV.elements();
            int e = 0;
            while (exceptions.hasMoreElements()) {
                if (e++ > 0) {
                    messages.append(", ");
                }
                messages.append(e).append(". ");
                messages.append(((Exception) exceptions.nextElement()).getMessage());
            }
            return messages.toString();
        } else {
            return EMPTY_MSG;
        }
    }

    @Override
    public String toString() {
        if (exceptionV.size() > 0) {
            StringBuilder encodings = new StringBuilder();
            Enumeration exceptions = exceptionV.elements();
            int e = 0;
            while (exceptions.hasMoreElements()) {
                if (e++ > 0) {
                    encodings.append("\n\n");
                }
                encodings.append(e).append(". ");
                encodings.append(((Exception) exceptions.nextElement()).toString());
            }
            return encodings.toString();
        } else {
            return EMPTY_MSG;
        }
    }

    @Override
    public void printStackTrace() {
        printStackTrace(System.err);
    }

    @Override
    public void printStackTrace(PrintStream s) {
        if (exceptionV.size() > 0) {
            Enumeration exceptions = exceptionV.elements();
            int e = 0;
            while (exceptions.hasMoreElements()) {
                if (e++ > 0) {
                    s.print("\n");
                }
                ((Exception) exceptions.nextElement()).printStackTrace(s);
            }
        } else {
            s.println(EMPTY_MSG);
        }
    }

    @Override
    public void printStackTrace(PrintWriter s) {
        if (exceptionV.size() > 0) {
            Enumeration exceptions = exceptionV.elements();
            int e = 0;
            while (exceptions.hasMoreElements()) {
                if (e++ > 0) {
                    s.print("\n");
                }
                ((Exception) exceptions.nextElement()).printStackTrace(s);
            }
        } else {
            s.println(EMPTY_MSG);
        }
    }

    @Override
    public Throwable fillInStackTrace() {
        if (exceptionV == null) {
            return null;
        }
        Enumeration exceptions = exceptionV.elements();
        for (int i = 0; i < exceptionV.size(); ++i) {
            exceptionV.setElementAt(
                    ((Exception) exceptionV.elementAt(i)).fillInStackTrace(),
                    i);
        }
        return this;
    }

    public int size() {
        return exceptionV.size();
    }
}


package lisken.systoolbox;

import java.io.PrintStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;

public class ExceptionGroup extends Exception {

    private static final String EMPTY_MSG = "empty ExceptionGroup";
    private List<Exception> exceptionV = new ArrayList<>();

    public void add(Exception ex) {
        exceptionV.add(ex);
    }

    @Override
    public String getMessage() {
        if (exceptionV.size() > 0) {
            StringBuilder messages = new StringBuilder();
            ListIterator<Exception> exceptions = exceptionV.listIterator();
            int e = 0;
            while (exceptions.hasNext()) {
                if (e++ > 0) {
                    messages.append(", ");
                }
                messages.append(e).append(". ");
                messages.append(exceptions.next().getMessage());
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
            ListIterator<Exception> exceptions = exceptionV.listIterator();
            int e = 0;
            while (exceptions.hasNext()) {
                if (e++ > 0) {
                    encodings.append("\n\n");
                }
                encodings.append(e).append(". ");
                encodings.append(exceptions.next().toString());
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
            ListIterator<Exception> exceptions = exceptionV.listIterator();
            int e = 0;
            while (exceptions.hasNext()) {
                if (e++ > 0) {
                    s.print("\n");
                }
                exceptions.next().printStackTrace(s);
            }
        } else {
            s.println(EMPTY_MSG);
        }
    }

    @Override
    public void printStackTrace(PrintWriter s) {
        if (exceptionV.size() > 0) {
            ListIterator<Exception> exceptions = exceptionV.listIterator();
            int e = 0;
            while (exceptions.hasNext()) {
                if (e++ > 0) {
                    s.print("\n");
                }
                exceptions.next().printStackTrace(s);
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
        for (int i = 0; i < exceptionV.size(); ++i) {
            exceptionV.set(i, (Exception)(exceptionV.get(i).fillInStackTrace()));
        }
        return this;
    }

    public int size() {
        return exceptionV.size();
    }
}


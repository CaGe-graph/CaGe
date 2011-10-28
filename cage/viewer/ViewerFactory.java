package cage.viewer;

import cage.CaGe;
import cage.utility.Debug;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.Hashtable;

public class ViewerFactory {

    private static Hashtable viewerClasses = new Hashtable();
    private static Hashtable viewers = new Hashtable();
    private static String lastErrorMessage;

    public static CaGeViewer getCaGeViewer(String viewerName, int dimension) {
        CaGeViewer result = null;
        try {
            result = (CaGeViewer) viewers.get(viewerName);
            result.setDimension(dimension);
            return result;
        } catch (Exception ex) {
            Debug.reportException(ex);
            result = null;
        }
        try {
            Class viewerClass = getViewerClass(viewerName);
            checkAvailability(viewerClass, dimension);
            result = (CaGeViewer) viewerClass.newInstance();
            result.setDimension(dimension);
            viewers.put(viewerName, result);
        } catch (Throwable ex) {
            Debug.reportException(ex);
            if (ex instanceof ExceptionInInitializerError) {
                lastErrorMessage =
                        ((ExceptionInInitializerError) ex).getException().getMessage();
            } else {
                lastErrorMessage = ex.getMessage();
            }
            result = null;
        }
        return result;
    }

    /**
     * Returns the last error message created in the methode
     * {@link #getCaGeViewer(java.lang.String, int)}.
     * @return the last error message created in the methode
     * {@link #getCaGeViewer(java.lang.String, int)}.
     */
    public static String lastErrorMessage() {
        return lastErrorMessage;
    }

    /**
     * Checks whether the given class of viewers is available for the given
     * dimension. If this class of viewers is not available for the given
     * dimension, this method will throw an exception.
     *
     * @param viewerClass A class of viewers
     * @param dimension A dimension
     * @throws java.lang.Exception If this class of viewers is not available for the given
     * dimension.
     */
    private static void checkAvailability(Class viewerClass, int dimension)
            throws Exception {
        //TODO: this looks like the misuse of an Exception. This method should return a boolean
        //also: is there no way to get rid of this reflection?
        Method availabilityMethod = null;
        try {
            availabilityMethod = viewerClass.getMethod(
                    "isAvailable", new Class[]{Integer.TYPE});
        } catch (NoSuchMethodException ex1) {
            Debug.reportException(ex1);
            return;
        } catch (SecurityException ex2) {
            Debug.reportException(ex2);
            return;
        }
        if (!availabilityMethod.getReturnType().equals(Boolean.TYPE)) {
            throw new Exception();
        }
        if (!Modifier.isStatic(availabilityMethod.getModifiers())) {
            throw new Exception();
        }
        Boolean result = (Boolean) availabilityMethod.invoke(null,
                new Object[]{new Integer(dimension)});
        if (!result.booleanValue()) {
            throw new Exception();
        }
    }

    /**
     * Returns whether the given type of viewer is available for the given
     * dimension.
     *
     * @param viewerName A type of viewer
     * @param dimension A dimension
     * @return <tt>true</tt> if this type is available for the given dimension
     */
    public static boolean checkAvailability(String viewerName, int dimension) {
        try {
            checkAvailability(getViewerClass(viewerName), dimension);
            return true;
        } catch (Exception ex) {
            Debug.reportException(ex);
            return false;
        }
    }

    /**
     * Returns the <code>Class</code> for a given type of viewer.
     *
     * @param viewerName The name of a type of viewer
     * @return the <code>Class</code> for <tt>viewerName</tt>
     * @throws java.lang.Exception If the <code>Class</tt> for this type
     * cannot be found.
     */
    private static Class getViewerClass(String viewerName)
            throws Exception {
        Class viewerClass;
        viewerClass = (Class) viewerClasses.get(viewerName);
        if (viewerClass == null) {
            String viewerClassName = CaGe.config.getProperty(viewerName + ".Class");
            viewerClass = Class.forName(viewerClassName);
            viewerClasses.put(viewerName, viewerClass);
        }
        return viewerClass;
    }
}


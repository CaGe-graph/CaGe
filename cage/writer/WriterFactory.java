package cage.writer;

import cage.CaGe;

import java.util.Hashtable;

/**
 * Utility class for the creation of {@link CaGeWriter} objects.
 */
public class WriterFactory {

    /** <code>CaGeWriter</code>s are cached with this table */
    private static Hashtable writerClasses = new Hashtable();

    //this class shouldn't be instantiated.
    private WriterFactory(){
    }

    /**
     * Returns a <code>CaGeWriter</code> based on the given
     * <tt>format</tt>. To determine the writer this methods goes
     * through the following steps:
     * <ol>
     * <li>First it is checked if there is cached value for
     *     the format.</li>
     * <li>If {@link CaGe#nativesAvailable} is <tt>true</tt>, then
     *     it tries to find a class named <tt>Native</tt> + <tt>
     *     format</tt> + <tt>Writer</tt> in this package.</li>
     * <li>If this fails, it looks for a class named <tt>format</tt>
     *     + <tt>Writer</tt> in this package.</li>
     * <li>If it still didn't find a writer, it interprets the format
     *     as a full class name (including package) and tries to load
     *     it.</li>
     * </ol>
     * If all these attempts fail, this method gives up and returns
     * <tt>null</tt>. Otherwise it caches the class and returns a new
     * instance of the writer.
     *
     * @param format A String describing the format for which you want
     *               a writer, or the full name of a writer.
     * @return A new instance of the writer corresponding to the given
     *         format or <tt>null</tt> if no writer class could be found
     *         for that format.
     */
    public static CaGeWriter createCaGeWriter(String format) {
        Class writerClass;
        CaGeWriter result = null;
        writerClass = (Class) writerClasses.get(format);
        if (writerClass != null) {
            try {
                result = (CaGeWriter) writerClass.newInstance();
                return result;
            } catch (Exception ex) {
                writerClasses.remove(format);
            }
        }
        String className, packageName;
        className = format + "Writer";
        packageName = WriterFactory.class.getName();
        packageName = packageName.substring(0, packageName.lastIndexOf('.') + 1);
        if (CaGe.nativesAvailable) {
            try {
                writerClass = Class.forName(packageName + "Native" + className);
                result = (CaGeWriter) writerClass.newInstance();
            } catch (Exception ex) {
            }
        }
        if (result == null) {
            try {
                writerClass = Class.forName(packageName + className);
                result = (CaGeWriter) writerClass.newInstance();
            } catch (Exception ex) {
            }
        }
        if (result == null) {
            try {
                writerClass = Class.forName(format);
                result = (CaGeWriter) writerClass.newInstance();
            } catch (Exception ex) {
            }
        }
        if (result != null) {
            writerClasses.put(format, writerClass);
        }
        return result;
    }
}


package util;

/**
 * Utility class for returning a processed system property. The properties are
 * processed as follows: First all the points in this property are replaced
 * with dashes and then the substring from the start to the first character
 * that is not a letter, a digit, a dash or an underscore is taken. If we take,
 * for instance, the system property <i>os.name</i> on Mac OS X, this will be
 * <tt>Mac OS X</tt>, but the method <code>get</code> will return <tt>Mac</tt>.
 * <p>
 * This class is not only used by the CaGe program but also in the installation
 * script. This package is packed into a separate jar named <tt>sysinfo.jar
 * </tt>.
 */
public class SysInfo {

    private SysInfo(){
        //should not be instantiated
    }

    /**
     * Returns the system property with the name <tt>propertyName</tt>. First
     * all the points in this property are replaced with dashes and then the
     * substring from the start to the first character that is not a letter, a
     * digit, a dash or an underscore is returned, unless this string  has
     * length 0 in which case <tt>null</tt> is returned.
     *
     * @param propertyName The name of the system property that should be
     *                     returned
     * @return The processed system property for the given name
     */
    public static String get(String propertyName) {
        String sysInfo = System.getProperty(propertyName);
        if(sysInfo==null)
            return null;
        sysInfo = sysInfo.replace('.', '-');
        int n = sysInfo.length(), i;
        for (i = 0; i < n; ++i) {
            char c = sysInfo.charAt(i);
            if (c != '-' && c != '_' && !Character.isLetterOrDigit(c)) {
                break;
            }
        }
        if (i > 0) {
            return sysInfo.substring(0, i);
        }
        return null;
    }

    /**
     * Main method that allows this class to be run from the command line.
     * It requires at least one argument, and will fail when this is not the case.
     * The system property with the name equal to the first argument will be
     * printed to standard out if such a property exists, otherwise nothing will
     * happen.
     *
     * @see #get(java.lang.String)
     * @param argv An array containing at least one element.
     */
    public static void main(String[] argv) {
        String sysInfo = get(argv[0]);
        if (sysInfo != null) {
            System.out.print(sysInfo + "\n");
        }
    }
}
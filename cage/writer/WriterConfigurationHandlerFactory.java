package cage.writer;

import cage.CaGe;

import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Utility class for the creation of {@link WriterConfigurationHandler} objects.
 */
public class WriterConfigurationHandlerFactory {

    /** <code>WriterConfigurationHandler</code>s are cached with this map */
    private static final Map<String, Class> handlerClasses = new HashMap<>();

    //this class shouldn't be instantiated.
    private WriterConfigurationHandlerFactory() {
    }

    /**
     * Returns a <code>WriterConfigurationHandler</code> based on the given
     * <tt>format</tt>. To determine the writer this methods goes
     * through the following steps:
     * <ol>
     * <li>First it is checked if there is cached value for
     *     the format.</li>
     * <li>If {@link CaGe#nativesAvailable} is <tt>true</tt>, then
     *     it tries to find a class named <tt>Native</tt> + <tt>
     *     format</tt> + <tt>WriterConfigurationHandler</tt> in this package.</li>
     * <li>If this fails, it looks for a class named <tt>format</tt>
     *     + <tt>WriterConfigurationHandler</tt> in this package.</li>
     * <li>If it still didn't find a writer, it interprets the format
     *     as a full class name for the writer (including package) and
     *     tries to load the handler by appending <tt>ConfigurationHandler</tt>,
     *     and then loading that class.</li>
     * </ol>
     * If all these attempts fail, this method gives up and returns
     * <tt>null</tt>. Otherwise it caches the class and returns a new
     * instance of the writer.
     *
     * @param format A String describing the format for which you want
     *               a writer configuration handler, or the full name of a
     *               writer configuration handler.
     * @return A new instance of the writer configuration handler corresponding
     *         to the given format or <tt>null</tt> if no writer configuration
     *         handler class could be found for that format.
     */
    public static WriterConfigurationHandler createWriterConfigurationHandler(String format) {
        WriterConfigurationHandler result = null;
        Class handlerClass = handlerClasses.get(format);
        if (handlerClass != null) {
            try {
                result = (WriterConfigurationHandler) handlerClass.newInstance();
                Logger.getLogger(WriterConfigurationHandlerFactory.class.getName())
                        .log(
                            Level.INFO,
                            "Loaded configuration handler: {0}", 
                            result.getClass().getName());
                return result;
            } catch (InstantiationException | IllegalAccessException ex) {
                Logger.getLogger(WriterConfigurationHandlerFactory.class.getName())
                        .log(
                            Level.INFO,
                            "Failed attempt while loading configuration handler",
                            ex);
                handlerClasses.remove(format);
            }
        }
        String className, packageName;
        className = format + "WriterConfigurationHandler";
        packageName = WriterConfigurationHandlerFactory.class.getName();
        packageName = packageName.substring(0, packageName.lastIndexOf('.') + 1);
        if (CaGe.nativesAvailable) {
            try {
                handlerClass = Class.forName(packageName + "Native" + className);
                result = (WriterConfigurationHandler) handlerClass.newInstance();
            } catch (ClassNotFoundException | InstantiationException | IllegalAccessException ex) {
                Logger.getLogger(WriterConfigurationHandlerFactory.class.getName())
                        .log(
                            Level.INFO,
                            "Failed attempt while loading configuration handler",
                            ex);
            }
        }
        if (result == null) {
            try {
                handlerClass = Class.forName(packageName + className);
                result = (WriterConfigurationHandler) handlerClass.newInstance();
            } catch (ClassNotFoundException | InstantiationException | IllegalAccessException ex) {
                Logger.getLogger(WriterConfigurationHandlerFactory.class.getName())
                        .log(
                            Level.INFO,
                            "Failed attempt while loading configuration handler",
                            ex);
            }
        }
        if (result == null) {
            try {
                handlerClass = Class.forName(format);
                result = (WriterConfigurationHandler) handlerClass.newInstance();
            } catch (ClassNotFoundException | InstantiationException | IllegalAccessException ex) {
                Logger.getLogger(WriterConfigurationHandlerFactory.class.getName())
                        .log(
                            Level.INFO,
                            "Failed attempt while loading configuration handler",
                            ex);
            }
        }
        if (result != null) {
            Logger.getLogger(WriterConfigurationHandlerFactory.class.getName())
                    .log(
                        Level.INFO,
                        "Loaded configuration handler: {0}", 
                        result.getClass().getName());
            handlerClasses.put(format, handlerClass);
        } else {
            Logger.getLogger(WriterConfigurationHandlerFactory.class.getName())
                    .log(
                        Level.INFO, 
                        "No configuration handler loaded for format ''{0}''",
                        format);
        }
        return result;
    }
}


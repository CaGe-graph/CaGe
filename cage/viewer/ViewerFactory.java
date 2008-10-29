
package cage.viewer;

import cage.CaGe;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.Hashtable;


public class ViewerFactory
{
  private static Hashtable viewerClasses = new Hashtable();
  private static Hashtable viewers = new Hashtable();
  private static String lastErrorMessage;

  public static CaGeViewer getCaGeViewer(String viewerName, int dimension)
  {
    CaGeViewer result = null;
    try {
      result = (CaGeViewer) viewers.get(viewerName);
      result.setDimension(dimension);
      return result;
    } catch (Exception ex) {
      result = null;
    }
    try {
      Class viewerClass = getViewerClass(viewerName);
      checkAvailability(viewerClass, dimension);
      result = (CaGeViewer) viewerClass.newInstance();
      result.setDimension(dimension);
      viewers.put(viewerName, result);
    } catch (Throwable ex) {
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

  public static String lastErrorMessage()
  {
    return lastErrorMessage;
  }

  static void checkAvailability(Class viewerClass, int dimension)
   throws Exception
  {
    Method availabilityMethod = null;
    try {
      availabilityMethod = viewerClass.getMethod(
       "isAvailable", new Class[] { Integer.TYPE });
    } catch (NoSuchMethodException ex1) {
      return;
    } catch (SecurityException ex2) {
      return;
    }
    if (! availabilityMethod.getReturnType().equals(Boolean.TYPE))
      throw new Exception();
    if (! Modifier.isStatic(availabilityMethod.getModifiers()))
      throw new Exception();
    Boolean result = (Boolean) availabilityMethod.invoke(null,
     new Object[] { new Integer(dimension) });
    if (! result.booleanValue())
      throw new Exception();
  }

  public static boolean checkAvailability(String viewerName, int dimension)
  {
    try {
      checkAvailability(getViewerClass(viewerName), dimension);
      return true;
    } catch (Exception ex) {
      return false;
    }
  }

  static Class getViewerClass(String viewerName)
   throws Exception
  {
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



package cage.writer;


import java.util.*;
import cage.*;


public class WriterFactory
{
  private static Hashtable writerClasses = new Hashtable();

  public static CaGeWriter createCaGeWriter(String format)
  {
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


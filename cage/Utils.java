
package cage;

import java.util.Vector;
import javax.swing.AbstractButton;


public class Utils
{
  public static void addIfSelected(Vector vector, AbstractButton button, String string)
  {
    addIfSelected(vector, button, new String[] { string });
  }
  public static void addIfSelected(Vector vector, AbstractButton button, String[] strings)
  {
    if (button.isSelected()) {
      for (int i = 0; i < strings.length; ++i)
      {
        vector.addElement(strings[i]);
      }
    }
  }
}



package cage;

import java.beans.PropertyChangeListener;


public interface GeneratorListener extends PropertyChangeListener
{
  void showException(Exception e, String context);
}


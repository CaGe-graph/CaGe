
package cage;


import java.beans.*;


public interface GeneratorListener extends PropertyChangeListener
{
  void showException(Exception e, String context);
}


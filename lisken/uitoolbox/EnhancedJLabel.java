
package lisken.uitoolbox;


import java.awt.*;
import java.awt.event.*;
import javax.swing.*;


public class EnhancedJLabel extends JLabel implements java.io.Serializable // , FocusListener
{
  public EnhancedJLabel(String text, Icon icon, int horizontalAlignment)
  {
    super(text, icon, horizontalAlignment);
//    addFocusListener(this);
  }
  public EnhancedJLabel(String text, int horizontalAlignment)
  {
    super(text, horizontalAlignment);
//    addFocusListener(this);
  }
  public EnhancedJLabel(String text)
  {
    super(text);
//    addFocusListener(this);
  }
  public EnhancedJLabel(Icon image, int horizontalAlignment)
  {
    super(image, horizontalAlignment);
//    addFocusListener(this);
  }
  public EnhancedJLabel(Icon image)
  {
    super(image);
//    addFocusListener(this);
  }
  public EnhancedJLabel()
  {
    super();
//    addFocusListener(this);
  }

/*
  public void focusGained(FocusEvent e)
  {
    Component c = super.getLabelFor();
    if (c != null) {
      c.requestFocus();
    }
  }
  public void focusLost(FocusEvent e)
  { }
*/

  public boolean isFocusTraversable()
  { return false; }
}


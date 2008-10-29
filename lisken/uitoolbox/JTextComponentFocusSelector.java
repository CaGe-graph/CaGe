
package lisken.uitoolbox;

import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.InputEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.text.JTextComponent;


public class JTextComponentFocusSelector extends MouseAdapter implements FocusListener
{
  private boolean dontSelect = false;
  public void focusGained(FocusEvent e)
  {
    if (e.isTemporary()) {
      return;
    } else if (dontSelect) {
      dontSelect = false;
      return;
    }
    ((JTextComponent) e.getSource()).selectAll();
  }
  public void focusLost(FocusEvent e)
  {
    if (e.isTemporary()) return;
    JTextComponent tc = (JTextComponent) e.getSource();
    try {
      tc.setCaretPosition(tc.getCaretPosition());
    } catch (IllegalArgumentException ex) {
    }
  }
  public void mousePressed(MouseEvent e)
  {
    if ((e.getModifiers() & InputEvent.BUTTON1_MASK) == 0) {
      return;
    } else if (((JTextComponent) e.getSource()).hasFocus()) {
      return;
    }
    dontSelect = true;
  }
  public void setField(JTextComponent tc)
  {
    tc.addFocusListener((FocusListener) this);
    tc.addMouseListener(this);
  }

  public JTextComponentFocusSelector(JTextComponent tc)
  {
    this.setField(tc);
  }
}



package lisken.uitoolbox;


import java.awt.*;
import java.awt.event.*;
import javax.swing.*;


public class FlaggedJDialog extends JDialog implements ActionListener
{
  private boolean success = false;
  protected AbstractButton cancelButton = null;
  protected Component nearComponent = null;

  public FlaggedJDialog(Frame owner, String title, boolean modal)
  {
    super(owner, title, modal);
    getRootPane().registerKeyboardAction(this,
     KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0),
     JComponent.WHEN_IN_FOCUSED_WINDOW);
  }

  public void setDefaultButton(JButton defaultButton)
  {
    defaultButton.addActionListener(this);
    getRootPane().setDefaultButton(defaultButton);
  }

  public AbstractButton getDefaultButton()
  {
    return getRootPane().getDefaultButton();
  }

  public void setCancelButton(AbstractButton cancelButton)
  {
    cancelButton.addActionListener(this);
    this.cancelButton = cancelButton;
  }

  public void actionPerformed(ActionEvent e)
  {
    Object source = e.getSource();
    if (source instanceof JRootPane) {
      if (cancelButton != null) cancelButton.doClick();
    } else {
      success = source != cancelButton;
      setVisible(false);
    }
  }

  public void setSuccess(boolean success)
  {
    this.success = success;
  }

  public boolean getSuccess()
  {
    return success;
  }

  public void setNearComponent(Component nearComponent)
  {
    this.nearComponent = nearComponent;
  }

  public void setVisible(boolean visible)
  {
    if (visible && ! isVisible()) {
      UItoolbox.moveComponentNearComponent(this, nearComponent);
    }
    super.setVisible(visible);
  }
}


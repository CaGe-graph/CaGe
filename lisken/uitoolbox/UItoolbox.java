
package lisken.uitoolbox;

import java.awt.Component;
import java.awt.Dialog;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Frame;
import java.awt.Point;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.util.Dictionary;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSlider;
import javax.swing.JTextArea;
import javax.swing.KeyStroke;
import javax.swing.RootPaneContainer;
import javax.swing.ScrollPaneLayout;
import javax.swing.SwingUtilities;


public class UItoolbox
{
  public static void addExitOnEscape(RootPaneContainer container)
  {
    container.getRootPane().registerKeyboardAction(new ActionListener() {
        public void actionPerformed(ActionEvent actionEvent) {
          System.exit(0);
        }
      },
      KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0), JComponent.WHEN_IN_FOCUSED_WINDOW
    );
  }

  public static void showTextInfo(String title, String info)
  {
    showTextInfo(title, info, false, null);
  }

  public static void showTextInfo(String title, String info, boolean monospaced)
  {
    showTextInfo(title, info, monospaced, null);
  }

  public static void showTextInfo
   (String title, String info, Component nearComponent)
  {
    showTextInfo(title, info, false, nearComponent);
  }

  public static void showTextInfo
   (String title, String info, boolean monospaced, Component nearComponent)
  {
    final JDialog dialog = new JDialog((Frame) null, title, true);
    ActionListener endDialog = new ActionListener() {
      public void actionPerformed(ActionEvent e)
      {
        dialog.dispose();
      }
    };
    JTextArea infoArea = new AutosizedTextArea();
    infoArea.setEditable(false);
    infoArea.setText(info);
    infoArea.setCaretPosition(0);
    if (monospaced) {
      infoArea.setFont(new Font("Monospaced", 0, infoArea.getFont().getSize()));
    }
    infoArea.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    JScrollPane infoPane = new JScrollPane(infoArea);
    restrictScrollPaneToScreenPart(infoPane, 0.8f, 0.8f);
    JPanel content = new JPanel();
    dialog.setContentPane(content);
    content.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
    content.setLayout(new BoxLayout(content, BoxLayout.Y_AXIS));
    content.add(infoPane);
    content.add(Box.createVerticalStrut(10));
    JButton okButton = new JButton("Ok");
    okButton.setAlignmentX(0.5f);
    okButton.addActionListener(endDialog);
    content.add(okButton);
    dialog.getRootPane().setDefaultButton(okButton);
    dialog.getRootPane().registerKeyboardAction(endDialog, null,
     KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0),
     JComponent.WHEN_IN_FOCUSED_WINDOW);
    dialog.pack();
    okButton.requestFocus();
    moveComponentNearComponent(dialog, nearComponent);
    dialog.setVisible(true);
  }

  public static void moveComponentNearComponent
   (Component component, Component nearComponent)
  {
    if (nearComponent != null) {
      Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
      Dimension componentSize = component.getSize();
      Point location = nearComponent.getLocationOnScreen();
      location.x -= 100;
      location.y -= 50;
      location.x = Math.min(location.x,
       screenSize.width - componentSize.width);
      location.y = Math.min(location.y,
       screenSize.height - componentSize.height);
      component.setLocation(location);
    }
  }

  public static void centerOnScreen(Component component)
  {
    Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
    Dimension componentSize = component.getSize();
    if (componentSize.height > screenSize.height)
      componentSize.height = screenSize.height;
    if (componentSize.width > screenSize.width)
      componentSize.width = screenSize.width;
    component.setLocation(
     (screenSize.width - componentSize.width) / 2,
     (screenSize.height - componentSize.height) / 2);
  }

  public static void setmaxToPref(JComponent component)
  {
    component.setMaximumSize(component.getPreferredSize());
  }

  public static void pack(Component c)
  {
    while (c != null)
    {
      if (c instanceof Window) {
        ((Window) c).pack();
        break;
      }
      c = c.getParent();
    }
  }

  public static Dimension restrictSizeToScreenPart
   (Dimension originalSize, float widthPart, float heightPart)
  {
    Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
    originalSize.width = Math.min(originalSize.width,
     (int) (screenSize.width * widthPart + 0.5f));
    originalSize.height = Math.min(originalSize.height,
     (int) (screenSize.height * heightPart + 0.5f));
    return originalSize;
  }

  public static Dimension restrictSizeToScreenPart
   (Component component, float widthPart, float heightPart)
  {
    return restrictSizeToScreenPart
     (component.getSize(), widthPart, heightPart);
  }

  public static void restrictScrollPaneToScreenPart
   (JScrollPane scrollPane, float widthPart, float heightPart)
  {
    Dimension maxSize = Toolkit.getDefaultToolkit().getScreenSize();
    maxSize.width = (int) (maxSize.width * widthPart + 0.5f);
    maxSize.height = (int) (maxSize.height * heightPart + 0.5f);
    Dimension scrollPaneSize = scrollPane.getPreferredSize();
    Dimension barSize;
    ScrollPaneLayout layout;
    if (scrollPaneSize.width > maxSize.width
	&& scrollPaneSize.height > maxSize.height) {
      scrollPaneSize = maxSize;
    } else if (scrollPaneSize.width > maxSize.width) {
      layout = (ScrollPaneLayout) scrollPane.getLayout();
      barSize = layout.getHorizontalScrollBar().getPreferredSize();
      scrollPaneSize.width = maxSize.width;
      scrollPaneSize.height =
       Math.min(scrollPaneSize.height + barSize.height, maxSize.height);
    } else if (scrollPaneSize.height > maxSize.height) {
      layout = (ScrollPaneLayout) scrollPane.getLayout();
      barSize = layout.getVerticalScrollBar().getPreferredSize();
      scrollPaneSize.height = maxSize.height;
      scrollPaneSize.width =
       Math.min(scrollPaneSize.width + barSize.width, maxSize.width);
    }
    scrollPane.setPreferredSize(scrollPaneSize);
  }

  public static void addLabel(JSlider slider, int value)
  {
    Dictionary table = slider.getLabelTable();
    table.put(new Integer(value), new JLabel(Integer.toString(value)));
    slider.setLabelTable(table);
  }

  public static void invokeAndWait(final Runnable runner)
  {
    if (SwingUtilities.isEventDispatchThread()) {
      SwingUtilities.invokeLater(runner);
    } else {
      try {
	SwingUtilities.invokeAndWait(runner);
      } catch (java.lang.reflect.InvocationTargetException e) {
        e.getTargetException().printStackTrace();
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }
  }

  public static String getTitle(Window window)
  {
    if (window instanceof Dialog) {
      return ((Dialog) window).getTitle();
    } else if (window instanceof Frame) {
      return ((Frame) window).getTitle();
    } else {
      return null;
    }
  }

  public static void focusWindow(final Window window)
  {
    SwingUtilities.invokeLater(new Runnable() {
      public void run()
      {
        window.toFront();
	// focusSomeComponentIn(window);
      }
    });
  }

/*
  static boolean focusSomeComponentIn(Container container)
  {
    int components = container.getComponentCount();
    for (int i = 0; i < components; ++i)
    {
      Component component = container.getComponent(i);
      boolean mayRequestFocus = component.isFocusTraversable();
      if (mayRequestFocus && component instanceof JComponent) {
        mayRequestFocus = ((JComponent) component).isRequestFocusEnabled();
      }
      if (mayRequestFocus) {
	component.requestFocus();
	return true;
      } else if (component instanceof Container) {
        if (focusSomeComponentIn((Container) component)) {
	  return true;
	}
      }
    }
    return false;
  }
*/
}


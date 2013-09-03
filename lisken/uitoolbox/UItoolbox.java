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

/**
 * Provides a set of convenience methods for user interfaces.
 */
public class UItoolbox {

    /**
     * Registers an <code>ActionListener</code> with the root pane of a container
     * so that the program exits upon pressing the escape key.
     * @param container The container which should respond to the escape key.
     */
    public static void addExitOnEscape(RootPaneContainer container) {
        container.getRootPane().registerKeyboardAction(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent actionEvent) {
                System.exit(0);
            }
        }, KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0), JComponent.WHEN_IN_FOCUSED_WINDOW);
    }

    /**
     * Shows a dialog with info and the specified title.
     *
     * @param title The title of the dialog.
     * @param info The info to be shown.
     */
    public static void showTextInfo(String title, String info) {
        showTextInfo(title, info, false, null);
    }

    /**
     * Shows a dialog with info, the specified title and the specified font type.
     *
     * @param title The title of the dialog.
     * @param info The info to be shown.
     * @param monospaced The font type of the info.
     */
    public static void showTextInfo(String title, String info, boolean monospaced) {
        showTextInfo(title, info, monospaced, null);
    }

    /**
     * Shows a dialog with info and the specified title near the given component.
     *
     * @param title The title of the dialog.
     * @param info The info to be shown.
     * @param nearComponent The component in which proximity the dialog should be shown.
     */
    public static void showTextInfo(String title, String info, Component nearComponent) {
        showTextInfo(title, info, false, nearComponent);
    }

    /**
     * Shows a dialog with info and the specified title near the given component.
     *
     * @param title The title of the dialog.
     * @param info The info to be shown.
     * @param monospaced The font type of the info.
     * @param nearComponent The component in which proximity the dialog should be shown.
     */
    public static void showTextInfo(String title, String info, boolean monospaced, Component nearComponent) {
        final JDialog dialog = new JDialog((Frame) null, title, true);
        ActionListener endDialog = new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
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

    /**
     * Moves one component to another component.
     *
     * @param component The component which will be moved.
     * @param nearComponent The component to which will be moved.
     */
    public static void moveComponentNearComponent(Component component, Component nearComponent) {
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

    /**
     * Centers a component on the screen.
     * 
     * @param component The component to be centered on the screen.
     */
    public static void centerOnScreen(Component component) {
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        Dimension componentSize = component.getSize();
        if (componentSize.height > screenSize.height) {
            componentSize.height = screenSize.height;
        }
        if (componentSize.width > screenSize.width) {
            componentSize.width = screenSize.width;
        }
        component.setLocation(
                (screenSize.width - componentSize.width) / 2,
                (screenSize.height - componentSize.height) / 2);
    }

    /**
     * Sets the maximum size of a component to its preferred size.
     *
     * @param component The component for which the maximum size must be set.
     */
    public static void setmaxToPref(JComponent component) {
        component.setMaximumSize(component.getPreferredSize());
    }

    /**
     * Packs the window that contains a certain component.
     *
     * @param c The component which parent window should be packed.
     */
    public static void pack(Component c) {
        while (c != null) {
            if (c instanceof Window) {
                ((Window) c).pack();
                break;
            }
            c = c.getParent();
        }
    }

    /**
     * Restricts a dimension to a certain percentage of the screen size. If the
     * original dimension is larger than the given percentages of the screen size,
     * then it will be truncated. The percentages need to be given as floats where
     * 1.0f means 100%.
     *
     * @param originalSize The dimension that needs to be restricted.
     * @param widthPart The percentage of the screen width.
     * @param heightPart The percentage of the screen height.
     * @return The restricted dimension.
     */
    public static Dimension restrictSizeToScreenPart(Dimension originalSize, float widthPart, float heightPart) {
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        originalSize.width = Math.min(originalSize.width,
                (int) (screenSize.width * widthPart + 0.5f));
        originalSize.height = Math.min(originalSize.height,
                (int) (screenSize.height * heightPart + 0.5f));
        return originalSize;
    }

    /**
     * Returns the dimension of this component restricted to a certain percentage
     * of the screen size.
     * 
     * @param component The component for which the dimension needs to be restricted.
     * @param widthPart The percentage of the screen width.
     * @param heightPart The percentage of the screen height.
     * @return The restricted dimension.
     * @see #restrictSizeToScreenPart(java.awt.Dimension, float, float) 
     */
    public static Dimension restrictSizeToScreenPart(Component component, float widthPart, float heightPart) {
        return restrictSizeToScreenPart(component.getSize(), widthPart, heightPart);
    }

    public static void restrictScrollPaneToScreenPart(JScrollPane scrollPane, float widthPart, float heightPart) {
        Dimension maxSize = Toolkit.getDefaultToolkit().getScreenSize();
        maxSize.width = (int) (maxSize.width * widthPart + 0.5f);
        maxSize.height = (int) (maxSize.height * heightPart + 0.5f);
        Dimension scrollPaneSize = scrollPane.getPreferredSize();
        Dimension barSize;
        ScrollPaneLayout layout;
        if (scrollPaneSize.width > maxSize.width && scrollPaneSize.height > maxSize.height) {
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

    public static void addLabel(JSlider slider, int value) {
        Dictionary<Integer,JLabel> table = slider.getLabelTable();
        table.put(value, new JLabel(Integer.toString(value)));
        slider.setLabelTable(table);
    }

    public static void invokeAndWait(final Runnable runner) {
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

    /**
     * Returns the title of a window if it's a <code>Frame</code> or a <code>Dialog</code>
     * or <tt>null</tt> otherwise.
     *
     * @param window The window for which the title must be returned.
     * @return The title of <tt>window</tt> or <tt>null</tt> if not applicable.
     */
    public static String getTitle(Window window) {
        if (window instanceof Dialog) {
            return ((Dialog) window).getTitle();
        } else if (window instanceof Frame) {
            return ((Frame) window).getTitle();
        } else {
            return null;
        }
    }

    /**
     * Tries to focus a window.
     *
     * @param window The window that needs to be focussed.
     */
    public static void focusWindow(final Window window) {
        SwingUtilities.invokeLater(new Runnable() {

            @Override
            public void run() {
                window.toFront();
            }
        });
    }
}


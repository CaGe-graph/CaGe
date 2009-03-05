package cage.utility;

import java.awt.CardLayout;
import java.awt.Component;
import java.awt.Container;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.AbstractButton;

/**
 * Implementation of an <code>ActionListener</code> that shows a certain
 * component in a <code>Container</code> with a <code>CardLayout</code>.
 * <p>
 * Note that this class assumes <tt>container</tt> has a <code>CardLayout
 * </code>. If this is not the case the <tt>actionPerformed</tt> method will
 * throw a <code>ClassCastException</code>.
 */
public class OnActionClickerLayoutSwitcher implements ActionListener {

    private AbstractButton button;
    private Container container;

    /**
     * Constructs a <code>OnActionClicker</code> object.
     *
     * @param b A button that is selected after the
     *          <tt>actionPerformed</tt> method.
     * @param c The container for which the layout needs to be switched.
     */
    public OnActionClickerLayoutSwitcher(AbstractButton b, Container c) {
        button = b;
        container = c;
    }

    /**
     * Constructs a <code>OnActionClicker</code> object and registers it
     * with <tt>target</tt>.
     *
     * @param b A button that is selected after the
     *          <tt>actionPerformed</tt> method.
     * @param c The container for which the layout needs to be switched.
     * @param target A button with which this listener will be registered.
     */
    public OnActionClickerLayoutSwitcher(AbstractButton b, Container c, AbstractButton target) {
        this(b, c);
        target.addActionListener(this);
    }

    public void actionPerformed(ActionEvent e) {
        Component c = (Component) e.getSource();
        ((CardLayout) container.getLayout()).show(container, e.getActionCommand());
        if (c.isVisible()) {
            button.setSelected(true);
            c.getParent().transferFocus();
        }
    }
}

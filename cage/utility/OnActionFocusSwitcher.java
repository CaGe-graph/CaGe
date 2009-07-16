package cage.utility;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.AbstractButton;
import javax.swing.JComboBox;
import javax.swing.JComponent;

/**
 * Implementation of <code>ActionListener</code> that tries to focus a given
 * component when fired.
 */
public class OnActionFocusSwitcher implements ActionListener {

    private JComponent component;

    /**
     * Creates a new <code>OnActionFocusSwitcher</code>.
     * @param c The component that needs to be focused.
     */
    public OnActionFocusSwitcher(JComponent c) {
        component = c;
    }

    /**
     * Creates a new <code>OnActionFocusSwitcher</code> and adds it to the
     * specified <code>JComboBox</code> or <code>AbstractButton</code>.
     * @param c The component that needs to be focused.
     * @param target The <code>JComboBox</code> or <code>AbstractButton</code>
     *               to which needs to be listened
     */
    public OnActionFocusSwitcher(JComponent c, JComponent target) {
        this(c);
        if (target instanceof JComboBox) {
            ((JComboBox) target).addActionListener(this);
        } else if (target instanceof AbstractButton) {
            ((AbstractButton) target).addActionListener(this);
        }
    }

    public void actionPerformed(ActionEvent e) {
        component.requestFocus();
    }
}
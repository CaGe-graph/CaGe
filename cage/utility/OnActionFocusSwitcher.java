package cage.utility;

import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.AbstractButton;
import javax.swing.JComboBox;
import javax.swing.JComponent;

/**
 *
 */
public class OnActionFocusSwitcher implements ActionListener {

    private JComponent component;

    public OnActionFocusSwitcher(JComponent c) {
        component = c;
    }

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
        Component source = (Component) e.getSource();
        if (source instanceof AbstractButton) {
            if (!((AbstractButton) source).isSelected()) {
                return;
            }
        }
//    source.transferFocus();
    }
}
package lisken.uitoolbox;

import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.InputEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.text.JTextComponent;

/**
 * Implementation of <code>FocusListener</code> and <code>MouseListener</code>, that
 * can be used to manage the automatic selection of the text in a <code>JTextComponent</code>
 * when it gains focus. This listener should not directly be added as a listener to
 * a text field, but the method {@link #setField(javax.swing.text.JTextComponent)} should
 * be used.
 */
public class JTextComponentFocusSelector extends MouseAdapter implements FocusListener {

    private boolean dontSelect = false;

    @Override
    public void focusGained(FocusEvent e) {
        if (e.isTemporary()) {
            return;
        } else if (dontSelect) {
            dontSelect = false;
            return;
        }
        ((JTextComponent) e.getSource()).selectAll();
    }

    @Override
    public void focusLost(FocusEvent e) {
        if (e.isTemporary()) {
            return;
        }
        JTextComponent tc = (JTextComponent) e.getSource();
        try {
            tc.setCaretPosition(tc.getCaretPosition());
        } catch (IllegalArgumentException ex) {
        }
    }

    @Override
    public void mousePressed(MouseEvent e) {
        if ((e.getModifiers() & InputEvent.BUTTON1_MASK) == 0) {
            return;
        } else if (((JTextComponent) e.getSource()).hasFocus()) {
            return;
        }
        dontSelect = true;
    }

    /**
     * Adds this focus selector to the text component <tt>tc</tt>.
     *
     * @param tc A text component for which this focus selector will be used.
     */
    public void setField(JTextComponent tc) {
        tc.addFocusListener((FocusListener) this);
        tc.addMouseListener(this);
    }

    /**
     * Creates a new <code>JTextComponentFocusSelector</code> and adds it as a
     * listener to <tt>tc</tt>.
     *
     * @param tc A text component for which this focus selector will be used.
     */
    public JTextComponentFocusSelector(JTextComponent tc) {
        setField(tc);
    }
}


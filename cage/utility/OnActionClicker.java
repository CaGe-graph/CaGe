package cage.utility;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.AbstractButton;

/**
 * Implementation of an <code>ActionListener</code> that forwards the click
 * to another button depending on the selection state of the source button.
 */
public class OnActionClicker implements ActionListener {

    private AbstractButton buttonTrue, buttonFalse, target;

    /**
     * Constructs a <code>OnActionClicker</code> object.
     * <p>
     * This method throws an <code>IllegalArgumentException</code> if
     * <tt>bf</tt> or <tt>target</tt> is <tt>null</tt>.
     * A <tt>null</tt> <tt>bt</tt> string is legal.
     *
     * @param bt The button to be clicked if target is selected.
     * @param bf The button to be clicked if target is deselected.
     * @param target The button to which this listener will listen.
     * @throws IllegalArgumentException if <tt>bf</tt> or <tt>target</tt> is null
     */
    public OnActionClicker(AbstractButton bt, AbstractButton bf, AbstractButton target) {
        if(bf==null || target==null)
            throw new IllegalArgumentException("Both bf and target should be non-null");
        buttonTrue = bt;
        buttonFalse = bf;
        this.target = target;
        target.addActionListener(this);
    }

    /**
     * Constructs a <code>OnActionClicker</code> object with the true
     * button set to <tt>null</tt>.
     * <p>
     * This method throws an <code>IllegalArgumentException</code> if
     * <tt>bf</tt> or <tt>target</tt> is <tt>null</tt>.
     *
     * @param bf The button to be clicked if target is deselected.
     * @param target The button to which this listener will listen.
     * @throws IllegalArgumentException if <tt>bf</tt> or <tt>target</tt> is null
     */
    public OnActionClicker(AbstractButton bf, AbstractButton target) {
        this(null, bf, target);
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        AbstractButton source = (AbstractButton) e.getSource();
        if(!target.equals(source))
            return; //this listener should only be registered with target

        if (source.isSelected()) {
            if (buttonTrue != null) {
                buttonTrue.doClick();
                if (!buttonTrue.isVisible()) {
                    source.transferFocus();
                }
            }
        } else {
            if (buttonFalse != null) {
                buttonFalse.doClick();
            }
        }
    }
}

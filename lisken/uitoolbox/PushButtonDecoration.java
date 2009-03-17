package lisken.uitoolbox;

import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
import javax.swing.border.Border;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

public class PushButtonDecoration implements ChangeListener {

    private AbstractButton button;
    private boolean isToggle;
    private Border upBorder,  downBorder,  otherBorder;

    public PushButtonDecoration(AbstractButton b) {
        this(b, false);
    }

    public PushButtonDecoration(AbstractButton b, boolean isT) {
        button = b;
        isToggle = isT;
        otherBorder = button.getBorder();
        computeBorders();
        button.getModel().addChangeListener(this);
    }

    private void computeBorders() {
        upBorder = BorderFactory.createCompoundBorder(
                BorderFactory.createRaisedBevelBorder(), otherBorder);
        downBorder = BorderFactory.createCompoundBorder(
                BorderFactory.createLoweredBevelBorder(), otherBorder);
        updateBorder();
    }

    private void updateBorder() {
        boolean depressed = isToggle ? button.getModel().isSelected() : button.getModel().isPressed();
        button.setBorder(depressed ? downBorder : upBorder);
    }

    public void stateChanged(ChangeEvent e) {
        updateBorder();
    }
}

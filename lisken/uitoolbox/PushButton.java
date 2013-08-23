package lisken.uitoolbox;

import javax.swing.BorderFactory;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.border.Border;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

public class PushButton extends JButton {

    private Border upBorder,  downBorder,  otherBorder;

    public PushButton(String text, Icon icon) {
        super(text, icon);
        otherBorder = null;
        computeBorders();
        getModel().addChangeListener(new PushButtonListener());
    }

    public PushButton(String text) {
        this(text, null);
    }

    public PushButton(Icon icon) {
        this("", icon);
    }

    public PushButton() {
        this("", null);
    }

    @Override
    public void setBorder(Border b) {
        otherBorder = b;
        computeBorders();
    }

    private void computeBorders() {
        upBorder = BorderFactory.createCompoundBorder(
                BorderFactory.createRaisedBevelBorder(), otherBorder);
        downBorder = BorderFactory.createCompoundBorder(
                BorderFactory.createLoweredBevelBorder(), otherBorder);
        updateBorder();
    }

    private void updateBorder() {
        super.setBorder(this.getModel().isPressed() ? downBorder : upBorder);
    }

    class PushButtonListener implements ChangeListener {

        public PushButtonListener() {
        }

        @Override
        public void stateChanged(ChangeEvent e) {
            updateBorder();
        }
    }
}

package cage.utility;

import java.awt.event.ItemEvent;
import java.util.Enumeration;
import javax.swing.AbstractButton;

/**
 * An implementation of <code>GenericButtonGroup</code> that represents a
 * group where all the buttons have the same selection state. This state is
 * determined by the last button that was added to this group or the last
 * button for which the selection state changed, whichever of these two has
 * happened last.
 */
public class SyncButtonGroup extends AbstractButtonGroup {

    public void add(AbstractButton button) {
        if (buttons.size() > 0) {
            boolean selected = button.isSelected();
            AbstractButton otherButton = (AbstractButton) buttons.elementAt(0);
            //We only need to check the first button.
            //If this button has the same selection state as the new button,
            //then all the buttons will have this. Otherwise the change of
            //the first button will cause a cascade changing all the buttons.
            if (otherButton.isSelected() != selected) {
                otherButton.setSelected(selected);
                button.setSelected(otherButton.isSelected());
            }
        }
        super.add(button);
    }

    public void itemStateChanged(ItemEvent e) {
        int change = e.getStateChange();
        if (change != ItemEvent.SELECTED && change != ItemEvent.DESELECTED) {
            return;
        }
        AbstractButton button = (AbstractButton) e.getSource();
        boolean selected = button.isSelected();
        Enumeration btns = buttons.elements();
        while (btns.hasMoreElements()) {
            AbstractButton otherButton = (AbstractButton) btns.nextElement();
            if (otherButton.isSelected() == selected) {
                continue;
            }
            otherButton.setSelected(selected);
            // Change the selection of just one button (that needs to be changed).
            // That button will create its own item event,
            // eventually triggering all necessary changes.
            break;
        }
    }
}
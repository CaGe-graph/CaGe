package cage.utility;

import java.awt.event.ItemListener;
import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;
import javax.swing.AbstractButton;

/**
 * An implementation of <code>GenericButtonGroup</code> that uses a <code>
 * Vector</code>. This class implements <code>ItemListener</code> and registers
 * itself with any button that is added to the group and removes itself from
 * any button that is removed from the group.
 */
public abstract class AbstractButtonGroup implements GenericButtonGroup, ItemListener {

    /** The <code>List</code> that contains the elements of this group. */
    protected List<AbstractButton> buttons = new ArrayList<>();

    /**
     * Add <tt>button</tt> to this group and <tt>this</tt> as listener to
     * that button.
     * @param button The <code>AbstractButton</code> to add to this group.
     */
    @Override
    public void add(AbstractButton button) {
        buttons.add(button);
        button.addItemListener(this);
    }

    /**
     * Remove <tt>button</tt> from this group and do nothing if
     * it wasn't in this group. Also removes <tt>this</tt> as listener
     * from the button.
     * @param button The <code>AbstractButton</code> to remove to this group.
     */
    @Override
    public void remove(AbstractButton button) {
        button.getModel().removeItemListener(this);
        buttons.remove(button);
    }

    /**
     * Returns an enumeration of the buttons in this group.
     * @return An enumeration of the buttons in this group.
     */
    @Override
    public ListIterator<AbstractButton> getElements() {
        return buttons.listIterator();
    }

    /**
     * Always returns <tt>null</tt>.
     * @return <tt>null</tt>
     */
    public AbstractButton getSelection() {
        return null;
    }
}
package cage.utility;

import java.util.Enumeration;
import javax.swing.AbstractButton;

/**
 * An object implementing this interface represents a group of
 * <code>AbstractButton</code>s.
 */
public interface GenericButtonGroup {

    /**
     * Add <tt>button</tt> to this group.
     * @param button The <code>AbstractButton</code> to add to this group.
     */
    void add(AbstractButton button);

    /**
     * Remove <tt>button</tt> from this group and do nothing if
     * it wasn't in this group.
     * @param button The <code>AbstractButton</code> to remove to this group.
     */
    void remove(AbstractButton button);

    /**
     * Returns an enumeration of the buttons in this group.
     * @return An enumeration of the buttons in this group.
     */
    Enumeration getElements();
}
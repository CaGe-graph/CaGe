package cage.viewer.jmol;

import java.text.MessageFormat;
import org.jmol.api.JmolViewer;

/**
 * Action that allows to change the view of a given JmolViewer.
 *
 * @author nvcleemp
 */
public class ViewAction extends JmolAction {

    public static final String FRONT =  "moveto 2.0 front;delay 1";
    public static final String LEFT =   "moveto 1.0 front;moveto 2.0 left;delay 1";
    public static final String RIGHT =  "moveto 1.0 front;moveto 2.0 right;delay 1";
    public static final String TOP =    "moveto 1.0 front;moveto 2.0 top;delay 1";
    public static final String BOTTOM = "moveto 1.0 front;moveto 2.0 bottom;delay 1";
    public static final String BACK =   "moveto 1.0 front;moveto 2.0 back;delay 1";


    private final String command;

    /**
     * Creates a <code>ViewAction</code> that changes the given <code>
     * JmolViewer</code> to the given view. It is adviced to pass in one
     * of the constants defined in this class as <tt>view</tt>.
     *
     * @param name The name for this action
     * @param viewer The viewer to which this action applies
     * @param view The view that is to be shown
     */
    public ViewAction(String name, JmolViewer viewer, String view) {
        super(name, viewer);
        command = MessageFormat.format("if not(showBoundBox);if not(showUnitcell);boundbox on;{0};boundbox off;endif;else;{0};endif;", new String[]{view});
    }

    public String getCommand() {
        return command;
    }

}

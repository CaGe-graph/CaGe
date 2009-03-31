package cage.viewer.jmol;

import java.awt.event.ActionEvent;
import javax.swing.AbstractAction;
import org.jmol.api.JmolViewer;

/**
 * An implementation of <code>AbstractAction</code> that evaluates a script in
 * a <code>JmolViewer</code> as action.
 * 
 * @author nvcleemp
 */
public abstract class JmolAction extends AbstractAction {

    private JmolViewer viewer;

    public JmolAction(String name, JmolViewer viewer) {
        super(name);
        this.viewer = viewer;
    }

    public JmolViewer getViewer() {
        return viewer;
    }

    public void setViewer(JmolViewer viewer) {
        this.viewer = viewer;
    }

    public void actionPerformed(ActionEvent e) {
        viewer.evalString(getCommand());
    }

    public abstract String getCommand();

}

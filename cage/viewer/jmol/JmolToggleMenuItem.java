package cage.viewer.jmol;

import javax.swing.JCheckBoxMenuItem;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.jmol.api.JmolViewer;

/**
 *
 * @author nvcleemp
 */
public class JmolToggleMenuItem extends JCheckBoxMenuItem{

    public JmolToggleMenuItem(String text, final String onAction, final String offAction, final JmolViewer viewer) {
        this(text, onAction, offAction, viewer, false);
    }

    public JmolToggleMenuItem(String text, final String onAction, final String offAction, final JmolViewer viewer, boolean isSelected) {
        super(text, isSelected);
        addChangeListener(new ChangeListener() {

            public void stateChanged(ChangeEvent e) {
                if(isSelected()){
                    viewer.evalString(onAction);
                } else {
                    viewer.evalString(offAction);
                }
            }
        });
    }

}

package cage.viewer.jmol;

import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.jmol.api.JmolViewer;

/**
 *
 * @author nvcleemp
 */
public class StoredJmolToggleMenuItem extends JmolToggleMenuItem{

    public StoredJmolToggleMenuItem(String text, final String onAction, final String offAction, final JmolViewer viewer, final JmolPanel panel, final String key) {
        this(text, onAction, offAction, viewer, panel, key, false);
    }

    public StoredJmolToggleMenuItem(String text, final String onAction, final String offAction, final JmolViewer viewer, final JmolPanel panel, final String key, boolean isSelected) {
        super(text, onAction, offAction, viewer, isSelected);
        addChangeListener(new ChangeListener() {

            @Override
            public void stateChanged(ChangeEvent e) {
                if(isSelected()){
                    panel.storeCommand(key, onAction);
                } else {
                    panel.storeCommand(key, offAction);
                }
            }
        });
    }

}

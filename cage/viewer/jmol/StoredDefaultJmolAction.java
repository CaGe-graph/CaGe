package cage.viewer.jmol;

import java.awt.event.ActionEvent;
import org.jmol.api.JmolViewer;

/**
 *
 * @author nvcleemp
 */
public class StoredDefaultJmolAction extends DefaultJmolAction{
    
    private final String key;
    private final JmolPanel panel;

    public StoredDefaultJmolAction(String name, JmolViewer viewer, JmolPanel panel, String key, String command) {
        super(name, viewer, command);
        this.panel = panel;
        this.key = key;
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        super.actionPerformed(e);
        panel.storeCommand(key, getCommand());
    }

}

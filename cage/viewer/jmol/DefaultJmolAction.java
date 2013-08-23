package cage.viewer.jmol;

import org.jmol.api.JmolViewer;

/**
 *
 * @author nvcleemp
 */
public class DefaultJmolAction extends JmolAction{

    private String command;

    public DefaultJmolAction(String name, JmolViewer viewer, String command) {
        super(name, viewer);
        this.command = command;
    }

    @Override
    public String getCommand() {
        return command;
    }

}

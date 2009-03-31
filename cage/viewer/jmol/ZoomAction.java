package cage.viewer.jmol;

import java.text.MessageFormat;
import org.jmol.api.JmolViewer;

/**
 *
 * @author nvcleemp
 */
public class ZoomAction extends JmolAction{

    private String command;

    public ZoomAction(JmolViewer viewer, int amount) {
        super(MessageFormat.format("{0}%", new Integer[]{Integer.valueOf(amount)}), viewer);
        command = MessageFormat.format("zoom {0}", new Integer[]{Integer.valueOf(amount)});

    }

    public String getCommand() {
        return command;
    }

}

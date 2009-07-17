package cage.viewer.jmol;

import java.awt.Color;
import java.text.MessageFormat;
import javax.swing.colorchooser.ColorSelectionModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.jmol.api.JmolViewer;

/**
 *
 * @author nvcleemp
 */
public class JmolColorChangeListener implements ChangeListener{

    protected JmolViewer viewer;
    protected String component;
    private ColorSelectionModel colorModel;

    public JmolColorChangeListener(JmolViewer viewer, String component, ColorSelectionModel colorModel) {
        this.viewer = viewer;
        this.component = component;
        this.colorModel = colorModel;
    }

    protected static String colorToString(Color c){
        Integer[] colors = {Integer.valueOf(c.getRed()),
                            Integer.valueOf(c.getGreen()),
                            Integer.valueOf(c.getBlue())};
        String colorString = MessageFormat.format("[{0},{1},{2}]", colors);
        return colorString;
    }

    public void stateChanged(ChangeEvent e) {
        Object[] obj = {component, colorToString(colorModel.getSelectedColor())};
        viewer.evalString(MessageFormat.format("color {0} {1}", obj));
    }

}

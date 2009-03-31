package cage.viewer.jmol;

import java.awt.Color;
import java.text.MessageFormat;
import javax.swing.ButtonModel;
import javax.swing.colorchooser.ColorSelectionModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import org.jmol.api.JmolViewer;

/**
 *
 * @author nvcleemp
 */
public class JmolTranslucentColorChangeListener implements ChangeListener{

    protected JmolViewer viewer;
    protected String component;
    protected ButtonModel translucencyModel;
    private ColorSelectionModel colorModel;

    public JmolTranslucentColorChangeListener(JmolViewer viewer, String component, ColorSelectionModel colorModel, ButtonModel translucencyModel) {
        this.viewer = viewer;
        this.component = component;
        this.colorModel = colorModel;
        this.translucencyModel = translucencyModel;
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
        if(translucencyModel.isSelected())
            viewer.evalString(MessageFormat.format("color {0} TRANSLUCENT {1}", obj));
        else
            viewer.evalString(MessageFormat.format("color {0} {1}", obj));
    }

}

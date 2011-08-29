package cage.viewer.jmol;

import cage.utility.CaGeColorChooser;
import java.awt.Color;
import javax.swing.ButtonModel;
import javax.swing.colorchooser.DefaultColorSelectionModel;

import org.jmol.api.JmolViewer;

/**
 *
 * @author nvcleemp
 */
public class JmolTranslucentColorAction extends JmolColorAction{

    public JmolTranslucentColorAction(String name, ButtonModel previewModel, String component, Color startColor, JmolViewer viewer, ButtonModel translucencyModel) {
        super(name, previewModel);
        colorModel = new DefaultColorSelectionModel(startColor);
        colorModel.addChangeListener(new JmolTranslucentColorChangeListener(viewer, component, colorModel, translucencyModel));
        chooser = new CaGeColorChooser(colorModel);
        
    }

}

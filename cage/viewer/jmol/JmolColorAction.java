package cage.viewer.jmol;

import java.awt.Color;
import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.AbstractAction;
import javax.swing.ButtonModel;
import javax.swing.JColorChooser;
import javax.swing.JDialog;
import javax.swing.colorchooser.ColorSelectionModel;
import javax.swing.colorchooser.DefaultColorSelectionModel;
import org.jmol.api.JmolViewer;

/**
 *
 * @author nvcleemp
 */
public class JmolColorAction extends AbstractAction{

    protected ButtonModel previewModel;
    protected ColorSelectionModel colorModel;
    protected JColorChooser chooser;
    protected Component parentComponent;

    public JmolColorAction(String name, ButtonModel previewModel, String component, Color startColor, JmolViewer viewer) {
        super(name);
        this.previewModel = previewModel;
        colorModel = new DefaultColorSelectionModel(startColor);
        colorModel.addChangeListener(new JmolColorChangeListener(viewer, component, colorModel));
        chooser = new JColorChooser(colorModel);
    }

    public JmolColorAction(String name, ButtonModel previewModel) {
        super(name);
        this.previewModel = previewModel;
    }

    public void actionPerformed(ActionEvent e) {
        if(previewModel.isSelected()){
            //if preview is enabled we use the chooser connected to the colormodel
            //we just need to reset the color when cancel is pressed.
            final Color oldColor = chooser.getColor();
            JDialog d = JColorChooser.createDialog(parentComponent,
                getValue(AbstractAction.NAME).toString(), true , chooser,
                null, new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        colorModel.setSelectedColor(oldColor);
                    }
                });
            d.setVisible(true);
        } else {
            //if preview is disabled we open a general chooser and set the color
            //afterwards.
            Color c = JColorChooser.showDialog(parentComponent, getValue(AbstractAction.NAME).toString(), colorModel.getSelectedColor());
            if(c!=null){
                colorModel.setSelectedColor(c);
            }
        }
    }
}

package cage.utility;

import java.awt.Color;
import javax.swing.JColorChooser;
import javax.swing.colorchooser.AbstractColorChooserPanel;
import javax.swing.colorchooser.ColorSelectionModel;

/**
 * Extension of JColorChooser that doesn't have the HSB panel because this was
 * causing freezing GUI's for some users. This workaround was suggested in the
 * bug report at {@link http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=6419354}.
 * 
 * @author nvcleemp
 */
public class CaGeColorChooser extends JColorChooser {

    public CaGeColorChooser(ColorSelectionModel model) {
        super(model);
    }

    public CaGeColorChooser(Color initialColor) {
        super(initialColor);
    }

    public CaGeColorChooser() {
        super();
    }

    @Override
    public void setChooserPanels(AbstractColorChooserPanel[] panels) {
        AbstractColorChooserPanel[] panels2 = panels;
        if (panels != null) {
            int numNeeded = 0;
            for (int i = 0; i < panels.length; i++) {
                if (!(panels[i].getClass().getName().contains("DefaultHSBChooserPanel"))) {
                    numNeeded++;
                }
            }
            if (numNeeded < panels.length) {
                panels2 = new AbstractColorChooserPanel[numNeeded];
                int j = 0;
                for (int i = 0; i < panels.length; i++) {
                    if (!(panels[i].getClass().getName().contains("DefaultHSBChooserPanel"))) {
                        panels2[j++] = panels[i];
                    }
                }
            }
        }
        super.setChooserPanels(panels2);
    }
}

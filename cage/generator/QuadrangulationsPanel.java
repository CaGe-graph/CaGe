package cage.generator;

import cage.CombinedGeneratorPanel;

/**
 * Panel that combines all the configuration panels for generators of
 * quadrangulations. The different panels can be accessed through tabs.
 *
 * @author nvcleemp
 */
public class QuadrangulationsPanel extends CombinedGeneratorPanel {

    public QuadrangulationsPanel() {
        addTab("general", new GeneralQuadrangulationsPanel(false));
        addTab("with given degree", new QuadRestrictPanel(false));
        add(pane);
    }

}







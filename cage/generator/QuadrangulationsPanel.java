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
        addTab("all degrees allowed", new GeneralQuadrangulationsPanel(false));
        addTab("quadrangulations with given degrees", new QuadRestrictPanel(false));
        add(pane);
    }

}







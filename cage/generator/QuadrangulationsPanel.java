package cage.generator;

import cage.CombinedGeneratorPanel;
import cage.GeneratorInfo;
import cage.GeneratorPanel;

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

    public GeneratorInfo getGeneratorInfo() {
        GeneratorPanel chosenPanel = (GeneratorPanel) pane.getSelectedComponent();
        GeneratorInfo info = chosenPanel.getGeneratorInfo();
        return info;
    }
}







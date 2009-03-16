package cage.generator;

import cage.CombinedGeneratorPanel;
import cage.GeneratorInfo;
import cage.GeneratorPanel;

/**
 * Panel that combines all the configuration panels for generators of 4-regular
 * plane graphs. The different panels can be accessed through tabs.
 * 
 * @author nvcleemp
 */
public class FourRegularPlaneGraphsPanel extends CombinedGeneratorPanel {

    public FourRegularPlaneGraphsPanel() {
        addTab("all face sizes allowed", new GeneralQuadrangulationsPanel(true));
        addTab("quartic graphs with given faces", new QuadRestrictPanel(true));
        add(pane);
    }

    public void showing() {
    }

    public GeneratorInfo getGeneratorInfo() {
        GeneratorPanel chosenPanel = (GeneratorPanel) pane.getSelectedComponent();
        GeneratorInfo info = chosenPanel.getGeneratorInfo();
        return info;
    }
}
package cage.generator;

import cage.CombinedGeneratorPanel;

/**
 * Panel that combines all the configuration panels for generators of 4-regular
 * plane graphs. The different panels can be accessed through tabs.
 * 
 * @author nvcleemp
 */
public class FourRegularPlaneGraphsPanel extends CombinedGeneratorPanel {

    public FourRegularPlaneGraphsPanel() {
        addTab("all face sizes allowed", new GeneralQuadrangulationsPanel(true));
        addTab("3-conn. quartic graphs with given faces", new QuadRestrictPanel(true));
        add(pane);
    }
    
}

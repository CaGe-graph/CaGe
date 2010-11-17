package cage.generator;

import cage.CombinedGeneratorPanel;

public class ThreeRegularPlaneGraphsPanel extends CombinedGeneratorPanel {

    public ThreeRegularPlaneGraphsPanel() {
        addTab("general cubic plane graphs", new GeneralTriangulationsPanel(true));
        addTab("fullerenes", new FullgenPanel());
        addTab("cubic plane graphs with given faces", new CGFPanel(false));
        addTab("bipartite cubic plane graphs", new EulerianTriangulationsPanel(true));
        add(pane);
    }
}
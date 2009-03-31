package cage.generator;

import cage.GeneratorInfo;
import cage.GeneratorPanel;

import javax.swing.BorderFactory;
import javax.swing.JTabbedPane;

public class ThreeRegularPlaneGraphsPanel extends GeneratorPanel {

    private static final boolean debug = false;
    private JTabbedPane pane = new JTabbedPane();

    public ThreeRegularPlaneGraphsPanel() {
        pane.addTab("general cubic plane graphs", new GeneralTriangulationsPanel(true));
        pane.addTab("fullerenes", new FullgenPanel());
        pane.addTab("cubic plane graphs with given faces", new CGFPanel(false));
        pane.addTab("bipartite cubic plane graphs", new EulerianTriangulationsPanel(true));
        for (int i = 0; i < pane.getTabCount(); ++i) {
            ((GeneratorPanel) pane.getComponentAt(i)).setBorder(
                    BorderFactory.createEmptyBorder(20, 20, 20, 20));
        }
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
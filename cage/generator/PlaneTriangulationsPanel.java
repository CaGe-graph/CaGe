package cage.generator;

import cage.GeneratorInfo;
import cage.GeneratorPanel;

import javax.swing.BorderFactory;
import javax.swing.JTabbedPane;

public class PlaneTriangulationsPanel extends GeneratorPanel {

    public PlaneTriangulationsPanel() {
        pane.addTab("general triangulations", new GeneralTriangulationsPanel(false));
        pane.addTab("Eulerian triangulations", new EulerianTriangulationsPanel(false));
        pane.addTab("triangulations with given degrees", new CGFPanel(true));
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
    JTabbedPane pane = new JTabbedPane();
}

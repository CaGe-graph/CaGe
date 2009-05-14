package cage.generator;

import cage.CombinedGeneratorPanel;
import cage.GeneratorInfo;
import cage.GeneratorPanel;

public class TriangulationsPanel extends CombinedGeneratorPanel {

    private static final boolean debug = false;
    private GeneratorPanel lastChosenPanel = null;

    public TriangulationsPanel() {
        pane.addTab("of the plane", new PlaneTriangulationsPanel());
        //there shouldn't be a border around PlaneTriangulationsPanel, that is why
        //this is done separately. TODO: provide method for this.
        addTab("of the disk", new DiskTriangulationsPanel());
        add(pane);
    }

    public GeneratorInfo getGeneratorInfo() {
        GeneratorPanel chosenPanel = (GeneratorPanel) pane.getSelectedComponent();
        GeneratorInfo info = chosenPanel.getGeneratorInfo();
        if (chosenPanel != lastChosenPanel &&
                (chosenPanel instanceof DiskTriangulationsPanel ||
                lastChosenPanel instanceof DiskTriangulationsPanel)) {
            info.getEmbedder().setConstant(false);
        }
        lastChosenPanel = chosenPanel;
        return info;
    }
}







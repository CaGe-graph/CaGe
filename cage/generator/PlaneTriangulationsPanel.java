package cage.generator;

import cage.CombinedGeneratorPanel;

public class PlaneTriangulationsPanel extends CombinedGeneratorPanel {

    public PlaneTriangulationsPanel() {
        addTab("general triangulations", new GeneralTriangulationsPanel(false));
        addTab("Eulerian triangulations", new EulerianTriangulationsPanel(false));
        addTab("triangulations with given degrees", new CGFPanel(true));
        add(pane);
    }
}

package cage.generator;

import cage.CombinedGeneratorPanel;

public class HCgenPanel extends CombinedGeneratorPanel {

    public HCgenPanel() {
        addTab("by formula", new FormulaHCgenPanel());
        addTab("by boundary structure", new BoundaryHCgenPanel());
        addTab("by number of hexagons (fusenes)", new HexagonsHCgenPanel());
        addTab("generalised fusenes", new GeneralisedFusenesPanel());
        add(pane);
    }

}

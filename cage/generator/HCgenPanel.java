package cage.generator;

import cage.GeneratorInfo;
import cage.GeneratorPanel;

import javax.swing.BorderFactory;
import javax.swing.JTabbedPane;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

public class HCgenPanel extends GeneratorPanel {

    private GeneratorPanel lastChosenPanel = null;
    private JTabbedPane pane = new JTabbedPane();
    private FormulaHCgenPanel formulaHCgenPanel;
    private BoundaryHCgenPanel boundaryHCgenPanel;
    private HexagonsHCgenPanel hexagonsHCgenPanel;

    public HCgenPanel() {
        formulaHCgenPanel = new FormulaHCgenPanel();
        boundaryHCgenPanel = new BoundaryHCgenPanel();
        hexagonsHCgenPanel = new HexagonsHCgenPanel();
        pane.addTab("by formula", formulaHCgenPanel);
        pane.addTab("by boundary structure", boundaryHCgenPanel);
        pane.addTab("by number of hexagons (fusenes)", hexagonsHCgenPanel);
        for (int i = 0; i < pane.getTabCount(); i++) {
            ((GeneratorPanel) pane.getComponentAt(i)).setBorder(
                    BorderFactory.createEmptyBorder(20, 20, 20, 20));
        }
        add(pane);

        pane.addChangeListener(new ChangeListener() {

            public void stateChanged(ChangeEvent event) {
                JTabbedPane pane = (JTabbedPane) event.getSource();
                ((GeneratorPanel) pane.getSelectedComponent()).showing();
            }
        });
    }

    public void showing() {
        ((GeneratorPanel) pane.getSelectedComponent()).showing();
    //formulaHCgenPanel.setDefaultButton(SwingUtilities.getRootPane(this).getDefaultButton());
    //formulaHCgenPanel.showing();
    //boundaryHCgenPanel.showing();
    }

    public GeneratorInfo getGeneratorInfo() {
        GeneratorPanel chosenPanel = (GeneratorPanel) pane.getSelectedComponent();
        GeneratorInfo info = chosenPanel.getGeneratorInfo();
        return info;
    }
}

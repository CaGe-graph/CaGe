

package cage.generator;


import cage.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import lisken.systoolbox.*;
import lisken.uitoolbox.*;


public class HCgenPanel extends GeneratorPanel
{

    private GeneratorPanel lastChosenPanel = null;

    private JTabbedPane pane = new JTabbedPane();

    private FormulaHCgenPanel formulaHCgenPanel;

    private DegreeListHCgenPanel degreeListHCgenPanel;

    private HexagonsHCgenPanel hexagonsHCgenPanel;

    public HCgenPanel() {
	formulaHCgenPanel = new FormulaHCgenPanel();
	degreeListHCgenPanel = new DegreeListHCgenPanel();
	hexagonsHCgenPanel = new HexagonsHCgenPanel();
	pane.addTab("by formula", formulaHCgenPanel);
	pane.addTab("by degree list", degreeListHCgenPanel);
	pane.addTab("by number of hexagons", hexagonsHCgenPanel);
	for (int i = 0; i < pane.getTabCount(); i++) {
	    ((GeneratorPanel) pane.getComponentAt(i)).setBorder(
	        BorderFactory.createEmptyBorder(20, 20, 20, 20));
	}
	add(pane);

	pane.addChangeListener(new ChangeListener() {
		public void stateChanged(ChangeEvent event) {
		    JTabbedPane pane = (JTabbedPane)event.getSource();
		    ((GeneratorPanel)pane.getSelectedComponent()).showing();
		}
	    });
    }

    public void showing() {
	((GeneratorPanel)pane.getSelectedComponent()).showing();
	//formulaHCgenPanel.setDefaultButton(SwingUtilities.getRootPane(this).getDefaultButton());
	//formulaHCgenPanel.showing();
	//degreeListHCgenPanel.showing();
    }

    public GeneratorInfo getGeneratorInfo() {
	GeneratorPanel chosenPanel = (GeneratorPanel) pane.getSelectedComponent();
	GeneratorInfo info = chosenPanel.getGeneratorInfo();
	return info;
    }

}

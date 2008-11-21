package cage.generator;

import cage.GeneratorInfo;
import cage.GeneratorPanel;
import javax.swing.BorderFactory;
import javax.swing.JTabbedPane;

public class PlaneTriangulationsPanel extends GeneratorPanel
{

  private GeneratorPanel lastChosenPanel = null;

  public PlaneTriangulationsPanel()
  {
    pane.addTab("general", new GeneralTriangulationsPanel());
    pane.addTab("eulerian", new EulerianTriangulationsPanel());
    for (int i = 0; i < pane.getTabCount(); ++i)
    {
      ((GeneratorPanel) pane.getComponentAt(i)).setBorder(
       BorderFactory.createEmptyBorder(20, 20, 20, 20));
    }
    add(pane);
  }

  public void showing()
  {
  }

  public GeneratorInfo getGeneratorInfo()
  {
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

  JTabbedPane pane = new JTabbedPane();
  
}

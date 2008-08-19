
package cage.generator;


import cage.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import lisken.uitoolbox.*;
import lisken.systoolbox.*;


public class TriangulationsPanel extends GeneratorPanel
{
  private static final boolean debug = false;

  private GeneratorPanel lastChosenPanel = null;

  public TriangulationsPanel()
  {
    pane.addTab("general", new GeneralTriangulationsPanel());
    pane.addTab("eulerian", new EulerianTriangulationsPanel());
    pane.addTab("of the disk", new DiskTriangulationsPanel());
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


class GeneralTriangulationsPanel extends GeneratorPanel
 implements ActionListener
{
  public static final int MIN_VERTICES = 4;
  public static final int MAX_VERTICES = 50;
  public static final int DEFAULT_VERTICES = 4;

  public GeneralTriangulationsPanel()
  {
    setLayout(new GridBagLayout());
    dual = new JCheckBox("dual (cubic planar) graphs");
    dual.setMnemonic(KeyEvent.VK_D);
    dual.setActionCommand("d");
    dual.addActionListener(this);
    add(dual,
     new GridBagConstraints2(1, 0, 2, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 5, 10, 0), 0, 0));
    verticesSlider = new EnhancedSlider();
    verticesSlider.setMinimum(MIN_VERTICES);
    verticesSlider.setMaximum(MAX_VERTICES);
    verticesSlider.setValue(DEFAULT_VERTICES);
    verticesSlider.setMinorTickSpacing(1);
    verticesSlider.setMajorTickSpacing(MAX_VERTICES - MIN_VERTICES);
    verticesSlider.setPaintTicks(true);
    verticesSlider.setPaintLabels(true);
    verticesSlider.setSnapWhileDragging(1);
    verticesSlider.setClickScrollByBlock(false);
    verticesSlider.setSizeFactor(4);
    add(verticesSlider,
     new GridBagConstraints2(1, 1, 2, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 5, 15, 0), 0, 0));
    JLabel verticesLabel = new JLabel("number of vertices");
    verticesLabel.setLabelFor(verticesSlider.slider());
    verticesLabel.setDisplayedMnemonic(KeyEvent.VK_V);
    add(verticesLabel,
     new GridBagConstraints2(0, 1, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 20, 10), 0, 0));
    JLabel minDegLabel = new JLabel("minimum degree");
    add(minDegLabel,
     new GridBagConstraints2(0, 2, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 10), 0, 0));
    minDegGroup = new ButtonGroup();
    JPanel minDegPanel = new JPanel();
    for (int i = 0; i < degButton.length; ++i)
    {
      String is = Integer.toString(i+3);
      degButton[i] = new JRadioButton(is, i == 0);
      degButton[i].setActionCommand("v" + is);
      degButton[i].addActionListener(this);
      minDegGroup.add(degButton[i]);
      minDegPanel.add(degButton[i]);
    }
    add(minDegPanel,
     new GridBagConstraints2(1, 2, 1, 1, 1.0, 1.0,
      GridBagConstraints.EAST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 0), 0, 0));
    add(Box.createHorizontalGlue(),
     new GridBagConstraints2(2, 2, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 0), 0, 0));
    JLabel minConnLabel = new JLabel("minimum connectivity");
    add(minConnLabel,
     new GridBagConstraints2(0, 3, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 10), 0, 0));
    minConnGroup = new ButtonGroup();
    JPanel minConnPanel = new JPanel();
    for (int i = 0; i < connButton.length; ++i)
    {
      String is = Integer.toString(i+1);
      connButton[i] = new JRadioButton(is, i == 2);
      connButton[i].setEnabled(i > 1);
      connButton[i].setActionCommand(is);
      connButton[i].setActionCommand("c" + is);
      connButton[i].addActionListener(this);
      minConnGroup.add(connButton[i]);
      minConnPanel.add(connButton[i]);
    }
    add(minConnPanel,
     new GridBagConstraints2(1, 3, 1, 1, 1.0, 1.0,
      GridBagConstraints.EAST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 0), 0, 0));
    add(Box.createHorizontalGlue(),
     new GridBagConstraints2(2, 3, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 0), 0, 0));
    exactConn = new JCheckBox("restrict connectivity to minimum");
    exactConn.setMnemonic(KeyEvent.VK_R);
    add(exactConn,
     new GridBagConstraints2(1, 4, 2, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 5, 0, 0), 0, 0));
  }

  public void showing()
  {
  }

  public GeneratorInfo getGeneratorInfo()
  {
    Vector genCmd = new Vector();
    String filename = "";

    genCmd.addElement("plantri");
    filename += "tri";
    String v = Integer.toString(verticesSlider.getValue());
    filename += "_" + v;
    if (dual.isSelected()) {
      genCmd.addElement("-d");
      filename += "_d";
    }
    String minConn = minConnGroup.getSelection().getActionCommand().substring(1);
    minConn += exactConn.isSelected() ? "x" : "";
    genCmd.addElement("-c" + minConn);
    filename += "_c" + minConn;
    String minDeg = minDegGroup.getSelection().getActionCommand().substring(1);
    genCmd.addElement("-m" + minDeg);
    filename += "_m" + minDeg;
    genCmd.addElement(v);

    String[][] generator = new String[1][genCmd.size()];
    genCmd.copyInto(generator[0]);

    String[][] embed2D = { { "embed" } };
    String[][] embed3D = { { "embed", "-d3", "-it" } };

    boolean dualGraphs = dual.isSelected();
    return new StaticGeneratorInfo(
     generator,
     EmbedFactory.createEmbedder(true, embed2D, embed3D),
     filename, dualGraphs ? 0 : 3, true);
  }

  public void actionPerformed(ActionEvent e)
  {
    String cmd = e.getActionCommand();
    char deg, conn;
    switch (cmd.charAt(0))
    {
      case 'd':
	if (dual.isSelected()) {
	  for (int i = 0; i < 2; ++i)
	  {
	    connButton[i].setEnabled(true);
	  }
	} else {
	  for (int i = 0; i < 2; ++i)
	  {
	    connButton[i].setEnabled(false);
	  }
	  if (minConnGroup.getSelection().getActionCommand().charAt(1) < '3') {
	    connButton[2].setSelected(true);
	  }
	}
        break;
      case 'c':
	deg = minDegGroup.getSelection().getActionCommand().charAt(1);
	conn = cmd.charAt(1);
	if (deg < conn) {
	  degButton[conn - '0' - 3].setSelected(true);
	}
        break;
      case 'v':
	conn = minConnGroup.getSelection().getActionCommand().charAt(1);
	deg = cmd.charAt(1);
	if (deg < conn) {
	  connButton[deg - '0' - 1].setSelected(true);
	}
        break;
    }
  }

  JCheckBox dual;
  EnhancedSlider verticesSlider;
  ButtonGroup minDegGroup;
  AbstractButton[] degButton = new AbstractButton[3];
  ButtonGroup minConnGroup;
  AbstractButton[] connButton = new AbstractButton[5];
  JCheckBox exactConn;
}


class EulerianTriangulationsPanel extends GeneratorPanel
{
  public static final int MIN_VERTICES = 6;
  public static final int MAX_VERTICES = 40;
  public static final int DEFAULT_VERTICES = 6;

  public EulerianTriangulationsPanel()
  {
    setLayout(new GridBagLayout());
    dual = new JCheckBox("dual (bipartite 3-connected cubic planar) graphs");
    dual.setMnemonic(KeyEvent.VK_D);
    add(dual,
     new GridBagConstraints2(1, 0, 2, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 5, 10, 0), 0, 0));
    verticesSlider = new EnhancedSlider();
    verticesSlider.setMinimum(MIN_VERTICES);
    verticesSlider.setMaximum(MAX_VERTICES);
    verticesSlider.setValue(DEFAULT_VERTICES);
    verticesSlider.setMinorTickSpacing(1);
    verticesSlider.setMajorTickSpacing(MAX_VERTICES - MIN_VERTICES);
    verticesSlider.setPaintTicks(true);
    verticesSlider.setPaintLabels(true);
    verticesSlider.setSnapWhileDragging(1);
    verticesSlider.setClickScrollByBlock(false);
    verticesSlider.setSizeFactor(4);
    add(verticesSlider,
     new GridBagConstraints2(1, 1, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 5, 15, 0), 0, 0));
    JLabel verticesLabel = new JLabel("number of vertices");
    verticesLabel.setLabelFor(verticesSlider.slider());
    verticesLabel.setDisplayedMnemonic(KeyEvent.VK_V);
    add(verticesLabel,
     new GridBagConstraints2(0, 1, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 20, 10), 0, 0));
    JLabel minConnLabel = new JLabel("minimum connectivity");
    add(minConnLabel,
     new GridBagConstraints2(0, 2, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 15, 10), 0, 0));
    minConnGroup = new ButtonGroup();
    JPanel minConnPanel = new JPanel();
    for (int i = 3; i <= 4; ++i)
    {
      String is = Integer.toString(i);
      JRadioButton connButton = new JRadioButton(is, i == 3);
      connButton.setActionCommand(is);
      minConnGroup.add(connButton);
      minConnPanel.add(connButton);
    }
    add(minConnPanel,
     new GridBagConstraints2(1, 2, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 0), 0, 0));
    exactConn = new JCheckBox("restrict connectivity to minimum");
    exactConn.setMnemonic(KeyEvent.VK_R);
    add(exactConn,
     new GridBagConstraints2(1, 3, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 5, 0, 0), 0, 0));
  }

  public void showing()
  {
  }

  public GeneratorInfo getGeneratorInfo()
  {
    Vector genCmd = new Vector();
    String filename = "";

    genCmd.addElement("plantri");
    genCmd.addElement("-b");
    filename += "tri_euler";
    String v = Integer.toString(verticesSlider.getValue());
    filename += "_" + v;
    if (dual.isSelected()) {
      genCmd.addElement("-d");
      filename += "_d";
    }
    String minConn = minConnGroup.getSelection().getActionCommand();
    if (minConn.charAt(0) < '4') minConn += exactConn.isSelected() ? "x" : "";
    genCmd.addElement("-c" + minConn);
    filename += "_c" + minConn;
    genCmd.addElement(v);

    String[][] generator = new String[1][genCmd.size()];
    genCmd.copyInto(generator[0]);

    String[][] embed2D = { { "embed" } };
    String[][] embed3D = { { "embed", "-d3", "-it" } };

    boolean dualGraphs = dual.isSelected();
    return new StaticGeneratorInfo(
     generator,
     EmbedFactory.createEmbedder(true, embed2D, embed3D),
     filename, dualGraphs ? 0 : 3, true);
  }

  JCheckBox dual;
  EnhancedSlider verticesSlider;
  ButtonGroup minConnGroup;
  JCheckBox exactConn;
}


class DiskTriangulationsPanel extends GeneratorPanel
 implements ActionListener
{
  public static final int MIN_VERTICES = 4;
  public static final int MAX_VERTICES = 22;
  public static final int DEFAULT_VERTICES = 4;
  public static final int MIN_BOUNDARY_SEGMENTS = 3;
  public static final int DEFAULT_BOUNDARY_SEGMENTS = 3;
  private static final String[] keys = new String[] { "blr", "do" };

  static final boolean enableReembed2D =
   CaGe.getCaGePropertyAsBoolean("PlantriT.Disk.EnableReembed2D", false);

  public static String[] permission = new String[]
   { "forbidden", "allowed", "required" };

  public DiskTriangulationsPanel()
  {
    setLayout(new GridBagLayout());
    verticesSlider = new EnhancedSlider();
    verticesSlider.setMinimum(MIN_VERTICES);
    verticesSlider.setMaximum(MAX_VERTICES);
    verticesSlider.setValue(DEFAULT_VERTICES);
    verticesSlider.setMinorTickSpacing(1);
    verticesSlider.setMajorTickSpacing(MAX_VERTICES - MIN_VERTICES);
    verticesSlider.setPaintTicks(true);
    verticesSlider.setPaintLabels(true);
    verticesSlider.setSnapWhileDragging(1);
    verticesSlider.setClickScrollByBlock(false);
    verticesSlider.setSizeFactor(6);
    add(verticesSlider,
     new GridBagConstraints2(1, 0, 1, 1, 1.0, 1.0,
      GridBagConstraints.EAST, GridBagConstraints.NONE,
      new Insets(0, 5, 15, 0), 0, 0));
    JLabel verticesLabel = new JLabel("number of vertices");
    verticesLabel.setLabelFor(verticesSlider.slider());
    verticesLabel.setDisplayedMnemonic(KeyEvent.VK_V);
    add(verticesLabel,
     new GridBagConstraints2(0, 0, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 20, 10), 0, 0));
    boundarySegmentsSlider = new EnhancedSlider();
    boundarySegmentsSlider.setMinimum(MIN_BOUNDARY_SEGMENTS);
    boundarySegmentsSlider.setMaximum(MAX_VERTICES);
    boundarySegmentsSlider.setValue(DEFAULT_BOUNDARY_SEGMENTS);
    boundarySegmentsSlider.setMinorTickSpacing(1);
    boundarySegmentsSlider.setMajorTickSpacing(MAX_VERTICES - MIN_BOUNDARY_SEGMENTS);
    boundarySegmentsSlider.setPaintTicks(true);
    boundarySegmentsSlider.setPaintLabels(true);
    boundarySegmentsSlider.setSnapWhileDragging(1);
    boundarySegmentsSlider.setClickScrollByBlock(false);
    boundarySegmentsSlider.setSizeFactor(6);
    boundarySegmentsSlider.setEnabled(false);
    boundarySegmentsSlider.getValueLabel().setForeground(getBackground());
    new MinMaxEqListener
     (boundarySegmentsSlider.getModel(), verticesSlider.getModel(), false);
    add(boundarySegmentsSlider,
     new GridBagConstraints2(1, 1, 1, 1, 1.0, 1.0,
      GridBagConstraints.EAST, GridBagConstraints.NONE,
      new Insets(0, 5, 15, 0), 0, 0));
    JLabel boundarySegmentsLabel = new JLabel("boundary segments");
    boundarySegmentsLabel.setLabelFor(boundarySegmentsSlider.slider());
    boundarySegmentsLabel.setDisplayedMnemonic(KeyEvent.VK_S);
    add(boundarySegmentsLabel,
     new GridBagConstraints2(0, 1, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 20, 10), 0, 0));
    boundarySegmentsAll = new JCheckBox("any number", true);
    boundarySegmentsAll.setMnemonic(KeyEvent.VK_A);
    boundarySegmentsAll.setActionCommand("a");
    boundarySegmentsAll.addActionListener(this);
    add(boundarySegmentsAll,
     new GridBagConstraints2(2, 1, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 10, 15, 0), 0, 0));
    JLabel chordsLabel = new JLabel("chords");
    add(chordsLabel,
     new GridBagConstraints2(0, 2, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 10), 0, 0));
    chordsGroup = new ButtonGroup();
    JPanel chordsPanel = new JPanel();
    for (int i = 0; i < chordsButton.length; ++i)
    {
      chordsButton[i] = new JRadioButton(permission[i], i == 0);
      chordsButton[i].setMnemonic(keys[0].charAt(i));
      chordsButton[i].setActionCommand("c" + i);
      chordsButton[i].addActionListener(this);
      chordsGroup.add(chordsButton[i]);
      chordsPanel.add(chordsButton[i]);
    }
    add(chordsPanel,
     new GridBagConstraints2(1, 2, 3, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 0), 0, 0));
    JLabel vertices2Label = new JLabel("2-valent vertices on boundary");
    add(vertices2Label,
     new GridBagConstraints2(0, 3, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 10), 0, 0));
    vertices2Group = new ButtonGroup();
    JPanel vertices2Panel = new JPanel();
    for (int i = 0; i < vertices2Button.length; ++i)
    {
      vertices2Button[i] = new JRadioButton(permission[i], i == 0);
      vertices2Button[i].setMnemonic(keys[1].charAt(i));
      vertices2Button[i].setActionCommand("v" + i);
      vertices2Button[i].addActionListener(this);
      vertices2Group.add(vertices2Button[i]);
      vertices2Panel.add(vertices2Button[i]);
    }
    add(vertices2Panel,
     new GridBagConstraints2(1, 3, 3, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 0), 0, 0));
  }

  public void showing()
  {
  }

  public GeneratorInfo getGeneratorInfo()
  {
    Vector genCmd = new Vector();
    String filename = "";

    genCmd.addElement("plantri");
    genCmd.addElement("-P" + (boundarySegmentsAll.isSelected() ? "" : Integer.toString(boundarySegmentsSlider.getValue())));
    filename += "tri_disk" + (boundarySegmentsAll.isSelected() ? "" : ("_" + boundarySegmentsSlider.getValue()));
    String v = Integer.toString(verticesSlider.getValue());
    filename += "_" + v;
    char chords = chordsGroup.getSelection().getActionCommand().charAt(1);
    char vertices2 = vertices2Group.getSelection().getActionCommand().charAt(1);
    String c = "c" + (chords == '0' ? "3" : "2") + (chords == '2' ? "x" : "");
    genCmd.addElement("-" + c);
    filename += "_" + c;
    String m = chords == '0' ? "m3" : vertices2 == '1' ? "m2" : "";
    if (m.length() > 0) {
      genCmd.addElement("-" + m);
      filename += "_" + m;
    }
    genCmd.addElement(v);

    String[][] generator = new String[1][genCmd.size()];
    genCmd.copyInto(generator[0]);

    String[][] embed2D = { { "embed", "-b2,1", "-a-" } };
    String[][] embed3D = { { "embed", "-d3", "-it" } };

    return new StaticGeneratorInfo(
     generator,
     EmbedFactory.createEmbedder(true, embed2D, embed3D),
     filename, 3, enableReembed2D);
  }

  public void actionPerformed(ActionEvent e)
  {
    String cmd = e.getActionCommand();
    boolean forbidden;
    switch (cmd.charAt(0))
    {
      case 'a':
	boolean enabled = ! boundarySegmentsAll.isSelected();
	boundarySegmentsSlider.setEnabled(enabled);
	boundarySegmentsSlider.getValueLabel().setForeground(
	 enabled ? Color.black : getBackground());
	if (enabled) boundarySegmentsSlider.slider().requestFocus();
        break;
      case 'c':
	forbidden = cmd.charAt(1) == '0';
	if (forbidden) {
	  vertices2Button[0].setSelected(true);
	}
        break;
      case 'v':
	forbidden = cmd.charAt(1) == '0';
	char chords =
	 chordsGroup.getSelection().getActionCommand().charAt(1);
	if (chords == '0' && ! forbidden) {
	  chordsButton[1].setSelected(true);
	}
        break;
    }
  }

  EnhancedSlider verticesSlider;
  EnhancedSlider boundarySegmentsSlider;
  JCheckBox boundarySegmentsAll;
  AbstractButton[] chordsButton = new AbstractButton[3];
  ButtonGroup chordsGroup;
  AbstractButton[] vertices2Button = new AbstractButton[2];
  ButtonGroup vertices2Group;
}


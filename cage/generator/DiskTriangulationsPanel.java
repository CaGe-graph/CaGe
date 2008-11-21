package cage.generator;

import cage.CaGe;
import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;

import java.awt.Color;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.util.Vector;
import javax.swing.AbstractButton;
import javax.swing.ButtonGroup;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;

import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.GridBagConstraints2;
import lisken.uitoolbox.MinMaxEqListener;

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
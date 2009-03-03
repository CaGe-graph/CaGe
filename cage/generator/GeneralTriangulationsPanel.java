package cage.generator;

import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.util.Vector;
import javax.swing.AbstractButton;
import javax.swing.Box;
import javax.swing.ButtonGroup;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;

import lisken.uitoolbox.EnhancedSlider;

class GeneralTriangulationsPanel extends GeneratorPanel
 implements ActionListener
{
  public static final int MIN_VERTICES = 4;
  public static final int MAX_VERTICES = 50;
  public static final int DEFAULT_VERTICES = 4;

  public GeneralTriangulationsPanel()
  {
    setLayout(new GridBagLayout());
    verticesSlider = new EnhancedSlider();
    verticesSlider.setMinimum(MIN_VERTICES);
    verticesSlider.setMaximum(MAX_VERTICES);
    verticesSlider.setValue(DEFAULT_VERTICES);
    verticesSlider.setMinorTickSpacing(2); //vertices has to be even
    verticesSlider.setMajorTickSpacing(MAX_VERTICES - MIN_VERTICES);
    verticesSlider.setPaintTicks(true);
    verticesSlider.setPaintLabels(true);
    verticesSlider.setSnapWhileDragging(2);
    verticesSlider.setClickScrollByBlock(false);
    verticesSlider.setSnapToTicks(true); //vertices has to be even
    verticesSlider.setSizeFactor(4);
    add(verticesSlider,
     new GridBagConstraints(1, 1, 2, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 5, 15, 0), 0, 0));
    JLabel verticesLabel = new JLabel("number of vertices");
    verticesLabel.setLabelFor(verticesSlider.slider());
    verticesLabel.setDisplayedMnemonic(KeyEvent.VK_V);
    add(verticesLabel,
     new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 20, 10), 0, 0));
    JLabel minDegLabel = new JLabel("minimum face size");
    add(minDegLabel,
     new GridBagConstraints(0, 2, 1, 1, 1.0, 1.0,
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
     new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0,
      GridBagConstraints.EAST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 0), 0, 0));
    add(Box.createHorizontalGlue(),
     new GridBagConstraints(2, 2, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 0), 0, 0));
    JLabel minConnLabel = new JLabel("minimum connectivity of the dual");
    add(minConnLabel,
     new GridBagConstraints(0, 3, 1, 1, 1.0, 1.0,
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
     new GridBagConstraints(1, 3, 1, 1, 1.0, 1.0,
      GridBagConstraints.EAST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 0), 0, 0));
    add(Box.createHorizontalGlue(),
     new GridBagConstraints(2, 3, 1, 1, 1.0, 1.0,
      GridBagConstraints.WEST, GridBagConstraints.NONE,
      new Insets(0, 0, 10, 0), 0, 0));
    exactConn = new JCheckBox("only graphs with connectivity exactly the given minimum");
    exactConn.setMnemonic(KeyEvent.VK_R);
    add(exactConn,
     new GridBagConstraints(1, 4, 2, 1, 1.0, 1.0,
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
	//plantri takes the number of faces as argument, so we use the euler formula to derive it from the number of vertices
    String faces = Integer.toString(verticesSlider.getValue()/2 + 2);
    filename += "_" + faces;
    if (dual) {
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
    genCmd.addElement(faces);

    String[][] generator = new String[1][genCmd.size()];
    genCmd.copyInto(generator[0]);

    String[][] embed2D = { { "embed" } };
    String[][] embed3D = { { "embed", "-d3", "-it" } };

    return new StaticGeneratorInfo(
     generator,
     EmbedFactory.createEmbedder(true, embed2D, embed3D),
     filename, dual ? 0 : 3, true);
  }

  public void actionPerformed(ActionEvent e)
  {
    String cmd = e.getActionCommand();
    char deg, conn;
    switch (cmd.charAt(0))
    {
      case 'd':
	if (dual) {
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
  
  public void setDual(boolean dual) {
      this.dual = dual;
  }

  boolean dual;
  EnhancedSlider verticesSlider;
  ButtonGroup minDegGroup;
  AbstractButton[] degButton = new AbstractButton[3];
  ButtonGroup minConnGroup;
  AbstractButton[] connButton = new AbstractButton[5];
  JCheckBox exactConn;
  
}

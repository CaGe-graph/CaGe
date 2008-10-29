
package cage.generator;

import cage.ElementRule;
import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;
import cage.ValencyElementRule;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.EventListener;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Vector;
import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import javax.swing.JToggleButton;
import javax.swing.SwingConstants;
import javax.swing.UIManager;
import lisken.systoolbox.Integer2;
import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.GridBagConstraints2;
import lisken.uitoolbox.MinMaxEqListener;
import lisken.uitoolbox.UItoolbox;




public class CPFPanel extends GeneratorPanel
{
  public static final int minAtoms = 4;
  public static final int maxAtoms = 250;
  public static final int minPolygonFaces = 3;
  public static final int maxPolygonFaces = 40;

  JSeparator sep1 = new JSeparator(SwingConstants.HORIZONTAL);
  JSeparator sep2 = new JSeparator(SwingConstants.HORIZONTAL);
  JPanel CPFFacesPanel = new JPanel();
  JPanel CPFFaceOptionsPanel = new JPanel();
  JPanel CPFExtrasPanel = new JPanel();
  EnhancedSlider facesSlider = new EnhancedSlider();
  Hashtable facesLabels = new Hashtable();
  JPanel CPFAtomsPanel = new JPanel();
  JLabel minAtomsLabel = new JLabel();
  JLabel maxAtomsLabel = new JLabel();
  EnhancedSlider minAtomsSlider = new EnhancedSlider();
  EnhancedSlider maxAtomsSlider = new EnhancedSlider();
  JCheckBox minEqMax = new JCheckBox();
  GridBagLayout CPFAtomsPanelLayout = new GridBagLayout();
  GridBagLayout CPFPanelLayout = new GridBagLayout();
  GridBagLayout CPFFaceOptionsLayout = new GridBagLayout();
  GridBagLayout CPFExtrasLayout = new GridBagLayout();
  JLabel faceTypeLabel = new JLabel();
  JToggleButton facesButton = new JToggleButton();
  JLabel includedFacesLabel = new JLabel();
  GridBagLayout CPFFacesLayout = new GridBagLayout();
  JLabel dummyLabel = new JLabel();
  JCheckBox dual = new JCheckBox();
  JCheckBox altECC = new JCheckBox();
  JCheckBox maxPathFace = new JCheckBox();
  JCheckBox faceStats = new JCheckBox();
  JCheckBox patchStats = new JCheckBox();
  JCheckBox conn1 = new JCheckBox();
  JCheckBox conn2 = new JCheckBox();
  JCheckBox conn3 = new JCheckBox();
/* --- "Cases and Priorities" disabled ---
  JToggleButton selectCasesPrios = new JToggleButton();
*/

  GonOptionsMap gonOptionsMap;

/* --- "Cases and Priorities" disabled ---
  OrderedChoice casesPriosChoice;
  String cases, nonCases;
*/

  public CPFPanel()
  {
    try
    {
      jbInit();
    }
    catch (Exception ex)
    {
      ex.printStackTrace();
    }
  }


  void jbInit() throws Exception
  {
    setLayout(CPFPanelLayout);
    CPFExtrasPanel.setLayout(CPFExtrasLayout);
    CPFAtomsPanel.setLayout(CPFAtomsPanelLayout);
    CPFFaceOptionsPanel.setLayout(CPFFaceOptionsLayout);
    minAtomsLabel.setText("minimum number of Atoms");
    minAtomsLabel.setLabelFor(minAtomsSlider.slider());
    minAtomsLabel.setDisplayedMnemonic(KeyEvent.VK_N);
    maxAtomsLabel.setText("maximum number of Atoms");
    maxAtomsLabel.setLabelFor(maxAtomsSlider.slider());
    maxAtomsLabel.setDisplayedMnemonic(KeyEvent.VK_X);
    minAtomsSlider.setMajorTickSpacing(maxAtoms - minAtoms);
    minAtomsSlider.setSnapToTicks(true);
    minAtomsSlider.setMinimum(minAtoms);
    minAtomsSlider.setMaximum(maxAtoms);
    minAtomsSlider.setValue(minAtomsSlider.getMinimum());
    minAtomsSlider.setMinorTickSpacing(2 - (maxAtoms - minAtoms) % 2);
    minAtomsSlider.setPaintMinorTicks(false);
    minAtomsSlider.setPaintLabels(true);
    minAtomsSlider.setPaintTicks(true);
    minAtomsSlider.setSnapWhileDragging(minAtomsSlider.getMinorTickSpacing());
    minAtomsSlider.setClickScrollByBlock(false);
    maxAtomsSlider.setMajorTickSpacing(maxAtoms - minAtoms);
    maxAtomsSlider.setSnapToTicks(true);
    maxAtomsSlider.setMinimum(minAtoms);
    maxAtomsSlider.setMaximum(maxAtoms);
    maxAtomsSlider.setValue(maxAtomsSlider.getMinimum());
    maxAtomsSlider.setMinorTickSpacing(2 - (maxAtoms - minAtoms) % 2);
    maxAtomsSlider.setPaintMinorTicks(false);
    maxAtomsSlider.setPaintLabels(true);
    maxAtomsSlider.setPaintTicks(true);
    maxAtomsSlider.setSnapWhileDragging(maxAtomsSlider.getMinorTickSpacing());
    maxAtomsSlider.setClickScrollByBlock(false);
    minEqMax.setText("min = max");
    minEqMax.setSelected(true);
    minEqMax.setMnemonic(KeyEvent.VK_M);
    EventListener l = new MinMaxEqListener(minAtomsSlider.getModel(), maxAtomsSlider.getModel(), minEqMax.getModel(), false);
    facesLabels = facesSlider.createStandardLabels(5, 10);
    facesLabels.put(new Integer(3), new JLabel("3"));
    facesLabels.put(new Integer(6), new JLabel("6"));
    faceTypeLabel.setText("Face Type");
    faceTypeLabel.setLabelFor(facesSlider.slider());
    faceTypeLabel.setDisplayedMnemonic(KeyEvent.VK_F);
//    faceTypeLabel.setAlignmentX((float) 0.5);
    facesButton.setText("include this Face Type");
//    facesButton.setAlignmentX((float) 0.5);
    facesButton.setMnemonic(KeyEvent.VK_I);
    facesButton.setBorder(BorderFactory.createCompoundBorder(
     facesButton.getBorder(),
     BorderFactory.createEmptyBorder(5, 0, 5, 0)));
    facesSlider.setOrientation(SwingConstants.HORIZONTAL);
    facesSlider.setLabelTable(facesLabels);
    facesSlider.setMinorTickSpacing(1);
    facesSlider.setSnapToTicks(true);
    facesSlider.setPaintTicks(true);
    facesSlider.setPaintLabels(true);
    facesSlider.setMaximum(maxPolygonFaces);
    facesSlider.setMinimum(minPolygonFaces);
    facesSlider.setValue(facesSlider.getMinimum() + 1);
    facesSlider.setMajorTickSpacing(facesSlider.getMaximum() - facesSlider.getMinimum());
    facesSlider.setSnapWhileDragging(1);
    facesSlider.setSizeFactor(12);
    facesSlider.setClickScrollByBlock(false);
    facesSlider.slider().addKeyListener(new KeyAdapter() {
      public void keyPressed(KeyEvent e)
      {
        if (e.getModifiers() != 0) return;
	int n = e.getKeyCode() - KeyEvent.VK_0;
	if (n < 0 || n > 9) return;
	n = (n + 7) % 10 + 3;
	facesSlider.setValue(n);
      }
    });
    gonOptionsMap = new GonOptionsMap(CPFFaceOptionsPanel, facesSlider.slider(), facesSlider.getModel(), facesButton);
    gonOptionsMap.setGonIncluded(facesSlider.getMinimum(), true);
    includedFacesLabel.setText("included Face Types:");
//    includedFacesLabel.setLabelFor(CPFFaceOptionsPanel);
//    includedFacesLabel.setDisplayedMnemonic(KeyEvent.VK_L);
    ActionListener connDualListener = new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
	boolean c[] = { conn1.isSelected(), conn2.isSelected(), conn3.isSelected() };
	AbstractButton conn[] = { conn1, conn2, conn3 };
	int cs = (c[0] ? 1 : 0) + (c[1] ? 1 : 0) + (c[2] ? 1 : 0);
	boolean d = dual.isSelected();
	char a;
	switch (a = ((AbstractButton) e.getSource()).getActionCommand().charAt(0))
	{
	  case 'd':
	   if (d) {
	     conn[0].setSelected(c[0] = false);
	     conn[1].setSelected(c[1] = false);
	     conn[2].setSelected(c[2] = true);
	     cs = 1;
	   }
	   break;
	 case '1':
	 case '2':
	 case '3':
	   int i = a - '1';
	   if (cs == 0) {
	     conn[i].setSelected(c[i] = true);
	     cs = 1;
	   }
	   if (d && ! (c[2] && cs == 1)) {
	     dual.setSelected(false);
	   }
	}
      }
    };
    dual.setText("output Dual graph (triangulation)");
    dual.setMnemonic(KeyEvent.VK_D);
    dual.setActionCommand("d");
    dual.addActionListener(connDualListener);
    altECC.setText("alternative ECC");
    altECC.setMnemonic(KeyEvent.VK_E);
    maxPathFace.setText("Maximal Pathface");
    maxPathFace.setMnemonic(KeyEvent.VK_A);
    faceStats.setText("Face Statistics");
    faceStats.setMnemonic(KeyEvent.VK_S);
    patchStats.setText("Patch Statistics");
    patchStats.setMnemonic(KeyEvent.VK_P);
    conn1.setText("1-connected graphs");
    conn1.setMnemonic(KeyEvent.VK_1);
    conn1.setActionCommand("1");
    conn1.setSelected(false);
    conn1.addActionListener(connDualListener);
    conn2.setText("2-connected graphs");
    conn2.setMnemonic(KeyEvent.VK_2);
    conn2.setSelected(false);
    conn2.setActionCommand("2");
    conn2.addActionListener(connDualListener);
    conn3.setText("3-connected graphs");
    conn3.setMnemonic(KeyEvent.VK_3);
    conn3.setSelected(true);
    conn3.setActionCommand("3");
    conn3.addActionListener(connDualListener);
/* --- "Cases and Priorities" disabled ---
    selectCasesPrios.setText("Cases and Priorities");
    selectCasesPrios.setMnemonic(KeyEvent.VK_C);
    selectCasesPrios.setBorder(BorderFactory.createCompoundBorder(
     selectCasesPrios.getBorder(),
     BorderFactory.createEmptyBorder(5, 0, 5, 0)));
    selectCasesPrios.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e)
      {
        selectCasesPrios();
      }
    });
    casesPriosChoice = new OrderedChoice(new String[] { "case 1", "case 2", "case 3" });
    casesPriosChoice.setPositions(new int[] { 0, 1, 2 });
    casesPriosChoice.allowEmptySelection(false);
    cases = "123";
*/
    CPFFacesPanel.setLayout(CPFFacesLayout);
    CPFFacesPanel.add(faceTypeLabel, new GridBagConstraints2(0, 0, 2, 1, 1.0, 1.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 5, 5, 0), 0, 0));
    CPFFacesPanel.add(facesSlider, new GridBagConstraints2(0, 1, 1, 1, 1.0, 1.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
    CPFFacesPanel.add(facesButton, new GridBagConstraints2(1, 1, 1, 1, 0.0010, 1.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 30, 0, 5), 0, 0));
//    CPFFacesPanel.setBorder(BorderFactory.createEtchedBorder());
//    CPFFaceOptionsPanel.setBorder(BorderFactory.createEtchedBorder());
//    CPFExtrasPanel.setBorder(BorderFactory.createEtchedBorder());
    CPFAtomsPanel.add(minAtomsLabel, new GridBagConstraints2(0, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 5, 5, 0), 0, 0));
    CPFAtomsPanel.add(maxAtomsLabel, new GridBagConstraints2(1, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 5, 5, 0), 0, 0));
    CPFAtomsPanel.add(minAtomsSlider, new GridBagConstraints2(0, 1, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 20), 0, 0));
    CPFAtomsPanel.add(maxAtomsSlider, new GridBagConstraints2(1, 1, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 20), 0, 0));
    CPFAtomsPanel.add(minEqMax, new GridBagConstraints2(2, 1, 1, 1, 0.001, 1.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
    CPFExtrasPanel.add(dummyLabel, new GridBagConstraints2(0, 0, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
    CPFExtrasPanel.add(dual, new GridBagConstraints2(1, 0, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
    CPFExtrasPanel.add(altECC, new GridBagConstraints2(1, 1, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
    CPFExtrasPanel.add(maxPathFace, new GridBagConstraints2(1, 2, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
    CPFExtrasPanel.add(faceStats, new GridBagConstraints2(1, 3, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
    CPFExtrasPanel.add(patchStats, new GridBagConstraints2(1, 4, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
    CPFExtrasPanel.add(conn1, new GridBagConstraints2(3, 0, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
    CPFExtrasPanel.add(conn2, new GridBagConstraints2(3, 1, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
    CPFExtrasPanel.add(conn3, new GridBagConstraints2(3, 2, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
/* --- "Cases and Priorities" disabled ---
    CPFExtrasPanel.add(selectCasesPrios, new GridBagConstraints2(3, 3, 1, 3, 1.0, 1.0
            ,GridBagConstraints.NORTHWEST, GridBagConstraints.NONE, new Insets(10, 0, 0, 0), 0, 0));
*/
    this.add(CPFAtomsPanel, new GridBagConstraints2(0, 0, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
    this.add(sep1, new GridBagConstraints2(0, 1, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(20, 0, 20, 0), 0, 0));
    this.add(CPFFacesPanel, new GridBagConstraints2(0, 2, 1, 1, 1.0, 1.0
            ,GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
    this.add(includedFacesLabel, new GridBagConstraints2(0, 3, 1, 1, 1.0, 1.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(20, 5, 0, 0), 0, 0));
    this.add(CPFFaceOptionsPanel, new GridBagConstraints2(0, 4, 1, 1, 1.0, 1.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 10, 10, 0), 0, 0));
    this.add(sep2, new GridBagConstraints2(0, 5, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(20, 0, 20, 0), 0, 0));
    this.add(CPFExtrasPanel, new GridBagConstraints2(0, 6, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
  }

/*
  void selectCasesPrios()
  {
    selectCasesPrios.setSelected(true);
    casesPriosChoice.runDialog("CaGe - CPF - select cases and priorities");
    if (casesPriosChoice.getDialogCompleted()) {
      cases = "";
      Object[] selection = casesPriosChoice.getSelection();
      for (int i = 0; i < selection.length; ++i)
      {
	cases += ((String) selection[i]).charAt(5);
      }
      nonCases = "";
      for (int i = 1; i <= 3; ++i)
      {
        if (cases.indexOf('0' + i) < 0) {
	  nonCases += i;
	}
      }
    }
    selectCasesPrios.setSelected(! cases.equals("123"));
  }
*/


  public GeneratorInfo getGeneratorInfo()
  {
    String[][] generator, embed2D, embed3D;
    String filename;
    int maxFacesize;

    String c;
    Vector genV = new Vector(), fileV = new Vector();

    int min = minAtomsSlider.getValue();
    int max = maxAtomsSlider.getValue();

    genV.addElement("CPF");
    genV.addElement("stdout");
    genV.addElement("n");
    genV.addElement(Integer.toString(max));
    fileV.addElement("cpf");
    fileV.addElement("n" + max);

    if (min != max) {
      genV.addElement("s");
      genV.addElement(Integer.toString(min));
      fileV.addElement("s" + min);
    }

    Iterator it = gonOptionsMap.values().iterator();
    while (it.hasNext())
    {
      GonOption gonOption = (GonOption) it.next();
      if (! gonOption.isActive()) continue;
      genV.addElement("f");
      genV.addElement(Integer.toString(gonOption.faces));
      String s = "f" + gonOption.faces;
      if (gonOption.isLimited) {
        genV.addElement("+" + gonOption.min);
        genV.addElement("-" + gonOption.max);
        s = s + "+" + gonOption.min + "-" + gonOption.max;
      }
      fileV.addElement(s);
    }

    cage.Utils.addIfSelected(genV, dual, "dual");
    cage.Utils.addIfSelected(genV, altECC, "alt");
    cage.Utils.addIfSelected(genV, maxPathFace, "pathface_max");
    cage.Utils.addIfSelected(genV, faceStats, "facestat");
    cage.Utils.addIfSelected(genV, patchStats, "patchstat");

/*
    c = "t";
    for (int i = 1; i <= 3; ++i)
    {
      if (cases.indexOf('0' + i) < 0) {
        genV.addElement("no_" + i);
      } else {
	c += i;
      }
    }
    if (c.length() < 4) fileV.addElement(c);
    if (! cases.equals("123")) {
      genV.addElement("priority");
      genV.addElement(cases + nonCases);
      fileV.addElement("p" + cases + nonCases);
    }
*/

    cage.Utils.addIfSelected(fileV, maxPathFace, "pmax");
    cage.Utils.addIfSelected(fileV, altECC, "alt");

    c = "c";
    if (conn1.isSelected()) {
      genV.addElement("con");
      genV.addElement("1");
      c = c + "1";
    }
    if (conn2.isSelected()) {
      genV.addElement("con");
      genV.addElement("2");
      c = c + "2";
    }
    if (conn3.isSelected()) {
      genV.addElement("con");
      genV.addElement("3");
      c = c + "3";
    }
    if (c.length() < 4) fileV.addElement(c);

    cage.Utils.addIfSelected(fileV, dual, "dual");

    generator = new String[1][genV.size()];
    genV.copyInto(generator[0]);
    String[] array = new String[fileV.size()];
    fileV.copyInto(array);
    filename = Systoolbox.join(array, "_");

    embed2D = new String[][] { { "embed" } };
    embed3D = new String[][] { { "embed", "-d3", "-it" } };

    if (dual.isSelected()) {
      maxFacesize = 3;
    } else {
      maxFacesize = ((Integer2) gonOptionsMap.lastKey()).intValue();
    }

    ElementRule rule = new ValencyElementRule("H O C Si N S I");

    return new StaticGeneratorInfo(
     generator,
     EmbedFactory.createEmbedder(true, embed2D, embed3D),
     filename, maxFacesize, rule);
  }

  public void showing()
  {
  }


  public static void main(String[] args)
  {
    try  {
      //com.sun.java.swing.UIManager.setLookAndFeel(new com.sun.java.swing.plaf.windows.WindowsLookAndFeel());
      //com.sun.java.swing.UIManager.setLookAndFeel(new com.sun.java.swing.plaf.motif.MotifLookAndFeel());
      //com.sun.java.swing.UIManager.setLookAndFeel(new com.sun.java.swing.plaf.metal.MetalLookAndFeel());
      UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
    }
    catch (Exception e) {
    }
    final CPFPanel p = new CPFPanel();
    final JFrame f = new JFrame("Output Dialog");
    f.addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        f.setVisible(false);
	GeneratorInfo info = p.getGeneratorInfo();
        JOptionPane.showInputDialog(null, "Command Line", "CPF results",
         JOptionPane.PLAIN_MESSAGE, null, null,
         "Command: " + Systoolbox.join(info.getGenerator()[0], " ") + "\nOutput: " + info.getFilename());
        System.exit(0);
      }
    });
    UItoolbox.addExitOnEscape(f);
    f.setContentPane(p);
    f.show();
    f.pack();
    f.show();
//    f.setResizable(false);
  }

}


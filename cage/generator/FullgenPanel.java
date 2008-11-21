

package cage.generator;

import cage.ElementRule;
import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;
import cage.ValencyElementRule;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.Vector;
import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import javax.swing.JToggleButton;
import javax.swing.SwingConstants;
import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.FlaggedJDialog;
import lisken.uitoolbox.MinMaxEqListener;
import lisken.uitoolbox.PushButtonDecoration;
import lisken.uitoolbox.UItoolbox;


public class FullgenPanel extends GeneratorPanel
 implements ActionListener
{
  public static final int minAtoms = 20;
  public static final int maxAtoms = 250;
  private static final int defaultAtoms = 60;

  boolean embedderIsConstant = false;

  JPanel FullgenAtomsPanel = new JPanel();
  JLabel minAtomsLabel = new JLabel();
  JLabel maxAtomsLabel = new JLabel();
  EnhancedSlider minAtomsSlider = new EnhancedSlider();
  EnhancedSlider maxAtomsSlider = new EnhancedSlider();
  JSeparator sep1 = new JSeparator(SwingConstants.HORIZONTAL);
  JPanel FullgenExtrasPanel = new JPanel();
  JCheckBox minEqMax = new JCheckBox();
  JCheckBox ipr = new JCheckBox();
  JCheckBox dual = new JCheckBox();
  JCheckBox spiralStats = new JCheckBox();
  JCheckBox symmStats = new JCheckBox();
  JToggleButton symmetryFilterButton = new JToggleButton();
  JButton symmetriesOkButton = new JButton();
  JButton symmetriesAllButton = new JButton();
/* --- "select cases" disabled ---
  JToggleButton selectCasesButton = new JToggleButton();
  ButtonGroup casesButtonGroup = new ButtonGroup();
  AbstractButton[] caseButton = new AbstractButton[4];
  FlaggedJDialog casesDialog = new FlaggedJDialog((Frame) null, "Fullgen - select cases", true);
*/
  FlaggedJDialog symmetriesDialog = new FlaggedJDialog((Frame) null, "Fullgen - symmetry filter", true);

  final String[] symmetry = new String[] {
   "C1",  "C2",  "Ci",  "Cs",
   "C3",  "D2",  "S4",  "C2v",
   "C2h", "D3",  "S6",  "C3v",
   "C3h", "D2h", "D2d", "D5",
   "D6",  "D3h", "D3d", "T",
   "D5h", "D5d", "D6h", "D6d",
   "Td",  "Th",  "I",   "Ih"
  };
  final int symmetries = symmetry.length;
  AbstractButton[] symmetryButton = new AbstractButton[symmetries];
  boolean[] selectedSymmetry = new boolean[symmetries];
  final int symmRows = 4;

  int cases = 0, selectedSymmetries = 0;

  public FullgenPanel()
  {
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
    minAtomsSlider.setMinorTickSpacing(2 - (maxAtoms - minAtoms) % 2);
    minAtomsSlider.setPaintMinorTicks(false);
    minAtomsSlider.setPaintLabels(true);
    minAtomsSlider.setPaintTicks(true);
    minAtomsSlider.setSnapWhileDragging(minAtomsSlider.getMinorTickSpacing());
    minAtomsSlider.setClickScrollByBlock(false);
    minAtomsSlider.setValue(defaultAtoms);
    maxAtomsSlider.setMajorTickSpacing(maxAtoms - minAtoms);
    maxAtomsSlider.setSnapToTicks(true);
    maxAtomsSlider.setMinimum(minAtoms);
    maxAtomsSlider.setMaximum(maxAtoms);
    maxAtomsSlider.setMinorTickSpacing(2 - (maxAtoms - minAtoms) % 2);
    maxAtomsSlider.setPaintMinorTicks(false);
    maxAtomsSlider.setPaintLabels(true);
    maxAtomsSlider.setPaintTicks(true);
    maxAtomsSlider.setSnapWhileDragging(maxAtomsSlider.getMinorTickSpacing());
    maxAtomsSlider.setClickScrollByBlock(false);
    maxAtomsSlider.setValue(defaultAtoms);
    minEqMax.setText("min = max");
    minEqMax.setSelected(true);
    minEqMax.setMnemonic(KeyEvent.VK_M);
    new MinMaxEqListener(minAtomsSlider.getModel(), maxAtomsSlider.getModel(), minEqMax.getModel(), false);
    FullgenAtomsPanel.setLayout(new GridBagLayout());
    ipr.setText("Isolated Pentagons (ipr)");
    ipr.setMnemonic(KeyEvent.VK_I);
    dual.setText("output Dual graph (triangulation)");
    dual.setMnemonic(KeyEvent.VK_D);
    dual.setActionCommand("Dual");
    dual.addActionListener(this);
    spiralStats.setText("Spiral Statistics");
    spiralStats.setMnemonic(KeyEvent.VK_P);
    symmStats.setText("Symmetry Statistics");
    symmStats.setMnemonic(KeyEvent.VK_Y);
    symmetryFilterButton.setText("Symmetry Filter");
    symmetryFilterButton.setMnemonic(KeyEvent.VK_F);
    symmetryFilterButton.setActionCommand("F");
    symmetryFilterButton.setBorder(BorderFactory.createCompoundBorder(
     symmetryFilterButton.getBorder(),
     BorderFactory.createEmptyBorder(5, 0, 5, 0)));
    symmetryFilterButton.addActionListener(this);
/* --- "select cases" disabled ---
    selectCasesButton.setText("Select Cases");
    selectCasesButton.setMnemonic(KeyEvent.VK_C);
    selectCasesButton.setActionCommand("c");
    selectCasesButton.setBorder(BorderFactory.createCompoundBorder(
     selectCasesButton.getBorder(),
     BorderFactory.createEmptyBorder(5, 0, 5, 0)));
    selectCasesButton.addActionListener(this);
    selectCasesButton.setPreferredSize(symmetryFilterButton.getPreferredSize());
*/
    FullgenAtomsPanel.add(minAtomsLabel,
     new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0,
     GridBagConstraints.WEST, GridBagConstraints.BOTH,
     new Insets(0, 5, 5, 0), 0, 0));
    FullgenAtomsPanel.add(maxAtomsLabel,
     new GridBagConstraints(1, 0, 1, 1, 0.0, 0.0,
     GridBagConstraints.WEST, GridBagConstraints.BOTH,
     new Insets(0, 5, 5, 0), 0, 0));
    FullgenAtomsPanel.add(minAtomsSlider,
     new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 0, 0, 20), 0, 0));
    FullgenAtomsPanel.add(maxAtomsSlider,
     new GridBagConstraints(1, 1, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 0, 0, 20), 0, 0));
    FullgenAtomsPanel.add(minEqMax,
     new GridBagConstraints(2, 1, 1, 1, 0.001, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(0, 0, 0, 0), 0, 0));
    FullgenExtrasPanel.setLayout(new GridBagLayout());
    FullgenExtrasPanel.add(Box.createRigidArea(new Dimension(0, 0)),
     new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.BOTH,
     new Insets(0, 0, 0, 0), 0, 0));
    FullgenExtrasPanel.add(ipr,
     new GridBagConstraints(1, 0, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.BOTH,
     new Insets(0, 0, 0, 0), 0, 0));
    FullgenExtrasPanel.add(dual,
     new GridBagConstraints(1, 1, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.BOTH,
     new Insets(0, 0, 0, 0), 0, 0));
    FullgenExtrasPanel.add(spiralStats,
     new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.BOTH,
     new Insets(0, 0, 0, 0), 0, 0));
    FullgenExtrasPanel.add(symmStats,
     new GridBagConstraints(1, 3, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.BOTH,
     new Insets(0, 0, 0, 0), 0, 0));
    FullgenExtrasPanel.add(symmetryFilterButton,
     new GridBagConstraints(3, 0, 1, 2, 1.0, 1.0,
     GridBagConstraints.NORTHWEST, GridBagConstraints.NONE,
     new Insets(0, 0, 0, 0), 0, 0));
/* --- "select cases" disabled ---
    FullgenExtrasPanel.add(selectCasesButton,
     new GridBagConstraints(3, 2, 1, 2, 1.0, 1.0,
     GridBagConstraints.NORTHWEST, GridBagConstraints.NONE,
     new Insets(0, 0, 0, 0), 0, 0));
*/
    this.setLayout(new GridBagLayout());
    this.add(FullgenAtomsPanel,
     new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.BOTH,
     new Insets(0, 0, 0, 0), 0, 0));
    this.add(sep1,
     new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.BOTH,
     new Insets(25, 0, 25, 0), 0, 0));
    this.add(FullgenExtrasPanel,
     new GridBagConstraints(0, 2, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.BOTH,
     new Insets(0, 0, 0, 0), 0, 0));

    JPanel symmetriesContent = (JPanel) symmetriesDialog.getContentPane();
    symmetriesContent.setLayout(new BoxLayout(symmetriesContent, BoxLayout.Y_AXIS));
    JPanel symmetryButtonPanel = new JPanel();
    symmetryButtonPanel.setLayout(new GridLayout(symmRows, 0, 10, 10));
    symmetryButtonPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    int symmCols = (symmetries - 1) / symmRows + 1;
    for (int i = 0; i < symmetries; ++i)
    {
      int k = (i % symmCols) * symmRows + (i / symmCols);
      symmetryButton[k] = new JToggleButton(symmetry[k]);
      symmetryButton[k].setBorder(BorderFactory.createEmptyBorder(3, 7, 3, 7));
      new PushButtonDecoration(symmetryButton[k], true);
      symmetryButton[k].setSelected(true);
      selectedSymmetry[k] = true;
      symmetryButton[k].setActionCommand("s");
      symmetryButton[k].addActionListener(this);
      symmetryButtonPanel.add(symmetryButton[k]);
    }
    selectedSymmetries = symmetries;
    symmetriesContent.add(symmetryButtonPanel);
    JPanel symmetriesFinishPanel = new JPanel();
    symmetriesAllButton.setText("Set all");
    symmetriesAllButton.setMnemonic(KeyEvent.VK_S);
    symmetriesAllButton.setActionCommand("a+");
    symmetriesAllButton.addActionListener(this);
    symmetriesFinishPanel.add(symmetriesAllButton);
    JButton symmetriesNoneButton = new JButton("Clear all");
    symmetriesNoneButton.setMnemonic(KeyEvent.VK_C);
    symmetriesNoneButton.setActionCommand("a-");
    symmetriesNoneButton.addActionListener(this);
    symmetriesFinishPanel.add(Box.createHorizontalStrut(5));
    symmetriesFinishPanel.add(symmetriesNoneButton);
    symmetriesOkButton.setText("Ok");
    symmetriesFinishPanel.add(Box.createHorizontalStrut(5));
    symmetriesFinishPanel.add(symmetriesOkButton);
    JButton symmetriesCancelButton = new JButton("Cancel");
    symmetriesFinishPanel.add(Box.createHorizontalStrut(5));
    symmetriesFinishPanel.add(symmetriesCancelButton);
    symmetriesContent.add(Box.createVerticalStrut(10));
    symmetriesContent.add(symmetriesFinishPanel);
    symmetriesContent.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
    symmetriesDialog.setDefaultButton(symmetriesOkButton);
    symmetriesDialog.setCancelButton(symmetriesCancelButton);
    symmetriesDialog.pack();

/* --- "select cases" disabled ---
    JPanel casesContent = (JPanel) casesDialog.getContentPane();
    casesContent.setLayout(new BoxLayout(casesContent, BoxLayout.Y_AXIS));
    for (int i = 0; i <= 3; ++i)
    {
      caseButton[i] = new JRadioButton();
      caseButton[i].setActionCommand("d" + i);
      caseButton[i].addActionListener(this);
      casesButtonGroup.add(caseButton[i]);
      if (i == 0) {
        caseButton[i].setText("All cases");
	caseButton[i].setMnemonic(KeyEvent.VK_A);
      } else {
        caseButton[i].setText("case " + i);
	caseButton[i].setMnemonic(KeyEvent.VK_0 + i);
      }
      casesContent.add(caseButton[i]);
      casesContent.add(Box.createVerticalStrut(5));
    }
    JPanel casesFinishPanel = new JPanel();
    JButton casesOkButton = new JButton("Ok");
    casesFinishPanel.add(casesOkButton);
    JButton casesCancelButton = new JButton("Cancel");
    casesFinishPanel.add(casesCancelButton);
    casesContent.add(Box.createVerticalStrut(5));
    casesContent.add(casesFinishPanel);
    casesContent.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
    casesDialog.setDefaultButton(casesOkButton);
    casesDialog.setCancelButton(casesCancelButton);
    casesDialog.pack();
*/
  }

  public void actionPerformed(ActionEvent e)
  {
    String actionCommand = e.getActionCommand();
    switch (actionCommand.charAt(0))
    {
/* --- "select cases" disabled ---
      case 'c':
	selectCases();
        break;
      case 'd':
	selectCasesButton.setSelected(actionCommand.charAt(1) != '0');
        break;
*/
      case 'D':
        embedderIsConstant = false;
	break;
      case 'F':
	symmetryFilter();
        break;
      case 'a':
	boolean selected = actionCommand.charAt(1) == '+';
	for (int i = 0; i < symmetries; ++i)
	{
	  symmetryButton[i].setSelected(selected);
	}
	selectedSymmetries = selected ? symmetries : 0;
	symmetriesOkButton.setEnabled(selectedSymmetries > 0);
	symmetryFilterButton.setSelected(selectedSymmetries < symmetries);
        break;
      case 's':
	AbstractButton sb = (AbstractButton) e.getSource();
	selectedSymmetries += sb.isSelected() ? +1 : -1;
	symmetriesOkButton.setEnabled(selectedSymmetries > 0);
	symmetryFilterButton.setSelected(selectedSymmetries < symmetries);
        break;
    }
  }

/* --- "select cases" disabled ---
  public void selectCases()
  {
    selectCasesButton.setSelected(cases != 0);
    casesDialog.setSuccess(false);
    caseButton[cases].setSelected(true);
    caseButton[cases].requestFocus();
    casesDialog.show();
    if (casesDialog.getSuccess()) {
      cases = casesButtonGroup.getSelection().getActionCommand().charAt(1) - '0';
    }
    caseButton[cases].setSelected(true);
    selectCasesButton.setSelected(cases != 0);
  }
*/

  public void symmetryFilter()
  {
    symmetryFilterButton.setSelected(selectedSymmetries < symmetries);
    symmetriesDialog.setSuccess(false);
    symmetriesAllButton.requestFocus();
    symmetriesDialog.setVisible(true);
    if (symmetriesDialog.getSuccess()) {
      for (int i = 0; i < symmetries; ++i)
      {
        selectedSymmetry[i] = symmetryButton[i].isSelected();
      }
    } else {
      selectedSymmetries = 0;
      for (int i = 0; i < symmetries; ++i)
      {
	symmetryButton[i].setSelected(selectedSymmetry[i]);
	selectedSymmetries += selectedSymmetry[i] ? 1 : 0;
      }
    }
    symmetriesOkButton.setEnabled(selectedSymmetries > 0);
    // actually, selectedSymmetries is guaranteed to be positive
    symmetryFilterButton.setSelected(selectedSymmetries < symmetries);
  }

  public GeneratorInfo getGeneratorInfo()
  {
    String filename = "";
    Vector command = new Vector();

    int min = minAtomsSlider.getValue();
    int max = maxAtomsSlider.getValue();

    command.addElement("fullgen");
    command.addElement(Integer.toString(max));
    filename += "full_" + max;
    if (max != min) {
      command.addElement("start");
      command.addElement(Integer.toString(min));
      filename += "_start_" + min;
    }
    if (ipr.isSelected()) {
      command.addElement("ipr");
      filename += "_ipr";
    }
/* --- "select cases" disabled ---
    if (cases != 0) {
      command.addElement("case");
      command.addElement(Integer.toString(cases));
      filename += "_c" + cases;
    }
*/
    if (spiralStats.isSelected()) {
      command.addElement("spistat");
    }
    if (symmStats.isSelected()) {
      command.addElement("symstat");
    }
    if (selectedSymmetries < symmetries) {
      for (int k = 0; k < symmetries; ++k)
      {
        if (selectedSymmetry[k]) {
	  command.addElement("symm");
	  command.addElement(symmetry[k]);
	  filename += "_" + symmetry[k];
	}
      }
    }
    command.addElement("code");
    if (dual.isSelected()) {
      command.addElement("7");
    } else {
      command.addElement("1");
    }
    command.addElement("stdout");
    command.addElement("logerr");

    String[][] generator = new String[1][command.size()];
    command.copyInto(generator[0]);

    String[][] embed2D = { { "embed" } };
    String[][] embed3D;
    if (dual.isSelected()) {
      embed3D = new String[][] { { "embed", "-d3" } };
    } else {
      embed3D = new String[][] { { "embed", "-d3", "-it" } };
    }

    int maxFacesize = dual.isSelected() ? 3 : 6;

    ElementRule rule = new ValencyElementRule("3:C Si N S I");

    StaticGeneratorInfo generatorInfo = new StaticGeneratorInfo(
     generator,
     EmbedFactory.createEmbedder(embedderIsConstant, embed2D, embed3D),
     filename, maxFacesize, rule);
    embedderIsConstant = true;
    return generatorInfo;
  }

  public void showing()
  {
  }

  public static void main(String[] args)
  {
    final FullgenPanel p = new FullgenPanel();
    p.setBorder(BorderFactory.createTitledBorder(
     BorderFactory.createCompoundBorder(
      BorderFactory.createCompoundBorder(
       BorderFactory.createEmptyBorder(10, 10, 10, 10),
       BorderFactory.createEtchedBorder()),
      BorderFactory.createEmptyBorder(20, 20, 20, 20)),
     " Fullgen Options "));
    final JFrame f = new JFrame("Fullgen Dialog");
    f.addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        f.setVisible(false);
	GeneratorInfo info = p.getGeneratorInfo();
        JOptionPane.showInputDialog(null, "Command Line", "Fullgen results",
         JOptionPane.PLAIN_MESSAGE, null, null,
         "Command: " + Systoolbox.join(info.getGenerator()[0], " ") + "\nOutput: " + info.getFilename());
        System.exit(0);
      }
    });
    UItoolbox.addExitOnEscape(f);
    f.setContentPane(p);
    f.pack();
    f.setVisible(true);
  }
}

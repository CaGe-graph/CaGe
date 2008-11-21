

package cage.generator;

import cage.ElementRule;
import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;
import cage.ValencyElementRule;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
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
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JToggleButton;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.MinMaxEqListener;
import lisken.uitoolbox.UItoolbox;


public class TubetypePanel extends GeneratorPanel
{
  public static final int MAX_TUBELENGTH = 30;
  public static final int MAX_OFFSET = 30;

/*
  JRadioButton tubeButton = new JRadioButton();
  JRadioButton fullButton = new JRadioButton();
  ButtonGroup capsGroup = new ButtonGroup();
*/
  EnhancedSlider tubelengthSlider = new EnhancedSlider();
/*
  SpinButton perimeterControl = new SpinButton(10, 4, 2 * MAX_OFFSET + 1);
  SpinButton shiftControl = new SpinButton(0, 0, MAX_OFFSET);
  SpinButton offset1Control = new SpinButton(5, 2, MAX_OFFSET);
  SpinButton offset2Control = new SpinButton(0, 0, MAX_OFFSET);
*/
  EnhancedSlider offset1Control = new EnhancedSlider();
  EnhancedSlider offset2Control = new EnhancedSlider();
  JCheckBox ipr = new JCheckBox();
  AbstractButton defaultTubelengthButton = new JToggleButton();

  boolean adjusting = false;

  public TubetypePanel()
  {
    setLayout(new GridBagLayout());
    tubelengthSlider.setMinimum(0);
    tubelengthSlider.setMaximum(MAX_TUBELENGTH);
    tubelengthSlider.setMinorTickSpacing(1);
    tubelengthSlider.setMajorTickSpacing(tubelengthSlider.getMaximum() - tubelengthSlider.getMinimum());
    tubelengthSlider.setPaintTicks(true);
    tubelengthSlider.setPaintLabels(true);
    tubelengthSlider.setSnapWhileDragging(1);
    tubelengthSlider.setClickScrollByBlock(false);
    tubelengthSlider.setSizeFactor(10);
    tubelengthSlider.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent e)
      {
        if (! adjusting) defaultTubelengthButton.setSelected(false);
      }
    });
    defaultTubelengthButton.setText("default");
    defaultTubelengthButton.setSelected(false);
    defaultTubelengthButton.setMnemonic(KeyEvent.VK_D);
    defaultTubelengthButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e)
      {
	if (defaultTubelengthButton.isSelected()) adjustTubelength();
      }
    });
/*
    perimeterControl.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent e)
      {
        adjustShift();
	adjustOffsets();
	if (defaultTubelengthButton.isSelected()) adjustTubelength();
      }
    });
    perimeterControl.setNextFocusableComponent(shiftControl);
    shiftControl.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent e)
      {
        adjustPerimeter();
	adjustOffsets();
	if (defaultTubelengthButton.isSelected()) adjustTubelength();
      }
    });
    shiftControl.setNextFocusableComponent(offset1Control);
*/
    offset1Control.setMinimum(2);
    offset1Control.setMaximum(MAX_OFFSET);
    offset1Control.setValue(5);
    offset1Control.setMinorTickSpacing(1);
    offset1Control.setMajorTickSpacing(offset1Control.getMaximum() - offset1Control.getMinimum());
    offset1Control.setPaintTicks(true);
    offset1Control.setPaintLabels(true);
    offset1Control.setSnapWhileDragging(1);
    offset1Control.setClickScrollByBlock(false);
    offset1Control.setSizeFactor(4);
    offset2Control.setMinimum(0);
    offset2Control.setMaximum(MAX_OFFSET);
    offset2Control.setValue(0);
    offset2Control.setMinorTickSpacing(1);
    offset2Control.setMajorTickSpacing(offset2Control.getMaximum() - offset2Control.getMinimum());
    offset2Control.setPaintTicks(true);
    offset2Control.setPaintLabels(true);
    offset2Control.setSnapWhileDragging(1);
    offset2Control.setClickScrollByBlock(false);
    offset2Control.setSizeFactor(4);
    adjustTubelength();
    ChangeListener offsetListener = new ChangeListener() {
      public void stateChanged(ChangeEvent e)
      {
//	adjustPerimeterAndShift();
	if (defaultTubelengthButton.isSelected()) adjustTubelength();
      }
    };
    offset1Control.addChangeListener(offsetListener);
    offset1Control.setNextFocusableComponent(offset2Control);
    offset2Control.addChangeListener(offsetListener);
    new MinMaxEqListener(offset2Control.getModel(), offset1Control.getModel(), false);
    JLabel tubelengthLabel = new JLabel("Tube length:");
    tubelengthLabel.setDisplayedMnemonic(KeyEvent.VK_T);
    tubelengthLabel.setLabelFor(tubelengthSlider.slider());
/*
    tubeButton.setText("one side (half-open tubes)");
    tubeButton.setMnemonic(KeyEvent.VK_O);
    tubeButton.setActionCommand("1");
    capsGroup.add(tubeButton);
    fullButton.setText("both sides (spherical fullerenes)");
    fullButton.setMnemonic(KeyEvent.VK_B);
    fullButton.setActionCommand("2");
    capsGroup.add(fullButton);
    tubeButton.setSelected(true);
*/
    ipr.setText("isolated pentagons");
    ipr.setMnemonic(KeyEvent.VK_I);
    Font font = ipr.getFont();
    font = new Font(
     font.getName(),
     font.getStyle() & ~ Font.BOLD,
     font.getSize());
    defaultTubelengthButton.setFont(font);
    ipr.setFont(font);
/*
    tubeButton.setFont(font);
    fullButton.setFont(font);
*/
    add(tubelengthLabel,
     new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
     GridBagConstraints.EAST, GridBagConstraints.NONE,
     new Insets(0, 0, 20, 10), 0, 0));
    add(tubelengthSlider,
     new GridBagConstraints(1, 0, 3, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 0, 20, 5), 0, 0));
    add(defaultTubelengthButton,
     new GridBagConstraints(4, 0, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(0, 10, 20, 0), 0, 0));
/*
    JLabel perimeterLabel = new JLabel("Perimeter");
    perimeterLabel.setDisplayedMnemonic(KeyEvent.VK_P);
    perimeterLabel.setLabelFor(perimeterControl);
    add(perimeterLabel,
     new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(5, 5, 5, 0), 0, 0));
    add(perimeterControl,
     new GridBagConstraints(1, 3, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(0, 5, 0, 0), 0, 0));
    JLabel shiftLabel = new JLabel("Shift");
    shiftLabel.setDisplayedMnemonic(KeyEvent.VK_S);
    shiftLabel.setLabelFor(shiftControl);
    add(shiftLabel,
     new GridBagConstraints(1, 4, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(5, 5, 5, 0), 0, 0));
    add(shiftControl,
     new GridBagConstraints(1, 5, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(0, 5, 0, 0), 0, 0));
    add(new JLabel(""),
     new GridBagConstraints(2, 3, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.NONE,
     new Insets(0, 0, 0, 0), 0, 0));
    JLabel offset1Label = new JLabel("Offset 1");
    offset1Label.setDisplayedMnemonic(KeyEvent.VK_1);
    offset1Label.setLabelFor(offset1Control);
    add(offset1Label,
     new GridBagConstraints(3, 2, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(5, 5, 5, 0), 0, 0));
    add(offset1Control,
     new GridBagConstraints(3, 3, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(0, 5, 0, 0), 0, 0));
    JLabel offset2Label = new JLabel("Offset 2");
    offset2Label.setDisplayedMnemonic(KeyEvent.VK_2);
    offset2Label.setLabelFor(offset2Control);
    add(offset2Label,
     new GridBagConstraints(3, 4, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(5, 5, 5, 0), 0, 0));
    add(offset2Control,
     new GridBagConstraints(3, 5, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(0, 5, 0, 0), 0, 0));
*/
    JLabel offsetsLabel = new JLabel("Boundary parameters:");
    add(offsetsLabel,
     new GridBagConstraints(0, 2, 1, 1, 1.0, 1.0,
     GridBagConstraints.EAST, GridBagConstraints.NONE,
     new Insets(5, 0, 5, 10), 0, 0));
    JLabel offset1Label = new JLabel("l");
    offset1Label.setDisplayedMnemonic(KeyEvent.VK_L);
    offset1Label.setLabelFor(offset1Control.slider());
    add(offset1Label,
     new GridBagConstraints(1, 2, 1, 1, 0.001, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.NONE,
     new Insets(5, 5, 5, 0), 0, 0));
    add(offset1Control,
     new GridBagConstraints(2, 2, 1, 1, 0.1, 1.0,
     GridBagConstraints.EAST, GridBagConstraints.NONE,
     new Insets(5, 5, 5, 0), 0, 0));
    JLabel offset2Label = new JLabel("m");
    offset2Label.setDisplayedMnemonic(KeyEvent.VK_M);
    offset2Label.setLabelFor(offset2Control.slider());
    add(offset2Label,
     new GridBagConstraints(1, 3, 1, 1, 0.001, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.NONE,
     new Insets(5, 5, 5, 0), 0, 0));
    add(offset2Control,
     new GridBagConstraints(2, 3, 1, 1, 0.1, 1.0,
     GridBagConstraints.EAST, GridBagConstraints.NONE,
     new Insets(5, 5, 5, 0), 0, 0));
    add(Box.createHorizontalGlue(),
     new GridBagConstraints(3, 2, 1, 1, 1.0, 1.0,
     GridBagConstraints.EAST, GridBagConstraints.NONE,
     new Insets(0, 0, 0, 0), 0, 0));
/*
    add(new JLabel("Caps on:"),
     new GridBagConstraints(0, 6, 1, 1, 1.0, 1.0,
     GridBagConstraints.EAST, GridBagConstraints.NONE,
     new Insets(20, 0, 0, 10), 0, 0));
    JPanel capsPanel = new JPanel();
    capsPanel.add(tubeButton);
    capsPanel.add(Box.createHorizontalStrut(5));
    capsPanel.add(fullButton);
    add(capsPanel,
     new GridBagConstraints(1, 6, 4, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(20, 0, 0, 0), 0, 0));
*/
    add(ipr,
     new GridBagConstraints(1, 7, 3, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(30, 5, 0, 0), 0, 0));
  }

  void adjustTubelength()
  {
    if (adjusting) return;
    adjusting = true;
    int l = Math.max(offset1Control.getValue(), offset2Control.getValue()) - 1;
    tubelengthSlider.setValue(l);
    adjusting = false;
  }

/*
  void adjustOffsets()
  {
    if (adjusting) return;
    adjusting = true;
    int perimeter = perimeterControl.getValue();
    int shift = shiftControl.getValue();
    int o_n = perimeter / 2 - shift, o_m = shift;
    boolean n_is_1 = shift >= 0;
    if (n_is_1) {
      offset1Control.setValue(o_n);
      offset2Control.setValue(o_m);
    } else {
      offset1Control.setValue(o_m);
      offset2Control.setValue(o_n);
    }
    adjusting = false;
  }

  void adjustPerimeterAndShift()
  {
    if (adjusting) return;
    adjusting = true;
    int o1 = offset1Control.getValue();
    int o2 = offset2Control.getValue();
    int shift = Math.min(o1, o2);
    perimeterControl.setValue(2 * (o1 + o2) + shift % 2);
    shiftControl.setValue(shift);
    adjusting = false;
  }

  void adjustShift()
  {
    if (adjusting) return;
    adjusting = true;
    int perimeter = perimeterControl.getValue();
    int shift = shiftControl.getValue();
    int d = perimeter % 2 - shift % 2;
    if (d != 0) shiftControl.setValue(shift + d);
    adjusting = false;
  }

  void adjustPerimeter()
  {
    if (adjusting) return;
    adjusting = true;
    int perimeter = perimeterControl.getValue();
    int shift = shiftControl.getValue();
    int d = perimeter % 2 - shift % 2;
    if (d != 0) perimeterControl.setValue(perimeter - d);
    adjusting = false;
  }

  public String divideBy2(int value)
  {
    return (value / 2) + "." + (value % 2 == 1 ? "5" : "0");
  }
*/

  public GeneratorInfo getGeneratorInfo()
  {
    Vector command = new Vector();
    String filename;

    command.addElement("tubetype");
/*
    if (tubeButton.isSelected()) {
*/
      filename = "tubetypes";
/*
    } else {
      filename = "ttfullerenes";
    }
*/
    command.addElement(Integer.toString(offset1Control.getValue()));
    command.addElement(Integer.toString(offset2Control.getValue()));
    filename += "_l" + offset1Control.getValue();
    filename += "_m" + offset2Control.getValue();
    command.addElement("tube");
    command.addElement(Integer.toString(tubelengthSlider.getValue()));
    filename += "_t" + tubelengthSlider.getValue();
/*
    if (fullButton.isSelected()) {
      command.addElement("fullerenes");
    }
*/
    if (ipr.isSelected()) {
      command.addElement("ipr");
      filename += "_ip";
    }

    String[][] generator = new String[1][command.size()];
    command.copyInto(generator[0]);

    String[][] embed2D = { { "embed" } };
    String[][] embed3D = { { "embed", "-d3", "-it" } };

    ElementRule rule = new ValencyElementRule("1:H 3:C");

    return new StaticGeneratorInfo(
     generator,
     EmbedFactory.createEmbedder(true, embed2D, embed3D),
     filename, 6, rule);
  }

  public void showing()
  {
  }

  public static void main(String[] args)
  {
    final TubetypePanel p = new TubetypePanel();
    p.setBorder(BorderFactory.createTitledBorder(
     BorderFactory.createCompoundBorder(
      BorderFactory.createCompoundBorder(
       BorderFactory.createEmptyBorder(10, 10, 10, 10),
       BorderFactory.createEtchedBorder()),
      BorderFactory.createEmptyBorder(20, 20, 20, 20)),
     " Tubetype Options "));
    final JFrame f = new JFrame("Tubetype Dialog");
    f.addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        f.setVisible(false);
	GeneratorInfo info = p.getGeneratorInfo();
        JOptionPane.showInputDialog(null, "Command Line", "Tubetype results",
         JOptionPane.PLAIN_MESSAGE, null, null,
         "Command: " + Systoolbox.join(info.getGenerator()[0], " ") + "\nOutput: " + info.getFilename());
        System.exit(0);
      }
    });
    UItoolbox.addExitOnEscape(f);
    f.setContentPane(p);
    f.pack();
    Dimension d = f.getSize();
    d.height += 2;
    f.setSize(d);
    f.setVisible(true);
  }
}

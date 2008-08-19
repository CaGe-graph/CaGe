

package cage.generator;


import cage.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import lisken.systoolbox.*;
import lisken.uitoolbox.*;


public class FormulaHCgenPanel extends GeneratorPanel
 implements ActionListener
{
  static final int MAX_ATOMS = 450; /* keep this even */

  static final int MIN_C = 5;
  static final int MAX_C = MAX_ATOMS;
  static final int MIN_PENT = 0;
  static final int MAX_PENT = 5;
  static final int MIN_H = 5;
  static final int MAX_H = maxH(MAX_C, MIN_PENT);
  static final int MAX_HGAP = MAX_C;
  static final int DEFAULT_HGAP = 4;

  static final Color limitOkColor = Color.black;
  static final Color limitErrorColor = new Color(0.75f, 0.0f, 0.0f);

  static final boolean enableReembed2D =
   CaGe.getCaGePropertyAsBoolean("HCgen.EnableReembed2D", false);

  int C = 6, H = 6, pentagons = 0;
  int minC, maxC, minH, maxH, minPent, maxPent;
  boolean COk, HOk, pentagonsOk, parityOk;

  EnhancedSlider CSlider = new EnhancedSlider();
  JButton minCButton = new JButton();
  JButton maxCButton = new JButton();
  JPanel rangeCPanel = new JPanel();
  EnhancedSlider HSlider = new EnhancedSlider();
  JButton minHButton = new JButton();
  JButton maxHButton = new JButton();
  JPanel rangeHPanel = new JPanel();
  EnhancedSlider pentSlider = new EnhancedSlider();
  JButton minPentButton = new JButton();
  JButton maxPentButton = new JButton();
  JPanel rangePentPanel = new JPanel();
  EnhancedSlider HGapSlider = new EnhancedSlider();
  JCheckBox ipr = new JCheckBox();
  JCheckBox includeH = new JCheckBox();
  JCheckBox peri = new JCheckBox();

  JButton defaultButton;

  public FormulaHCgenPanel()
  {
    setLayout(new GridBagLayout());
    CSlider.setMinimum(MIN_C);
    CSlider.setMaximum(MAX_C);
    CSlider.setValue(C);
    CSlider.setMinorTickSpacing(10);
    CSlider.setMajorTickSpacing(CSlider.getMaximum() - CSlider.getMinimum());
    CSlider.setPaintTicks(true);
    CSlider.setPaintLabels(true);
    CSlider.setSnapWhileDragging(1);
    CSlider.setSizeFactor(0.5);
    CSlider.setClickScrollByBlock(false);
    CSlider.addChangeListener(new ChangeAdapter() {
      public void stateChanged(ChangeEvent e)
      {
        C = CSlider.getValue();
	checkCLimits();
	getParity();
	getHLimits();
	getPentagonLimits();
      }
    });
    JLabel CLabel = new JLabel("number of C atoms");
    CLabel.setLabelFor(CSlider.slider());
    CLabel.setDisplayedMnemonic(KeyEvent.VK_C);
    HSlider.setMinimum(MIN_H);
    HSlider.setMaximum(MAX_H);
    HSlider.setValue(H);
    HSlider.setMinorTickSpacing(10);
    HSlider.setMajorTickSpacing(HSlider.getMaximum() - HSlider.getMinimum());
    HSlider.setPaintTicks(true);
    HSlider.setPaintLabels(true);
    HSlider.setSnapWhileDragging(1);
    HSlider.setClickScrollByBlock(false);
    HSlider.addChangeListener(new ChangeAdapter() {
      public void stateChanged(ChangeEvent e)
      {
        H = HSlider.getValue();
	checkHLimits();
	getParity();
	getCLimits();
	getPentagonLimits();
      }
    });
    JLabel HLabel = new JLabel("number of H atoms");
    HLabel.setLabelFor(HSlider.slider());
    HLabel.setDisplayedMnemonic(KeyEvent.VK_H);
    pentSlider.setMinimum(MIN_PENT);
    pentSlider.setMaximum(MAX_PENT);
    pentSlider.setValue(pentagons);
    pentSlider.setMinorTickSpacing(1);
    pentSlider.setMajorTickSpacing(pentSlider.getMaximum() - pentSlider.getMinimum());
    pentSlider.setPaintTicks(true);
    pentSlider.setPaintLabels(true);
    pentSlider.setSnapWhileDragging(1);
//    pentSlider.setSizeFactor(35);
    pentSlider.setClickScrollByBlock(false);
    pentSlider.addChangeListener(new ChangeAdapter() {
      public void stateChanged(ChangeEvent e)
      {
        pentagons = pentSlider.getValue();
	checkPentagonLimits();
	getHLimits();
	getCLimits();
      }
    });
    JLabel pentLabel = new JLabel("number of Pentagons");
    pentLabel.setLabelFor(pentSlider.slider());
    pentLabel.setDisplayedMnemonic(KeyEvent.VK_P);
    HGapSlider.setMinimum(1);
    HGapSlider.setMaximum(MAX_HGAP);
    HGapSlider.setValue(DEFAULT_HGAP);
    HGapSlider.setMinorTickSpacing(10);
    HGapSlider.setMajorTickSpacing(HGapSlider.getMaximum() - HGapSlider.getMinimum());
    HGapSlider.setPaintTicks(true);
    HGapSlider.setPaintLabels(true);
    HGapSlider.setSnapWhileDragging(1);
    HGapSlider.setSizeFactor(0.5);
    HGapSlider.setClickScrollByBlock(false);
    JLabel HGapLabel = new JLabel("maximum Gap between H atoms");
    HGapLabel.setLabelFor(HGapSlider.slider());
    HGapLabel.setDisplayedMnemonic(KeyEvent.VK_G);
    ipr.setText("isolated pentagons (ipr)");
    ipr.setMnemonic(KeyEvent.VK_I);
    ipr.setSelected(false);
    includeH.setText("include H atoms");
    includeH.setMnemonic(KeyEvent.VK_A);
    includeH.setSelected(true);
    peri.setText("strictly peri-condensed");
    peri.setMnemonic(KeyEvent.VK_E);
    peri.setSelected(false);
    Font font = ipr.getFont();
    font = new Font(
     font.getName(),
     font.getStyle() & ~ Font.BOLD,
     font.getSize() - 2);
/*
    FontMetrics metrics = Toolkit.getDefaultToolkit().getFontMetrics(font);
    int extra1 = metrics.charWidth('0');
    int extra2 = metrics.charWidth('-');
*/
    minCButton.setText("min. 000");
    minCButton.setFont(font);
    minCButton.setActionCommand("c-");
    minCButton.addActionListener(this);
    maxCButton.setText("min. 00000");
    maxCButton.setFont(font);
    maxCButton.setActionCommand("c+");
    maxCButton.addActionListener(this);
    minHButton.setText("min. 000");
    minHButton.setFont(font);
    minHButton.setActionCommand("h-");
    minHButton.addActionListener(this);
    maxHButton.setText("min. 000");
    maxHButton.setFont(font);
    maxHButton.setActionCommand("h+");
    maxHButton.addActionListener(this);
    minPentButton.setText("min. -0");
    minPentButton.setFont(font);
    minPentButton.setActionCommand("p-");
    minPentButton.addActionListener(this);
    maxPentButton.setText("min. -0");
    maxPentButton.setFont(font);
    maxPentButton.setActionCommand("p+");
    maxPentButton.addActionListener(this);
    rangeCPanel.setLayout(new FlowLayout(FlowLayout.CENTER, 4, 0));
    rangeCPanel.add(minCButton);
    rangeCPanel.add(maxCButton);
    rangeHPanel.setLayout(new FlowLayout(FlowLayout.CENTER, 4, 0));
    rangeHPanel.add(minHButton);
    rangeHPanel.add(maxHButton);
    rangePentPanel.setLayout(new FlowLayout(FlowLayout.CENTER, 4, 0));
    rangePentPanel.add(minPentButton);
    rangePentPanel.add(maxPentButton);
    add(CLabel,
     new GridBagConstraints2(0, 0, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(0, 10, 5, 10), 0, 0));
    add(CSlider,
     new GridBagConstraints2(0, 1, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(Box.createHorizontalStrut(rangeCPanel.getPreferredSize().width),
     new GridBagConstraints2(0, 2, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.NONE,
     new Insets(0, 30, 0, 30), 0, 0));
    add(rangeCPanel,
     new GridBagConstraints2(0, 3, 1, 1, 0.001, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.NONE,
     new Insets(0, 0, 25, 0), 0, 0));
    add(HLabel,
     new GridBagConstraints2(1, 0, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(0, 10, 5, 10), 0, 0));
    add(HSlider,
     new GridBagConstraints2(1, 1, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(Box.createHorizontalStrut(rangeHPanel.getPreferredSize().width),
     new GridBagConstraints2(1, 2, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.NONE,
     new Insets(0, 30, 0, 30), 0, 0));
    add(rangeHPanel,
     new GridBagConstraints2(1, 3, 1, 1, 0.001, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.NONE,
     new Insets(0, 0, 25, 0), 0, 0));
    add(pentLabel,
     new GridBagConstraints2(2, 0, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(0, 5, 5, 10), 0, 0));
    add(pentSlider,
     new GridBagConstraints2(2, 1, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL,
     new Insets(0, 5, 0, 10), 0, 0));
    add(Box.createHorizontalStrut(rangePentPanel.getPreferredSize().width),
     new GridBagConstraints2(2, 2, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.NONE,
     new Insets(0, 30, 0, 30), 0, 0));
    add(rangePentPanel,
     new GridBagConstraints2(2, 3, 1, 1, 0.001, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.NONE,
     new Insets(0, 0, 25, 0), 0, 0));
    add(ipr,
     new GridBagConstraints2(0, 4, 3, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(5, 10, 5, 0), 0, 0));
    add(includeH,
     new GridBagConstraints2(0, 5, 3, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(5, 10, 5, 0), 0, 0));
    add(peri,
     new GridBagConstraints2(0, 6, 3, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(5, 10, 5, 0), 0, 0));
    add(HGapLabel,
     new GridBagConstraints2(0, 7, 3, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(30, 10, 5, 0), 0, 0));
    add(HGapSlider,
     new GridBagConstraints2(0, 8, 2, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL,
     new Insets(0, 0, 0, 0), 0, 0));
  }

  void getCLimits()
  {
    maxC = maxC(H, pentagons);
    minC = minC(H, pentagons, maxC);
    minCButton.setText("min. " + minC);
    maxCButton.setText("max. " + maxC);
    checkCLimits();
  }

  void checkCLimits()
  {
    COk = minC <= C && C <= maxC;
    minCButton.setForeground(minC <= C ? limitOkColor : limitErrorColor);
    maxCButton.setForeground(C <= maxC ? limitOkColor : limitErrorColor);
    boolean limitOk = minC <= maxC && minC <= MAX_C && maxC >= MIN_C;
    minCButton.setEnabled(limitOk);
    maxCButton.setEnabled(limitOk);
    defaultButton.setEnabled(COk && HOk && pentagonsOk && parityOk);
  }

  void getHLimits()
  {
    minH = minH(C, pentagons);
    maxH = maxH(C, pentagons);
    minHButton.setText("min. " + minH);
    maxHButton.setText("max. " + maxH);
    checkHLimits();
  }

  void checkHLimits()
  {
    HOk = minH <= H && H <= maxH;
    minHButton.setForeground(minH <= H ? limitOkColor : limitErrorColor);
    maxHButton.setForeground(H <= maxH ? limitOkColor : limitErrorColor);
    boolean limitOk = minH <= maxH && minH <= MAX_H && maxH >= MIN_H;
    minHButton.setEnabled(limitOk);
    maxHButton.setEnabled(limitOk);
    defaultButton.setEnabled(COk && HOk && pentagonsOk && parityOk);
  }

  void getPentagonLimits()
  {
    calculatePentagonLimits();
    minPentButton.setText("min. " + minPent);
    maxPentButton.setText("max. " + maxPent);
    checkPentagonLimits();
  }

  void checkPentagonLimits()
  {
    pentagonsOk = minPent <= pentagons && pentagons <= maxPent;
    minPentButton.setForeground(minPent <= pentagons ? limitOkColor : limitErrorColor);
    maxPentButton.setForeground(pentagons <= maxPent ? limitOkColor : limitErrorColor);
    boolean limitOk = minPent <= maxPent && minPent <= MAX_PENT && maxPent >= MIN_PENT;
    minPentButton.setEnabled(limitOk);
    maxPentButton.setEnabled(limitOk);
    defaultButton.setEnabled(COk && HOk && pentagonsOk && parityOk);
  }

  void getParity()
  {
    parityOk = C % 2 == H % 2;
    checkParity();
  }

  void checkParity()
  {
    defaultButton.setText(parityOk ? "Next" : "C-H Parity Error");
    defaultButton.setEnabled(COk && HOk && pentagonsOk && parityOk);
  }

  public void actionPerformed(ActionEvent e)
  {
    String command = e.getActionCommand();
    boolean isMin = command.charAt(1) == '-';
    switch (command.charAt(0))
    {
      case 'c':
	CSlider.setValue(isMin ? minC : maxC);
        break;
      case 'h':
	HSlider.setValue(isMin ? minH : maxH);
        break;
      case 'p':
	pentSlider.setValue(isMin ? minPent : maxPent);
        break;
    }
  }

  public GeneratorInfo getGeneratorInfo()
  {
    Vector command = new Vector();
    String filename = "";
    command.addElement("hcgen");
    filename += "hc";
    command.addElement(Integer.toString(CSlider.getValue()));
    filename += "_c" + CSlider.getValue();
    command.addElement(Integer.toString(HSlider.getValue()));
    filename += "h" + HSlider.getValue();
    command.addElement(Integer.toString(pentSlider.getValue()));
    filename += "_" + pentSlider.getValue() + "pent";
    command.addElement("gap");
    command.addElement(Integer.toString(HGapSlider.getValue()));
    if (ipr.isSelected()) {
      command.addElement("ipr");
      filename += "_ipr";
    }
    if (peri.isSelected()) {
      command.addElement("peri_condensed");
      filename += "_pc";
    }
    if (! includeH.isSelected()) {
      command.addElement("without_H");
      filename += "_noH";
    }
    command.addElement("stdout");
    command.addElement("logerr");
    String[][] generator = new String[1][command.size()];
    command.copyInto(generator[0]);
    String[][] embed2D = { { "embed" } };
    String[][] embed3D = { { "embed", "-d3", "-f1,1,4" } };
    ElementRule rule = new ValencyElementRule("H C C");
    return new StaticGeneratorInfo(
     generator,
     EmbedFactory.createEmbedder(true, embed2D, embed3D),
     filename, 6, enableReembed2D, rule);
  }

  public void showing()
  {
    defaultButton = SwingUtilities.getRootPane(this).getDefaultButton();
    getCLimits();
    getHLimits();
    getPentagonLimits();
    getParity();
  }

  public static void main(String[] args)
  {
    final FormulaHCgenPanel p = new FormulaHCgenPanel();
    p.setBorder(BorderFactory.createTitledBorder(
     BorderFactory.createCompoundBorder(
      BorderFactory.createCompoundBorder(
       BorderFactory.createEmptyBorder(10, 10, 10, 10),
       BorderFactory.createEtchedBorder()),
      BorderFactory.createEmptyBorder(20, 20, 20, 20)),
     " HCgen Options "));
    final JFrame f = new JFrame("HCgen Dialog");
    final WindowAdapter closeListener = new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        f.setVisible(false);
	GeneratorInfo info = p.getGeneratorInfo();
        JOptionPane.showInputDialog(null, "Command Line", "HCgen results",
         JOptionPane.PLAIN_MESSAGE, null, null,
         "Command: " + Systoolbox.join(info.getGenerator()[0], " ") + "\nOutput: " + info.getFilename());
        System.exit(0);
      }
    };
    f.addWindowListener(closeListener);
    UItoolbox.addExitOnEscape(f);
    JButton okButton = new JButton("Ok");
    okButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e)
      {
        closeListener.windowClosing(new WindowEvent(f, WindowEvent.WINDOW_CLOSING));
      }
    });
    JPanel okPanel = new JPanel();
    okPanel.add(okButton);
    f.getContentPane().add(p, BorderLayout.CENTER);
    f.getContentPane().add(okPanel, BorderLayout.SOUTH);
    f.getRootPane().setDefaultButton(okButton);
    f.pack();
    Dimension d = f.getSize();
    d.height += 2;
    f.setSize(d);
    f.show();
    p.showing();
  }

/*
   The following methods are copied from the C&Tcl/Tk version of CaGe
   and are variations of the "min_rand" function from HCgen.c. In this
   version, comments have been stripped and variable names anglicised.
*/
  static int minH(int c, int pentagons)
  {
    int level, faces, cur_c, bndry_level, h, inner_faces, spiral_bndry;
    switch (pentagons)
    {
      case 0:
	cur_c = 6;
	faces = 1;
	for (level = 1; cur_c + 6 + 12 * level <= c; level++)
	{
	  faces += (6 * level);
	  cur_c += 6 + 12 * level;
	}
	if (cur_c < c) {
	  bndry_level = 6 + 12 * (level - 1);
	  inner_faces = faces;
	  while (bndry_level + 2 * ((faces - inner_faces) / level) + 2 <
	   (2 * (c - 2 * faces + 2) - 6))
	  {
	    faces++;
	  }
	  if (bndry_level + 2 * ((faces - inner_faces) / level) + 2 >
	      (2 * (c - 2 * faces + 2) - 6)) {
	    faces--;
	  }
	}
	h = c - 2 * faces + 2;
	if (! (c >= 6 && h >= 6)) {
	  h = 0;
	}
	break;
      case 1:
	cur_c = 5;
	faces = 1;
	for (level = 1; cur_c + 5 + 10 * level <= c; level++)
	{
	  faces += (5 * level);
	  cur_c += 5 + 10 * level;
	}
	if (cur_c < c) {
	  bndry_level = 5 + 10 * (level - 1);
	  inner_faces = faces;
	  while (bndry_level + 2 * ((faces - inner_faces) / level) + 2 <
	   (2 * (c - 2 * faces + 2) - 6 + 1))
	  {
	    faces++;
	  }
	  if (bndry_level + 2 * ((faces - inner_faces) / level) + 2 >
	      (2 * (c - 2 * faces + 2) - 6 + 1)) {
	    faces--;
          }
        }
        h = c - 2 * faces + 2;
	if (! (c >= 5 && h >= 5)) {
          h = 0;
        }
	break;
      case 2:
        cur_c = 8;
	faces = 2;
        for (level = 1; cur_c + 8 + 8 * level <= c; level++)
	{
          faces += (4 * level + 2);                                                       cur_c += 8 + 8 * level;
        }
        if (cur_c < c) {
          spiral_bndry = 8 + 8 * (level - 1);
          if (spiral_bndry + 2 <= 2 * (c - 2 * (faces + level) + 2) - 6 + 3) {
            spiral_bndry += 2;
            faces += level;
            if (spiral_bndry + 2 <= 2 * (c - 2 * (faces + level) + 2) - 6 + 3) {
              spiral_bndry += 2;
              faces += level;
              if (spiral_bndry + 2 <= 2 * (c - 2 * (faces + level + 1) + 2) - 6 + 3) {
                spiral_bndry += 2;
                faces += level + 1;
                if (spiral_bndry + 2 <= 2 * (c - 2 * (faces + level) + 2) - 6 + 3) {
                  spiral_bndry += 2;
                  faces += level;
                }
              }
            }
          }
          while (spiral_bndry + 2 <= 2 * (c - 2 * (faces + 1) + 2) - 6 + 3)
	  {
	    faces++;
          }
        }
        h = c - 2 * faces + 2;
        if (! (c >= 8 && h >= 6)) {
          h = 0;
        }
	break;
      case 3:
        cur_c = 10;
	faces = 3;
        for (level = 1; cur_c + 9 + 6 * level <= c; level++)
	{
          faces += (3 * level + 3);
          cur_c += 9 + 6 * level;
        }
        if (cur_c < c) {
          spiral_bndry = 9 + 6 * (level - 1);
          if (spiral_bndry + 2 <= 2 * (c - 2 * (faces + level) + 2) - 6 + 3) {
            spiral_bndry += 2;
            faces += level;
	    if (spiral_bndry + 2 <= 2 * (c - 2 * (faces + level + 1) + 2) - 6 + 3) {
              spiral_bndry += 2;
              faces += level + 1;
	      if (spiral_bndry + 2 <= 2 * (c - 2 * (faces + level + 1) + 2) - 6 + 3) {
                spiral_bndry += 2;
                faces += level + 1;
              }
            }
          }
	  while (spiral_bndry + 2 <= 2 * (c - 2 * (faces + 1) + 2) - 6 + 3)
	  {
	    faces++;
          }
        }
        h = c - 2 * faces + 2;
        if (! (c >= 10 && h >= 6)) {
          h = 0;
        }
	break;
      case 4:
        cur_c = 12;
	faces = 4;
        for (level = 1; cur_c + 10 + 4 * level <= c; level++)
	{
          faces += (2 * level + 4);
          cur_c += 10 + 4 * level;
        }
        if (cur_c < c) {
          spiral_bndry = 10 + 4 * (level - 1);
          if (spiral_bndry + 2 <= 2 * (c - 2 * (faces + level + 1) + 2) - 6 + 4) {
            spiral_bndry += 2;
            faces += level + 1;
            if (spiral_bndry + 2 <= 2 * (c - 2 * (faces + level + 2) + 2) - 6 + 4) {
              spiral_bndry += 2;
              faces += level + 2;
            }
          }
          while (spiral_bndry + 2 <= 2 * (c - 2 * (faces + 1) + 2) - 6 + 4)
	  {
	    faces++;
          }
        }
        h = c - 2 * faces + 2;
        if (! (c >= 12 && h >= 6)) {
          h = 0;
        }
	break;
      case 5:
        if (c == 14) {
          h = 6;
          break;
        }
        if (c == 15) {
          h = 7;
          break;
        }
        cur_c = 16;
	faces = 6;
        for (level = 1; cur_c + 11 + 2 * level <= c; level++)
	{
          faces += (level + 5);
          cur_c += 11 + 2 * level;
        }
        if (cur_c < c) {
          spiral_bndry = 11 + 2 * (level - 1);
          while (spiral_bndry + 2 <= 2 * (c - 2 * (faces + 1) + 2) - 6 + 6)
	  {
	    faces++;
          }
        }
        h = c - 2 * faces + 2;
        if (! (c >= 14 && h >= 6)) {
          h = 0;
        }
	break;
      default:
	throw new RuntimeException("minH called with invalid pentagon number");
    }
    return h;
  }

  static int maxC(int h, int pentagons)
  {
    int level, faces, bndry, cur_bndry, c, inner_faces;
    bndry = 2 * h - 6 + pentagons;
    cur_bndry = 6;
    switch (pentagons)
    {
      case 0:
	faces = 1;
        for (level = 1; 6 + 12 * level <= bndry; level++)
	{
          faces += 6 * level;
          cur_bndry = 6 + 12 * level;
        }                                                                               inner_faces = faces;                                                            if (cur_bndry < bndry) {
          while (cur_bndry + 2 * ((faces - inner_faces) / level) + 2 <= bndry)
	  {
	    faces++;
          }
          faces--;
        }
        c = 2 * faces + h - 2;
        if (! (c >= 6 && h >= 6)) {
          c = 0;
        }
	break;
      case 1:
	faces = 1;
        for (level = 1; 5 + 10 * level <= bndry; level++)
	{
          faces += (5 * level);
          cur_bndry = 5 + 10 * level;
        }
        inner_faces = faces;
        if (cur_bndry < bndry) {
          while (cur_bndry + 2 * ((faces - inner_faces) / level) + 2 <= bndry)
	  {
	    faces++;
          }
          if (level > 1) {
            faces--;
          }
        }
        c = 2 * faces + h - 2;
        if (! (c >= 5 && h >= 5)) {
          c = 0;
        }
        break;
      case 2:
	faces = 2;
        for (level = 1; 8 + 8 * level <= bndry; level++) {
          faces += (4 * level + 2);
          cur_bndry = 8 + 8 * level;
        }
        if (level == 1) {
          faces--;                                                                        if (bndry == 12) {                                                                 faces--;
          }                                                                             }
        if (cur_bndry + 2 <= bndry) {
          cur_bndry += 2;
          faces += level;
          if (cur_bndry + 2 <= bndry) {
            cur_bndry += 2;
            faces += level;
            if (cur_bndry + 2 <= bndry) {
              cur_bndry += 2;
              faces += level + 1;
              if (cur_bndry + 2 <= bndry) {
                cur_bndry += 2;
                faces += level;
              }
            }
          }
        }
        c = 2 * faces + h - 2;
        if (! (c >= 8 && h >= 6)) {
          c = 0;
        }
        break;
      case 3:
	faces = 3;
        for (level = 1; 9 + 6 * level <= bndry; level++)
	{
          faces += (3 * level + 3);
          cur_bndry = 9 + 6 * level;
        }
        if (level == 1) {
          faces--;                                                                        if (bndry == 11) {                                                                 faces--;
          }                                                                             }
        if (cur_bndry + 2 <= bndry) {
          cur_bndry += 2;
	  faces += level;
          if (cur_bndry + 2 <= bndry) {
            cur_bndry += 2;
	    faces += level + 1;
            if (cur_bndry + 2 <= bndry) {
              cur_bndry += 2;
	      faces += level;
            }
          }
        }
        c = 2 * faces + h - 2;
        if (! (c >= 10 && h >= 6)) {
          c = 0;
        }
        break;
      case 4:
	faces = 4;
        for (level = 1; 10 + 4 * level <= bndry; level++)
	{
          faces += (2 * level + 4);
          cur_bndry = 10 + 4 * level;
        }
        if (level == 1) {
          faces--;
          if (bndry == 10) {
            faces -= 2;
          }
        }
        if (cur_bndry + 2 <= bndry) {
          cur_bndry += 2;
          faces += level + 1;
          if (cur_bndry + 2 <= bndry) {
            cur_bndry += 2;
            faces += level;
          }
        }
        c = 2 * faces + h - 2;
        if (! (c >= 12 && h >= 6)) {
          c = 0;
        }
        break;
      case 5:
        if (h == 7) {
          c = 29;
          break;
        }
        if (h == 6) {
          c = 16;
          break;
        }
	faces = 6;
        for (level = 1; 11 + 2 * level <= bndry; level++)
	{
          faces += (level + 5);
          cur_bndry = 11 + 2 * level;
        }
        if (level == 1) {
          faces--;
        }
        if (cur_bndry + 2 <= bndry) {
          cur_bndry += 2;
          faces += level;
        }
        c = 2 * faces + h - 2;
        if (! (c >= 14 && h >= 6)) {
          c = 0;
        }
        break;
      default:
	throw new RuntimeException("maxC called with invalid pentagon number");
    }
    return c;
  }

  static int minBoundary(int hexagons, int pentagons)
  {
    int level, faces, total_faces, bndry_len;
    total_faces = hexagons + pentagons;
    switch (pentagons)
    {
      case 0:
	faces = 1;
        for (level = 1; faces + (6 * level) <= total_faces; level++)
	{
          faces += (6 * level);
	}
        bndry_len = 6 + 12 * (level - 1);
        faces = total_faces - faces;
        if (faces != 0) bndry_len += 2 * (faces / level) + 2;
        break;
      case 1:
	faces = 1;
        for (level = 1; faces + (5 * level) <= total_faces; level++)
	{
          faces += (5 * level);
	}
        bndry_len = 5 + 10 * (level - 1);
        faces = total_faces - faces;
        if (faces != 0) bndry_len += 2 * (faces / level) + 2;
        break;
      case 2:
	faces = 2;
        for (level = 1; faces + 2 + (4 * level) <= total_faces; level++)
	{
          faces += (4 * level + 2);
	}
        bndry_len = 8 + 8 * (level - 1);
        faces = total_faces - faces;
        if (faces != 0) {
          bndry_len += 2;
          faces -= level;
          if (faces > 0) {
            bndry_len += 2;
            faces -= level;
            if (faces > 0) {
              bndry_len += 2;
	      faces -= (level + 1);
              if (faces > 0)
                bndry_len += 2;
            }
          }
        }
        break;
      case 3:
	faces = 3;
        for (level = 1; faces + 3 + (3 * level) <= total_faces; level++)
	{
          faces += (3 * level + 3);
	}
        bndry_len = 9 + 6 * (level - 1);
        faces = total_faces - faces;
        if (faces != 0) {
          bndry_len += 2;
          faces -= level;
          if (faces > 0) {
            bndry_len += 2;
            faces -= (level + 1);
            if (faces > 0)
              bndry_len += 2;
          }
        }
        break;
      case 4:
	faces = 4;
        for (level = 1; faces + 4 + (2 * level) <= total_faces; level++)
	{
          faces += (2 * level + 4);
	}
        bndry_len = 10 + 4 * (level - 1);
        faces = total_faces - faces;
        if (faces != 0) {
          bndry_len += 2;
          faces -= (level + 1);
          if (faces > 0)
            bndry_len += 2;
        }
        break;
      case 5:
        if (hexagons == 0) {
          bndry_len = 11;
          break;
        }
	faces = 6;
        for (level = 1; faces + 5 + level <= total_faces; level++)
	{
          faces += (level + 5);
	}
        bndry_len = 11 + 2 * (level - 1);
        faces = total_faces - faces;
        if (faces != 0)
          bndry_len += 2;
        break;
      default:
	throw new RuntimeException("minBoundary called with invalid pentagon number");
    }
    return bndry_len;
  }

/*
  These are also adapted from the C&Tcl/Tk version.
*/
  static int minC(int h, int pentagons, int maxC)
  {
    if (pentagons == 5 && maxC == 16) {
      return 14;
    } else if (pentagons == 4 && maxC == 12) {
      return 12;
    }
    int minC = Math.max(5, 2*h - 6 + pentagons);
    if (minC % 2 != h % 2) {
      minC += 1;
    }
    return minC;
  }

  static int maxH(int c, int pentagons)
  {
    int maxH = (c + 6 - pentagons) / 2;
    if (maxH % 2 != c % 2) {
      maxH -= 1;
    }
    return maxH;
  }

  void calculatePentagonLimits()
  {
    minPent = maxPent = -1;
    for (int pent = 0; pent <= MAX_PENT; ++pent)
    {
      int hexagons = (C - 2*pent - H + 2) / 2;
      int bndry_len = 2*H - 6 + pent;
      int inner = C - bndry_len;
      int min_bndry = minBoundary(hexagons, pent);
      if (inner >= 0 && bndry_len >= min_bndry) {
        if (minPent == -1) {
	  minPent = pent;
	}
	maxPent = pent;
      }
    }
  }
}

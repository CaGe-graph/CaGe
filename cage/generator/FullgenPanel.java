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
import java.util.ArrayList;
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
import lisken.uitoolbox.MinMaxRestrictor;
import lisken.uitoolbox.PushButtonDecoration;
import lisken.uitoolbox.UItoolbox;

public class FullgenPanel extends GeneratorPanel {

    public static final int MIN_ATOMS = 20;
    public static final int MAX_ATOMS = 250;
    private static final int DEFAULT_ATOMS = 60;
    
    
    private static final String[] SYMMETRY = new String[]{
        "C1", "C2", "Ci", "Cs",
        "C3", "D2", "S4", "C2v",
        "C2h", "D3", "S6", "C3v",
        "C3h", "D2h", "D2d", "D5",
        "D6", "D3h", "D3d", "T",
        "D5h", "D5d", "D6h", "D6d",
        "Td", "Th", "I", "Ih"
    };
    private static final int SYMMETRIES_COUNT = SYMMETRY.length;
    private static final int SYMMETRIES_ROWS = 4;
    
    private boolean embedderIsConstant = false;
    private JPanel FullgenAtomsPanel = new JPanel();
    private EnhancedSlider minAtomsSlider = new EnhancedSlider();
    private EnhancedSlider maxAtomsSlider = new EnhancedSlider();
    private JPanel FullgenExtrasPanel = new JPanel();
    private JCheckBox minEqMax = new JCheckBox();
    private JCheckBox ipr = new JCheckBox();
    private JCheckBox dual = new JCheckBox();
    private JCheckBox spiralStats = new JCheckBox();
    private JCheckBox symmStats = new JCheckBox();
    private JToggleButton symmetryFilterButton = new JToggleButton();
    private JButton symmetriesOkButton = new JButton();
    private JButton symmetriesAllButton = new JButton();
    private FlaggedJDialog symmetriesDialog = new FlaggedJDialog((Frame) null, "Fullgen - symmetry filter", true);
    private AbstractButton[] symmetryButton = new AbstractButton[SYMMETRIES_COUNT];
    private boolean[] selectedSymmetry = new boolean[SYMMETRIES_COUNT];
    private int selectedSymmetries = 0;
    
    private ActionListener actionListener = new ActionListener() {


        public void actionPerformed(ActionEvent e) {
            String actionCommand = e.getActionCommand();
            switch (actionCommand.charAt(0)) {
                case 'D':
                    embedderIsConstant = false;
                    break;
                case 'F':
                    symmetryFilter();
                    break;
                case 'a':
                    boolean selected = actionCommand.charAt(1) == '+';
                    for (int i = 0; i < SYMMETRIES_COUNT; ++i) {
                        symmetryButton[i].setSelected(selected);
                    }
                    selectedSymmetries = selected ? SYMMETRIES_COUNT : 0;
                    symmetriesOkButton.setEnabled(selectedSymmetries > 0);
                    symmetryFilterButton.setSelected(selectedSymmetries < SYMMETRIES_COUNT);
                    break;
                case 's':
                    AbstractButton sb = (AbstractButton) e.getSource();
                    selectedSymmetries += sb.isSelected() ? +1 : -1;
                    symmetriesOkButton.setEnabled(selectedSymmetries > 0);
                    symmetryFilterButton.setSelected(selectedSymmetries < SYMMETRIES_COUNT);
                    break;
            }
        }
    };

    public FullgenPanel() {
        JLabel minAtomsLabel = new JLabel("minimum number of Atoms");
        minAtomsLabel.setLabelFor(minAtomsSlider.slider());
        minAtomsLabel.setDisplayedMnemonic(KeyEvent.VK_N);
        JLabel maxAtomsLabel = new JLabel("maximum number of Atoms");
        maxAtomsLabel.setLabelFor(maxAtomsSlider.slider());
        maxAtomsLabel.setDisplayedMnemonic(KeyEvent.VK_X);
        minAtomsSlider.setMajorTickSpacing(MAX_ATOMS - MIN_ATOMS);
        minAtomsSlider.setSnapToTicks(true);
        minAtomsSlider.setMinimum(MIN_ATOMS);
        minAtomsSlider.setMaximum(MAX_ATOMS);
        minAtomsSlider.setMinorTickSpacing(2 - (MAX_ATOMS - MIN_ATOMS) % 2);
        minAtomsSlider.setPaintMinorTicks(false);
        minAtomsSlider.setPaintLabels(true);
        minAtomsSlider.setPaintTicks(true);
        minAtomsSlider.setSnapWhileDragging(minAtomsSlider.getMinorTickSpacing());
        minAtomsSlider.setClickScrollByBlock(false);
        minAtomsSlider.setValue(DEFAULT_ATOMS);
        maxAtomsSlider.setMajorTickSpacing(MAX_ATOMS - MIN_ATOMS);
        maxAtomsSlider.setSnapToTicks(true);
        maxAtomsSlider.setMinimum(MIN_ATOMS);
        maxAtomsSlider.setMaximum(MAX_ATOMS);
        maxAtomsSlider.setMinorTickSpacing(2 - (MAX_ATOMS - MIN_ATOMS) % 2);
        maxAtomsSlider.setPaintMinorTicks(false);
        maxAtomsSlider.setPaintLabels(true);
        maxAtomsSlider.setPaintTicks(true);
        maxAtomsSlider.setSnapWhileDragging(maxAtomsSlider.getMinorTickSpacing());
        maxAtomsSlider.setClickScrollByBlock(false);
        maxAtomsSlider.setValue(DEFAULT_ATOMS);
        minEqMax.setText("min = max");
        minEqMax.setSelected(true);
        minEqMax.setMnemonic(KeyEvent.VK_M);
        MinMaxRestrictor.keepConsistentOrEqual(minAtomsSlider.getModel(), maxAtomsSlider.getModel(), minEqMax.getModel());
        FullgenAtomsPanel.setLayout(new GridBagLayout());
        ipr.setText("Isolated Pentagons (ipr)");
        ipr.setMnemonic(KeyEvent.VK_I);
        dual.setText("output Dual graph (triangulation)");
        dual.setMnemonic(KeyEvent.VK_D);
        dual.setActionCommand("Dual");
        dual.addActionListener(actionListener);
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
        symmetryFilterButton.addActionListener(actionListener);
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
        FullgenExtrasPanel.add(symmStats,
                new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.BOTH,
                new Insets(0, 0, 0, 0), 0, 0));
        FullgenExtrasPanel.add(symmetryFilterButton,
                new GridBagConstraints(3, 0, 1, 2, 1.0, 1.0,
                GridBagConstraints.NORTHWEST, GridBagConstraints.NONE,
                new Insets(0, 0, 0, 0), 0, 0));
        this.setLayout(new GridBagLayout());
        this.add(FullgenAtomsPanel,
                new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.BOTH,
                new Insets(0, 0, 0, 0), 0, 0));
        this.add(new JSeparator(SwingConstants.HORIZONTAL),
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
        symmetryButtonPanel.setLayout(new GridLayout(SYMMETRIES_ROWS, 0, 10, 10));
        symmetryButtonPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        int symmCols = (SYMMETRIES_COUNT - 1) / SYMMETRIES_ROWS + 1;
        for (int i = 0; i < SYMMETRIES_COUNT; ++i) {
            int k = (i % symmCols) * SYMMETRIES_ROWS + (i / symmCols);
            symmetryButton[k] = new JToggleButton(SYMMETRY[k]);
            symmetryButton[k].setBorder(BorderFactory.createEmptyBorder(3, 7, 3, 7));
            PushButtonDecoration.decorate(symmetryButton[k], true);
            symmetryButton[k].setSelected(true);
            selectedSymmetry[k] = true;
            symmetryButton[k].setActionCommand("s");
            symmetryButton[k].addActionListener(actionListener);
            symmetryButtonPanel.add(symmetryButton[k]);
        }
        selectedSymmetries = SYMMETRIES_COUNT;
        symmetriesContent.add(symmetryButtonPanel);
        JPanel symmetriesFinishPanel = new JPanel();
        symmetriesAllButton.setText("Set all");
        symmetriesAllButton.setMnemonic(KeyEvent.VK_S);
        symmetriesAllButton.setActionCommand("a+");
        symmetriesAllButton.addActionListener(actionListener);
        symmetriesFinishPanel.add(symmetriesAllButton);
        JButton symmetriesNoneButton = new JButton("Clear all");
        symmetriesNoneButton.setMnemonic(KeyEvent.VK_C);
        symmetriesNoneButton.setActionCommand("a-");
        symmetriesNoneButton.addActionListener(actionListener);
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
    }

    public void symmetryFilter() {
        symmetryFilterButton.setSelected(selectedSymmetries < SYMMETRIES_COUNT);
        symmetriesDialog.setSuccess(false);
        symmetriesAllButton.requestFocus();
        symmetriesDialog.setVisible(true);
        if (symmetriesDialog.getSuccess()) {
            for (int i = 0; i < SYMMETRIES_COUNT; ++i) {
                selectedSymmetry[i] = symmetryButton[i].isSelected();
            }
        } else {
            selectedSymmetries = 0;
            for (int i = 0; i < SYMMETRIES_COUNT; ++i) {
                symmetryButton[i].setSelected(selectedSymmetry[i]);
                selectedSymmetries += selectedSymmetry[i] ? 1 : 0;
            }
        }
        symmetriesOkButton.setEnabled(selectedSymmetries > 0);
        // actually, selectedSymmetries is guaranteed to be positive
        symmetryFilterButton.setSelected(selectedSymmetries < SYMMETRIES_COUNT);
    }

    public GeneratorInfo getGeneratorInfo() {
        String filename = "";
        ArrayList<String> command = new ArrayList<String>();

        int min = minAtomsSlider.getValue();
        int max = maxAtomsSlider.getValue();

        command.add("fullgen");
        command.add(Integer.toString(max));
        filename += "full_" + max;
        if (max != min) {
            command.add("start");
            command.add(Integer.toString(min));
            filename += "_start_" + min;
        }
        if (ipr.isSelected()) {
            command.add("ipr");
            filename += "_ipr";
        }
        if (spiralStats.isSelected()) {
            command.add("spistat");
        }
        if (symmStats.isSelected()) {
            command.add("symstat");
        }
        if (selectedSymmetries < SYMMETRIES_COUNT) {
            for (int k = 0; k < SYMMETRIES_COUNT; ++k) {
                if (selectedSymmetry[k]) {
                    command.add("symm");
                    command.add(SYMMETRY[k]);
                    filename += "_" + SYMMETRY[k];
                }
            }
        }
        command.add("code");
        if (dual.isSelected()) {
            command.add("7");
        } else {
            command.add("1");
        }
        command.add("stdout");
        command.add("logerr");

        String[][] generator = new String[1][command.size()];
        generator[0] = command.toArray(generator[0]);

        String[][] embed2D = {{"embed"}};
        String[][] embed3D;
        if (dual.isSelected()) {
            embed3D = new String[][]{{"embed", "-d3"}};
        } else {
            embed3D = new String[][]{{"embed", "-d3", "-it"}};
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

    public void showing() {
    }

    public static void main(String[] args) {
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

            @Override
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

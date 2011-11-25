package cage.generator;

import cage.ElementRule;
import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;
import cage.ValencyElementRule;

import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.ArrayList;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import javax.swing.JToggleButton;
import javax.swing.SwingConstants;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.MinMaxRestrictor;
import lisken.uitoolbox.UItoolbox;

public class FullgenPanel extends GeneratorPanel {

    public static final int MIN_ATOMS = 20;
    public static final int MAX_ATOMS = 250;
    private static final int DEFAULT_ATOMS = 60;
    
    private boolean embedderIsConstant = false;
    private EnhancedSlider minAtomsSlider = new EnhancedSlider();
    private EnhancedSlider maxAtomsSlider = new EnhancedSlider();
    private JCheckBox ipr = new JCheckBox();
    private JCheckBox dual = new JCheckBox();
    private JCheckBox spiralStats = new JCheckBox();
    private JCheckBox symmStats = new JCheckBox();
    private JToggleButton symmetryFilterButton = new JToggleButton();
    
    private SymmetriesDialog symmetriesDialog = new SymmetriesDialog(null, "Fullerenes - symmetry filter", true);

    public FullgenPanel() {
        initGui();
        
        symmetriesDialog.addChangeListener(new ChangeListener() {

            public void stateChanged(ChangeEvent e) {
                symmetryFilterButton.setSelected(!symmetriesDialog.areAllSymmetriesSelected());
            }
        });
    }

    private void initGui() {
        setLayout(new BoxLayout(this, BoxLayout.PAGE_AXIS));
        add(buildAtomsSelectionPanel());
        add(Box.createVerticalStrut(25));
        add(new JSeparator(SwingConstants.HORIZONTAL));
        add(Box.createVerticalStrut(25));
        add(buildFullerenesExtrasPanel());
    }

    private JPanel buildAtomsSelectionPanel() {
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
        JCheckBox minEqMax = new JCheckBox("min = max");
        minEqMax.setSelected(true);
        minEqMax.setMnemonic(KeyEvent.VK_M);
        MinMaxRestrictor.keepConsistentOrEqual(minAtomsSlider.getModel(), maxAtomsSlider.getModel(), minEqMax.getModel());
        
        JPanel atomsSelectionPanel = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0,
                GridBagConstraints.WEST, GridBagConstraints.BOTH,
                new Insets(0, 5, 5, 0), 0, 0);
        atomsSelectionPanel.add(minAtomsLabel, gbc);
        
        gbc.gridx = 1;
        atomsSelectionPanel.add(maxAtomsLabel, gbc);
        
        gbc.gridx = 0;
        gbc.gridy = 1;
        gbc.weightx = gbc.weighty = 1.0;
        gbc.anchor = GridBagConstraints.CENTER;
        gbc.fill = GridBagConstraints.HORIZONTAL;
        gbc.insets = new Insets(0, 0, 0, 20);
        atomsSelectionPanel.add(minAtomsSlider, gbc);
        
        gbc.gridx = 1;
        atomsSelectionPanel.add(maxAtomsSlider, gbc);
        
        gbc.gridx = 2;
        gbc.weightx = 0.001;
        gbc.anchor = GridBagConstraints.WEST;
        gbc.fill = GridBagConstraints.NONE;
        gbc.insets = new Insets(0, 0, 0, 0);
        atomsSelectionPanel.add(minEqMax, gbc);
        
        return atomsSelectionPanel;
    }

    private JPanel buildFullerenesExtrasPanel() {
        ipr.setText("Isolated Pentagons (ipr)");
        ipr.setMnemonic(KeyEvent.VK_I);
        dual.setText("output Dual graph (triangulation)");
        dual.setMnemonic(KeyEvent.VK_D);
        dual.setActionCommand("Dual");
        dual.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                //TODO: is this needed? Strange that the state of the check box is not looked up
                embedderIsConstant = false;
            }
        });
        spiralStats.setText("Spiral Statistics");
        spiralStats.setMnemonic(KeyEvent.VK_P);
        symmStats.setText("Symmetry Statistics");
        symmStats.setMnemonic(KeyEvent.VK_Y);
        symmetryFilterButton.setText("Symmetry Filter");
        symmetryFilterButton.setMnemonic(KeyEvent.VK_F);
        symmetryFilterButton.setActionCommand("F");
        symmetryFilterButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                showSymmetryFilterDialog();
            }
        });
        
        JPanel fullerenesExtrasPanel = new JPanel(new GridBagLayout());
        final GridBagConstraints gbc = new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
                                       GridBagConstraints.CENTER, GridBagConstraints.BOTH,
                                       new Insets(0, 0, 0, 0), 0, 0);
        fullerenesExtrasPanel.add(Box.createRigidArea(new Dimension(0, 0)), gbc);
        gbc.gridx = 1;
        fullerenesExtrasPanel.add(ipr, gbc);
        gbc.gridy = 1;
        fullerenesExtrasPanel.add(dual, gbc);
        gbc.gridy = 2;
        fullerenesExtrasPanel.add(symmStats, gbc);
        gbc.gridx = 3;
        gbc.gridy = 0;
        gbc.gridheight = 2;
        gbc.anchor = GridBagConstraints.NORTHWEST;
        gbc.fill = GridBagConstraints.NONE;
        fullerenesExtrasPanel.add(symmetryFilterButton, gbc);
        return fullerenesExtrasPanel;
    }

    private void showSymmetryFilterDialog() {
        symmetryFilterButton.setSelected(!symmetriesDialog.areAllSymmetriesSelected());
        symmetriesDialog.setVisible(true);
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
        if (!symmetriesDialog.areAllSymmetriesSelected()) {
            for (String symmetry : symmetriesDialog.getSelectedSymmetries()) {
                command.add("symm");
                command.add(symmetry);
                filename += "_" + symmetry;
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

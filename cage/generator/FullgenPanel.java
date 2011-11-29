package cage.generator;

import cage.ElementRule;
import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;
import cage.ValencyElementRule;

import java.awt.Color;
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
import javax.swing.JTextArea;
import javax.swing.JToggleButton;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.MinMaxRestrictor;
import lisken.uitoolbox.RevealableComponent;
import lisken.uitoolbox.UItoolbox;

public class FullgenPanel extends GeneratorPanel {

    public static final int MIN_ATOMS = 20;
    public static final int MAX_ATOMS = 250;
    public static final int ATLAS_ORDER_MAX_ATOMS = 100;
    private static final int DEFAULT_ATOMS = 60;
    
    private boolean embedderIsConstant = false;
    private EnhancedSlider minAtomsSlider = new EnhancedSlider();
    private EnhancedSlider maxAtomsSlider = new EnhancedSlider();
    private JCheckBox minEqMax;
    private JCheckBox ipr;
    private JCheckBox dual;
    private JCheckBox spiralStats;
    private JCheckBox symmStats;
    private JCheckBox atlasOrder;
    private JToggleButton symmetryFilterButton;
    
    private SymmetriesDialog symmetriesDialog = new SymmetriesDialog(null, "Fullerenes - symmetry filter", true);
    
    private ChangeListener sliderListener = new ChangeListener() {
        public void stateChanged(ChangeEvent e) {
            getNextButton().setEnabled(isValidConfiguration());
        }
    };

    public FullgenPanel() {
        initGui();
        
        symmetriesDialog.addChangeListener(new ChangeListener() {

            public void stateChanged(ChangeEvent e) {
                if(symmetriesDialog.areAllSymmetriesSelected()){
                    SwingUtilities.invokeLater(new Runnable() {
                        public void run() {
                            atlasOrder.setEnabled(true);
                            symmetryFilterButton.setSelected(false);
                            atlasOrder.setText("Atlas order");
                        }
                    });
                } else {
                    SwingUtilities.invokeLater(new Runnable() {
                        public void run() {
                            atlasOrder.setEnabled(false);
                            atlasOrder.setSelected(false);
                            symmetryFilterButton.setSelected(false);
                            atlasOrder.setText("Atlas order (Remove symmetry restrictions to enable)");
                        }
                    });
                }
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
        
        minAtomsSlider.setSnapToTicks(true);
        minAtomsSlider.setPaintMinorTicks(false);
        minAtomsSlider.setPaintLabels(true);
        minAtomsSlider.setPaintTicks(true);
        minAtomsSlider.setClickScrollByBlock(false);
        configureSliderBounds(minAtomsSlider, MIN_ATOMS, MAX_ATOMS);
        minAtomsSlider.setValue(DEFAULT_ATOMS);
        minAtomsSlider.addChangeListener(sliderListener);
        
        maxAtomsSlider.setSnapToTicks(true);
        maxAtomsSlider.setPaintMinorTicks(false);
        maxAtomsSlider.setPaintLabels(true);
        maxAtomsSlider.setPaintTicks(true);
        maxAtomsSlider.setClickScrollByBlock(false);
        configureSliderBounds(maxAtomsSlider, MIN_ATOMS, MAX_ATOMS);
        maxAtomsSlider.setValue(DEFAULT_ATOMS);
        maxAtomsSlider.addChangeListener(sliderListener);
        
        minEqMax = new JCheckBox("min = max", true);
        minEqMax.setMnemonic(KeyEvent.VK_M);
        MinMaxRestrictor.keepConsistentOrEqual(minAtomsSlider.getModel(), maxAtomsSlider.getModel(), minEqMax.getModel());
        minEqMax.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                if(!minEqMax.isSelected()){
                    SwingUtilities.invokeLater(new Runnable() {
                        public void run() {
                            atlasOrder.setSelected(false);
                        }
                    });
                }
            }
        });
        
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
    
    /*
     * Utility method to set the bounds of a slider
     */
    private void configureSliderBounds(EnhancedSlider slider, int minimum, int maximum){
        slider.setMajorTickSpacing(maximum - minimum);
        slider.setMinimum(minimum);
        slider.setMaximum(maximum);
        slider.setMinorTickSpacing(2 - (maximum - minimum) % 2);
        slider.setSnapWhileDragging(slider.getMinorTickSpacing());
    }

    private JPanel buildFullerenesExtrasPanel() {
        //only generate IPR fullerenes or not?
        ipr = new JCheckBox("Isolated Pentagons (ipr)");
        ipr.setMnemonic(KeyEvent.VK_I);
        
        //output the dual graphs?
        dual = new JCheckBox("output Dual graph (triangulation)");
        dual.setMnemonic(KeyEvent.VK_D);
        dual.setActionCommand("Dual");
        dual.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                //TODO: is this needed? Strange that the state of the check box is not looked up
                embedderIsConstant = false;
            }
        });
        dual.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                if(dual.isSelected()){
                    SwingUtilities.invokeLater(new Runnable() {
                        public void run() {
                            atlasOrder.setSelected(false);
                        }
                    });
                }
            }
        });
        
        //output spiral statistics?
        spiralStats = new JCheckBox("Spiral Statistics");
        spiralStats.setMnemonic(KeyEvent.VK_P);
        
        //output symmetry statistics?
        symmStats = new JCheckBox("Symmetry Statistics");
        symmStats.setMnemonic(KeyEvent.VK_Y);
        
        //output graphs in atlas order
        atlasOrder = new JCheckBox("Atlas order");
        atlasOrder.setMnemonic(KeyEvent.VK_A);
        
        //warning to show when atlas order is chosen
        JTextArea warningText = new JTextArea("Warning: when fullerenes need to be output in atlas order, " +
                                            "the generator will first generate all fullerenes and then " +
                                            "sort them. This might take some time. Therefore this option " +
                                            "is only available for up to 100 vertices.");
        warningText.setWrapStyleWord(true);
        warningText.setLineWrap(true);
        warningText.setEnabled(false);
        warningText.setDisabledTextColor(Color.RED);
        final RevealableComponent<JTextArea> warningField = new RevealableComponent<JTextArea>(warningText, false);
        warningText.setBackground(warningField.getBackground());
        
        //impose some restrictions when atlas order is chosen
        atlasOrder.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                if(atlasOrder.isSelected()){
                    SwingUtilities.invokeLater(new Runnable() {
                        public void run() {
                            minEqMax.setSelected(true);
                            dual.setSelected(false);
                            symmetryFilterButton.setEnabled(false);
                            warningField.setRevealed(true);
                            getNextButton().setEnabled(isValidConfiguration());
                            repaint();
                        }
                    });
                } else {
                    SwingUtilities.invokeLater(new Runnable() {
                        public void run() {
                            symmetryFilterButton.setEnabled(true);
                            warningField.setRevealed(false);
                            getNextButton().setEnabled(isValidConfiguration());
                            repaint();
                        }
                    });
                }
            }
        });
        
        symmetryFilterButton = new JToggleButton("Symmetry Filter");
        symmetryFilterButton.setMnemonic(KeyEvent.VK_F);
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
        gbc.gridy++;
        fullerenesExtrasPanel.add(dual, gbc);
        gbc.gridy++;
        fullerenesExtrasPanel.add(symmStats, gbc);
        gbc.gridy++;
        fullerenesExtrasPanel.add(atlasOrder, gbc);
        gbc.gridx = 3;
        gbc.gridheight = gbc.gridy + 1;
        gbc.gridy = 0;
        gbc.anchor = GridBagConstraints.CENTER;
        gbc.fill = GridBagConstraints.NONE;
        fullerenesExtrasPanel.add(symmetryFilterButton, gbc);
        gbc.gridx = 0;
        gbc.gridy = gbc.gridheight;
        gbc.gridheight = 1;
        gbc.gridwidth = 4;
        gbc.fill = GridBagConstraints.HORIZONTAL;
        gbc.insets = new Insets(10, 0, 0, 0);
        fullerenesExtrasPanel.add(warningField, gbc);
        return fullerenesExtrasPanel;
    }

    private void showSymmetryFilterDialog() {
        symmetryFilterButton.setSelected(!symmetriesDialog.areAllSymmetriesSelected());
        symmetriesDialog.setVisible(true);
    }
    
    private boolean isValidConfiguration() {
        return atlasOrder.isSelected() != (minAtomsSlider.getValue() > 100);
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
            if(atlasOrder.isSelected()){
                command.add("3");
                command.add("list");
                if(min<=90){
                    command.add("100000");
                } else {
                    command.add("285914");
                }
            } else {
                command.add("1");
            }
        }
        command.add("stdout");
        command.add("logerr");

        String[][] generator;
        if(atlasOrder.isSelected()){
            generator = new String[2][command.size()];
            generator[0] = command.toArray(generator[0]);
            generator[1] = new String[]{"short_spiral_to_pl"};
        } else {
            generator = new String[1][command.size()];
            generator[0] = command.toArray(generator[0]);
        }

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

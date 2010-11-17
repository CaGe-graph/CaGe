package cage.generator;

import cage.CaGe;
import cage.Embedder;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;
import cage.ValencyElementRule;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.util.Enumeration;
import javax.swing.AbstractButton;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JSeparator;
import javax.swing.JTextField;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.JTextComponentFocusSelector;

/**
 * Panel that allows the configuration of a generator that is not in
 * CaGe by default.
 */
public class ExternalPanel extends GeneratorPanel implements ActionListener {

    private JButton defaultButton;
    private JRadioButton externalCmdButton = new JRadioButton("generator command: ");
    private JTextField externalCmd;
    private JRadioButton fromFileButton = new JRadioButton("Input from file: ");
    private JTextField fromFileCmd;
    private ButtonGroup commandGroup = new ButtonGroup();
    private ButtonGroup embeddedModeGroup = new ButtonGroup();
    private ButtonGroup embedTemplateGroup = new ButtonGroup();
    private boolean nonNullCmd,  embedderRequired;
    private boolean embedderIsConstant = false;
    private EnhancedSlider embedIntensity = new EnhancedSlider(CaGe.debugMode);
    private JCheckBox embedExpertBox = new JCheckBox();

    public ExternalPanel() {
        setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));

        //------------Command section-----------------
        JPanel commandPanel = new JPanel();
        externalCmd = new JTextField(30);
        nonNullCmd = false;
        externalCmd.setActionCommand("c");
        externalCmd.addActionListener(this);
        externalCmd.getDocument().addDocumentListener(new DocumentListener() {

            public void insertUpdate(DocumentEvent e) {
                checkCmd();
            }

            public void removeUpdate(DocumentEvent e) {
                checkCmd();
            }

            public void changedUpdate(DocumentEvent e) {
                checkCmd();
            }
        });
        new JTextComponentFocusSelector(externalCmd);
        commandGroup.add(externalCmdButton);
        externalCmdButton.setMnemonic(KeyEvent.VK_C);
        externalCmdButton.setSelected(true);
        externalCmdButton.addChangeListener(new ChangeListener() {

            public void stateChanged(ChangeEvent e) {
                externalCmd.setEnabled(externalCmdButton.isSelected());
                if (externalCmdButton.isSelected()) {
                    externalCmd.requestFocusInWindow();
                    checkCmd();
                }
            }
        });

        fromFileCmd = new JTextField(30);
        fromFileCmd.setActionCommand("f");
        fromFileCmd.addActionListener(this);
        fromFileCmd.getDocument().addDocumentListener(new DocumentListener() {

            public void insertUpdate(DocumentEvent e) {
                checkCmd();
            }

            public void removeUpdate(DocumentEvent e) {
                checkCmd();
            }

            public void changedUpdate(DocumentEvent e) {
                checkCmd();
            }
        });
        fromFileCmd.setEnabled(false);
        new JTextComponentFocusSelector(fromFileCmd);
        commandGroup.add(fromFileButton);
        fromFileButton.setMnemonic(KeyEvent.VK_F);
        fromFileButton.setSelected(false);
        fromFileButton.addChangeListener(new ChangeListener() {

            public void stateChanged(ChangeEvent e) {
                fromFileCmd.setEnabled(fromFileButton.isSelected());
                if (fromFileButton.isSelected()) {
                    fromFileCmd.requestFocusInWindow();
                    checkCmd();
                }
            }
        });

        //commandPanel.add(externalCmdLabel);
        commandPanel.setLayout(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints(0, 0, 1, 1, 0, 0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 5, 5);
        commandPanel.add(externalCmdButton, gbc);
        gbc.gridx++;
        commandPanel.add(externalCmd, gbc);
        gbc.gridx = 0;
        gbc.gridy++;
        commandPanel.add(fromFileButton, gbc);
        gbc.gridx++;
        commandPanel.add(fromFileCmd, gbc);
        add(commandPanel);
        add(Box.createVerticalStrut(20));
        add(new JSeparator(SwingConstants.HORIZONTAL));
        add(Box.createVerticalStrut(20));

        //------------Embedded Mode section-----------------
        JPanel embeddedModePanel = new JPanel();
        embeddedModePanel.setLayout(new BoxLayout(embeddedModePanel, BoxLayout.Y_AXIS));
        JRadioButton embeddedModeIgnore = new JRadioButton();
        embeddedModeIgnore.setText("create new embedding (ignore old coordinates)");
        embeddedModeIgnore.setActionCommand("i");
        embeddedModeIgnore.setMnemonic(KeyEvent.VK_N);
        embeddedModeIgnore.addActionListener(this);
        embeddedModeGroup.add(embeddedModeIgnore);
        embeddedModePanel.add(embeddedModeIgnore);
        JRadioButton embeddedModeKeep = new JRadioButton();
        embeddedModeKeep.setText("skip embedding (keep old coordinates if provided)");
        embeddedModeKeep.setActionCommand("k");
        embeddedModeKeep.setMnemonic(KeyEvent.VK_S);
        embeddedModeKeep.addActionListener(this);
        embeddedModeGroup.add(embeddedModeKeep);
        embeddedModePanel.add(embeddedModeKeep);
        JRadioButton embeddedModeRefine = new JRadioButton();
        embeddedModeRefine.setText("refine embedding (start with old coordinates if provided)");
        embeddedModeRefine.setActionCommand("r");
        embeddedModeRefine.setMnemonic(KeyEvent.VK_R);
        embeddedModeRefine.addActionListener(this);
        embeddedModeGroup.add(embeddedModeRefine);
        embeddedModePanel.add(embeddedModeRefine);
        embeddedModeIgnore.doClick();
        add(embeddedModePanel);
        add(Box.createVerticalStrut(20));
        add(new JSeparator(SwingConstants.HORIZONTAL));
        add(Box.createVerticalStrut(20));
        //------------Embedding type section--------------------
        add(new JLabel("The expected form of the graphs is"));
        add(Box.createVerticalStrut(5));
        JPanel embedTemplatePanel = new JPanel();
        embedTemplatePanel.setLayout(new BoxLayout(embedTemplatePanel, BoxLayout.Y_AXIS));
        for (int i = 0; i < CaGe.getNumberOfEmbeddingTypeFactories(); ++i) {
            Object[] types = CaGe.getEmbeddingTypeFactory(i).getEmbeddingTypes();
            for (int j = 0; j < types.length; j++) {
                Object type = types[j];
                JRadioButton embedTemplateButton = new JRadioButton(type.toString());
                embedTemplateButton.setActionCommand("e" + type.toString());
                embedTemplateButton.addActionListener(this);
                embedTemplateGroup.add(embedTemplateButton);
                embedTemplatePanel.add(embedTemplateButton);
            }
        }
        add(embedTemplatePanel);
        embedIntensity.setMinimum(10);
        embedIntensity.setMaximum(1000);
        embedIntensity.setValue(100);
        embedIntensity.setMinorTickSpacing(10);
        embedIntensity.setMajorTickSpacing(embedIntensity.getMaximum() - embedIntensity.getMinimum());
        embedIntensity.setPaintTicks(true);
        embedIntensity.setPaintLabels(true);
        embedIntensity.setSnapWhileDragging(10);
        embedIntensity.setSizeFactor(0.2);
        JLabel embedIntensityLabel = new JLabel("embed iteration count (% of default value)");
        embedIntensityLabel.setLabelFor(embedIntensity.slider());
        embedIntensityLabel.setDisplayedMnemonic(KeyEvent.VK_I);

        //------------Embedded intensity section-----------------
        add(Box.createVerticalStrut(20));
        add(embedIntensityLabel);
        add(Box.createVerticalStrut(5));
        add(embedIntensity);
        embedExpertBox.setText("edit embed commands");
        embedExpertBox.setMnemonic(KeyEvent.VK_E);
        embedExpertBox.setSelected(Systoolbox.parseBoolean(
                CaGe.config.getProperty("CaGe.ExpertMode"), false));
        embedExpertBox.setActionCommand("x");
        embedExpertBox.addActionListener(this);
        add(Box.createVerticalStrut(20));
        add(embedExpertBox);
    }

    private void checkCmd() {
        String content = externalCmdButton.isSelected() ? externalCmd.getText() : fromFileCmd.getText();
        nonNullCmd = !content.replace('|', ' ').trim().equals("");
        checkNextEnabled();
    }

    private void checkNextEnabled() {
        if (defaultButton != null) {
            defaultButton.setEnabled(nonNullCmd && !(embedderRequired && embedTemplateGroup.getSelection() == null));
        }
    }

    public void actionPerformed(ActionEvent e) {
        char actionCommand = e.getActionCommand().charAt(0);
        switch (actionCommand) {
            case 'c':
            case 'f':
                if (defaultButton != null) {
                    defaultButton.doClick();
                }
                break;
            case 'e':
                embedderIsConstant = false;
                checkNextEnabled();
                break;
            case 'i':
            case 'r':
            case 'k':
                embedderRequired = true;
                // enableEmbedControls(embedderRequired);
                checkNextEnabled();
                break;
            case 'x':
                break;
        }
    }

    /**
     * Sets whether or not the embed controls are enabled.
     *
     * @param enabled true if this component should be enabled, false otherwise
     */
    public void enableEmbedControls(boolean enabled) {
        Enumeration e = embedTemplateGroup.getElements();
        while (e.hasMoreElements()) {
            ((AbstractButton) e.nextElement()).setEnabled(enabled);
        }
        embedIntensity.setEnabled(enabled);
        embedExpertBox.setEnabled(enabled);
    }

    public GeneratorInfo getGeneratorInfo() {
        GeneratorInfo templateInfo = null;
        int embeddedMode = Embedder.IGNORE_OLD_EMBEDDING;
        switch (embeddedModeGroup.getSelection().getActionCommand().charAt(0)) {
            case 'k':
                embeddedMode = Embedder.KEEP_OLD_EMBEDDING;
                break;
            case 'r':
                embeddedMode = Embedder.REFINE_OLD_EMBEDDING;
                break;
        }

        String embeddingType = embedTemplateGroup.getSelection().getActionCommand().substring(1);
        Embedder embedder = null;
        {
            int i=0;
            while(i < CaGe.getNumberOfEmbeddingTypeFactories() && embedder == null) {
                embedder = CaGe.getEmbeddingTypeFactory(i).getEmbedderFor(embeddingType);
                i++;
            }
            if(embedder==null)
                throw new RuntimeException("Unknown embedding type " + embeddingType);
        }
        embedder.setConstant(embedderIsConstant);
        embedderIsConstant = true;
        embedder.setIntensityFactor(embedIntensity.getValue() / 100.0f);
        embedder.setMode(embeddedMode);
        boolean embedExpert =
                embedExpertBox.isEnabled() && embedExpertBox.isSelected();
        String generatorCmdLine;
        if (externalCmdButton.isSelected()) {
            generatorCmdLine = externalCmd.getText();
        } else {
            generatorCmdLine = "cat " + fromFileCmd.getText();
        }
        StringBuffer filename = new StringBuffer();
        boolean inWord = false;
        int n = generatorCmdLine.length();
        for (int i = 0; i < n; ++i) {
            char c = generatorCmdLine.charAt(i);
            if (Character.isLetterOrDigit(c)) {
                filename.append(c);
                inWord = true;
            } else if (inWord) {
                filename.append('_');
                inWord = false;
            }
        }
        n = filename.length() - 1;
        if (filename.charAt(n) == '_') {
            filename.setLength(n);
        }
        return new StaticGeneratorInfo(
                Systoolbox.parseCmdLine(generatorCmdLine),
                embedder,
                filename.toString(),
                0,
                templateInfo != null ? templateInfo.getElementRule() : new ValencyElementRule("H O C Si N S I"),
                GeneratorInfo.createExpertMode(
                GeneratorInfo.GENERATOR_EXPERT |
                (embedExpert ? GeneratorInfo.EMBED_EXPERT : 0),
                embedExpert ? 0 : GeneratorInfo.EMBED_EXPERT));
    }

    public void showing() {
        defaultButton = SwingUtilities.getRootPane(externalCmd).getDefaultButton();
        checkCmd();
        externalCmd.requestFocus();
    }
}

package cage.generator;

import cage.CaGe;
import cage.Embedder;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;
import cage.ValencyElementRule;
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
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.JTextComponentFocusSelector;

public class ExternalPanel extends GeneratorPanel implements ActionListener {

    JButton defaultButton;
    JTextField externalCmd;
    ButtonGroup embeddedModeGroup = new ButtonGroup();
    ButtonGroup embedTemplateGroup = new ButtonGroup();
    JLabel embedTemplateLabel = new JLabel();
    boolean nonNullCmd, embedderRequired;
    boolean embedderIsConstant = false;
    EnhancedSlider embedIntensity = new EnhancedSlider();
    JCheckBox embedExpertBox = new JCheckBox();

    public ExternalPanel() {
        setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
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
        JLabel externalCmdLabel = new JLabel("generator command:  ");
        externalCmdLabel.setLabelFor(externalCmd);
        externalCmdLabel.setDisplayedMnemonic(KeyEvent.VK_C);
        commandPanel.add(externalCmdLabel);
        commandPanel.add(externalCmd);
        add(commandPanel);
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
        add(Box.createVerticalStrut(20));
        add(new JSeparator(SwingConstants.HORIZONTAL));
        add(Box.createVerticalStrut(20));
        add(embeddedModePanel);
        add(Box.createVerticalStrut(20));
        add(new JSeparator(SwingConstants.HORIZONTAL));
        add(Box.createVerticalStrut(20));
        add(embedTemplateLabel);
        add(Box.createVerticalStrut(5));
        JPanel embedTemplatePanel = new JPanel();
        embedTemplatePanel.setLayout(new BoxLayout(embedTemplatePanel, BoxLayout.Y_AXIS));
        for (int i = 0; i < CaGe.generator.length; ++i) {
            if (i == CaGe.lastGeneratorChoice) {
                continue;
            }
            String generatorName = CaGe.generator[i];
            boolean enabled =
                    CaGe.getCaGePropertyAsBoolean(generatorName + ".ForExternal", true);
            if (!enabled) {
                continue;
            }
            JRadioButton embedTemplateButton = new JRadioButton();
            embedTemplateButton.setText(CaGe.config.getProperty(generatorName + ".Title"));
            if (i < 10) {
                embedTemplateButton.setMnemonic(KeyEvent.VK_0 + (i + 1) % 10);
            }
            embedTemplateButton.setActionCommand("e" + generatorName);
            embedTemplateButton.addActionListener(this);
            embedTemplateGroup.add(embedTemplateButton);
            embedTemplatePanel.add(embedTemplateButton);
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

    void checkCmd() {
        String content = externalCmd.getText();
        nonNullCmd = !content.replace('|', ' ').trim().equals("");
        checkNextEnabled();
    }

    void checkNextEnabled() {
        if (defaultButton != null) {
            defaultButton.setEnabled(nonNullCmd && !(embedderRequired && embedTemplateGroup.getSelection() == null));
        }
    }

    public void actionPerformed(ActionEvent e) {
        char actionCommand = e.getActionCommand().charAt(0);
        switch (actionCommand) {
            case 'c':
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
                embedderRequired = true;
                // enableEmbedControls(embedderRequired);
                embedTemplateLabel.setText("embed graphs as:");
                checkNextEnabled();
                break;
            case 'k':
                embedderRequired = true;
                // enableEmbedControls(embedderRequired);
                embedTemplateLabel.setText("if no coordinates provided, embed graphs as:");
                checkNextEnabled();
                break;
            case 'x':
                break;
        }
    }

    void enableEmbedControls(boolean enabled) {
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
        try {
            String templateGenerator = embedTemplateGroup.getSelection().getActionCommand().substring(1);
            GeneratorPanel templatePanel = (GeneratorPanel) Class.forName(CaGe.config.getProperty(templateGenerator + ".ConfigPanel")).newInstance();
            // Don't call templatePanel.showing() - it might try to manipulate a null default button
            templateInfo = templatePanel.getGeneratorInfo();
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        Embedder embedder = templateInfo.getEmbedder();
        embedder.setConstant(embedderIsConstant);
        embedderIsConstant = true;
        embedder.setIntensityFactor(embedIntensity.getValue() / 100.0f);
        embedder.setMode(embeddedMode);
        boolean embedExpert =
                embedExpertBox.isEnabled() && embedExpertBox.isSelected();
        String generatorCmdLine = externalCmd.getText();
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
                embedExpert ? GeneratorInfo.EMBED_EXPERT : 0,
                GeneratorInfo.GENERATOR_EXPERT |
                (embedExpert ? 0 : GeneratorInfo.EMBED_EXPERT)));
    }

    public void showing() {
        defaultButton = SwingUtilities.getRootPane(externalCmd).getDefaultButton();
        checkCmd();
        externalCmd.requestFocus();
    }
}

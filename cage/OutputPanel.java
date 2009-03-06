package cage;

import cage.utility.ComponentLogicalGroup;
import cage.utility.GenericButtonGroup;
import cage.utility.Min1ButtonGroup;
import cage.utility.OnActionClicker;
import cage.utility.OnActionClickerLayoutSwitcher;
import cage.utility.SyncButtonGroup;
import cage.viewer.CaGeViewer;
import cage.viewer.ViewerFactory;
import cage.writer.CaGeWriter;

import java.awt.CardLayout;
import java.awt.Color;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.ButtonModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JSeparator;
import javax.swing.JTextField;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.JTextComponentFocusSelector;
import lisken.uitoolbox.UItoolbox;

public class OutputPanel extends JPanel implements ActionListener, DocumentListener {

    private GeneratorInfo generatorInfo;
    private boolean generatorInfoChanged;
    private String generatorName;
    private Vector viewers2D, viewers3D, viewersXD;
    private StringBuffer viewerErrors;
    private JButton defaultButton;
    private JLabel expertLabel = new JLabel();
    private JTextField generatorCmdLine = new JTextField("");
    private JTextField embed2DCmdLine = new JTextField("");
    private JTextField embed3DCmdLine = new JTextField("");
    private JCheckBox outPreFilterCheckBox = new JCheckBox();
    private JTextField outPreFilterCommand = new JTextField();
    private JRadioButton outPreFilterNone = new JRadioButton();
    private JRadioButton outPreFilter = new JRadioButton();
    private ButtonGroup outPreFilterGroup = new ButtonGroup();
    private JCheckBox out3DCheckBox = new JCheckBox();
    private ButtonGroup out3DDestGroup = new ButtonGroup();
    private JRadioButton out3DViewer = new JRadioButton();
    private JRadioButton out3DFile = new JRadioButton();
    private JRadioButton out3DNoDest = new JRadioButton();
    private Min1ButtonGroup out3DViewerGroup = new Min1ButtonGroup("3D", false, out3DCheckBox);
    private JTextField out3DFileName = new JTextField();
    private FileFormatBox out3DFileFormat = new FileFormatBox("3D", out3DFileName);
    private JCheckBox out2DCheckBox = new JCheckBox();
    private ButtonGroup out2DDestGroup = new ButtonGroup();
    private Min1ButtonGroup out2DViewerGroup = new Min1ButtonGroup("2D", false, out2DCheckBox);
    private JRadioButton out2DViewer = new JRadioButton();
    private JRadioButton out2DFile = new JRadioButton();
    private JTextField out2DFileName = new JTextField();
    private JRadioButton out2DNoDest = new JRadioButton();
    private FileFormatBox out2DFileFormat = new FileFormatBox("2D", out2DFileName);
    private ButtonGroup outAdjDestGroup = new ButtonGroup();
    private JCheckBox outAdjCheckBox = new JCheckBox();
    private JRadioButton outAdjNoDest = new JRadioButton();
    private JRadioButton outAdjFile = new JRadioButton();
    private JTextField outAdjFileName = new JTextField();
    private FileFormatBox outAdjFileFormat = new FileFormatBox("Adjacency", outAdjFileName);

    private ComponentLogicalGroup expertControlsGroup = new ComponentLogicalGroup();
    private ComponentLogicalGroup embedControlsGroup = new ComponentLogicalGroup();
    private ComponentLogicalGroup generatorControlsGroup = new ComponentLogicalGroup();

    public OutputPanel() {
        this(null);
    }

    public OutputPanel(String gn) {
        generatorName = gn;
        generatorInfoChanged = true;

        final String FilterHint = "pipe embedded graphs through an external filter command";
        final String ShortcutHint = "hint: just choose 'Viewer' or 'File/Pipe' on the right";
        final String FilePipeHint = "start with | to send into a pipe";

        this.setLayout(new GridBagLayout());
        this.setBorder(BorderFactory.createTitledBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createCompoundBorder(
                BorderFactory.createEmptyBorder(10, 10, 10, 10),
                BorderFactory.createEtchedBorder()),
                BorderFactory.createEmptyBorder(20, 20, 20, 20)), " Output Options "));

        expertLabel.setText("generator/embedders");
        expertLabel.setForeground(Color.black);
        Font font = expertLabel.getFont();
        expertControlsGroup.addComponent(expertLabel);
        font = new Font(
                font.getName(),
                font.getStyle() & ~Font.BOLD,
                font.getSize());
        JLabel generatorLabel = new JLabel();
        generatorLabel.setText("generator");
        generatorLabel.setLabelFor(generatorCmdLine);
        generatorLabel.setDisplayedMnemonic(KeyEvent.VK_G);
        generatorControlsGroup.addComponent(generatorLabel);
        // generatorLabel.setFont(font);
        JLabel embed2DLabel = new JLabel("2D embedder");
        // embed2DLabel.setFont(font);
        embed2DLabel.setLabelFor(embed2DCmdLine);
        embed2DLabel.setDisplayedMnemonic(KeyEvent.VK_M);
        embedControlsGroup.addComponent(embed2DLabel);
        JLabel embed3DLabel = new JLabel("3D embedder");
        // embed3DLabel.setFont(font);
        embed3DLabel.setLabelFor(embed3DCmdLine);
        embed3DLabel.setDisplayedMnemonic(KeyEvent.VK_B);
        embedControlsGroup.addComponent(embed3DLabel);
        generatorCmdLine.setColumns(10);
        generatorCmdLine.setActionCommand("generator");
        generatorCmdLine.addActionListener(this);
        generatorCmdLine.getDocument().addDocumentListener(this);
        generatorControlsGroup.addComponent(generatorCmdLine);
        new JTextComponentFocusSelector(generatorCmdLine);
        embed2DCmdLine.setColumns(10);
        embed2DCmdLine.setActionCommand("e2");
        embed2DCmdLine.addActionListener(this);
        embed2DCmdLine.getDocument().addDocumentListener(this);
        new JTextComponentFocusSelector(embed2DCmdLine);
        embedControlsGroup.addComponent(embed2DCmdLine);
        embed3DCmdLine.setColumns(10);
        embed3DCmdLine.setActionCommand("e3");
        embed3DCmdLine.addActionListener(this);
        embed3DCmdLine.getDocument().addDocumentListener(this);
        new JTextComponentFocusSelector(embed3DCmdLine);
        embedControlsGroup.addComponent(embed3DCmdLine);
        JPanel expertPanel = new JPanel();
        expertPanel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        expertPanel.setLayout(new GridBagLayout());
        expertPanel.add(generatorLabel, new GridBagConstraints(0, 0, 1, 1, 0.01, 1.0, GridBagConstraints.WEST, GridBagConstraints.VERTICAL, new Insets(3, 0, 3, 5), 0, 0));
        expertPanel.add(generatorCmdLine, new GridBagConstraints(1, 0, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(3, 0, 3, 0), 0, 0));
        expertPanel.add(embed2DLabel, new GridBagConstraints(0, 1, 1, 1, 0.01, 1.0, GridBagConstraints.WEST, GridBagConstraints.VERTICAL, new Insets(3, 0, 3, 5), 0, 0));
        expertPanel.add(embed2DCmdLine, new GridBagConstraints(1, 1, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(3, 0, 3, 0), 0, 0));
        expertPanel.add(embed3DLabel, new GridBagConstraints(0, 2, 1, 1, 0.01, 1.0, GridBagConstraints.WEST, GridBagConstraints.VERTICAL, new Insets(3, 0, 3, 5), 0, 0));
        expertPanel.add(embed3DCmdLine, new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(3, 0, 3, 0), 0, 0));
        expertControlsGroup.addComponent(expertPanel);
        JSeparator expertControlsSeparator = new JSeparator(SwingConstants.HORIZONTAL);
        this.add(expertLabel, new GridBagConstraints(0, 0, 1, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(0, 17, 0, 10), 0, 0));
        this.add(expertPanel, new GridBagConstraints(1, 0, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(expertControlsSeparator, new GridBagConstraints(0, 1, GridBagConstraints.REMAINDER, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(20, 0, 20, 10), 0, 0));
        expertControlsGroup.addComponent(expertControlsSeparator);
        JPanel outPreFilterPanel = new JPanel();
        OnActionClickerLayoutSwitcher outPreFilterListener =
                new OnActionClickerLayoutSwitcher(outPreFilterCheckBox, outPreFilterPanel);
        outPreFilterCheckBox.setText("Pre-filter graphs");
        outPreFilterCheckBox.setMnemonic(KeyEvent.VK_P);
        outPreFilterCheckBox.setToolTipText(FilterHint);
        outPreFilterCheckBox.setActionCommand("outPreFilterCheckBox");
        outPreFilterCheckBox.addActionListener(this);
        outPreFilterCommand.setColumns(30);
        new JTextComponentFocusSelector(outPreFilterCommand);
        outPreFilterCommand.setToolTipText(FilterHint);
        outPreFilterCommand.addFocusListener(new FocusListener() {

            public void focusGained(FocusEvent e) {
            }

            public void focusLost(FocusEvent e) {
                outPreFilterCommand.setText(outPreFilterCommand.getText().trim());
                if (outPreFilterCommand.getText().length() == 0 && !outPreFilterCheckBox.hasFocus()) {
                    outPreFilterCheckBox.doClick();
                }
            }
        });
        outPreFilterPanel.setLayout(new CardLayout());
        outPreFilterPanel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        outPreFilterPanel.add(outPreFilter, "");
        outPreFilterPanel.add(outPreFilterNone, "");
        outPreFilterPanel.add(outPreFilterCommand, "outPreFilter");
        outPreFilterPanel.add(new JPanel(), "outPreFilterNone");
        outPreFilter.setVisible(false);
        outPreFilter.setActionCommand("outPreFilter");
        outPreFilter.addActionListener(outPreFilterListener);
        outPreFilter.setText("Pipe");
        outPreFilterNone.setVisible(false);
        outPreFilterNone.addActionListener(outPreFilterListener);
        outPreFilterNone.setActionCommand("outPreFilterNone");
        outPreFilterNone.setText("None");
        outPreFilterGroup.add(outPreFilter);
        outPreFilterGroup.add(outPreFilterNone);
        new OnActionClicker(outPreFilter, outPreFilterNone, outPreFilterCheckBox);
        this.add(outPreFilterCheckBox, new GridBagConstraints(0, 2, 1, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 10), 0, 0));
        this.add(outPreFilterPanel, new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));

        this.add(new JSeparator(SwingConstants.HORIZONTAL), new GridBagConstraints(0, 3, GridBagConstraints.REMAINDER, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(20, 0, 20, 10), 0, 0));

        JPanel out3DDestOptionsPanel = new JPanel(new CardLayout());
        OnActionClickerLayoutSwitcher out3DDestListener =
                new OnActionClickerLayoutSwitcher(out3DCheckBox, out3DDestOptionsPanel);
        out3DCheckBox.setText("3D representation");
        out3DCheckBox.setMnemonic(KeyEvent.VK_3);
        out3DCheckBox.setToolTipText(ShortcutHint);
        out3DCheckBox.addActionListener(this);
        out3DViewer.setText("Viewer");
        out3DViewer.setMnemonic(KeyEvent.VK_V);
        out3DViewer.setActionCommand("out3DViewer");
        out3DViewer.addActionListener(out3DDestListener);
        out3DViewer.addActionListener(this);
        out3DViewer.setToolTipText("send 3D embeddings to some of the installed viewers");
        out3DFile.setText("File/Pipe");
        out3DFile.setMnemonic(KeyEvent.VK_F);
        out3DFile.setActionCommand("out3DFile");
        out3DFile.addActionListener(out3DDestListener);
        out3DFile.addActionListener(this);
        out3DFile.setToolTipText("send 3D embeddings into a file or pipe");
        out3DNoDest.setText("None");
        out3DNoDest.setActionCommand("out3DNoDest");
        out3DNoDest.addActionListener(out3DDestListener);
        out3DNoDest.setVisible(false);
        JLabel out3DFileFormatLabel = new JLabel("Format");
        out3DFileFormatLabel.setLabelFor(out3DFileFormat);
        out3DFileFormatLabel.setDisplayedMnemonic(KeyEvent.VK_O);
        out3DFileFormatLabel.setBorder(BorderFactory.createEmptyBorder(0, 20, 0, 10));
        JLabel out3DFileNameLabel = new JLabel("Filename");
        JPanel out3DFilePanel = new JPanel();
        out3DFilePanel.setLayout(new BoxLayout(out3DFilePanel, BoxLayout.X_AXIS));
        out3DFilePanel.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 10));
        out3DFilePanel.add(out3DFileNameLabel, null);
        out3DFilePanel.add(out3DFileName, null);
        out3DFilePanel.add(Box.createHorizontalGlue());
        out3DFilePanel.add(out3DFileFormatLabel, null);
        out3DFilePanel.add(out3DFileFormat, null);
        out3DFileFormat.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        out3DFileFormat.setMaximumSize(out3DFileFormat.getPreferredSize());
        out3DFileNameLabel.setLabelFor(out3DFileName);
        out3DFileNameLabel.setDisplayedMnemonic(KeyEvent.VK_N);
        out3DFileNameLabel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        out3DFileNameLabel.setToolTipText(FilePipeHint);
        out3DFileName.setColumns(15);
        out3DFileName.setMaximumSize(out3DFileName.getPreferredSize());
        out3DFileName.setToolTipText(FilePipeHint);
        out3DFileName.addActionListener(this);
        JPanel out3DViewerPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 5));

        JPanel out3DDestPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10 , 5));
        if (addViewers("3D", viewers3D, out3DViewerGroup, out3DViewerPanel) > 0) {
            out3DDestGroup.add(out3DViewer);
            out3DDestPanel.add(out3DViewer, null);
            new OnActionClicker(out3DViewer, out3DNoDest, out3DCheckBox);
        } else {
            new OnActionClicker(out3DFile, out3DNoDest, out3DCheckBox);
        }
        out3DDestGroup.add(out3DFile);
        out3DDestGroup.add(out3DNoDest);

        out3DDestOptionsPanel.add(new JPanel(), "out3DNoDest");
        out3DDestOptionsPanel.add(out3DViewerPanel, "out3DViewer");
        out3DDestOptionsPanel.add(out3DFilePanel, "out3DFile");
        out3DDestPanel.add(out3DFile, null);
        out3DDestPanel.add(out3DNoDest, null);
        new JTextComponentFocusSelector(out3DFileName);
        this.add(out3DCheckBox, new GridBagConstraints(0, 4, 1, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 10), 0, 0));
        this.add(out3DDestPanel, new GridBagConstraints(1, 4, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(out3DDestOptionsPanel, new GridBagConstraints(1, 5, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));

        this.add(new JSeparator(SwingConstants.HORIZONTAL), new GridBagConstraints(0, 6, GridBagConstraints.REMAINDER, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(20, 0, 20, 10), 0, 0));

        JPanel out2DDestOptionsPanel = new JPanel(new CardLayout());
        OnActionClickerLayoutSwitcher out2DDestListener =
                new OnActionClickerLayoutSwitcher(out2DCheckBox, out2DDestOptionsPanel);
        out2DFile.addActionListener(out2DDestListener);
        out2DFile.addActionListener(this);
        out2DFile.setText("File/Pipe");
        out2DFile.setActionCommand("out2DFile");
        out2DFile.setMnemonic(KeyEvent.VK_I);
        out2DFile.setToolTipText("send 2D embeddings into a file or pipe");
        JPanel out2DDestPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 5));
        out2DFileName.setColumns(15);
        out2DFileName.setMaximumSize(out2DFileName.getPreferredSize());
        out2DFileName.setToolTipText(FilePipeHint);
        out2DFileName.addActionListener(this);
        JPanel out2DFilePanel = new JPanel();
        out2DFilePanel.setLayout(new BoxLayout(out2DFilePanel, BoxLayout.X_AXIS));
        out2DFilePanel.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 10));
        JPanel out2DViewerPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 5));
        out2DNoDest.setVisible(false);
        out2DNoDest.addActionListener(out2DDestListener);
        out2DNoDest.setActionCommand("out2DNoDest");
        out2DNoDest.setText("None");
        JLabel out2DFileNameLabel = new JLabel("Filename");
        out2DFileNameLabel.setDisplayedMnemonic(KeyEvent.VK_M);
        out2DFileNameLabel.setLabelFor(out2DFileName);
        out2DFileNameLabel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        out2DFileNameLabel.setToolTipText(FilePipeHint);
        out2DViewer.addActionListener(out2DDestListener);
        out2DViewer.addActionListener(this);
        out2DViewer.setActionCommand("out2DViewer");
        out2DViewer.setText("Viewer");
        out2DViewer.setMnemonic(KeyEvent.VK_E);
        out2DViewer.setToolTipText("send 2D embeddings to some of the installed viewers");
        JLabel out2DFileFormatLabel = new JLabel("Format");
        out2DFileFormatLabel.setDisplayedMnemonic(KeyEvent.VK_R);
        out2DFileFormatLabel.setLabelFor(out2DFileFormat);
        out2DFileFormatLabel.setBorder(BorderFactory.createEmptyBorder(0, 20, 0, 10));
        out2DFileFormat.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        out2DFileFormat.setMaximumSize(out2DFileFormat.getPreferredSize());
        out2DFilePanel.add(out2DFileNameLabel, null);
        out2DFilePanel.add(out2DFileName, null);
        out2DFilePanel.add(Box.createHorizontalGlue());
        out2DFilePanel.add(out2DFileFormatLabel, null);
        out2DFilePanel.add(out2DFileFormat, null);
        out2DCheckBox.setText("2D representation");
        out2DCheckBox.setMnemonic(KeyEvent.VK_2);
        out2DCheckBox.setToolTipText(ShortcutHint);
        out2DCheckBox.addActionListener(this);

        if (addViewers("2D", viewers2D, out2DViewerGroup, out2DViewerPanel) > 0) {
            out2DDestGroup.add(out2DViewer);
            out2DDestPanel.add(out2DViewer, null);
            new OnActionClicker(out2DViewer, out2DNoDest, out2DCheckBox);
        } else {
            new OnActionClicker(out2DFile, out2DNoDest, out2DCheckBox);
        }
        out2DDestGroup.add(out2DFile);
        out2DDestGroup.add(out2DNoDest);

        out2DDestPanel.add(out2DFile, null);
        out2DDestPanel.add(out2DNoDest, null);
        out2DDestOptionsPanel.add(new JPanel(), "out2DNoDest");
        out2DDestOptionsPanel.add(out2DViewerPanel, "out2DViewer");
        out2DDestOptionsPanel.add(out2DFilePanel, "out2DFile");
        new JTextComponentFocusSelector(out2DFileName);
        this.add(out2DCheckBox, new GridBagConstraints(0, 7, 1, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 10), 0, 0));
        this.add(out2DDestPanel, new GridBagConstraints(1, 7, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(out2DDestOptionsPanel, new GridBagConstraints(1, 8, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));

        this.add(new JSeparator(SwingConstants.HORIZONTAL), new GridBagConstraints(0, 9, GridBagConstraints.REMAINDER, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(20, 0, 20, 10), 0, 0));

        JPanel outAdjDestOptionsPanel = new JPanel(new CardLayout());
        OnActionClickerLayoutSwitcher outAdjDestListener =
                new OnActionClickerLayoutSwitcher(outAdjCheckBox, outAdjDestOptionsPanel);
        outAdjCheckBox.setText("Adjacency information");
        outAdjCheckBox.setMnemonic(KeyEvent.VK_A);
        outAdjCheckBox.setToolTipText("send connection table into a file or pipe");
        outAdjCheckBox.addActionListener(this);
        JPanel outAdjFilePanel = new JPanel();
        outAdjFilePanel.setLayout(new BoxLayout(outAdjFilePanel, BoxLayout.X_AXIS));
        outAdjFilePanel.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 10));
        JLabel outAdjFileNameLabel = new JLabel();
        outAdjFileNameLabel.setDisplayedMnemonic(0);
        outAdjNoDest.setVisible(false);
        outAdjFile.setVisible(false);
        outAdjDestGroup.add(outAdjFile);
        outAdjDestGroup.add(outAdjNoDest);
        outAdjFileName.setColumns(15);
        outAdjFileName.setMaximumSize(outAdjFileName.getPreferredSize());
        outAdjFileName.setToolTipText(FilePipeHint);
        outAdjFileName.addActionListener(this);
        JLabel outAdjFileFormatLabel = new JLabel("Format");
        outAdjFileFormatLabel.setLabelFor(outAdjFileFormat);
        outAdjFileFormatLabel.setBorder(BorderFactory.createEmptyBorder(0, 20, 0, 10));
        outAdjFileFormat.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        outAdjFileFormat.setMaximumSize(outAdjFileFormat.getPreferredSize());
        outAdjDestOptionsPanel.setLayout(new CardLayout());
        outAdjFile.addActionListener(outAdjDestListener);
        outAdjFile.setActionCommand("outAdjFile");
        outAdjFile.setText("File/Pipe");
        outAdjNoDest.addActionListener(outAdjDestListener);
        outAdjNoDest.setActionCommand("outAdjNoDest");
        outAdjNoDest.setText("None");
        outAdjFileNameLabel.setLabelFor(outAdjFileName);
        outAdjFileNameLabel.setText("Filename");
        outAdjFileNameLabel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        outAdjFileNameLabel.setToolTipText(FilePipeHint);
        outAdjDestOptionsPanel.add(outAdjFile, "");
        outAdjDestOptionsPanel.add(outAdjNoDest, "");
        outAdjDestOptionsPanel.add(new JPanel(), "outAdjNoDest");
        outAdjDestOptionsPanel.add(outAdjFilePanel, "outAdjFile");
        outAdjFilePanel.add(outAdjFileNameLabel, null);
        outAdjFilePanel.add(outAdjFileName, null);
        outAdjFilePanel.add(Box.createHorizontalGlue());
        outAdjFilePanel.add(outAdjFileFormatLabel, null);
        outAdjFilePanel.add(outAdjFileFormat, null);
        new OnActionClicker(outAdjFile, outAdjNoDest, outAdjCheckBox);
        new JTextComponentFocusSelector(outAdjFileName);
        this.add(outAdjCheckBox, new GridBagConstraints(0, 10, 1, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 10), 0, 0));
        this.add(outAdjDestOptionsPanel, new GridBagConstraints(1, 10, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));

    }

    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        this.generatorInfo = generatorInfo;
        setGeneratorInfoChanged();
        Embedder embedder = generatorInfo.getEmbedder();
        boolean generalExpertMode = CaGe.expertMode;
        boolean showExpertControls = generatorInfo.expertModeContains(GeneratorInfo.GENERATOR_EXPERT | GeneratorInfo.EMBED_EXPERT, generalExpertMode);
        boolean showGeneratorControls = generatorInfo.expertModeContains(GeneratorInfo.GENERATOR_EXPERT, generalExpertMode);
        boolean showEmbedControls = generatorInfo.expertModeContains(GeneratorInfo.EMBED_EXPERT, generalExpertMode);
        String expertLabelText = "";
        if (showGeneratorControls) {
            generatorCmdLine.setText(Systoolbox.makeCmdLine(generatorInfo.getGenerator()));
            expertLabelText += "generator";
        }
        generatorControlsGroup.setVisible(showGeneratorControls);
        if (showEmbedControls) {
            if (embed2DCmdLine.getText().length() == 0 || !embedder.isConstant()) {
                embed2DCmdLine.setText(Systoolbox.makeCmdLine(embedder.getEmbed2DNew()));
                embed3DCmdLine.setText(Systoolbox.makeCmdLine(embedder.getEmbed3DNew()));
            }
            expertLabelText += (expertLabelText.length() > 0 ? "/" : "") + "embedders";
        }
        embedControlsGroup.setVisible(showEmbedControls);
        expertControlsGroup.setVisible(showExpertControls);
        expertLabel.setText(expertLabelText);
        if (showExpertControls) {
            generatorCmdLine.setToolTipText("generator command");
            String toolTipText = "embed command";
            switch (embedder.getMode()) {
                case Embedder.IGNORE_OLD_EMBEDDING:
                    toolTipText += " for all graphs. Any generated coordinates will be overwritten.";
                    break;
                case Embedder.KEEP_OLD_EMBEDDING:
                    toolTipText += " for graphs without coordinates. Old coordinates will be left unchanged.";
                    break;
                case Embedder.REFINE_OLD_EMBEDDING:
                    toolTipText += " for graphs without coordinates. Old coordinates will be used as start values with an embed command derived from this one.";
                    break;
            }
            embed2DCmdLine.setToolTipText("2D " + toolTipText);
            embed3DCmdLine.setToolTipText("3D " + toolTipText);
        }
        String filename = generatorInfo.getFilename();
        out2DFileName.setText(filename);
        out2DFileFormat.addExtension();
        out3DFileName.setText(filename);
        out3DFileFormat.addExtension();
        outAdjFileName.setText(filename);
        outAdjFileFormat.addExtension();
    }

    public GeneratorInfo getGeneratorInfo() {
        if (generatorInfoChanged) {
            Embedder embedder = generatorInfo.getEmbedder();
            if (generatorCmdLine.isVisible()) {
                String cmdLineGenerator = generatorCmdLine.getText();
                generatorInfo.setGenerator(Systoolbox.parseCmdLine(cmdLineGenerator));
            }
            if (embed2DCmdLine.isVisible()) {
                String cmdLine2D = embed2DCmdLine.getText();
                embedder.setEmbed2D(Systoolbox.parseCmdLine(cmdLine2D));
            }
            if (embed3DCmdLine.isVisible()) {
                String cmdLine3D = embed3DCmdLine.getText();
                embedder.setEmbed3D(Systoolbox.parseCmdLine(cmdLine3D));
            }
            generatorInfo.setEmbedder(embedder);
            generatorInfoChanged = false;
        }
        return generatorInfo;
    }

    public void showing() {
        defaultButton = SwingUtilities.getRootPane(this).getDefaultButton();
        checkOutputOptions();
    }

    public void actionPerformed(ActionEvent e) {
        if (e.getSource() instanceof javax.swing.text.JTextComponent) {
            if (defaultButton != null) {
                defaultButton.doClick();
            }
        } else {
            checkOutputOptions();
        }
    }

    public void insertUpdate(DocumentEvent e) {
        setGeneratorInfoChanged();
    }

    public void removeUpdate(DocumentEvent e) {
        setGeneratorInfoChanged();
    }

    public void changedUpdate(DocumentEvent e) {
        setGeneratorInfoChanged();
    }

    public void setGeneratorInfoChanged() {
        generatorInfoChanged = true;
    }

    void checkOutputOptions() {
        boolean someViewer, someFile;
        someViewer = out2DViewer.isSelected() || out3DViewer.isSelected();
        someFile =
                outAdjFile.isSelected() ||
                out2DFile.isSelected() ||
                out3DFile.isSelected();
        defaultButton.setEnabled(someViewer ^ someFile);
        defaultButton.setToolTipText(
                defaultButton.isEnabled() ? "start generation process (Return)" : someFile | someViewer ? "don't mix viewer and file output" : "choose some output options and press Return");
    }

    Vector createViewerNames(String dimension, Vector viewersDim) {
        if (viewersDim == null) {
            viewersDim = Systoolbox.stringToVector(
                    CaGe.config.getProperty(generatorName + ".Viewers." + dimension));
        }
        return viewersDim;
    }

    int addViewers(String dimName, Vector viewersDim,
            GenericButtonGroup buttonGroup, JComponent component) {
        int dimension = dimName.charAt(0) - '0';
        viewersDim = createViewerNames(dimName, viewersDim);
        viewersXD = createViewerNames("xD", viewersXD);
        Vector[] vector = new Vector[]{viewersDim, viewersXD};
        SyncButtonGroup viewersXDGroup = new SyncButtonGroup();
        int n = 0;
        for (int i = 0; i < vector.length; ++i) {
            Enumeration viewerNames = vector[i].elements();
            while (viewerNames.hasMoreElements()) {
                String viewerName = (String) viewerNames.nextElement();
                if (!ViewerFactory.checkAvailability(viewerName, dimension)) {
                    continue;
                }
                AbstractButton viewerButton = new JCheckBox(
                        CaGe.config.getProperty(viewerName + ".Title"), n++ == 0);
                viewerButton.setActionCommand(viewerName);
                buttonGroup.add(viewerButton);
                component.add(viewerButton);
                if (i == 1) {
                    viewersXDGroup.add(viewerButton);
                }
            }
        }
        return n;
    }

    public String[][] getPreFilter() {
        if (outPreFilter.isSelected()) {
            return Systoolbox.parseCmdLine(outPreFilterCommand.getText());
        } else {
            return null;
        }
    }

    public boolean requests2D() {
        return out2DCheckBox.isSelected();
    }

    public boolean requests3D() {
        return out3DCheckBox.isSelected();
    }

    public Vector getViewers() {
        ButtonModel dest;
        Vector viewers = new Vector();
        viewerErrors = new StringBuffer();
        dest = out2DDestGroup.getSelection();
        if (dest == out2DViewer.getModel()) {
            addSelectedViewers(viewers, out2DViewerGroup, 2);
        }
        dest = out3DDestGroup.getSelection();
        if (dest == out3DViewer.getModel()) {
            addSelectedViewers(viewers, out3DViewerGroup, 3);
        }
        return viewers;
    }

    void addSelectedViewers(Vector viewers, GenericButtonGroup buttonGroup, int dimension) {
        Enumeration buttons = buttonGroup.getElements();
        while (buttons.hasMoreElements()) {
            AbstractButton button = (AbstractButton) buttons.nextElement();
            String viewerName = button.getActionCommand();
            if (button.isSelected()) {
                CaGeViewer viewer = ViewerFactory.getCaGeViewer(
                        button.getActionCommand(), dimension);
                if (viewer == null) {
                    viewerErrors.append(viewerName);
                    viewerErrors.append(": ");
                    viewerErrors.append(ViewerFactory.lastErrorMessage());
                    viewerErrors.append("\n");
                    continue;
                }
                if (!viewers.contains(viewer)) {
                    viewer.setGeneratorInfo(getGeneratorInfo());
                    viewers.addElement(viewer);
                }
            }
        }
    }

    public String getViewerErrors() {
        if (viewerErrors.length() == 0) {
            return null;
        } else {
            return viewerErrors.toString();
        }
    }

    public Vector getWriters() {
        ButtonModel dest;
        Vector writers = new Vector();
        dest = outAdjDestGroup.getSelection();
        if (dest == outAdjFile.getModel()) {
            addWriter(writers, outAdjFileFormat, 0);
        }
        dest = out2DDestGroup.getSelection();
        if (dest == out2DFile.getModel()) {
            addWriter(writers, out2DFileFormat, 2);
        }
        dest = out3DDestGroup.getSelection();
        if (dest == out3DFile.getModel()) {
            addWriter(writers, out3DFileFormat, 3);
        }
        return writers;
    }

    void addWriter(Vector writers, FileFormatBox format, int dimension) {
        CaGeWriter writer = format.getCaGeWriter();
        writer.setGeneratorInfo(getGeneratorInfo());
        writers.addElement(writer);
    }

    public Vector getWriteDestinations() {
        ButtonModel dest;
        Vector writeDests = new Vector();
        dest = outAdjDestGroup.getSelection();
        if (dest == outAdjFile.getModel()) {
            writeDests.addElement(outAdjFileName.getText());
        }
        dest = out2DDestGroup.getSelection();
        if (dest == out2DFile.getModel()) {
            writeDests.addElement(out2DFileName.getText());
        }
        dest = out3DDestGroup.getSelection();
        if (dest == out3DFile.getModel()) {
            writeDests.addElement(out3DFileName.getText());
        }
        return writeDests;
    }

    public static void main(String[] args) {
        try {
            //com.sun.java.swing.UIManager.setLookAndFeel(new com.sun.java.swing.plaf.windows.WindowsLookAndFeel());
            //com.sun.java.swing.UIManager.setLookAndFeel(new com.sun.java.swing.plaf.motif.MotifLookAndFeel());
            //com.sun.java.swing.UIManager.setLookAndFeel(new com.sun.java.swing.plaf.metal.MetalLookAndFeel());
            UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
        } catch (Exception e) {
        }
        JFrame f = new JFrame("Output Dialog");
        f.addWindowListener(new WindowAdapter() {

            public void windowClosing(WindowEvent e) {
                System.exit(0);
            }
        });
        UItoolbox.addExitOnEscape(f);
        f.setContentPane(new OutputPanel());
        f.pack();
        f.setVisible(true);
//    f.setResizable(false);
    }
}

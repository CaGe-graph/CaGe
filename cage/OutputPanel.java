package cage;

import cage.viewer.CaGeViewer;
import cage.viewer.ViewerFactory;
import cage.writer.CaGeWriter;

import java.awt.CardLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.InputEvent;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.Enumeration;
import java.util.Hashtable;
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
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedJLabel;
import lisken.uitoolbox.JTextComponentFocusSelector;
import lisken.uitoolbox.UItoolbox;

public class OutputPanel extends JPanel implements ActionListener, DocumentListener {

    GeneratorInfo generatorInfo;
    boolean generatorInfoChanged;
    String generatorName;
    Vector viewers2D, viewers3D, viewersXD;
    SyncButtonGroup viewersXDGroup = new SyncButtonGroup();
    StringBuffer viewerErrors;
    JButton defaultButton;
    GridBagLayout outputLayout = new GridBagLayout();
    JSeparator sep1 = new JSeparator(SwingConstants.HORIZONTAL);
    JSeparator sep2 = new JSeparator(SwingConstants.HORIZONTAL);
    JSeparator sep3 = new JSeparator(SwingConstants.HORIZONTAL);
    JSeparator sep4 = new JSeparator(SwingConstants.HORIZONTAL);
    JLabel expertLabel = new JLabel();
    JPanel expertPanel = new JPanel();
    GridBagLayout expertLayout = new GridBagLayout();
    JLabel generatorLabel = new JLabel();
    JTextField generatorCmdLine = new JTextField("");
    JLabel embed2DLabel = new JLabel();
    JTextField embed2DCmdLine = new JTextField("");
    JLabel embed3DLabel = new JLabel();
    JTextField embed3DCmdLine = new JTextField("");
    JCheckBox outPreFilterCheckBox = new JCheckBox();
    JTextField outPreFilterCommand = new JTextField();
    JPanel outPreFilterPanel = new JPanel();
    JPanel outPreFilterNonePanel = new JPanel();
    CardLayout outPreFilterLayout = new CardLayout();
    JRadioButton outPreFilterNone = new JRadioButton();
    JRadioButton outPreFilter = new JRadioButton();
    ButtonGroup outPreFilterGroup = new ButtonGroup();
    JPanel out3DDestPanel = new JPanel();
    FlowLayout out3DDestLayout = new FlowLayout();
    JCheckBox out3DCheckBox = new JCheckBox();
    JPanel out3DDestOptionsPanel = new JPanel();
    CardLayout out3DDestOptionsLayout = new CardLayout();
    ButtonGroup out3DDestGroup = new ButtonGroup();
    JRadioButton out3DViewer = new JRadioButton();
    JRadioButton out3DFile = new JRadioButton();
    JRadioButton out3DNoDest = new JRadioButton();
    JPanel out3DNoDestPanel = new JPanel();
    JPanel out3DViewerPanel = new JPanel();
    Min1ButtonGroup out3DViewerGroup = new Min1ButtonGroup("3D", false, out3DCheckBox);
    JPanel out3DFilePanel = new JPanel();
    JLabel out3DFileFormatLabel = new JLabel();
    EnhancedJLabel out3DFileNameLabel = new EnhancedJLabel();
    BoxLayout out3DFileLayout = new BoxLayout(out3DFilePanel, BoxLayout.X_AXIS);
    JTextField out3DFileName = new JTextField();
    FlowLayout out3DViewerLayout = new FlowLayout();
    FileFormatBox out3DFileFormat = new FileFormatBox("3D", out3DFileName);
    JCheckBox out2DCheckBox = new JCheckBox();
    ButtonGroup out2DDestGroup = new ButtonGroup();
    Min1ButtonGroup out2DViewerGroup = new Min1ButtonGroup("2D", false, out2DCheckBox);
    JPanel out2DNoDestPanel = new JPanel();
    JLabel out2DFileFormatLabel = new JLabel();
    JRadioButton out2DViewer = new JRadioButton();
    EnhancedJLabel out2DFileNameLabel = new EnhancedJLabel();
    JRadioButton out2DFile = new JRadioButton();
    JPanel out2DDestPanel = new JPanel();
    JTextField out2DFileName = new JTextField();
    FlowLayout out2DViewerLayout = new FlowLayout();
    JPanel out2DFilePanel = new JPanel();
    JPanel out2DViewerPanel = new JPanel();
    JPanel out2DDestOptionsPanel = new JPanel();
    CardLayout out2DDestOptionsLayout = new CardLayout();
    FlowLayout out2DDestLayout = new FlowLayout();
    BoxLayout out2DFileLayout = new BoxLayout(out2DFilePanel, BoxLayout.X_AXIS);
    JRadioButton out2DNoDest = new JRadioButton();
    FileFormatBox out2DFileFormat = new FileFormatBox("2D", out2DFileName);
    ButtonGroup outAdjDestGroup = new ButtonGroup();
    JCheckBox outAdjCheckBox = new JCheckBox();
    JPanel outAdjFilePanel = new JPanel();
    EnhancedJLabel outAdjFileNameLabel = new EnhancedJLabel();
    JRadioButton outAdjNoDest = new JRadioButton();
    JRadioButton outAdjFile = new JRadioButton();
    JPanel outAdjNoDestPanel = new JPanel();
    BoxLayout outAdjFileLayout = new BoxLayout(outAdjFilePanel, BoxLayout.X_AXIS);
    CardLayout outAdjDestOptionsLayout = new CardLayout();
    JTextField outAdjFileName = new JTextField();
    JPanel outAdjDestOptionsPanel = new JPanel();
    JLabel outAdjFileFormatLabel = new JLabel();
    FileFormatBox outAdjFileFormat = new FileFormatBox("Adjacency", outAdjFileName);

    public OutputPanel() {
        this(null);
    }

    public OutputPanel(String gn) {
        generatorName = gn;
        generatorInfoChanged = true;

        final String FilterHint = "pipe embedded graphs through an external filter command";
        final String ShortcutHint = "hint: just choose 'Viewer' or 'File/Pipe' on the right";
        final String FilePipeHint = "start with | to send into a pipe";

        this.setLayout(outputLayout);
        this.setBorder(BorderFactory.createTitledBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createCompoundBorder(
                BorderFactory.createEmptyBorder(10, 10, 10, 10),
                BorderFactory.createEtchedBorder()),
                BorderFactory.createEmptyBorder(20, 20, 20, 20)), " Output Options "));

        expertLabel.setText("generator/embedders");
        expertLabel.setForeground(Color.black);
        Font font = expertLabel.getFont();
        font = new Font(
                font.getName(),
                font.getStyle() & ~Font.BOLD,
                font.getSize());
        generatorLabel.setText("generator");
        generatorLabel.setLabelFor(generatorCmdLine);
        generatorLabel.setDisplayedMnemonic(KeyEvent.VK_G);
        // generatorLabel.setFont(font);
        embed2DLabel.setText("2D embedder");
        // embed2DLabel.setFont(font);
        embed2DLabel.setLabelFor(embed2DCmdLine);
        embed2DLabel.setDisplayedMnemonic(KeyEvent.VK_M);
        embed3DLabel.setText("3D embedder");
        // embed3DLabel.setFont(font);
        embed3DLabel.setLabelFor(embed3DCmdLine);
        embed3DLabel.setDisplayedMnemonic(KeyEvent.VK_B);
        generatorCmdLine.setColumns(10);
        generatorCmdLine.setActionCommand("generator");
        generatorCmdLine.addActionListener(this);
        generatorCmdLine.getDocument().addDocumentListener(this);
        new JTextComponentFocusSelector(generatorCmdLine);
        embed2DCmdLine.setColumns(10);
        embed2DCmdLine.setActionCommand("e2");
        embed2DCmdLine.addActionListener(this);
        embed2DCmdLine.getDocument().addDocumentListener(this);
        new JTextComponentFocusSelector(embed2DCmdLine);
        embed3DCmdLine.setColumns(10);
        embed3DCmdLine.setActionCommand("e3");
        embed3DCmdLine.addActionListener(this);
        embed3DCmdLine.getDocument().addDocumentListener(this);
        new JTextComponentFocusSelector(embed3DCmdLine);
        expertPanel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        expertPanel.setLayout(expertLayout);
        expertPanel.add(generatorLabel, new GridBagConstraints(0, 0, 1, 1, 0.01, 1.0, GridBagConstraints.WEST, GridBagConstraints.VERTICAL, new Insets(3, 0, 3, 5), 0, 0));
        expertPanel.add(generatorCmdLine, new GridBagConstraints(1, 0, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(3, 0, 3, 0), 0, 0));
        expertPanel.add(embed2DLabel, new GridBagConstraints(0, 1, 1, 1, 0.01, 1.0, GridBagConstraints.WEST, GridBagConstraints.VERTICAL, new Insets(3, 0, 3, 5), 0, 0));
        expertPanel.add(embed2DCmdLine, new GridBagConstraints(1, 1, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(3, 0, 3, 0), 0, 0));
        expertPanel.add(embed3DLabel, new GridBagConstraints(0, 2, 1, 1, 0.01, 1.0, GridBagConstraints.WEST, GridBagConstraints.VERTICAL, new Insets(3, 0, 3, 5), 0, 0));
        expertPanel.add(embed3DCmdLine, new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(3, 0, 3, 0), 0, 0));
        this.add(expertLabel, new GridBagConstraints(0, 0, 1, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(0, 17, 0, 10), 0, 0));
        this.add(expertPanel, new GridBagConstraints(1, 0, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(sep1, new GridBagConstraints(0, 1, GridBagConstraints.REMAINDER, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(20, 0, 20, 10), 0, 0));

        onActionClickerLayoutSwitcher outPreFilterListener =
                new onActionClickerLayoutSwitcher(outPreFilterCheckBox, outPreFilterPanel);
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
        outPreFilterPanel.setLayout(outPreFilterLayout);
        outPreFilterPanel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        outPreFilterPanel.add(outPreFilter, "");
        outPreFilterPanel.add(outPreFilterNone, "");
        outPreFilterPanel.add(outPreFilterCommand, "outPreFilter");
        outPreFilterPanel.add(outPreFilterNonePanel, "outPreFilterNone");
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
        new onActionClicker(outPreFilter, outPreFilterNone, outPreFilterCheckBox);
        this.add(outPreFilterCheckBox, new GridBagConstraints(0, 2, 1, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 10), 0, 0));
        this.add(outPreFilterPanel, new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));

        this.add(sep2, new GridBagConstraints(0, 3, GridBagConstraints.REMAINDER, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(20, 0, 20, 10), 0, 0));

        onActionClickerLayoutSwitcher out3DDestListener =
                new onActionClickerLayoutSwitcher(out3DCheckBox, out3DDestOptionsPanel);
        out3DDestLayout.setAlignment(0);
        out3DDestLayout.setHgap(10);
        out3DDestPanel.setLayout(out3DDestLayout);
        out3DCheckBox.setText("3D representation");
        out3DCheckBox.setMnemonic(KeyEvent.VK_3);
        out3DCheckBox.setToolTipText(ShortcutHint);
        out3DCheckBox.addActionListener(this);
        out3DDestOptionsPanel.setLayout(out3DDestOptionsLayout);
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
        out3DFilePanel.setLayout(out3DFileLayout);
        out3DFilePanel.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 10));
        out3DFilePanel.add(out3DFileNameLabel, null);
        out3DFilePanel.add(out3DFileName, null);
        out3DFilePanel.add(Box.createHorizontalGlue());
        out3DFilePanel.add(out3DFileFormatLabel, null);
        out3DFilePanel.add(out3DFileFormat, null);
        out3DFileFormatLabel.setLabelFor(out3DFileFormat);
        out3DFileFormatLabel.setDisplayedMnemonic(KeyEvent.VK_O);
        out3DFileFormatLabel.setText("Format");
        out3DFileFormatLabel.setBorder(BorderFactory.createEmptyBorder(0, 20, 0, 10));
        out3DFileFormat.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        out3DFileFormat.setMaximumSize(out3DFileFormat.getPreferredSize());
        out3DFileNameLabel.setText("Filename");
        out3DFileNameLabel.setLabelFor(out3DFileName);
        out3DFileNameLabel.setDisplayedMnemonic(KeyEvent.VK_N);
        out3DFileNameLabel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        out3DFileNameLabel.setToolTipText(FilePipeHint);
        out3DFileName.setColumns(15);
        out3DFileName.setMaximumSize(out3DFileName.getPreferredSize());
        out3DFileName.setToolTipText(FilePipeHint);
        out3DFileName.addActionListener(this);
        out3DViewerPanel.setLayout(out3DViewerLayout);
        out3DViewerLayout.setHgap(10);
        out3DViewerLayout.setAlignment(0);

        if (addViewers("3D", viewers3D, out3DViewerGroup, out3DViewerPanel) > 0) {
            out3DDestGroup.add(out3DViewer);
            out3DDestPanel.add(out3DViewer, null);
            new onActionClicker(out3DViewer, out3DNoDest, out3DCheckBox);
        } else {
            new onActionClicker(out3DFile, out3DNoDest, out3DCheckBox);
        }
        out3DDestGroup.add(out3DFile);
        out3DDestGroup.add(out3DNoDest);

        out3DDestOptionsPanel.add(out3DNoDestPanel, "out3DNoDest");
        out3DDestOptionsPanel.add(out3DViewerPanel, "out3DViewer");
        out3DDestOptionsPanel.add(out3DFilePanel, "out3DFile");
        out3DDestPanel.add(out3DFile, null);
        out3DDestPanel.add(out3DNoDest, null);
        new JTextComponentFocusSelector(out3DFileName);
        this.add(out3DCheckBox, new GridBagConstraints(0, 4, 1, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 10), 0, 0));
        this.add(out3DDestPanel, new GridBagConstraints(1, 4, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(out3DDestOptionsPanel, new GridBagConstraints(1, 5, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));

        this.add(sep3, new GridBagConstraints(0, 6, GridBagConstraints.REMAINDER, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(20, 0, 20, 10), 0, 0));

        onActionClickerLayoutSwitcher out2DDestListener =
                new onActionClickerLayoutSwitcher(out2DCheckBox, out2DDestOptionsPanel);
        out2DFile.addActionListener(out2DDestListener);
        out2DFile.addActionListener(this);
        out2DFile.setText("File/Pipe");
        out2DFile.setActionCommand("out2DFile");
        out2DFile.setMnemonic(KeyEvent.VK_I);
        out2DFile.setToolTipText("send 2D embeddings into a file or pipe");
        out2DDestPanel.setLayout(out2DDestLayout);
        out2DFileName.setColumns(15);
        out2DFileName.setMaximumSize(out2DFileName.getPreferredSize());
        out2DFileName.setToolTipText(FilePipeHint);
        out2DFileName.addActionListener(this);
        out2DViewerLayout.setAlignment(0);
        out2DViewerLayout.setHgap(10);
        out2DFilePanel.setLayout(out2DFileLayout);
        out2DFilePanel.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 10));
        out2DViewerPanel.setLayout(out2DViewerLayout);
        out2DDestOptionsPanel.setLayout(out2DDestOptionsLayout);
        out2DDestLayout.setHgap(10);
        out2DNoDest.setVisible(false);
        out2DNoDest.addActionListener(out2DDestListener);
        out2DNoDest.setActionCommand("out2DNoDest");
        out2DNoDest.setText("None");
        out2DDestLayout.setAlignment(0);
        out2DFileNameLabel.setDisplayedMnemonic(KeyEvent.VK_M);
        out2DFileNameLabel.setLabelFor(out2DFileName);
        out2DFileNameLabel.setText("Filename");
        out2DFileNameLabel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        out2DFileNameLabel.setToolTipText(FilePipeHint);
        out2DViewer.addActionListener(out2DDestListener);
        out2DViewer.addActionListener(this);
        out2DViewer.setActionCommand("out2DViewer");
        out2DViewer.setText("Viewer");
        out2DViewer.setMnemonic(KeyEvent.VK_E);
        out2DViewer.setToolTipText("send 2D embeddings to some of the installed viewers");
        out2DFileFormatLabel.setDisplayedMnemonic(KeyEvent.VK_R);
        out2DFileFormatLabel.setLabelFor(out2DFileFormat);
        out2DFileFormatLabel.setText("Format");
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
            new onActionClicker(out2DViewer, out2DNoDest, out2DCheckBox);
        } else {
            new onActionClicker(out2DFile, out2DNoDest, out2DCheckBox);
        }
        out2DDestGroup.add(out2DFile);
        out2DDestGroup.add(out2DNoDest);

        out2DDestPanel.add(out2DFile, null);
        out2DDestPanel.add(out2DNoDest, null);
        out2DDestOptionsPanel.add(out2DNoDestPanel, "out2DNoDest");
        out2DDestOptionsPanel.add(out2DViewerPanel, "out2DViewer");
        out2DDestOptionsPanel.add(out2DFilePanel, "out2DFile");
        new JTextComponentFocusSelector(out2DFileName);
        this.add(out2DCheckBox, new GridBagConstraints(0, 7, 1, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 10), 0, 0));
        this.add(out2DDestPanel, new GridBagConstraints(1, 7, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(out2DDestOptionsPanel, new GridBagConstraints(1, 8, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));

        this.add(sep4, new GridBagConstraints(0, 9, GridBagConstraints.REMAINDER, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(20, 0, 20, 10), 0, 0));

        onActionClickerLayoutSwitcher outAdjDestListener =
                new onActionClickerLayoutSwitcher(outAdjCheckBox, outAdjDestOptionsPanel);
        outAdjCheckBox.setText("Adjacency information");
        outAdjCheckBox.setMnemonic(KeyEvent.VK_A);
        outAdjCheckBox.setToolTipText("send connection table into a file or pipe");
        outAdjCheckBox.addActionListener(this);
        outAdjFilePanel.setLayout(outAdjFileLayout);
        outAdjFilePanel.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 10));
        outAdjFileNameLabel.setDisplayedMnemonic(0);
        outAdjNoDest.setVisible(false);
        outAdjFile.setVisible(false);
        outAdjDestGroup.add(outAdjFile);
        outAdjDestGroup.add(outAdjNoDest);
        outAdjFileName.setColumns(15);
        outAdjFileName.setMaximumSize(outAdjFileName.getPreferredSize());
        outAdjFileName.setToolTipText(FilePipeHint);
        outAdjFileName.addActionListener(this);
        outAdjFileFormatLabel.setLabelFor(outAdjFileFormat);
        outAdjFileFormatLabel.setText("Format");
        outAdjFileFormatLabel.setBorder(BorderFactory.createEmptyBorder(0, 20, 0, 10));
        outAdjFileFormat.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        outAdjFileFormat.setMaximumSize(outAdjFileFormat.getPreferredSize());
        outAdjDestOptionsPanel.setLayout(outAdjDestOptionsLayout);
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
        outAdjDestOptionsPanel.add(outAdjNoDestPanel, "outAdjNoDest");
        outAdjDestOptionsPanel.add(outAdjFilePanel, "outAdjFile");
        outAdjFilePanel.add(outAdjFileNameLabel, null);
        outAdjFilePanel.add(outAdjFileName, null);
        outAdjFilePanel.add(Box.createHorizontalGlue());
        outAdjFilePanel.add(outAdjFileFormatLabel, null);
        outAdjFilePanel.add(outAdjFileFormat, null);
        new onActionClicker(outAdjFile, outAdjNoDest, outAdjCheckBox);
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
        generatorLabel.setVisible(showGeneratorControls);
        generatorCmdLine.setVisible(showGeneratorControls);
        if (showEmbedControls) {
            if (embed2DCmdLine.getText().length() == 0 || !embedder.isConstant()) {
                embed2DCmdLine.setText(Systoolbox.makeCmdLine(embedder.getEmbed2DNew()));
                embed3DCmdLine.setText(Systoolbox.makeCmdLine(embedder.getEmbed3DNew()));
            }
            expertLabelText += (expertLabelText.length() > 0 ? "/" : "") + "embedders";
        }
        embed2DLabel.setVisible(showEmbedControls);
        embed3DLabel.setVisible(showEmbedControls);
        embed2DCmdLine.setVisible(showEmbedControls);
        embed3DCmdLine.setVisible(showEmbedControls);
        expertLabel.setVisible(showExpertControls);
        expertLabel.setText(expertLabelText);
        expertPanel.setVisible(showExpertControls);
        sep1.setVisible(showExpertControls);
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

class onActionClickerLayoutSwitcher implements ActionListener {

    AbstractButton button;
    Container container;

    public onActionClickerLayoutSwitcher(AbstractButton b, Container c) {
        button = b;
        container = c;
    }

    public onActionClickerLayoutSwitcher(AbstractButton b, Container c, AbstractButton target) {
        this(b, c);
        target.addActionListener(this);
    }

    public void actionPerformed(ActionEvent e) {
        Component c = (Component) e.getSource();
        ((CardLayout) container.getLayout()).show(container, e.getActionCommand());
        if (c.isVisible()) {
            button.setSelected(true);
            c.getParent().transferFocus();
        }
    }
}

class onActionClicker implements ActionListener {

    AbstractButton buttonTrue, buttonFalse;

    public onActionClicker(AbstractButton bt, AbstractButton bf, AbstractButton target) {
        buttonTrue = bt;
        buttonFalse = bf;
        target.addActionListener(this);
    }

    public onActionClicker(AbstractButton bf, AbstractButton target) {
        buttonTrue = null;
        buttonFalse = bf;
        target.addActionListener(this);
    }

    public void actionPerformed(ActionEvent e) {
        AbstractButton source = (AbstractButton) e.getSource();
        if (source.isSelected()) {
            if (buttonTrue != null) {
                buttonTrue.doClick();
                if (!buttonTrue.isVisible()) {
                    source.transferFocus();
                }
            }
        } else {
            if (buttonFalse != null) {
                buttonFalse.doClick();
            }
        }
    }
}

interface GenericButtonGroup {

    void add(AbstractButton button);

    void remove(AbstractButton button);

    Enumeration getElements();
}

abstract class AbstractButtonGroup implements GenericButtonGroup, ItemListener {

    Vector buttons = new Vector();

    public void add(AbstractButton button) {
        buttons.addElement(button);
        button.addItemListener(this);
    }

    public void remove(AbstractButton button) {
        button.getModel().removeItemListener(this);
        buttons.removeElement(button);
    }

    public Enumeration getElements() {
        return buttons.elements();
    }

    public AbstractButton getSelection() {
        return null;
    }
}

class Min1ButtonGroup extends AbstractButtonGroup
        implements ChangeListener, KeyListener, MouseListener {

    String id;
    boolean active;
    AbstractButton deactivateButton;
    Hashtable selections;
    int lastModifiers;

    public Min1ButtonGroup() {
        this("", true, null);
    }

    public Min1ButtonGroup(String id) {
        this(id, true, null);
    }

    public Min1ButtonGroup(String id, boolean active) {
        this(id, active, null);
    }

    public Min1ButtonGroup(String id, boolean active, AbstractButton deactivateButton) {
        this.id = id;
        this.active = active;
        this.deactivateButton = deactivateButton;
        deactivateButton.addChangeListener(this);
        selections = new Hashtable();
    }

    public void add(AbstractButton button) {
        super.add(button);
        button.addKeyListener(this);
        button.addMouseListener(this);
        if (button.isSelected()) {
            selections.put(button, this);
        }
    }

    public void remove(AbstractButton button) {
        super.remove(button);
        button.removeKeyListener(this);
        button.removeMouseListener(this);
        if (button.isSelected()) {
            selections.remove(button);
        }
    }

    public void itemStateChanged(ItemEvent e) {
        AbstractButton button = (AbstractButton) e.getSource();
        if (button.isSelected()) {
            selections.put(button, this);
        } else {
            selections.remove(button);
        }
        int lastLowLevelModifiers = lastModifiers;
        lastModifiers = 0;
        if (active && selections.size() == 0) {
            if (deactivateButton != null && !button.hasFocus()) {
                deactivateButton.doClick();
                if (!active) {
                    return;
                }
            }
            button.setSelected(true);
        }
        if (lastLowLevelModifiers == InputEvent.SHIFT_MASK) {
            button.setSelected(true);
            Enumeration elements = getElements();
            while (elements.hasMoreElements()) {
                AbstractButton otherButton = (AbstractButton) elements.nextElement();
                if (otherButton != button && otherButton.isSelected()) {
                    otherButton.setSelected(false);
                }
            }
        }
    }

    public void setActive(boolean active) {
        if (this.active == active) {
            return;
        }
        this.active = active;
        if (active && selections.size() == 0 && buttons.size() > 0) {
            ((AbstractButton) buttons.elementAt(0)).setSelected(true);
        }
    }

    public boolean isActive() {
        return active;
    }

    public void stateChanged(ChangeEvent e) {
        AbstractButton source = (AbstractButton) e.getSource();
        if (source == deactivateButton) {
            setActive(source.isSelected());
        }
    }

    public void keyTyped(KeyEvent e) {
        if (e.getKeyChar() == ' ' && lastModifiers == InputEvent.SHIFT_MASK) {
            ((AbstractButton) e.getSource()).doClick();
        }
    }

    public void keyPressed(KeyEvent e) {
        lastModifiers = e.getModifiers();
    }

    public void keyReleased(KeyEvent e) {
    }

    public void mouseClicked(MouseEvent e) {
    }

    public void mousePressed(MouseEvent e) {
        lastModifiers = e.getModifiers() &
                ~(InputEvent.BUTTON1_MASK |
                InputEvent.BUTTON2_MASK |
                InputEvent.BUTTON3_MASK);
    }

    public void mouseReleased(MouseEvent e) {
    }

    public void mouseEntered(MouseEvent e) {
    }

    public void mouseExited(MouseEvent e) {
    }
}

class SyncButtonGroup extends AbstractButtonGroup {

    public void add(AbstractButton button) {
        if (buttons.size() > 0) {
            boolean selected = button.isSelected();
            AbstractButton otherButton = (AbstractButton) buttons.elementAt(0);
            if (otherButton.isSelected() != selected) {
                otherButton.setSelected(selected);
                button.setSelected(otherButton.isSelected());
            }
        }
        super.add(button);
    }

    public void itemStateChanged(ItemEvent e) {
        int change = e.getStateChange();
        if (change != ItemEvent.SELECTED && change != ItemEvent.DESELECTED) {
            return;
        }
        AbstractButton button = (AbstractButton) e.getSource();
        boolean selected = button.isSelected();
        Enumeration btns = buttons.elements();
        while (btns.hasMoreElements()) {
            AbstractButton otherButton = (AbstractButton) btns.nextElement();
            if (otherButton.isSelected() == selected) {
                continue;
            }
            otherButton.setSelected(selected);
            // Change the selection of just one button (that needs to be changed).
            // That button will create its own item event,
            // eventually triggering all necessary changes.
            break;
        }
    }
}


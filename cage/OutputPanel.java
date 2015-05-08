package cage;

import cage.utility.ComponentLogicalGroup;
import cage.utility.GenericButtonGroup;
import cage.utility.Min1ButtonGroup;
import cage.utility.OnActionClicker;
import cage.utility.OnActionClickerLayoutSwitcher;
import cage.utility.SingleActionDocumentLister;
import cage.utility.SyncButtonGroup;
import cage.viewer.CaGeViewer;
import cage.viewer.ViewerFactory;
import cage.viewer.twoview.BatchTwoViewConfigurationPanel;
import cage.viewer.twoview.BatchTwoViewModel;
import cage.writer.CaGeWriter;
import cage.writer.WriterConfigurationHandler;

import java.awt.CardLayout;
import java.awt.Color;
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.ListIterator;
import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
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

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.JTextComponentFocusSelector;
import lisken.uitoolbox.UItoolbox;

public class OutputPanel extends JPanel {

    private GeneratorInfo generatorInfo;
    private String generatorName;
    private List<String> viewersXD;
    private StringBuffer viewerErrors;
    private JButton defaultButton;
    private JLabel expertLabel = new JLabel();
    private JTextField generatorCmdLine = new JTextField("");
    private JTextField embed2DCmdLine = new JTextField("");
    private JTextField embed3DCmdLine = new JTextField("");

    //button group that makes sure that text viewers are always all in the same selection state.
    private SyncButtonGroup viewersXDGroup = new SyncButtonGroup();

    private JCheckBox outPreFilterCheckBox = new JCheckBox();
    private JTextField outPreFilterCommand = new JTextField();

    private JCheckBox out3DCheckBox = new JCheckBox();
    private ButtonGroup out3DDestGroup = new ButtonGroup();
    private JRadioButton out3DViewer = new JRadioButton();
    private JRadioButton out3DFile = new JRadioButton();
    private JRadioButton out3DPipe = new JRadioButton();
    private Min1ButtonGroup out3DViewerGroup = new Min1ButtonGroup("3D", false, out3DCheckBox);
    private TargetPanel out3DFilePanel = TargetPanel.creatFilePanel("3D", KeyEvent.VK_N, KeyEvent.VK_O);
    private TargetPanel out3DPipePanel = TargetPanel.creatPipePanel("3D", KeyEvent.VK_N, KeyEvent.VK_O);
    
    private JCheckBox out2DCheckBox = new JCheckBox();
    private ButtonGroup out2DDestGroup = new ButtonGroup();
    private JRadioButton out2DViewer = new JRadioButton();
    private JRadioButton out2DFile = new JRadioButton();
    private JRadioButton out2DPipe = new JRadioButton();
    private JRadioButton out2DBatch = new JRadioButton();
    private Min1ButtonGroup out2DViewerGroup = new Min1ButtonGroup("2D", false, out2DCheckBox);
    private TargetPanel out2DFilePanel = TargetPanel.creatFilePanel("2D", KeyEvent.VK_M, KeyEvent.VK_R);
    private TargetPanel out2DPipePanel = TargetPanel.creatPipePanel("2D", KeyEvent.VK_M, KeyEvent.VK_R);
    
    private ButtonGroup outAdjDestGroup = new ButtonGroup();
    private JRadioButton outAdjFile = new JRadioButton();
    private JRadioButton outAdjPipe = new JRadioButton();
    private TargetPanel outAdjFilePanel = TargetPanel.creatFilePanel("Adjacency", KeyEvent.VK_M, KeyEvent.VK_R);
    private TargetPanel outAdjPipePanel = TargetPanel.creatPipePanel("Adjacency", KeyEvent.VK_M, KeyEvent.VK_R);

    private ComponentLogicalGroup expertControlsGroup = new ComponentLogicalGroup();
    private ComponentLogicalGroup embedControlsGroup = new ComponentLogicalGroup();
    private ComponentLogicalGroup generatorControlsGroup = new ComponentLogicalGroup();
    
    private final BatchTwoViewModel batchTwoViewModel = new BatchTwoViewModel();
    
    private ActionListener uiActionListener = new ActionListener() {

        @Override
        public void actionPerformed(ActionEvent e) {
            if (defaultButton != null) {
                defaultButton.doClick();
            }
        }
    };
    
    private ActionListener outputOptionsListener = new ActionListener() {

        @Override
        public void actionPerformed(ActionEvent e) {
            checkOutputOptions();
        }
    };

    public OutputPanel() {
        this(null);
    }

    @SuppressWarnings("ResultOfObjectAllocationIgnored")
    public OutputPanel(String gn) {
        generatorName = gn;

        final String FilterHint = "pipe embedded graphs through an external filter command";
        final String ShortcutHint = "hint: just choose 'Viewer' or 'File/Pipe' on the right";
        final String FilePipeHint = "start with | to send into a pipe";

        this.setLayout(new GridBagLayout());
        this.setBorder(BorderFactory.createTitledBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createCompoundBorder(
                BorderFactory.createEmptyBorder(10, 10, 10, 10),
                BorderFactory.createEtchedBorder()),
                BorderFactory.createEmptyBorder(20, 20, 20, 20)), " Output Options "));

        //-----------expert section--------------
        expertLabel.setText("generator/embedders");
        expertLabel.setForeground(Color.black);
        expertControlsGroup.addComponent(expertLabel);
        JPanel expertPanel = new JPanel();
        expertPanel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        expertPanel.setLayout(new GridBagLayout());

        //expert section: generator
        JLabel generatorLabel = new JLabel();
        generatorLabel.setText("generator");
        generatorLabel.setLabelFor(generatorCmdLine);
        generatorLabel.setDisplayedMnemonic(KeyEvent.VK_G);
        generatorControlsGroup.addComponent(generatorLabel);
        expertPanel.add(generatorLabel, new GridBagConstraints(0, 0, 1, 1, 0.01, 1.0, GridBagConstraints.WEST, GridBagConstraints.VERTICAL, new Insets(3, 0, 3, 5), 0, 0));
        generatorCmdLine.setColumns(10);
        generatorCmdLine.setActionCommand("generator");
        generatorCmdLine.addActionListener(uiActionListener);
        generatorCmdLine.getDocument().addDocumentListener(new SingleActionDocumentLister() {

            @Override
            public void update() {
                if (generatorCmdLine.isVisible()) {
                    String cmdLineGenerator = generatorCmdLine.getText();
                    generatorInfo.setGenerator(Systoolbox.parseCmdLine(cmdLineGenerator));
                    fireGeneratorInfoChanged();
                }
            }
        });
        generatorControlsGroup.addComponent(generatorCmdLine);
        new JTextComponentFocusSelector(generatorCmdLine);
        expertPanel.add(generatorCmdLine, new GridBagConstraints(1, 0, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(3, 0, 3, 0), 0, 0));

        //expert section: embed 2D
        JLabel embed2DLabel = new JLabel("2D embedder");
        embed2DLabel.setLabelFor(embed2DCmdLine);
        embed2DLabel.setDisplayedMnemonic(KeyEvent.VK_M);
        embedControlsGroup.addComponent(embed2DLabel);
        expertPanel.add(embed2DLabel, new GridBagConstraints(0, 1, 1, 1, 0.01, 1.0, GridBagConstraints.WEST, GridBagConstraints.VERTICAL, new Insets(3, 0, 3, 5), 0, 0));
        embed2DCmdLine.setColumns(10);
        embed2DCmdLine.setActionCommand("e2");
        embed2DCmdLine.addActionListener(uiActionListener);
        embed2DCmdLine.getDocument().addDocumentListener(new SingleActionDocumentLister() {

            @Override
            public void update() {
                if (embed2DCmdLine.isVisible() &&
                        !embed2DCmdLine.getText().equals(
                                Systoolbox.makeCmdLine(
                                        generatorInfo.getEmbedder().getEmbed2DNew())) &&
                        embed2DCmdLine.getText().trim().length() > 0) {
                    String cmdLine2D = embed2DCmdLine.getText();
                    generatorInfo.getEmbedder().setEmbed2D(Systoolbox.parseCmdLine(cmdLine2D));
                    fireGeneratorInfoChanged();
                }
            }
        });
        new JTextComponentFocusSelector(embed2DCmdLine);
        embedControlsGroup.addComponent(embed2DCmdLine);
        expertPanel.add(embed2DCmdLine, new GridBagConstraints(1, 1, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(3, 0, 3, 0), 0, 0));

        //expert section: embed 3D
        JLabel embed3DLabel = new JLabel("3D embedder");
        embed3DLabel.setLabelFor(embed3DCmdLine);
        embed3DLabel.setDisplayedMnemonic(KeyEvent.VK_B);
        embedControlsGroup.addComponent(embed3DLabel);
        expertPanel.add(embed3DLabel, new GridBagConstraints(0, 2, 1, 1, 0.01, 1.0, GridBagConstraints.WEST, GridBagConstraints.VERTICAL, new Insets(3, 0, 3, 5), 0, 0));
        embed3DCmdLine.setColumns(10);
        embed3DCmdLine.setActionCommand("e3");
        embed3DCmdLine.addActionListener(uiActionListener);
        embed3DCmdLine.getDocument().addDocumentListener(new SingleActionDocumentLister() {

            @Override
            public void update() {
                if (embed3DCmdLine.isVisible() &&
                        !embed3DCmdLine.getText().equals(
                                Systoolbox.makeCmdLine(
                                        generatorInfo.getEmbedder().getEmbed3DNew())) &&
                        embed3DCmdLine.getText().trim().length() > 0) {
                    String cmdLine3D = embed3DCmdLine.getText();
                    generatorInfo.getEmbedder().setEmbed3D(Systoolbox.parseCmdLine(cmdLine3D));
                    fireGeneratorInfoChanged();
                }
            }
        });
        new JTextComponentFocusSelector(embed3DCmdLine);
        embedControlsGroup.addComponent(embed3DCmdLine);
        expertPanel.add(embed3DCmdLine, new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(3, 0, 3, 0), 0, 0));
        expertControlsGroup.addComponent(expertPanel);

        JSeparator expertControlsSeparator = new JSeparator(SwingConstants.HORIZONTAL);
        expertControlsGroup.addComponent(expertControlsSeparator);
        this.add(expertLabel, new GridBagConstraints(0, 0, 1, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(0, 17, 0, 10), 0, 0));
        this.add(expertPanel, new GridBagConstraints(1, 0, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(expertControlsSeparator, new GridBagConstraints(0, 1, GridBagConstraints.REMAINDER, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(20, 0, 20, 10), 0, 0));

        //-----------prefilter section--------------
        JPanel outPreFilterPanel = new JPanel();
        outPreFilterCheckBox.setText("Pre-filter graphs");
        outPreFilterCheckBox.setMnemonic(KeyEvent.VK_P);
        outPreFilterCheckBox.setToolTipText(FilterHint);
        outPreFilterCheckBox.setActionCommand("outPreFilterCheckBox");
        outPreFilterCheckBox.addActionListener(outputOptionsListener);
        outPreFilterCommand.setColumns(30);
        new JTextComponentFocusSelector(outPreFilterCommand);
        outPreFilterCommand.setToolTipText(FilterHint);
        outPreFilterCommand.getDocument().addDocumentListener(new SingleActionDocumentLister() {

            @Override
            public void update() {
                //verify check box
                if(outPreFilterCommand.getText().trim().length()==0)
                    outPreFilterCheckBox.setSelected(false);
                else
                    outPreFilterCheckBox.setSelected(true);
            }
        });
        outPreFilterPanel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        outPreFilterPanel.add(outPreFilterCommand, null);
        this.add(outPreFilterCheckBox, new GridBagConstraints(0, 2, 1, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 10), 0, 0));
        this.add(outPreFilterPanel, new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));

        this.add(new JSeparator(SwingConstants.HORIZONTAL), new GridBagConstraints(0, 3, GridBagConstraints.REMAINDER, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(20, 0, 20, 10), 0, 0));

        //-----------3D section--------------
        JPanel out3DDestOptionsPanel = new JPanel(new CardLayout());
        OnActionClickerLayoutSwitcher out3DDestListener =
                new OnActionClickerLayoutSwitcher(out3DCheckBox, out3DDestOptionsPanel);
        out3DCheckBox.setText("3D representation");
        out3DCheckBox.setMnemonic(KeyEvent.VK_3);
        out3DCheckBox.setToolTipText(ShortcutHint);
        out3DCheckBox.addActionListener(outputOptionsListener);
        out3DViewer.setText("Viewer");
        out3DViewer.setMnemonic(KeyEvent.VK_V);
        out3DViewer.setActionCommand("out3DViewer");
        out3DViewer.addActionListener(out3DDestListener);
        out3DViewer.addActionListener(outputOptionsListener);
        out3DViewer.setToolTipText("send 3D embeddings to some of the installed viewers");
        out3DFile.setText("File");
        out3DFile.setMnemonic(KeyEvent.VK_F);
        out3DFile.setActionCommand("out3DFile");
        out3DFile.addActionListener(out3DDestListener);
        out3DFile.addActionListener(outputOptionsListener);
        out3DFile.setToolTipText("send 3D embeddings into a file");
        out3DPipe.setText("Pipe");
        out3DPipe.setMnemonic(KeyEvent.VK_P);
        out3DPipe.setActionCommand("out3DPipe");
        out3DPipe.addActionListener(out3DDestListener);
        out3DPipe.addActionListener(outputOptionsListener);
        out3DPipe.setToolTipText("send 3D embeddings into a pipe");
        JRadioButton out3DNoDest = new JRadioButton("None");
        out3DNoDest.setActionCommand("out3DNoDest");
        out3DNoDest.addActionListener(out3DDestListener);
        out3DNoDest.setVisible(false);

        out3DFilePanel.addActionListener(uiActionListener);

        JPanel out3DViewerPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 5));

        JPanel out3DDestPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10 , 5));
        if (addViewers("3D", null, out3DViewerGroup, out3DViewerPanel) > 0) {
            out3DDestGroup.add(out3DViewer);
            out3DDestPanel.add(out3DViewer, null);
            new OnActionClicker(out3DViewer, out3DNoDest, out3DCheckBox);
        } else {
            new OnActionClicker(out3DFile, out3DNoDest, out3DCheckBox);
        }
        out3DDestGroup.add(out3DFile);
        out3DDestGroup.add(out3DPipe);
        out3DDestGroup.add(out3DNoDest);

        out3DDestOptionsPanel.add(new JPanel(), "out3DNoDest");
        out3DDestOptionsPanel.add(out3DViewerPanel, "out3DViewer");
        out3DDestOptionsPanel.add(out3DFilePanel, "out3DFile");
        out3DDestOptionsPanel.add(out3DPipePanel, "out3DPipe");
        out3DDestPanel.add(out3DFile, null);
        out3DDestPanel.add(out3DPipe, null);
        out3DDestPanel.add(out3DNoDest, null);

        this.add(out3DCheckBox, new GridBagConstraints(0, 4, 1, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 10), 0, 0));
        this.add(out3DDestPanel, new GridBagConstraints(1, 4, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(out3DDestOptionsPanel, new GridBagConstraints(1, 5, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));

        this.add(new JSeparator(SwingConstants.HORIZONTAL), new GridBagConstraints(0, 6, GridBagConstraints.REMAINDER, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(20, 0, 20, 10), 0, 0));

        //-----------2D section--------------
        JPanel out2DDestOptionsPanel = new JPanel(new CardLayout());
        OnActionClickerLayoutSwitcher out2DDestListener =
                new OnActionClickerLayoutSwitcher(out2DCheckBox, out2DDestOptionsPanel);
        out2DCheckBox.setText("2D representation");
        out2DCheckBox.setMnemonic(KeyEvent.VK_2);
        out2DCheckBox.setToolTipText(ShortcutHint);
        out2DCheckBox.addActionListener(outputOptionsListener);
        out2DViewer.setText("Viewer");
        out2DViewer.setMnemonic(KeyEvent.VK_E);
        out2DViewer.setActionCommand("out2DViewer");
        out2DViewer.addActionListener(out2DDestListener);
        out2DViewer.addActionListener(outputOptionsListener);
        out2DViewer.setToolTipText("send 2D embeddings to some of the installed viewers");
        out2DFile.setText("File");
        out2DFile.setMnemonic(KeyEvent.VK_I);
        out2DFile.setActionCommand("out2DFile");
        out2DFile.addActionListener(out2DDestListener);
        out2DFile.addActionListener(outputOptionsListener);
        out2DFile.setToolTipText("send 2D embeddings into a file");
        out2DPipe.setText("Pipe");
        out2DPipe.setMnemonic(KeyEvent.VK_P);
        out2DPipe.setActionCommand("out2DPipe");
        out2DPipe.addActionListener(out2DDestListener);
        out2DPipe.addActionListener(outputOptionsListener);
        out2DPipe.setToolTipText("send 2D embeddings into a pipe");
        out2DBatch.setText("Batch");
        out2DBatch.setMnemonic(KeyEvent.VK_B);
        out2DBatch.setActionCommand("out2DBatch");
        out2DBatch.addActionListener(out2DDestListener);
        out2DBatch.addActionListener(outputOptionsListener);
        out2DBatch.setToolTipText("send 2D embeddings to TwoView painter");
        JRadioButton out2DNoDest = new JRadioButton("None");
        out2DNoDest.setActionCommand("out2DNoDest");
        out2DNoDest.addActionListener(out2DDestListener);
        out2DNoDest.setVisible(false);

        out2DFilePanel.addActionListener(uiActionListener);

        JPanel out2DViewerPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 5));

        JPanel out2DDestPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10 , 5));
        if (addViewers("2D", null, out2DViewerGroup, out2DViewerPanel) > 0) {
            out2DDestGroup.add(out2DViewer);
            out2DDestPanel.add(out2DViewer, null);
            new OnActionClicker(out2DViewer, out2DNoDest, out2DCheckBox);
        } else {
            new OnActionClicker(out2DFile, out2DNoDest, out2DCheckBox);
        }
        
        final BatchTwoViewConfigurationPanel out2DBatchPanel =
                new BatchTwoViewConfigurationPanel(batchTwoViewModel);
        addOutputSettingsListener(new OutputSettingsListener() {

            @Override
            public void generatorInfoChanged(GeneratorInfo generatorInfo) {
                String numberFormat;
                if(generatorInfo.getFilename().endsWith("_")) {
                    numberFormat = "%d";
                } else {
                    numberFormat = "_%d";
                }
                batchTwoViewModel.setFileNameTemplate(
                        generatorInfo.getFilename() + numberFormat +
                        batchTwoViewModel.getSaver().getExtension());
            }
        });
        
        out2DDestGroup.add(out2DFile);
        out2DDestGroup.add(out2DPipe);
        out2DDestGroup.add(out2DBatch);
        out2DDestGroup.add(out2DNoDest);

        out2DDestOptionsPanel.add(new JPanel(), "out2DNoDest");
        out2DDestOptionsPanel.add(out2DViewerPanel, "out2DViewer");
        out2DDestOptionsPanel.add(out2DFilePanel, "out2DFile");
        out2DDestOptionsPanel.add(out2DPipePanel, "out2DPipe");
        out2DDestOptionsPanel.add(out2DBatchPanel, "out2DBatch");
        out2DDestPanel.add(out2DFile, null);
        out2DDestPanel.add(out2DPipe, null);
        out2DDestPanel.add(out2DBatch, null);
        out2DDestPanel.add(out2DNoDest, null);

        this.add(out2DCheckBox, new GridBagConstraints(0, 7, 1, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 10), 0, 0));
        this.add(out2DDestPanel, new GridBagConstraints(1, 7, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(out2DDestOptionsPanel, new GridBagConstraints(1, 8, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));

        this.add(new JSeparator(SwingConstants.HORIZONTAL), new GridBagConstraints(0, 9, GridBagConstraints.REMAINDER, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(20, 0, 20, 10), 0, 0));

        //-----------Adjacency section--------------
        JPanel outAdjDestOptionsPanel = new JPanel(new CardLayout());
        JCheckBox outAdjCheckBox = new JCheckBox("Adjacency information");
        outAdjCheckBox.setMnemonic(KeyEvent.VK_A);
        outAdjCheckBox.setToolTipText("send connection table into a file or a pipe");
        outAdjCheckBox.addActionListener(outputOptionsListener);
        OnActionClickerLayoutSwitcher outAdjDestListener =
                new OnActionClickerLayoutSwitcher(outAdjCheckBox, outAdjDestOptionsPanel);
        outAdjFile.setText("File");
        outAdjFile.setMnemonic(KeyEvent.VK_L);
        outAdjFile.setActionCommand("outAdjFile");
        outAdjFile.addActionListener(outAdjDestListener);
        outAdjFile.addActionListener(outputOptionsListener);
        outAdjFile.setToolTipText("send connection table into a file");
        outAdjPipe.setText("Pipe");
        outAdjPipe.setMnemonic(KeyEvent.VK_P);
        outAdjPipe.setActionCommand("outAdjPipe");
        outAdjPipe.addActionListener(outAdjDestListener);
        outAdjPipe.addActionListener(outputOptionsListener);
        outAdjPipe.setToolTipText("send connection table into a pipe");
        JRadioButton outAdjNoDest = new JRadioButton("None");
        outAdjNoDest.setActionCommand("outAdjNoDest");
        outAdjNoDest.addActionListener(outAdjDestListener);
        outAdjNoDest.setVisible(false);

        outAdjFilePanel.addActionListener(uiActionListener);

        JPanel outAdjDestPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10 , 5));
        new OnActionClicker(outAdjFile, outAdjNoDest, outAdjCheckBox);
        outAdjDestGroup.add(outAdjFile);
        outAdjDestGroup.add(outAdjPipe);
        outAdjDestGroup.add(outAdjNoDest);

        outAdjDestOptionsPanel.add(new JPanel(), "outAdjNoDest");
        outAdjDestOptionsPanel.add(outAdjFilePanel, "outAdjFile");
        outAdjDestOptionsPanel.add(outAdjPipePanel, "outAdjPipe");
        outAdjDestPanel.add(outAdjFile, null);
        outAdjDestPanel.add(outAdjPipe, null);
        outAdjDestPanel.add(outAdjNoDest, null);

        this.add(outAdjCheckBox, new GridBagConstraints(0, 10, 1, 1, 0.1, 1.0, GridBagConstraints.NORTHWEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 10), 0, 0));
        this.add(outAdjDestPanel, new GridBagConstraints(1, 10, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(outAdjDestOptionsPanel, new GridBagConstraints(1, 11, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
    }

    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        this.generatorInfo = generatorInfo;
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
            } else {
                embedder.setEmbed2D(Systoolbox.parseCmdLine(embed2DCmdLine.getText()));
                fireGeneratorInfoChanged();
            }
            if (embed3DCmdLine.getText().length() == 0 || !embedder.isConstant()) {
                embed3DCmdLine.setText(Systoolbox.makeCmdLine(embedder.getEmbed3DNew()));
            } else {
                embedder.setEmbed3D(Systoolbox.parseCmdLine(embed3DCmdLine.getText()));
                fireGeneratorInfoChanged();
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
        out2DFilePanel.setTargetName(filename);
        out3DFilePanel.setTargetName(filename);
        outAdjFilePanel.setTargetName(filename);
        fireGeneratorInfoChanged();
    }

    public GeneratorInfo getGeneratorInfo() {
        return generatorInfo;
    }

    public void showing() {
        defaultButton = SwingUtilities.getRootPane(this).getDefaultButton();
        checkOutputOptions();
    }

    public void actionPerformed(ActionEvent e) {
        if (e.getSource() instanceof javax.swing.text.JTextComponent || e.getSource() instanceof TargetPanel) {
            if (defaultButton != null) {
                defaultButton.doClick();
            }
        } else {
            checkOutputOptions();
        }
    }

    void checkOutputOptions() {
        boolean someViewer, someFile;
        someViewer = out2DViewer.isSelected() || out3DViewer.isSelected();
        someFile =
                outAdjFile.isSelected() ||
                out2DFile.isSelected() ||
                out3DFile.isSelected() ||
                outAdjPipe.isSelected() ||
                out2DPipe.isSelected() ||
                out3DPipe.isSelected() ||
                out2DBatch.isSelected();
        defaultButton.setEnabled(someViewer ^ someFile);
        defaultButton.setToolTipText(
                defaultButton.isEnabled() ? "start generation process (Return)" : someFile | someViewer ? "don't mix viewer and file output" : "choose some output options and press Return");
    }

    List<String> createViewerNames(String dimension, List<String> viewersDim) {
        if (viewersDim == null) {
            viewersDim = Systoolbox.stringToVector(
                    CaGe.config.getProperty(generatorName + ".Viewers." + dimension));
        }
        return viewersDim;
    }

    final int addViewers(String dimName, List<String> viewersDim,
            GenericButtonGroup buttonGroup, JComponent component) {
        int dimension = dimName.charAt(0) - '0';
        viewersDim = createViewerNames(dimName, viewersDim);
        viewersXD = createViewerNames("xD", viewersXD);
        List<List<String>> vector = new ArrayList<>(Arrays.asList(viewersDim, viewersXD));
        int n = 0;
        for (int i = 0; i < vector.size(); ++i) {
            ListIterator<String> viewerNames = vector.get(i).listIterator();
            while (viewerNames.hasNext()) {
                String viewerName = viewerNames.next();
                if (!ViewerFactory.checkAvailability(viewerName, dimension)) {
                    continue;
                }
                AbstractButton viewerButton = new JCheckBox(
                        CaGe.config.getProperty(viewerName + ".Title"), n++ == 0);
                viewerButton.setActionCommand(viewerName);
                buttonGroup.add(viewerButton);
                component.add(viewerButton);
                if (i == 1) {
                    //add text viewer button to viewersXDGroup
                    viewersXDGroup.add(viewerButton);
                }
            }
        }
        return n;
    }

    public String[][] getPreFilter() {
        if (outPreFilterCheckBox.isSelected()) {
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

    public List<CaGeViewer> getViewers() {
        ButtonModel dest;
        List<CaGeViewer> viewers = new ArrayList<>();
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

    void addSelectedViewers(List<CaGeViewer> viewers, GenericButtonGroup buttonGroup, int dimension) {
        ListIterator<AbstractButton> buttons = buttonGroup.getElements();
        while (buttons.hasNext()) {
            AbstractButton button = (AbstractButton) buttons.next();
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
                    viewers.add(viewer);
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

    public List<CaGeWriter> getWriters() {
        ButtonModel dest;
        List<CaGeWriter> writers = new ArrayList<>();
        dest = outAdjDestGroup.getSelection();
        if (outAdjFile.getModel().equals(dest)) {
            addWriter(writers, outAdjFilePanel, 0);
        } else if (outAdjPipe.getModel().equals(dest)){
            addWriter(writers, outAdjPipePanel, 0);
        }
        dest = out2DDestGroup.getSelection();
        if (out2DFile.getModel().equals(dest)) {
            addWriter(writers, out2DFilePanel, 2);
        } else if (out2DPipe.getModel().equals(dest)){
            addWriter(writers, out2DPipePanel, 2);
        }
        dest = out3DDestGroup.getSelection();
        if (out3DFile.getModel().equals(dest)) {
            addWriter(writers, out3DFilePanel, 3);
        } else if (out3DPipe.getModel().equals(dest)){
            addWriter(writers, out3DPipePanel, 3);
        }
        return writers;
    }

    public List<WriterConfigurationHandler> getConfigurationHandlers() {
        ButtonModel dest;
        List<WriterConfigurationHandler> handlers = new ArrayList<>();
        dest = outAdjDestGroup.getSelection();
        if (outAdjFile.getModel().equals(dest)) {
            handlers.add(outAdjFilePanel.getConfigurationHandler());
        } else if (outAdjPipe.getModel().equals(dest)){
            handlers.add(outAdjPipePanel.getConfigurationHandler());
        }
        dest = out2DDestGroup.getSelection();
        if (out2DFile.getModel().equals(dest)) {
            handlers.add(out2DFilePanel.getConfigurationHandler());
        } else if (out2DPipe.getModel().equals(dest)){
            handlers.add(out2DPipePanel.getConfigurationHandler());
        }
        dest = out3DDestGroup.getSelection();
        if (out3DFile.getModel().equals(dest)) {
            handlers.add(out3DFilePanel.getConfigurationHandler());
        } else if (out3DPipe.getModel().equals(dest)){
            handlers.add(out3DPipePanel.getConfigurationHandler());
        }
        return handlers;
    }

    void addWriter(List<CaGeWriter> writers, FileFormatBox format, int dimension) {
        CaGeWriter writer = format.getCaGeWriter();
        writer.setGeneratorInfo(getGeneratorInfo());
        writers.add(writer);
    }

    void addWriter(List<CaGeWriter> writers, TargetPanel filePanel, int dimension) {
        CaGeWriter writer = filePanel.getCaGeWriter();
        writer.setGeneratorInfo(getGeneratorInfo());
        writers.add(writer);
    }

    public List<String> getWriteDestinations() {
        ButtonModel dest;
        List<String> writeDests = new ArrayList<>();
        dest = outAdjDestGroup.getSelection();
        if (outAdjFile.getModel().equals(dest)) {
            writeDests.add(outAdjFilePanel.getTargetName());
        } else if(outAdjPipe.getModel().equals(dest)){
            writeDests.add(outAdjPipePanel.getTargetName());
        }
        dest = out2DDestGroup.getSelection();
        if (out2DFile.getModel().equals(dest)) {
            writeDests.add(out2DFilePanel.getTargetName());
        } else if(out2DPipe.getModel().equals(dest)){
            writeDests.add(out2DPipePanel.getTargetName());
        }
        dest = out3DDestGroup.getSelection();
        if (out3DFile.getModel().equals(dest)) {
            writeDests.add(out3DFilePanel.getTargetName());
        } else if(out3DPipe.getModel().equals(dest)){
            writeDests.add(out3DPipePanel.getTargetName());
        }
        return writeDests;
    }
    
    public boolean isBatchProcessorSelected(){
        return out2DBatch.isSelected();
    }
    
    public BatchTwoViewModel getBatchConfiguration(){
        return batchTwoViewModel;
    }
    
    private List<OutputSettingsListener> outputSettingsListeners = new ArrayList<>();
    
    public final void addOutputSettingsListener(OutputSettingsListener l){
        outputSettingsListeners.add(l);
    }
    
    public final void removeOutputSettingsListener(OutputSettingsListener l){
        outputSettingsListeners.remove(l);
    }

    private void fireGeneratorInfoChanged(){
        for (OutputSettingsListener l : outputSettingsListeners) {
            l.generatorInfoChanged(generatorInfo);
        }
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

            @Override
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

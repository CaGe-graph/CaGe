package cage.generator;

import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.util.Vector;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.ButtonGroup;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.SpinButton;

public class GeneralPlanarGraphsPanel extends GeneratorPanel
        implements ActionListener, ChangeListener {

    private static final boolean debug = false;
    public static final int MIN_VERTICES = 4;
    public static final int MAX_VERTICES = 40;
    public static final int DEFAULT_VERTICES = 4;

    private JCheckBox dual;
    private EnhancedSlider verticesSlider;
    private ButtonGroup minDegGroup;
    private ButtonGroup minConnGroup;
    private SpinButton minEdges, maxEdges;
    private JCheckBox defaultEdges;
    private SpinButton maxFacesize;
    private JCheckBox defaultMaxFacesize;
    private JRadioButton[] degButtons = new JRadioButton[5];
    private JRadioButton[] connButtons = new JRadioButton[3];
    private JCheckBox exactConn;

    public GeneralPlanarGraphsPanel() {
        setLayout(new GridBagLayout());
        dual = new JCheckBox("dual graphs");
        dual.setMnemonic(KeyEvent.VK_D);
        add(dual,
                new GridBagConstraints(1, 0, 2, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 5, 20, 0), 0, 0));
        verticesSlider = new EnhancedSlider();
        verticesSlider.setMinimum(MIN_VERTICES);
        verticesSlider.setMaximum(MAX_VERTICES);
        verticesSlider.setValue(DEFAULT_VERTICES);
        verticesSlider.setMinorTickSpacing(1);
        verticesSlider.setMajorTickSpacing(MAX_VERTICES - MIN_VERTICES);
        verticesSlider.setPaintTicks(true);
        verticesSlider.setPaintLabels(true);
        verticesSlider.setSnapWhileDragging(1);
        verticesSlider.setClickScrollByBlock(false);
        verticesSlider.setSizeFactor(4);
        verticesSlider.addChangeListener(this);
        add(verticesSlider,
                new GridBagConstraints(1, 1, 2, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 5, 15, 0), 0, 0));
        JLabel verticesLabel = new JLabel("number of vertices");
        verticesLabel.setLabelFor(verticesSlider.slider());
        verticesLabel.setDisplayedMnemonic(KeyEvent.VK_V);
        add(verticesLabel,
                new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 20, 10), 0, 0));
        add(new JLabel("minimum degree"),
                new GridBagConstraints(0, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 20, 10), 0, 0));
        minDegGroup = new ButtonGroup();
        JPanel minDegPanel = new JPanel();
        for (int i = 1; i <= 5; ++i) {
            String is = Integer.toString(i);
            degButtons[i-1] = new JRadioButton(is, i == 3);
            degButtons[i-1].setActionCommand(is);
            degButtons[i-1].setMnemonic(KeyEvent.VK_0 + i);
            minDegGroup.add(degButtons[i-1]);
            minDegPanel.add(degButtons[i-1]);
            degButtons[i-1].addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    int minDegree = Integer.parseInt(minDegGroup.getSelection().getActionCommand());
                    int minConn = Integer.parseInt(minConnGroup.getSelection().getActionCommand());
                    if(minDegree < minConn)
                        connButtons[minDegree - 1].setSelected(true);
                    setValues();
                }
            });
        }
        add(minDegPanel,
                new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 20, 0), 0, 0));
        add(new JLabel("minimum connectivity"),
                new GridBagConstraints(0, 3, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 20, 10), 0, 0));
        minConnGroup = new ButtonGroup();
        JPanel minConnPanel = new JPanel();
        for (int i = 1; i <= 3; ++i) {
            String is = Integer.toString(i);
            connButtons[i-1] = new JRadioButton(is, i == 3);
            connButtons[i-1].setActionCommand(is);
            connButtons[i-1].setMnemonic(KeyEvent.VK_0 + i);
            minConnGroup.add(connButtons[i-1]);
            minConnPanel.add(connButtons[i-1]);
            connButtons[i-1].addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    int minDegree = Integer.parseInt(minDegGroup.getSelection().getActionCommand());
                    int minConn = Integer.parseInt(minConnGroup.getSelection().getActionCommand());
                    if(minDegree < minConn)
                        degButtons[minConn - 1].setSelected(true);
                    if(minConn==3){
                        exactConn.setSelected(false);
                        exactConn.setEnabled(false);
                    } else {
                        exactConn.setEnabled(true);
                    }
                    setValues();
                }
            });
        }
        add(minConnPanel,
                new GridBagConstraints(1, 3, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 20, 0), 0, 0));
        exactConn = new JCheckBox("only graphs with connectivity exactly the given minimum");
        exactConn.setMnemonic(KeyEvent.VK_R);
        exactConn.setEnabled(false);
        add(exactConn,
                new GridBagConstraints(0, 4, 2, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 10), 0, 0));
        add(new JLabel("number of edges"),
                new GridBagConstraints(0, 5, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 10), 0, 0));
        defaultEdges = new JCheckBox("all", true);
        defaultEdges.setMnemonic(KeyEvent.VK_A);
        defaultEdges.setActionCommand("a");
        defaultEdges.addActionListener(this);
        minEdges = new SpinButton(1, 1, 999);
        minEdges.setEnabled(false);
        maxEdges = new SpinButton(1, 1, 999);
        maxEdges.setEnabled(false);
        JLabel minEdgesLabel = new JLabel("min.");
        minEdgesLabel.setLabelFor(minEdges);
        minEdgesLabel.setDisplayedMnemonic(KeyEvent.VK_I);
        JLabel maxEdgesLabel = new JLabel("max.");
        maxEdgesLabel.setLabelFor(maxEdges);
        maxEdgesLabel.setDisplayedMnemonic(KeyEvent.VK_A);
        JPanel edgesPanel = new JPanel();
        edgesPanel.add(minEdgesLabel);
        edgesPanel.add(minEdges);
        edgesPanel.add(Box.createHorizontalStrut(10));
        edgesPanel.add(maxEdgesLabel);
        edgesPanel.add(maxEdges);
        add(edgesPanel,
                new GridBagConstraints(1, 5, 1, 1, 1.0, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 0), 0, 0));
        add(defaultEdges,
                new GridBagConstraints(2, 5, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 20, 10, 0), 0, 0));
        maxFacesize = new SpinButton(DEFAULT_VERTICES - 1, 3, MAX_VERTICES - 1);
        maxFacesize.setEnabled(false);
        JLabel maxFacesizeLabel = new JLabel("max.");
        maxFacesizeLabel.setLabelFor(maxFacesize);
        maxFacesizeLabel.setDisplayedMnemonic(KeyEvent.VK_X);
        JPanel maxFacesizePanel = new JPanel();
        maxFacesizePanel.add(maxFacesizeLabel);
        maxFacesizePanel.add(maxFacesize);
        add(new JLabel("face size"),
                new GridBagConstraints(0, 6, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 0, 10), 0, 0));
        add(maxFacesizePanel,
                new GridBagConstraints(1, 6, 1, 1, 1.0, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(0, 0, 0, 0), 0, 0));
        defaultMaxFacesize = new JCheckBox("all", true);
        defaultMaxFacesize.setMnemonic(KeyEvent.VK_L);
        defaultMaxFacesize.setActionCommand("d");
        defaultMaxFacesize.addActionListener(this);
        add(defaultMaxFacesize,
                new GridBagConstraints(2, 6, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 20, 0, 0), 0, 0));
        add(Box.createVerticalStrut(80),
                new GridBagConstraints(3, 5, 1, 2, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 0, 0), 0, 0));
        setValues();
        setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));
    }

    private void setValues() {
        int n = verticesSlider.getValue();
        int minDegree = Integer.parseInt(minDegGroup.getSelection().getActionCommand());
        int minConn = Integer.parseInt(minConnGroup.getSelection().getActionCommand());
        //the lowerbound is the ceil of minDegree * n / 2 except in case of minimum degree
        //equal to 1. Because of the connectedness the lowerbound is then n-1.
        int min = minDegree == 1 ? (n-1) : (minDegree * n / 2 + (minDegree * n % 2));
        int max = 3 * n - 6;
        // make sure both intervals are first widened and then narrowed.
        boolean nIncreased = maxEdges.getMaximum() < max;
        if (nIncreased) {
            minEdges.setMaximum(max);
            minEdges.setMinimum(min);
            maxEdges.setMaximum(max);
            maxEdges.setMinimum(min);
        } else {
            minEdges.setMinimum(min);
            minEdges.setMaximum(max);
            maxEdges.setMinimum(min);
            maxEdges.setMaximum(max);
        }
        if (defaultEdges.isSelected()) {
            minEdges.setValue(min);
            maxEdges.setValue(max);
        }
        maxFacesize.setMaximum(n - 1);
        if (defaultMaxFacesize.isSelected()) {
            maxFacesize.setValue(n - 1);
        }
    }

    public void showing() {
    }

    public GeneratorInfo getGeneratorInfo() {
        Vector genCmd = new Vector();
        String filename = "";

        genCmd.addElement("plantri");
        genCmd.addElement("-p");
        filename += "genplan";
        String v = Integer.toString(verticesSlider.getValue());
        filename += "_" + v;
        if (dual.isSelected()) {
            genCmd.addElement("-d");
            filename += "_d";
        }
        String minDeg = minDegGroup.getSelection().getActionCommand();
        genCmd.addElement("-m" + minDeg);
        filename += "_m" + minDeg;
        String minConn = minConnGroup.getSelection().getActionCommand();
        genCmd.addElement("-c" + minConn + (exactConn.isSelected() ? "x" : ""));
        filename += "_c" + minConn + (exactConn.isSelected() ? "x" : "");
        if (!defaultEdges.isSelected()) {
            genCmd.addElement("-e" + minEdges.getValue() + ":" + maxEdges.getValue());
            filename += "_e" + minEdges.getValue() + "-" + maxEdges.getValue();
        }
        if (!defaultMaxFacesize.isSelected()) {
            genCmd.addElement("-f" + maxFacesize.getValue());
            filename += "_f" + maxFacesize.getValue();
        }
        genCmd.addElement(v);

        String[][] generator = new String[1][genCmd.size()];
        genCmd.copyInto(generator[0]);
        if (debug) {
            System.err.println(Systoolbox.makeCmdLine(generator));
        }

        String[][] embed2D = {{"embed"}};
        String[][] embed3D = {{"embed", "-d3", "-it"}};

        return new StaticGeneratorInfo(
                generator,
                EmbedFactory.createEmbedder(true, embed2D, embed3D),
                filename,
                dual.isSelected() ? verticesSlider.getValue() - 1 : maxFacesize.getValue());
    }

    public void actionPerformed(ActionEvent e) {
        switch (e.getActionCommand().charAt(0)) {
            case 'a':
                if (defaultEdges.isSelected()) {
                    minEdges.setValue(minEdges.getMinimum());
                    maxEdges.setValue(maxEdges.getMaximum());
                    minEdges.setEnabled(false);
                    maxEdges.setEnabled(false);
                } else {
                    minEdges.setEnabled(true);
                    maxEdges.setEnabled(true);
                    minEdges.requestFocus();
                }
                break;
            case 'd':
                if (defaultMaxFacesize.isSelected()) {
                    maxFacesize.setValue(verticesSlider.getValue() - 1);
                    maxFacesize.setEnabled(false);
                } else {
                    maxFacesize.setEnabled(true);
                    maxFacesize.requestFocus();
                }
                break;
        }
    }

    public void stateChanged(ChangeEvent e) {
        setValues();
    }
}


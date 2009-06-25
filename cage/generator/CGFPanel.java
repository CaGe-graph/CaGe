package cage.generator;

import cage.ElementRule;
import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;
import cage.ValencyElementRule;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.EventListener;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Vector;
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import javax.swing.JToggleButton;
import javax.swing.SwingConstants;
import javax.swing.UIManager;

import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import lisken.systoolbox.MutableInteger;
import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.MinMaxEqListener;
import lisken.uitoolbox.UItoolbox;

public class CGFPanel extends GeneratorPanel {

    public static final int MIN_ATOMS = 4;
    public static final int MAX_ATOMS = 250;
    public static final int MIN_POLYGON_FACES = 3;
    public static final int MAX_POLYGON_FACES = 40;

    public static final int DUAL_MIN_ATOMS = 4;
    public static final int DUAL_MAX_ATOMS = 127;

    private final boolean dual;

    private EnhancedSlider minAtomsSlider = new EnhancedSlider();
    private EnhancedSlider maxAtomsSlider = new EnhancedSlider();
    private JCheckBox faceStats = new JCheckBox();
    private JCheckBox conn1 = new JCheckBox();
    private JCheckBox conn2 = new JCheckBox();
    private JCheckBox conn3 = new JCheckBox();
    private GonOptionsMap gonOptionsMap;

    public CGFPanel() {
        this(false);
    }

    public CGFPanel(boolean dual) {
        this.dual = dual;
        try {
            initGUI();
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    void initGUI() throws Exception {
        setLayout(new GridBagLayout());
        JPanel CGFExtrasPanel = new JPanel(new GridBagLayout());
        JPanel CGFAtomsPanel = new JPanel(new GridBagLayout());
        JPanel CGFFaceOptionsPanel = new JPanel(new GridBagLayout());
        JLabel minAtomsLabel = new JLabel("minimum number of vertices");
        minAtomsLabel.setLabelFor(minAtomsSlider.slider());
        minAtomsLabel.setDisplayedMnemonic(KeyEvent.VK_N);
        JLabel maxAtomsLabel = new JLabel("maximum number of vertices");
        maxAtomsLabel.setLabelFor(maxAtomsSlider.slider());
        maxAtomsLabel.setDisplayedMnemonic(KeyEvent.VK_X);
        minAtomsSlider.setMajorTickSpacing(dual ? (DUAL_MAX_ATOMS - DUAL_MIN_ATOMS) : (MAX_ATOMS - MIN_ATOMS));
        minAtomsSlider.setSnapToTicks(true);
        minAtomsSlider.setMinimum(dual ? DUAL_MIN_ATOMS : MIN_ATOMS);
        minAtomsSlider.setMaximum(dual ? DUAL_MAX_ATOMS : MAX_ATOMS);
        minAtomsSlider.setValue(minAtomsSlider.getMinimum());
        minAtomsSlider.setMinorTickSpacing(2 - (dual ? (DUAL_MAX_ATOMS - DUAL_MIN_ATOMS) : (MAX_ATOMS - MIN_ATOMS)) % 2);
        minAtomsSlider.setPaintMinorTicks(false);
        minAtomsSlider.setPaintLabels(true);
        minAtomsSlider.setPaintTicks(true);
        minAtomsSlider.setSnapWhileDragging(minAtomsSlider.getMinorTickSpacing());
        minAtomsSlider.setClickScrollByBlock(false);
        maxAtomsSlider.setMajorTickSpacing(dual ? (DUAL_MAX_ATOMS - DUAL_MIN_ATOMS) : (MAX_ATOMS - MIN_ATOMS));
        maxAtomsSlider.setSnapToTicks(true);
        maxAtomsSlider.setMinimum(dual ? DUAL_MIN_ATOMS : MIN_ATOMS);
        maxAtomsSlider.setMaximum(dual ? DUAL_MAX_ATOMS : MAX_ATOMS);
        maxAtomsSlider.setValue(maxAtomsSlider.getMinimum());
        maxAtomsSlider.setMinorTickSpacing(2 - (dual ? (DUAL_MAX_ATOMS - DUAL_MIN_ATOMS) : (MAX_ATOMS - MIN_ATOMS)) % 2);
        maxAtomsSlider.setPaintMinorTicks(false);
        maxAtomsSlider.setPaintLabels(true);
        maxAtomsSlider.setPaintTicks(true);
        maxAtomsSlider.setSnapWhileDragging(maxAtomsSlider.getMinorTickSpacing());
        maxAtomsSlider.setClickScrollByBlock(false);
        final JCheckBox minEqMax = new JCheckBox("min = max", true);
        minEqMax.setMnemonic(KeyEvent.VK_M);
        EventListener l = new MinMaxEqListener(minAtomsSlider.getModel(), maxAtomsSlider.getModel(), minEqMax.getModel(), false);
        final EnhancedSlider facesSlider = new EnhancedSlider();
        Hashtable facesLabels = facesSlider.createStandardLabels(5, 10);
        facesLabels.put(new Integer(3), new JLabel("3"));
        facesLabels.put(new Integer(6), new JLabel("6"));
        JLabel faceTypeLabel = new JLabel(dual ? "Degrees" : "Face Type");
        faceTypeLabel.setLabelFor(facesSlider.slider());
        faceTypeLabel.setDisplayedMnemonic(KeyEvent.VK_F);
        JToggleButton facesButton = new JToggleButton(dual ? "include this degree" : "include this face type");
        facesButton.setMnemonic(KeyEvent.VK_I);
        facesSlider.setOrientation(SwingConstants.HORIZONTAL);
        facesSlider.setLabelTable(facesLabels);
        facesSlider.setMinorTickSpacing(1);
        facesSlider.setSnapToTicks(true);
        facesSlider.setPaintTicks(true);
        facesSlider.setPaintLabels(true);
        facesSlider.setMaximum(MAX_POLYGON_FACES);
        facesSlider.setMinimum(MIN_POLYGON_FACES);
        facesSlider.setValue(facesSlider.getMinimum() + 1);
        facesSlider.setMajorTickSpacing(facesSlider.getMaximum() - facesSlider.getMinimum());
        facesSlider.setSnapWhileDragging(1);
        facesSlider.setSizeFactor(12);
        facesSlider.setClickScrollByBlock(false);
        facesSlider.slider().addKeyListener(new KeyAdapter() {

            public void keyPressed(KeyEvent e) {
                if (e.getModifiers() != 0) {
                    return;
                }
                int n = e.getKeyCode() - KeyEvent.VK_0;
                if (n < 0 || n > 9) {
                    return;
                }
                n = (n + 7) % 10 + 3;
                facesSlider.setValue(n);
            }
        });
        gonOptionsMap = new GonOptionsMap(CGFFaceOptionsPanel, facesSlider.slider(), facesSlider.getModel(), facesButton, dual, true);
        gonOptionsMap.setGonIncluded(facesSlider.getMinimum(), true);
        gonOptionsMap.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                verifyData();
            }
        });
        JLabel includedFacesLabel = new JLabel(dual ? "included degrees" : "included face types:");
        ActionListener connListener = new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                verifyData();
            }
        };
        faceStats.setText(dual ? "Degree Statistics" : "Face Statistics");
        faceStats.setMnemonic(KeyEvent.VK_S);
        conn1.setText("graphs with connectivity number 1");
        conn1.setMnemonic(KeyEvent.VK_1);
        conn1.setActionCommand("1");
        conn1.setSelected(false);
        conn1.addActionListener(connListener);
        conn2.setText("graphs with connectivity number 2");
        conn2.setMnemonic(KeyEvent.VK_2);
        conn2.setSelected(false);
        conn2.setActionCommand("2");
        conn2.addActionListener(connListener);
        conn3.setText("graphs with connectivity number 3");
        conn3.setMnemonic(KeyEvent.VK_3);
        conn3.setSelected(true);
        conn3.setActionCommand("3");
        conn3.addActionListener(connListener);
        JPanel CGFFacesPanel = new JPanel(new GridBagLayout());
        CGFFacesPanel.add(faceTypeLabel, new GridBagConstraints(0, 0, 2, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 5, 5, 0), 0, 0));
        CGFFacesPanel.add(facesSlider, new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        CGFFacesPanel.add(facesButton, new GridBagConstraints(1, 1, 1, 1, 0.0010, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 30, 0, 5), 0, 5));
        CGFAtomsPanel.add(minAtomsLabel, new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 5, 5, 0), 0, 0));
        CGFAtomsPanel.add(maxAtomsLabel, new GridBagConstraints(1, 0, 1, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 5, 5, 0), 0, 0));
        CGFAtomsPanel.add(minAtomsSlider, new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 20), 0, 0));
        CGFAtomsPanel.add(maxAtomsSlider, new GridBagConstraints(1, 1, 1, 1, 1.0, 1.0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 20), 0, 0));
        CGFAtomsPanel.add(minEqMax, new GridBagConstraints(2, 1, 1, 1, 0.001, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        if(!dual) CGFExtrasPanel.add(new JLabel(), new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        //CGFExtrasPanel.add(faceStats, new GridBagConstraints(1, 0, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        if(!dual){
            CGFExtrasPanel.add(conn1, new GridBagConstraints(3, 0, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
            CGFExtrasPanel.add(conn2, new GridBagConstraints(3, 1, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
            CGFExtrasPanel.add(conn3, new GridBagConstraints(3, 2, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        }
        this.add(CGFAtomsPanel, new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0, GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(new JSeparator(SwingConstants.HORIZONTAL), new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0, GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(20, 0, 20, 0), 0, 0));
        this.add(CGFFacesPanel, new GridBagConstraints(0, 2, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(includedFacesLabel, new GridBagConstraints(0, 3, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(20, 5, 0, 0), 0, 0));
        this.add(CGFFaceOptionsPanel, new GridBagConstraints(0, 4, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 10, 10, 0), 0, 0));
        if(!dual){
            this.add(new JSeparator(SwingConstants.HORIZONTAL), new GridBagConstraints(0, 5, 1, 1, 1.0, 1.0, GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(20, 0, 20, 0), 0, 0));
            this.add(CGFExtrasPanel, new GridBagConstraints(0, 6, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        }
    }

    public GeneratorInfo getGeneratorInfo() {
        String[][] generator, embed2D, embed3D;
        String filename;
        int maxFacesize;

        String c;
        Vector genV = new Vector(), fileV = new Vector();

        int min = dual ? minAtomsSlider.getValue()*2-4 : minAtomsSlider.getValue();
        int max = dual ? maxAtomsSlider.getValue()*2-4 : maxAtomsSlider.getValue();

        int nrOfRestrictions = 0;
        int maxSize = 0;
        {
            Iterator it = gonOptionsMap.values().iterator();
            while (it.hasNext()) {
                GonOption gonOption = (GonOption) it.next();
                if (!gonOption.isActive()) {
                    continue;
                }
                nrOfRestrictions++;
                if(maxSize<gonOption.getSize())
                    maxSize = gonOption.getSize();
            }
        }

        boolean useCgf = (min != max) || faceStats.isSelected() ||
                conn1.isSelected() || conn2.isSelected() ||
                (nrOfRestrictions < 5) || (maxSize>10);

        if(useCgf){
            Systoolbox.addArray(genV, new String[]{
                        "cgf", "-g", "0", "-output", "stdout", "-logfile", "stderr",
                        "-save", "0", "-no_recover", "-topdown", "-outputmem", "0", "0"});
            genV.addElement("-v");
            genV.addElement(Integer.toString(max));
            fileV.addElement("cgf");
            fileV.addElement("n" + max);

            if (min != max) {
                genV.addElement("-vs");
                genV.addElement(Integer.toString(min));
                fileV.addElement("s" + min);
            }

            Iterator it = gonOptionsMap.values().iterator();
            while (it.hasNext()) {
                GonOption gonOption = (GonOption) it.next();
                if (!gonOption.isActive()) {
                    continue;
                }
                genV.addElement("-f");
                genV.addElement(Integer.toString(gonOption.getFaces()));
                String s = "f" + gonOption.getFaces();
                if (gonOption.isLimited()) {
                    genV.addElement("l" + gonOption.getMin() + "-" + gonOption.getMax() + "u");
                    s = s + "+" + gonOption.getMin() + "-" + gonOption.getMax();
                }
                fileV.addElement(s);
            }

            if(dual) genV.add("-dual");
            cage.Utils.addIfSelected(genV, faceStats, "-facestat");

            if(dual){
                c = "3";
            } else {
                c = "";
                if (conn1.isSelected()) {
                    c = c + "1";
                }
                if (conn2.isSelected()) {
                    c = c + "2";
                }
                if (conn3.isSelected()) {
                    c = c + "3";
                }
            }
            genV.addElement("-mapcon");
            genV.addElement(c);
            if (c.length() < 4) {
                fileV.addElement("c" + c);
            }

            if(dual) fileV.add("dual");

            generator = new String[1][genV.size()];
            genV.copyInto(generator[0]);
            String[] array = new String[fileV.size()];
            fileV.copyInto(array);
            filename = Systoolbox.join(array, "_");

        } else {
            int vertices = dual ? minAtomsSlider.getValue() : minAtomsSlider.getValue()/2+2;
            generator = new String[1][dual ? 3 : 4];
            Iterator it = gonOptionsMap.values().iterator();
            genV.addElement("-");
            fileV.addElement("plantri_ad_");
            while (it.hasNext()) {
                GonOption gonOption = (GonOption) it.next();
                if (!gonOption.isActive()) {
                    continue;
                }
                genV.addElement("F");
                fileV.addElement("F");
                genV.addElement(Integer.toString(gonOption.getFaces()));
                fileV.addElement(Integer.toString(gonOption.getFaces()));
                if (gonOption.isLimited()) {
                    genV.addElement("_" + gonOption.getMin() + "^" + gonOption.getMax());
                    fileV.addElement("m" + gonOption.getMin() + "M" + gonOption.getMax());
                }
            }
            if(!dual) fileV.addElement("_d");
            fileV.addElement("_" + vertices);
            String[] array = new String[genV.size()];
            genV.copyInto(array);
            String restrict = Systoolbox.join(array, "");
            int j = 0;
            generator[0][j++] = "plantri_ad";
            generator[0][j++] = restrict;
            if(!dual)
                generator[0][j++] = "-d";
            generator[0][j++] = Integer.toString(vertices);
            String[] fileArray = new String[fileV.size()];
            fileV.copyInto(fileArray);
            filename = Systoolbox.join(fileArray, "");
        }

        embed2D = new String[][]{{"embed"}};
        embed3D = new String[][]{{"embed", "-d3", "-it"}};

        if (dual) {
            maxFacesize = 3;
        } else {
            maxFacesize = maxSize;
        }

        ElementRule rule = new ValencyElementRule("H O C Si N S I");

        return new StaticGeneratorInfo(
                generator,
                EmbedFactory.createEmbedder(true, embed2D, embed3D),
                filename, maxFacesize, rule);
    }

    public void showing() {
        verifyData();
    }

    /**
     * verifies whether the next button should be enabled or disabled.
     */
    private void verifyData(){
        //is at least one of the connected boxes checked
        boolean c[] = {conn1.isSelected(), conn2.isSelected(), conn3.isSelected()};
        int cs = (c[0] ? 1 : 0) + (c[1] ? 1 : 0) + (c[2] ? 1 : 0);

        //is at least one face type added
        boolean faceAdded = false;

        Iterator it = gonOptionsMap.values().iterator();
        if(it.hasNext()){
            GonOption gonOption = (GonOption) it.next();
            while (!gonOption.isActive() && it.hasNext()) {
                gonOption = (GonOption) it.next();
            }
            faceAdded = gonOption.isActive();
        }

        getNextButton().setEnabled(cs!=0 && faceAdded);
    }

    public static void main(String[] args) {
        try {
            UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
        } catch (Exception e) {
        }
        final CGFPanel p = new CGFPanel();
        final JFrame f = new JFrame("Output Dialog");
        f.addWindowListener(new WindowAdapter() {

            public void windowClosing(WindowEvent e) {
                f.setVisible(false);
                GeneratorInfo info = p.getGeneratorInfo();
                JOptionPane.showInputDialog(null, "Command Line", "CGF results",
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


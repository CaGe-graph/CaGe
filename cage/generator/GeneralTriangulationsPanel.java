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
import javax.swing.AbstractButton;
import javax.swing.Box;
import javax.swing.ButtonGroup;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;

import lisken.uitoolbox.EnhancedSlider;

/**
 * Configuration panel for general triangulations and 3-regular plane graphs.
 * This is used to configure the generator <i>plantri</i>.
 * <p>
 * This configuration panel can be used to generate both triangulations and
 * there dual, 3-regular plane graphs. The number of vertices
 * in the dual, i.e. the number of faces in the original, is calculated
 * using the Euler formula.
 * <p>
 * For the triangulations: E=(3/2)F<br>
 * Therefore:
 * <blockquote>
 *     V - E + F = 2<br>
 *     V - (3/2)F + F = 2<br>
 *     F/2 = V - 2<br>
 *     F = 2V - 4<br>
 * </blockquote>
 * So the dual has two vertices less than the original.
 */
public class GeneralTriangulationsPanel extends GeneratorPanel implements ActionListener {

    /**
     * Is this panel in the normal mode or the dual mode? This needs to be set
     * on construction, i.e. the dual state of the panel is immutable.
     */
    private final boolean dual;

    //The minimum number of vertices allowed for this generator
    public static final int MIN_VERTICES = 4;
    //The maximum number of vertices allowed for this generator
    public static final int MAX_VERTICES = 50;
    //The default number of vertices for this generator
    public static final int DEFAULT_VERTICES = 4;

    //The minimum number of vertices allowed for this generator in the dual case
    //The minimum for a general triangulation is 4 = (4+4/2)
    private static final int DUAL_MIN_VERTICES = 4;
    //The maximum number of vertices allowed for this generator in the dual case
    //The maximum for a general triangulation is 50 = (96+4/2)
    private static final int DUAL_MAX_VERTICES = 96;
    //The default number of vertices for this generator in the dual case
    private static final int DUAL_DEFAULT_VERTICES = 4;


    //slider to select number of vertices
    private EnhancedSlider verticesSlider;
    //buttons to select the minimum degree
    private ButtonGroup minDegGroup;
    private AbstractButton[] degButton = new AbstractButton[3];
    //buttons to select the minimum connectivity
    private ButtonGroup minConnGroup;
    private AbstractButton[] connButton;
    //checkbox to generate only graphs with exactly the minimum connectivity
    private JCheckBox exactConn;

    public GeneralTriangulationsPanel() {
        this(false);
    }

    public GeneralTriangulationsPanel(boolean dual) {
        this.dual = dual;
        setLayout(new GridBagLayout());
        verticesSlider = new EnhancedSlider();
        verticesSlider.setMinimum(dual ? DUAL_MIN_VERTICES : MIN_VERTICES);
        verticesSlider.setMaximum(dual ? DUAL_MAX_VERTICES : MAX_VERTICES);
        verticesSlider.setValue(dual ? DUAL_DEFAULT_VERTICES : DEFAULT_VERTICES);
        verticesSlider.setMinorTickSpacing(dual ? 2 : 1); //vertices has to be even for dual
        verticesSlider.setMajorTickSpacing(dual ?
            (DUAL_MAX_VERTICES - DUAL_MIN_VERTICES) :
            (MAX_VERTICES - MIN_VERTICES));
        verticesSlider.setPaintTicks(true);
        verticesSlider.setPaintLabels(true);
        verticesSlider.setSnapWhileDragging(dual ? 2 : 1);
        verticesSlider.setClickScrollByBlock(false);
        verticesSlider.setSnapToTicks(true); //vertices has to be even
        verticesSlider.setSizeFactor(4);
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
        JLabel minDegLabel = new JLabel(dual ? "minimum face size" : "minimum degree");
        add(minDegLabel,
                new GridBagConstraints(0, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 10), 0, 0));
        minDegGroup = new ButtonGroup();
        JPanel minDegPanel = new JPanel();
        for (int i = 0; i < degButton.length; ++i) {
            String is = Integer.toString(i + 3);
            degButton[i] = new JRadioButton(is, i == 0);
            degButton[i].setActionCommand("v" + is);
            degButton[i].addActionListener(this);
            minDegGroup.add(degButton[i]);
            minDegPanel.add(degButton[i]);
        }
        add(minDegPanel,
                new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 0), 0, 0));
        add(Box.createHorizontalGlue(),
                new GridBagConstraints(2, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 0), 0, 0));
        JLabel minConnLabel = new JLabel(dual ? "minimum cyclic connectivity" : "minimum connectivity");
        add(minConnLabel,
                new GridBagConstraints(0, 3, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 10), 0, 0));
        minConnGroup = new ButtonGroup();
        JPanel minConnPanel = new JPanel();
        connButton = new AbstractButton[dual ? 5 : 3];
        for (int i = 0; i < connButton.length; ++i) {
            String is = Integer.toString(i + (dual? 1 : 3));
            connButton[i] = new JRadioButton(is, i == connButton.length - 3);
            connButton[i].setActionCommand(is);
            connButton[i].setActionCommand("c" + is);
            connButton[i].addActionListener(this);
            minConnGroup.add(connButton[i]);
            minConnPanel.add(connButton[i]);
        }
        add(minConnPanel,
                new GridBagConstraints(1, 3, 1, 1, 1.0, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 0), 0, 0));
        add(Box.createHorizontalGlue(),
                new GridBagConstraints(2, 3, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 0), 0, 0));
        exactConn = new JCheckBox(dual ? "only graphs with cyclic connectivity exactly the given minimum" : "only graphs with connectivity exactly the given minimum");
        exactConn.setMnemonic(KeyEvent.VK_R);
        add(exactConn,
                new GridBagConstraints(1, 4, 2, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 5, 0, 0), 0, 0));
    }

    public void showing() {
    }

    public GeneratorInfo getGeneratorInfo() {
        Vector genCmd = new Vector();
        String filename = "";

        genCmd.addElement("plantri");
        filename += "tri";
        String vertices;
        if(dual)
            vertices = Integer.toString(verticesSlider.getValue() / 2 + 2);
            //plantri takes the number of vertices as argument, i.e. number of
            //faces in the dual graph so we use the euler formula to derive it
            //from the number of vertices
        else
            vertices = Integer.toString(verticesSlider.getValue());
        filename += "_" + vertices;
        if(dual){
            genCmd.addElement("-d");
            filename += "_d";
        }
        String minConn = minConnGroup.getSelection().getActionCommand().substring(1);
        minConn += exactConn.isSelected() ? "x" : "";
        genCmd.addElement("-c" + minConn);
        filename += "_c" + minConn;
        String minDeg = minDegGroup.getSelection().getActionCommand().substring(1);
        genCmd.addElement("-m" + minDeg);
        filename += "_m" + minDeg;
        genCmd.addElement(vertices);

        String[][] generator = new String[1][genCmd.size()];
        genCmd.copyInto(generator[0]);

        String[][] embed2D = {{"embed"}};
        String[][] embed3D = {{"embed", "-d3", "-it"}};

        return new StaticGeneratorInfo(
                generator,
                EmbedFactory.createEmbedder(true, embed2D, embed3D),
                filename, 3, true);
    }

    public void actionPerformed(ActionEvent e) {
        String cmd = e.getActionCommand();
        char deg, conn;
        switch (cmd.charAt(0)) {
            case 'c':
                deg = minDegGroup.getSelection().getActionCommand().charAt(1);
                conn = cmd.charAt(1);
                if (deg < conn) {
                    degButton[conn - '0' - 3].setSelected(true);
                }
                break;
            case 'v':
                conn = minConnGroup.getSelection().getActionCommand().charAt(1);
                deg = cmd.charAt(1);
                if (deg < conn) {
                    connButton[deg - '0' - (dual ? 1 : 3)].setSelected(true);
                }
                break;
        }
    }
}

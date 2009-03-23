package cage.generator;

import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.KeyEvent;
import java.util.Vector;
import javax.swing.ButtonGroup;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;

import lisken.uitoolbox.EnhancedSlider;

public class EulerianTriangulationsPanel extends GeneratorPanel {

    /**
     * Is this panel in the normal mode or the dual mode? This needs to be set
     * on construction, i.e. the dual state of the panel is immutable.
     */
    private final boolean dual;

    //The minimum number of vertices allowed for this generator
    private static final int MIN_VERTICES = 6;
    //The maximum number of vertices allowed for this generator
    private static final int MAX_VERTICES = 40;
    //The default number of vertices for this generator
    private static final int DEFAULT_VERTICES = 6;

    //The minimum number of vertices allowed for this generator in the dual case
    //The minimum for a eulerian triangulation is 6 = (8+4/2)
    private static final int DUAL_MIN_VERTICES = 8;
    //The maximum number of vertices allowed for this generator in the dual case
    //The maximum for a eulerian triangulation is 40 = (76+4/2)
    private static final int DUAL_MAX_VERTICES = 76;
    //The default number of vertices for this generator in the dual case
    private static final int DUAL_DEFAULT_VERTICES = 8;


    //slider to select number of vertices
    private EnhancedSlider verticesSlider;
    //buttons to select the minimum connectivity
    private ButtonGroup minConnGroup;
    //checkbox to generate only graphs with exactly the minimum connectivity
    private JCheckBox exactConn;

    public EulerianTriangulationsPanel() {
        this(false);
    }

    public EulerianTriangulationsPanel(final boolean dual) {
        this.dual = dual;
        setLayout(new GridBagLayout());
        verticesSlider = new EnhancedSlider();
        verticesSlider.setMinimum(dual ? DUAL_MIN_VERTICES : MIN_VERTICES);
        verticesSlider.setMaximum(dual ? DUAL_MAX_VERTICES : MAX_VERTICES);
        verticesSlider.setValue(dual ? DUAL_DEFAULT_VERTICES : DEFAULT_VERTICES);
        verticesSlider.setMinorTickSpacing(1);
        verticesSlider.setMajorTickSpacing(dual ?
            (DUAL_MAX_VERTICES - DUAL_MIN_VERTICES) :
            (MAX_VERTICES - MIN_VERTICES));
        verticesSlider.setPaintTicks(true);
        verticesSlider.setPaintLabels(true);
        verticesSlider.setSnapWhileDragging(1);
        verticesSlider.setClickScrollByBlock(false);
        verticesSlider.setSizeFactor(4);
        add(verticesSlider,
                new GridBagConstraints(1, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 5, 15, 0), 0, 0));
        JLabel verticesLabel = new JLabel("number of vertices");
        verticesLabel.setLabelFor(verticesSlider.slider());
        verticesLabel.setDisplayedMnemonic(KeyEvent.VK_V);
        add(verticesLabel,
                new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 20, 10), 0, 0));
        JLabel minConnLabel = new JLabel(dual ? "minimum cyclic connectivity" : "minimum connectivity");
        add(minConnLabel,
                new GridBagConstraints(0, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 15, 10), 0, 0));
        minConnGroup = new ButtonGroup();
        JPanel minConnPanel = new JPanel();
        for (int i = 3; i <= 4; ++i) {
            String is = Integer.toString(i);
            JRadioButton connButton = new JRadioButton(is, i == 3);
            connButton.setActionCommand(is);
            minConnGroup.add(connButton);
            minConnPanel.add(connButton);
        }
        add(minConnPanel,
                new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 0), 0, 0));
        exactConn = new JCheckBox("only graphs with connectivity exactly the given minimum");
        exactConn.setMnemonic(KeyEvent.VK_R);
        add(exactConn,
                new GridBagConstraints(1, 3, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 5, 0, 0), 0, 0));
    }

    public void showing() {
    }

    public GeneratorInfo getGeneratorInfo() {
        Vector genCmd = new Vector();
        String filename = "";

        genCmd.addElement("plantri");
        genCmd.addElement("-b");
        filename += "tri_euler";
        String v;
        if(dual)
            v = Integer.toString(verticesSlider.getValue());
        else
            v = Integer.toString(verticesSlider.getValue() / 2 + 2);
            //plantri takes the number of vertices as argument, i.e. number of
            //faces in the dual graph so we use the euler formula to derive it
            //from the number of vertices

        filename += "_" + v;
        if(dual){
            genCmd.addElement("-d");
            filename += "_d";
        }
        String minConn = minConnGroup.getSelection().getActionCommand();
        if (minConn.charAt(0) < '4') {
            minConn += exactConn.isSelected() ? "x" : "";
        }
        genCmd.addElement("-c" + minConn);
        filename += "_c" + minConn;
        genCmd.addElement(v);

        String[][] generator = new String[1][genCmd.size()];
        genCmd.copyInto(generator[0]);

        String[][] embed2D = {{"embed"}};
        String[][] embed3D = {{"embed", "-d3", "-it"}};

        return new StaticGeneratorInfo(
                generator,
                EmbedFactory.createEmbedder(true, embed2D, embed3D),
                filename, 3, true);
    }
}

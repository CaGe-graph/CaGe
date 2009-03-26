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
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;

import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import lisken.uitoolbox.EnhancedSlider;

/**
 * Configuration panel for general quadrangulations and 4-regular plane graphs.
 * This is used to configure the generator <i>plantri -q</i>.
 * <p>
 * This configuration panel can be used to generate both quadrangulations and
 * there dual, 4-regular plane graphs. The number of vertices
 * in the dual, i.e. the number of faces in the original, is calculated
 * using the Euler formula.
 * <p>
 * For the quadrangulations: E=(4/2)F = 2F<br>
 * Therefore:
 * <blockquote>
 *     V - E + F = 2<br>
 *     V - 2F + F = 2<br>
 *     F = V - 2<br>
 * </blockquote>
 * So the dual has two vertices less than the original.

 * @author nvcleemp
 */
public class GeneralQuadrangulationsPanel extends GeneratorPanel {

    /**
     * Is this panel in the normal mode or the dual mode? This needs to be set
     * on construction, i.e. the dual state of the panel is immutable.
     */
    private final boolean dual;
    /**
     * The minimum number of vertices allowed for this generator in the non-dual case
     */
    public static final int MIN_VERTICES = 8;
    /**
     * The minimum number of vertices allowed for this generator in the non-dual case
     * for 2-connected quadrangulations.
     */
    public static final int MIN_VERTICES_2_CONNECTED = 4;
    /**
     * The maximum number of vertices allowed for this generator in the non-dual case
     */
    public static final int MAX_VERTICES = 50;
    /**
     * The default number of vertices for this generator in the non-dual case
     */
    public static final int DEFAULT_VERTICES = MIN_VERTICES;

    //The same values for the dual. These are calculated using the euler formula.
    
    /**
     * The minimum number of vertices allowed for this generator in the dual case
     */
    public static final int DUAL_MIN_VERTICES = MIN_VERTICES - 2;
    /**
     * The maximum number of vertices allowed for this generator in the dual case
     */
    public static final int DUAL_MAX_VERTICES = MAX_VERTICES - 2;
    /**
     * The default number of vertices for this generator in the dual case
     */
    public static final int DUAL_DEFAULT_VERTICES = DUAL_MIN_VERTICES;

    //slider to select number of vertices
    private EnhancedSlider verticesSlider;

    //buttongroup for the selection of the options
    private ButtonGroup optionGroup = new ButtonGroup();

    /**
     * Constructs a non-dual <code>GeneralQuadrangulationsPanel</code> object.
     */
    public GeneralQuadrangulationsPanel() {
        this(false);
    }

    /**
     * Constructs a <code>GeneralQuadrangulationsPanel</code> object.
     * @param dual Flag to indicate whether the generator should be used to create
     *             quadrangulations (<tt>false</tt>) or 4-regular plane graphs
     *             (<tt>true</tt>)
     */
    public GeneralQuadrangulationsPanel(boolean dual) {
        this.dual = dual;
        setLayout(new GridBagLayout());

        verticesSlider = new EnhancedSlider();
        verticesSlider.setMinimum(dual ? DUAL_MIN_VERTICES : MIN_VERTICES_2_CONNECTED);
        verticesSlider.setMaximum(dual ? DUAL_MAX_VERTICES : MAX_VERTICES);
        verticesSlider.setValue(dual ? DUAL_DEFAULT_VERTICES : DEFAULT_VERTICES);
        verticesSlider.setMinorTickSpacing(2); //vertices has to be even
        verticesSlider.setMajorTickSpacing(dual ? DUAL_MAX_VERTICES - DUAL_MIN_VERTICES : MAX_VERTICES - MIN_VERTICES_2_CONNECTED);
        verticesSlider.setPaintTicks(true);
        verticesSlider.setPaintLabels(true);
        verticesSlider.setSnapWhileDragging(2);
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

        JPanel optionPanel = new JPanel();
        optionPanel.setLayout(new BoxLayout(optionPanel, BoxLayout.Y_AXIS));
        JRadioButton button;
        //button for -c2m2
        if(!dual){
            //for the dual case this has no meaning, because
            //the option -c2m2 would lead to multigraphs, which
            //are not supported by CaGe at the moment
            button = new JRadioButton("minimum degree 2", true);
            button.setActionCommand("-c2m2");
            button.addChangeListener(new ChangeListener() {

                public void stateChanged(ChangeEvent e) {
                    JRadioButton radioButton = (JRadioButton)e.getSource();
                    if(radioButton.isSelected()){
                        verticesSlider.setMinimum(MIN_VERTICES_2_CONNECTED);
                        verticesSlider.setMajorTickSpacing(MAX_VERTICES - MIN_VERTICES_2_CONNECTED);
                    } else {
                        verticesSlider.setMinimum(MIN_VERTICES);
                        verticesSlider.setMajorTickSpacing(MAX_VERTICES - MIN_VERTICES);
                    }
                }
            });
            optionGroup.add(button);
            optionPanel.add(button);
        }

        //button for -c2
        button = new JRadioButton(dual ? "4-edge-connected graphs" : "minimum degree 3", dual);
        button.setActionCommand("-c2");
        optionGroup.add(button);
        optionPanel.add(button);

        //button for -c3m3
        button = new JRadioButton(dual ? "3-connected graphs" : "3-connected", false);
        button.setActionCommand("-c3m3");
        optionGroup.add(button);
        optionPanel.add(button);

        //button for -c4
        button = new JRadioButton(dual ? "3-connected, 6-cyclically-edge-connected graphs" :
                                         "3-connected, no non-facial 4-cycles", false);
        button.setActionCommand("-c4");
        optionGroup.add(button);
        optionPanel.add(button);

        add(optionPanel,
                new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 0), 0, 0));
    }

    public void showing() {
        //enable next button, because this might have been disabled.
        if(getNextButton()!=null)
            getNextButton().setEnabled(true);
    }

    public GeneratorInfo getGeneratorInfo() {
        Vector genCmd = new Vector();
        String filename = "";


        filename += "quad";
        //for the dual case the number of vertices is actually the number of faces
        String vertices = Integer.toString(verticesSlider.getValue() + (dual ? 2 : 0));
        filename += "_" + vertices;
        String option = optionGroup.getSelection().getActionCommand();
        filename += option.replace('-', '_');
        if(dual) filename += "_d";

        genCmd.addElement("plantri");
        genCmd.addElement("-q");
        genCmd.addElement(option);
        if(dual) genCmd.addElement("-d");
        genCmd.addElement(vertices);

        String[][] generator = new String[1][genCmd.size()];
        genCmd.copyInto(generator[0]);

        String[][] embed2D = {{"embed"}};
        String[][] embed3D = {{"embed", "-d3", "-it"}};

        return new StaticGeneratorInfo(
                generator,
                EmbedFactory.createEmbedder(true, embed2D, embed3D),
                filename, dual ? 0 : 4, true);
    }
}

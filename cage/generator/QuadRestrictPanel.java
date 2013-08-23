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
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.util.Iterator;
import java.util.Vector;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import javax.swing.JToggleButton;
import javax.swing.SwingConstants;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.uitoolbox.EnhancedSlider;

/**
 * Configuration panel for quadrangulations with given degree and 4-regular
 * plane graphs with given faces. This is used to configure
 * the generator <i>quad_restrict -q</i>, which is actually <i>plantri</i> with
 * the filter <i>quad_restrict.c</i> by Sebastian Funke.
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
 *
 * @author nvcleemp
 */
public class QuadRestrictPanel extends GeneratorPanel implements ChangeListener {

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

    //the lower bound for a restricted degree/face
    private static final int RESTRICTION_MINIMUM = 3;
    private static final int RESTRICTION_MAXIMUM = 40;
    //slider to select number of vertices
    private EnhancedSlider verticesSlider;

    //
    private SizeOptionsMap sizeOptionsMap;

    /**
     * Constructs a non-dual <code>GeneralQuadrangulationsPanel</code> object.
     */
    public QuadRestrictPanel() {
        this(false);
    }

    /**
     * Constructs a <code>GeneralQuadrangulationsPanel</code> object.
     * @param dual Flag to indicate whether the generator should be used to create
     *             quadrangulations (<tt>false</tt>) or 4-regular plane graphs
     *             (<tt>true</tt>)
     */
    public QuadRestrictPanel(boolean dual) {
        this.dual = dual;
        setLayout(new GridBagLayout());

        //vertices
        verticesSlider = new EnhancedSlider();
        verticesSlider.setMinimum(dual ? DUAL_MIN_VERTICES : MIN_VERTICES);
        verticesSlider.setMaximum(dual ? DUAL_MAX_VERTICES : MAX_VERTICES);
        verticesSlider.setValue(dual ? DUAL_DEFAULT_VERTICES : DEFAULT_VERTICES);
        verticesSlider.setMinorTickSpacing(1);
        verticesSlider.setMajorTickSpacing(dual ? DUAL_MAX_VERTICES - DUAL_MIN_VERTICES : MAX_VERTICES - MIN_VERTICES);
        verticesSlider.setPaintTicks(true);
        verticesSlider.setPaintLabels(true);
        verticesSlider.setSnapWhileDragging(1);
        verticesSlider.setClickScrollByBlock(false);
        verticesSlider.setSnapToTicks(true); //vertices has to be even
        verticesSlider.setSizeFactor(4);
        add(verticesSlider,
                new GridBagConstraints(1, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 5, 15, 0), 0, 0));
        JLabel verticesLabel = new JLabel("number of vertices");
        verticesLabel.setLabelFor(verticesSlider.slider());
        verticesLabel.setDisplayedMnemonic(KeyEvent.VK_V);
        add(verticesLabel,
                new GridBagConstraints(0, 1, 1, 1, 0.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 20, 10), 0, 0));
        add(new JSeparator(),
                new GridBagConstraints(0, 2, 2, 1, 0.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.BOTH,
                new Insets(0, 0, 20, 10), 0, 0));

        //restrictions
        JPanel restrictionPanel = new JPanel(new GridBagLayout());
        final EnhancedSlider restrictionSlider = new EnhancedSlider();
        restrictionSlider.setOrientation(SwingConstants.HORIZONTAL);
        restrictionSlider.setMinorTickSpacing(1);
        restrictionSlider.setSnapToTicks(true);
        restrictionSlider.setPaintTicks(true);
        restrictionSlider.setPaintLabels(true);
        restrictionSlider.setMaximum(RESTRICTION_MAXIMUM);
        restrictionSlider.setMinimum(RESTRICTION_MINIMUM);
        restrictionSlider.setValue(restrictionSlider.getMinimum() + 1);
        restrictionSlider.setMajorTickSpacing(restrictionSlider.getMaximum() - restrictionSlider.getMinimum());
        restrictionSlider.setSnapWhileDragging(1);
        restrictionSlider.setSizeFactor(10);
        restrictionSlider.setClickScrollByBlock(false);
        restrictionSlider.slider().addKeyListener(new KeyAdapter() {

            @Override
            public void keyPressed(KeyEvent e) {
                if (e.getModifiers() != 0) {
                    return;
                }
                int n = e.getKeyCode() - KeyEvent.VK_0;
                if (n < 0 || n > 9) {
                    return;
                }
                n = (n + 7) % 10 + 3;
                restrictionSlider.setValue(n);
            }
        });
        restrictionSlider.setLabelTable(restrictionSlider.createStandardLabels(5, 5));
        JLabel restrictionTypeLabel = new JLabel(dual ? "Face type" : "Degree");
        restrictionTypeLabel.setLabelFor(restrictionSlider.slider());
        restrictionTypeLabel.setDisplayedMnemonic(KeyEvent.VK_F);
        JToggleButton addRestrictedSizeButton = new JToggleButton(dual ? "include this face type" : "include this degree");
        addRestrictedSizeButton.addActionListener(new ActionListener() {

            @Override
            public void actionPerformed(ActionEvent e) {
                checkParameters();
            }
        });
        addRestrictedSizeButton.setMnemonic(KeyEvent.VK_I);
        JPanel restrictedPanel = new JPanel(new GridBagLayout());
        sizeOptionsMap = new SizeOptionsMap(restrictedPanel, restrictionSlider.slider(), restrictionSlider.getModel(), addRestrictedSizeButton, !dual, true);
        sizeOptionsMap.addChangeListener(this);
        sizeOptionsMap.setSizeIncluded(restrictionSlider.getMinimum(), true);
        JLabel includedRestrictionsLabel = new JLabel(dual ? "included face types:" : "included degrees");
        restrictionPanel.add(restrictionTypeLabel, new GridBagConstraints(0, 0, 2, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 5, 5, 0), 0, 0));
        restrictionPanel.add(restrictionSlider, new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        restrictionPanel.add(addRestrictedSizeButton, new GridBagConstraints(1, 1, 1, 1, 0.0010, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 30, 0, 5), 0, 5));

        add(restrictionPanel, new GridBagConstraints(0, 3, 2, 1, 1.0, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 0), 0, 0));
        add(includedRestrictionsLabel, new GridBagConstraints(0, 4, 1, 1, 0.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 0), 0, 0));
        add(restrictedPanel, new GridBagConstraints(0, 5, 2, 1, 0.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 0), 0, 0));
    }

    @Override
    public void showing() {
        checkParameters();
    }

    /**
     * Construct the generator info for this generator. The restrictions are
     * given in the form -FxFy... where x,y,... are the sizes which are allowed.
     * The option -q is always necessary for quad_restrict and in case this
     * generator is used for dual graphs, the option -d is given and the number
     * of vertices is augmented with two.
     *
     * @return A <code>GeneratorInfo</code> object reflecting the settings on
     *         this panel.
     */
    @Override
    public GeneratorInfo getGeneratorInfo() {
        Vector genCmd = new Vector();
        StringBuilder filename = new StringBuilder("quad");

        //vertices
        //for the dual case the number of vertices is actually the number of faces
        String vertices = Integer.toString(verticesSlider.getValue() + (dual ? 2 : 0));
        filename.append("_").append(vertices);

        //restrictions
        StringBuilder restrictions = new StringBuilder();
        Iterator it = sizeOptionsMap.values().iterator();
        while (it.hasNext()) {
            SizeOption sizeOption = (SizeOption) it.next();
            if (!sizeOption.isActive()) {
                continue;
            }
            restrictions.append("F");
            restrictions.append(Integer.toString(sizeOption.getSize()));
            if(sizeOption.isLimited()){
                restrictions.append("_");
                restrictions.append(Integer.toString(sizeOption.getMin()));
                restrictions.append("^");
                restrictions.append(Integer.toString(sizeOption.getMax()));
            }
        }
        filename.append("_").append(restrictions.toString());

        //construct generator command
        genCmd.addElement("quad_restrict");
        genCmd.addElement("-q");
        genCmd.addElement("-" + restrictions.toString());
        if (dual) {
            genCmd.addElement("-d");
        }
        genCmd.addElement(vertices);

        String[][] generator = new String[1][genCmd.size()];
        genCmd.copyInto(generator[0]);

        //construct embedder commands
        String[][] embed2D = {{"embed"}};
        String[][] embed3D = {{"embed", "-d3", "-it"}};

        return new StaticGeneratorInfo(
                generator,
                EmbedFactory.createEmbedder(true, embed2D, embed3D),
                filename.toString(), dual ? 0 : 4, true);
    }

    @Override
    public void stateChanged(ChangeEvent e) {
        checkParameters();
    }

    private void checkParameters(){
        if (getNextButton() != null) {
            boolean enabled = false;
            for(Iterator it = sizeOptionsMap.values().iterator(); it.hasNext();)
                enabled =(((SizeOption)it.next()).isActive() || enabled);
            getNextButton().setEnabled(enabled);
        }
    }
}

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
import javax.swing.ButtonGroup;
import javax.swing.DefaultBoundedRangeModel;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JRadioButton;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.SpinButton;

/**
 * Panel used for the configuration of the generator cone.
 */
public class NanoConesPanel extends GeneratorPanel implements ActionListener {

    private EnhancedSlider numberOfPentagonsSlider = new EnhancedSlider();
    private SpinButton lengthOfSide = new SpinButton(new DefaultBoundedRangeModel(0, 0, 0, Integer.MAX_VALUE));
    private SpinButton hexagonLayers = new SpinButton(new DefaultBoundedRangeModel(1, 0, 1, Integer.MAX_VALUE));
    private JRadioButton symmetricButton;
    private JRadioButton nearSymmetricButton;
    private ButtonGroup symmetricGroup;
    private JCheckBox iprBox;
    private JCheckBox mirrorBox;
    private JCheckBox hexagonLayersBox;

    public NanoConesPanel() {
        setLayout(new GridBagLayout());

        numberOfPentagonsSlider.setMinimum(1);
        numberOfPentagonsSlider.setMaximum(5);
        numberOfPentagonsSlider.setMinorTickSpacing(1);
        numberOfPentagonsSlider.setMajorTickSpacing(numberOfPentagonsSlider.getMaximum() - numberOfPentagonsSlider.getMinimum());
        numberOfPentagonsSlider.setPaintTicks(true);
        numberOfPentagonsSlider.setPaintLabels(true);
        numberOfPentagonsSlider.setSnapToTicks(true);

        JLabel numberOfPentagonsLabel = new JLabel("Number of pentagons");

        symmetricButton = new JRadioButton("symmetric");
        nearSymmetricButton = new JRadioButton("nearsymmetric");
        symmetricButton.addActionListener(this);
        nearSymmetricButton.addActionListener(this);
        symmetricGroup = new ButtonGroup();
        symmetricGroup.add(symmetricButton);
        symmetricGroup.add(nearSymmetricButton);
        symmetricButton.setSelected(true);

        iprBox = new JCheckBox("isolated pentagons (ipr)");
        mirrorBox = new JCheckBox("Mirror-images to be considered nonisomorphic");
        hexagonLayersBox = new JCheckBox("Add a number of hexagon layers");
        hexagonLayers.setEnabled(hexagonLayersBox.isSelected());
        hexagonLayersBox.addActionListener(new HexagonLayersBoxListener());

        JLabel lengthOfSideLabel = new JLabel("Length of longest Side");

        add(symmetricButton,
                new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(nearSymmetricButton,
                new GridBagConstraints(1, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(numberOfPentagonsLabel,
                new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 10, 5, 10), 0, 0));
        add(numberOfPentagonsSlider,
                new GridBagConstraints(1, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(lengthOfSideLabel,
                new GridBagConstraints(0, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 10, 5, 10), 0, 0));
        add(lengthOfSide,
                new GridBagConstraints(1, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(iprBox,
                new GridBagConstraints(0, 3, 2, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(mirrorBox,
                new GridBagConstraints(0, 4, 2, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(hexagonLayersBox,
                new GridBagConstraints(0, 5, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(hexagonLayers,
                new GridBagConstraints(1, 5, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
    }

    public GeneratorInfo getGeneratorInfo() {
        String symmetric = symmetricButton.isSelected() ? " s" : "n";
        String pentagons = Integer.toString(numberOfPentagonsSlider.getValue());
        String length = Integer.toString(lengthOfSide.getValue() - (symmetricButton.isSelected() ? 0 : 1));

        String ipr = iprBox.isSelected() ? " -i" : "";
        String mirror = mirrorBox.isSelected() ? " -m" : "";

        String layers = hexagonLayersBox.isSelected() ? Integer.toString(hexagonLayers.getValue()) : "" ;

        return new StaticGeneratorInfo(
                Systoolbox.parseCmdLine("cone " + pentagons + " " + length + " " + symmetric + ipr + mirror + " " + layers),
                EmbedFactory.createEmbedder(new String[][]{{"embed"}}, new String[][]{{"embed", "-d3"}}),
                "test",
                6);
    }

    public void showing() {
        //
    }

    public void actionPerformed(ActionEvent e) {
        if (symmetricButton.isSelected()) {
            numberOfPentagonsSlider.setMinimum(1);
            numberOfPentagonsSlider.setMaximum(5);
            numberOfPentagonsSlider.setLabelTable(null);
            numberOfPentagonsSlider.setMajorTickSpacing(4);
        } else {
            numberOfPentagonsSlider.setMinimum(2);
            numberOfPentagonsSlider.setMaximum(4);
            numberOfPentagonsSlider.setLabelTable(null);
            numberOfPentagonsSlider.setMajorTickSpacing(2);
        }
    }

    /**
     * Listener responsible for disabling the hexagon layers SpinButton
     */
    private class HexagonLayersBoxListener implements ActionListener{

        public void actionPerformed(ActionEvent e) {
            hexagonLayers.setEnabled(hexagonLayersBox.isSelected());
        }

    }
}

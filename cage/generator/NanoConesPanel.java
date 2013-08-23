package cage.generator;

import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.SingleElementRule;
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
        numberOfPentagonsSlider.setSnapWhileDragging(1);

        symmetricButton = new JRadioButton("symmetric");
        nearSymmetricButton = new JRadioButton("nonsymmetric");
        symmetricButton.addActionListener(this);
        nearSymmetricButton.addActionListener(this);
        symmetricGroup = new ButtonGroup();
        symmetricGroup.add(symmetricButton);
        symmetricGroup.add(nearSymmetricButton);
        symmetricButton.setSelected(true);

        iprBox = new JCheckBox("isolated pentagons (ipr)");
        hexagonLayersBox = new JCheckBox("Add a number of hexagon layers");
        hexagonLayers.setEnabled(hexagonLayersBox.isSelected());
        hexagonLayersBox.addActionListener(new HexagonLayersBoxListener());

        add(symmetricButton,
                new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(nearSymmetricButton,
                new GridBagConstraints(1, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(new JLabel("Number of pentagons"),
                new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 10, 5, 10), 0, 0));
        add(numberOfPentagonsSlider,
                new GridBagConstraints(1, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(new JLabel("Length of longest Side"),
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
        add(hexagonLayersBox,
                new GridBagConstraints(0, 4, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(hexagonLayers,
                new GridBagConstraints(1, 4, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
    }

    @Override
    public GeneratorInfo getGeneratorInfo() {
        String symmetric = symmetricButton.isSelected() ? "s" : "n";
        String pentagons = Integer.toString(numberOfPentagonsSlider.getValue());
        String length = Integer.toString(lengthOfSide.getValue() - (symmetricButton.isSelected() ? 0 : 1));

        String ipr = iprBox.isSelected() ? " -i" : "";
        String iprFile = iprBox.isSelected() ? "_i" : "";

        String layers = hexagonLayersBox.isSelected() ? Integer.toString(hexagonLayers.getValue()) : "";

        return new StaticGeneratorInfo(
                Systoolbox.parseCmdLine("cone " + pentagons + " " + length + " " + symmetric + ipr + " " + layers),
                EmbedFactory.createEmbedder(new String[][]{{"embed"}}, new String[][]{{"java", "-cp", "CaGe.jar", "cage.embedder.NanoconeEmbedder"}}),
                "cone_" + pentagons + "_" + length + "_" + symmetric + iprFile + "_" + layers,
                6, true, new SingleElementRule("C"), 0);
    }

    @Override
    public void showing() {
        //
    }

    @Override
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
    private class HexagonLayersBoxListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {
            hexagonLayers.setEnabled(hexagonLayersBox.isSelected());
        }
    }
}

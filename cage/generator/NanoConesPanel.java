package cage.generator;

import cage.CaGe;
import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.SingleElementRule;
import cage.StaticGeneratorInfo;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.ButtonGroup;
import javax.swing.DefaultBoundedRangeModel;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.SpinButton;

/**
 * Panel used for the configuration of the generator cone.
 */
public class NanoConesPanel extends GeneratorPanel {

    private final ActionListener typeButtonListener = new ActionListener() {

        @Override
        public void actionPerformed(ActionEvent e) {
            if (faceCenteredButton.isSelected()) {
                numberOfPentagonsSlider.setMinimum(1);
                numberOfPentagonsSlider.setMaximum(5);
                numberOfPentagonsSlider.setLabelTable(null);
                numberOfPentagonsSlider.setMajorTickSpacing(4);
                numberOfPentagonsSlider.setMinorTickSpacing(1);
                numberOfPentagonsSlider.setSnapWhileDragging(1);
                numberOfPentagonsSlider.setEnabled(true);
            } else if (vertexCenteredButton.isSelected()) {
                numberOfPentagonsSlider.setMinimum(2);
                numberOfPentagonsSlider.setMaximum(4);
                numberOfPentagonsSlider.setLabelTable(null);
                numberOfPentagonsSlider.setMajorTickSpacing(2);
                numberOfPentagonsSlider.setMinorTickSpacing(2);
                numberOfPentagonsSlider.setSnapWhileDragging(2);
                numberOfPentagonsSlider.setEnabled(true);
            } else {
                numberOfPentagonsSlider.setMinimum(3);
                numberOfPentagonsSlider.setMaximum(3);
                numberOfPentagonsSlider.setLabelTable(null);
                numberOfPentagonsSlider.setMajorTickSpacing(1);
                numberOfPentagonsSlider.setMinorTickSpacing(1);
                numberOfPentagonsSlider.setSnapWhileDragging(1);
                numberOfPentagonsSlider.setEnabled(false);
            }
        }
    };
    
    private final EnhancedSlider numberOfPentagonsSlider = new EnhancedSlider();
    private final SpinButton lengthOfSide = new SpinButton(new DefaultBoundedRangeModel(0, 0, 0, Integer.MAX_VALUE));
    private final SpinButton hexagonLayers = new SpinButton(new DefaultBoundedRangeModel(1, 0, 1, Integer.MAX_VALUE));
    private final JRadioButton faceCenteredButton;
    private final JRadioButton vertexCenteredButton;
    private final JRadioButton edgeCenteredButton;
    private final ButtonGroup symmetricGroup;
    private final JCheckBox iprBox;
    private final JCheckBox hexagonLayersBox;

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

        faceCenteredButton = new JRadioButton("face-centered");
        vertexCenteredButton = new JRadioButton("vertex-centered");
        edgeCenteredButton = new JRadioButton("edge-centered");
        faceCenteredButton.addActionListener(typeButtonListener);
        vertexCenteredButton.addActionListener(typeButtonListener);
        edgeCenteredButton.addActionListener(typeButtonListener);
        symmetricGroup = new ButtonGroup();
        symmetricGroup.add(faceCenteredButton);
        symmetricGroup.add(vertexCenteredButton);
        symmetricGroup.add(edgeCenteredButton);
        faceCenteredButton.setSelected(true);

        iprBox = new JCheckBox("isolated pentagons (ipr)");
        hexagonLayersBox = new JCheckBox("Add a number of hexagon layers");
        hexagonLayers.setEnabled(hexagonLayersBox.isSelected());
        hexagonLayersBox.addActionListener(new HexagonLayersBoxListener());
        
        JPanel typeSelectionPanel = new JPanel(new GridLayout(1, 3));
        typeSelectionPanel.add(faceCenteredButton);
        typeSelectionPanel.add(vertexCenteredButton);
        typeSelectionPanel.add(edgeCenteredButton);

        add(typeSelectionPanel,
                new GridBagConstraints(0, 0, 2, 1, 1.0, 1.0,
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
        String symmetric = faceCenteredButton.isSelected() ? "s" : "n";
        String pentagons = Integer.toString(numberOfPentagonsSlider.getValue());
        String length = Integer.toString(lengthOfSide.getValue() - (faceCenteredButton.isSelected() ? 0 : 1));

        String ipr = iprBox.isSelected() ? " -i" : "";
        String iprFile = iprBox.isSelected() ? "_i" : "";

        String layers = hexagonLayersBox.isSelected() ? Integer.toString(hexagonLayers.getValue()) : "";

        return new StaticGeneratorInfo(
                Systoolbox.parseCmdLine("cone -s " + pentagons + " " + length + " " + symmetric + ipr + " " + layers),
                EmbedFactory.createEmbedder(new String[][]{{"embed"}}, new String[][]{{"java", "-cp", CaGe.installDirectory() + "/CaGe.jar", "cage.embedder.NanoconeEmbedder"}, {"embed", "-d3", "-ik", "-f0,0,0.01"}}),
                "cone_" + pentagons + "_" + length + "_" + symmetric + iprFile + "_" + layers,
                6, true, new SingleElementRule("C"), 0);
    }

    @Override
    public void showing() {
        //
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

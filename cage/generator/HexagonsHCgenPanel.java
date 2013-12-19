package cage.generator;

import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.KeyEvent;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.SwingUtilities;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;

public class HexagonsHCgenPanel extends GeneratorPanel {

    static final int MIN_HEXAGONS = 2;
    static final int MAX_HEXAGONS = 30;
    EnhancedSlider hexagonsSlider = new EnhancedSlider();
    JCheckBox iprBox;
    JCheckBox showHBox;
    JCheckBox cataCondensedBox;
    JCheckBox benzenoidBox;
    JCheckBox kekuleBox;
    JButton defaultButton;

    public HexagonsHCgenPanel() {
        setLayout(new GridBagLayout());

        hexagonsSlider.setMinimum(MIN_HEXAGONS);
        hexagonsSlider.setMaximum(MAX_HEXAGONS);
        hexagonsSlider.setValue(MIN_HEXAGONS);
        hexagonsSlider.setMinorTickSpacing(1);
        hexagonsSlider.setMajorTickSpacing(hexagonsSlider.getMaximum() - hexagonsSlider.getMinimum());
        hexagonsSlider.setPaintTicks(true);
        hexagonsSlider.setPaintLabels(true);
        hexagonsSlider.setSnapWhileDragging(1);
        hexagonsSlider.setSizeFactor(0.5);

        iprBox = new JCheckBox("isolated pentagons (ipr)");
        iprBox.setMnemonic(KeyEvent.VK_I);
        showHBox = new JCheckBox("include H atoms");
        showHBox.setMnemonic(KeyEvent.VK_A);

        cataCondensedBox = new JCheckBox("Cata condensed");
        benzenoidBox = new JCheckBox("Benzenoid");
        kekuleBox = new JCheckBox("Kekulean");

        add(hexagonsSlider,
                new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(cataCondensedBox,
                new GridBagConstraints(0, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(benzenoidBox,
                new GridBagConstraints(0, 3, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(kekuleBox,
                new GridBagConstraints(0, 4, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
    }

    @Override
    public GeneratorInfo getGeneratorInfo() {

        String generator;

        if (kekuleBox.isSelected() && !cataCondensedBox.isSelected()) {
            generator = "fusgen ";
        } else {
            generator = "catacondensed ";
        }

        String numberOfHexagons = hexagonsSlider.getValue() + " ";

        String catacondensed = cataCondensedBox.isSelected() ? " C" : "";
        String bezenoid = benzenoidBox.isSelected() ? " b" : "";
        String kekule = (kekuleBox.isSelected() && !cataCondensedBox.isSelected()) ? " k" : "";

        //String ipr = iprBox.isSelected() ? " ipr" : "";
        //String showH = showHBox.isSelected() ? " H" : "";

        //String preComputedPath = " p" + CaGe.getSystemProperty("CaGe.InstallDir") + "/PreCompute";

        return new StaticGeneratorInfo(
                Systoolbox.parseCmdLine(generator + numberOfHexagons + "p " + catacondensed + bezenoid + kekule),
                EmbedFactory.createEmbedder(new String[][]{{"embed"}}, new String[][]{{"embed", "-d3"}}),
                (generator + numberOfHexagons + "p" + catacondensed + bezenoid + kekule).replace(' ', '_'),
                6);
    }

    @Override
    public void showing() {
        defaultButton = SwingUtilities.getRootPane(this).getDefaultButton();
        defaultButton.setText("Next");
        defaultButton.setEnabled(true);
    }
}

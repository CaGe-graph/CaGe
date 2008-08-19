

package cage.generator;


import cage.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.text.*;
import lisken.systoolbox.*;
import lisken.uitoolbox.*;


public class HexagonsHCgenPanel extends GeneratorPanel
{

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
	kekuleBox = new JCheckBox("Kekule");

    add(hexagonsSlider,
     new GridBagConstraints2(0, 1, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(cataCondensedBox,
     new GridBagConstraints2(0, 2, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(benzenoidBox,
     new GridBagConstraints2(0, 3, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(kekuleBox,
     new GridBagConstraints2(0, 4, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    }

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
	String kekule = kekuleBox.isSelected()  ? " k" : "";

	//String ipr = iprBox.isSelected() ? " ipr" : "";
	//String showH = showHBox.isSelected() ? " H" : "";

	//String preComputedPath = " p" + CaGe.getSystemProperty("CaGe.InstallDir") + "/PreCompute";

	return new StaticGeneratorInfo(
            Systoolbox.parseCmdLine(generator + numberOfHexagons + "p " + catacondensed + bezenoid + kekule),
	    EmbedFactory.createEmbedder(new String[][]{{"embed"}}, new String[][]{{"embed", "-d3"}}),
	    "test",
	    6);
    }


    public void showing() {
	defaultButton = SwingUtilities.getRootPane(this).getDefaultButton();
	defaultButton.setText("Next");
	defaultButton.setEnabled(true);
    }

}

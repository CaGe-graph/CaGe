
package cage.generator;

import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import javax.swing.JCheckBox;
import javax.swing.JLabel;

import javax.swing.JTextField;
import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.GridBagConstraints2;

public class NanoConesPanel extends GeneratorPanel {
    
    EnhancedSlider numberOfPentagonsSlider = new EnhancedSlider();
    
    JTextField lengthOfSide = new JTextField();
    JCheckBox symmetricBox = new JCheckBox();

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
        symmetricBox = new JCheckBox("symmetric / semi-symmetric");
        
        JLabel lengthOfSideLabel = new JLabel("Length of Side");

    add(numberOfPentagonsLabel,
     new GridBagConstraints2(0, 0, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(0, 10, 5, 10), 0, 0));
    add(numberOfPentagonsSlider,
     new GridBagConstraints2(0, 1, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(lengthOfSideLabel,
     new GridBagConstraints2(0, 2, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(0, 10, 5, 10), 0, 0));
    add(lengthOfSide,
     new GridBagConstraints2(0, 3, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(symmetricBox,
     new GridBagConstraints2(0, 4, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    }

    public GeneratorInfo getGeneratorInfo() {
	String symmetric = symmetricBox.isSelected() ? " s" : "n";
	String pentagons = "" + numberOfPentagonsSlider.getValue();
        String length = "" + lengthOfSide.getText();

	return new StaticGeneratorInfo(
            Systoolbox.parseCmdLine("cone " +  pentagons + " " + length + " " + symmetric),
	    EmbedFactory.createEmbedder(new String[][]{{"embed"}}, new String[][]{{"embed", "-d3"}}),
	    "test",
	    6);
    }

    @Override
    public void showing() {
        //
    }

}

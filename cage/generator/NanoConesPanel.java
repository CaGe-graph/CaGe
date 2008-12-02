
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
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JRadioButton;
import javax.swing.JTextField;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;

public class NanoConesPanel extends GeneratorPanel implements ActionListener {
    
    EnhancedSlider numberOfPentagonsSlider = new EnhancedSlider();
    
    JTextField lengthOfSide = new JTextField();
    JRadioButton symmetricButton;
    JRadioButton nearSymmetricButton;
    ButtonGroup symmetricGroup;
    JCheckBox iprBox;
    JCheckBox mirrorBox;

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
        mirrorBox = new JCheckBox("Mirror-images are considered nonisomorphic");
        
        JLabel lengthOfSideLabel = new JLabel("Length of Side");

    add(numberOfPentagonsLabel,
     new GridBagConstraints(0, 0, 2, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(0, 10, 5, 10), 0, 0));
    add(numberOfPentagonsSlider,
     new GridBagConstraints(0, 1, 2, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(lengthOfSideLabel,
     new GridBagConstraints(0, 2, 2, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(0, 10, 5, 10), 0, 0));
    add(lengthOfSide,
     new GridBagConstraints(0, 3, 2, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(symmetricButton,
     new GridBagConstraints(0, 4, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(nearSymmetricButton,
     new GridBagConstraints(1, 4, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(iprBox,
     new GridBagConstraints(0, 5, 2, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(mirrorBox,
     new GridBagConstraints(0, 6, 2, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    }

    public GeneratorInfo getGeneratorInfo() {
	String symmetric = symmetricButton.isSelected() ? " s" : "n";
	String pentagons = "" + numberOfPentagonsSlider.getValue();
        String length = "" + lengthOfSide.getText();
        
        String ipr = iprBox.isSelected() ? " -i" : "";
        String mirror = mirrorBox.isSelected() ? " -m" : "";

	return new StaticGeneratorInfo(
            Systoolbox.parseCmdLine("cone " +  pentagons + " " + length + " " + symmetric + ipr + mirror),
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
}

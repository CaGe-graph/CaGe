

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


public class DegreeListHCgenPanel extends GeneratorPanel
 implements ActionListener
{

    JTextField degreeList;
    JLabel numberOfPentagonsLabel;

    JCheckBox iprBox;
    JCheckBox showHBox;

    JButton defaultButton;


    public DegreeListHCgenPanel() {
	setLayout(new GridBagLayout());

	degreeList = new JTextField(30);
	degreeList.setActionCommand("d");
	degreeList.getDocument().addDocumentListener(new MyListener());
	((AbstractDocument)(degreeList.getDocument())).setDocumentFilter(new DegreeListDocumentFilter());
	numberOfPentagonsLabel = new JLabel("................................");
	JLabel degreeListLabel = new JLabel("List of degrees (consecutive 2's and 3's)");

	iprBox = new JCheckBox("isolated pentagons (ipr)");
	iprBox.setMnemonic(KeyEvent.VK_I);
	showHBox = new JCheckBox("include H atoms");
	showHBox.setMnemonic(KeyEvent.VK_A);
	//add(degreeList);

	//add(numberOfPentagonsLabel);
	//add(degreeListLabel);

    add(degreeListLabel,
     new GridBagConstraints2(0, 0, 1, 1, 1.0, 1.0,
     GridBagConstraints.WEST, GridBagConstraints.NONE,
     new Insets(0, 10, 5, 10), 0, 0));
    add(degreeList,
     new GridBagConstraints2(0, 1, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(numberOfPentagonsLabel,
     new GridBagConstraints2(0, 2, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(iprBox,
     new GridBagConstraints2(0, 3, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    add(showHBox,
     new GridBagConstraints2(0, 4, 1, 1, 1.0, 1.0,
     GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
     new Insets(0, 10, 0, 10), 0, 0));
    }

    public GeneratorInfo getGeneratorInfo() {
	String ipr = iprBox.isSelected() ? " ipr" : "";
	String showH = showHBox.isSelected() ? " H" : "";

	String preComputedPath = " p" + CaGe.getSystemProperty("CaGe.InstallDir") + "/PreCompute";

	return new StaticGeneratorInfo(
            Systoolbox.parseCmdLine("vul_in " + degreeList.getText() + " o" + preComputedPath + ipr + showH),
	    EmbedFactory.createEmbedder(new String[][]{{"embed"}}, new String[][]{{"embed", "-d3"}}),
	    "test",
	    6);
    }


    public void showing() {
	defaultButton = SwingUtilities.getRootPane(this).getDefaultButton();
	defaultButton.setText("Next");
	checkList();
    }

    public void actionPerformed(ActionEvent e) {
	char actionCommand = e.getActionCommand().charAt(0);
	System.out.println(actionCommand);
	switch(actionCommand) {
	case 'd':
	    checkList();
	    break;
	}
    }

    private void checkList() {
	String list = degreeList.getText();
	int pentagons = 6 - countCharOccurence('2', list) + countCharOccurence('3', list);
	numberOfPentagonsLabel.setText("Number of pentagons: " + pentagons);
	if (pentagons > 5 || pentagons < 0) {
	    numberOfPentagonsLabel.setForeground(Color.RED);
	    defaultButton.setEnabled(false);
	} else {
	    defaultButton.setEnabled(true);
	    numberOfPentagonsLabel.setForeground(Color.BLACK);
	}
    }

    public int countCharOccurence(char c, String s) {
	int count = 0;
	for (int i = 0; i < s.length(); i++)
	    if (s.charAt(i) == c)
		count++;
	return count;

    }

    private class MyListener implements DocumentListener {

        public void insertUpdate(DocumentEvent e) {
	    checkList();
        }

        public void removeUpdate(DocumentEvent e) {
	    checkList();
        }

        public void changedUpdate(DocumentEvent e) {
	    checkList();
        }

    }

    private class DegreeListDocumentFilter extends DocumentFilter {

	@Override
	public void insertString(FilterBypass fb, int offset, String string, AttributeSet attr) throws BadLocationException {
	    int i = 0;
	    while (i < string.length() && (string.charAt(i) == '2' || string.charAt(i) == '3'))
		i++;
	    if (i == string.length())
		super.insertString(fb, offset, string, attr);

	}

	@Override
	public void replace(FilterBypass fb, int offset, int length, String string, AttributeSet attr) throws BadLocationException {
	    int i = 0;
	    while (i < string.length() && (string.charAt(i) == '2' || string.charAt(i) == '3'))
		i++;
	    if (i == string.length())
		super.replace(fb, offset, length, string, attr);
	}

    }

}

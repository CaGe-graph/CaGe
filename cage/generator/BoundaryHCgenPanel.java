package cage.generator;

import cage.CaGe;
import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;
import java.awt.Color;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.text.AbstractDocument;
import javax.swing.text.AttributeSet;
import javax.swing.text.BadLocationException;
import javax.swing.text.DocumentFilter;
import javax.swing.text.DocumentFilter.FilterBypass;
import lisken.systoolbox.Systoolbox;

public class BoundaryHCgenPanel extends GeneratorPanel
        implements ActionListener {

    JTextField boundaryList;
    JLabel numberOfPentagonsLabel;
    JCheckBox iprBox;
    JCheckBox showHBox;
    JButton defaultButton;

    public BoundaryHCgenPanel() {
        setLayout(new GridBagLayout());

        boundaryList = new JTextField(30);
        boundaryList.setActionCommand("d");
        boundaryList.getDocument().addDocumentListener(new MyListener());
        ((AbstractDocument) (boundaryList.getDocument())).setDocumentFilter(new BoundaryListDocumentFilter());
        numberOfPentagonsLabel = new JLabel("................................");
        JLabel boundaryListLabel = new JLabel("List of degrees (consecutive 2's and 3's)");

        iprBox = new JCheckBox("isolated pentagons (ipr)");
        iprBox.setMnemonic(KeyEvent.VK_I);
        showHBox = new JCheckBox("include H atoms");
        showHBox.setMnemonic(KeyEvent.VK_A);
        //add(boundaryList);

        //add(numberOfPentagonsLabel);
        //add(boundaryListLabel);

        add(boundaryListLabel,
                new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 10, 5, 10), 0, 0));
        add(boundaryList,
                new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(numberOfPentagonsLabel,
                new GridBagConstraints(0, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(iprBox,
                new GridBagConstraints(0, 3, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
        add(showHBox,
                new GridBagConstraints(0, 4, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 10, 0, 10), 0, 0));
    }

    public GeneratorInfo getGeneratorInfo() {
        String ipr = iprBox.isSelected() ? " ipr" : "";
        String showH = showHBox.isSelected() ? " H" : "";

        String preComputedPath = " p" + CaGe.getSystemProperty("CaGe.InstallDir") + "/PreCompute";

        return new StaticGeneratorInfo(
                Systoolbox.parseCmdLine("vul_in " + boundaryList.getText() + " o" + preComputedPath + ipr + showH),
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
        switch (actionCommand) {
            case 'd':
                checkList();
                break;
        }
    }

    private void checkList() {
        String list = boundaryList.getText();
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
        for (int i = 0; i < s.length(); i++) {
            if (s.charAt(i) == c) {
                count++;
            }
        }
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

    private class BoundaryListDocumentFilter extends DocumentFilter {

        public void insertString(FilterBypass fb, int offset, String string, AttributeSet attr) throws BadLocationException {
            int i = 0;
            while (i < string.length() && (string.charAt(i) == '2' || string.charAt(i) == '3')) {
                i++;
            }
            if (i == string.length()) {
                super.insertString(fb, offset, string, attr);
            }

        }

        public void replace(FilterBypass fb, int offset, int length, String string, AttributeSet attr) throws BadLocationException {
            int i = 0;
            while (i < string.length() && (string.charAt(i) == '2' || string.charAt(i) == '3')) {
                i++;
            }
            if (i == string.length()) {
                super.replace(fb, offset, length, string, attr);
            }
        }
    }
}

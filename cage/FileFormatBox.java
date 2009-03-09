package cage;

import cage.writer.CaGeWriter;
import cage.writer.WriterFactory;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.JComboBox;
import javax.swing.KeyStroke;
import javax.swing.text.JTextComponent;

import lisken.systoolbox.Systoolbox;

public class FileFormatBox extends JComboBox implements ActionListener {

    Vector writers = new Vector();
    int dimension = 0;
    JTextComponent filenameField;
    String oldExtension;

    public FileFormatBox(String variety, JTextComponent filenameField) {
        this.filenameField = filenameField;
        char firstChar = variety.charAt(0);
        if (Character.isDigit(firstChar)) {
            dimension = firstChar - '0';
        }
        Enumeration formats = Systoolbox.stringToVector(
                CaGe.config.getProperty("CaGe.Writers." + variety)).elements();
        while (formats.hasMoreElements()) {
            String format = (String) formats.nextElement();
            CaGeWriter writer = createCaGeWriter(format);
            format = createCaGeWriter(format).getFormatName();
            addItem(format);
            writers.addElement(writer);
        }
        addActionListener(this);
        registerKeyboardAction(this, "",
                KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0),
                WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);
        // new onActionFocusSwitcher(filenameField, this);
        oldExtension = getCaGeWriter().getFileExtension();
    }

    CaGeWriter createCaGeWriter(String format) {
        CaGeWriter writer;
        writer = WriterFactory.createCaGeWriter(format);
        if (dimension > 0) {
            writer.setDimension(dimension);
        }
        return writer;
    }

    public CaGeWriter getCaGeWriter() {
        return (CaGeWriter) writers.elementAt(getSelectedIndex());
    }

    public void addExtension() {
        String currentName = filenameField.getText();
        if (currentName.trim().startsWith("|")) {
            return;
        }
        CaGeWriter writer = getCaGeWriter();
        String extension = writer.getFileExtension();
        filenameField.setText(currentName + "." + extension);
        oldExtension = extension;
    }

    public void actionPerformed(ActionEvent e) {
        String currentName = filenameField.getText();
        if (currentName.trim().startsWith("|")) {
            return;
        }
        int cut = currentName.length() - oldExtension.length() - 1;
        if (cut >= 0 &&
                currentName.substring(cut).equalsIgnoreCase("." + oldExtension)) {
            filenameField.setText(currentName.substring(0, cut));
        }
        addExtension();
        if (e.getActionCommand().length() == 0) {
            filenameField.requestFocus();
        }
    }
}

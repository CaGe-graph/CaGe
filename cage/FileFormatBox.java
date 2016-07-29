package cage;

import cage.writer.CaGeWriter;
import cage.writer.WriterConfigurationHandler;
import cage.writer.WriterConfigurationHandlerFactory;
import cage.writer.WriterFactory;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.util.ArrayList;
import java.util.List;
import javax.swing.JComboBox;
import javax.swing.KeyStroke;
import javax.swing.text.JTextComponent;

import lisken.systoolbox.Systoolbox;

/**
 * Combobox for selecting a file format. This component is connected
 * with a textfield of which the name is altered based on the selected
 * file format.
 */
public class FileFormatBox extends JComboBox<String> {

    private final List<CaGeWriter> writers = new ArrayList<>();
    private final List<WriterConfigurationHandler> handlers = new ArrayList<>();
    private int dimension = 0;
    private JTextComponent filenameField;
    private String oldExtension;
    private boolean addExtension;
    
    private final ActionListener actionListener = new ActionListener() {
        @Override
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
    };

    public FileFormatBox(String variety, JTextComponent filenameField) {
        this(variety, filenameField, true);
    }

    public FileFormatBox(String variety, JTextComponent filenameField, boolean addExtension) {
        this.filenameField = filenameField;
        this.addExtension = addExtension;
        char firstChar = variety.charAt(0);
        if (Character.isDigit(firstChar)) {
            dimension = firstChar - '0';
        }
        for(String format : Systoolbox.stringToVector(
                CaGe.getCaGeProperty("CaGe.Writers." + variety))){
            CaGeWriter writer = createCaGeWriter(format);
            WriterConfigurationHandler handler = createConfigurationHandler(format);
            format = createCaGeWriter(format).getFormatName();
            addItem(format);
            writers.add(writer);
            handlers.add(handler);
        }
        addActionListener(actionListener);
        registerKeyboardAction(actionListener, "",
                KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0),
                WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);
        // new onActionFocusSwitcher(filenameField, this);
        oldExtension = getCaGeWriter().getFileExtension();
    }

    private CaGeWriter createCaGeWriter(String format) {
        CaGeWriter writer;
        writer = WriterFactory.createCaGeWriter(format);
        if (dimension > 0) {
            writer.setDimension(dimension);
        }
        return writer;
    }
    
    private WriterConfigurationHandler createConfigurationHandler(String format){
        return WriterConfigurationHandlerFactory.createWriterConfigurationHandler(format);
    }

    public CaGeWriter getCaGeWriter() {
        return writers.get(getSelectedIndex());
    }

    public WriterConfigurationHandler getConfigurationHandler() {
        return handlers.get(getSelectedIndex());
    }

    /**
     * Add an extension to the content of the corresponding text field.
     */
    public void addExtension() {
        String currentName = filenameField.getText();
        if (currentName.trim().startsWith("|") || !addExtension) {
            return;
        }
        CaGeWriter writer = getCaGeWriter();
        String extension = writer.getFileExtension();
        filenameField.setText(currentName + "." + extension);
        oldExtension = extension;
    }
}

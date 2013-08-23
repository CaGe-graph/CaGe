package cage;

import cage.writer.CaGeWriter;
import cage.writer.WriterFactory;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.text.MessageFormat;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

import lisken.uitoolbox.JTextComponentFocusSelector;

/**
 * Panel that allows the selection of a certain target (i.e. file or
 * pipe). Mainly used in {@link OutputPanel}.
 * 
 * @author nvcleemp
 */
public class TargetPanel extends JPanel {
    
    protected FileFormatBox fileFormat;
    protected JTextField fileName = new JTextField();
    private String targetTemplate;
    private boolean addExtension = true;

    /**
     * Private constructor because you should use the factory methods.
     * 
     * @param variety
     * @param nameMnemonic
     * @param formatMnemonic
     * @param prefix
     * @param addExtension
     * @param targetName
     */
    private TargetPanel(String variety, int nameMnemonic, int formatMnemonic, String prefix, boolean addExtension, String targetName) {
        super();
        fileFormat = new FileFormatBox(variety, fileName, addExtension);
        targetTemplate = prefix==null? "{0}" : prefix + "{0}";
        this.addExtension = addExtension;

        JLabel targetNameLabel = new JLabel(targetName);
        targetNameLabel.setLabelFor(fileName);
        targetNameLabel.setDisplayedMnemonic(nameMnemonic);
        targetNameLabel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        JLabel prefixLabel = new JLabel(prefix);
        prefixLabel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 0));
        JLabel fileFormatLabel = new JLabel("Format");
        fileFormatLabel.setLabelFor(fileFormat);
        fileFormatLabel.setDisplayedMnemonic(formatMnemonic);
        fileFormatLabel.setBorder(BorderFactory.createEmptyBorder(0, 20, 0, 10));

        fileFormat.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        fileFormat.setMaximumSize(fileFormat.getPreferredSize());

        fileName.setColumns(15);
        fileName.setMaximumSize(fileName.getPreferredSize());
        fileName.addActionListener(new FilePanelActionListener());

        new JTextComponentFocusSelector(fileName);
        
        setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
        setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 10));
        
        add(targetNameLabel, null);
        add(prefixLabel, null);
        add(fileName, null);
        add(Box.createHorizontalGlue());
        add(fileFormatLabel, null);
        add(fileFormat, null);
    }

    public void addActionListener(ActionListener l) {
        listenerList.add(ActionListener.class, l);
    }

    public void removeActionListener(ActionListener l) {
        listenerList.remove(ActionListener.class, l);
    }

    protected void fireActionPerformed() {
        // Guaranteed to return a non-null array
        Object[] listeners = listenerList.getListenerList();
        ActionEvent event = new ActionEvent(this, ActionEvent.ACTION_PERFORMED, getTargetName());
        // Process the listeners last to first, notifying
        // those that are interested in this event
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i] == ActionListener.class) {
                ((ActionListener) listeners[i + 1]).actionPerformed(event);
            }
        }
    }

    public CaGeWriter getCaGeWriter() {
        return fileFormat.getCaGeWriter();
    }

    /**
     *
     * @return The name of the target
     */
    public String getTargetName(){
        return MessageFormat.format(targetTemplate, fileName.getText());
    }

    public void setTargetName(String fileName) {
        this.fileName.setText(fileName);
        if(addExtension)
            fileFormat.addExtension();
    }

    /**
     * A private listener class. This class is private to make sure it can
     * only be used to listen to the textfield of this file panel.
     */
    protected class FilePanelActionListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {
            fireActionPerformed();
        }
    }

    /**
     * Convenience method for the creation of a <code>TargetPanel</code>
     * for a file.
     *
     * @param variety The variety that for which this panel is intended.
     * @param nameMnemonic The mnemonic for the targetname textfield
     * @param formatMnemonic The mnemonic for the format combobox.
     * @return A new <code>TargetPanel</code>.
     *
     * @see WriterFactory
     */
    public static TargetPanel creatFilePanel(String variety, int nameMnemonic, int formatMnemonic){
        return new TargetPanel(variety, nameMnemonic, formatMnemonic, null, true, "Filename");
    }

    /**
     * Convenience method for the creation of a <code>TargetPanel</code>
     * for a pipe.
     *
     * @param variety The variety that for which this panel is intended.
     * @param nameMnemonic The mnemonic for the targetname textfield
     * @param formatMnemonic The mnemonic for the format combobox.
     * @return A new <code>TargetPanel</code>.
     *
     * @see WriterFactory
     */
    public static TargetPanel creatPipePanel(String variety, int nameMnemonic, int formatMnemonic){
        return new TargetPanel(variety, nameMnemonic, formatMnemonic, "|", false, "Pipename");
    }
}

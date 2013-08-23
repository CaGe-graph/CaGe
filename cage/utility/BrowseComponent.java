package cage.utility;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

/**
 * Utility component that allows the selection of a file by entering the path
 * or by browsing using a file chooser.
 * 
 * @author nvcleemp
 */
public class BrowseComponent extends JPanel {
    
    //
    private final JTextField fileTextField = new JTextField();
    
    //
    private final JFileChooser fileChooser;
    
    //
    private final Action browseAction;

    public BrowseComponent(File startFile) {
        this(startFile, "Browse", FileSelectionMode.FILES_AND_DIRECTORIES);
    }
    
    public BrowseComponent(File startFile, boolean editable) {
        this(startFile, "Browse", FileSelectionMode.FILES_AND_DIRECTORIES, editable);
    }

    public BrowseComponent(File startFile, String text, FileSelectionMode mode) {
        this(startFile, text, mode, false);
    }
    
    public BrowseComponent(File startFile, String text, FileSelectionMode mode, boolean editable) {
        fileTextField.setEditable(editable);
        try {
            fileTextField.setText(startFile.getCanonicalPath());
        } catch (IOException ex) {
            Debug.reportException(ex);
        }
        fileChooser = new JFileChooser(startFile);
        fileChooser.setMultiSelectionEnabled(false);
        mode.configureBrowseComponent(this);
        
        browseAction = new AbstractAction(text) {

                @Override
                public void actionPerformed(ActionEvent e) {
                    if (fileChooser.showOpenDialog(BrowseComponent.this) == JFileChooser.APPROVE_OPTION) {
                        try {
                            fileTextField.setText(fileChooser.getSelectedFile().getCanonicalPath());
                        } catch (IOException ex) {
                            Debug.reportException(ex);
                        }
                    }
                }
            };
        initGui();
        fileTextField.getDocument().addDocumentListener(new SingleActionDocumentLister() {

            @Override
            public void update() {
                fireStateChanged();
            }
        });
    }

    public File getFile() {
        return new File(fileTextField.getText());
    }
    
    public void setFile(File file) {
        if(file!=null){
            fileTextField.setText(file.getPath());
        }
    }

    private void initGui() {
        add(fileTextField, BorderLayout.CENTER);
        add(new JButton(browseAction), BorderLayout.EAST);
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        fileTextField.setEnabled(enabled);
        browseAction.setEnabled(enabled);
    }
    
    public void setManualEditable(boolean editable) {
        fileTextField.setEditable(editable);
    }
    
    public boolean isManualEditable(){
        return fileTextField.isEditable();
    }
    
    private List<ChangeListener> listeners = new ArrayList<>();
    private ChangeEvent event = new ChangeEvent(this);
    
    public void addChangeListener(ChangeListener listener){
        listeners.add(listener);
    }
    
    public void removeChangeListener(ChangeListener listener){
        listeners.remove(listener);
    }
    
    private void fireStateChanged(){
        for (ChangeListener changeListener : listeners) {
            changeListener.stateChanged(event);
        }
    }
    
    public static enum FileSelectionMode{
        FILES_ONLY(JFileChooser.FILES_ONLY), DIRECTORIES_ONLY(JFileChooser.DIRECTORIES_ONLY), FILES_AND_DIRECTORIES(JFileChooser.FILES_AND_DIRECTORIES);

        private final int mode;
        
        private FileSelectionMode(int mode){
            this.mode = mode;
        }
        
        public void configureBrowseComponent(BrowseComponent browseComponent){
            browseComponent.fileChooser.setFileSelectionMode(mode);
        }
    }
    
}

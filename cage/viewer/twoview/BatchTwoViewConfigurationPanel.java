package cage.viewer.twoview;

import cage.utility.BrowseComponent;

import cage.utility.SingleActionDocumentLister;
import java.awt.EventQueue;
import java.awt.FlowLayout;
import javax.swing.AbstractListModel;
import javax.swing.ComboBoxModel;
import javax.swing.JComboBox;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

/**
 *
 * @author nvcleemp
 */
public class BatchTwoViewConfigurationPanel extends JPanel{
    
    private final BatchTwoViewModel batchTwoViewModel;
    private JTextField filenameField;
    private BrowseComponent folderSelector;
    private SaverComboBoxModel comboBoxModel;
    
    private BatchTwoViewConfigurationListener listener = new BatchTwoViewConfigurationListener() {

        @Override
        public void fileNameTemplateChanged() {
            filenameField.setText(batchTwoViewModel.getFileNameTemplate());
        }

        @Override
        public void folderChanged() {
            folderSelector.setFile(batchTwoViewModel.getFolder());
        }

        @Override
        public void saverChanged(TwoViewSavers oldSaver, TwoViewSavers newSaver) {
            //do nothing: the combo box model takes care of this
        }
    };

    public BatchTwoViewConfigurationPanel(BatchTwoViewModel batchTwoViewModel) {
        super(new FlowLayout(FlowLayout.LEFT, 10, 5));
        this.batchTwoViewModel = batchTwoViewModel;
        batchTwoViewModel.addBatchTwoViewConfigurationListener(listener);
        comboBoxModel = new SaverComboBoxModel();
        initGui();
    }
    
    private void initGui(){
        folderSelector = new BrowseComponent(batchTwoViewModel.getFolder(), true);
        filenameField = new JTextField(batchTwoViewModel.getFileNameTemplate(), 20);
        filenameField.getDocument().addDocumentListener(new SingleActionDocumentLister() {

            @Override
            public void update() {
                EventQueue.invokeLater(new Runnable() {
                    @Override
                    public void run() {
                        batchTwoViewModel.setFileNameTemplate(filenameField.getText());
                    }
                });
                
            }
        });
        folderSelector.addChangeListener(new ChangeListener() {

            @Override
            public void stateChanged(ChangeEvent e) {
                batchTwoViewModel.setFolder(folderSelector.getFile());
            }
        });
        
        add(folderSelector);
        add(filenameField);
        add(new JComboBox<>(comboBoxModel));
    }
    
    public void setFilenameTemplate(String template){
        if(!filenameField.getText().equals(template)){
            filenameField.setText(template);
        }
    }
    
    private class SaverComboBoxModel extends AbstractListModel<TwoViewSavers> implements ComboBoxModel<TwoViewSavers>, BatchTwoViewConfigurationListener {
        

        @Override
        public void setSelectedItem(Object saver) {
            if(saver instanceof TwoViewSavers && saver != getSelectedItem()){
                batchTwoViewModel.setSaver((TwoViewSavers)saver);
                fireContentsChanged(this, -1, -1);
            } else {
                throw new IllegalArgumentException("Illegal saver");
            }
        }

        @Override
        public TwoViewSavers getSelectedItem() {
            return batchTwoViewModel.getSaver();
        }

        @Override
        public int getSize() {
            return TwoViewSavers.values().length;
        }

        @Override
        public TwoViewSavers getElementAt(int index) {
            return TwoViewSavers.values()[index];
        }

        @Override
        public void fileNameTemplateChanged() {
            //do nothing
        }

        @Override
        public void folderChanged() {
            //do nothing
        }

        @Override
        public void saverChanged(TwoViewSavers oldSaver, TwoViewSavers newSaver) {
            fireContentsChanged(this, -1, -1);
        }
    }
}

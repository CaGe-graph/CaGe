package cage.generator;

import java.awt.Frame;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JToggleButton;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.uitoolbox.PushButtonDecoration;

/**
 * Dialog that can be used to select allowed symmetries for the generation of 
 * fullerenes. A ChangeListener can be registered with this dialog to be notified
 * when the selected symmetries change. Pressing cancel in this dialog will reset
 * the selection to the previous selection. The notification about the changes
 * are already sent while the dialog is shown and cancelling will lead to more 
 * notifications.
 * 
 * @author nvcleemp
 */
public class SymmetriesDialog extends JDialog{
    
    
    private static final String[] SYMMETRY = new String[]{
        "C1", "C2", "Ci", "Cs",
        "C3", "D2", "S4", "C2v",
        "C2h", "D3", "S6", "C3v",
        "C3h", "D2h", "D2d", "D5",
        "D6", "D3h", "D3d", "T",
        "D5h", "D5d", "D6h", "D6d",
        "Td", "Th", "I", "Ih"
    };
    private static final int SYMMETRIES_COUNT = SYMMETRY.length;
    private static final int SYMMETRIES_ROWS = 4;
    
    private SymmetriesModel selectedSymmetriesModel = new SymmetriesModel();
    private Set<String> storedState = new HashSet<String>();

    public SymmetriesDialog(Frame owner, String title, boolean modal) {
        super(owner, title, modal);
        initGui();
        /* Listeners can register with this dialog to be notified when the selected
         * this listener makes sure that these listeners are notified.
         */
        selectedSymmetriesModel.addSymmetriesModelListener(new SingleActionSymmetriesModelListener() {
            @Override
            public void selectionChanged() {
                fireStateChanged();
            }
        });
    }
    
    // --------------- GUI ---------------------

    private void initGui() {        
        JPanel contentPane = new JPanel();
        contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
        contentPane.add(buildSymmetriesPanel());
        contentPane.add(Box.createVerticalStrut(10));
        contentPane.add(buildButtonsPanel());
        contentPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        setContentPane(contentPane);
        
        pack();
    }

    private JPanel buildSymmetriesPanel() {
        JPanel symmetriesPanel = new JPanel(new GridLayout(SYMMETRIES_ROWS, 0, 10, 10));
        symmetriesPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        int symmCols = (SYMMETRIES_COUNT - 1) / SYMMETRIES_ROWS + 1;
        for (int i = 0; i < SYMMETRIES_COUNT; ++i) {
            int k = (i % symmCols) * SYMMETRIES_ROWS + (i / symmCols);
            symmetriesPanel.add(getSymmetryButton(SYMMETRY[k]));
            selectedSymmetriesModel.addSymmetry(SYMMETRY[k]);
        }
        return symmetriesPanel;
    }

    private JPanel buildButtonsPanel() {
        //select all button
        JButton symmetriesAllButton = new JButton("Set all");
        symmetriesAllButton.setMnemonic(KeyEvent.VK_S);
        symmetriesAllButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                for (String symmetry : SYMMETRY) {
                    selectedSymmetriesModel.addSymmetry(symmetry);
                }
            }
        });
        
        //clear all button
        JButton symmetriesNoneButton = new JButton("Clear all");
        symmetriesNoneButton.setMnemonic(KeyEvent.VK_C);
        symmetriesNoneButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                selectedSymmetriesModel.clear();
            }
        });
        
        //OK button
        final JButton okButton = new JButton("Ok");
        okButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                setVisible(false);
            }
        });
        selectedSymmetriesModel.addSymmetriesModelListener(new SingleActionSymmetriesModelListener() {
            @Override
            public void selectionChanged() {
                okButton.setEnabled(!selectedSymmetriesModel.isEmpty());
            }
        });
        
        //cancel button
        JButton cancelButton = new JButton("Cancel");
        cancelButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                //restore the state of the selection and hide the dialog
                restoreState();
                setVisible(false);
            }
        });
        
        //set special buttons for dialog
        getRootPane().setDefaultButton(okButton);
        
        //construct panel
        JPanel buttonsPanel = new JPanel();
        buttonsPanel.add(symmetriesAllButton);
        buttonsPanel.add(Box.createHorizontalStrut(5));
        buttonsPanel.add(symmetriesNoneButton);
        buttonsPanel.add(Box.createHorizontalStrut(5));
        buttonsPanel.add(okButton);
        buttonsPanel.add(Box.createHorizontalStrut(5));
        buttonsPanel.add(cancelButton);
        return buttonsPanel;
    }
    
    private JToggleButton getSymmetryButton(final String symmetry){
        final JToggleButton button = new JToggleButton(symmetry);
        button.setBorder(BorderFactory.createEmptyBorder(3, 7, 3, 7));
        PushButtonDecoration.decorate(button, true);
        
        //add action listener that (de)selects the symmetry in the model when the button is pressed
        button.addActionListener(new ActionListener() {

            @Override
            public void actionPerformed(ActionEvent e) {
                if(button.isSelected()){
                    selectedSymmetriesModel.addSymmetry(symmetry);
                } else {
                    selectedSymmetriesModel.removeSymmetry(symmetry);
                }
            }
        });
        
        //add listener for model that keeps the button's selection state in sync with the model
        selectedSymmetriesModel.addSymmetriesModelListener(new SymmetriesModelListener() {

            @Override
            public void symmetryAdded(String localSymmetry) {
                if(localSymmetry.equals(symmetry)){
                    button.setSelected(true);
                }
            }

            @Override
            public void symmetryRemoved(String localSymmetry) {
                if(localSymmetry.equals(symmetry)){
                    button.setSelected(false);
                }
            }

            @Override
            public void selectedSymmetriesChanged() {
                button.setSelected(selectedSymmetriesModel.containsSymmetry(symmetry));
            }
        });
        
        return button;
    }
    
    // --------------- save and restore ---------------------

    @Override
    public void setVisible(boolean visible) {
        if(visible){
            saveState();
        }
        super.setVisible(visible);
    }
    
    private void saveState(){
        storedState.clear();
        for (String symmetry : selectedSymmetriesModel.getSelectedSymmetries()) {
            storedState.add(symmetry);
        }
    }
    
    private void restoreState(){
        selectedSymmetriesModel.setSelectedSymmetries(storedState);
    }
    
    // --------------- Public methods ---------------------
    
    public boolean areAllSymmetriesSelected(){
        return selectedSymmetriesModel.size() == SYMMETRY.length;
    }

    public Set<String> getSelectedSymmetries() {
        return selectedSymmetriesModel.getSelectedSymmetries();
    }
    
    private List<ChangeListener> listeners = new ArrayList<ChangeListener>();
    
    public void addChangeListener(ChangeListener l){
        listeners.add(l);
    }
    
    public void removeChangeListener(ChangeListener l){
        listeners.remove(l);
    }
    
    private ChangeEvent changeEvent = new ChangeEvent(this);
    
    private void fireStateChanged(){
        for (ChangeListener l : listeners) {
            l.stateChanged(changeEvent);
        }
    }
    
    /**
     * Wrapper model around a Set to keep track of which symmetries are selected
     */
    private static class SymmetriesModel{
        private Set<String> selectedSymmetries = new HashSet<String>();
        
        public void addSymmetry(String symmetry){
            if(symmetry != null && selectedSymmetries.add(symmetry)){
                fireSymmetryAdded(symmetry);
            }
        }
        
        public void removeSymmetry(String symmetry){
            if(selectedSymmetries.remove(symmetry)){
                fireSymmetryRemoved(symmetry);
            }
        }
        
        public void clear(){
            if(!selectedSymmetries.isEmpty()){
                selectedSymmetries.clear();
                fireSelectedSymmetriesChanged();
            }
        }
        
        public void setSelectedSymmetries(Set<String> set){
            selectedSymmetries.clear();
            selectedSymmetries.addAll(set);
            fireSelectedSymmetriesChanged();
        }
        
        public boolean containsSymmetry(String symmetry){
            return selectedSymmetries.contains(symmetry);
        }
        
        public boolean isEmpty(){
            return selectedSymmetries.isEmpty();
        }
        
        public int size(){
            return selectedSymmetries.size();
        }
        
        public Set<String> getSelectedSymmetries(){
            return new HashSet<String>(selectedSymmetries);
        }
        
        private List<SymmetriesModelListener> listeners = new ArrayList<SymmetriesModelListener>();
        
        public void addSymmetriesModelListener(SymmetriesModelListener listener){
            listeners.add(listener);
        }
        
        public void removeSymmetriesModelListener(SymmetriesModelListener listener){
            listeners.remove(listener);
        }
        
        private void fireSymmetryAdded(String symmetry){
            for (SymmetriesModelListener l : listeners) {
                l.symmetryAdded(symmetry);
            }
        }
        
        private void fireSymmetryRemoved(String symmetry){
            for (SymmetriesModelListener l : listeners) {
                l.symmetryRemoved(symmetry);
            }
        }
        
        private void fireSelectedSymmetriesChanged(){
            for (SymmetriesModelListener l : listeners) {
                l.selectedSymmetriesChanged();
            }
        }
    }
    
    /**
     * Listener for SymmetriesModel
     */
    private static interface SymmetriesModelListener {
        
        /**
         * Called when a symmetry was added to the model.
         * @param symmetry The symmetry that was added.
         */
        void symmetryAdded(String symmetry);
        
        /**
         * Called when a symmetry was removed from the model.
         * @param symmetry The symmetry that was removed.
         */
        void symmetryRemoved(String symmetry);
        
        /**
         * Called when the selection changed in a way other than
         * an atomic addition or removal.
         */
        void selectedSymmetriesChanged();
    }
    
    /**
     * Abstract implementation of SymmetriesModelListener that calls a single method
     * in all cases.
     */
    private static abstract class SingleActionSymmetriesModelListener implements SymmetriesModelListener {

        @Override
        public void symmetryAdded(String symmetry) {
            selectionChanged();
        }

        @Override
        public void symmetryRemoved(String symmetry) {
            selectionChanged();
        }

        @Override
        public void selectedSymmetriesChanged() {
            selectionChanged();
        }
        
        public abstract void selectionChanged();
        
    }
    
}

package cage.writer;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.FocusAdapter;
import java.awt.event.FocusEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

/**
 * Configuration handler for {@link SpinputWriter} objects.
 * 
 * @author nvcleemp
 */
public class SpinputWriterConfigurationHandler implements WriterConfigurationHandler {

    private final SpinputWriterConfigurationModel model;

    public SpinputWriterConfigurationHandler() {
        model = new SpinputWriterConfigurationModel();
    }
    
    @Override
    public JPanel getConfigurationPanel() {
        return new SpinputWriterConfigurationPanel(model);
    }

    @Override
    public void configureWriter(CaGeWriter writer) {
        if(!(writer instanceof SpinputWriter)){
            throw new RuntimeException("Illegal class of writer for this configuration handler.");
        }
        SpinputWriter sWriter = (SpinputWriter)writer;
        sWriter.setUseSingleElementRule(model.useSingleElementRule());
        if(model.useSingleElementRule()){
            sWriter.setSingleElement(
                    model.getSingleElementNumber(),
                    model.getSingleElementName());
        }
        sWriter.setScalingFactor(model.getScalingFactor());
    }
    
    private static class SpinputWriterConfigurationModel {
        private boolean useSingleElementRule = false;
        private int singleElementNumber = 6;
        private String singleElementName = "C";

        private float scalingFactor = 1.0f;
        
        private final List<SpinputWriterConfigurationModelListener> listeners =
                new ArrayList<>();

        public boolean useSingleElementRule() {
            return useSingleElementRule;
        }

        public void setUseSingleElementRule(boolean useSingleElementRule) {
            if(this.useSingleElementRule != useSingleElementRule){
                this.useSingleElementRule = useSingleElementRule;
                fireValuesChanged();
            }
        }

        public int getSingleElementNumber() {
            return singleElementNumber;
        }

        public void setSingleElementNumber(int singleElementNumber) {
            if(this.singleElementNumber != singleElementNumber){
                this.singleElementNumber = singleElementNumber;
                fireValuesChanged();
            }
        }

        public String getSingleElementName() {
            return singleElementName;
        }

        public void setSingleElementName(String singleElementName) {
            if(singleElementName != null &&
                    !this.singleElementName.equals(singleElementName)){
                this.singleElementName = singleElementName;
                fireValuesChanged();
            }
        }

        public float getScalingFactor() {
            return scalingFactor;
        }

        public void setScalingFactor(float scalingFactor) {
            if(this.scalingFactor != scalingFactor){
                this.scalingFactor = scalingFactor;
                fireValuesChanged();
            }
        }
        
        public void addListener(SpinputWriterConfigurationModelListener l){
            listeners.add(l);
        }
        
        public void removeListener(SpinputWriterConfigurationModelListener l){
            listeners.remove(l);
        }
        
        private void fireValuesChanged(){
            for (SpinputWriterConfigurationModelListener l : listeners) {
                l.valuesChanged();
            }
        }
    }
    
    private static interface SpinputWriterConfigurationModelListener {
        void valuesChanged();
    }
    
    private static class SpinputWriterConfigurationPanel extends JPanel {
        
        private final SpinputWriterConfigurationModel model;

        public SpinputWriterConfigurationPanel(SpinputWriterConfigurationModel model) {
            this.model = model;
            setupGUI();
        }
        
        private void setupGUI(){
            //checkbox for single element rule
            final JCheckBox singleElementBox = new JCheckBox(
                            "use single element",
                            model.useSingleElementRule());
            singleElementBox.addChangeListener(new ChangeListener() {
                @Override
                public void stateChanged(ChangeEvent e) {
                    model.setUseSingleElementRule(singleElementBox.isSelected());
                }
            });
            model.addListener(new SpinputWriterConfigurationModelListener() {
                @Override
                public void valuesChanged() {
                    singleElementBox.setSelected(model.useSingleElementRule());
                }
            });
            
            //text field for single element name
            final JTextField nameField = new JTextField(model.getSingleElementName());
            nameField.addFocusListener(new FocusAdapter() {
                @Override
                public void focusLost(FocusEvent e) {
                    model.setSingleElementName(nameField.getText());
                }
            });
            model.addListener(new SpinputWriterConfigurationModelListener() {
                @Override
                public void valuesChanged() {
                    nameField.setText(model.getSingleElementName());
                }
            });
            
            //text field for single element number
            final JFormattedTextField numberField = 
                    new JFormattedTextField(model.getSingleElementNumber());
            numberField.addPropertyChangeListener("value", new PropertyChangeListener() {
                @Override
                public void propertyChange(PropertyChangeEvent evt) {
                    model.setSingleElementNumber((Integer)(numberField.getValue()));
                }
            });
            model.addListener(new SpinputWriterConfigurationModelListener() {
                @Override
                public void valuesChanged() {
                    numberField.setValue(model.getSingleElementNumber());
                }
            });
            
            //text field for scaling factor
            final JFormattedTextField scalingField = 
                    new JFormattedTextField(model.getScalingFactor());
            scalingField.addPropertyChangeListener("value", new PropertyChangeListener() {
                @Override
                public void propertyChange(PropertyChangeEvent evt) {
                    model.setScalingFactor((Float)(scalingField.getValue()));
                }
            });
            model.addListener(new SpinputWriterConfigurationModelListener() {
                @Override
                public void valuesChanged() {
                    scalingField.setValue(model.getScalingFactor());
                }
            });
            
            //some labels
            final JLabel nameLabel = new JLabel("name:");
            final JLabel numberLabel = new JLabel("atomic number:");
            
            //place components            
            setLayout(new GridBagLayout());
            GridBagConstraints gbc = new GridBagConstraints();
            gbc.gridx = gbc.gridy = 0;
            gbc.fill = GridBagConstraints.HORIZONTAL;
            gbc.gridwidth = GridBagConstraints.REMAINDER;
            gbc.insets = new Insets(5, 5, 5, 5);
            gbc.weightx = 1;
            add(singleElementBox, gbc);
            gbc.gridy++;
            gbc.gridwidth = 1;
            gbc.weightx = 0;
            add(nameLabel, gbc);
            gbc.gridy++;
            add(numberLabel, gbc);
            gbc.gridy++;
            add(new JLabel("scaling factor:"), gbc);
            gbc.gridy-=2;
            gbc.gridx++;
            gbc.weightx = 1;
            gbc.gridwidth = GridBagConstraints.REMAINDER;
            add(nameField, gbc);
            gbc.gridy++;
            add(numberField, gbc);
            gbc.gridy++;
            add(scalingField, gbc);
            
            //disable components for single element
            final List<JComponent> components = new ArrayList<>();
            Collections.addAll(components, nameField, numberField, nameLabel, numberLabel);
            for (JComponent component : components) {
                component.setEnabled(model.useSingleElementRule());
            }
            //and add listener to enable or disable them based on the model
            model.addListener(new SpinputWriterConfigurationModelListener() {
                @Override
                public void valuesChanged() {
                    for (JComponent component : components) {
                        component.setEnabled(model.useSingleElementRule());
                    }
                }
            });
        }
        
    }
}

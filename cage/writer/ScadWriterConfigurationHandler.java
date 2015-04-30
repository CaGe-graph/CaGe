package cage.writer;

import cage.CaGe;
import cage.writer.scad.BallStickType;
import cage.writer.scad.HighScadResolution;
import cage.writer.scad.LowScadResolution;
import cage.writer.scad.MediumScadResolution;
import cage.writer.scad.ScadResolution;
import cage.writer.scad.ScadType;
import cage.writer.scad.WireFrameType;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import javax.swing.AbstractAction;
import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;

/**
 * Configuration handler for {@link ScadWriter} objects.
 * 
 * @author nvcleemp
 */
public class ScadWriterConfigurationHandler implements WriterConfigurationHandler {

    private final Map<String, ScadType> types = new HashMap<>();
    private final Map<String, ScadResolution> resolutions = new HashMap<>();
    private final List<String> typeOrder = new ArrayList<>();
    private final List<String> resolutionOrder = new ArrayList<>();
    private final ScadWriterConfigurationModel model;

    public ScadWriterConfigurationHandler() {
        //construct all possible types
        types.put(BallStickType.class.getName(), new BallStickType());
        types.put(WireFrameType.class.getName(), new WireFrameType());
        typeOrder.add(BallStickType.class.getName());
        typeOrder.add(WireFrameType.class.getName());
        
        //construct all possible resolutions
        ScadResolution low = new LowScadResolution();
        ScadResolution medium = new MediumScadResolution();
        ScadResolution high = new HighScadResolution();
        resolutions.put(low.getName(), low);
        resolutions.put(medium.getName(), medium);
        resolutions.put(high.getName(), high);
        resolutionOrder.add(low.getName());
        resolutionOrder.add(medium.getName());
        resolutionOrder.add(high.getName());
        
        //create model
        model = new ScadWriterConfigurationModel(types, resolutions);
    }
    
    @Override
    public JPanel getConfigurationPanel() {
        return new ScadWriterConfigurationPanel(model,typeOrder, types, resolutionOrder);
    }

    @Override
    public void configureWriter(CaGeWriter writer) {
        if(!(writer instanceof ScadWriter)){
            throw new RuntimeException("Illegal class of writer for this configuration handler.");
        }
        ScadWriter sWriter = (ScadWriter)writer;
        sWriter.setType(model.getResolution().configure(model.getType()));
    }
    
    private static class ScadWriterConfigurationModel {
        
        private ScadType type;
        private ScadResolution resolution;
        
        private final Map<String, ScadType> types;
        private final Map<String, ScadResolution> resolutions;

        public ScadWriterConfigurationModel(Map<String, ScadType> types, Map<String, ScadResolution> resolutions) {
            this.types = types;
            this.resolutions = resolutions;
            
            type = types.get(CaGe.getCaGeProperty("Scad.type", BallStickType.class.getName()));
            if(type == null){
                type = types.get(BallStickType.class.getName());
            }
            
            resolution = resolutions.get(CaGe.getCaGeProperty("Scad.resolution", "medium"));
            if(resolution == null){
                resolution = resolutions.get("medium");
            }
        }
        
        public ScadType getType(){
            return type;
        }

        public ScadResolution getResolution() {
            return resolution;
        }
        
        public void setType(String type){
            if(!this.type.getClass().getName().equals(type)){
                ScadType newType = types.get(type);
                if(newType!=null){
                    this.type = newType;
                    fireValuesChanged();
                }
            }
        }
        
        public void setResolution(String resolution){
            if(!this.resolution.getName().equals(resolution)){
                ScadResolution newResolution = resolutions.get(resolution);
                if(newResolution!=null){
                    this.resolution = newResolution;
                    fireValuesChanged();
                }
            }
        }
        
        private final List<ScadWriterConfigurationModelListener> listeners =
                new ArrayList<>();
        
        public void addListener(ScadWriterConfigurationModelListener l){
            listeners.add(l);
        }
        
        public void removeListener(ScadWriterConfigurationModelListener l){
            listeners.remove(l);
        }
        
        private void fireValuesChanged(){
            for (ScadWriterConfigurationModelListener l : listeners) {
                l.valuesChanged();
            }
        }
    }
    
    private static interface ScadWriterConfigurationModelListener {
        void valuesChanged();
    }
    
    private static class ScadWriterConfigurationPanel extends JPanel {
        
        private final ScadWriterConfigurationModel model;

        public ScadWriterConfigurationPanel(ScadWriterConfigurationModel model,
                List<String> typeOrder, Map<String, ScadType> types,
                List<String> resolutionOrder) {
            this.model = model;
            setupGUI(typeOrder, types, resolutionOrder);
        }
        
        private void setupGUI(List<String> typeOrder, Map<String, ScadType> types,
                List<String> resolutionOrder){
            
            //some labels
            final JLabel resolutionLabel = new JLabel("resolution:");
            
            //some insets
            Insets labelInsets = new Insets(5, 5, 5, 5);
            Insets radioButtonInsets = new Insets(0, 20, 0, 5);
            
            //place components            
            setLayout(new GridBagLayout());
            GridBagConstraints gbc = new GridBagConstraints();
            gbc.gridx = gbc.gridy = 0;
            gbc.fill = GridBagConstraints.HORIZONTAL;
            gbc.gridwidth = GridBagConstraints.REMAINDER;
            gbc.insets = labelInsets;
            add(new JLabel("type:"), gbc);
            gbc.insets = radioButtonInsets;
            final ButtonGroup typeGroup = new ButtonGroup();
            for (final String type : typeOrder) {
                gbc.gridy++;
                final JRadioButton typeButton = new JRadioButton(
                        new AbstractAction(types.get(type).getName()) {
                    @Override
                    public void actionPerformed(ActionEvent e) {
                        model.setType(type);
                    }
                });
                typeButton.setSelected(model.getType().getClass().getName().equals(type));
                model.addListener(new ScadWriterConfigurationModelListener() {
                    @Override
                    public void valuesChanged() {
                        typeButton.setSelected(model.getType().getClass().getName().equals(type));
                    }
                });
                add(typeButton, gbc);
                typeGroup.add(typeButton);
            }
            gbc.gridy++;
            gbc.insets = labelInsets;
            add(resolutionLabel, gbc);
            gbc.insets = radioButtonInsets;
            final ButtonGroup resolutionGroup = new ButtonGroup();
            for (final String resolution : resolutionOrder) {
                gbc.gridy++;
                final JRadioButton resolutionButton = new JRadioButton(
                        new AbstractAction(resolution) {
                    @Override
                    public void actionPerformed(ActionEvent e) {
                        model.setResolution(resolution);
                    }
                });
                resolutionButton.setSelected(model.getResolution().getName().equals(resolution));
                model.addListener(new ScadWriterConfigurationModelListener() {
                    @Override
                    public void valuesChanged() {
                        resolutionButton.setSelected(model.getResolution().getName().equals(resolution));
                    }
                });
                add(resolutionButton, gbc);
                resolutionGroup.add(resolutionButton);
            }
            
            
            //disable components for resolution if needed
            resolutionLabel.setEnabled(model.getType().hasResolution());
            for (Enumeration<AbstractButton> e = resolutionGroup.getElements(); e.hasMoreElements();){
                e.nextElement().setEnabled(model.getType().hasResolution());
            }
            
            //and add listener to enable or disable them based on the model
            model.addListener(new ScadWriterConfigurationModelListener() {
                @Override
                public void valuesChanged() {
                    resolutionLabel.setEnabled(model.getType().hasResolution());
                    for (Enumeration<AbstractButton> e = resolutionGroup.getElements(); e.hasMoreElements();){
                        e.nextElement().setEnabled(model.getType().hasResolution());
                    }
                }
            });
            
            //set insets for this panel
            this.setBorder(BorderFactory.createEmptyBorder(10, 20, 10, 20));
        }
        
    }
}

package cage.writer;

import javax.swing.JPanel;

/**
 * An interface for objects that configure a specific writer.
 * 
 * @author nvcleemp
 */
public interface WriterConfigurationHandler {
    
    public JPanel getConfigurationPanel();
    
    public void configureWriter(CaGeWriter writer);
}

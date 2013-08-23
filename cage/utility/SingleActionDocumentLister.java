package cage.utility;

import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

/**
 * Implementation of DocumentListener that always performs the same action.
 * 
 * @author nvcleemp
 */
public abstract class SingleActionDocumentLister implements DocumentListener {

    @Override
    public void insertUpdate(DocumentEvent e) {
        update();
    }

    @Override
    public void removeUpdate(DocumentEvent e) {
        update();
    }

    @Override
    public void changedUpdate(DocumentEvent e) {
        update();
    }
    
    public abstract void update();
    
}

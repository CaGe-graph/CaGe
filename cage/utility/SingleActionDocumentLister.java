package cage.utility;

import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

/**
 * Implementation of DocumentListener that always performs the same action.
 * 
 * @author nvcleemp
 */
public abstract class SingleActionDocumentLister implements DocumentListener {

    public void insertUpdate(DocumentEvent e) {
        update();
    }

    public void removeUpdate(DocumentEvent e) {
        update();
    }

    public void changedUpdate(DocumentEvent e) {
        update();
    }
    
    public abstract void update();
    
}

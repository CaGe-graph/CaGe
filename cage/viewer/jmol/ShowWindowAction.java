package cage.viewer.jmol;

import java.awt.event.ActionEvent;
import javax.swing.AbstractAction;
import javax.swing.JFrame;

/**
 *
 * @author nvcleemp
 */
public class ShowWindowAction extends AbstractAction{
    
    private JFrame window;

    public ShowWindowAction(String title, JFrame window) {
        super(title);
        this.window = window;
    }

    public void actionPerformed(ActionEvent e) {
        if(!window.isVisible())
            window.setVisible(true);
    }

}

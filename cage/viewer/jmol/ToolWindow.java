package cage.viewer.jmol;

import java.awt.Frame;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.JFrame;
import javax.swing.JPanel;

/**
 *
 * @author nvcleemp
 */
public class ToolWindow extends JFrame {

    public ToolWindow(JPanel toolPanel, JFrame parentWindow) {
        this(toolPanel, parentWindow, false);
    }
    
    public ToolWindow(JPanel toolPanel, JFrame parentWindow, boolean maximize) {
        super(toolPanel.getName());
        parentWindow.addWindowListener(new ParentWindowListener());
        add(toolPanel);
        setAlwaysOnTop(true);
        pack();
        if(maximize)
            setExtendedState(Frame.MAXIMIZED_BOTH);
    }

    private class ParentWindowListener extends WindowAdapter{

        @Override
        public void windowClosing(WindowEvent e) {
            ToolWindow.this.setVisible(false);
        }

        @Override
        public void windowClosed(WindowEvent e) {
            ToolWindow.this.setVisible(false);
            ToolWindow.this.dispose();
        }

    }

}

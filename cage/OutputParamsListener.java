package cage;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import lisken.uitoolbox.Wizard;

/**
 *
 */
public class OutputParamsListener implements ActionListener {

    OutputPanel outputPanel;

    public OutputParamsListener(OutputPanel outputPanel) {
        this.outputPanel = outputPanel;
    }

    public void actionPerformed(ActionEvent e) {
        String cmd = e.getActionCommand();
        if (cmd.equals(Wizard.SHOWING)) {
            outputPanel.showing();
        } else if (cmd.equals(Wizard.NEXT)) {
            CaGeStarter starter = new CaGeStarter(outputPanel);
            starter.start();
        } else {
            CaGe.listener.actionPerformed(e);
        }
    }
}

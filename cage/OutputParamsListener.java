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

    @Override
    public void actionPerformed(ActionEvent e) {
        String cmd = e.getActionCommand();
        if (cmd.equals(Wizard.SHOWING)) {
            outputPanel.showing();
        } else if (cmd.equals(Wizard.NEXT)) {
            final CaGeStarter starter = new CaGeStarter(outputPanel);
            new Thread(new Runnable() {
                @Override
                public void run() {
                    starter.start();
                }
            }).start();
        } else {
            CaGe.listener.actionPerformed(e);
        }
    }
}

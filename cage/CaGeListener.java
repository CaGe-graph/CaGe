package cage;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import lisken.uitoolbox.Wizard;

/**
 *
 */
public class CaGeListener implements ActionListener {

    @Override
    public void actionPerformed(ActionEvent e) {
        String cmd = e.getActionCommand();
        if (cmd.equals(Wizard.CANCEL)) {
            if (CaGe.wizard().getStageNo() > 1) {
                CaGe.wizard().toStage(1, true);
                return;
            }
        } else if (cmd.equals(Wizard.EXIT)) {
            CaGe.exit();
            return;
        }
        CaGe.wizard().actionPerformed(e);
    }
}
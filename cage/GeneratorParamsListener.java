package cage;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import lisken.uitoolbox.Wizard;

/**
 *
 */
public class GeneratorParamsListener implements ActionListener {

    GeneratorPanel generatorPanel;
    String generatorName;

    public GeneratorParamsListener(GeneratorPanel generatorPanel, String generatorName) {
        this.generatorPanel = generatorPanel;
        this.generatorName = generatorName;
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        String cmd = e.getActionCommand();
        if (cmd.equals(Wizard.SHOWING)) {
            generatorPanel.showing();
        } else if (cmd.equals(Wizard.CANCEL)) {
            CaGe.getWizardStage().getPreviousButton().doClick();
        } else if (cmd.equals(Wizard.NEXT)) {
            CaGe.getWizardWindow().setVisible(false);
            GeneratorInfo generatorInfo = generatorPanel.getGeneratorInfo();
            generatorInfo.setGeneratorName(
                    CaGe.generatorButton[CaGe.lastGeneratorChoice].getText());
            OutputPanel outputPanel;
            if (CaGe.rememberPanels) {
                outputPanel = CaGe.outputPanels.get(generatorName);
            } else {
                outputPanel = null;
            }
            if (outputPanel == null) {
                outputPanel = new OutputPanel(generatorName);
            }
            if (CaGe.rememberPanels) {
                CaGe.outputPanels.put(generatorName, outputPanel);
            }
            outputPanel.setGeneratorInfo(generatorInfo);
            CaGe.wizard().nextStage(outputPanel,
                    new OutputParamsListener(outputPanel),
                    Wizard.PREVIOUS, "Start", null, Wizard.CANCEL, null);
        } else {
            CaGe.listener.actionPerformed(e);
        }
    }
}
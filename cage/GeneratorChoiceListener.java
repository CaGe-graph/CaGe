package cage;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.BorderFactory;
import javax.swing.JButton;

import lisken.uitoolbox.Wizard;

/**
 *
 */
public class GeneratorChoiceListener implements ActionListener {

    @Override
    public void actionPerformed(ActionEvent e) {
        CaGe.getWizardWindow().setVisible(false);
        CaGe.lastGeneratorChoice = Integer.parseInt(e.getActionCommand());
        String generator = CaGe.generator[CaGe.lastGeneratorChoice];
        String configPanelName = CaGe.config.getProperty(generator + ".ConfigPanel");
        try {
            GeneratorPanel configPanel;
            if (CaGe.rememberPanels) {
                configPanel = (GeneratorPanel) CaGe.generatorPanels.get(configPanelName);
            } else {
                configPanel = null;
            }
            if (configPanel == null) {
                configPanel = (GeneratorPanel) Class.forName(configPanelName).newInstance();
            }
            if (CaGe.rememberPanels) {
                CaGe.generatorPanels.put(configPanelName, configPanel);
            }
            configPanel.setBorder(BorderFactory.createCompoundBorder(
                        BorderFactory.createCompoundBorder(
                            BorderFactory.createEmptyBorder(10, 10, 10, 10),
                            BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(), " Generator Options:  " + ((JButton) e.getSource()).getText() + " ")
                        ),
                        BorderFactory.createEmptyBorder(20, 20, 20, 20)
                    ));
            CaGe.wizard().nextStage(configPanel,
                    new GeneratorParamsListener(configPanel, generator),
                    Wizard.PREVIOUS, Wizard.NEXT, null, null, null);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
}
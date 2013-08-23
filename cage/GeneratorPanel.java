package cage;

import javax.swing.JButton;
import javax.swing.JPanel;

import lisken.uitoolbox.WizardAwareComponent;

public abstract class GeneratorPanel extends JPanel implements WizardAwareComponent {

    private JButton nextButton;

    /**
     * Get the current <i>next</i> button in the wizard.
     *
     * @return the value of nextButton
     */
    public JButton getNextButton() {
        return nextButton;
    }

    /**
     * Set the current <i>next</i> button in the wizard. This button can then
     * be disabled by this panel in case of a non-valid state.
     *
     * @param nextButton The next button of the current stage
     */
    @Override
    public void setNextButton(JButton nextButton) {
        this.nextButton = nextButton;
    }

    private JButton previousButton;

    /**
     * Get the current <i>previous</i> button in the wizard.
     *
     * @return the value of previousButton
     */
    public JButton getPreviousButton() {
        return previousButton;
    }

    /**
     * Set the current <i>previous</i> button in the wizard.
     *
     * @param previousButton The previous button of the current stage
     */
    @Override
    public void setPreviousButton(JButton previousButton) {
        this.previousButton = previousButton;
    }

    private JButton finishButton;

    /**
     * Get the current <i>finish</i> button in the wizard.
     *
     * @return the value of finishButton
     */
    public JButton getFinishButton() {
        return finishButton;
    }

    /**
     * Set the current <i>finish</i> button in the wizard. This button can then
     * be disabled by this panel in case of a non-valid state.
     *
     * @param finishButton The finish button of the current stage
     */
    @Override
    public void setFinishButton(JButton finishButton) {
        this.finishButton = finishButton;
    }

    private JButton cancelButton;

    /**
     * Get the current <i>cancel</i> button in the wizard.
     *
     * @return the value of cancelButton
     */
    public JButton getCancelButton() {
        return cancelButton;
    }

    /**
     * Set the current <i>cancel</i> button in the wizard.
     *
     * @param cancelButton The cancel button of the current stage
     */
    @Override
    public void setCancelButton(JButton cancelButton) {
        this.cancelButton = cancelButton;
    }

    private JButton exitButton;

    /**
     * Get the current <i>exit</i> button in the wizard.
     *
     * @return the value of exitButton
     */
    public JButton getExitButton() {
        return exitButton;
    }

    /**
     * Set the current <i>exit</i> button in the wizard.
     *
     * @param exitButton The exit button of the current stage
     */
    @Override
    public void setExitButton(JButton exitButton) {
        this.exitButton = exitButton;
    }

    public abstract GeneratorInfo getGeneratorInfo();

    /**
     * Method called when this panel will be shown. This happens after
     * the stage of the wizard containing this panel is activated.
     */
    public abstract void showing();
}

package lisken.uitoolbox;

import javax.swing.JButton;

/**
 * Interface to identify components that are aware of the wizard
 * they are used in. This allows the {@link WizardStage} to pass
 * in references to some of its buttons.
 * 
 * @author nvcleemp
 */
public interface WizardAwareComponent {

    /**
     * Makes the component aware of the <i>previous</i> button.
     *
     * @param previousButton The previous button of this WizardStage
     */
    public void setPreviousButton(JButton previousButton);

    /**
     * Makes the component aware of the <i>next</i> button.
     *
     * @param nextButton The next button of this WizardStage
     */
    public void setNextButton(JButton nextButton);

    /**
     * Makes the component aware of the <i>finish</i> button.
     *
     * @param finishButton The finish button of this WizardStage
     */
    public void setFinishButton(JButton finishButton);

    /**
     * Makes the component aware of the <i>cancel</i> button.
     *
     * @param cancelButton The cancel button of this WizardStage
     */
    public void setCancelButton(JButton cancelButton);

    /**
     * Makes the component aware of the <i>exit</i> button.
     *
     * @param exitButton The exit button of this WizardStage
     */
    public void setExitButton(JButton exitButton);
}

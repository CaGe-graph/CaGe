package lisken.uitoolbox;

import java.awt.event.ActionListener;
import java.awt.event.WindowListener;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JRootPane;
import javax.swing.SwingConstants;

/**
 * A <code>WizardStage</code> is an individual stage in a {@link Wizard}.
 * @author nvcleemp
 */
public class WizardStage {

    private JComponent content;
    private ActionListener listener;
    private JButton previousButton;
    private JButton nextButton;
    private JButton finishButton;
    private JButton cancelButton;
    private JButton exitButton;
    private boolean setDefaultButton;
    private boolean hasAnyButtons;

    public WizardStage(String title, JComponent content,
            WindowListener windowListener, ActionListener escapeListener,
            ActionListener wizardListener,
            String previous, String next, String finish, String cancel, String exit,
            boolean setDefaultButton) {
        this.content = content;
        this.listener = wizardListener;
        this.setDefaultButton = setDefaultButton;
        hasAnyButtons = false;

        previousButton = createButton(previous, Wizard.PREVIOUS, "lisken/uitoolbox/WizardPrevious.gif", SwingConstants.RIGHT);
        nextButton = createButton(next, Wizard.NEXT, "lisken/uitoolbox/WizardNext.gif", SwingConstants.LEFT);
        finishButton = createButton(finish, Wizard.FINISH, null, 0);
        cancelButton = createButton(cancel, Wizard.CANCEL, null, 0);
        exitButton = createButton(exit, Wizard.EXIT, null, 0);

        if (content instanceof WizardAwareComponent) {
            WizardAwareComponent wac = (WizardAwareComponent) content;
            wac.setPreviousButton(previousButton);
            wac.setNextButton(nextButton);
            wac.setFinishButton(finishButton);
            wac.setCancelButton(cancelButton);
            wac.setExitButton(exitButton);
        }
    }

    final JButton createButton(String buttonText, String actionCmd, String IconPath, int textPosition) {
        if (buttonText == null) {
            return null;
        }
        hasAnyButtons = true;
        JButton button = new JButton(buttonText);
        if (IconPath != null) {
            button.setIcon(new ImageIcon(ClassLoader.getSystemResource(IconPath)));
            button.setHorizontalTextPosition(textPosition);
        }
        button.setActionCommand(actionCmd);
        button.addActionListener(getListener());
        return button;
    }

    /*
    public void setDefaultButton(JButton defaultButton)
    {
    if (defaultButton != null || getRootPane().getDefaultButton() != null) {
    getRootPane().setDefaultButton(defaultButton);
    }
    }
     */
    public void setDefaultButton(JRootPane rootPane) {
        if (nextButton != null) {
            rootPane.setDefaultButton(nextButton);
        } else if (finishButton != null) {
            rootPane.setDefaultButton(finishButton);
        } else {
            // rootPane.setDefaultButton(null);
        }
    }

    public JComponent getContent() {
        return content;
    }

    public ActionListener getListener() {
        return listener;
    }

    public JButton getPreviousButton() {
        return previousButton;
    }

    public JButton getNextButton() {
        return nextButton;
    }

    public JButton getFinishButton() {
        return finishButton;
    }

    public JButton getCancelButton() {
        return cancelButton;
    }

    public JButton getExitButton() {
        return exitButton;
    }

    public boolean mustSetDefaultButton() {
        return setDefaultButton;
    }

    public boolean hasAnyButtons() {
        return hasAnyButtons;
    }
}

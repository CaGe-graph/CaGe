package lisken.uitoolbox;

import java.awt.BorderLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.util.Vector;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.KeyStroke;

/**
 * A <code>Wizard</code> object can be used to represent the classical
 * GUI pattern of a sequence of dialogs which guide the user to a set
 * of parameters that need to be provided. The individual stages of a
 * <code>Wizard</code> are represented by {@link WizardStage} objects.
 */
public class Wizard implements ActionListener {

    public static final String PREVIOUS = "Previous";
    public static final String NEXT = "Next";
    public static final String FINISH = "Finish";
    public static final String CANCEL = "Cancel";
    public static final String EXIT = "Exit";
    public static final String SHOWING = "Showing";

    private WizardStage stage;
    private int stageNo;
    private String title;
    private WindowListener windowListener;
    private ActionListener escapeListener;
    private Vector stageVector;
    private JFrame currentWindow;

    /**
     * Creates a new <code>Wizard</code>.
     *
     * @param title The title for this <code>Wizard</code>.
     */
    public Wizard(String title) {
        this.title = title;
        windowListener = new WindowAdapter() {

            public void windowClosing(WindowEvent e) {
                if (stage.getExitButton() != null) {
                    stage.getExitButton().doClick();
                } else if (stage.getListener() != null) {
                    stage.getListener().actionPerformed(new ActionEvent(e, WindowEvent.WINDOW_CLOSING, EXIT));
                } else {
                    System.exit(0);
                }
            }
        };
        escapeListener = new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                if (stage.getCancelButton() != null) {
                    stage.getCancelButton().doClick();
                } else if (stage.getListener() != null) {
                    stage.getListener().actionPerformed(e);
                } else {
                    Wizard.this.actionPerformed(e);
                }
            }
        };
        stageVector = new Vector();
        stageNo = 0;
    }

    public void nextStage(JComponent wizardContent,
            ActionListener wizardListener, String previousText, String nextText,
            String finishText, String cancelText, String exitText) {
        nextStage(wizardContent, wizardListener,
                previousText, nextText, finishText, cancelText, exitText,
                true);
    }

    public void nextStage(JComponent wizardContent,
            ActionListener wizardListener, String previousText, String nextText,
            String finishText, String cancelText, String exitText,
            boolean setDefaultButton) {
        if (stageNo > stageVector.size()) {
            throw new RuntimeException("Wizard has fallen behind stage " + (stageNo + 1));
        } else if (stageNo == stageVector.size()) {
            stageVector.addElement(null);
        }
        ActionListener listener = wizardListener != null ? wizardListener : this;
        stageVector.setElementAt(
                new WizardStage(title, wizardContent,
                windowListener, escapeListener, listener,
                previousText, nextText, finishText, cancelText, exitText,
                setDefaultButton),
                stageNo);
        ++stageNo;
        activate();
    }

    public void previousStage() {
        if (stageNo <= 1) {
            throw new RuntimeException("previousStage called with no previous stage to go to");
        }
        --stageNo;
        activate();
    }

    public void toStage(int n) {
        toStage(n, false);
    }

    public void toStage(int n, boolean forgetLaterStages) {
        if (n <= 0) {
            throw new RuntimeException("Illegal stage number (minimum 1): " + n);
        } else if (stageVector.size() < n) {
            throw new RuntimeException("Wizard hasn't yet reached stage " + n);
        }
        stageNo = n;
        if (forgetLaterStages) {
            stageVector.setSize(stageNo);
        }
        activate();
    }

    /**
     * Returns the current stage of this <code>Wizard</code>.
     * @return the current stage of this <code>Wizard</code>.
     */
    public WizardStage getStage() {
        return stage;
    }

    public int getStageNo() {
        return stageNo;
    }

    public Window getWindow() {
        return currentWindow;
    }

    void activate() {
        if (currentWindow != null) {
            currentWindow.dispose();
        }
        stage = (WizardStage) stageVector.elementAt(stageNo - 1);

        currentWindow = new JFrame(title);
        currentWindow.addWindowListener(windowListener);
        currentWindow.getRootPane().registerKeyboardAction(escapeListener,
                Wizard.CANCEL,
                KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0),
                JComponent.WHEN_IN_FOCUSED_WINDOW);

        JPanel pane = new JPanel();
        pane.setLayout(new BorderLayout());
        if (stage.getContent() != null) {
            pane.add(stage.getContent(), BorderLayout.CENTER);
        }
        if (stage.hasAnyButtons()) {
            JPanel buttonPanel = new JPanel();
            buttonPanel.setLayout(new GridBagLayout());
            GridBagConstraints lc = new GridBagConstraints();
            lc.gridx = 0;
            lc.gridheight = 1;
            /*
            lc.gridy = 0;
            lc.gridwidth = GridBagConstraints.REMAINDER;
            lc.insets = new Insets(5, 5, 10, 5);
            buttonPanel.add(new JSeparator(SwingConstants.HORIZONTAL), lc);
             */
            lc.gridy = 1;
            lc.gridwidth = 1;
            lc.fill = GridBagConstraints.VERTICAL;
            lc.insets = new Insets(5, 10, 10, 10);
            addButton(buttonPanel, lc, stage.getPreviousButton(), KeyEvent.VK_LEFT);
            addButton(buttonPanel, lc, stage.getNextButton(), KeyEvent.VK_RIGHT);
            addButton(buttonPanel, lc, stage.getFinishButton(), KeyEvent.VK_UNDEFINED);
            addButton(buttonPanel, lc, stage.getCancelButton(), KeyEvent.VK_UNDEFINED);
            addButton(buttonPanel, lc, stage.getExitButton(), KeyEvent.VK_UNDEFINED);
            pane.add(buttonPanel, BorderLayout.SOUTH);
            if (stage.mustSetDefaultButton()) {
                stage.setDefaultButton(currentWindow.getRootPane());
            }
        }
        currentWindow.getContentPane().add(pane);
        /*
        int expectedWidth =
        currentWindow.getMinimumSize().width;
        // + currentWindow.getInsets().left + currentWindow.getInsets().right
        currentWindow.setSize(0, 0);
        while (currentWindow.getSize().width < expectedWidth)
        {
        currentWindow.invalidate();
        currentWindow.pack();
        }
         */
        currentWindow.pack();

        UItoolbox.centerOnScreen(currentWindow);
        currentWindow.setVisible(true);
        stage.getListener().actionPerformed(new ActionEvent(this, WindowEvent.WINDOW_ACTIVATED, SHOWING));
    }

    void addButton(JPanel panel, GridBagConstraints lc,
            JButton button, int mnemonic) {
        if (button == null) {
            return;
        }
        if (mnemonic != KeyEvent.VK_UNDEFINED) {
            button.setMnemonic(mnemonic);
        }
        ++lc.gridx;
        panel.add(button, lc);
    }

    public void actionPerformed(ActionEvent e) {
        String cmd = e.getActionCommand();
        if (cmd.equals(Wizard.PREVIOUS)) {
            previousStage();
        } else if (cmd.equals(Wizard.CANCEL) || cmd.equals(Wizard.EXIT)) {
            System.exit(0);
        }
    }
}


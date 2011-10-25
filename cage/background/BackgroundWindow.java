package cage.background;

import cage.CaGe;
import cage.FoldnetThread;

import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Point;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import javax.swing.JTextField;
import javax.swing.KeyStroke;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.border.Border;

import lisken.uitoolbox.UItoolbox;

public class BackgroundWindow extends JFrame
        implements PropertyChangeListener, Runnable, ActionListener {

    static final String foldnetNextText = "abort curr";
    public static final Border border;
    public static final Font font,  boldFont;
    public static final FontMetrics metrics;
    public static final int labelDist,  buttonDist,  fieldDist;


    static {
        JPanel panel = new JPanel();
        Font stdFont = panel.getFont();
        font = new Font(
                stdFont.getName(),
                stdFont.getStyle() & ~Font.BOLD,
                stdFont.getSize() - 2);
        boldFont = new Font(
                stdFont.getName(),
                stdFont.getStyle() | Font.BOLD,
                stdFont.getSize() - 2);
        metrics = panel.getFontMetrics(font);
        border = BorderFactory.createCompoundBorder(
                BorderFactory.createEtchedBorder(),
                BorderFactory.createEmptyBorder(1, 3, 1, 3));
        JButton button = new JButton("1");
        button.setFont(font);
        button.setBorder(border);
        Insets buttonInsets = button.getInsets();
        JTextField field = new JTextField();
        field.setFont(font);
        Insets fieldInsets = field.getInsets();
        int maxDist = Math.max(fieldInsets.top, buttonInsets.top) + 2;
        labelDist = maxDist;
        buttonDist = maxDist - buttonInsets.top;
        fieldDist = maxDist - fieldInsets.top;
    }
    int runners, activeRunners, displayedRunners;
    JPanel runnersPanel;
    Vector runnerControls;
    FoldnetThread foldnetThread;
    JPanel foldnetsPanel;
    JTextField foldnetsLeft;
    JTextField foldnetsSucceeded;
    JTextField foldnetsFailed;
    JButton foldnetNext;
    JPanel exitPanel;
    JLabel msg1, msg2;
    JButton exitButton, cancelButton;
    JSeparator exitSeparator;
    int exitPhase = 0;
    boolean stopButtonUsed;

    public BackgroundWindow(final FoldnetThread foldnetThread) {
        super("CaGe - background tasks");
        setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
        addWindowListener(new WindowAdapter() {

            @Override
            public void windowClosing(WindowEvent e) {
                if (active()) {
                    return;
                }
                setVisible(false);
                Enumeration controls = runnerControls.elements();
                while (controls.hasMoreElements()) {
                    ((RunnerControl) controls.nextElement()).removeIfFinished();
                }
            }
        });
        runners = activeRunners = displayedRunners = 0;
        JPanel content = new JPanel();
        content.setLayout(new GridBagLayout());
        content.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        setContentPane(content);
        runnersPanel = new JPanel();
        runnersPanel.setLayout(new GridBagLayout());
        JLabel runnersLabel = new JLabel("generators in background");
        runnersLabel.setFont(font);
        runnersPanel.add(runnersLabel,
                new GridBagConstraints(0, 0, 3, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 0), 0, 0));
        JLabel foldnetsLabel = new JLabel("folding nets");
        foldnetsLabel.setFont(font);
        foldnetsPanel = new JPanel();
        foldnetsPanel.setLayout(new GridBagLayout());
        foldnetsPanel.add(foldnetsLabel,
                new GridBagConstraints(0, 0, 3, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 0), 0, 0));
        foldnetsLeft = addFoldnetsView("left", foldnetsPanel, 1, font);
        foldnetsSucceeded = addFoldnetsView("made", foldnetsPanel, 2, font);
        foldnetsFailed = null;
        foldnetNext = new JButton(foldnetNextText);
        foldnetNext.setFont(font);
        foldnetNext.setBorder(border);
        foldnetNext.setActionCommand("next");
        foldnetNext.addActionListener(this);
        foldnetNext.setVisible(false);
        foldnetsPanel.add(foldnetNext,
                new GridBagConstraints(2, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(buttonDist, 5, buttonDist, 2), 0, 0));
        foldnetsPanel.add(
                Box.createHorizontalStrut(Math.max(
                metrics.stringWidth("made"), metrics.stringWidth("failed"))),
                new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(0, 2, 0, 5), 0, 0));
        foldnetsPanel.add(
                Box.createHorizontalStrut(foldnetNext.getPreferredSize().width),
                new GridBagConstraints(2, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 5, 0, 2), 0, 0));
        this.foldnetThread = foldnetThread;
        foldnetThread.addPropertyChangeListener(this);
        runnersPanel.setVisible(false);
        content.add(runnersPanel,
                new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(10, 10, 10, 10), 0, 0));
        foldnetsPanel.setVisible(false);
        content.add(foldnetsPanel,
                new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(10, 10, 10, 10), 0, 0));
        runnerControls = new Vector(0);
        stopButtonUsed = false;
        exitPanel = new JPanel();
        exitPanel.setLayout(new BoxLayout(exitPanel, BoxLayout.Y_AXIS));
        exitSeparator = new JSeparator();
        exitPanel.add(exitSeparator);
        exitPanel.add(Box.createVerticalStrut(10));
        msg1 = new JLabel("\u00a0");
        msg2 = new JLabel("\u00a0");
        JPanel msgPanel = new JPanel();
        msgPanel.setLayout(new BoxLayout(msgPanel, BoxLayout.Y_AXIS));
        msgPanel.add(msg1);
        msgPanel.add(msg2);
        msgPanel.setAlignmentX(1.0f);
        exitPanel.add(msgPanel);
        JPanel buttonPanel = new JPanel();
        buttonPanel.setLayout(new FlowLayout(FlowLayout.CENTER, 0, 5));
        exitButton = new JButton("Exit");
        exitButton.setActionCommand("exit");
        exitButton.addActionListener(this);
        cancelButton = new JButton("Cancel");
        cancelButton.setActionCommand("cancel");
        cancelButton.addActionListener(this);
        buttonPanel.add(exitButton);
        buttonPanel.add(Box.createHorizontalStrut(10));
        buttonPanel.add(cancelButton);
        buttonPanel.setAlignmentX(0.5f);
        exitPanel.add(Box.createVerticalStrut(10));
        exitPanel.add(buttonPanel);
        exitPanel.setVisible(false);
        content.add(exitPanel,
                new GridBagConstraints(0, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(10, 10, 10, 10), 0, 0));
        getRootPane().registerKeyboardAction(this, "cancel",
                KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0),
                JComponent.WHEN_IN_FOCUSED_WINDOW);
    }

    public static Font getContentFont() {
        return font;
    }

    public static Border getButtonBorder() {
        return border;
    }

    public static int getLabelDist() {
        return labelDist;
    }

    public static int getFieldDist() {
        return fieldDist;
    }

    public static int getButtonDist() {
        return buttonDist;
    }

    public void adjustActiveRunners(int adjust) {
        activeRunners += adjust;
        checkForExit();
    }

    public int getActiveRunners() {
        return activeRunners;
    }

    public int getDisplayedRunners() {
        return displayedRunners;
    }

    public void adjustDisplayedRunners(int adjust) {
        displayedRunners += adjust;
        boolean showingRunners = displayedRunners > 0;
        boolean showingFoldnets = foldnetsPanel.isVisible();
        boolean showingAny = showingRunners || showingFoldnets;
        runnersPanel.setVisible(showingRunners);
        exitSeparator.setVisible(showingAny);
        pack();
        if (exitPhase == 0) {
            setVisible(showingAny);
        }
    }

    public void addRunner(BackgroundRunner runner) {
        runners += 1;
        runnerControls.addElement(
                new RunnerControl(runner, this, runners, runnersPanel));
        runner.start();
    }

    JTextField addFoldnetsView(String name, JPanel panel, int y, Font font) {
        JTextField foldnetsField = new JTextField(3);
        foldnetsField.setFont(font);
        foldnetsField.setEnabled(false);
        foldnetsField.setHorizontalAlignment(SwingConstants.RIGHT);
        foldnetsField.setText("0");
        JLabel label = new JLabel(name);
        label.setFont(font);
        panel.add(label,
                new GridBagConstraints(0, y, 1, 1, 0.001, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(labelDist, 2, labelDist, 5), 0, 0));
        panel.add(foldnetsField,
                new GridBagConstraints(1, y, 1, 1, 0.001, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(fieldDist, 5, fieldDist, 2), 0, 0));
        return foldnetsField;
    }

    public void propertyChange(PropertyChangeEvent e) {
        SwingUtilities.invokeLater(this);
    }

    public void run() {
        int left = foldnetThread.tasksLeft();
        foldnetsLeft.setText(Integer.toString(left));
        int succeeded = foldnetThread.tasksSucceeded();
        foldnetsSucceeded.setText(Integer.toString(succeeded));
        int failed = foldnetThread.tasksFailed();
        if (failed > 0 && foldnetsFailed == null) {
            foldnetsFailed = addFoldnetsView("failed", foldnetsPanel, 3, font);
        }
        if (foldnetsFailed != null) {
            foldnetsFailed.setText(Integer.toString(failed));
        }
        foldnetNext.setText(foldnetNextText);
        foldnetNext.setVisible(left > 0);
        foldnetsPanel.setVisible(true);
        pack();
        boolean showingRunners = runnersPanel.isVisible();
        if (!showingRunners) {
            setVisible(true);
        }
        checkForExit();
    }

    public boolean active() {
        if (exitPhase < 2) {
            return getActiveRunners() > 0 || foldnetThread.tasksLeft() > 0;
        } else {
            return false;
        }
    }

    public void actionPerformed(ActionEvent e) {
        switch (e.getActionCommand().charAt(0)) {
            case 'n':
                stopButtonUsed = true;
                foldnetThread.abortCurrent();
                break;
            case 'e':
                finishAndExit();
                break;
            case 'c':
                if (exitPhase == 0) {
                    return;
                }
                setVisible(false);
                exitPanel.setVisible((exitPhase = 0) > 0);
                pack();
                CaGe.getWizardWindow().setVisible(true);
                setVisible(active() || displayedRunners > 0);
                break;
        }
    }

    @Override
    public void setVisible(boolean visible) {
        Window wizardWindow = CaGe.getWizardWindow();
        if (visible && !isVisible()) {
            if (wizardWindow.isVisible()) {
                Point location = wizardWindow.getLocation();
                Dimension size = wizardWindow.getSize();
                location.x += size.width;
                setLocation(location);
            } else {
                UItoolbox.centerOnScreen(this);
            }
        }
        super.setVisible(visible);
        if (visible && wizardWindow.isVisible()) {
            UItoolbox.focusWindow(wizardWindow);
        }
    }

    public void setStopButtonUsed() {
        stopButtonUsed = true;
    }

    public void resetStopButtonUsed() {
        stopButtonUsed = false;
    }

    public void askForExit() {
        setVisible(false);
        resetStopButtonUsed();
        msg1.setText("Some background tasks are still active.");
        msg2.setText("Waiting ...");
        exitPanel.setVisible((exitPhase = 1) > 0);
        pack();
        CaGe.getWizardWindow().setVisible(false);
        setVisible(true);
    }

    void checkForExit() {
        if (exitPhase > 0 && !active()) {
            msg1.setText("All background tasks have finished.");
            if (stopButtonUsed) {
                msg2.setText("Click to exit or cancel.");
            } else {
                msg2.setText("CaGe will now exit.");
                finishAndExit();
            }
            pack();
        }
    }

    void finishAndExit() {
        Enumeration controls = runnerControls.elements();
        while (controls.hasMoreElements()) {
            ((RunnerControl) controls.nextElement()).stop();
        }
        foldnetThread.exit();
        exitPhase = 2;
        CaGe.exit();
    }
}


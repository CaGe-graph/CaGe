package cage.background;

import cage.CaGe;

import java.awt.Color;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.border.Border;

import lisken.uitoolbox.UItoolbox;

/**
 *
 * @author nvcleemp
 */
public class RunnerControl implements PropertyChangeListener {

    private BackgroundRunner runner;
    private boolean running;
    private BackgroundWindow window;
    private JPanel panel;
    private JButton infoButton;
    private JTextField graphNoField;
    private JButton stopButton;
    private boolean stopButtonUsed;

    public RunnerControl(BackgroundRunner runner,
            BackgroundWindow window, int index, JPanel panel) {
        this.runner = runner;
        this.window = window;
        this.panel = panel;
        
        running = runner.isAlive();
        if (running) {
            window.adjustActiveRunners(+1);
        }
        Font font = BackgroundWindow.getContentFont();
        Border border = BackgroundWindow.getButtonBorder();
        
        //info button
        infoButton = new JButton(Integer.toString(index));
        infoButton.setHorizontalAlignment(SwingConstants.RIGHT);
        infoButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                if(RunnerControl.this.runner!=null){
                    UItoolbox.showTextInfo("task info", RunnerControl.this.runner.getInfoText(), infoButton);
                    infoButton.setForeground(Color.black);
                }
                removeIfFinished();
            }
        });
        infoButton.setFont(font);
        infoButton.setBorder(border);
        
        //textfield with graph number
        graphNoField = new JTextField(CaGe.graphNoDigits);
        graphNoField.setFont(font);
        graphNoField.setEnabled(false);
        graphNoField.setHorizontalAlignment(SwingConstants.RIGHT);
        graphNoField.setText("0");
        
        //stop button
        stopButton = new JButton(running ? "stop" : "dead");
        stopButton.setEnabled(running);
        stopButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                stopButtonUsed = true;
                RunnerControl.this.window.setStopButtonUsed();
                stop();
                // stopButton.setEnabled(false);
            }
        });
        stopButton.setFont(font);
        stopButton.setBorder(border);
        stopButtonUsed = false;
        
        panel.add(infoButton,
                new GridBagConstraints(0, index, 1, 1, 0.001, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.HORIZONTAL,
                new Insets(BackgroundWindow.buttonDist, 2, BackgroundWindow.buttonDist, 2), 0, 0));
        panel.add(graphNoField,
                new GridBagConstraints(1, index, 1, 1, 0.001, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(BackgroundWindow.fieldDist, 2, BackgroundWindow.fieldDist, 2), 0, 0));
        panel.add(stopButton,
                new GridBagConstraints(2, index, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(BackgroundWindow.buttonDist, 2, BackgroundWindow.buttonDist, 2), 0, 0));
        runner.addPropertyChangeListener(this);
        window.adjustDisplayedRunners(+1);
    }

    public void stop() {
        if (running) {
            if (runner.isAlive()) {
                runner.removePropertyChangeListener(this);
                runner.abort();
                running = false;
                runningChanged("stopped");
            } else {
                runnerCrashed();
            }
        }
    }

    public void removeIfFinished() {
        if (!running) {
            if (infoButton.isVisible()) {
                infoButton.setVisible(false);
                panel.remove(infoButton);
                panel.remove(graphNoField);
                panel.remove(stopButton);
                window.adjustDisplayedRunners(-1);
            }
        }
    }

    public void propertyChange(final PropertyChangeEvent e) {
        SwingUtilities.invokeLater(new Runnable() {

            public void run() {
                handlePropertyChange(e);
            }
        });
    }

    void handlePropertyChange(PropertyChangeEvent e) {
        switch (e.getPropertyName().charAt(0)) {
            case 'g':
                if (running) {
                    graphNoField.setText(Integer.toString(runner.getGraphNo()));
                }
                break;
            case 'r':
                boolean newRunning = ((Boolean) e.getNewValue()).booleanValue();
                if (newRunning != running) {
                    running = newRunning;
                    runningChanged(
                            running ? "stop" : stopButtonUsed ? "stopped" : "finished");
                }
                break;
            case 'e':
                exceptionOccurred((Exception) e.getNewValue());
                break;
            case 'c':
                runnerCrashed();
                break;
        }
    }

    void exceptionOccurred(Exception e) {
        infoButton.setForeground(Color.red);
    }

    void runningChanged(String stopButtonText) {
        graphNoField.setText(Integer.toString(runner.getGraphNo()));
        stopButton.setEnabled(running);
        stopButton.setText(stopButtonText);
        window.pack();
        window.adjustActiveRunners(running ? +1 : -1);
        if (!running) {
            runner = null;
        }
    }

    void runnerCrashed() {
        running = false;
        runningChanged("crashed");
    }
}


/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package cage;

import cage.writer.CaGeWriter;
import java.awt.Color;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Enumeration;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.border.Border;
import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.UItoolbox;

/**
 *
 * @author nvcleemp
 */
public class RunnerControl
        implements PropertyChangeListener, ActionListener {

    BackgroundRunner runner;
    boolean running;
    StringBuffer infoText;
    BackgroundWindow window;
    JPanel panel;
    JButton infoButton;
    JTextField graphNoField;
    JButton stopButton;
    boolean stopButtonUsed;

    public RunnerControl(BackgroundRunner runner,
            BackgroundWindow window, int index, JPanel panel) {
        this.runner = runner;
        this.window = window;
        this.panel = panel;
        GeneratorInfo generatorInfo = runner.getGeneratorInfo();
        running = runner.isAlive();
        if (running) {
            window.adjustActiveRunners(+1);
        }
        infoText = new StringBuffer();
        infoText.append("generator:\t " + generatorInfo.getGeneratorName() + "\n");
        if (CaGe.expertMode) {
            infoText.append("  command:\t " + Systoolbox.makeCmdLine(generatorInfo.getGenerator()) + "\n");
        }
        infoText.append("\noutput:\n");
        Enumeration writers = runner.getWriters().elements();
        Enumeration writeDests = runner.getWriteDestinations().elements();
        while (writeDests.hasMoreElements()) {
            int dimension = ((CaGeWriter) writers.nextElement()).getDimension();
            infoText.append("  ");
            infoText.append(dimension <= 0 ? "adj" : dimension + "D");
            infoText.append(" >\t " + (String) writeDests.nextElement() + "\n");
        }
        Font font = BackgroundWindow.getContentFont();
        Border border = BackgroundWindow.getButtonBorder();
        infoButton = new JButton(Integer.toString(index));
        infoButton.setHorizontalAlignment(SwingConstants.RIGHT);
        infoButton.setActionCommand("info");
        infoButton.addActionListener(this);
        infoButton.setFont(font);
        infoButton.setBorder(border);
        graphNoField = new JTextField(CaGe.graphNoDigits);
        graphNoField.setFont(font);
        graphNoField.setEnabled(false);
        graphNoField.setHorizontalAlignment(SwingConstants.RIGHT);
        graphNoField.setText("0");
        stopButton = new JButton(running ? "stop" : "dead");
        stopButton.setEnabled(running);
        stopButton.setActionCommand("stop");
        stopButton.addActionListener(this);
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

    public void actionPerformed(ActionEvent e) {
        switch (e.getActionCommand().charAt(0)) {
            case 'i':
                UItoolbox.showTextInfo("task info", infoText.toString(), infoButton);
                infoButton.setForeground(Color.black);
                removeIfFinished();
                break;
            case 's':
                stopButtonUsed = true;
                window.setStopButtonUsed();
                stop();
                // stopButton.setEnabled(false);
                break;
        }
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
        infoText.append("\nException occurred:\n");
        infoText.append(Systoolbox.getStackTrace(e));
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


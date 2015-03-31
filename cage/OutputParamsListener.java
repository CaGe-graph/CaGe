package cage;

import cage.writer.CaGeWriter;
import cage.writer.WriterConfigurationHandler;
import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.List;
import javax.swing.AbstractAction;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JPanel;

import lisken.uitoolbox.Wizard;

/**
 * Wizard stage listener for the output configuration stage.
 */
public class OutputParamsListener implements ActionListener {

    OutputPanel outputPanel;

    public OutputParamsListener(OutputPanel outputPanel) {
        this.outputPanel = outputPanel;
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        String cmd = e.getActionCommand();
        if (cmd.equals(Wizard.SHOWING)) {
            outputPanel.showing();
        } else if (cmd.equals(Wizard.NEXT)) {
            //first check if some writers need extra configuration
            List<WriterConfigurationHandler> handlers = outputPanel.getConfigurationHandlers();
            List<CaGeWriter> writers = outputPanel.getWriters();
            for (int i = 0; i < writers.size(); i++) {
                WriterConfigurationHandler handler = handlers.get(i);
                if(handler!=null){
                    JPanel configurationPanel = handler.getConfigurationPanel();
                    if(configurationPanel != null){
                        //show dialog containing panel
                        final JDialog configurationDialog = new JDialog((JDialog)null, "Options", true);
                        configurationDialog.setLayout(new BorderLayout(5, 5));
                        configurationDialog.add(configurationPanel, BorderLayout.NORTH);
                        configurationDialog.add(new JButton(new AbstractAction("OK") {
                            @Override
                            public void actionPerformed(ActionEvent e) {
                                //just close the dialog
                                configurationDialog.setVisible(false);
                            }
                        }), BorderLayout.SOUTH);
                        configurationDialog.pack();
                        configurationDialog.setLocationRelativeTo(outputPanel);
                        configurationDialog.setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
                        configurationDialog.setVisible(true);

                        //configure writer
                        handler.configureWriter(writers.get(i));
                    }
                }
            }
            
            //then start the generation
            final CaGeStarter starter = new CaGeStarter(outputPanel);
            new Thread(new Runnable() {
                @Override
                public void run() {
                    starter.start();
                }
            }).start();
        } else {
            CaGe.listener.actionPerformed(e);
        }
    }
}

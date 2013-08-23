package cage;

import cage.background.BackgroundRunner;
import cage.background.DefaultBackgroundRunner;
import cage.background.TwoViewBatchBackgroundRunner;
import cage.viewer.CaGeViewer;
import cage.writer.CaGeWriter;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.List;
import javax.swing.AbstractButton;

import lisken.systoolbox.ExceptionGroup;
import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.UItoolbox;
import lisken.uitoolbox.Wizard;

/**
 * Class that is responsible for starting the generation once the configuration is complete.
 */
public class CaGeStarter implements ActionListener {

    private OutputPanel outputPanel;
    private GeneratorInfo generatorInfo;
    private List<CaGeViewer> viewers;
    private List<CaGeWriter> writers;
    private List<String> writeDests;
    private int nViewers, nWriters;
    private CaGePipe generatorPipe = null;
    private ResultPanel resultPanel = null;
    private boolean stopped;

    /**
     * Creates a new CageStarter that takes its configuration from the given OutputPanel.
     * 
     * @param outputPanel The OutputPanel used to configure this run.
     */
    public CaGeStarter(OutputPanel outputPanel) {
        this.outputPanel = outputPanel;
    }

    /**
     * Tries to start the generation
     */
    public void start() {
        if (checkOutputOptions()) {
            CaGe.getWizardWindow().setVisible(false);
            prepareGeneratorAndEmbedder();
            if (nViewers > 0) {
                resultPanel = new ResultPanel(
                        generatorPipe, generatorInfo,
                        outputPanel.requests2D(), outputPanel.requests3D(),
                        viewers, writers);
                CaGe.wizard().nextStage(resultPanel, this,
                        CaGe.expertMode ? Wizard.PREVIOUS : null,
                        null, "Stop", Wizard.CANCEL, Wizard.EXIT, false);
            } else if (nWriters > 0) {
                BackgroundRunner backgroundRunner = new DefaultBackgroundRunner(
                        generatorPipe, generatorInfo,
                        outputPanel.requests2D(), outputPanel.requests3D(),
                        writers, writeDests);
                CaGe.wizard().toStage(1, true);
                CaGe.backgroundWindow().addRunner(backgroundRunner);
            }
            if(outputPanel.isBatchProcessorSelected()){
                BackgroundRunner backgroundRunner = 
                        new TwoViewBatchBackgroundRunner(
                                generatorPipe, generatorInfo, 
                                outputPanel.getBatchConfiguration());
                CaGe.wizard().toStage(1, true);
                CaGe.backgroundWindow().addRunner(backgroundRunner);
            }
        }
    }

    /**
     * Check the output options to see if the generation can be started.
     * 
     * @return true if the generation can be started and false otherwise.
     */
    private boolean checkOutputOptions() {
        generatorInfo = outputPanel.getGeneratorInfo();
        viewers = outputPanel.getViewers();
        writers = outputPanel.getWriters();
        writeDests = outputPanel.getWriteDestinations();
        try {
            setWriterOutputStreams();
        } catch (Exception ex) {
            CaGe.getWizardWindow().setVisible(true);
            UItoolbox.showTextInfo("File/Pipe output failure",
                    "Some of your output destinations are invalid.\n" +
                    "The following exceptions have occurred:\n\n" +
                    ex.toString() + "\n" +
                    "\nPlease change your choices in the output options window.",
                    CaGe.getWizardStage().getNextButton());
            return false;
        }
        nViewers = viewers == null ? 0 : viewers.size();
        nWriters = writers == null ? 0 : writers.size();
        if (nViewers <= 0 && nWriters <= 0 && !outputPanel.isBatchProcessorSelected()) {
            String viewerErrors = outputPanel.getViewerErrors();
            UItoolbox.showTextInfo("output failure",
                    "Failed to instantiate any of the selected viewers or graph formatters.\n" +
                    "Please change your choices in the output options window.\n" +
                    (viewerErrors == null ? "" : "\nErrors during attempted instantiation:\n\n" + viewerErrors),
                    CaGe.getWizardStage().getNextButton());
            return false;
        }
        return true;
    }

    private void prepareGeneratorAndEmbedder() {
        String runDir, path;
        runDir = CaGe.config.getProperty("CaGe.Generators.RunDir");
        path = CaGe.config.getProperty("CaGe.Generators.Path");
        String[][] generator, preFilter;
        Embedder embedder = generatorInfo.getEmbedder();
        embedder.setRunDir(runDir);
        embedder.setPath(path);
        generator = generatorInfo.getGenerator();
        preFilter = outputPanel.getPreFilter();
        if (preFilter != null) {
            String[][] newGenerator = new String[generator.length + preFilter.length][];
            System.arraycopy(generator, 0, newGenerator, 0, generator.length);
            System.arraycopy(preFilter, 0, newGenerator, generator.length, preFilter.length);
            generator = newGenerator;
        }
        try {
            generatorPipe = new NativeCaGePipe(generator,
                    CaGe.config.getProperty("CaGe.Generators.ErrFile"));
            generatorPipe.setRunDir(runDir);
            generatorPipe.setPath(path);
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }

    public void actionPerformed(ActionEvent e) {
        AbstractButton stopButton = CaGe.getWizardStage().getFinishButton();
        String cmd = e.getActionCommand();
        if (e.getSource() == resultPanel) {
            stopped = true;
            stopButton.setText("Restart");
            stopButton.setEnabled(true);
        } else if (cmd.equals(Wizard.SHOWING)) {
            resultPanel.setStopListener(this);
            stopped = false;
            resultPanel.start();
        } else if (cmd.equals(Wizard.FINISH)) {
            if (stopped) {
                stopButton.setEnabled(false);
                resultPanel.reset();
                stopButton.setText("Stop");
                stopButton.setEnabled(true);
                resultPanel.setStopListener(this);
                stopped = false;
                resetWriterOutputStreams();
                resultPanel.start();
            } else {
                stopButton.setEnabled(false);
                resultPanel.stop();
            }
        } else if (cmd.equals(Wizard.PREVIOUS) || cmd.equals(Wizard.CANCEL) || cmd.equals(Wizard.EXIT)) {
            resultPanel.setStopListener(null);
            stopButton.setEnabled(false);
            resultPanel.stop();
            stopped = true;
            resultPanel.reset();
            stopButton.setText("Restart");
            stopButton.setEnabled(true);
            CaGe.listener.actionPerformed(e);
        }
    }

    private void setWriterOutputStreams() throws Exception {
        ExceptionGroup exceptionGroup = new ExceptionGroup();
        int n = Math.min(writers.size(), writeDests.size());
        for (int i = 0; i < n; ++i) {
            setWriterOutputStream(writers.get(i), writeDests.get(i), exceptionGroup);
        }
        if (exceptionGroup.size() > 0) {
            throw exceptionGroup;
        }
    }

    private void resetWriterOutputStreams() {
        try {
            setWriterOutputStreams();
        } catch (Exception ex) {
            UItoolbox.showTextInfo("File/Pipe output exceptions", ex.toString(), resultPanel);
        }
    }

    private void setWriterOutputStream(CaGeWriter writer, String dest, ExceptionGroup exceptionGroup) {
        try {
            writer.setOutputStream(Systoolbox.createOutputStream(dest,
                    CaGe.config.getProperty("CaGe.Generators.RunDir")));
        } catch (Exception ex) {
            exceptionGroup.add(ex);
        }
    }
}

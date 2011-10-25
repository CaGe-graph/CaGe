package cage.background;

import cage.CaGe;
import cage.CaGePipe;
import cage.CaGeResult;
import cage.CaGeTimer;
import cage.EmbedThread;
import cage.EmbedThreadListener;
import cage.EmbeddableGraph;
import cage.GeneratorInfo;
import cage.utility.Debug;
import cage.utility.StackTrace;
import cage.writer.CaGeWriter;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Enumeration;
import java.util.Vector;

import lisken.systoolbox.MessageQueue;
import lisken.systoolbox.Systoolbox;

public class DefaultBackgroundRunner extends Thread implements BackgroundRunner {

    static final int graphNoFireInterval = CaGe.getCaGePropertyAsInt("CaGe.GraphNoFireInterval.Background", 10);
    static final int graphNoFirePeriod = CaGe.getCaGePropertyAsInt("CaGe.GraphNoFirePeriod.Background", 10000);
    private static int threadCount = 0;
    private boolean halted;
    private MessageQueue queue = new MessageQueue(CaGe.debugMode);
    private PropertyChangeEvent event;
    private StringBuffer infoText = new StringBuffer();
    CaGePipe generator;
    GeneratorInfo generatorInfo;
    boolean generatorFlowing, generatorRunning;
    int graphNo;
    boolean doEmbed2D, doEmbed3D;
    CaGeWriter[] writer;
    Vector writers, writeDests;
    final Vector propertyChangeListeners;
    EmbedThread embedThread;
    CaGeTimer timer = null;
    
    private EmbedThreadListener embedThreadListener = new EmbedThreadListener() {

        public void showEmbeddingException(Exception ex, String context, String embedDiagnostics) {
            String diagnostics = embedDiagnostics == null ? "" : embedDiagnostics;
            int p = diagnostics.length() - 1;
            if (p > 0 && diagnostics.charAt(p) == '\n') {
                diagnostics = diagnostics.substring(0, p);
            }
            Exception e = new Exception(
                    "embedding exception:\n  context: '" + context + "'" +
                    (embedDiagnostics != null && embedDiagnostics.length() > 0 ? "\n  diagnostic output: '" + embedDiagnostics + "'" : "\n  diagnostic output: <none>") +
                    "\n  exception:\t" + ex.toString());
            fireExceptionOccurred((Exception) e.fillInStackTrace());
        }

        public void embeddingFinished() {
            if (!generatorRunning) {
                end();
            }
        }
    };
    
    private PropertyChangeListener propertyChangeListener = new PropertyChangeListener() {

        @SuppressWarnings("CallToThreadDumpStack")
        public void propertyChange(PropertyChangeEvent e) {
            if (CaGe.debugMode) {
                new StackTrace("queueing property change: " + e.getPropertyName() + " = " + e.getNewValue() + " (old value: " + e.getOldValue() + ")").printStackTrace();
            }
            queue.put(e);
        }
    };

    public DefaultBackgroundRunner(CaGePipe generator, GeneratorInfo generatorInfo,
            boolean doEmbed2D, boolean doEmbed3D,
            Vector writers, Vector writeDests) {
        super("BackgroundRunner " + ++threadCount);
        Systoolbox.lowerPriority(this, 2);
        this.generator = generator;
        this.generatorInfo = generatorInfo;
        this.doEmbed2D = doEmbed2D;
        this.doEmbed3D = doEmbed3D;
        this.writers = writers;
        this.writeDests = writeDests;
        graphNo = 0;
        writer = new CaGeWriter[writers.size()];
        writers.copyInto(writer);
        generator.addPropertyChangeListener(propertyChangeListener);
        propertyChangeListeners = new Vector();
        embedThread = new EmbedThread(generatorInfo.getEmbedder(), 3);
        embedThread.setEmbedThreadListener(embedThreadListener);
        if (graphNoFirePeriod > 0) {
            timer = new CaGeTimer(this, graphNoFirePeriod);
        }
    }

    public GeneratorInfo getGeneratorInfo() {
        return generatorInfo;
    }

    public boolean doesEmbed2D() {
        return doEmbed2D;
    }

    public boolean doesEmbed3D() {
        return doEmbed3D;
    }

    public int getGraphNo() {
        return graphNo;
    }

    public Vector getWriters() {
        return writers;
    }

    public Vector getWriteDestinations() {
        return writeDests;
    }
    
    public String getInfoText() {
        infoText.append("generator:\t ").append(generatorInfo.getGeneratorName()).append("\n");
        if (CaGe.expertMode) {
            infoText.append("  command:\t ").append(Systoolbox.makeCmdLine(generatorInfo.getGenerator())).append("\n");
        }
        infoText.append("\noutput:\n");
        Enumeration writersEnumeration = writers.elements();
        Enumeration writeDestsEnumeration = writeDests.elements();
        while (writeDestsEnumeration.hasMoreElements()) {
            int dimension = ((CaGeWriter) writersEnumeration.nextElement()).getDimension();
            infoText.append("  ");
            infoText.append(dimension <= 0 ? "adj" : dimension + "D");
            infoText.append(" >\t ").append((String) writeDestsEnumeration.nextElement()).append("\n");
        }
        return infoText.toString();
    }

    @Override
    public void start()
            throws IllegalThreadStateException {
        Debug.print("Started BackgroundRunner");
        super.start();
        try {
            generator.start();
            embedThread.start();
        } catch (Exception ex) {
            Debug.reportException(ex);
            abort();
        }
        try {
            generator.yieldAndAdvanceBy(1);
        } catch (Exception ex) {
            Debug.reportException(ex);
            generator.fireExceptionOccurred(ex);
            end();
        }
        if (timer != null) {
            timer.start();
        }
    }

    public void abort() {
        if (timer != null) {
            timer.stop();
        }
        if (embedThread != null) {
            embedThread.setEmbedThreadListener(null);
            embedThread.abort();
            embedThread = null;
        }
        generator.stop();
    }

    void end() {
        queue.put(null);
    }

    void last() {
        setHalted(true);
        end();
    }

    @Override
    public void run() {
        while (getNextEvent()) {
            if (halted()) {
                break;
            }
            handlePropertyChange(event);
            yield();
        }
        finish();
    }

    private synchronized void setHalted(boolean halted) {
        Debug.print("halted: " + halted);
        this.halted = halted;
    }

    private synchronized boolean halted() {
        return halted;
    }

    private boolean getNextEvent() {
        try {
            event = (PropertyChangeEvent) queue.get();
        } catch (InterruptedException ex) {
            fireExceptionOccurred(ex);
            event = null;
        }
        return event != null;
    }

    void handlePropertyChange(PropertyChangeEvent e) {
        Debug.print("handling property change: " + e.getPropertyName() + " = " + e.getNewValue() + " (old value: " + e.getOldValue() + ")");
        switch (e.getPropertyName().charAt(0)) {
            case 'g':
                graphNoChanged(((Integer) e.getNewValue()).intValue());
                break;
            case 'f':
                flowingChanged(((Boolean) e.getNewValue()).booleanValue());
                break;
            case 'r':
                runningChanged(((Boolean) e.getNewValue()).booleanValue());
                break;
            case 'e':
                infoText.append("\nException occurred:\n");
                infoText.append(Systoolbox.getStackTrace((Exception) e.getNewValue()));
                firePropertyChange(e);
                break;
            case 'c':
                CaGeResult result = (CaGeResult) e.getNewValue();
                boolean success = ((Boolean) e.getOldValue()).booleanValue();
                embeddingMade(result, success);
                break;
            default:
                Debug.print("unimplemented property change: " + e.getPropertyName());
                break;
        }
    }

    void graphNoChanged(int graphNo) {
        Debug.print("graphNo: " + graphNo);
        if (graphNo <= 0) {
            return;
        }
        if (generatorFlowing) {
            return;
        }
        if (embedThread == null) {
            return;
        }
        try {
            EmbeddableGraph graph = generator.getGraph();
            embedThread.embed(
                    new CaGeResult(graph, graphNo),
                    propertyChangeListener, doEmbed2D, doEmbed3D, false);
        } catch (Exception ex) {
            generator.fireExceptionOccurred(ex);
        }
    }

    void flowingChanged(boolean flowing) {
        generatorFlowing = flowing;
        Debug.print("flowing: " + generatorFlowing);
    }

    void runningChanged(boolean running) {
        generatorRunning = running;
        Debug.print("running: " + generatorRunning);
        if (generatorRunning) {
            fireRunningChanged();
        } else if (embedThread != null && embedThread.isAlive()) {
            embedThread.end();
        } else {
            end();
        }
    }

    void embeddingMade(CaGeResult result, boolean success) {
        if (success) {
            for (int i = 0; i < writer.length; ++i) {
                try {
                    writer[i].outputResult(result);
                    writer[i].throwLastIOException();
                } catch (Exception ex) {
                    fireExceptionOccurred(ex);
                    end();
                }
            }
            graphNo = result.getGraphNo();
            if (graphNo == 1 ||
                    (graphNoFireInterval > 0 && graphNo % graphNoFireInterval == 0)) {
                fireGraphNoChanged();
            }
            try {
                if (generator.isRunning()) {
                    generator.yieldAndAdvanceBy(1);
                }
            } catch (Exception ex) {
                generator.fireExceptionOccurred(ex);
                end();
            }
        } else {
            end();
        }
    }

    void finish() {
        Debug.print("finishing");
        if (generatorRunning) {
            Debug.print("announcing crash");
            generator.removePropertyChangeListener(propertyChangeListener);
            generator.stop();
            generatorRunning = false;
            firePropertyChange(
                    new PropertyChangeEvent(
                    this, "crashed", null, null));
        }
        for (int i = 0; i < writer.length; ++i) {
            writer[i].stop();
        }
        writer = null;
        if (embedThread != null) {
            embedThread.end();
            try {
                embedThread.join();
            } catch (InterruptedException ex) {
            }
        }
        fireRunningChanged();
    }

    public void addPropertyChangeListener(PropertyChangeListener listener) {
        if (propertyChangeListeners != null) {
            synchronized (propertyChangeListeners) {
                if (!propertyChangeListeners.contains(listener)) {
                    propertyChangeListeners.addElement(listener);
                }
            }
        }
    }

    public void removePropertyChangeListener(PropertyChangeListener listener) {
        if (propertyChangeListeners != null) {
            synchronized (propertyChangeListeners) {
                propertyChangeListeners.removeElement(listener);
            }
        }
    }

    protected void firePropertyChange(PropertyChangeEvent e) {
        if (propertyChangeListeners != null) {
            Vector listeners;
            synchronized (propertyChangeListeners) {
                listeners = (Vector) propertyChangeListeners.clone();
            }
            int count = listeners.size();
            for (int i = 0; i < count; i++) {
                ((PropertyChangeListener) listeners.elementAt(i)).propertyChange(e);
            }
        }
    }

    public void fireGraphNoChanged() {
        firePropertyChange(
                new PropertyChangeEvent(
                this, "graphNo", null, new Integer(graphNo)));
    }

    public void fireRunningChanged() {
        Debug.print("announcing running change: " + generatorRunning);
        firePropertyChange(
                new PropertyChangeEvent(
                this, "running", null, generatorRunning));
    }

    public void fireExceptionOccurred(Exception e) {
        if (propertyChangeListeners != null) {
            firePropertyChange(
                    new PropertyChangeEvent(
                    this, "exception", null, e));
        } else {
            e.printStackTrace();
        }
    }
}


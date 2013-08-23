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

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.List;

import lisken.systoolbox.MessageQueue;
import lisken.systoolbox.Systoolbox;

/**
 * Abstract implementation of BackgroundRunner that takes care of basic functionality
 * such as proceeding to the next graph, terminating a run and communication with the
 * user interface.
 * 
 * This BackgroundRunner is constructed as an extension of Thread. Typical usage is
 * that a BackgroundRunner is created and added to a {@link BackgroundWindow} by using
 * the method {@link BackgroundWindow#addRunner(cage.background.BackgroundRunner)}.
 * This method then stores this runners, creates a {@link RunnerControl} for it and
 * invokes the <code>start()</code> method.
 * 
 * @author nvcleemp
 */
public abstract class AbstractBackgroundRunner extends Thread implements BackgroundRunner {

    private static final int graphNoFireInterval = CaGe.getCaGePropertyAsInt("CaGe.GraphNoFireInterval.Background", 10);
    private static final int graphNoFirePeriod = CaGe.getCaGePropertyAsInt("CaGe.GraphNoFirePeriod.Background", 10000);
    
    protected StringBuffer infoText = new StringBuffer();
    private PropertyChangeListener propertyChangeListener = new PropertyChangeListener() {

        @SuppressWarnings(value = "CallToThreadDumpStack")
        public void propertyChange(PropertyChangeEvent e) {
            if (CaGe.debugMode) {
                new StackTrace("queueing property change: " + e.getPropertyName() + " = " + e.getNewValue() + " (old value: " + e.getOldValue() + ")").printStackTrace();
            }
            queue.put(e);
        }
    };
    private int graphNo = 0;
    private final List<PropertyChangeListener> propertyChangeListeners = new ArrayList<>();
    private MessageQueue queue = new MessageQueue(CaGe.debugMode);
    private boolean doEmbed2D;
    private boolean doEmbed3D;
    private EmbedThread embedThread;
    private EmbedThreadListener embedThreadListener = new EmbedThreadListener() {

        public void showEmbeddingException(Exception ex, String context, String embedDiagnostics) {
            String diagnostics = embedDiagnostics == null ? "" : embedDiagnostics;
            int p = diagnostics.length() - 1;
            if (p > 0 && diagnostics.charAt(p) == '\n') {
                diagnostics = diagnostics.substring(0, p);
            }
            Exception e = new Exception(
                    "embedding exception:\n  context: '" + context + "'" + 
                    (embedDiagnostics != null && embedDiagnostics.length() > 0 ?
                            "\n  diagnostic output: '" + embedDiagnostics + "'" :
                            "\n  diagnostic output: <none>") + "\n  exception:\t"
                                    + ex.toString());
            fireExceptionOccurred((Exception) e.fillInStackTrace());
        }

        public void embeddingFinished() {
            if (!generatorRunning) {
                end();
            }
        }
    };
    private boolean generatorFlowing;
    private boolean generatorRunning;
    
    //the pipe to the generator process
    private CaGePipe generator;
    
    /* will tell this runner to signal that its graph number got updated in case
     * there is an update period specified.
     */
    private CaGeTimer timer = null;
    
    protected GeneratorInfo generatorInfo;
    protected boolean halted;
    protected PropertyChangeEvent event;

    public AbstractBackgroundRunner(String name, CaGePipe generator, GeneratorInfo generatorInfo,
            boolean doEmbed2D, boolean doEmbed3D) {
        super(name);
        Systoolbox.lowerPriority(this, 2);
        this.generator = generator;
        this.generatorInfo = generatorInfo;
        this.doEmbed2D = doEmbed2D;
        this.doEmbed3D = doEmbed3D;
        generator.addPropertyChangeListener(propertyChangeListener);
        embedThread = new EmbedThread(generatorInfo.getEmbedder(), 3);
        embedThread.setEmbedThreadListener(embedThreadListener);
        if (graphNoFirePeriod > 0) {
            timer = new CaGeTimer(this, graphNoFirePeriod);
        }
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
                handleResult(result, success);
                break;
            default:
                Debug.print("unimplemented property change: " + e.getPropertyName());
                break;
        }
    }

    void flowingChanged(boolean flowing) {
        generatorFlowing = flowing;
        Debug.print("flowing: " + generatorFlowing);
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
            embedThread.embed(new CaGeResult(graph, graphNo), propertyChangeListener, doEmbed2D, doEmbed3D, false);
        } catch (Exception ex) {
            generator.fireExceptionOccurred(ex);
        }
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

    public void addPropertyChangeListener(PropertyChangeListener listener) {
        if (propertyChangeListeners != null) {
            synchronized (propertyChangeListeners) {
                if (!propertyChangeListeners.contains(listener)) {
                    propertyChangeListeners.add(listener);
                }
            }
        }
    }

    public void removePropertyChangeListener(PropertyChangeListener listener) {
        if (propertyChangeListeners != null) {
            synchronized (propertyChangeListeners) {
                propertyChangeListeners.remove(listener);
            }
        }
    }

    protected void firePropertyChange(PropertyChangeEvent e) {
        if (propertyChangeListeners != null) {
            for (PropertyChangeListener listener : propertyChangeListeners) {
                listener.propertyChange(e);
            }
        }
    }
    
    private void handleResult(CaGeResult result, boolean success){
        if(success){
            
            embeddingMade(result);
            
            graphNo = result.getGraphNo();
            if (graphNo == 1 || (graphNoFireInterval > 0 && graphNo % graphNoFireInterval == 0)) {
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

    protected abstract void embeddingMade(CaGeResult result);

    @SuppressWarnings("CallToThreadDumpStack")
    public void fireExceptionOccurred(Exception e) {
        if (propertyChangeListeners != null) {
            firePropertyChange(new PropertyChangeEvent(this, "exception", null, e));
        } else {
            e.printStackTrace();
        }
    }

    public void fireGraphNoChanged() {
        firePropertyChange(new PropertyChangeEvent(this, "graphNo", null, new Integer(graphNo)));
    }

    public void fireRunningChanged() {
        Debug.print("announcing running change: " + generatorRunning);
        firePropertyChange(new PropertyChangeEvent(this, "running", null, generatorRunning));
    }

    public int getGraphNo() {
        return graphNo;
    }

    /**
     * Calls the <code>start()</code> method of the <code>Thread</code> which causes
     * this thread to run parallel with the current thread and invoke its <code>run()</code>
     * method, and also starts the generator and the embedders.
     * 
     * @throws IllegalThreadStateException 
     */
    @Override
    public void start() throws IllegalThreadStateException {
        Debug.print("Started BackgroundRunner");
        
        //start the thread (will cause the run() method to be invoked)
        super.start();
        
        //start the generator process and the embedder thread
        try {
            generator.start();
            embedThread.start();
        } catch (Exception ex) {
            Debug.reportException(ex);
            abort();
        }
        
        //advance the generator to the first graph.
        try {
            generator.yieldAndAdvanceBy(1);
        } catch (Exception ex) {
            Debug.reportException(ex);
            generator.fireExceptionOccurred(ex);
            end();
        }
        
        //in case there is a CaGeTimer we also start it at this point
        if (timer != null) {
            timer.start();
        }
    }

    /**
     * Called when the BackgroundRunner needs to stop unexpectedly. This can be
     * due to an error while starting the generator or embedder, or because the
     * user requested the termination of the background task.
     */
    public void abort() {
        //in case there is a CaGeTimer we stop this timer
        if (timer != null) {
            timer.stop();
        }
        
        //in case there is an EmbedThread we remove its listeners and stop it.
        if (embedThread != null) {
            embedThread.setEmbedThreadListener(null);
            embedThread.abort();
            embedThread = null;
        }
        
        //finally we also stop the generator
        generator.stop();
    }
    
    /**
     * Signals the BackgroundRunner to finish as soon as possible with handling
     * any more graphs except the ones that are currently already waiting for
     * processing.
     */
    protected void end() {
        /* ending is done by posting null to the queue. The getNextEvent() method
         * will return false once it encounters this null on the queue which will
         * then cause the run() method to terminate.
         */
        queue.put(null);
    }

    private synchronized boolean halted() {
        return halted;
    }

    private synchronized void setHalted(boolean halted) {
        Debug.print("halted: " + halted);
        this.halted = halted;
    }

    /*
     * Called in run() method to get the next event. Will return false once
     * an exception occurs or the queue contains a null (meaning that the end()
     * method has been invoked).
     */
    private boolean getNextEvent() {
        try {
            event = (PropertyChangeEvent) queue.get();
        } catch (InterruptedException ex) {
            fireExceptionOccurred(ex);
            event = null;
        }
        return event != null;
    }

    private void cleanUpGenerator(){
        if (generatorRunning) {
            Debug.print("announcing crash");
            generator.removePropertyChangeListener(propertyChangeListener);
            generator.stop();
            generatorRunning = false;
            firePropertyChange(new PropertyChangeEvent(this, "crashed", null, null));
        }
    }
    
    /**
     * Called at the end of the run() method to perform any clean up that needs
     * to be performed at this time. The generator has already been stopped at
     * the moment that this method is invoked. The embedder thread, if any, is
     * still running, but will be stopped once this method returns.
     */
    protected abstract void cleanUp();
    
    private void cleanUpEmbedder(){
        if (embedThread != null) {
            embedThread.end();
            try {
                embedThread.join();
            } catch (InterruptedException ex) {
            }
        }
    }

    /*
     * Called by the run() method to clean up, once the run() method has terminated.
     */
    private void finish() {
        Debug.print("finishing");
        //kill the generator in case it is running
        cleanUpGenerator();
        //perform any clean up specific for this implementation
        cleanUp();
        //kill the embedder in case it is running
        cleanUpEmbedder();
        //signal that the Backgroundrunner has finished
        fireRunningChanged();
    }

    private void last() {
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
            Thread.yield();
        }
        finish();
    }
    
}

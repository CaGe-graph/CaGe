package cage;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.List;

import lisken.systoolbox.Pipe;

public abstract class CaGePipe extends Pipe
        implements CaGeRunner {

    public abstract void advanceBy(int n) throws Exception;

    public abstract void yieldAndAdvanceBy(int n) throws Exception;

    public abstract void setGraphNoFireInterval(int interval);

    public abstract EmbeddableGraph getGraph() throws Exception;

    public CaGePipe(String[][] generatorCmds,
            String inFilename, String outFilename, String errFilename)
            throws Exception {
        super(generatorCmds, inFilename, outFilename, errFilename);
    }

    @Override
    public void start()
            throws Exception {
        super.start();
        graphNo = 0;
        flowing = false;
        running = true;
        fireRunningChanged();
    }
    int graphNo = 0;

    @Override
    public int getGraphNo() {
        return graphNo;
    }

    public void setGraphNo(int n)
            throws Exception {
        int d = n - graphNo;
        if (d > 0) {
            advanceBy(d);
        }
    }
    boolean flowing = false;

    public synchronized void setFlowing(boolean flowing)
            throws Exception {
        if (!running) {
            return;
        }
        if (this.flowing != flowing) {
            this.flowing = flowing;
            fireFlowingChanged();
        }
    }

    public synchronized boolean isFlowing() {
        return flowing;
    }
    boolean running = false;

    public synchronized boolean isRunning() {
        return running;
    }

    synchronized void setRunning(boolean running) {
        this.running = running;
    }

    protected final List<PropertyChangeListener> propertyChangeListeners = new ArrayList<>();

    @Override
    public void addPropertyChangeListener(PropertyChangeListener listener) {
        if (propertyChangeListeners != null) {
            synchronized (propertyChangeListeners) {
                if (!propertyChangeListeners.contains(listener)) {
                    propertyChangeListeners.add(listener);
                }
            }
        }
    }

    @Override
    public void removePropertyChangeListener(PropertyChangeListener listener) {
        if (propertyChangeListeners != null) {
            synchronized (propertyChangeListeners) {
                propertyChangeListeners.remove(listener);
            }
        }
    }

    synchronized void removeAllPropertyChangeListeners() {
        if (propertyChangeListeners != null) {
            synchronized (propertyChangeListeners) {
                propertyChangeListeners.clear();
            }
        }
    }

    @Override
    public void fireGraphNoChanged() {
        firePropertyChange(
                new PropertyChangeEvent(this, "graphNo", null, new Integer(graphNo)));
    }

    public void fireFlowingChanged() {
        firePropertyChange(
                new PropertyChangeEvent(this, "flowing", null, new Boolean(flowing)));
    }

    @Override
    public void fireRunningChanged() {
        firePropertyChange(
                new PropertyChangeEvent(this, "running", null, new Boolean(running)));
    }

    @Override
    public void fireExceptionOccurred(Exception e) {
        firePropertyChange(new PropertyChangeEvent(this, "exception", "generator exception (after " + getGraphNo() + " graphs)", e));
    }

    protected void firePropertyChange(PropertyChangeEvent e) {
        if (propertyChangeListeners != null) {
            for (PropertyChangeListener listener : propertyChangeListeners) {
                listener.propertyChange(e);
            }
        }
    }
}


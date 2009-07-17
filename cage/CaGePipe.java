package cage;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Vector;

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

    public void start()
            throws Exception {
        super.start();
        graphNo = 0;
        flowing = false;
        running = true;
        fireRunningChanged();
    }
    int graphNo = 0;

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

    private final Vector propertyChangeListeners = new Vector(0);

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

    synchronized void removeAllPropertyChangeListeners() {
        if (propertyChangeListeners != null) {
            synchronized (propertyChangeListeners) {
                propertyChangeListeners.setSize(0);
            }
        }
    }

    public void fireGraphNoChanged() {
        firePropertyChange(
                new PropertyChangeEvent(this, "graphNo", null, new Integer(graphNo)));
    }

    public void fireFlowingChanged() {
        firePropertyChange(
                new PropertyChangeEvent(this, "flowing", null, new Boolean(flowing)));
    }

    public void fireRunningChanged() {
        firePropertyChange(
                new PropertyChangeEvent(this, "running", null, new Boolean(running)));
    }

    public void fireExceptionOccurred(Exception e) {
        firePropertyChange(new PropertyChangeEvent(this, "exception", "generator exception (after " + getGraphNo() + " graphs)", e));
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
}


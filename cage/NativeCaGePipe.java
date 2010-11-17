package cage;

import cage.utility.Debug;
import cage.utility.StackTrace;
import lisken.systoolbox.Systoolbox;

public class NativeCaGePipe extends CaGePipe {

    final static int priorityOffset = 2;
    private long status;
    static private int flowThreadCount = 0;
    private FlowingThread flowingThread = null;
    private int advanceTarget = 0;

    private native void initCaGePipe();

    private native void startCaGePipe();

    private native void finalizeCaGePipe();

    private native synchronized void setAdvanceTarget(int advanceTarget);

    private native synchronized int getAdvanceTarget();

    private native void nStartAdvancing();

    public native EmbeddableGraph getGraph();

    public native boolean wouldBlock();

    public native void setGraphNoFireInterval(int interval);

    public NativeCaGePipe(String[][] generatorCmds,
            String inFilename, String outFilename, String errFilename)
            throws Exception {
        super(generatorCmds, inFilename, outFilename, errFilename);
        initCaGePipe();
    }

    public NativeCaGePipe(String[][] generatorCmds)
            throws Exception {
        this(generatorCmds, "/dev/null", null, null);
    }

    public NativeCaGePipe(String[][] generatorCmds, String errFilename)
            throws Exception {
        this(generatorCmds, "/dev/null", null, errFilename);
    }

    public void start()
            throws Exception {
        super.start();
        startCaGePipe();
    }

    class FlowingThread extends Thread {

        boolean moreWork;

        public FlowingThread() {
            super("FlowingThread-" + (++flowThreadCount));
        }

        public void run() {
            while (waitForWork()) {
                startAdvancing();
            }
        }

        synchronized boolean waitForWork() {
            try {
                Debug.print(Thread.currentThread().getName() + " is waiting");
                if (!moreWork) {
                    wait();
                }
                moreWork = false;
                return true;
            } catch (InterruptedException ex) {
                return false;
            }
        }

        public synchronized void getToWork() {
            Debug.print(Thread.currentThread().getName() + " is notifying " + flowingThread.getName());
            moreWork = true;
            this.notify();
        }
    }

    synchronized void setFlowingThread(FlowingThread flowingThread) {
        this.flowingThread = flowingThread;
    }

    synchronized FlowingThread getFlowingThread() {
        return flowingThread;
    }

    public void advanceBy(final int d) {
        advanceViaThread(d);
    }

    private void advanceViaThread(final int d) {
        if (CaGe.debugMode) {
            new StackTrace("debug: advanceViaThread(" + d + ") called").printStackTrace();
        }
        synchronized (this) {
            getAdvancePermission();
            if (getFlowingThread() == null) {
                FlowingThread newFlowingThread = new FlowingThread();
                Systoolbox.lowerPriority(newFlowingThread, priorityOffset);
                setFlowingThread(newFlowingThread);
                newFlowingThread.start();
                Debug.print("advance thread started.");
            }
            setAdvanceTarget(d < 0 ? d : graphNo + d);
            flowingThread.getToWork();
        }
    }

    public void yieldAndAdvanceBy(int d) {
        getAdvancePermission();
        while (wouldBlock()) {
            Thread.yield();
        }
        setAdvanceTarget(d < 0 ? d : graphNo + d);
        startAdvancing();
        Thread.yield();
    }

    private synchronized void getAdvancePermission() {
        if (isFlowing()) {
            throw new RuntimeException("NativeCaGePipe: stop flowing before advancing");
        }
        flowing = true;
        fireFlowingChanged();
    }

    private void startAdvancing() {
        try {
            if (CaGe.debugMode) {
                new StackTrace("debug: " + Thread.currentThread().getName() + " calls startAdvancing (target: " + getAdvanceTarget() + ")").printStackTrace();
            }
            nStartAdvancing();
        } catch (Exception e) {
            if (propertyChangeListeners.size() > 0) {
                fireExceptionOccurred(e);
            } else {
                e.printStackTrace();
            }
        }
    }

    public void setFlowing(boolean flowingOn) {
        if (!running) {
            return;
        }
        Debug.print("setFlowing: " + flowingOn);
        if (isFlowing() == flowingOn) {
            Debug.print("flowing unchanged.");
            return;
        }
        if (flowingOn) {
            synchronized (this) {
                advanceViaThread(-1);
            }
        } else {
            synchronized (this) {
                flowing = false;
                fireFlowingChanged();
                fireGraphNoChanged();
            }
        }
    }

    public void stop() {
        setRunning(false);
        setFlowing(false);
        super.stop();
        fireRunningChanged();
    }

    protected void finalize() throws Throwable {
        removeAllPropertyChangeListeners();
        finalizeCaGePipe();
        super.finalize();
    }
}


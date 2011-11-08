package lisken.systoolbox;

import cage.utility.StackTrace;

public class MessageQueue {

    public volatile boolean debug;

    public MessageQueue() {
        this(false);
    }

    public MessageQueue(boolean debug) {
        this.debug = debug;
    }

    /**
     * Waits until there is an item in the queue and returns its entry.
     * @return The entry of the first item in the queue.
     * @throws InterruptedException 
     */
    @SuppressWarnings("CallToThreadDumpStack")
    public synchronized Object get() throws InterruptedException {
        QueueItem item;
        while ((item = first) == null) {
            try {
                wait();
            } catch (InterruptedException ex) {
                throw ex;
            } catch (Exception ex) {
                ex.printStackTrace();
                item = null;
            }
        }
        first = first.next;
        return item.entry;
    }

    /**
     * Puts a new item on the queue with the given entry.
     * @param entry The entry to put in the queue.
     */
    @SuppressWarnings("CallToThreadDumpStack")
    public synchronized void put(Object entry) {
        if (debug) {
            new StackTrace("put: " + entry).printStackTrace();
        }
        QueueItem item = new QueueItem(entry);
        if (last != null) {
            last.next = item;
        }
        last = item;
        if (first == null) {
            first = item;
        }
        notifyAll();
    }
    private QueueItem first = null,  last = null;
    
    private static class QueueItem {

        public QueueItem(Object entry) {
            this.entry = entry;
        }
        public QueueItem next = null;
        public Object entry;
    }
}


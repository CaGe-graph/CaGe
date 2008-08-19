

package cage;


import java.io.*;
import java.beans.*;
import javax.swing.*;
import lisken.systoolbox.*;


public class EmbedThread extends Thread
{
  public volatile boolean debug = false;

  private static int threadCount = 0;

  public EmbedThread(Embedder embedder, int priorityOffset)
  {
    super("Embedder " + ++threadCount);
    this.embedder = embedder;
    Systoolbox.lowerPriority(this, priorityOffset);
  }

  public void run()
  {
    setHalted(false);
    while (getNextTask())
    {
      if (halted()) break;
      processTask();
      synchronized (task)
      {
	task.unfinished = false;
        task.notifyAll();
      }
      yield();
    }
    embeddingFinished();
  }

  private boolean getNextTask()
  {
    if (debug) System.err.println("Getting ...");
    try {
      task = (EmbedTask) queue.get();
    } catch (InterruptedException ex) {
      task = null;
    }
    if (debug) System.err.println(task);
    return task != null;
  }

  private void processTask()
  {
    task.success = ! (task.do2D || task.do3D || task.redo2D);
    EmbeddableGraph graph = task.result.graph;
    int graphNo = task.result.graphNo;
    if (task.do2D && ! task.redo2D) {
      try {
	embedder.embed2D(graph);
	task.result.reembed2DMade = false;
	task.success = true;
	checkDiagnosticOutput();
      } catch (Exception ex) {
	showEmbeddingException(ex,
	 "embedding exception (graph " + graphNo + ", 2D)",
	  getDiagnosticOutput());
      }
    }
    if (task.do3D) {
      try {
	embedder.embed3D(graph);
	task.success = true;
	checkDiagnosticOutput();
      } catch (Exception ex) {
	showEmbeddingException(ex,
	 "embedding exception (graph " + graphNo + ", 3D)",
	 getDiagnosticOutput());
      }
    }
    if (task.redo2D) {
      try {
	embedder.reembed2D(graph);
	task.result.reembed2DMade = true;
	task.success = true;
	checkDiagnosticOutput();
      } catch (Exception ex) {
	showEmbeddingException(ex,
	 "re-embedding exception (graph" + graphNo + ", 2D)",
	 getDiagnosticOutput());
      }
    }
    if (debug) System.err.println("task handled");
    synchronized (this)
    {
      ++tasksCompleted;
    }
    if (debug) System.err.print("firing task change ... ");
    fireTaskFinished();
    if (debug) System.err.println("done.");
  }

  void embeddingFinished()
  {
    if (getEmbedThreadListener() != null) {
      embedThreadListener.embeddingFinished();
    }
  }

  void showEmbeddingException
   (Exception ex, String context, String embedDiagnostics)
  {
    if (getEmbedThreadListener() != null) {
      embedThreadListener.showEmbeddingException(ex, context, embedDiagnostics);
    }
  }

  void checkDiagnosticOutput()
   throws Exception
  {
    setDiagnosticOutput(embedder.getDiagnosticOutput());
    if (diagnosticOutput != null && diagnosticOutput.length() > 0) {
      throw new IOException("embedder produced a valid embedding, but some diagnostic output as well");
    }
  }

  public synchronized void setDiagnosticOutput(String diagnosticOutput)
  {
    this.diagnosticOutput = diagnosticOutput;
  }

  public synchronized String getDiagnosticOutput()
  {
    return diagnosticOutput;
  }

  public void embed
   (CaGeResult result, PropertyChangeListener listener,
    boolean do2D, boolean do3D, boolean redo2D)
  {
    synchronized (this)
    {
      EmbedTask task = new EmbedTask(result, listener, do2D, do3D, redo2D);
      queue.put(task);
      ++tasksGiven;
    }
  }

  public synchronized void end()
  {
    queue.put(null);
  }

  public synchronized void last()
  {
    setHalted(true);
    end();
  }

  public synchronized void abort()
  {
    last();
    embedder.abort();
  }

  private synchronized void setHalted(boolean halted)
  {
    this.halted = halted;
  }

  private synchronized boolean halted()
  {
    return halted;
  }

  public synchronized int tasksGiven()
  {
    return tasksGiven;
  }

  public synchronized int tasksCompleted()
  {
    return tasksCompleted;
  }

  public synchronized int tasksLeft()
  {
    return tasksGiven - tasksCompleted;
  }

  public synchronized void fireTaskFinished()
  {
    task.listener.propertyChange(
     new PropertyChangeEvent(this, "coordinates",
      new Boolean(task.success), task.result));
  }

  public synchronized void setEmbedThreadListener(EmbedThreadListener embedThreadListener)
  {
    this.embedThreadListener = embedThreadListener;
  }

  public synchronized EmbedThreadListener getEmbedThreadListener()
  {
    return embedThreadListener;
  }

  private Embedder embedder;
  private MessageQueue queue = new MessageQueue();
  private EmbedTask task;
  private boolean halted;
  private int tasksGiven = 0, tasksCompleted = 0;
  private EmbedThreadListener embedThreadListener;
  private String diagnosticOutput;
}


class EmbedTask
{
  public EmbedTask
   (CaGeResult result, PropertyChangeListener listener,
    boolean do2D, boolean do3D, boolean redo2D)
  {
    this.result       = result;
    this.listener     = listener;
    this.do2D         = do2D;
    this.do3D         = do3D;
    this.redo2D       = redo2D;
    this.success      = false;
    this.unfinished   = true;
  }

  public CaGeResult result;
  public PropertyChangeListener listener;
  public boolean do2D;
  public boolean do3D;
  public boolean redo2D;
  public boolean success;
  public boolean unfinished;
}



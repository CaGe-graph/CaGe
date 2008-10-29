
package cage;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;
import lisken.systoolbox.BufferedFDOutputStream;
import lisken.systoolbox.Integer2;
import lisken.systoolbox.MessageQueue;
import lisken.systoolbox.Pipe;
import lisken.systoolbox.Systoolbox;


public class FoldnetThread extends Thread
{
  private static final boolean debug = false;

  public FoldnetThread()
  {
    super("Foldnet-Maker");
    Systoolbox.lowerPriority(this, 3);
    foldnetPageNos = new Hashtable();
  }

  public void setRunDir(String runDir)
  {
    this.runDir = runDir;
  }

  public void setPath(String path)
  {
    this.path = path;
  }

  public void run()
  {
    setHalted(false);
    while (getNextTask())
    {
      if (halted()) break;
      processTask();
    }
    finishFiles();
  }

  private boolean getNextTask()
  {
    if (debug) System.err.println("Getting ...");
    try {
      task = (FoldnetTask) queue.get();
    } catch (InterruptedException ex) {
      task = null;
    }
    if (debug) System.err.println(task);
    return task != null;
  }

  private void processTask()
  {
    Integer2 pageNo;
    if ((pageNo = (Integer2) foldnetPageNos.get(task.filename)) == null) {
      pageNo = new Integer2(0);
      foldnetPageNos.put(task.filename, pageNo);
    }
    boolean append = pageNo.value > 0;
    String command = "mkfoldnet"
     + (task.maxFacesize > 0 ? " -s " + task.maxFacesize : "")
     + (append ? " -n" : "")
     + " -p " + task.result.graphNo + " " + ++pageNo.value;
    try {
      if (task.filename.trim().charAt(0) == '|') {
	command += task.filename;
	task.filename = "/dev/null";
      }
      setFoldnetPipe(new Pipe(Systoolbox.parseCmdLine(command),
       null, task.filename, append, "/dev/null"));
      foldnetPipe.setRunDir(runDir);
      foldnetPipe.setPath(path);
      foldnetPipe.start();
      EmbeddableGraph graph = task.result.graph;
      BufferedFDOutputStream foldnetData = foldnetPipe.getOutputStream();
      String graphEncoding = "";
      float[][] coordinate = graph.get3DCoordinates();
      int graphSize = graph.getSize();
      for (int i0 = 0, i1 = 1; i0 < graphSize; i0 = i1++)
      {
	graphEncoding += Integer.toString(i1);
	for (int j = 0; j < 3; ++j)
	{
	  graphEncoding += " " + coordinate[i0][j];
	}
	EdgeIterator edges = graph.getEdgeIterator(i1);
	while (edges.hasNext())
	{
	  graphEncoding += " " + edges.nextEdge();
	}
	graphEncoding += "\n";
      }
      graphEncoding += "0\n";
      foldnetData.write(graphEncoding);
      foldnetData.close();
    } catch (Exception ex) {
       setFoldnetPipe(null);
    }
    if (debug) System.err.println("foldnet pipe filled.");
    int status = -1;
    if (getFoldnetPipe() != null) {
      status = foldnetPipe.waitForExit();
    }
    if (debug) System.err.println("foldnet pipe finished: " + status);
    synchronized (this)
    {
      setFoldnetPipe(null);
      ++tasksCompleted;
      if (status != 0) {
	++tasksFailed;
	--pageNo.value;
      }
    }
    if (debug) System.err.println("firing ...");
    fireTasksChanged();
    if (debug) System.err.println("fired.");
  }

  public void makeFoldnet
   (CaGeResult result, int maxFacesize, String filename)
  {
    synchronized (this)
    {
      queue.put(new FoldnetTask(result, maxFacesize, filename));
      ++tasksGiven;
    }
    fireTasksChanged();
  }

  private void finishFiles()
  {
    Enumeration files = foldnetPageNos.keys();
    while (files.hasMoreElements())
    {
      String filename = (String) files.nextElement();
      Integer2 pages = (Integer2) foldnetPageNos.get(filename);
      if (pages.value > 0) {
	FileWriter file = null;
	try {
	  file = new FileWriter(filename, true);
	  file.write("\n%%Pages: " + pages.value + "\n%%EOF\n\n");
	  file.close();
	} catch (IOException ex) {
	}
      }
    }
  }

  public synchronized void last()
  {
    setHalted(true);
    queue.put(null);
  }

  public synchronized void abortCurrent()
  {
    if (foldnetPipe != null) {
      foldnetPipe.stop();
    }
  }

  public void exit()
  {
    last();
    abortCurrent();
    try {
      join();
    } catch (InterruptedException ex) {
    }
  }

  private synchronized void setFoldnetPipe(Pipe foldnetPipe)
  {
    this.foldnetPipe = foldnetPipe;
  }

  private synchronized Pipe getFoldnetPipe()
  {
    return foldnetPipe;
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

  public synchronized int tasksFailed()
  {
    return tasksFailed;
  }

  public synchronized int tasksSucceeded()
  {
    return tasksCompleted - tasksFailed;
  }

  public void fireTasksChanged()
  {
    firePropertyChange(new PropertyChangeEvent(this, "tasks", null, null));
  }

  public void addPropertyChangeListener
   (PropertyChangeListener listener)
  {
    if (propertyChangeListeners != null) {
      synchronized (propertyChangeListeners)
      {
	if (! propertyChangeListeners.contains(listener)) {
	  propertyChangeListeners.addElement(listener);
        }
      }
    }
  }
  public void removePropertyChangeListener
   (PropertyChangeListener listener)
  {
    if (propertyChangeListeners != null) {
      synchronized (propertyChangeListeners)
      {
	propertyChangeListeners.removeElement(listener);
      }
    }
  }
  protected void firePropertyChange(PropertyChangeEvent e)
  {
    if (propertyChangeListeners != null) {
      Vector listeners;
      synchronized (propertyChangeListeners)
      {
	listeners = (Vector) propertyChangeListeners.clone();
      }
      int count = listeners.size();
      for (int i = 0; i < count; i++)
        ((PropertyChangeListener) listeners.elementAt(i)).propertyChange(e);
    }
  }


  private MessageQueue queue = new MessageQueue();
  private FoldnetTask task;
  private boolean halted;
  private Pipe foldnetPipe = null;
  private int tasksGiven = 0, tasksCompleted = 0, tasksFailed = 0;
  private Vector propertyChangeListeners = new Vector(0);
  private String runDir, path;
  private Hashtable foldnetPageNos;
}


class FoldnetTask
{
  public FoldnetTask(CaGeResult result, int maxFacesize, String filename)
  {
    String prefix;
    if (filename.startsWith(File.separator)) {
      prefix = "";
    } else if (filename.trim().startsWith("|")) {
      prefix = "";
    } else {
      prefix = CaGe.config.get("CaGe.Generators.RunDir") + File.separator;
    }
    this.filename     = prefix + filename;
    this.result       = result;
    this.maxFacesize  = maxFacesize;
  }

  public CaGeResult result;
  public int maxFacesize;
  public String filename;
}


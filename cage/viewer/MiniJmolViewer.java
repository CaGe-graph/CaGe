
package cage.viewer;

import cage.CaGe;
import cage.CaGeResult;
import cage.EmbeddableGraph;
import cage.GeneratorInfo;
import cage.ResultPanel;
import cage.writer.AbstractChemicalWriter;
import cage.writer.WriterFactory;
import java.applet.Applet;
import java.applet.AppletContext;
import java.applet.AppletStub;
import java.applet.AudioClip;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.Image;
import java.awt.Insets;
import java.awt.Label;
import java.awt.SystemColor;
import java.awt.Toolkit;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Hashtable;
import java.util.Iterator;
import org.openscience.jmol.DisplaySettings;
import org.openscience.miniJmol.JmolApplet;


public class MiniJmolViewer
 implements CaGeViewer, AppletContext, AppletStub
{
  static private boolean debug = false;
  static private Hashtable defaultParams;
  static private AbstractChemicalWriter cmlWriter;

  static {
    float atomSize = 0.75f;
    float zoom = 1.0f;
    try {
      zoom = Float.valueOf(
       CaGe.config.getProperty("MiniJmolViewer.Zoom")).floatValue();
    } catch (Exception ex) {
    }
    try {
      atomSize = Float.valueOf(
       CaGe.config.getProperty("MiniJmolViewer.AtomSize")).floatValue();
    } catch (Exception ex) {
    }
    defaultParams = new Hashtable();
    defaultParams.put("FCOLOUR", "#000000");
    defaultParams.put("STYLE", "HALFSHADED");
    defaultParams.put("WIREFRAMEROTATION", "OFF");
    defaultParams.put("ZOOM", Float.toString(zoom));
    defaultParams.put("ATOMSIZE", Float.toString(atomSize));
    cmlWriter = (AbstractChemicalWriter) WriterFactory.createCaGeWriter("CML");
  }

  public void setParameter(String name, String value)
  {
    params.put(name, value);
  }

  private JmolApplet applet;
  private boolean appletStarted;
  private URL baseURL;
  private Hashtable params;
  private DisplaySettings displaySettings = null;

  private Frame frame;
  private int configWidth, configHeight;
  private Label comment, status;

  private ResultPanel resultPanel;

  // private int[] dim;

  public MiniJmolViewer()
  {
    applet = null;
    appletStarted = false;
    try {
      baseURL = new URL("file", "", System.getProperty("user.dir") + java.io.File.separator);
    } catch (MalformedURLException e) {
      if (debug) System.err.println("baseURL: MalformedURLException");
      baseURL = null;
    }
    params = new Hashtable();
    comment = new Label("\u00a0");
    comment.setAlignment(Label.CENTER);
    status = new Label("\u00a0");
    status.setAlignment(Label.RIGHT);
    defaultParams.put("BCOLOUR", htmlColour(SystemColor.window));
    configWidth  = CaGe.getCaGePropertyAsInt("MiniJmolViewer.Width",  550);
    configHeight = CaGe.getCaGePropertyAsInt("MiniJmolViewer.Height", 500);
    createFrame();
  }

  private void createFrame()
  {
    if (frame != null) {
      return;
    }
    frame = new Frame("CaGe - Jmol Viewer Applet");
    frame.add(comment, BorderLayout.NORTH);
    frame.add(status, BorderLayout.SOUTH);
    appletResize(configWidth, configHeight);
    frame.addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
	setVisible(false);
        stopApplet();
      }
    });
  }

  String htmlColour(Color c)
  {
    String x = Integer.toHexString(c.getRGB() & 0x00ffffff), y = "#000000";
    return y.substring(0, 7 - x.length()) + x;
  }

  void installApplet()
  {
    applet = new JmolApplet();
    applet.setBackground(frame.getBackground());
    appletStarted = false;
    frame.add(applet, BorderLayout.CENTER);
  }

  void uninstallApplet()
  {
    stopApplet();
    if (applet != null) {
      try {
	displaySettings = applet.getDisplaySettings();
      } catch (NoSuchMethodError er) {
      }
      if (frame != null) frame.remove(applet);
    }
    status.setText("\u00a0");
    applet = null;
  }

  void startApplet()
  {
    if (appletStarted) return;
    if (debug) System.err.println("setting stub");
    applet.setStub(this);
    if (debug) System.err.println("calling init");
    try {
      applet.init();
    } catch (Exception ex) {
      resultPanel.showException(ex, "Jmol exception");
      appletStarted = true;
      return;
    }
    if (displaySettings != null) {
      try {
	applet.setDisplaySettings(displaySettings);
      } catch (NoSuchMethodError er) {
      }
    }
    if (debug) System.err.println("calling start");
    applet.start();
    if (debug) System.err.println("applet started");
    appletStarted = true;
  }

  void stopApplet()
  {
    if (! appletStarted) return;
    if (debug) System.err.println("calling stop");
    applet.stop();
    if (debug) System.err.println("calling destroy");
    applet.destroy();
    if (debug) System.err.println("applet destroyed");
    appletStarted = false;
  }

  public static void main(String argv[])
  {
    MiniJmolViewer t;
    t = new MiniJmolViewer();
    if (debug) System.err.println("applet installed");
    t.frame.addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        System.exit(0);
      }
    });
    if (argv.length > 1) {
      t.setParameter("MODEL", argv[0]);
      t.setParameter("FORMAT", argv[1]);
    } else {
      t.setParameter("MODEL", "methanol2.cml");
      t.setParameter("FORMAT", "CML");
    }
    t.comment.setText("a molecule");
    t.installApplet();
    t.startApplet();
    t.frame.setVisible(true);
  }

/* *** CaGeViewer methods ****************************************** */

  private int dimension = 0;

  public void setDimension(int d)
  {
    dimension = d;
    cmlWriter.setDimension(dimension);
  }

  public int getDimension()
  {
    return dimension;
  }

  public void setResultPanel(ResultPanel resultPanel)
  {
    this.resultPanel = resultPanel;
  }

  public void setGeneratorInfo(GeneratorInfo generatorInfo)
  {
    cmlWriter.setGeneratorInfo(generatorInfo);
  }

  public void setVisible(boolean isVisible)
  {
    if (isVisible) {
      createFrame();
      frame.setVisible(true);
    } else if (frame != null) {
      frame.dispose();
      frame = null;
    }
  }

  public void outputResult(CaGeResult result)
  {
    EmbeddableGraph graph = result.graph;
    int graphNo = result.graphNo;
    if (debug) System.err.println(graph.toString());
    String encoding = cmlWriter.encodeResult(result);
    String graphComment = graph.getComment();
    if (graphComment == null) graphComment = "";
    if (graphComment.length() > 0) {
      graphComment = " - " + graphComment;
    }
    graphComment = "Graph " + graphNo + " - " + graph.getSize() + " vertices"
     + graphComment;
    if (debug) System.err.println(encoding);
    comment.setText(graphComment);
    uninstallApplet();
    createFrame();
    installApplet();
    setParameter("MODEL", encoding);
    setParameter("FORMAT", "CMLSTRING");
    startApplet();
    if (frame.isVisible()) {
      frame.validate();
    } else {
      setVisible(true);
    }
  }

  public void stop()
  {
    stopApplet();
    setVisible(false);
  }

/* *** AppletContext methods *************************************** */

  public AudioClip getAudioClip(URL url)
  {
    if (debug) System.err.println("getAudioClip: " + url);
    return null;
  }

  public Image getImage(URL url)
  {
    if (debug) System.err.println("getImage: " + url);
    return Toolkit.getDefaultToolkit ().getImage (url);
  }

  public Applet getApplet(String name)
  {
    if (debug) System.err.println("getApplet: " + name);
    return null;
  }

  public Iterator getStreamKeys()
  {
	return null;
  }
  
  public InputStream getStream(String key)
  {
	return null;
  }
  
  public void setStream(String key, InputStream stream)
  {
	//
  }
  
  public java.util.Enumeration getApplets()
  {
    if (debug) System.err.println("getApplets");
    return null;
  }

  public void showDocument(URL url)
  {
    if (debug) System.err.println("showDocument: " + url);
  }

  public void showDocument(URL url, String target)
  {
    if (debug) System.err.println("showDocument: " + url + ", " + target);
  }

  public void showStatus(String msg)
  {
    status.setText(msg);
  }

/* *** AppletStub methods ****************************************** */

  public boolean isActive()
  {
    if (debug) System.err.println("isActive");
    return true;
  }

  public URL getDocumentBase()
  {
    if (debug) System.err.println("getDocumentBase > " + baseURL);
    return baseURL;
  }

  public URL getCodeBase()
  {
    if (debug) System.err.println("getCodeBase > " + baseURL);
    return baseURL;
  }

  public String getParameter(String name)
  {
    if (debug) System.err.println("getParameter: " + name);
    String result;
    if ((result = (String) params.get(name)) != null) {
      return result;
    } else if ((result = (String) defaultParams.get(name)) != null) {
      return result;
    } else {
      return null;
    }
  }

  public AppletContext getAppletContext()
  {
    return this;
  }

  public void appletResize(int width, int height)
  {
    if (debug) System.err.println("appletResize");

    Insets insets = frame.getInsets();
    Dimension commentSize = comment.getPreferredSize();
    Dimension statusSize = status.getPreferredSize();
    frame.setSize(
     Math.max(width, Math.max(commentSize.width, statusSize.width))
      + insets.left + insets.right,
     height + commentSize.height + statusSize.height
      + insets.top + insets.bottom);
  }
}

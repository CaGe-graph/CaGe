
package cage.viewer;

import cage.CaGe;
import cage.CaGeResult;
import cage.EdgeIterator;
import cage.EmbedThread;
import cage.EmbeddableGraph;
import cage.Embedder;
import cage.GeneratorInfo;
import cage.NativeEmbeddableGraph;
import cage.ResultPanel;
import cage.SavePSDialog;
import cage.StaticGeneratorInfo;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.Insets;
import java.awt.MediaTracker;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Properties;
import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JSlider;
import javax.swing.JToggleButton;
import javax.swing.SwingUtilities;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import lisken.systoolbox.Integer2;
import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.PushButtonDecoration;
import lisken.uitoolbox.SpinButton;
import lisken.uitoolbox.UItoolbox;


class FloatingPoint
{
  double x, y;
}


interface TwoViewDevice
{
  public void beginGraph();
  public void beginEdges();
  public void paintEdge
   (double x1, double y1, double x2, double y2, int v1, int v2);
  public void beginVertices();
  public void paintVertex(double x, double y, int number);
}


public class TwoView
 implements ActionListener, CaGeViewer, TwoViewDevice
{
  public static boolean debug = true;

  private JFrame frame;
  private JLabel title;
  private Box titlePanel;
  private JPanel savePanel;
  private TwoViewPanel twoViewPanel;
  private TwoViewPainter painter;
  private Font titleFont;

  private GeneratorInfo generatorInfo;
  private ResultPanel resultPanel;
  private CaGeResult result;

  private float edgeBrightness = 0.75f;
  private JSlider edgeBrightnessSlider;
  private SpinButton edgeWidthButton;
  private JToggleButton savePSButton;
  private SavePSDialog savePSDialog;
  private OutputStream savePSStream = null;
  private Hashtable psFilenames = new Hashtable();
  private Hashtable psPageNos = new Hashtable();

  public TwoView()
  {
    JScrollPane scrollPane;
    title = new JLabel("TwoView diagrams");
    titleFont = title.getFont();
    titleFont = new Font(
     titleFont.getName(),
     titleFont.getStyle() & ~ Font.BOLD,
     titleFont.getSize());
    title.setForeground(Color.black);
    title.setFont(titleFont);
    title.setAlignmentY(0.5f);
    Box titlePanel1 = new Box(BoxLayout.X_AXIS);
    titlePanel1.add(title);
    titlePanel1.add(Box.createHorizontalStrut(20));
    titlePanel1.add(Box.createHorizontalGlue());
    edgeBrightnessSlider =
     new JSlider(0, 15, (int) Math.round(20 * edgeBrightness));
    edgeBrightnessSlider.setPreferredSize(new Dimension(20, edgeBrightnessSlider.getPreferredSize().height));
    edgeBrightnessSlider.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent e)
      {
	edgeBrightness = getEdgeBrightness();
        twoViewPanel.setEdgeBrightness(edgeBrightness);
      }
    });
    JLabel edgeBrightnessLabel = new JLabel("edge brightness:");
    edgeBrightnessLabel.setFont(titleFont);
    edgeBrightnessLabel.setLabelFor(edgeBrightnessSlider);
    edgeBrightnessLabel.setDisplayedMnemonic(KeyEvent.VK_B);
    edgeWidthButton = new SpinButton(
     TwoViewPanel.DEFAULT_EDGE_WIDTH,
     TwoViewPanel.MIN_EDGE_WIDTH, TwoViewPanel.MAX_EDGE_WIDTH);
    edgeWidthButton.setMaximumSize(edgeWidthButton.getPreferredSize());
    edgeWidthButton.addChangeListener(new ChangeListener() {
      public void stateChanged(ChangeEvent e)
      {
        twoViewPanel.setEdgeWidth(edgeWidthButton.getValue());
      }
    });
    JLabel edgeWidthLabel = new JLabel("width:");
    edgeWidthLabel.setFont(titleFont);
    edgeWidthLabel.setLabelFor(edgeWidthButton);
    edgeWidthLabel.setDisplayedMnemonic(KeyEvent.VK_W);
    JPanel titlePanel2 = new JPanel();
    titlePanel2.setLayout(new BoxLayout(titlePanel2, BoxLayout.X_AXIS));
    titlePanel2.setBorder(BorderFactory.createEmptyBorder(5, 0, 5, 0));
    // the next line adds several buttons to titlePanel1, one to titlePanel2
    twoViewPanel = new TwoViewPanel(this, titlePanel1, titlePanel2, titleFont);
    titlePanel2.add(Box.createHorizontalStrut(5));
    titlePanel2.add(Box.createVerticalStrut(20));
    titlePanel2.add(Box.createHorizontalGlue());
    titlePanel2.add(edgeBrightnessLabel);
    titlePanel2.add(Box.createHorizontalStrut(5));
    titlePanel2.add(edgeBrightnessSlider);
    titlePanel2.add(Box.createHorizontalStrut(15));
    titlePanel2.add(edgeWidthLabel);
    titlePanel2.add(Box.createHorizontalStrut(5));
    titlePanel2.add(edgeWidthButton);
    titlePanel2.add(Box.createHorizontalStrut(5));
    try {
      setEdgeBrightness(Float.valueOf(
       CaGe.config.getProperty("TwoView.EdgeBrightness")).floatValue());
      edgeWidthButton.setValue(Integer.parseInt(
       CaGe.config.getProperty("TwoView.EdgeWidth")));
    } catch (Exception ex) {
    }
    titlePanel = new Box(BoxLayout.Y_AXIS);
    titlePanel.add(titlePanel1);
    titlePanel.add(titlePanel2);
    savePSButton = new JToggleButton("save PS");
    savePSButton.setFont(titleFont);
    savePSButton.setBorder(BorderFactory.createEmptyBorder(3, 7, 5, 7));
    new PushButtonDecoration(savePSButton);
    savePSButton.setMnemonic(KeyEvent.VK_P);
    savePSButton.setAlignmentY(0.5f);
    savePSButton.setActionCommand("s");
    savePSButton.addActionListener(this);
    savePanel = new JPanel();
    savePanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
    savePanel.add(savePSButton);
    createFrame();
    savePSDialog = new SavePSDialog("save Postscript");
    savePSDialog.setNearComponent(savePSButton);
  }

  private void createFrame()
  {
    if (frame != null) {
      return;
    }
    frame = new JFrame("CaGe - TwoView");
    JPanel content = (JPanel) frame.getContentPane();
    content.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    content.add(titlePanel, BorderLayout.NORTH);
    content.add(twoViewPanel, BorderLayout.CENTER);
    content.add(savePanel, BorderLayout.SOUTH);
    frame.pack();
  }

  public void actionPerformed(ActionEvent e)
  {
    savePSButtonPressed();
  }

  public void savePSButtonPressed()
  {
    savePSButton.getModel().setArmed(false);
    savePSButton.getModel().setPressed(true);
    savePSDialog.setVisible(true);
    if (savePSDialog.getSuccess()) {
      savePostScript();
    }
    savePSButton.getModel().setPressed(false);
    savePSButton.setSelected(result.saved2DPS > 0);
  }

  public void setEdgeBrightness(float edgeBrightness)
  {
    edgeBrightnessSlider.setValue((int) Math.round(edgeBrightness * 20.0f));
  }

  public float getEdgeBrightness()
  {
    return edgeBrightnessSlider.getValue() / 20.0f;
  }

  public void setDimension(int dimension)
  {
    if (dimension != 2) {
      throw new RuntimeException("TwoView is for 2D viewing only");
    }
  }

  public int getDimension()
  {
    return 2;
  }

  public void setResultPanel(ResultPanel resultPanel)
  {
    this.resultPanel = resultPanel;
    twoViewPanel.setResultPanel(resultPanel);
  }

  public void setGeneratorInfo(GeneratorInfo generatorInfo)
  {
    this.generatorInfo = generatorInfo;
    twoViewPanel.setGeneratorInfo(generatorInfo);
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
    CaGeResult previousResult = this.result;
    this.result = result;
    EmbeddableGraph graph = result.graph;
    int graphNo = result.graphNo;
    String graphComment = graph.getComment();
    if (graphComment == null) graphComment = "";
    if (graphComment.length() > 0) {
      graphComment = " - " + graphComment;
    }
    graphComment = "Graph " + graphNo + " - " + graph.getSize() + " vertices"
     + graphComment;
    title.setText(graphComment);
    savePSDialog.setInfo(graphComment);
    savePSButton.setSelected(result.saved2DPS > 0);
    String filename = (String) psFilenames.get(new Integer2(graphNo));
    if (filename == null && previousResult != null) {
      String previousFilename, previousNumber;
      int p;
      previousFilename =
       (String) psFilenames.get(new Integer2(previousResult.graphNo));
      if (previousFilename == null) {
	previousFilename = savePSDialog.getFilename();
      }
      previousNumber = "-" + previousResult.graphNo;
      if (previousFilename.toLowerCase().endsWith(".ps")) {
        p = previousFilename.length() - 3;
      } else if (previousFilename.toLowerCase().endsWith(".eps")) {
        p = previousFilename.length() - 4;
      } else {
        p = previousFilename.length();
      }
      int p0 = p - previousNumber.length();
      if (p0 >= 0 && previousFilename.substring(p0, p).equals(previousNumber)) {
        filename = previousFilename.substring(0, p0)
	 + "-" + graphNo + previousFilename.substring(p);
      } else {
        filename = previousFilename;
      }
    }
    if (filename == null) {
      filename = generatorInfo.getFilename() + "-2d-" + graphNo + ".ps";
    }
    savePSDialog.setFilename(filename);
    twoViewPanel.showResult(result);
    createFrame();
    if (! frame.isVisible()) {
      Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
      Dimension frameSize = frame.getSize();
      frame.setLocation(screenSize.width - frameSize.width, 0);
      setVisible(true);
    }
  }

  public void stop()
  {
    twoViewPanel.stop();
    Enumeration unfinishedFiles = psPageNos.keys();
    while (unfinishedFiles.hasMoreElements())
    {
      String psFilename = (String) unfinishedFiles.nextElement();
      try {
	savePSStream = Systoolbox.createOutputStream(
	 psFilename, CaGe.config.getProperty("CaGe.Generators.RunDir"), true);
	savePS("\n\n%%Pages: " + ((Integer2) psPageNos.get(psFilename)).intValue() + "\n");
	savePS("%%EOF\n\n");
	savePSStream.close();
      } catch (Exception ex) {
	UItoolbox.showTextInfo("Error finishing file '" + psFilename + "'",
	 Systoolbox.getStackTrace(ex));
      }
    }
    savePSStream = null;
    psFilenames = new Hashtable();
    psPageNos = new Hashtable();
    setVisible(false);
  }

  public void savePostScript()
  {
    String psFilename = savePSDialog.getFilename();
    Integer2 psPageNo = (Integer2) psPageNos.get(psFilename);
    boolean append = (psPageNo != null);
    try {
      savePSStream = Systoolbox.createOutputStream(
       psFilename, CaGe.config.getProperty("CaGe.Generators.RunDir"), append);
    } catch (Exception ex) {
      UItoolbox.showTextInfo(append ?
        "Error opening file" : "Error creating file",
       Systoolbox.getStackTrace(ex));
      savePSStream = null;
      return;
    }
    if (savePSStream == null) return;
    if (! append) {
      try {
	InputStream prolog = new BufferedInputStream(ClassLoader.getSystemResource("cage/viewer/TwoViewProlog.ps").openStream());
	int c;
	while ((c = prolog.read()) >= 0)
	{
	  savePSStream.write(c);
	}
	prolog.close();
      } catch (IOException ex1) {
	UItoolbox.showTextInfo("Error reading prolog",
	 Systoolbox.getStackTrace(ex1));
	try {
	  savePSStream.close();
	} catch (IOException ex2) {
	} finally {
	  savePSStream = null;
	}
      }
      psPageNo = new Integer2(0);
    }
    if (savePSStream == null) return;

    float factor = 1;
    float edgeWidth = twoViewPanel.getEdgeWidth() * factor * 2;
    float vertexRadius = twoViewPanel.getVertexSize() * factor;
    painter = new TwoViewPainter(this);
    painter.setPaintArea(
     42.520 + vertexRadius, 553.391 - vertexRadius,
     42.520 + vertexRadius, 658.493 - vertexRadius);
    // A4 page, 1.5 cm margin each side, another 5 cm clear on top
    painter.setGraph(result.graph);
    psPageNo.setValue(psPageNo.intValue()+1);
    savePS("\n%%Page: " + result.graphNo + " " + psPageNo.intValue() + "\n");
    FloatingPoint[] box = painter.getBoundingBox();
    savePS("%%BoundingBox: "
     + (float) (box[0].x - vertexRadius) + " "
     + (float) (box[0].y - vertexRadius) + " "
     + (float) (box[1].x + vertexRadius) + " "
     + (float) (box[1].y + vertexRadius) + "\n");
    savePS("\ngsave\n\n\n\n");
    if (savePSDialog.includeInfo()) {
      String info = Systoolbox.replace(savePSDialog.getInfo(),
       " - ", " \\320 ");
      savePS("/Helvetica findfont 20 scalefont setfont\n");
      savePS("297.9554 672.6665 (" + info + ") center_text\n");
    }
    savePS("/edge_width " + edgeWidth + " def\n");
    savePS("/edge_gray " + edgeBrightness + " def\n\n");
    savePS("/vertex_radius " + vertexRadius + " def\n");
    savePS("/vertex_linewidth " + (vertexRadius / 6) + " def\n");
    savePS("/vertex_color_1 { "
     + CaGe.config.getProperty("TwoView.VertexColor1")
     + " } bind def\n");
    savePS("/vertex_color_2 { "
     + CaGe.config.getProperty("TwoView.VertexColor2")
     + " } bind def\n");
    savePS("/vertex_number_color { "
     + CaGe.config.getProperty("TwoView.VertexNumberColor")
     + " } bind def\n\n");

    painter.paintGraph();
    savePS("\n\n\ngrestore\n\nshowpage\n");
    try {
      savePSStream.close();
      psPageNos.put(psFilename, psPageNo);
    } catch (IOException ex) {
      UItoolbox.showTextInfo("Error closing file",
       Systoolbox.getStackTrace(ex));
    }
    savePSStream = null;
    result.saved2DPS += 1;
    psFilenames.put(new Integer2(result.graphNo), psFilename);
  }

/*
  public String getPSFilename()
  {
    final FlaggedJDialog dialog = new FlaggedJDialog(this, "Filename Dialog", true);
    Container content = dialog.getContentPane();
    content.setLayout(new BoxLayout(content, BoxLayout.Y_AXIS));
    JPanel filePanel = new JPanel();
    JTextField psFilenameField = new JTextField(20);
    psFilenameField.setText(generatorInfo.getFilename() + "-2d.ps");
    filePanel.add(new JLabel("collect PS graphs in:"));
    filePanel.add(Box.createHorizontalStrut(5));
    filePanel.add(psFilenameField);
    filePanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    content.add(filePanel);
    JPanel buttonPanel = new JPanel();
    JButton okButton = new JButton("Ok");
    JButton cancelButton = new JButton("Cancel");
    buttonPanel.add(okButton);
    buttonPanel.add(cancelButton);
    content.add(buttonPanel);
    dialog.setDefaultButton(okButton);
    dialog.setCancelButton(cancelButton);
    dialog.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
    dialog.pack();
    new JTextComponentFocusSelector(psFilenameField);
    psFilenameField.requestFocus();
    dialog.show();
    return dialog.getSuccess() ? psFilenameField.getText() : null;
  }
*/

  public void savePS(String portion)
  {
    if (savePSStream == null) return;
    try {
      savePSStream.write(portion.getBytes());
    } catch (IOException ex1) {
      UItoolbox.showTextInfo("Error saving PostScript",
       Systoolbox.getStackTrace(ex1));
      try {
        savePSStream.close();
      } catch (IOException ex2) {
      } finally {
        savePSStream = null;
      }
    }
  }

  public void beginGraph()
  {
    int graphSize = painter.getGraphSize();
    for (int i = 1; i <= graphSize; ++i)
    {
      FloatingPoint p = painter.getCoordinatePoint(i);
      savePS("/v" + i + " { " + p.x + " " + p.y + " } bind def\n");
    }
  }

  public void beginEdges()
  {
    savePS("\n\nbegin_edges\n\n");
  }

  public void paintEdge
   (double x1, double y1, double x2, double y2, int v1, int v2)
  {
    savePS("v" + v1 + " " + "v" + v2 + " edge\n");
  }

  public void beginVertices()
  {
    savePS("\n\nbegin_vertices\n\n");
    if (twoViewPanel.getShowNumbers()) {
      savePS("(" + result.graph.getSize() + ") set_size\n\n");
    }
  }

  public void paintVertex(double x, double y, int number)
  {
    savePS("v" + number + " vertex\n");
    if (twoViewPanel.getShowNumbers()) {
      savePS("v" + number + " (" + number + ") vertex_number\n");
    }
  }

  public static void main(String[] argv) throws Exception
  {
    CaGe.config = new Properties();
    CaGe.config.load(new FileInputStream("CaGe.ini"));
    TwoView t;
    t = new TwoView();
    GeneratorInfo info = new StaticGeneratorInfo(null, null, null, 0, null);
    t.setGeneratorInfo(info);
    EmbeddableGraph g = new NativeEmbeddableGraph();
    g.addVertex();
    g.set2DCoordinates(1, new float[] { 0.0f, 0.0f });
    g.addEdge(2);
    g.addEdge(3);
    g.addEdge(4);
    g.addVertex();
    g.set2DCoordinates(2, new float[] { 0.5f, 1.0f });
    g.addEdge(1);
    g.addEdge(3);
    g.addVertex();
    g.set2DCoordinates(3, new float[] { -0.333f, 1.0f });
    g.addEdge(1);
    g.addEdge(2);
    g.addVertex();
    g.set2DCoordinates(4, new float[] { 0.25f, -1.0f });
    g.addEdge(1);
    CaGeResult r = new CaGeResult(g, 1);
    UItoolbox.addExitOnEscape(t.frame);
    t.frame.addWindowListener(new WindowAdapter() {
      public void WindowClosing(WindowEvent e)
      {
        System.exit(0);
      }
    });
    t.outputResult(r);
  }
}


class TwoViewPanel extends JPanel
 implements PropertyChangeListener, TwoViewDevice
{
  static int MIN_EDGE_WIDTH = 0, MAX_EDGE_WIDTH = 9;
  static int DEFAULT_EDGE_WIDTH = MAX_EDGE_WIDTH / 2;
  static int MAX_VERTEX_SIZE;
  static MediaTracker tracker;
  static int vertexSizes;
  static Image[] vertexImageArray;
  static AbstractButton[] sizeButtonArray;
  static ButtonGroup vertexSizeGroup;

  static {
    Toolkit toolkit = Toolkit.getDefaultToolkit();
    tracker = new MediaTracker(new JPanel());
    AbstractButton sizeButton;
    try {
      vertexSizes = Integer.parseInt(CaGe.config.getProperty("TwoView.MaxVertexSize"));
    } catch (Exception ex) {
      throw new RuntimeException("Couldn't read TwoView.MaxVertexSize from configuration");
    }
    vertexImageArray = new Image[vertexSizes];
    sizeButtonArray = new AbstractButton[vertexSizes];
    vertexSizeGroup = new ButtonGroup();
    for (int vertexID = 0; vertexID < vertexSizes; )
    {
      Image vertexImage = toolkit.getImage(ClassLoader.getSystemResource("Images/twoview-vertex-" + (vertexID+1) + ".gif"));
      vertexImageArray[vertexID] = vertexImage;
      tracker.addImage(vertexImage, vertexID);
      sizeButton = new JRadioButton(Integer.toString(vertexID+1));
      sizeButton.setActionCommand(" " + vertexID);
      sizeButton.setAlignmentY(0.5f);
      if (vertexID < 10) {
        sizeButton.setMnemonic(KeyEvent.VK_1 + vertexID % 10);
      }
      vertexSizeGroup.add(sizeButton);
      sizeButtonArray[vertexID] = sizeButton;
      if (++vertexID == vertexSizes) {
	tracker.addImage(vertexImage, 0);
	try {
	  tracker.waitForID(0);
	} catch (InterruptedException ex) {
	  ex.printStackTrace();
	}
	MAX_VERTEX_SIZE = Math.max(vertexImage.getWidth(null), vertexImage.getHeight(null));
      }
    }
    tracker.checkAll(true);
  }

  boolean showNumbers = false;
  int graphSize, vertexID, vertexSize, edgeWidth;
  CaGeResult result = null;
  Embedder embedder = null;
  boolean reembed2DDisabled = false;
  Image vertexImage;
  Font[] vertexFontArray;
  Font vertexFont;
  ActionListener sizeButtonListener;
  AbstractButton showNumbersButton;
  AbstractButton resetButton;
  ResultPanel resultPanel;
  TwoViewPainter painter;
  TwoView twoView;

  public TwoViewPanel
   (TwoView twoView,
    Container titlePanel1, Container titlePanel2, Font titleFont)
  {
    this.twoView = twoView;
    try {
      setPreferredSize(new Dimension(
       Integer.parseInt(CaGe.config.getProperty("TwoView.Width")),
       Integer.parseInt(CaGe.config.getProperty("TwoView.Height"))));
    } catch (Exception ex) {
    }
    sizeButtonListener = new ActionListener() {
      public void actionPerformed(ActionEvent e)
      {
	getVertexID();
	repaint();
      }
    };
    JLabel sizeLabel = new JLabel("vertex size:");
    sizeLabel.setFont(titleFont);
    titlePanel1.add(sizeLabel);
    titlePanel1.add(Box.createHorizontalStrut(5));
    Enumeration e = vertexSizeGroup.getElements();
    for (vertexID = 0; vertexID < vertexSizes; ++vertexID)
    {
      AbstractButton sizeButton = sizeButtonArray[vertexID];
      sizeButton.setFont(titleFont);
      titlePanel1.add(Box.createHorizontalStrut(5));
      titlePanel1.add(sizeButton);
      sizeButton.addActionListener(sizeButtonListener);
    }
    titlePanel1.add(Box.createHorizontalStrut(10));
    showNumbersButton = new JCheckBox("Numbers", showNumbers);
    showNumbersButton.setFont(titleFont);
    showNumbersButton.setMnemonic(KeyEvent.VK_N);
    showNumbersButton.setAlignmentY(0.5f);
    showNumbersButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e)
      {
	showNumbers(showNumbersButton.isSelected());
      }
    });
    titlePanel1.add(showNumbersButton);
    resetButton = new JButton("reset embedding");
    resetButton.setFont(titleFont);
    resetButton.setBorder(//BorderFactory.createCompoundBorder(
     //BorderFactory.createEtchedBorder(),
     BorderFactory.createEmptyBorder(3, 7, 5, 7));
    new PushButtonDecoration(resetButton);
    resetButton.setMnemonic(KeyEvent.VK_R);
    resetButton.setAlignmentY(0.5f);
    resetButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e)
      {
	resetButton.setText("re-embedding ...");
	resetButton.setEnabled(false);
	requestFocus();
	resetEmbedding();
      }
    });
    titlePanel2.add(resetButton);
    vertexFontArray = new Font[vertexSizes];
    try {
      vertexID = Integer.parseInt(
       CaGe.config.getProperty("TwoView.VertexSize"));
      if (vertexID <= 0) vertexID = 1;
      if (vertexID > vertexSizes) vertexID = vertexSizes;
      --vertexID;
    } catch (Exception ex) {
      vertexID = 1;
    }
    sizeButtonArray[vertexID].setSelected(true);
    getVertexImage();
    try {
      showNumbers = Systoolbox.parseBoolean(
       CaGe.config.getProperty("TwoView.ShowNumbers"),
       false);
      if (showNumbers) showNumbersButton.doClick();
    } catch (Exception ex) {
    }
    edgeWidth = DEFAULT_EDGE_WIDTH;
    addComponentListener(new ComponentAdapter() {
      public void componentResized(ComponentEvent e)
      {
        viewportChanged();
      }
    });
    addMouseListener(new MouseAdapter() {
      public void mouseClicked(MouseEvent e)
      {
	if (reembed2DDisabled || e.getModifiers() != InputEvent.BUTTON1_MASK) {
	  return;
	}
	FloatingPoint point = painter.getCoordinate(e.getX(), e.getY());
	if (embedder.reembed2DRequired(result.graph, (float) point.x, (float) point.y)) {
	  resetButton.setEnabled(false);
	  resetButton.requestFocus();
	  resetButton.setText("re-embedding ...");
	  EmbedThread embedThread = resultPanel.getEmbedThread();
	  if (embedThread != null)
	    embedThread.embed(result, TwoViewPanel.this, false, false, true);
	}
      }
    });
    painter = new TwoViewPainter(this);
  }

  void getVertexID()
  {
    String buttonAction = vertexSizeGroup.getSelection().getActionCommand();
    if (buttonAction.charAt(0) == '+') {
      showNumbers = true;
    } else if (buttonAction.charAt(0) == '-') {
      showNumbers = false;
    }
    vertexID = Integer.parseInt(buttonAction.substring(1));
    getVertexImage();
  }

  void getVertexImage()
  {
    try {
      tracker.waitForID(vertexID);
    } catch (InterruptedException ex) {
      ex.printStackTrace();
    }
    vertexImage = vertexImageArray[vertexID];
    vertexSize = Math.max(vertexImage.getWidth(null), vertexImage.getHeight(null));
    getVertexFont();
  }

  void getVertexFont()
  {
    if ((vertexFont = vertexFontArray[vertexID]) == null) {
      vertexFont = getFont();
      FontMetrics fm = getFontMetrics(vertexFont);
      int w = fm.stringWidth(Integer.toString(graphSize)), h = fm.getAscent();
      int fontSize;
      double factor = vertexSize * 0.85 / Math.sqrt(w*w + h*h);
      if (h * factor < 7.5) {
        fontSize = 0;
      } else {
	fontSize = (int) Math.round(vertexFont.getSize() * factor);
      }
      vertexFont = new Font(
       vertexFont.getName(),
       vertexFont.getStyle() & Font.BOLD,
       fontSize);
      vertexFontArray[vertexID] = vertexFont;
    }
    if (showNumbers && vertexFont.getSize() <= 0) {
      showNumbersButton.setSelected(showNumbers = false);
    }
  }

  public void stop()
  {
  }

  public void setResultPanel(ResultPanel resultPanel)
  {
    this.resultPanel = resultPanel;
  }

  public void setGeneratorInfo(GeneratorInfo generatorInfo)
  {
    boolean reembed2DEnabled = generatorInfo.isReembed2DEnabled();
    resetButton.setVisible(reembed2DEnabled);
    reembed2DDisabled = ! reembed2DEnabled;
    embedder = generatorInfo.getEmbedder();
  }

  public void showNumbers(boolean showNumbers)
  {
    this.showNumbers = showNumbers;
    if (showNumbers && vertexFont.getSize() <= 0) {
      showNumbersButton.setSelected(this.showNumbers = false);
      int oldID = vertexID;
      while (++vertexID < vertexSizes)
      {
        getVertexImage();
	if (vertexFont.getSize() > 0) {
	  sizeButtonArray[vertexID].setSelected(true);
	  showNumbersButton.setSelected(this.showNumbers = true);
	  break;
	}
      }
      if (vertexID >= vertexSizes) {
        vertexID = oldID;
	getVertexImage();
      }
    }
    repaint();
  }

  public boolean getShowNumbers()
  {
    return showNumbers;
  }

  public void setEdgeWidth(int edgeWidth)
  {
    this.edgeWidth = edgeWidth;
    repaint();
  }

  public int getEdgeWidth()
  {
    return edgeWidth;
  }

  public int getVertexSize()
  {
    return vertexSize;
  }

  public void setEdgeBrightness(float brightness)
  {
    edgeColor = new Color(brightness, brightness, brightness);
    repaint();
  }

  public void showResult(CaGeResult result)
  {
    this.result = result;
    graphChanged();
  }

  public void resetEmbedding()
  {
    EmbedThread embedThread = resultPanel.getEmbedThread();
    if (embedThread != null)
      embedThread.embed(result, this, true, false, false);
  }

  public void propertyChange(PropertyChangeEvent e)
  {
    final CaGeResult result = (CaGeResult) e.getNewValue();
    SwingUtilities.invokeLater(new Runnable() {
      public void run()
      {
	embeddingChanged(result);
      }
    });
  }

  public void embeddingChanged(CaGeResult result)
  {
    if (result != this.result) return;
    resultPanel.embeddingModified(this.twoView, result);
    graphChanged();
  }

  void graphChanged()
  {
    graphSize = result.graph.getSize();
    painter.setGraph(result.graph);
    for (int i = 0; i < vertexFontArray.length; ++i) vertexFontArray[i] = null;
    boolean showNumbers = this.showNumbers;
    getVertexFont();
    showNumbers(showNumbers);
    resetButton.setText("reset embedding");
    resetButton.setEnabled(result.reembed2DMade);
  }

  void viewportChanged()
  {
    Insets insets = getInsets();
    painter.setPaintArea(
     insets.left + (MAX_VERTEX_SIZE - 1) / 2,
     getWidth() - insets.right - MAX_VERTEX_SIZE / 2,
     getHeight() - insets.bottom - (MAX_VERTEX_SIZE - 1) / 2,
     insets.top + MAX_VERTEX_SIZE / 2);
    repaint();
  }

  Graphics graphics;
  Color edgeColor = new Color(0.75f, 0.75f, 0.75f);
  Color numbersColor = new Color(0.25f, 0.25f, 1.0f);

  public void paintComponent(Graphics graphics)
  {
    super.paintComponent(graphics);
    if (result.graph == null) return;
    this.graphics = graphics;
    Color oldColor = graphics.getColor();
    painter.paintGraph();
    graphics.setColor(oldColor);
  }

  int px[] = new int[4], py[] = new int[4];
  FontMetrics fontMetrics = null;
  int ascent = 0;

  public void beginGraph()
  {
  }

  public void beginEdges()
  {
    if (edgeWidth <= 0) return;
    graphics.setColor(edgeColor);
  }

  public void paintEdge
   (double x1, double y1, double x2, double y2, int v1, int v2)
  {
    int xp1, yp1, xp2, yp2;
    xp1 = (int) Math.floor (x1);
    yp1 = (int) Math.floor (y1);
    xp2 = (int) Math.floor (x2);
    yp2 = (int) Math.floor (y2);
    if (edgeWidth <= 0) {
      return;
    } else if (edgeWidth == 1) {
      graphics.drawLine(xp1, yp1, xp2, yp2);
    } else {
      double w = edgeWidth + 0.2;
      double dx = yp1 - yp2, dy = xp2 - xp1;
      double d = Math.sqrt(dx*dx + dy*dy);
      dx /= d;
      dy /= d;
      px[1] = (int) Math.round(xp1 - dx * w / 2);
      py[1] = (int) Math.round(yp1 - dy * w / 2);
      px[2] = (int) Math.round(xp2 - dx * w / 2);
      py[2] = (int) Math.round(yp2 - dy * w / 2);
      px[0] = (int) Math.round(px[1] + dx * w);
      py[0] = (int) Math.round(py[1] + dy * w);
      px[3] = (int) Math.round(px[2] + dx * w);
      py[3] = (int) Math.round(py[2] + dy * w);
      // System.err.println(i + " -> " + j + ": " + px[0] + "," + py[0] + " - " + px[1] + "," + py[1] + " - " + px[2] + "," + py[2] + " - " + px[3] + "," + py[3] + " (" + (float) dx + ", " + (float) dy + ")");
      graphics.fillPolygon(px, py, 4);
    }
  }

  public void beginVertices()
  {
    if (showNumbers && vertexFont.getSize() > 0) {
      graphics.setFont(vertexFont);
      fontMetrics = graphics.getFontMetrics();
      ascent = fontMetrics.getAscent();
    }
  }

  public void paintVertex(double x, double y, int number)
  {
    int xp = (int) Math.floor(x), yp = (int) Math.floor (y);
    if (edgeWidth > 0) {
      graphics.setColor(edgeColor);
      graphics.fillOval(xp - (edgeWidth - 1) / 2, yp - (edgeWidth - 1) / 2, edgeWidth, edgeWidth);
    }
    graphics.drawImage(vertexImage, xp - vertexSize / 2, yp - vertexSize / 2, null);
    if (showNumbers && vertexFont.getSize() > 0) {
      String numberString = Integer.toString(number);
      graphics.setColor(numbersColor);
      int width = fontMetrics.stringWidth(numberString);
      graphics.drawString(numberString, xp - (int) Math.floor(width * 0.52), yp + (int) Math.floor(ascent * 0.47));
    }
  }
}


class TwoViewPainter
{
  TwoViewDevice device;
  EmbeddableGraph graph;
  int graphSize;
  float coordinate[][];
  FloatingPoint p[];
  double xMin, xMax, yMin, yMax;
  double horMin, horMax, verMin, verMax;
  int horSign, verSign;
  double scale, delta, horOffset, verOffset;

  public TwoViewPainter(TwoViewDevice device)
  {
    this.device = device;
  }

  public void setGraph(EmbeddableGraph graph)
  {
    this.graph = graph;
    coordinate = graph.get2DCoordinates();
    graphSize = graph.getSize();
    if (graphSize <= 0) return;
    xMin = xMax = coordinate[0][0];
    yMin = yMax = coordinate[0][1];
    for (int i = 0; i < graphSize; ++i)
    {
      xMin = Math.min(xMin, coordinate[i][0]);
      xMax = Math.max(xMax, coordinate[i][0]);
      yMin = Math.min(yMin, coordinate[i][1]);
      yMax = Math.max(yMax, coordinate[i][1]);
    }
    viewportChanged();
  }

  public void setPaintArea
   (double horMin, double horMax, double verMin, double verMax)
  {
    this.verMin = verMin;
    this.verMax = verMax;
    if (horMin <= horMax) {
      horSign = +1;
      this.horMin = horMin;
      this.horMax = horMax;
    } else {
      horSign = -1;
      this.horMin = horMax;
      this.horMax = horMin;
    }
    if (verMin <= verMax) {
      verSign = +1;
      this.verMin = verMin;
      this.verMax = verMax;
    } else {
      verSign = -1;
      this.verMin = verMax;
      this.verMax = verMin;
    }
    viewportChanged();
  }

  void viewportChanged()
  {
    if (horSign != 0 && verSign != 0) {
      double horRng = horMax - horMin;
      double verRng = verMax - verMin;
      if (xMin == xMax || yMin == yMax) {
	delta = Math.max((xMax - xMin) / verRng, (yMax - yMin) / horRng) / 1e6;
      } else {
	delta = Math.min((xMax - xMin) / horRng, (yMax - yMin) / verRng) / 1e6;
      }
      scale = Math.max(delta, Math.min(horRng / (xMax - xMin + delta), verRng / (yMax - yMin + delta)));
      horOffset = (xMin + xMax + delta * horSign) / 2 * scale * horSign - horRng / 2 - horMin;
      verOffset = (yMin + yMax + delta * verSign) / 2 * scale * verSign - verRng / 2 - verMin;
      if (graphSize <= 0) {
	p = null;
      } else {
	p = new FloatingPoint[graphSize+1];
	for (int i = 0, j = 1; i < graphSize; i = j++)
	{
	  p[j] = getPoint(coordinate[i][0], coordinate[i][1]);
	}
      }
    }
  }

  public FloatingPoint getPoint(double x, double y)
  {
    FloatingPoint point = new FloatingPoint();
    point.x = Math.round((x * scale * horSign - horOffset - horMin) / delta) * delta + horMin;
    point.y = Math.round((y * scale * verSign - verOffset - verMin) / delta) * delta + verMin;
    return point;
  }

  public FloatingPoint getCoordinate(double px, double py)
  {
    FloatingPoint point = new FloatingPoint();
    point.x = (px + horOffset) / scale * horSign;
    point.y = (py + verOffset) / scale * verSign;
    return point;
  }

  public FloatingPoint[] getBoundingBox()
  {
    FloatingPoint[] box = new FloatingPoint[2];
    box[0] = getPoint(xMin, yMin);
    box[1] = getPoint(xMax, yMax);
    return box;
  }

  public int getGraphSize()
  {
    return graphSize;
  }

  public FloatingPoint getCoordinatePoint(int n)
  {
    return p[n];
  }

  public void paintGraph()
  {
    device.beginGraph();
    device.beginEdges();
    for (int i = graphSize; i > 0; --i)
    {
      EdgeIterator it = graph.getEdgeIterator(i);
      while (it.hasNext())
      {
	int j = it.nextEdge();
	if (j >= i) continue; // draw only edges to vertices that aren't drawn yet
	device.paintEdge(p[i].x, p[i].y, p[j].x, p[j].y, i, j);
      }
    }
    device.beginVertices();
    for (int i = graphSize; i > 0; --i)
    {
      device.paintVertex(p[i].x, p[i].y, i);
    }
  }
}

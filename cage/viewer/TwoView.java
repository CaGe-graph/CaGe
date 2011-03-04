package cage.viewer;

import cage.viewer.twoview.TwoViewPanel;
import cage.viewer.twoview.TwoViewPainter;
import cage.viewer.twoview.FloatingPoint;
import cage.viewer.twoview.TwoViewDevice;
import cage.CaGe;
import cage.CaGeResult;
import cage.EmbeddableGraph;
import cage.GeneratorInfo;
import cage.NativeEmbeddableGraph;
import cage.ResultPanel;
import cage.SavePSDialog;
import cage.StaticGeneratorInfo;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Properties;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.JToggleButton;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.systoolbox.MutableInteger;
import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.PushButtonDecoration;
import lisken.uitoolbox.SpinButton;
import lisken.uitoolbox.UItoolbox;

public class TwoView implements ActionListener, CaGeViewer, TwoViewDevice {

    private JFrame frame;
    private JLabel title;
    private Box titlePanel;
    private JPanel savePanel;
    private TwoViewPanel twoViewPanel;
    private TwoViewPainter painter;
    private GeneratorInfo generatorInfo;
    private ResultPanel resultPanel;
    private CaGeResult result;
    private float edgeBrightness = 0.75f;
    private JSlider edgeBrightnessSlider;
    private JToggleButton savePSButton;
    private SavePSDialog savePSDialog;
    private OutputStream savePSStream = null;
    private Hashtable psFilenames = new Hashtable();
    private Hashtable psPageNos = new Hashtable();

    public TwoView() {
        title = new JLabel("TwoView diagrams");
        Font titleFont = title.getFont();
        titleFont = new Font(
                titleFont.getName(),
                titleFont.getStyle() & ~Font.BOLD,
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

            public void stateChanged(ChangeEvent e) {
                edgeBrightness = getEdgeBrightness();
                twoViewPanel.setEdgeBrightness(edgeBrightness);
            }
        });
        JLabel edgeBrightnessLabel = new JLabel("edge brightness:");
        edgeBrightnessLabel.setFont(titleFont);
        edgeBrightnessLabel.setLabelFor(edgeBrightnessSlider);
        edgeBrightnessLabel.setDisplayedMnemonic(KeyEvent.VK_B);
        final SpinButton edgeWidthButton = new SpinButton(
                TwoViewPanel.DEFAULT_EDGE_WIDTH,
                TwoViewPanel.MIN_EDGE_WIDTH, TwoViewPanel.MAX_EDGE_WIDTH);
        edgeWidthButton.setMaximumSize(edgeWidthButton.getPreferredSize());
        edgeWidthButton.addChangeListener(new ChangeListener() {

            public void stateChanged(ChangeEvent e) {
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

    private void createFrame() {
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

    public void actionPerformed(ActionEvent e) {
        savePSButtonPressed();
    }

    public void savePSButtonPressed() {
        savePSButton.getModel().setArmed(false);
        savePSButton.getModel().setPressed(true);
        savePSDialog.setVisible(true);
        if (savePSDialog.getSuccess()) {
            savePostScript();
        }
        savePSButton.getModel().setPressed(false);
        savePSButton.setSelected(result.saved2DPS > 0);
    }

    public void setEdgeBrightness(float edgeBrightness) {
        edgeBrightnessSlider.setValue((int) Math.round(edgeBrightness * 20.0f));
    }

    public float getEdgeBrightness() {
        return edgeBrightnessSlider.getValue() / 20.0f;
    }

    public void setDimension(int dimension) {
        if (dimension != 2) {
            throw new RuntimeException("TwoView is for 2D viewing only");
        }
    }

    public int getDimension() {
        return 2;
    }

    public void setResultPanel(ResultPanel resultPanel) {
        this.resultPanel = resultPanel;
        twoViewPanel.setResultPanel(resultPanel);
    }

    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        this.generatorInfo = generatorInfo;
        twoViewPanel.setGeneratorInfo(generatorInfo);
    }

    public void setVisible(boolean isVisible) {
        if (isVisible) {
            createFrame();
            frame.setVisible(true);
        } else if (frame != null) {
            frame.dispose();
            frame = null;
        }
    }

    public void outputResult(CaGeResult result) {
        CaGeResult previousResult = this.result;
        this.result = result;
        EmbeddableGraph graph = result.graph;
        int graphNo = result.graphNo;
        String graphComment = graph.getComment();
        if (graphComment == null) {
            graphComment = "";
        }
        if (graphComment.length() > 0) {
            graphComment = " - " + graphComment;
        }
        graphComment = "Graph " + graphNo + " - " + graph.getSize() + " vertices" + graphComment;
        title.setText(graphComment);
        savePSDialog.setInfo(graphComment);
        savePSButton.setSelected(result.saved2DPS > 0);
        String filename = (String) psFilenames.get(new MutableInteger(graphNo));
        if (filename == null && previousResult != null) {
            String previousFilename, previousNumber;
            int p;
            previousFilename =
                    (String) psFilenames.get(new MutableInteger(previousResult.graphNo));
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
                filename = previousFilename.substring(0, p0) + "-" + graphNo + previousFilename.substring(p);
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
        if (!frame.isVisible()) {
            Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
            Dimension frameSize = frame.getSize();
            frame.setLocation(screenSize.width - frameSize.width, 0);
            setVisible(true);
        }
    }

    public void stop() {
        twoViewPanel.stop();
        Enumeration unfinishedFiles = psPageNos.keys();
        while (unfinishedFiles.hasMoreElements()) {
            String psFilename = (String) unfinishedFiles.nextElement();
            try {
                savePSStream = Systoolbox.createOutputStream(
                        psFilename, CaGe.config.getProperty("CaGe.Generators.RunDir"), true);
                savePS("\n\n%%Pages: " + ((MutableInteger) psPageNos.get(psFilename)).intValue() + "\n");
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

    public void savePostScript() {
        String psFilename = savePSDialog.getFilename();
        MutableInteger psPageNo = (MutableInteger) psPageNos.get(psFilename);
        boolean append = (psPageNo != null);
        try {
            savePSStream = Systoolbox.createOutputStream(
                    psFilename, CaGe.config.getProperty("CaGe.Generators.RunDir"), append);
        } catch (Exception ex) {
            UItoolbox.showTextInfo(append ? "Error opening file" : "Error creating file",
                    Systoolbox.getStackTrace(ex));
            savePSStream = null;
            return;
        }
        if (savePSStream == null) {
            return;
        }
        if (!append) {
            try {
                InputStream prolog = new BufferedInputStream(ClassLoader.getSystemResource("cage/viewer/TwoViewProlog.ps").openStream());
                int c;
                while ((c = prolog.read()) >= 0) {
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
            psPageNo = new MutableInteger(0);
        }
        if (savePSStream == null) {
            return;
        }

        float factor = 1;
        float edgeWidth = twoViewPanel.getEdgeWidth() * factor * 2;
        float vertexRadius = twoViewPanel.getVertexSize() * factor;
        painter = new TwoViewPainter(this);
        painter.setPaintArea(
                42.520 + vertexRadius, 553.391 - vertexRadius,
                42.520 + vertexRadius, 658.493 - vertexRadius);
        // A4 page, 1.5 cm margin each side, another 5 cm clear on top
        painter.setGraph(result.graph);
        psPageNo.setValue(psPageNo.intValue() + 1);
        savePS("\n%%Page: " + result.graphNo + " " + psPageNo.intValue() + "\n");
        FloatingPoint[] box = painter.getBoundingBox();
        savePS("%%BoundingBox: " + (float) (box[0].x - vertexRadius) + " " + (float) (box[0].y - vertexRadius) + " " + (float) (box[1].x + vertexRadius) + " " + (float) (box[1].y + vertexRadius) + "\n");
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
        savePS("/vertex_color_1 { " + CaGe.config.getProperty("TwoView.VertexColor1") + " } bind def\n");
        savePS("/vertex_color_2 { " + CaGe.config.getProperty("TwoView.VertexColor2") + " } bind def\n");
        savePS("/vertex_number_color { " + CaGe.config.getProperty("TwoView.VertexNumberColor") + " } bind def\n\n");

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
        psFilenames.put(new MutableInteger(result.graphNo), psFilename);
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

    /**
     * If there is a PS stream, this method writes <tt>portion</tt> to
     * that stream and otherwise returns. If this method fails to write to the
     * streams it will try to close the stream and set it to <tt>null</tt>.
     *
     * @param portion The text that should be written to the PS stream.
     */
    private void savePS(String portion) {
        if (savePSStream == null) {
            return;
        }
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

    public void beginGraph() {
        int graphSize = painter.getGraphSize();
        for (int i = 1; i <= graphSize; ++i) {
            FloatingPoint p = painter.getCoordinatePoint(i);
            savePS("/v" + i + " { " + p.x + " " + p.y + " } bind def\n");
        }
    }

    public void beginEdges() {
        savePS("\n\nbegin_edges\n\n");
    }

    public void paintEdge(double x1, double y1, double x2, double y2, int v1, int v2) {
        savePS("v" + v1 + " " + "v" + v2 + " edge\n");
    }

    public void beginVertices() {
        savePS("\n\nbegin_vertices\n\n");
        if (twoViewPanel.getShowNumbers()) {
            savePS("(" + result.graph.getSize() + ") set_size\n\n");
        }
    }

    public void paintVertex(double x, double y, int number) {
        savePS("v" + number + " vertex\n");
        if (twoViewPanel.getShowNumbers()) {
            savePS("v" + number + " (" + number + ") vertex_number\n");
        }
    }

    public static void main(String[] argv) throws Exception {
        CaGe.config = new Properties();
        CaGe.config.load(new FileInputStream("CaGe.ini"));
        TwoView t;
        t = new TwoView();
        GeneratorInfo info = new StaticGeneratorInfo(null, null, null, 0, null);
        t.setGeneratorInfo(info);
        EmbeddableGraph g = new NativeEmbeddableGraph();
        g.addVertex();
        g.set2DCoordinates(1, new float[]{0.0f, 0.0f});
        g.addEdge(2);
        g.addEdge(3);
        g.addEdge(4);
        g.addVertex();
        g.set2DCoordinates(2, new float[]{0.5f, 1.0f});
        g.addEdge(1);
        g.addEdge(3);
        g.addVertex();
        g.set2DCoordinates(3, new float[]{-0.333f, 1.0f});
        g.addEdge(1);
        g.addEdge(2);
        g.addVertex();
        g.set2DCoordinates(4, new float[]{0.25f, -1.0f});
        g.addEdge(1);
        CaGeResult r = new CaGeResult(g, 1);
        UItoolbox.addExitOnEscape(t.frame);
        t.frame.addWindowListener(new WindowAdapter() {

            public void WindowClosing(WindowEvent e) {
                System.exit(0);
            }
        });
        t.outputResult(r);
    }
}




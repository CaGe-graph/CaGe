package cage.viewer.twoview;


import cage.CaGe;
import cage.CaGeResult;
import cage.EmbedThread;
import cage.Embedder;
import cage.GeneratorInfo;
import cage.ResultPanel;
import cage.utility.Debug;
import cage.viewer.TwoView;

import java.awt.Color;
import java.awt.Container;
import java.awt.Dimension;
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
import java.awt.event.FocusAdapter;
import java.awt.event.FocusEvent;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Enumeration;
import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.SwingUtilities;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.PushButtonDecoration;

/**
 *
 */
public class TwoViewPanel extends JPanel
        implements PropertyChangeListener, TwoViewDevice {

    public static final int MIN_EDGE_WIDTH = 0,  MAX_EDGE_WIDTH = 9;
    public static final int DEFAULT_EDGE_WIDTH = MAX_EDGE_WIDTH / 2;
    static int MAX_VERTEX_SIZE;
    static final MediaTracker tracker;
    static final int vertexSizes;
    static Image[] vertexImageArray;
    static AbstractButton[] sizeButtonArray;
    static final ButtonGroup vertexSizeGroup;


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
        for (int vertexID = 0; vertexID < vertexSizes;) {
            Image vertexImage = toolkit.getImage(ClassLoader.getSystemResource("Images/twoview-vertex-" + (vertexID + 1) + ".gif"));
            vertexImageArray[vertexID] = vertexImage;
            tracker.addImage(vertexImage, vertexID);
            sizeButton = new JRadioButton(Integer.toString(vertexID + 1));
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
                    Debug.reportException(ex);
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

    AbstractButton highlightFacesButton;
    JFormattedTextField highlightedFacesSizeField;

    AbstractButton resetButton;
    ResultPanel resultPanel;
    TwoViewPainter painter;
    TwoView twoView;

    public TwoViewPanel(TwoView twoView,
            Container titlePanel1, Container titlePanel2, Font titleFont) {
        this.twoView = twoView;
        try {
            setPreferredSize(new Dimension(
                    Integer.parseInt(CaGe.config.getProperty("TwoView.Width")),
                    Integer.parseInt(CaGe.config.getProperty("TwoView.Height"))));
        } catch (Exception ex) {
        }
        sizeButtonListener = new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                getVertexID();
                repaint();
            }
        };
        JLabel sizeLabel = new JLabel("vertex size:");
        sizeLabel.setFont(titleFont);
        titlePanel1.add(sizeLabel);
        titlePanel1.add(Box.createHorizontalStrut(5));
        Enumeration e = vertexSizeGroup.getElements();
        for (vertexID = 0; vertexID < vertexSizes; ++vertexID) {
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

            public void actionPerformed(ActionEvent e) {
                showNumbers(showNumbersButton.isSelected());
            }
        });
        titlePanel1.add(showNumbersButton);

        highlightFacesButton = new JCheckBox("Highlight faces of size: ", false);
        highlightFacesButton.setFont(titleFont);
        highlightFacesButton.setAlignmentY(0.5f);
        highlightFacesButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                highlightFaces(highlightFacesButton.isSelected());
                highlightedFacesSizeField.setEnabled(highlightFacesButton.isSelected());
            }
        });
        titlePanel1.add(highlightFacesButton);

        highlightedFacesSizeField = new JFormattedTextField(5);
        highlightedFacesSizeField.setEnabled(highlightFacesButton.isSelected());
        highlightedFacesSizeField.setColumns(4);
        highlightedFacesSizeField.setAlignmentY(0.5f);
        highlightedFacesSizeField.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        setHighlightedFaces((Integer)highlightedFacesSizeField.getValue());
                    }
                });
            }
        });
        highlightedFacesSizeField.addFocusListener(new FocusAdapter() {

            @Override
            public void focusLost(FocusEvent e) {
                SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        setHighlightedFaces((Integer)highlightedFacesSizeField.getValue());
                    }
                });
            }
        });
        titlePanel1.add(highlightedFacesSizeField);

        resetButton = new JButton("reset embedding");
        resetButton.setFont(titleFont);
        resetButton.setBorder(//BorderFactory.createCompoundBorder(
                //BorderFactory.createEtchedBorder(),
                BorderFactory.createEmptyBorder(3, 7, 5, 7));
        new PushButtonDecoration(resetButton);
        resetButton.setMnemonic(KeyEvent.VK_R);
        resetButton.setAlignmentY(0.5f);
        resetButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
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
            if (vertexID <= 0) {
                vertexID = 1;
            }
            if (vertexID > vertexSizes) {
                vertexID = vertexSizes;
            }
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
            if (showNumbers) {
                showNumbersButton.doClick();
            }
        } catch (Exception ex) {
        }
        edgeWidth = DEFAULT_EDGE_WIDTH;
        addComponentListener(new ComponentAdapter() {

            @Override
            public void componentResized(ComponentEvent e) {
                viewportChanged();
            }
        });
        addMouseListener(new MouseAdapter() {

            @Override
            public void mouseClicked(MouseEvent e) {
                if (reembed2DDisabled || e.getModifiers() != InputEvent.BUTTON1_MASK) {
                    return;
                }
                FloatingPoint point = painter.getCoordinate(e.getX(), e.getY());
                if (embedder.reembed2DRequired(result.graph, (float) point.x, (float) point.y)) {
                    resetButton.setEnabled(false);
                    resetButton.requestFocus();
                    resetButton.setText("re-embedding ...");
                    EmbedThread embedThread = resultPanel.getEmbedThread();
                    if (embedThread != null) {
                        embedThread.embed(result, TwoViewPanel.this, false, false, true);
                    }
                }
            }
        });
        painter = new TwoViewPainter(this);
    }

    void getVertexID() {
        String buttonAction = vertexSizeGroup.getSelection().getActionCommand();
        if (buttonAction.charAt(0) == '+') {
            showNumbers = true;
        } else if (buttonAction.charAt(0) == '-') {
            showNumbers = false;
        }
        vertexID = Integer.parseInt(buttonAction.substring(1));
        getVertexImage();
    }

    void getVertexImage() {
        try {
            tracker.waitForID(vertexID);
        } catch (InterruptedException ex) {
            Debug.reportException(ex);
        }
        vertexImage = vertexImageArray[vertexID];
        vertexSize = Math.max(vertexImage.getWidth(null), vertexImage.getHeight(null));
        getVertexFont();
    }

    void getVertexFont() {
        if ((vertexFont = vertexFontArray[vertexID]) == null) {
            vertexFont = getFont();
            FontMetrics fm = getFontMetrics(vertexFont);
            int w = fm.stringWidth(Integer.toString(graphSize)), h = fm.getAscent();
            int fontSize;
            double factor = vertexSize * 0.85 / Math.sqrt(w * w + h * h);
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

    public void stop() {
    }

    public void setResultPanel(ResultPanel resultPanel) {
        this.resultPanel = resultPanel;
    }

    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        boolean reembed2DEnabled = generatorInfo.isReembed2DEnabled();
        resetButton.setVisible(reembed2DEnabled);
        reembed2DDisabled = !reembed2DEnabled;
        embedder = generatorInfo.getEmbedder();
    }

    public void showNumbers(boolean showNumbers) {
        this.showNumbers = showNumbers;
        if (showNumbers && vertexFont.getSize() <= 0) {
            showNumbersButton.setSelected(this.showNumbers = false);
            int oldID = vertexID;
            while (++vertexID < vertexSizes) {
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

    public boolean getShowNumbers() {
        return showNumbers;
    }


    public void highlightFaces(boolean highlightFaces) {
        painter.setHighlightFaces(highlightFaces);

        repaint();
    }

    public void setHighlightedFaces(int highlightedFacesSize) {
        painter.setHighlightedFacesSize(highlightedFacesSize);

        repaint();
    }

    public void setEdgeWidth(int edgeWidth) {
        this.edgeWidth = edgeWidth;
        repaint();
    }

    public int getEdgeWidth() {
        return edgeWidth;
    }

    public int getVertexSize() {
        return vertexSize;
    }

    public void setEdgeBrightness(float brightness) {
        edgeColor = new Color(brightness, brightness, brightness);
        specialEdgeColor = new Color((brightness + 0.25f)/2, 0.4f + (brightness + 0.25f)/2, (brightness + 0.25f)/2);
        repaint();
    }

    public void showResult(CaGeResult result) {
        this.result = result;
        graphChanged();
    }

    public void resetEmbedding() {
        EmbedThread embedThread = resultPanel.getEmbedThread();
        if (embedThread != null) {
            embedThread.embed(result, this, true, false, false);
        }
    }

    public void propertyChange(PropertyChangeEvent e) {
        final CaGeResult cageResult = (CaGeResult) e.getNewValue();
        SwingUtilities.invokeLater(new Runnable() {

            public void run() {
                embeddingChanged(cageResult);
            }
        });
    }

    public void embeddingChanged(CaGeResult result) {
        if (result != this.result) {
            return;
        }
        resultPanel.embeddingModified(this.twoView, result);
        graphChanged();
    }

    void graphChanged() {
        graphSize = result.graph.getSize();
        painter.setGraph(result.graph);
        for (int i = 0; i < vertexFontArray.length; ++i) {
            vertexFontArray[i] = null;
        }
        boolean showNumbersOld = this.showNumbers;
        getVertexFont();
        showNumbers(showNumbersOld);
        resetButton.setText("reset embedding");
        resetButton.setEnabled(result.reembed2DMade);
    }

    void viewportChanged() {
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
    Color specialEdgeColor = new Color(0.5f, 0.9f, 0.5f);
    Color numbersColor = new Color(0.25f, 0.25f, 1.0f);

    @Override
    public void paintComponent(Graphics graphics) {
        super.paintComponent(graphics);
        if (result.graph == null) {
            return;
        }
        this.graphics = graphics;
        Color oldColor = graphics.getColor();
        painter.paintGraph();
        graphics.setColor(oldColor);
    }
    int px[] = new int[4], py[] = new int[4];
    FontMetrics fontMetrics = null;
    int ascent = 0;

    public void beginGraph() {
    }

    public void beginEdges() {
        if (edgeWidth <= 0) {
            return;
        }
        graphics.setColor(edgeColor);
    }

    public void paintEdge(double x1, double y1, double x2, double y2, int v1, int v2, boolean useSpecialColour) {
        if(useSpecialColour)
            graphics.setColor(specialEdgeColor);
        int xp1, yp1, xp2, yp2;
        xp1 = (int) Math.floor(x1);
        yp1 = (int) Math.floor(y1);
        xp2 = (int) Math.floor(x2);
        yp2 = (int) Math.floor(y2);
        if (edgeWidth <= 0) {
            return;
        } else if (edgeWidth == 1) {
            graphics.drawLine(xp1, yp1, xp2, yp2);
        } else {
            double w = edgeWidth + 0.2;
            double dx = yp1 - yp2, dy = xp2 - xp1;
            double d = Math.sqrt(dx * dx + dy * dy);
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

        //Reset colour
        if(useSpecialColour)
            graphics.setColor(edgeColor);
    }

    public void beginVertices() {
        if (showNumbers && vertexFont.getSize() > 0) {
            graphics.setFont(vertexFont);
            fontMetrics = graphics.getFontMetrics();
            ascent = fontMetrics.getAscent();
        }
    }

    public void paintVertex(double x, double y, int number) {
        int xp = (int) Math.floor(x), yp = (int) Math.floor(y);
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
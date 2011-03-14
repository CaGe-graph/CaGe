package cage.viewer.twoview;


import cage.CaGe;
import cage.CaGeResult;
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
import java.util.Enumeration;
import javax.swing.AbstractButton;
import javax.swing.Box;
import javax.swing.ButtonGroup;
import javax.swing.JCheckBox;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.SwingUtilities;

import lisken.systoolbox.Systoolbox;

/**
 *
 */
public class TwoViewPanel extends JPanel
        implements TwoViewDevice {

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
    
    private boolean showNumbers = false;
    private int graphSize, vertexID;
    private CaGeResult result = null;
    private Embedder embedder = null;
    private boolean reembed2DDisabled = false;
    private Image vertexImage;
    private Font[] vertexFontArray;
    private Font vertexFont;
    private AbstractButton showNumbersButton;

    private ResultPanel resultPanel;
    private TwoViewPainter painter;
    private TwoView twoView;

    private TwoViewModel model;

    public TwoViewPanel(TwoView twoView, TwoViewModel model,
            Container titlePanel1, Font titleFont) {
        this.twoView = twoView;
        this.model = model;
        try {
            setPreferredSize(new Dimension(
                    Integer.parseInt(CaGe.config.getProperty("TwoView.Width")),
                    Integer.parseInt(CaGe.config.getProperty("TwoView.Height"))));
        } catch (Exception ex) {
        }
        ActionListener sizeButtonListener = new ActionListener() {

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

        final JCheckBox highlightFacesButton = new JCheckBox("Highlight faces of size: ", false);
        final JFormattedTextField highlightedFacesSizeField = new JFormattedTextField(5);
        
        highlightFacesButton.setFont(titleFont);
        highlightFacesButton.setAlignmentY(0.5f);
        highlightFacesButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                TwoViewPanel.this.model.setHighlightFaces(highlightFacesButton.isSelected());
            }
        });
        titlePanel1.add(highlightFacesButton);

        highlightedFacesSizeField.setEnabled(highlightFacesButton.isSelected());
        highlightedFacesSizeField.setColumns(4);
        highlightedFacesSizeField.setAlignmentY(0.5f);
        highlightedFacesSizeField.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        TwoViewPanel.this.model.setHighlightedFacesSize((Integer)highlightedFacesSizeField.getValue());
                    }
                });
            }
        });
        highlightedFacesSizeField.addFocusListener(new FocusAdapter() {

            @Override
            public void focusLost(FocusEvent e) {
                SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        TwoViewPanel.this.model.setHighlightedFacesSize((Integer)highlightedFacesSizeField.getValue());
                    }
                });
            }
        });
        titlePanel1.add(highlightedFacesSizeField);

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
        model.setEdgeWidth(DEFAULT_EDGE_WIDTH); //TODO: move to model
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
                if (embedder.reembed2DRequired(result.getGraph(), (float) point.x, (float) point.y)) {
                    //TODO: reembed2DRequired at the same time stores the coordinates
                    TwoViewPanel.this.model.reembedGraph(resultPanel.getEmbedThread());
                }
            }
        });
        painter = new TwoViewPainter(this, model);

        this.model.addTwoViewListener(new TwoViewAdapter() {

            @Override
            public void edgeWidthChanged() {
                repaint();
            }

            @Override
            public void highlightedFacesChanged() {
                highlightFacesButton.setSelected(TwoViewPanel.this.model.highlightFaces());
                highlightedFacesSizeField.setEnabled(TwoViewPanel.this.model.highlightFaces());
                highlightedFacesSizeField.setValue(TwoViewPanel.this.model.getHighlightedFacesSize());
                repaint();
            }
        });
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
        
        //TODO: cleaner separation of MVC
        model.setVertexSize(
                Math.max(vertexImage.getWidth(null), vertexImage.getHeight(null))
                );

        getVertexFont();
    }

    void getVertexFont() {
        if ((vertexFont = vertexFontArray[vertexID]) == null) {
            vertexFont = getFont();
            FontMetrics fm = getFontMetrics(vertexFont);
            int w = fm.stringWidth(Integer.toString(graphSize)), h = fm.getAscent();
            int fontSize;
            double factor = model.getVertexSize() * 0.85 / Math.sqrt(w * w + h * h);
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
            //TODO: cleaner separation of MVC
            model.setShowNumbers(showNumbers);
        }
    }

    public void stop() {
    }

    public void setResultPanel(ResultPanel resultPanel) {
        this.resultPanel = resultPanel;
    }

    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        boolean reembed2DEnabled = generatorInfo.isReembed2DEnabled();
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
        
        //TODO: move the logic above to the model
        model.setShowNumbers(this.showNumbers);

        repaint();
    }

    public boolean getShowNumbers() {
        return showNumbers;
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
        model.resetEmbedding(resultPanel.getEmbedThread());
    }

    public void embeddingChanged(CaGeResult result) {
        if (result != this.result) {
            return;
        }
        resultPanel.embeddingModified(this.twoView, result);
        graphChanged();
    }

    void graphChanged() {
        graphSize = result.getGraph().getSize();
        painter.setGraph(result.getGraph());
        for (int i = 0; i < vertexFontArray.length; ++i) {
            vertexFontArray[i] = null;
        }
        boolean showNumbersOld = this.showNumbers;
        getVertexFont();
        showNumbers(showNumbersOld);
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
        if (result.getGraph() == null) {
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
        if (model.getEdgeWidth() <= 0) {
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
        if (model.getEdgeWidth() <= 0) {
            return;
        } else if (model.getEdgeWidth() == 1) {
            graphics.drawLine(xp1, yp1, xp2, yp2);
        } else {
            double w = model.getEdgeWidth() + 0.2;
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
        if (model.getEdgeWidth() > 0) {
            graphics.setColor(edgeColor);
            graphics.fillOval(xp - (model.getEdgeWidth() - 1) / 2, yp - (model.getEdgeWidth() - 1) / 2, model.getEdgeWidth(), model.getEdgeWidth());
        }
        graphics.drawImage(vertexImage, xp - model.getVertexSize() / 2, yp - model.getVertexSize() / 2, null);
        if (showNumbers && vertexFont.getSize() > 0) {
            String numberString = Integer.toString(number);
            graphics.setColor(numbersColor);
            int width = fontMetrics.stringWidth(numberString);
            graphics.drawString(numberString, xp - (int) Math.floor(width * 0.52), yp + (int) Math.floor(ascent * 0.47));
        }
    }
}
package cage.viewer.twoview;


import cage.CaGe;
import cage.CaGeResult;
import cage.Embedder;
import cage.GeneratorInfo;
import cage.ResultPanel;
import cage.utility.Debug;
import cage.viewer.TwoView;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Insets;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.InputEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.JPanel;


/**
 *
 */
public class TwoViewPanel extends JPanel {
    
    private Embedder embedder = null;
    private boolean reembed2DDisabled = false;

    private ResultPanel resultPanel;
    private TwoViewPainter painter;
    private TwoView twoView;
    private GraphicsTwoViewDevice twoViewDevice;

    private TwoViewModel model;

    public TwoViewPanel(TwoView twoView, TwoViewModel model) {
        this.twoView = twoView;
        this.model = model;
        this.twoViewDevice = new GraphicsTwoViewDevice(model);

        try {
            setPreferredSize(new Dimension(
                    Integer.parseInt(CaGe.config.getProperty("TwoView.Width")),
                    Integer.parseInt(CaGe.config.getProperty("TwoView.Height"))));
        } catch (Exception ex) {
            Debug.reportException(ex);
        }

        painter = new TwoViewPainter(twoViewDevice, model);


        //listeners
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
                if (embedder.reembed2DRequired(TwoViewPanel.this.model.getResult().getGraph(), (float) point.x, (float) point.y)) {
                    //TODO: reembed2DRequired at the same time stores the coordinates
                    //      At least document this side effect somewhere
                    TwoViewPanel.this.model.reembedGraph(resultPanel.getEmbedThread());
                }
            }
        });
        this.model.addTwoViewListener(new TwoViewAdapter() {

            @Override
            public void edgeWidthChanged() {
                repaint();
            }

            @Override
            public void edgeBrightnessChanged() {
                repaint();
            }

            @Override
            public void vertexNumbersShownChanged() {
                repaint();
            }

            @Override
            public void vertexSizeChanged() {
                repaint();
            }

            @Override
            public void highlightedFacesChanged() {
                repaint();
            }

            @Override
            public void reembeddingFinished(CaGeResult caGeResult) {
                repaint();
            }

            @Override
            public void resultChanged() {
                repaint();
            }
        });
    }

    public void setResultPanel(ResultPanel resultPanel) {
        this.resultPanel = resultPanel;
    }

    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        boolean reembed2DEnabled = generatorInfo.isReembed2DEnabled();
        reembed2DDisabled = !reembed2DEnabled;
        embedder = generatorInfo.getEmbedder();
    }

    public void showResult(CaGeResult result) {
        graphChanged();
    }

    public void resetEmbedding() {
        model.resetEmbedding(resultPanel.getEmbedThread());
    }

    public void embeddingChanged(CaGeResult result) {
        resultPanel.embeddingModified(this.twoView, result);
        graphChanged();
    }

    void graphChanged() {
        painter.setGraph(model.getResult().getGraph());
        repaint();
    }

    void viewportChanged() {
        Insets insets = getInsets();
        painter.setPaintArea(
                insets.left + (twoViewDevice.getMaxVertexSize() - 1) / 2,
                getWidth() - insets.right - twoViewDevice.getMaxVertexSize() / 2,
                getHeight() - insets.bottom - (twoViewDevice.getMaxVertexSize() - 1) / 2,
                insets.top + twoViewDevice.getMaxVertexSize() / 2);
        repaint();
    }

    @Override
    public void paintComponent(Graphics g) {
        super.paintComponent(g);
        if (model.getResult().getGraph() == null) {
            return;
        }
        g = g.create();
        g.setFont(getFont());
        twoViewDevice.setGraphics(g);
        painter.paintGraph();
    }
}
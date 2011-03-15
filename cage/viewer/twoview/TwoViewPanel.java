package cage.viewer.twoview;


import cage.CaGe;
import cage.CaGeResult;
import cage.Embedder;
import cage.GeneratorInfo;
import cage.ResultPanel;
import cage.viewer.TwoView;

import java.awt.Container;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Insets;
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
import javax.swing.AbstractButton;
import javax.swing.Box;
import javax.swing.ButtonGroup;
import javax.swing.JCheckBox;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.SwingUtilities;


/**
 *
 */
public class TwoViewPanel extends JPanel {
    
    private boolean showNumbers = false;
    private CaGeResult result = null;
    private Embedder embedder = null;
    private boolean reembed2DDisabled = false;

    private ResultPanel resultPanel;
    private TwoViewPainter painter;
    private TwoView twoView;
    private GraphicsTwoViewDevice twoViewDevice;

    private TwoViewModel model;

    public TwoViewPanel(TwoView twoView, TwoViewModel model,
            Container titlePanel1, Font titleFont) {
        this.twoView = twoView;
        this.model = model;
        this.twoViewDevice = new GraphicsTwoViewDevice(model);

        final AbstractButton[] sizeButtonArray = new AbstractButton[model.getVertexSizesCount()];
        final ButtonGroup vertexSizeGroup = new ButtonGroup();
        for (int i = 0; i < model.getVertexSizesCount();i++) {
            AbstractButton sizeButton = new JRadioButton(Integer.toString(i + 1));
            sizeButton.setActionCommand(" " + i);
            sizeButton.setAlignmentY(0.5f);
            if (i < 10) {
                sizeButton.setMnemonic(KeyEvent.VK_1 + i % 10);
            }
            vertexSizeGroup.add(sizeButton);
            sizeButtonArray[i] = sizeButton;
        }

        try {
            setPreferredSize(new Dimension(
                    Integer.parseInt(CaGe.config.getProperty("TwoView.Width")),
                    Integer.parseInt(CaGe.config.getProperty("TwoView.Height"))));
        } catch (Exception ex) {
        }
        ActionListener sizeButtonListener = new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                TwoViewPanel.this.model.setVertexSizeID(Integer.parseInt(e.getActionCommand().substring(1)));
            }
        };
        JLabel sizeLabel = new JLabel("vertex size:");
        sizeLabel.setFont(titleFont);
        titlePanel1.add(sizeLabel);
        titlePanel1.add(Box.createHorizontalStrut(5));
        
        for (int i = 0; i < model.getVertexSizesCount(); ++i) {
            AbstractButton sizeButton = sizeButtonArray[i];
            sizeButton.setFont(titleFont);
            titlePanel1.add(Box.createHorizontalStrut(5));
            titlePanel1.add(sizeButton);
            sizeButton.addActionListener(sizeButtonListener);
        }
        titlePanel1.add(Box.createHorizontalStrut(10));
        final JCheckBox showNumbersButton = new JCheckBox("Numbers", showNumbers);
        showNumbersButton.setFont(titleFont);
        showNumbersButton.setMnemonic(KeyEvent.VK_N);
        showNumbersButton.setAlignmentY(0.5f);
        showNumbersButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                TwoViewPanel.this.model.setShowNumbers(showNumbersButton.isSelected());
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

        sizeButtonArray[model.getVertexSizeID()].setSelected(true);
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
                if (embedder.reembed2DRequired(result.getGraph(), (float) point.x, (float) point.y)) {
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
                showNumbersButton.setSelected(TwoViewPanel.this.model.getShowNumbers());
                repaint();
            }

            @Override
            public void vertexSizeChanged() {
                repaint();
            }

            @Override
            public void vertexSizeIDChanged() {
                sizeButtonArray[TwoViewPanel.this.model.getVertexSizeID()].setSelected(true);
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

    public void setResultPanel(ResultPanel resultPanel) {
        this.resultPanel = resultPanel;
    }

    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        boolean reembed2DEnabled = generatorInfo.isReembed2DEnabled();
        reembed2DDisabled = !reembed2DEnabled;
        embedder = generatorInfo.getEmbedder();
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
        painter.setGraph(model.getResult().getGraph());
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
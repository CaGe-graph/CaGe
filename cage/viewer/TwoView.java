package cage.viewer;

import cage.viewer.twoview.TwoViewPanel;
import cage.CaGe;
import cage.CaGeResult;
import cage.EmbeddableGraph;
import cage.GeneratorInfo;
import cage.NativeEmbeddableGraph;
import cage.ResultPanel;
import cage.SavePSDialog;
import cage.StaticGeneratorInfo;
import cage.utility.Debug;
import cage.utility.SaveActionListener;
import cage.viewer.twoview.PostScriptTwoViewPainter;
import cage.viewer.twoview.SvgTwoViewPainter;
import cage.viewer.twoview.TikzTwoViewPainter;
import cage.viewer.twoview.TwoViewAdapter;
import cage.viewer.twoview.TwoViewModel;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.GridLayout;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusAdapter;
import java.awt.event.FocusEvent;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import javax.imageio.ImageIO;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFormattedTextField;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.SwingUtilities;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.systoolbox.MutableInteger;
import lisken.uitoolbox.PushButtonDecoration;
import lisken.uitoolbox.SpinButton;
import lisken.uitoolbox.UItoolbox;

public class TwoView implements CaGeViewer {

    private JFrame frame;
    private JLabel title;
    private final Box titlePanel;
    private final JPanel savePanel;
    private TwoViewPanel twoViewPanel;
    private GeneratorInfo generatorInfo;
    private CaGeResult result;
    private final JButton savePSButton;
    private SavePSDialog savePSDialog;
    private Map<MutableInteger, String> psFilenames = new HashMap<>();
    private PostScriptTwoViewPainter psTwoViewPainter;
    private TwoViewModel model;
    private List<JButton> saveButtons = new ArrayList<>();
    private JSlider rotationSlider = new JSlider(JSlider.HORIZONTAL, -180, 180, 0);

    public TwoView() {
        model = new TwoViewModel();
        psTwoViewPainter = new PostScriptTwoViewPainter(model);

        title = new JLabel("TwoView diagrams");
        Font titleFont = title.getFont();
        titleFont = new Font(
                titleFont.getName(),
                titleFont.getStyle() & ~Font.BOLD,
                titleFont.getSize());
        title.setForeground(Color.black);
        title.setFont(titleFont);
        title.setAlignmentY(0.5f);
        JPanel titlePanel1 = new JPanel();
        titlePanel1.setLayout(new BoxLayout(titlePanel1, BoxLayout.X_AXIS));
        titlePanel1.add(title);
        titlePanel1.add(Box.createHorizontalStrut(20));
        titlePanel1.add(Box.createHorizontalGlue());
        JLabel sizeLabel = new JLabel("vertex size:");
        sizeLabel.setFont(titleFont);
        titlePanel1.add(sizeLabel);
        titlePanel1.add(Box.createHorizontalStrut(5));
        
        final JSlider vertexSizeSlider =
                new JSlider(TwoViewModel.MIN_VERTEX_SIZE,
                        TwoViewModel.MAX_VERTEX_SIZE,
                        model.getVertexSize());
        vertexSizeSlider.addChangeListener(new ChangeListener() {

            @Override
            public void stateChanged(ChangeEvent e) {
                model.setVertexSize(vertexSizeSlider.getValue());
            }
        });
        titlePanel1.add(vertexSizeSlider);
        titlePanel1.add(Box.createHorizontalStrut(10));

        final JCheckBox showNumbersButton = new JCheckBox("Numbers", model.getShowNumbers());
        showNumbersButton.setFont(titleFont);
        showNumbersButton.setMnemonic(KeyEvent.VK_N);
        showNumbersButton.setAlignmentY(0.5f);
        showNumbersButton.addActionListener(new ActionListener() {

            @Override
            public void actionPerformed(ActionEvent e) {
                model.setShowNumbers(showNumbersButton.isSelected());
            }
        });
        titlePanel1.add(showNumbersButton);

        final JCheckBox highlightFacesButton = new JCheckBox("Highlight faces of size: ", false);
        final JFormattedTextField highlightedFacesSizeField = new JFormattedTextField(5);

        highlightFacesButton.setFont(titleFont);
        highlightFacesButton.setAlignmentY(0.5f);
        highlightFacesButton.addActionListener(new ActionListener() {

            @Override
            public void actionPerformed(ActionEvent e) {
                model.setHighlightFaces(highlightFacesButton.isSelected());
            }
        });
        titlePanel1.add(highlightFacesButton);

        highlightedFacesSizeField.setEnabled(highlightFacesButton.isSelected());
        highlightedFacesSizeField.setColumns(4);
        highlightedFacesSizeField.setAlignmentY(0.5f);
        highlightedFacesSizeField.addActionListener(new ActionListener() {

            @Override
            public void actionPerformed(ActionEvent e) {
                SwingUtilities.invokeLater(new Runnable() {
                    @Override
                    public void run() {
                        model.setHighlightedFacesSize((Integer)highlightedFacesSizeField.getValue());
                    }
                });
            }
        });
        highlightedFacesSizeField.addFocusListener(new FocusAdapter() {

            @Override
            public void focusLost(FocusEvent e) {
                SwingUtilities.invokeLater(new Runnable() {
                    @Override
                    public void run() {
                        model.setHighlightedFacesSize((Integer)highlightedFacesSizeField.getValue());
                    }
                });
            }
        });
        titlePanel1.add(highlightedFacesSizeField);
        
        final JSlider edgeBrightnessSlider =
                new JSlider(0, 15, (int) Math.round(20 * model.getEdgeBrightness()));
        edgeBrightnessSlider.setPreferredSize(new Dimension(20, edgeBrightnessSlider.getPreferredSize().height));
        edgeBrightnessSlider.addChangeListener(new ChangeListener() {
            @Override
            public void stateChanged(ChangeEvent e) {
                model.setEdgeBrightness(edgeBrightnessSlider.getValue() / 20.0f);
            }
        });
        JLabel edgeBrightnessLabel = new JLabel("edge brightness:");
        edgeBrightnessLabel.setFont(titleFont);
        edgeBrightnessLabel.setLabelFor(edgeBrightnessSlider);
        edgeBrightnessLabel.setDisplayedMnemonic(KeyEvent.VK_B);
        final SpinButton edgeWidthButton = new SpinButton(
                TwoViewModel.DEFAULT_EDGE_WIDTH,
                TwoViewModel.MIN_EDGE_WIDTH, TwoViewModel.MAX_EDGE_WIDTH);
        edgeWidthButton.setMaximumSize(edgeWidthButton.getPreferredSize());
        edgeWidthButton.addChangeListener(new ChangeListener() {
            @Override
            public void stateChanged(ChangeEvent e) {
                model.setEdgeWidth(edgeWidthButton.getValue());
            }
        });
        JLabel edgeWidthLabel = new JLabel("width:");
        edgeWidthLabel.setFont(titleFont);
        edgeWidthLabel.setLabelFor(edgeWidthButton);
        edgeWidthLabel.setDisplayedMnemonic(KeyEvent.VK_W);

        final JButton resetButton = new JButton("reset embedding");
        resetButton.setFont(titleFont);
        resetButton.setBorder(BorderFactory.createEmptyBorder(3, 7, 5, 7));
        PushButtonDecoration.decorate(resetButton);
        resetButton.setMnemonic(KeyEvent.VK_R);
        resetButton.setAlignmentY(0.5f);
        resetButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                resetButton.setText("re-embedding ...");
                resetButton.setEnabled(false);
                twoViewPanel.requestFocus();
                twoViewPanel.resetEmbedding();
            }
        });
        model.addTwoViewListener(new TwoViewAdapter(){

            @Override
            public void vertexNumbersShownChanged() {
                showNumbersButton.setSelected(model.getShowNumbers());
            }

            @Override
            public void vertexSizeChanged() {
                vertexSizeSlider.setValue(model.getVertexSize());
            }

            @Override
            public void highlightedFacesChanged() {
                highlightFacesButton.setSelected(model.highlightFaces());
                highlightedFacesSizeField.setEnabled(model.highlightFaces());
                highlightedFacesSizeField.setValue(model.getHighlightedFacesSize());
            }

            @Override
            public void edgeWidthChanged() {
                edgeWidthButton.setValue(model.getEdgeWidth());
            }

            @Override
            public void edgeBrightnessChanged() {
                edgeBrightnessSlider.setValue((int)(20*model.getEdgeBrightness()));
            }

            @Override
            public void generatorInfoChanged() {
                resetButton.setVisible(model.getGeneratorInfo().isReembed2DEnabled());
            }

            @Override
            public void prepareReembedding() {
                resetButton.setEnabled(false);
                resetButton.requestFocus();
                resetButton.setText("re-embedding ...");
            }

            @Override
            public void reembeddingFinished(final CaGeResult caGeResult) {
                resetButton.setText("reset embedding");
                resetButton.setEnabled(result.isReembed2DMade());
                SwingUtilities.invokeLater(new Runnable() {

                    @Override
                    public void run() {
                        twoViewPanel.embeddingChanged(caGeResult);
                    }
                });
            }

            @Override
            public void startReembedding() {
            }

            @Override
            public void resultChanged() {
                resetButton.setText("reset embedding");
                resetButton.setEnabled(result.isReembed2DMade());

                title.setText(model.getGraphComment());
                savePSDialog.setInfo(model.getGraphComment());
            }
        });

        JPanel titlePanel2 = new JPanel();
        titlePanel2.setLayout(new BoxLayout(titlePanel2, BoxLayout.X_AXIS));
        titlePanel2.setBorder(BorderFactory.createEmptyBorder(5, 0, 5, 0));
        // the next line adds several buttons to titlePanel1, one to titlePanel2
        twoViewPanel = new TwoViewPanel(this, model);
        titlePanel2.add(resetButton);
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
        titlePanel = new Box(BoxLayout.Y_AXIS);
        titlePanel.add(titlePanel1);
        titlePanel.add(titlePanel2);
        savePSButton = new JButton("save PS");
        savePSButton.setFont(titleFont);
        savePSButton.setBorder(BorderFactory.createEmptyBorder(3, 7, 5, 7));
        savePSButton.setMnemonic(KeyEvent.VK_P);
        savePSButton.setAlignmentY(0.5f);
        savePSButton.setActionCommand("s");
        savePSButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                savePSButtonPressed();
            }
        });

        saveButtons.add(savePSButton);

        //TODO:rework other buttons and components

        addSaveButton("Save PNG", titleFont,
            new SaveActionListener(frame, new File(CaGe.config.getProperty("CaGe.Generators.RunDir")), ".png") {

                @Override
                protected void save(File file) {
                    BufferedImage im =new BufferedImage(
                        twoViewPanel.getWidth(), twoViewPanel.getHeight(), BufferedImage.TYPE_INT_ARGB);
                    twoViewPanel.paintComponent(im.getGraphics());
                    try {
                        ImageIO.write(im, "PNG", file);
                    } catch (IOException ex) {
                        Debug.reportException(ex);
                    }
                }
            }, KeyEvent.VK_G);

        addSaveButton("Save SVG", titleFont,
            new SaveActionListener(frame, new File(CaGe.config.getProperty("CaGe.Generators.RunDir")), ".svg") {

                SvgTwoViewPainter svgTwoViewPainter = new SvgTwoViewPainter(model);

                @Override
                protected void save(File file) {
                    svgTwoViewPainter.setGraph(model.getResult().getGraph());
                    svgTwoViewPainter.setSvgDimension(twoViewPanel.getSize());
                    svgTwoViewPainter.setRotation(twoViewPanel.getRotation());
                    svgTwoViewPainter.paintGraph();
                    try (FileWriter writer = new FileWriter(file)) {
                        writer.write(svgTwoViewPainter.getSvgContent());
                    } catch (IOException ex) {
                        Debug.reportException(ex);
                    }
                }
            }, KeyEvent.VK_V);

        addSaveButton("Save TikZ", titleFont,
            new SaveActionListener(frame, new File(CaGe.config.getProperty("CaGe.Generators.RunDir")), ".tikz") {

                TikzTwoViewPainter tikzTwoViewPainter = new TikzTwoViewPainter(model);

                @Override
                protected void save(File file) {
                    tikzTwoViewPainter.setGraph(model.getResult().getGraph());
                    //TODO: throws nullpointer exception because paint area is not initialized
                    tikzTwoViewPainter.setRotation(twoViewPanel.getRotation());
                    tikzTwoViewPainter.paintGraph();
                    try (FileWriter writer = new FileWriter(file)) {
                        writer.write(tikzTwoViewPainter.getTikzContent());
                    } catch (IOException ex) {
                        Debug.reportException(ex);
                    }
                }
            }, KeyEvent.VK_T);

        savePanel = new JPanel();
        savePanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        for (JButton button : saveButtons) {
            savePanel.add(button);
        }
        
        rotationSlider.addChangeListener(new ChangeListener() {

            @Override
            public void stateChanged(ChangeEvent e) {
                twoViewPanel.setRotation(rotationSlider.getValue());
            }
        });
        
        createFrame();
        savePSDialog = new SavePSDialog("save Postscript");
        savePSDialog.setNearComponent(savePSButton);
    }

    private void createFrame() {
        if (frame != null) {
            return;
        }
        frame = new JFrame("CaGe - TwoView");
        JPanel southPanel = new JPanel(new GridLayout(0, 1));
        southPanel.add(rotationSlider);
        southPanel.add(savePanel);
        JPanel content = (JPanel) frame.getContentPane();
        content.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        content.add(titlePanel, BorderLayout.NORTH);
        content.add(twoViewPanel, BorderLayout.CENTER);
        content.add(southPanel, BorderLayout.SOUTH);
        frame.pack();
    }

    private void addSaveButton(String caption, Font font, ActionListener listener, int mnemonic){
        JButton saveButton = new JButton(caption);
        saveButton.setFont(font);
        saveButton.setBorder(BorderFactory.createEmptyBorder(3, 7, 5, 7));
        saveButton.setMnemonic(mnemonic);
        saveButton.setAlignmentY(0.5f);
        saveButton.addActionListener(listener);
        saveButtons.add(saveButton);
    }

    public void savePSButtonPressed() {
        savePSButton.getModel().setArmed(false);
        savePSButton.getModel().setPressed(true);
        savePSDialog.setVisible(true);
        if (savePSDialog.getSuccess()) {
            psTwoViewPainter.setRotation(twoViewPanel.getRotation());
            psTwoViewPainter.savePostScript(
                    savePSDialog.getFilename(),
                    savePSDialog.includeInfo() ? savePSDialog.getInfo() : null);
            psFilenames.put(new MutableInteger(result.getGraphNo()), savePSDialog.getFilename());
        }
        savePSButton.getModel().setPressed(false);
        savePSButton.setSelected(result.getSaved2DPS() > 0);
    }

    @Override
    public void setDimension(int dimension) {
        if (dimension != 2) {
            throw new RuntimeException("TwoView is for 2D viewing only");
        }
    }

    @Override
    public int getDimension() {
        return 2;
    }

    @Override
    public void setResultPanel(ResultPanel resultPanel) {
        twoViewPanel.setResultPanel(resultPanel);
    }

    @Override
    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        this.generatorInfo = generatorInfo;
        twoViewPanel.setGeneratorInfo(generatorInfo);
    }

    @Override
    public void setVisible(boolean isVisible) {
        if (isVisible) {
            createFrame();
            frame.setVisible(true);
        } else if (frame != null) {
            frame.dispose();
            frame = null;
        }
    }

    @Override
    public void outputResult(CaGeResult result) {
        CaGeResult previousResult = this.result;
        this.result = result;
        model.setResult(result);
        EmbeddableGraph graph = result.getGraph();
        int graphNo = result.getGraphNo();
        savePSButton.setSelected(result.getSaved2DPS() > 0);

        String filename = psFilenames.get(new MutableInteger(graphNo));
        if (filename == null && previousResult != null) {
            String previousFilename, previousNumber;
            int p;
            previousFilename =
                    psFilenames.get(new MutableInteger(previousResult.getGraphNo()));
            if (previousFilename == null) {
                previousFilename = savePSDialog.getFilename();
            }
            previousNumber = "-" + previousResult.getGraphNo();
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

    @Override
    public void stop() {
        psTwoViewPainter.finishPostScriptFiles();
        psFilenames = new HashMap<>();
        setVisible(false);
    }

    public static void main(String[] argv) throws Exception {
        CaGe.config.clear();
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




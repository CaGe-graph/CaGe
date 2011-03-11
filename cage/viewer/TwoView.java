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
import cage.viewer.twoview.PostScriptTwoViewDevice;
import cage.viewer.twoview.TwoViewAdapter;
import cage.viewer.twoview.TwoViewModel;

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
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Hashtable;
import java.util.Properties;
import javax.imageio.ImageIO;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.JToggleButton;
import javax.swing.SwingUtilities;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.systoolbox.MutableInteger;
import lisken.uitoolbox.PushButtonDecoration;
import lisken.uitoolbox.SpinButton;
import lisken.uitoolbox.UItoolbox;

public class TwoView implements ActionListener, CaGeViewer {

    private JFrame frame;
    private JLabel title;
    private Box titlePanel;
    private JPanel savePanel;
    private TwoViewPanel twoViewPanel;
    private GeneratorInfo generatorInfo;
    private ResultPanel resultPanel;
    private CaGeResult result;
    private JSlider edgeBrightnessSlider;
    private JToggleButton savePSButton;
    private SavePSDialog savePSDialog;
    private Hashtable psFilenames = new Hashtable();
    private PostScriptTwoViewDevice psTwoViewDevice;
    private TwoViewModel model;

    public TwoView() {
        model = new TwoViewModel();
        psTwoViewDevice = new PostScriptTwoViewDevice(model);

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
        edgeBrightnessSlider =
                new JSlider(0, 15, (int) Math.round(20 * model.getEdgeBrightness()));
        edgeBrightnessSlider.setPreferredSize(new Dimension(20, edgeBrightnessSlider.getPreferredSize().height));
        edgeBrightnessSlider.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                model.setEdgeBrightness(edgeBrightnessSlider.getValue() / 20.0f);
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
        new PushButtonDecoration(resetButton);
        resetButton.setMnemonic(KeyEvent.VK_R);
        resetButton.setAlignmentY(0.5f);
        resetButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                resetButton.setText("re-embedding ...");
                resetButton.setEnabled(false);
                twoViewPanel.requestFocus();
                twoViewPanel.resetEmbedding();
            }
        });
        model.addTwoViewListener(new TwoViewAdapter(){
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
            }
        });

        JPanel titlePanel2 = new JPanel();
        titlePanel2.setLayout(new BoxLayout(titlePanel2, BoxLayout.X_AXIS));
        titlePanel2.setBorder(BorderFactory.createEmptyBorder(5, 0, 5, 0));
        // the next line adds several buttons to titlePanel1, one to titlePanel2
        twoViewPanel = new TwoViewPanel(this, model, titlePanel1, titleFont);
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
        try {
            model.setEdgeBrightness(Float.valueOf(
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

        //TODO:rework other buttons and components

        JToggleButton savePNGButton = new JToggleButton("Save PNG");
        savePNGButton.setFont(titleFont);
        savePNGButton.setBorder(BorderFactory.createEmptyBorder(3, 7, 5, 7));
        new PushButtonDecoration(savePNGButton);
        savePNGButton.setMnemonic(KeyEvent.VK_G);
        savePNGButton.setAlignmentY(0.5f);
        savePNGButton.addActionListener(new ActionListener() {

            JFileChooser fileChooser =
                    new JFileChooser(new File(
                            CaGe.config.getProperty("CaGe.Generators.RunDir")));

            public void actionPerformed(ActionEvent e) {
                if(fileChooser.showSaveDialog(frame)==JFileChooser.APPROVE_OPTION){
                    File f = fileChooser.getSelectedFile();
                    if(!f.getAbsolutePath().toLowerCase().endsWith(".png")){
                        f = new File(f.getAbsolutePath() + ".png");
                    }
                    //TODO: maybe ask before overwriting file
                    BufferedImage im =
                            new BufferedImage(twoViewPanel.getWidth(),
                                              twoViewPanel.getHeight(),
                                              BufferedImage.TYPE_INT_ARGB);
                    twoViewPanel.paintComponent(im.getGraphics());
                    try {
                        ImageIO.write(im, "PNG", f);
                    } catch (IOException ex) {
                        Debug.reportException(ex);
                    }
                }
            }
        });

        savePanel = new JPanel();
        savePanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        savePanel.add(savePSButton);
        savePanel.add(savePNGButton);
        createFrame();
        savePSDialog = new SavePSDialog("save Postscript");
        savePSDialog.setNearComponent(savePSButton);

        model.addTwoViewListener(new TwoViewAdapter() {

            @Override
            public void edgeBrightnessChanged() {
                twoViewPanel.setEdgeBrightness(model.getEdgeBrightness());
            }

            @Override
            public void edgeWidthChanged() {
                twoViewPanel.setEdgeWidth(model.getEdgeWidth());
            }
        });
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
            psTwoViewDevice.savePostScript(
                    savePSDialog.getFilename(),
                    savePSDialog.includeInfo() ? savePSDialog.getInfo() : null);
            psFilenames.put(new MutableInteger(result.getGraphNo()), savePSDialog.getFilename());
        }
        savePSButton.getModel().setPressed(false);
        savePSButton.setSelected(result.getSaved2DPS() > 0);
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
        model.setResult(result);
        EmbeddableGraph graph = result.getGraph();
        int graphNo = result.getGraphNo();
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
        savePSButton.setSelected(result.getSaved2DPS() > 0);
        String filename = (String) psFilenames.get(new MutableInteger(graphNo));
        if (filename == null && previousResult != null) {
            String previousFilename, previousNumber;
            int p;
            previousFilename =
                    (String) psFilenames.get(new MutableInteger(previousResult.getGraphNo()));
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

    public void stop() {
        twoViewPanel.stop();
        psTwoViewDevice.finishPostScriptFiles();
        psFilenames = new Hashtable();
        setVisible(false);
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




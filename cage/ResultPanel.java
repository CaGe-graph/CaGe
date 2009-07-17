package cage;

import cage.viewer.CaGeViewer;
import cage.writer.CaGeWriter;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.beans.PropertyChangeEvent;
import java.lang.reflect.Array;
import java.util.NoSuchElementException;
import java.util.TreeMap;
import java.util.Vector;
import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
import javax.swing.BoundedRangeModel;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.JToggleButton;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.systoolbox.MutableInteger;
import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.JTextComponentFocusSelector;
import lisken.uitoolbox.NumberDocument;
import lisken.uitoolbox.PushButtonDecoration;
import lisken.uitoolbox.SpinButton;
import lisken.uitoolbox.UItoolbox;

public class ResultPanel extends JPanel implements
        ActionListener, FocusListener, GeneratorListener, EmbedThreadListener {

    public static final int EXCEPTION_LEVEL = 1,  RUN_LEVEL = 2,  FOLDNET_LEVEL = 3,  ADVANCE_LEVEL = 4,  EMBED_LEVEL = 5;
    static boolean debug = false;
    static final int graphNoFireInterval = CaGe.getCaGePropertyAsInt("CaGe.GraphNoFireInterval.Foreground", 0);
    static final int graphNoFirePeriod = CaGe.getCaGePropertyAsInt("CaGe.GraphNoFirePeriod.Foreground", 1000);
    CaGePipe generator;
    GeneratorInfo generatorInfo;
    Embedder embedder;
    EmbedThread embedThread;
    CaGeResultList results;
    CaGeViewer[] viewers;
    CaGeWriter[] writers;
    CaGeTimer timer = null;
    boolean doEmbed2D, doEmbed3D;
    boolean useViewers, useWriters;
    boolean stopping;
    boolean running;
    int highestGeneratedGraphNo;
    JTextField pipeGraphNo, viewGraphNo;
    SpinButton advanceDist;
    JButton advance1Button, advanceButton;
    JButton reviewPrevButton, reviewNextButton;
    GraphNoLabel reviewPrevLabel, reviewCurrLabel, reviewNextLabel;
    JToggleButton flowButton;
    JComponent previousFocusOwner;
    JPanel statusPanel;
    Font statusFont;
    JLabel status;
    TreeMap statusMap;
    ActionListener stopListener;
    AbstractButton saveAdjButton, save2DButton, save3DButton;
    CaGeWriter saveAdjWriter, save2DWriter, save3DWriter;
    AbstractButton foldnetButton;
    SavePSDialog foldnetDialog;

    public ResultPanel(CaGePipe gen, GeneratorInfo info,
            boolean doEmbed2D, boolean doEmbed3D,
            Vector viewerV, Vector writerV) {
        setLayout(new GridBagLayout());

        pipeGraphNo = new JTextField(CaGe.graphNoDigits);
        pipeGraphNo.setHorizontalAlignment(SwingConstants.RIGHT);
        pipeGraphNo.setText("0");
        pipeGraphNo.setEnabled(false);
        pipeGraphNo.setToolTipText("number of graphs generated so far");
        add(new JLabel("generated:"),
                new GridBagConstraints(0, 0, 1, 1, 0.001, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(5, 5, 5, 20), 0, 0));
        add(pipeGraphNo,
                new GridBagConstraints(2, 0, 1, 1, 0.001, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL,
                new Insets(5, 2, 5, 2), 0, 0));
        add(Box.createHorizontalGlue(),
                new GridBagConstraints(3, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL,
                new Insets(5, 0, 5, 0), 0, 0));

        viewGraphNo = new JTextField(CaGe.graphNoDigits);
        viewGraphNo.setHorizontalAlignment(SwingConstants.RIGHT);
        viewGraphNo.setDocument(new NumberDocument(false));
        viewGraphNo.addFocusListener(this);
        viewGraphNo.setText("0");
        viewGraphNo.setToolTipText("number of the graph currently showing -- can be edited");
        JLabel viewGraphNoLabel = new JLabel("view/goto:");
        add(viewGraphNoLabel,
                new GridBagConstraints(0, 1, 1, 1, 0.001, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(5, 5, 5, 20), 0, 0));
        new JTextComponentFocusSelector(viewGraphNo);
        add(viewGraphNo,
                new GridBagConstraints(2, 1, 1, 1, 0.001, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL,
                new Insets(5, 2, 5, 2), 0, 0));

        JLabel saveLabel = new JLabel("save:");
        add(saveLabel,
                new GridBagConstraints(0, 2, 1, 1, 0.001, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(5, 5, 5, 20), 0, 0));
        foldnetButton = new JButton("folding net");
        foldnetButton.setMnemonic(KeyEvent.VK_N);
        foldnetButton.setBorder(BorderFactory.createEmptyBorder(2, 5, 2, 5));
        new PushButtonDecoration(foldnetButton);
        saveAdjButton = new JButton("adj");
        saveAdjButton.setMnemonic(KeyEvent.VK_A);
        saveAdjButton.setBorder(BorderFactory.createEmptyBorder(2, 5, 2, 5));
        new PushButtonDecoration(saveAdjButton);
        save2DButton = new JButton("2D");
        save2DButton.setMnemonic(KeyEvent.VK_2);
        save2DButton.setBorder(BorderFactory.createEmptyBorder(2, 5, 2, 5));
        new PushButtonDecoration(save2DButton);
        save3DButton = new JButton("3D");
        save3DButton.setMnemonic(KeyEvent.VK_3);
        save3DButton.setBorder(BorderFactory.createEmptyBorder(2, 5, 2, 5));
        new PushButtonDecoration(save3DButton);
        JPanel savePanel = new JPanel();
        savePanel.setLayout(new FlowLayout(FlowLayout.LEFT, 4, 0));
        savePanel.add(saveAdjButton);
        savePanel.add(save2DButton);
        savePanel.add(save3DButton);
        savePanel.add(foldnetButton);
        int big_flag = (doEmbed2D && doEmbed3D) ? 1 : 0;
        add(savePanel,
                new GridBagConstraints(2 - big_flag, 2, 2 + big_flag, 1, 100.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(5, 0, 5, 0), 0, 0));

        JLabel advanceLabel = new JLabel("advance:");
        add(advanceLabel,
                new GridBagConstraints(0, 3, 1, 1, 0.001, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(20, 5, 15, 20), 0, 0));
        advance1Button = new JButton("+1");
        advance1Button.setMnemonic(KeyEvent.VK_1);
        advance1Button.setBorder(BorderFactory.createEmptyBorder(2, 5, 2, 5));
        new PushButtonDecoration(advance1Button);
        advanceDist = new SpinButton(10, 2, Integer.parseInt(
                Systoolbox.multiply("9", CaGe.graphNoDigits - 1) + "8"));
//    advanceDist.setMajorAdjust(1);
        advanceButton = new JButton();
        advanceButton.setBorder(BorderFactory.createEmptyBorder(2, 5, 2, 5));
        new PushButtonDecoration(advanceButton);
        new AdvanceListener(advanceDist.getModel(), advanceButton);
        flowButton = new JToggleButton("flow");
        flowButton.setMnemonic(KeyEvent.VK_F);
        flowButton.setBorder(BorderFactory.createEmptyBorder(2, 5, 2, 5));
        new PushButtonDecoration(flowButton, true);
        add(advance1Button,
                new GridBagConstraints(1, 3, 1, 1, 0.001, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(20, 2, 15, 2), 0, 0));
        add(advanceDist,
                new GridBagConstraints(2, 3, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL,
                new Insets(20, 2, 15, 2), 0, 0));
        JPanel advanceFlowPanel = new JPanel();
        advanceFlowPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        advanceFlowPanel.add(advanceButton);
        advanceFlowPanel.add(Box.createHorizontalStrut(4));
        advanceFlowPanel.add(flowButton);
        add(advanceFlowPanel,
                new GridBagConstraints(3, 3, 1, 1, 100.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(20, 2, 15, 2), 0, 0));

        JPanel reviewPanel = new JPanel();
        reviewPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 5, 2));
        reviewPrevButton = new JButton("", new ImageIcon(ClassLoader.getSystemResource("Images/rev-left.gif")));
        reviewPrevButton.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEtchedBorder(), BorderFactory.createEmptyBorder(2, 5, 2, 6)));
        reviewNextButton = new JButton("", new ImageIcon(ClassLoader.getSystemResource("Images/rev-right.gif")));
        reviewNextButton.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEtchedBorder(), BorderFactory.createEmptyBorder(2, 6, 2, 5)));
        reviewPrevLabel = new GraphNoLabel("\u00a0");
        reviewCurrLabel = new GraphNoLabel("\u00a0");
        reviewCurrLabel.setForeground(Color.black);
        reviewNextLabel = new GraphNoLabel("\u00a0");
        reviewPanel.add(reviewPrevLabel);
        reviewPanel.add(reviewPrevButton);
        reviewPanel.add(reviewCurrLabel);
        reviewPanel.add(reviewNextButton);
        reviewPanel.add(reviewNextLabel);
        JLabel reviewLabel = new JLabel("review:");
        add(reviewLabel,
                new GridBagConstraints(0, 4, 1, 1, 0.001, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(15, 5, 15, 20), 0, 0));
        add(reviewPanel,
                new GridBagConstraints(1, 4, 3, 1, 0.001, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(5, 5, 5, 5), 0, 0));

        add(Box.createHorizontalStrut(400),
                new GridBagConstraints(0, 5, 5, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 0, 0), 0, 0));

        statusPanel = new JPanel();
        statusPanel.setLayout(new BoxLayout(statusPanel, BoxLayout.X_AXIS));
        statusPanel.setBackground(Color.lightGray);
        statusPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        status = new JLabel(gen == null ? "Status" : "\u00a0");
        status.setAlignmentY(0.5f);
        statusFont = status.getFont();
        statusFont = new Font(
                statusFont.getName(),
                statusFont.getStyle() & ~Font.BOLD,
                statusFont.getSize());
        status.setFont(statusFont);
        statusPanel.add(status);
        statusPanel.add(Box.createHorizontalGlue());
        statusPanel.add(Box.createVerticalStrut(
                getFontMetrics(statusFont).getHeight() + 8));
        add(statusPanel,
                new GridBagConstraints(0, 6, 5, 1, 1.0, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.HORIZONTAL,
                new Insets(5, 5, 5, 5), 0, 0));
        statusMap = new TreeMap();

        setBorder(BorderFactory.createTitledBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createCompoundBorder(
                BorderFactory.createEmptyBorder(10, 10, 10, 10),
                BorderFactory.createEtchedBorder()),
                BorderFactory.createEmptyBorder(20, 20, 20, 20)), " Result Selection "));

        previousFocusOwner = advance1Button;
        if (gen == null) {
            return;
        }
        generator = gen;
        generatorInfo = info;
        embedder = generatorInfo.getEmbedder();
        this.doEmbed2D = doEmbed2D;
        this.doEmbed3D = doEmbed3D;
        useViewers = viewerV != null && viewerV.size() > 0;
        useWriters = writerV != null && writerV.size() > 0;
        viewers = (CaGeViewer[]) createArray(useViewers, viewerV, CaGeViewer.class);
        writers = (CaGeWriter[]) createArray(useWriters, writerV, CaGeWriter.class);
        if (useViewers) {
            for (int i = 0; i < viewers.length; ++i) {
                viewers[i].setResultPanel(this);
            }
        }

        viewGraphNoLabel.setVisible(useViewers);
        viewGraphNo.setVisible(useViewers);
        savePanel.setVisible(useViewers);
        saveLabel.setVisible(useViewers);
        advanceLabel.setVisible(useViewers);
        advance1Button.setVisible(useViewers);
        advanceDist.setVisible(useViewers);
        advanceFlowPanel.setVisible(useViewers);
        reviewLabel.setVisible(useViewers);
        reviewPanel.setVisible(useViewers);

        saveAdjButton.setEnabled(false);
        save2DButton.setVisible(doEmbed2D);
        save2DButton.setEnabled(false);
        save3DButton.setVisible(doEmbed3D);
        save3DButton.setEnabled(false);
        saveAdjWriter = save2DWriter = save3DWriter = null;
        foldnetButton.setVisible(doEmbed3D);
        foldnetButton.setEnabled(false);
        foldnetDialog = new SavePSDialog("save folding net");
        foldnetDialog.setFilename(generatorInfo.getFilename() + "-nets.ps");
        foldnetDialog.useInfo(false);
        foldnetDialog.setNearComponent(foldnetButton);

        results = new CaGeResultList();
        highestGeneratedGraphNo = 0;
        generator.addPropertyChangeListener(this);
        if (graphNoFirePeriod > 0) {
            timer = new CaGeTimer(generator, graphNoFirePeriod);
        }

        viewGraphNo.setActionCommand("viewGraphNo");
        viewGraphNo.addActionListener(this);
        advance1Button.setActionCommand("advance1");
        advance1Button.addActionListener(this);
        advanceButton.setActionCommand("Advance");
        advanceButton.addActionListener(this);
        flowButton.setActionCommand("flow");
        flowButton.addActionListener(this);
        reviewPrevButton.setActionCommand("previous");
        reviewPrevButton.addActionListener(this);
        reviewPrevButton.setEnabled(false);
        reviewNextButton.setActionCommand("next");
        reviewNextButton.addActionListener(this);
        reviewNextButton.setEnabled(false);
        saveAdjButton.setActionCommand("sAdjacency");
        saveAdjButton.addActionListener(this);
        save2DButton.setActionCommand("s2D");
        save2DButton.addActionListener(this);
        save3DButton.setActionCommand("s3D");
        save3DButton.addActionListener(this);
        foldnetButton.setActionCommand("sFoldnet");
        foldnetButton.addActionListener(this);
    }

    Object[] createArray(boolean hasElements, Vector vector, Class type) {
        if (hasElements) {
            Object[] array = (Object[]) Array.newInstance(type, vector.size());
            vector.copyInto(array);
            return array;
        } else {
            return null;
        }
    }

    void advanceBy(int n) {
        if (generator.isRunning()) {
            setStatus("advancing to graph " + (generator.getGraphNo() + n) + " ...", ADVANCE_LEVEL);
            previousFocusOwner = advance1Button;
            try {
                generator.advanceBy(n);
            } catch (Exception ex) {
                exceptionOccurred(ex);
                advance1Button.setEnabled(true);
                advanceButton.setEnabled(true);
                if (timer != null) {
                    timer.stop();
                }
                if (previousFocusOwner != null) {
                    previousFocusOwner.requestFocus();
                }
            }
            if (debug) {
                System.err.println("advanceBy(" + n + ") completed.");
            }
        } else {
            advance1Button.setEnabled(false);
            advanceButton.setEnabled(false);
        }
    }

    void setFlowing(boolean flowing) {
        try {
            generator.setFlowing(flowing);
        } catch (Exception ex) {
            exceptionOccurred(ex);
            advance1Button.setEnabled(true);
            advanceButton.setEnabled(true);
            if (previousFocusOwner != null) {
                previousFocusOwner.requestFocus();
            }
        }
    }

    public void setStatus(String statusText, int statusLevel) {
        statusMap.put(new MutableInteger(statusLevel), statusText);
        updateStatus();
    }

    public void clearStatus(int statusLevel) {
        clearStatus(statusLevel, true);
    }

    public void clearStatus(int statusLevel, boolean update) {
        statusMap.remove(new MutableInteger(statusLevel));
        if (update) {
            updateStatus();
        }
    }

    void clearStatus() {
        statusMap = new TreeMap();
        updateStatus();
    }

    void updateStatus() {
        Object currentLevel;
        try {
            currentLevel = statusMap.firstKey();
            status.setText((String) statusMap.get(currentLevel));
        } catch (NoSuchElementException ex) {
            status.setText("\u00a0");
        }
    }

    public void start() {
        try {
            stopping = false;
            setStatus("starting ...", RUN_LEVEL);
            generator.setGraphNoFireInterval(graphNoFireInterval);
            generator.start();
            embedThread = new EmbedThread(embedder, 1);
            embedThread.setEmbedThreadListener(this);
            embedThread.start();
            clearStatus(RUN_LEVEL, false);
            setStatus("waiting for graph 1 ...", ADVANCE_LEVEL);
            advanceBy(1);
            if (debug) {
                System.err.println("start completed.");
            }
        } catch (Exception ex) {
            clearStatus(RUN_LEVEL, false);
            clearStatus(ADVANCE_LEVEL, false);
            showException(ex, "Exception at start");
        }
    }

    public void stop() {
        if (generator.isRunning()) {
            setStatus("stopping ...", RUN_LEVEL);
            stopping = true;
            if (embedThread != null) {
                embedThread.setEmbedThreadListener(null);
                embedThread.abort();
                embedThread = null;
            }
            generator.stop();
        } else {
            setStatus("already stopped.", RUN_LEVEL);
        }
        stopAll(viewers);
        stopAll(writers);
        if (saveAdjWriter != null) {
            saveAdjWriter.stop();
        }
        if (save2DWriter != null) {
            save2DWriter.stop();
        }
        if (save3DWriter != null) {
            save3DWriter.stop();
        }
        saveAdjWriter = save2DWriter = save3DWriter = null;
        CaGe.foldnetThread().removePropertyChangeListener(this);
        clearStatus(RUN_LEVEL);
        if (debug) {
            System.err.println("stopped.");
        }
    }

    public void reset() {
        stopAll(viewers);
        stopAll(writers);
        if (saveAdjWriter != null) {
            saveAdjWriter.stop();
        }
        if (save2DWriter != null) {
            save2DWriter.stop();
        }
        if (save3DWriter != null) {
            save3DWriter.stop();
        }
        saveAdjWriter = save2DWriter = save3DWriter = null;
        CaGe.foldnetThread().removePropertyChangeListener(this);
        foldnetButton.setEnabled(false);
        pipeGraphNo.setText("0");
        pipeGraphNo.setEnabled(false);
        viewGraphNo.setText("0");
        flowButton.setEnabled(true);
        results = new CaGeResultList();
        highestGeneratedGraphNo = 0;
        reviewPrevLabel.setText("\u00a0");
        reviewCurrLabel.setText("\u00a0");
        reviewNextLabel.setText("\u00a0");
        reviewPrevButton.setEnabled(false);
        reviewNextButton.setEnabled(false);
        statusPanel.removeAll();
        statusPanel.add(status);
        statusPanel.add(Box.createHorizontalGlue());
        statusPanel.add(Box.createVerticalStrut(
                getFontMetrics(statusFont).getHeight() + 8));
        clearStatus();
        UItoolbox.pack(this);
        previousFocusOwner = advance1Button;
    }

    void stopAll(CaGeOutlet[] outlet) {
        if (outlet == null) {
            return;
        }
        for (int i = 0; i < outlet.length; ++i) {
            outlet[i].stop();
        }
    }

    private void updateView() {
        CaGeResult result = results.getResult();
        EmbeddableGraph graph = result.graph;
        boolean has2D = graph.has2DCoordinates();
        boolean has3D = graph.has3DCoordinates();
        outputResult(viewers, result, has2D, has3D);
        outputResult(writers, result, has2D, has3D);
        int graphNo = result.graphNo;
        viewGraphNo.setText(Integer.toString(graphNo));
        if (viewGraphNo.hasFocus()) {
            viewGraphNo.selectAll();
        }
        saveAdjButton.setEnabled(!result.savedAdj);
        save2DButton.setEnabled(!result.saved2D);
        save3DButton.setEnabled(!result.saved3D);
        foldnetButton.setEnabled(!result.foldnetMade);
        reviewCurrLabel.setText(Integer.toString(graphNo));
        if (results.hasNext()) {
            reviewNextLabel.setText(Integer.toString(results.nextGraphNo()));
            reviewNextButton.setEnabled(true);
        } else {
            reviewNextLabel.setText("\u00a0");
            reviewNextButton.setEnabled(false);
        }
        if (results.hasPrevious()) {
            reviewPrevLabel.setText(Integer.toString(results.previousGraphNo()));
            reviewPrevButton.setEnabled(true);
        } else {
            reviewPrevLabel.setText("\u00a0");
            reviewPrevButton.setEnabled(false);
        }
    }

    void outputResult(CaGeOutlet[] outlet, CaGeResult result, boolean has2D, boolean has3D) {
        if (outlet == null) {
            return;
        }
        for (int i = 0; i < outlet.length; ++i) {
            int dimension = outlet[i].getDimension();
            if (dimension == 2 ? has2D : dimension == 3 ? has3D : true) {
                outlet[i].outputResult(result);
            }
        }
    }

    public void focusGained(FocusEvent e) {
    }

    public void focusLost(FocusEvent e) {
        Object source = e.getSource();
        if (source == viewGraphNo) {
            CaGeResult result = results.getResult();
            if (result != null) {
                viewGraphNo.setText(Integer.toString(results.getResult().graphNo));
            }
        }
    }

    public void actionPerformed(ActionEvent e) {
        String cmd = e.getActionCommand();
        switch (cmd.charAt(0)) {
            case 'v':
                viewGraphNoChanged();
                break;
            case 'a':
                advanceBy(1);
                break;
            case 'A':
                advanceBy(advanceDist.getValue());
                break;
            case 'f':
                changeFlow();
                break;
            case 'p':
                reviewPrevious();
                break;
            case 'n':
                reviewNext();
                break;
            case 's':
                switch (cmd.charAt(1)) {
                    case 'A':
                        saveAdjacency();
                        break;
                    case '2':
                        save2D();
                        break;
                    case '3':
                        save3D();
                        break;
                    case 'F':
                        saveFoldnet();
                        break;
                }
                break;
        }
    }

    void viewGraphNoChanged() {
        int pipeNo, viewNo;
        previousFocusOwner = viewGraphNo;
        pipeNo = generator.getGraphNo();
        try {
            viewNo = Integer.parseInt(viewGraphNo.getText());
        } catch (NumberFormatException ex) {
            Toolkit.getDefaultToolkit().beep();
            viewGraphNo.selectAll();
            return;
        }
        if (viewNo > pipeNo && generator.isRunning() && advanceButton.isEnabled()) {
            setStatus("advancing to graph " + viewNo + " ...", ADVANCE_LEVEL);
            advanceBy(viewNo - pipeNo);
            viewGraphNo.setText(Integer.toString(results.getGraphNo()));
            viewGraphNo.requestFocus();
        } else if (viewNo > 0 && viewNo <= pipeNo && results.findGraphNo(viewNo)) {
            results.gotoFound();
            updateView();
        } else if (viewNo == 0 && results.highestGraphNo() > 0) {
            results.gotoHighest();
            updateView();
        } else {
            viewGraphNo.setText(Integer.toString(results.getGraphNo()));
        }
        viewGraphNo.selectAll();
    }

    void changeFlow() {
        boolean flowing = flowButton.isSelected();
        if (flowing) {
            setStatus("flowing ...", ADVANCE_LEVEL);
        }
        previousFocusOwner = flowButton;
        setFlowing(flowing);
        if (debug) {
            System.err.println("setFlowing completed.");
        }
    }

    void reviewPrevious() {
        results.previous();
        updateView();
        if (!reviewPrevButton.isEnabled()) {
            reviewNextButton.requestFocus();
        }
        previousFocusOwner = reviewPrevButton;
    }

    void reviewNext() {
        results.next();
        updateView();
        if (!reviewNextButton.isEnabled()) {
            reviewPrevButton.requestFocus();
        }
        previousFocusOwner = reviewNextButton;
    }

    void saveAdjacency() {
        CaGeResult result = results.getResult();
        saveAdjWriter = saveResult(result, saveAdjWriter, saveAdjButton,
                "Adjacency", "save adjacency info");
        result.savedAdj = !saveAdjButton.isEnabled();
    }

    void save2D() {
        CaGeResult result = results.getResult();
        save2DWriter = saveResult(result, save2DWriter, save2DButton,
                "2D", "save 2D embedding");
        result.saved2D = !save2DButton.isEnabled();
    }

    void save3D() {
        CaGeResult result = results.getResult();
        save3DWriter = saveResult(result, save3DWriter, save3DButton,
                "3D", "save 3D embedding");
        result.saved3D = !save3DButton.isEnabled();
    }

    CaGeWriter saveResult(CaGeResult result, CaGeWriter saveWriter,
            AbstractButton saveButton, String variety, String dialogTitle) {
        saveButton.getModel().setPressed(true);
        String shortVariety = saveButton.getText();
        if (saveWriter == null) {
            SaveDialog saveDialog =
                    new SaveDialog(dialogTitle, variety);
            saveDialog.setFilename(generatorInfo.getFilename() + "-choice");
            saveDialog.addExtension();
            saveDialog.setNearComponent(saveButton);
            saveDialog.setVisible(true);
            if (saveDialog.getSuccess()) {
                saveWriter = saveDialog.getCaGeWriter();
                saveWriter.setGeneratorInfo(generatorInfo);
                try {
                    saveWriter.setOutputStream(
                            Systoolbox.createOutputStream(
                            saveDialog.getFilename(),
                            CaGe.config.getProperty("CaGe.Generators.RunDir")));
                } catch (Exception ex) {
                    showException(ex, "exception at " + shortVariety + "-save setup");
                    saveWriter = null;
                }
            }
            saveDialog.dispose();
        }
        saveButton.getModel().setArmed(false);
        saveButton.getModel().setPressed(false);
        if (saveWriter == null) {
            return null;
        }
        saveWriter.outputResult(result);
        if (saveWriter.wasIOException()) {
            showException(saveWriter.lastIOException(),
                    "exception while " + shortVariety + "-saving");
        } else {
            saveButton.setEnabled(false);
            if (previousFocusOwner != null) {
                previousFocusOwner.requestFocus();
            }
        }
        return saveWriter;
    }

    void saveFoldnet() {
        String oldFilename = foldnetDialog.getFilename();
        foldnetButton.getModel().setPressed(true);
        foldnetDialog.setVisible(true);
        foldnetButton.getModel().setArmed(false);
        foldnetButton.getModel().setPressed(false);
        if (foldnetDialog.getSuccess()) {
            FoldnetThread foldnetThread = CaGe.foldnetThread();
            synchronized (foldnetThread) {
                foldnetThread.removePropertyChangeListener(this);
                foldnetThread.makeFoldnet(results.getResult(),
                        generatorInfo.getMaxFacesize(), foldnetDialog.getFilename());
                int left = foldnetThread.tasksLeft();
                setStatus("Folding net enqueued - " + left + " left to make.",
                        FOLDNET_LEVEL);
                clearStatus(FOLDNET_LEVEL, false);
                foldnetThread.addPropertyChangeListener(this);
            }
            results.getResult().foldnetMade = true;
            foldnetButton.setEnabled(false);
        } else {
            foldnetDialog.setFilename(oldFilename);
        }
        if (advance1Button.isEnabled()) {
            advance1Button.requestFocus();
        } else {
            viewGraphNo.requestFocus();
        }
    }


    /*
    Handling of property changes is dispatched to the
    event handling thread.
     */
    public void propertyChange(final PropertyChangeEvent e) {
        if (debug) {
            System.err.println("property changed: " + e.getPropertyName());
        }
        Runnable handler = new Runnable() {

            public void run() {
                handlePropertyChange(e);
            }
        };
        switch (e.getPropertyName().charAt(0)) {
            case 'g':
            case 'f':
            case 'r':
                UItoolbox.invokeAndWait(handler);
                // SwingUtilities.invokeLater(handler);
                break;
            case 'e':
            case 'c':
            case 't':
                SwingUtilities.invokeLater(handler);
                break;
            default:
                if (debug) {
                    System.err.println("unexpected property: " + e.getPropertyName());
                }
                break;
        }
    }

    /*
    Contains code to handle property changes in the event thread.
     */
    void handlePropertyChange(PropertyChangeEvent e) {
        switch (e.getPropertyName().charAt(0)) {
            case 'g':
                graphNoChanged(((Integer) e.getNewValue()).intValue());
                break;
            case 'f':
                flowingChanged(((Boolean) e.getNewValue()).booleanValue());
                break;
            case 'r':
                runningChanged(((Boolean) e.getNewValue()).booleanValue());
                break;
            case 'c':
                boolean embeddingSuccess = ((Boolean) e.getOldValue()).booleanValue();
                if (embeddingSuccess) {
                    embeddingMade((CaGeResult) e.getNewValue());
                }
                break;
            case 'e':
                showException((Exception) e.getNewValue(), (String) e.getOldValue());
                break;
            case 't':
                int left = CaGe.foldnetThread().tasksLeft();
                if (left > 0) {
                    setStatus(left + " folding net" + (left == 1 ? "" : "s") + " left to make.", FOLDNET_LEVEL);
                    clearStatus(FOLDNET_LEVEL, false);
                } else {
                    setStatus("Folding net saved.", FOLDNET_LEVEL);
                    clearStatus(FOLDNET_LEVEL, false);
                }
                break;
            default:
                if (debug) {
                    System.err.println("unexpected property: " + e.getPropertyName());
                }
                break;
        }
    }

    void graphNoChanged(int graphNo) {
        if (graphNo < 0) {
            return;
        }
        pipeGraphNo.setText(Integer.toString(graphNo));
        boolean flowing = generator.isFlowing();
        if (debug) {
            System.err.println("graphNo changed: " + graphNo + ", flowing=" + flowing);
        }
        if (flowing) {
            return;
        }
        if (stopping) {
            return;
        }
        clearStatus(ADVANCE_LEVEL);
        advance1Button.setEnabled(running);
        advanceButton.setEnabled(running);
        if (timer != null) {
            timer.stop();
        }
        if (previousFocusOwner != null) {
            previousFocusOwner.requestFocus();
        }
        if (graphNo <= highestGeneratedGraphNo) {
            return;
        }
        highestGeneratedGraphNo = graphNo;
        try {
            EmbeddableGraph graph = generator.getGraph();
            setStatus("embedding graph " + graphNo + " ...", EMBED_LEVEL);
            embedThread.embed(new CaGeResult(graph, graphNo), this,
                    doEmbed2D, doEmbed3D, false);
        } catch (Exception ex) {
            exceptionOccurred(ex);
        }
    }

    void flowingChanged(boolean flowing) {
        if (debug) {
            System.err.println("flowing changed: " + flowing);
        }
        flowButton.setSelected(flowing);
        if (flowing) {
            flowButton.requestFocus();
            advance1Button.setEnabled(false);
            advanceButton.setEnabled(false);
            if (timer != null) {
                timer.start();
            }
        }
    }

    void runningChanged(boolean running) {
        if (debug) {
            System.err.println("running changed: " + running);
        }
        this.running = running;
        if (running) {
            return;
        }
        if (stopListener != null) {
            stopListener.actionPerformed(new ActionEvent(this, 0, null));
        }
        if (timer != null) {
            timer.stop();
        }
        viewGraphNo.requestFocus();
        flowButton.setSelected(false);
        flowButton.setEnabled(false);
        advance1Button.setEnabled(false);
        advanceButton.setEnabled(false);
        String finishMsg = "Generation process finished.";
        setStatus(finishMsg, RUN_LEVEL);
        final String logText = Systoolbox.getFileContent(
                CaGe.config.getProperty("CaGe.Generators.ErrFile"));
        if (logText.length() == 0) {
            setStatus(finishMsg + " (No log output.)", RUN_LEVEL);
            return;
        }
        final JButton logButton = new JButton("log file");
        logButton.setAlignmentY(0.5f);
        logButton.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEtchedBorder(), BorderFactory.createEmptyBorder(1, 3, 1, 3)));
        logButton.setFont(statusFont);
        logButton.setBackground(statusPanel.getBackground());
        logButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                UItoolbox.showTextInfo("generator's diagnostic output", logText, true);
                logButton.requestFocus();
            }
        });
        statusPanel.add(Box.createHorizontalStrut(10));
        statusPanel.add(logButton);
        UItoolbox.pack(this);
        logButton.requestFocus();
    }

    void embeddingMade(CaGeResult result) {
        clearStatus(EMBED_LEVEL);
        results.addResult(result);
        updateView();
        if (!(useViewers || stopping)) {
            advanceBy(1);
        }
    }

    void exceptionOccurred(Exception ex) {
        showException(ex, null);
    }

    public void showException(Exception ex, String context) {
        showException(ex, context, false, null);
    }

    public void showEmbeddingException(final Exception ex, final String context, final String diagnosticOutput) {
        SwingUtilities.invokeLater(new Runnable() {

            public void run() {
                showException(ex, context, true, diagnosticOutput);
            }
        });
    }

    void showException(final Exception ex, final String context,
            final boolean isEmbeddingException, final String embedDiagnostics) {
        setStatus(context == null ? "Exception occurred" : context,
                EXCEPTION_LEVEL);
        String eName = ex.getClass().getName();
        eName = eName.substring(eName.lastIndexOf(".") + 1);
        final JButton exceptionButton = new JButton(eName);
        final Component separator = Box.createHorizontalStrut(10);
        exceptionButton.setAlignmentY(0.5f);
        exceptionButton.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEtchedBorder(), BorderFactory.createEmptyBorder(1, 3, 1, 3)));
        exceptionButton.setFont(statusFont);
        exceptionButton.setBackground(statusPanel.getBackground());
        exceptionButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                StringBuffer message = new StringBuffer(Systoolbox.getStackTrace(ex));
                if (context != null) {
                    message.append("\nContext was '");
                    message.append(context);
                    message.append("'.\n");
                }
                if (isEmbeddingException) {
                    if (embedDiagnostics != null && embedDiagnostics.length() > 0) {
                        message.append("\nEmbedder's diagnostic output:\n");
                        message.append(embedDiagnostics);
                    } else {
                        message.append("\nEmbedder produced no diagnostic output.\n");
                    }
                }
                UItoolbox.showTextInfo("exception information",
                        message.toString());
                clearStatus(EXCEPTION_LEVEL);
                statusPanel.remove(separator);
                statusPanel.remove(exceptionButton);
                UItoolbox.pack(ResultPanel.this);
                statusPanel.revalidate();
                statusPanel.repaint();
                if (flowButton.isEnabled()) {
                    flowButton.requestFocus();
                } else {
                    viewGraphNo.requestFocus();
                }
            }
        });
        statusPanel.add(separator);
        statusPanel.add(exceptionButton);
        UItoolbox.pack(this);
    }

    public void embeddingFinished() {
    }

    public void setStopListener(ActionListener stopListener) {
        this.stopListener = stopListener;
    }

    public EmbedThread getEmbedThread() {
        return embedThread;
    }

    public void embeddingModified(CaGeViewer modifyingViewer, CaGeResult result) {
        int modifiedDimension = modifyingViewer.getDimension();
        EmbeddableGraph graph = result.graph;
        boolean modifiedOk = modifiedDimension == 2 ? graph.has2DCoordinates() : graph.has3DCoordinates();
        if (!modifiedOk) {
            return;
        }
        AbstractButton saveButton = modifiedDimension == 2 ? save2DButton : save3DButton;
        saveButton.setEnabled(true);
        if (modifiedDimension == 2) {
            result.saved2D = false;
        } else {
            result.saved3D = false;
        }
        if (viewers == null) {
            return;
        }
        for (int i = 0; i < viewers.length; ++i) {
            if (viewers[i] != modifyingViewer) {
                int viewerDimension = viewers[i].getDimension();
                if (viewerDimension == 0 || viewerDimension == modifiedDimension) {
                    viewers[i].outputResult(result);
                }
            }
        }
    }

    public static void main(String argv[]) {
        JFrame f;
        ResultPanel r;
        r = new ResultPanel(null, null, false, false, null, null);
        f = new JFrame("CaGe - result window");
        f.setContentPane(r);
        f.addWindowListener(new WindowAdapter() {

            public void windowClosing(WindowEvent e) {
                System.exit(0);
            }
        });
        UItoolbox.addExitOnEscape(f);
        f.pack();
        f.setVisible(true);
    }

    private class AdvanceListener implements ChangeListener {

        private AbstractButton advanceButton;

        public AdvanceListener(BoundedRangeModel m, AbstractButton b) {
            advanceButton = b;
            stateChanged(new ChangeEvent(m));
            m.addChangeListener(this);
        }

        public void stateChanged(ChangeEvent e) {
            advanceButton.setText("+" + ((BoundedRangeModel) e.getSource()).getValue());
            advanceButton.setMnemonic(KeyEvent.VK_ADD);
        }
    }

    private class GraphNoLabel extends JLabel {

        private Dimension minimumSize = new Dimension();

        public GraphNoLabel(String text) {
            super(text);
            setHorizontalAlignment(SwingConstants.CENTER);
            FontMetrics metrics = getFontMetrics(getFont());
            minimumSize.width = metrics.stringWidth(Systoolbox.multiply("0", CaGe.graphNoDigits)) + 4;
            minimumSize.height = metrics.getHeight();
        }

        public Dimension getMinimumSize() {
            return minimumSize;
        }

        public Dimension getPreferredSize() {
            return minimumSize;
        }
    }
}
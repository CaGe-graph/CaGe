package cage.viewer;

import cage.CaGeResult;
import cage.EmbeddableGraph;
import cage.GeneratorInfo;
import cage.ResultPanel;
import cage.viewer.jmol.JmolPanel;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Insets;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.SwingConstants;

/**
 *
 * @author nvcleemp
 */
public class CaGeJmolViewer implements CaGeViewer{

    private int dimension = 0;
    private ResultPanel resultPanel;
    private JLabel comment;
    private JFrame frame;
    private JmolPanel jmolPanel = new JmolPanel();

    public CaGeJmolViewer() {
        comment = new JLabel("\u00a0");
        comment.setHorizontalAlignment(SwingConstants.CENTER);
    }

    private void createFrame() {
        if (frame != null) {
            return;
        }
        frame = new JFrame("CaGe - Jmol Viewer");
        frame.setLayout(new BorderLayout());
        frame.add(comment, BorderLayout.NORTH);
        frame.add(jmolPanel, BorderLayout.CENTER);
        frame.addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                setVisible(false);
            }
        });
    }

    public void setResultPanel(ResultPanel resultPanel) {
        this.resultPanel = resultPanel;
    }

    public void setVisible(boolean isVisible) {
        if (isVisible) {
            createFrame();
            setViewerSize(jmolPanel.getPreferredSize().width, jmolPanel.getPreferredSize().height);
            frame.setVisible(true);
        } else if (frame != null) {
            frame.dispose();
            frame = null;
        }
    }

    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        jmolPanel.setGeneratorInfo(generatorInfo);
    }

    public void setDimension(int d) {
        dimension = d;
    }

    public int getDimension() {
        return dimension;
    }

    public void outputResult(CaGeResult result) {
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
        comment.setText(graphComment);
        createFrame();
        jmolPanel.setGraph(graph);
        if (frame.isVisible()) {
            frame.validate();
        } else {
            setVisible(true);
        }
    }

    public void stop() {
        setVisible(false);
    }

    public void setViewerSize(int width, int height) {
        Insets insets = frame.getInsets();
        Dimension commentSize = comment.getPreferredSize();
        frame.setSize(
                Math.max(width, commentSize.width) + insets.left + insets.right,
                height + commentSize.height + insets.top + insets.bottom);
    }
}

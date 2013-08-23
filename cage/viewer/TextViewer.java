package cage.viewer;

import cage.CaGeResult;
import cage.EmbeddableGraph;
import cage.GeneratorInfo;
import cage.ResultPanel;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Point;
import java.awt.Rectangle;
import javax.swing.BorderFactory;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.border.AbstractBorder;
import javax.swing.border.Border;

import lisken.uitoolbox.AutosizedTextArea;
import lisken.uitoolbox.UItoolbox;

public class TextViewer implements CaGeViewer {

    private JFrame frame;
    private JPanel content;
    private JTextArea text,  header;
    private JLabel title;
    private JScrollPane scrollPane;

    @Override
    public void outputResult(CaGeResult result) {
        EmbeddableGraph graph = result.getGraph();
        int graphNo = result.getGraphNo();
        title.setText(
                "Graph " + graphNo + " - " + graph.getSize() + " vertices (in writegraph format)");
        String headerText = "   n" + (graph.has2DCoordinates() ? "\t\t x(2D)\t y(2D)" : "") + (graph.has3DCoordinates() ? "\t\t x(3D)\t y(3D)\t z(3D)" : "") + "\t\t  adj";
        String graphText = graph.toString();
        text.setText(graphText);
        text.setCaretPosition(0);
        scrollPane.getViewport().setViewPosition(new Point(0, 0));
        header.setText(headerText);
        header.setSize(text.getPreferredSize());
        scrollPane.setPreferredSize(null);
        UItoolbox.restrictScrollPaneToScreenPart(scrollPane, 0.8f, 0.8f);
        createFrame();
        frame.pack();
        if (!frame.isVisible()) {
            frame.setVisible(true);
        }
        // the following three duplicate, apparently pointless lines
        // deal with a bug in Swing for Java 1.1.x
        text.setText(graphText);
        text.setCaretPosition(0);
        scrollPane.getViewport().setViewPosition(new Point(0, 0));
    }

    public TextViewer() {
        content = new JPanel();
        text = new AutosizedTextArea();
        text.setFont(new Font("Monospaced", 0, text.getFont().getSize()));
        text.setTabSize(5);
        text.setEditable(false);
        text.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        scrollPane = new JScrollPane(text);
        title = new JLabel();
        header = new JTextArea() {

            @Override
            public Dimension getPreferredSize() {
                return new Dimension(text.getPreferredSize().width + 10,
                        super.getPreferredSize().height + 2);
            }

            @Override
            public void paintComponent(Graphics g) {
                Color oldColor = g.getColor();
                g.setColor(getBackground());
                Rectangle clip = g.getClipBounds();
                g.fillRect(clip.x, clip.y, clip.x + clip.width, clip.y + clip.height);
                g.setColor(oldColor);
                g.translate(5, 1);
                super.paintComponent(g);
                g.translate(-5, -1);
            }
        };
        header.setBackground(title.getBackground());
        header.setFont(text.getFont());
        header.setTabSize(text.getTabSize());
        header.setEnabled(false);
        Border underlineBorder = new AbstractBorder() {

            @Override
            public boolean isBorderOpaque() {
                return false;
            }

            @Override
            public void paintBorder(Component c, Graphics g, int x, int y, int width, int height) {
                Color oldColor = g.getColor();
                g.setColor(Color.black);
                g.drawLine(0, height - 1, width - 1, height - 1);
                g.setColor(oldColor);
            }
        };
        header.setBorder(underlineBorder);
        scrollPane.setColumnHeaderView(header);
        JPanel corner = new JPanel();
        corner.setBorder(underlineBorder);
        scrollPane.setCorner(JScrollPane.UPPER_RIGHT_CORNER, corner);
        content.setLayout(new GridBagLayout());
        content.add(title,
                new GridBagConstraints(0, 0, 1, 1, 1.0, 0.001,
                GridBagConstraints.CENTER, GridBagConstraints.NONE,
                new Insets(5, 2, 5, 2), 0, 0));
        content.add(scrollPane,
                new GridBagConstraints(0, 3, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.BOTH,
                new Insets(2, 2, 2, 2), 0, 0));
        createFrame();
    }

    private void createFrame() {
        if (frame != null) {
            return;
        }
        frame = new JFrame("CaGe - TextViewer");
        frame.setContentPane(content);
    }

    @Override
    public void setResultPanel(ResultPanel resultPanel) {
    }

    @Override
    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
    }

    @Override
    public void setDimension(int d) {
    }

    @Override
    public int getDimension() {
        return 0;
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
    public void stop() {
        setVisible(false);
    }

    /*
    public void stateChanged (ChangeEvent e)
    {
    }
     */
    public static void main(String[] argv) {
        TextViewer t;
        t = new TextViewer();
        t.title.setText("graph encodings");
        t.text.setText("\n\n\tDon't close this window, press ESC instead.\n\n\n");
        t.header.setText("header line, scrolls with main area");
        t.frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        UItoolbox.addExitOnEscape(t.frame);
        t.frame.pack();
        t.setVisible(true);
    }
}


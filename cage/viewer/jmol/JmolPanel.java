package cage.viewer.jmol;

/**
 *
 * @author nvcleemp
 */
import cage.CaGe;
import cage.EmbeddableGraph;
import cage.GeneratorInfo;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Rectangle;
import java.util.HashMap;
import java.util.Map;
import javax.swing.JPanel;

import org.jmol.api.JmolViewer;

public class JmolPanel extends JPanel {

    private JmolViewer viewer;
    private CaGeJmolAdapter adapter;
    private final Map<String,String> storedCommands = new HashMap<>();

    public JmolPanel() {
        super();
        adapter = new CaGeJmolAdapter();
        int configWidth = CaGe.getCaGePropertyAsInt("CaGeJmolViewer.Width", 550);
        int configHeight = CaGe.getCaGePropertyAsInt("CaGeJmolViewer.Height", 500);
        preferredSize = new Dimension(configWidth, configHeight);
        viewer = JmolViewer.allocateViewer(this, adapter);
        viewer.setIntProperty("logLevel", 2);
        viewer.setColorBackground(jmolColour(getBackground()));
        viewer.evalString("zap");
    }

    public void setGraph(EmbeddableGraph graph){
        /*
         * First we set the graph on the adapter and then we trigger the viewer
         * to reread the graph from the adapter.
         */
        adapter.setGraph(graph);
        viewer.openDOM(null);
        for (String value : storedCommands.values()) {
            viewer.evalString(value);
        }
        viewer.evalString("delay;");
    }

    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        adapter.setGeneratorInfo(generatorInfo);
    }

    public JmolViewer getViewer() {
        return viewer;
    }

    final Dimension currentSize = new Dimension();
    final Rectangle rectClip = new Rectangle();
    final Dimension preferredSize;

    @Override
    public void paint(Graphics g) {
        if(viewer==null)
            return;
        getSize(currentSize);
        g.getClipBounds(rectClip);
        viewer.renderScreenImage(g, currentSize, rectClip);
    }

    @Override
    public Dimension getPreferredSize() {
        return preferredSize;
    }

    private String jmolColour(Color c) {
        String x = Integer.toHexString(c.getRGB() & 0x00ffffff), y = "#000000";
        return y.substring(0, 7 - x.length()) + x;
    }
    
    //package private!
    /*
    This allows us to store commands that need to be run each time a new graph is loaded.
    */
    void storeCommand(String key, String command){
        storedCommands.put(key, command);
    }
}

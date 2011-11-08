package cage.viewer.twoview;

import cage.utility.Debug;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

/**
 * Implementation of TwoViewSaver that saves the graph as a TikZ picture.
 *
 * @author nvcleemp
 */
public class TikZTwoViewSaver implements TwoViewSaver {
    
    private final TikzTwoViewPainter tikzTwoViewPainter;
    private final TwoViewModel model;

    public TikZTwoViewSaver(TwoViewModel model) {
        this.model = model;
        tikzTwoViewPainter = new TikzTwoViewPainter(model);
    }

    public void saveFile(File file) {
        tikzTwoViewPainter.setGraph(model.getResult().getGraph());
        tikzTwoViewPainter.paintGraph();
        try {
            FileWriter writer = new FileWriter(file);
            writer.write(tikzTwoViewPainter.getTikzContent());
            writer.close();
        } catch (IOException ex) {
            Debug.reportException(ex);
        }
    }
    
}

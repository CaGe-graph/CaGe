package cage.viewer.twoview;

import cage.CaGe;
import cage.utility.Debug;

import java.awt.Dimension;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

/**
 * Implementation of TwoViewSaver that saves the graph as a SVG image.
 * 
 * @author nvcleemp
 */
public class SvgTwoViewSaver implements TwoViewSaver {
    
    private final SvgTwoViewPainter svgTwoViewPainter;
    private final TwoViewModel model;

    public SvgTwoViewSaver(TwoViewModel model) {
        this.model = model;
        svgTwoViewPainter = new SvgTwoViewPainter(model);
    }
    
    @Override
    public void saveFile(File file){
        svgTwoViewPainter.setGraph(model.getResult().getGraph());
        svgTwoViewPainter.setSvgDimension(new Dimension(
                    Integer.parseInt(CaGe.config.getProperty("TwoView.Width")),
                    Integer.parseInt(CaGe.config.getProperty("TwoView.Height"))));
        svgTwoViewPainter.setRotation(0);
        svgTwoViewPainter.paintGraph();
        try {
            FileWriter writer = new FileWriter(file);
            writer.write(svgTwoViewPainter.getSvgContent());
            writer.close();
        } catch (IOException ex) {
            Debug.reportException(ex);
        }
    }
}

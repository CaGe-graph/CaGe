/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package cage.viewer.twoview;

import cage.CaGe;

import cage.utility.Debug;

import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;

/**
 * Implementation of TwoViewSaver that saves the graph as a PNG image.
 *
 * @author nvcleemp
 */
public class PngTwoViewSaver implements TwoViewSaver{
    
    private final GraphicsTwoViewPainter graphicsTwoViewPainter;
    private final TwoViewModel model;
    private final int width;
    private final int height;

    public PngTwoViewSaver(TwoViewModel model) {
        this.model = model;
        graphicsTwoViewPainter = new GraphicsTwoViewPainter(model);
        width = Integer.parseInt(CaGe.config.getProperty("TwoView.Width"));
        height = Integer.parseInt(CaGe.config.getProperty("TwoView.Height"));
        graphicsTwoViewPainter.setPaintArea(
                5 + (graphicsTwoViewPainter.getMaxVertexSize() - 1) / 2,
                width - 5 - graphicsTwoViewPainter.getMaxVertexSize() / 2,
                height - 5 - (graphicsTwoViewPainter.getMaxVertexSize() - 1) / 2,
                5 + graphicsTwoViewPainter.getMaxVertexSize() / 2);
    }

    public void saveFile(File file) {
        BufferedImage im = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
        graphicsTwoViewPainter.setGraph(model.getResult().getGraph());
        
        graphicsTwoViewPainter.setGraphics(im.createGraphics());
        graphicsTwoViewPainter.paintGraph();
        
        try {
            ImageIO.write(im, "PNG", file);
        } catch (IOException ex) {
            Debug.reportException(ex);
        }
    }
    
}

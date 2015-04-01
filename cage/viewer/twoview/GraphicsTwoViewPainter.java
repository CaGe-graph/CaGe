/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package cage.viewer.twoview;

import cage.CaGeResult;

import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.GradientPaint;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import javax.swing.SwingUtilities;

/**
 * An implementation of {@code TwoViewPainter} that prints the graph to a
 * {@code Graphics} object.
 * 
 * @author nvcleemp
 */
public class GraphicsTwoViewPainter extends TwoViewPainter {
    
    private static final Color DEFAULT_VERTEX_COLOR = new Color(250, 178, 126);
    
    private Graphics graphics;

    private Color edgeColor;
    private Color specialEdgeColor;
    private final Color numbersColor = new Color(0.25f, 0.25f, 1.0f);
    private Font[] vertexFontArray;

    private final TwoViewListener listener = new TwoViewAdapter() {

        @Override
        public void resultChanged() {
            invalidateVertexFonts();
        }

        @Override
        public void reembeddingFinished(CaGeResult caGeResult) {
            //TODO: check whether these fonts need to be invalidated after re-embedding
            invalidateVertexFonts();
        }

        @Override
        public void vertexSizeChanged() {
            determineVertexFont();
        }

        @Override
        public void vertexNumbersShownChanged() {
            handleShowNumbers();
        }

        @Override
        public void edgeBrightnessChanged() {
            calculateColors();
        }
    };

    public GraphicsTwoViewPainter(TwoViewModel model) {
        super(model);
        initializeVertexFonts();
        calculateColors();
        
        this.model.addTwoViewListener(listener);
    }

    private void initializeVertexFonts(){
        vertexFontArray = new Font[model.getVertexSizesCount()];
    }
    
    private void calculateColors(){
        float brightness = model.getEdgeBrightness();
        edgeColor = new Color(brightness, brightness, brightness);
        specialEdgeColor = new Color((brightness + 0.25f)/2, 0.4f + (brightness + 0.25f)/2, (brightness + 0.25f)/2);
    }

    public void setGraphics(Graphics graphics) {
        this.graphics = graphics;
        determineVertexFont_impl();
        handleShowNumbers();
    }

    public int getMaxVertexSize() {
        return TwoViewModel.MAX_VERTEX_SIZE;
    }

    /*
     * This method determines the font used for the label of the vertices for
     * the current vertex size and then checks whether the labels can be
     * displayed for this vertex size. If this font was already determined,
     * then this method only determines whether the labels can be displayed.
     */
    private void determineVertexFont() {
        if(graphics==null)
            return;

        determineVertexFont_impl();

        model.setShowNumbers(model.getShowNumbers() && vertexFontArray[model.getVertexSize()-TwoViewModel.MIN_VERTEX_SIZE].getSize()>0);
    }

    /*
     * This method determines the font used for the label of the vertices for
     * the current vertex size.
     */
    private void determineVertexFont_impl() {
        if (vertexFontArray[model.getVertexSize()-TwoViewModel.MIN_VERTEX_SIZE] == null) {
            Font vertexFont = graphics.getFont();
            int fontSize = getVertexFontSize(model.getVertexSize()-TwoViewModel.MIN_VERTEX_SIZE);
            vertexFontArray[model.getVertexSize()-TwoViewModel.MIN_VERTEX_SIZE] = new Font(
                    vertexFont.getName(),
                    vertexFont.getStyle() & Font.BOLD,
                    fontSize);
        }
    }

    private int getVertexFontSize(int vertexSizeID) {
        if(graphics==null)
            return -1;
        if (vertexFontArray[vertexSizeID] == null) {
            Font vertexFont = graphics.getFont();
            FontMetrics fm = graphics.getFontMetrics(vertexFont);
            int w = fm.stringWidth(Integer.toString(model.getResult().getGraph().getSize()));
            int h = fm.getAscent();
            int fontSize;
            double factor = (vertexSizeID + TwoViewModel.MIN_VERTEX_SIZE) * 0.85 / Math.sqrt(w * w + h * h);
            if (h * factor < 7.5) {
                fontSize = 0;
            } else {
                fontSize = (int) Math.round(vertexFont.getSize() * factor);
            }
            return fontSize;
        } else {
            return vertexFontArray[vertexSizeID].getSize();
        }
    }

    private void invalidateVertexFonts() {
        for (int i = 0; i < vertexFontArray.length; i++) {
            vertexFontArray[i]=null;
        }
        /*
         * We need to redetermine the vertex fonts because the size of the 
         * graph might have changed. This can however change whether the
         * numbers are shown. Therefore we save this state and try to 
         * restore it afterwards.
         */
        boolean showNumbers = model.getShowNumbers();
        determineVertexFont();
        model.setShowNumbers(showNumbers);
    }

    private void handleShowNumbers(){
        if(model.getShowNumbers() && getVertexFontSize(model.getVertexSize()-TwoViewModel.MIN_VERTEX_SIZE) <= 0){
            int id = model.getVertexSize()-TwoViewModel.MIN_VERTEX_SIZE + 1;
            while(id<model.getVertexSizesCount() && getVertexFontSize(id) <= 0){
                id++;
            }
            if(id<model.getVertexSizesCount()){
                final int idFinal = id;
                SwingUtilities.invokeLater(new Runnable() {
                    @Override
                    public void run() {
                        model.setVertexSize(idFinal + TwoViewModel.MIN_VERTEX_SIZE);
                    }
                });
            } else {
                //we can't display the numbers because they are to large even
                //for the largest vertex size
                SwingUtilities.invokeLater(new Runnable() {
                    @Override
                    public void run() {
                        model.setShowNumbers(false);
                    }
                });
            }
        }
    }

    //---Begin implementation TwoViewPainter---

    @Override
    protected void beginGraph() {
        if(graphics==null)
            throw new IllegalStateException("Graphics hasn't been set yet!");
        
        //turn on antialiasing
        ((Graphics2D)graphics).setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
    }

    @Override
    protected void beginEdges() {
        if (model.getEdgeWidth() <= 0) {
            return;
        }
        graphics.setColor(edgeColor);
    }

    @Override
    protected void paintEdge(double x1, double y1, double x2, double y2, int v1, int v2, boolean useSpecialColour) {
        if(useSpecialColour)
            graphics.setColor(specialEdgeColor);
        int xp1, yp1, xp2, yp2;
        xp1 = (int) Math.floor(x1);
        yp1 = (int) Math.floor(y1);
        xp2 = (int) Math.floor(x2);
        yp2 = (int) Math.floor(y2);
        if (model.getEdgeWidth() <= 0) {
            return;
        } else if (model.getEdgeWidth() == 1) {
            graphics.drawLine(xp1, yp1, xp2, yp2);
        } else {
            double w = model.getEdgeWidth() + 0.2;
            double dx = yp1 - yp2, dy = xp2 - xp1;
            double d = Math.sqrt(dx * dx + dy * dy);
            dx /= d;
            dy /= d;
            int[] px = new int[4];
            int[] py = new int[4];
            px[1] = (int) Math.round(xp1 - dx * w / 2);
            py[1] = (int) Math.round(yp1 - dy * w / 2);
            px[2] = (int) Math.round(xp2 - dx * w / 2);
            py[2] = (int) Math.round(yp2 - dy * w / 2);
            px[0] = (int) Math.round(px[1] + dx * w);
            py[0] = (int) Math.round(py[1] + dy * w);
            px[3] = (int) Math.round(px[2] + dx * w);
            py[3] = (int) Math.round(py[2] + dy * w);
            // System.err.println(i + " -> " + j + ": " + px[0] + "," + py[0] + " - " + px[1] + "," + py[1] + " - " + px[2] + "," + py[2] + " - " + px[3] + "," + py[3] + " (" + (float) dx + ", " + (float) dy + ")");
            graphics.fillPolygon(px, py, 4);
        }

        //Reset colour
        if(useSpecialColour)
            graphics.setColor(edgeColor);
    }

    @Override
    protected void beginVertices() {
        if (model.getShowNumbers() && vertexFontArray[model.getVertexSize()-TwoViewModel.MIN_VERTEX_SIZE].getSize() > 0) {
            graphics.setFont(vertexFontArray[model.getVertexSize()-TwoViewModel.MIN_VERTEX_SIZE]);
        }
    }

    @Override
    protected void paintVertex(double x, double y, int number) {
        int xp = (int) Math.floor(x), yp = (int) Math.floor(y);
        if (model.getEdgeWidth() > 0) {
            graphics.setColor(edgeColor);
            graphics.fillOval(xp - (model.getEdgeWidth() - 1) / 2, yp - (model.getEdgeWidth() - 1) / 2, model.getEdgeWidth(), model.getEdgeWidth());
        }
        
        final int vertexDiameter = model.getVertexSize();
        final int vertexCornerX = xp - vertexDiameter/2;
        final int vertexCornerY = yp - vertexDiameter/2;
        
        //calculate the two endpoints of the gradient
        final Color colorA = new Color(
                Math.min(DEFAULT_VERTEX_COLOR.getRed() + 4, 255),
                Math.max(DEFAULT_VERTEX_COLOR.getGreen() - 14, 0),
                Math.max(DEFAULT_VERTEX_COLOR.getBlue() - 26, 0));
        final Color colorB = new Color(
                Math.max(DEFAULT_VERTEX_COLOR.getRed() - 4, 0),
                Math.min(DEFAULT_VERTEX_COLOR.getGreen() + 14, 255),
                Math.min(DEFAULT_VERTEX_COLOR.getBlue() + 26, 255));
        
        ((Graphics2D)graphics).setPaint(
                new GradientPaint(
                        vertexCornerX, vertexCornerY, colorA, //from color A in upper left corner
                        vertexCornerX + vertexDiameter, vertexCornerY+vertexDiameter, colorB)); //to color B in lower right corner
        
        graphics.fillOval(vertexCornerX, vertexCornerY, vertexDiameter, vertexDiameter);
        graphics.setColor(Color.BLACK);
        graphics.drawOval(vertexCornerX, vertexCornerY, vertexDiameter, vertexDiameter);
        if (model.getShowNumbers() && vertexFontArray[model.getVertexSize()-TwoViewModel.MIN_VERTEX_SIZE].getSize() > 0) {
            String numberString = Integer.toString(number);
            graphics.setColor(numbersColor);
            int width = graphics.getFontMetrics().stringWidth(numberString);
            graphics.drawString(numberString,
                    xp - (int) Math.floor(width * 0.52),
                    yp + (int) Math.floor(graphics.getFontMetrics().getAscent() * 0.47));
        }
    }

    @Override
    protected void endGraph() {
        //do nothing
    }
}

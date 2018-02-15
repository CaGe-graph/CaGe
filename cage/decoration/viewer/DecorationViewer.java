package cage.decoration.viewer;

import cage.decoration.EmbeddedDecorationGraph;
import cage.decoration.FacetType;
import cage.decoration.Neighbour;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import javax.swing.JPanel;

/**
 * A component that can be used to show a decoration applied to certain tiling.
 * 
 * @author nvcleemp
 */
public class DecorationViewer extends JPanel {
    
    private final GeometricChamberCoordinates[] chambers;
    
    private EmbeddedDecorationGraph edg;
    private boolean drawTiling = false;

    DecorationViewer(GeometricChamberCoordinates[] chambers, int width, int height) {
        super(null);
        setSize(width, height);
        final Dimension dimension = new Dimension(width, height);
        setPreferredSize(dimension);
        setMinimumSize(dimension);
        setMaximumSize(dimension);
        this.chambers = chambers;
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        if(drawTiling){
            paintUnderlyingTiling(g);
        }
        if(edg!=null){
            paintDecoration(g);
        }
    }
    
    private void paintUnderlyingTiling(Graphics g) {
        Graphics2D g2 = (Graphics2D)g.create();
        g2.setStroke(new BasicStroke(3));
        g2.setColor(Color.GRAY);
        for (GeometricChamberCoordinates chamber : chambers) {
            GeometricChamberCoordinates neighbour = chamber.getNeighbouringChamber(FacetType.FACE);
            if(chamber.getNumber() < neighbour.getNumber()){
                int x1 = (int)chamber.getV()[0];
                int y1 = (int)chamber.getV()[1];
                int x2 = (int)chamber.getE()[0];
                int y2 = (int)chamber.getE()[1];
                g2.drawLine(x1, y1, x2, y2);
            }
        }
    }
    
    private void paintDecoration(Graphics g) {
        Graphics2D g2 = (Graphics2D)g.create();
        g2.setStroke(new BasicStroke(2));
        // First draw the edges, so the vertices cover the end points of the edges.
        paintEdges(g2);
        paintVertices(g2);
    }
    
    private void paintEdges(Graphics g) {
        g.setColor(Color.BLACK);
        for (int i = 0; i < chambers.length; i++) {
            paintChamberEdges(g, chambers[i]);
        }
    }
    
    private void paintVertices(Graphics g) {
        for (int i = 0; i < chambers.length; i++) {
            paintChamberVertices(g, chambers[i]);
        }
    }
    
    private void paintChamber(Graphics g, GeometricChamberCoordinates chamber) {
        int x1 = (int)chamber.getV()[0];
        int y1 = (int)chamber.getV()[1];
        int x2 = (int)chamber.getE()[0];
        int y2 = (int)chamber.getE()[1];
        int x3 = (int)chamber.getF()[0];
        int y3 = (int)chamber.getF()[1];
        g.setColor(Color.BLACK);
        g.drawLine(x1, y1, x2, y2);
        g.drawLine(x1, y1, x3, y3);
        g.drawLine(x3, y3, x2, y2);
    }
    
    private void paintChamberEdges(Graphics g, GeometricChamberCoordinates chamber) {
        for (int i = 0; i < edg.getGraph().getOrder(); i++) {
            for (Neighbour neighbour : edg.getGraph().getNeighboursFor(i)) {
                if(neighbour.getType()==null){
                    double[] coordinatesStart = edg.getBarycentricCoordinatesFor(i);
                    int x1 = (int)(coordinatesStart[0]*chamber.getV()[0] + coordinatesStart[1]*chamber.getE()[0] + coordinatesStart[2]*chamber.getF()[0]);
                    int y1 = (int)(coordinatesStart[0]*chamber.getV()[1] + coordinatesStart[1]*chamber.getE()[1] + coordinatesStart[2]*chamber.getF()[1]);
                    double[] coordinatesEnd = edg.getBarycentricCoordinatesFor(neighbour.getVertex());
                    int x2 = (int)(coordinatesEnd[0]*chamber.getV()[0] + coordinatesEnd[1]*chamber.getE()[0] + coordinatesEnd[2]*chamber.getF()[0]);
                    int y2 = (int)(coordinatesEnd[0]*chamber.getV()[1] + coordinatesEnd[1]*chamber.getE()[1] + coordinatesEnd[2]*chamber.getF()[1]);
                    g.drawLine(x1, y1, x2, y2);
                } else {
                    GeometricChamberCoordinates targetChamber = chamber.getNeighbouringChamber(neighbour.getType());
                    if(targetChamber==null) continue; //TODO: add extra (invisible) chambers
                    if(targetChamber.getNumber() > chamber.getNumber()){
                        double[] coordinatesStart = edg.getBarycentricCoordinatesFor(i);
                        int x1 = (int)(coordinatesStart[0]*chamber.getV()[0] + coordinatesStart[1]*chamber.getE()[0] + coordinatesStart[2]*chamber.getF()[0]);
                        int y1 = (int)(coordinatesStart[0]*chamber.getV()[1] + coordinatesStart[1]*chamber.getE()[1] + coordinatesStart[2]*chamber.getF()[1]);
                        double[] coordinatesEnd = edg.getBarycentricCoordinatesFor(neighbour.getVertex());
                        int x2 = (int)(coordinatesEnd[0]*targetChamber.getV()[0] + coordinatesEnd[1]*targetChamber.getE()[0] + coordinatesEnd[2]*targetChamber.getF()[0]);
                        int y2 = (int)(coordinatesEnd[0]*targetChamber.getV()[1] + coordinatesEnd[1]*targetChamber.getE()[1] + coordinatesEnd[2]*targetChamber.getF()[1]);
                        g.drawLine(x1, y1, x2, y2);
                    }
                }
            }
        }
    }
    
    private void paintChamberVertices(Graphics g, GeometricChamberCoordinates chamber) {
        for (int i = 0; i < edg.getGraph().getOrder(); i++) {
            double[] coordinates = edg.getBarycentricCoordinatesFor(i);
            int x = (int)(coordinates[0]*chamber.getV()[0] + coordinates[1]*chamber.getE()[0] + coordinates[2]*chamber.getF()[0]);
            int y = (int)(coordinates[0]*chamber.getV()[1] + coordinates[1]*chamber.getE()[1] + coordinates[2]*chamber.getF()[1]);
            g.setColor(Color.WHITE);
            g.fillOval(x-4, y-4, 8, 8);
            g.setColor(Color.BLACK);
            g.drawOval(x-4, y-4, 8, 8);
        }
    }

    /**
     * Set whether the underlying tiling should be drawn.
     * 
     * @param drawTiling 
     */
    public void setDrawTiling(boolean drawTiling) {
        this.drawTiling = drawTiling;
        repaint();
    }

    /**
     * Should the underlying tiling be drawn.
     * 
     * @return 
     */
    public boolean getDrawTiling() {
        return drawTiling;
    }

    public void setGraph(EmbeddedDecorationGraph edg) {
        this.edg = edg;
        repaint();
    }
    
    /**
     * Returns a <code>DecorationViewer</code> which shows decorations applied
     * to the hexagon tiling.
     * 
     * @param height
     * @return 
     */
    public static DecorationViewer hexagonTilingViewer(int height){
        GeometricChamberCoordinates[] chambers = new GeometricChamberCoordinates[84];
        
        double hexHeight = (height-20)/3.0;
        double hexRadius = hexHeight/Math.sqrt(3);
        
        //the chambers in the center hexagon
        {
            double hexX = height/2.0;
            double hexY = height/2.0;
            for (int i = 0; i < 12; i++) {
                if(i%2==0){
                    chambers[i] = new GeometricChamberCoordinates(i,
                            new double[]{hexX + hexRadius*Math.cos(Math.PI*2/3 - (i/2)*Math.PI/3), hexY + hexRadius*Math.sin(Math.PI*2/3 - (i/2)*Math.PI/3)}, 
                            new double[]{hexX + hexHeight/2*Math.cos(Math.PI/2 - (i/2)*Math.PI/3), hexY + hexHeight/2*Math.sin(Math.PI/2 - (i/2)*Math.PI/3)},
                            new double[]{hexX, hexY});
                } else {
                    int iV = (i+1)%12;
                    chambers[i] = new GeometricChamberCoordinates(i,
                            new double[]{hexX + hexRadius*Math.cos(Math.PI*2/3 - (iV/2)*Math.PI/3), hexY + hexRadius*Math.sin(Math.PI*2/3 - (iV/2)*Math.PI/3)}, 
                            new double[]{hexX + hexHeight/2*Math.cos(Math.PI/2 - (i/2)*Math.PI/3), hexY + hexHeight/2*Math.sin(Math.PI/2 - (i/2)*Math.PI/3)},
                            new double[]{hexX, hexY});
                }
            }
        }
        //the chambers in the outer hexagons
        for (int j = 0; j < 6; j++) {
            double hexX = height/2.0 + hexHeight*Math.cos(Math.PI/2-j*Math.PI/3);
            double hexY = height/2.0 + hexHeight*Math.sin(Math.PI/2-j*Math.PI/3);
            for (int i = 0; i < 12; i++) {
                if(i%2==0){
                    chambers[12+j*12+i] = new GeometricChamberCoordinates(12+j*12+i,
                            new double[]{hexX + hexRadius*Math.cos(Math.PI*2/3 - (i/2)*Math.PI/3), hexY + hexRadius*Math.sin(Math.PI*2/3 - (i/2)*Math.PI/3)}, 
                            new double[]{hexX + hexHeight/2*Math.cos(Math.PI/2 - (i/2)*Math.PI/3), hexY + hexHeight/2*Math.sin(Math.PI/2 - (i/2)*Math.PI/3)},
                            new double[]{hexX, hexY});
                } else {
                    int iV = (i+1)%12;
                    chambers[12+j*12+i] = new GeometricChamberCoordinates(12+j*12+i,
                            new double[]{hexX + hexRadius*Math.cos(Math.PI*2/3 - (iV/2)*Math.PI/3), hexY + hexRadius*Math.sin(Math.PI*2/3 - (iV/2)*Math.PI/3)}, 
                            new double[]{hexX + hexHeight/2*Math.cos(Math.PI/2 - (i/2)*Math.PI/3), hexY + hexHeight/2*Math.sin(Math.PI/2 - (i/2)*Math.PI/3)},
                            new double[]{hexX, hexY});
                }
            }
        }
        
        //set the VERTEX and EDGE neighbours for each chamber
        for (int i = 0; i < 7; i++) {
            for (int j = 0; j < 12; j++) {
                if(j%2==0){
                    int c = i*12 + j;
                    int n1 = i*12 + (j+1)%12;
                    int n2 = i*12 + (j+11)%12;
                    chambers[c].setNeighbouringChamber(FacetType.VERTEX, chambers[n1]);
                    chambers[c].setNeighbouringChamber(FacetType.EDGE, chambers[n2]);
                } else {
                    int c = i*12 + j;
                    int n1 = i*12 + (j+1)%12;
                    int n2 = i*12 + (j+11)%12;
                    chambers[c].setNeighbouringChamber(FacetType.EDGE, chambers[n1]);
                    chambers[c].setNeighbouringChamber(FacetType.VERTEX, chambers[n2]);
                }
            }
        }
        //set the FACE neighbours for each chamber
        for (int i = 0; i < 6; i++) {
            int oppositeI = (i + 3)%6;
            chambers[2*i].setNeighbouringChamber(FacetType.FACE, chambers[12+i*12+2*oppositeI+1]);
            chambers[2*i+1].setNeighbouringChamber(FacetType.FACE, chambers[12+i*12+2*oppositeI]);
            chambers[12+i*12+2*oppositeI].setNeighbouringChamber(FacetType.FACE, chambers[2*i+1]);
            chambers[12+i*12+2*oppositeI+1].setNeighbouringChamber(FacetType.FACE, chambers[2*i]);
        }
        for (int i = 0; i < 6; i++) {
            int j = (i + 2)%6;
            int oppositeI = (i + 1)%6;
            int oppositeJ = (j+3)%6;
            chambers[12+i*12+2*j].setNeighbouringChamber(FacetType.FACE, chambers[12+oppositeI*12+2*oppositeJ+1]);
            chambers[12+i*12+2*j+1].setNeighbouringChamber(FacetType.FACE, chambers[12+oppositeI*12+2*oppositeJ]);
            chambers[12+oppositeI*12+2*oppositeJ].setNeighbouringChamber(FacetType.FACE, chambers[12+i*12+2*j+1]);
            chambers[12+oppositeI*12+2*oppositeJ+1].setNeighbouringChamber(FacetType.FACE, chambers[12+i*12+2*j]);
        }
        //some extra chambers on the edge
        int infinity = 84;
        for (int i = 0; i < 6; i++) {
            double hexX = height/2.0 + hexHeight*Math.cos(Math.PI/2-i*Math.PI/3);
            double hexY = height/2.0 + hexHeight*Math.sin(Math.PI/2-i*Math.PI/3);
            for (int j = i + 5; j <= i + 7; j++) {
                int side = j % 6;
                int oppositeSide = (side + 3) % 6;
                int oppositeSideNext = (side + 4) % 6;
                //determine center of neighbouring hexagon
                double nHexX = hexX + hexHeight*Math.cos(Math.PI/2-side*Math.PI/3);
                double nHexY = hexY + hexHeight*Math.sin(Math.PI/2-side*Math.PI/3);
                chambers[12+i*12+2*side].setNeighbouringChamber(FacetType.FACE, 
                        new GeometricChamberCoordinates(infinity,
                                new double[]{
                                    nHexX + hexRadius*Math.cos(Math.PI*2/3 - oppositeSideNext*Math.PI/3), 
                                    nHexY + hexRadius*Math.sin(Math.PI*2/3 - oppositeSideNext*Math.PI/3)}, 
                                new double[]{
                                    nHexX + hexHeight/2*Math.cos(Math.PI/2 - oppositeSide*Math.PI/3),
                                    nHexY + hexHeight/2*Math.sin(Math.PI/2 - oppositeSide*Math.PI/3)},
                                new double[]{nHexX, nHexY}));
                chambers[12+i*12+2*side+1].setNeighbouringChamber(FacetType.FACE, 
                        new GeometricChamberCoordinates(infinity,
                                new double[]{
                                    nHexX + hexRadius*Math.cos(Math.PI*2/3 - oppositeSide*Math.PI/3), 
                                    nHexY + hexRadius*Math.sin(Math.PI*2/3 - oppositeSide*Math.PI/3)}, 
                                new double[]{
                                    nHexX + hexHeight/2*Math.cos(Math.PI/2 - oppositeSide*Math.PI/3),
                                    nHexY + hexHeight/2*Math.sin(Math.PI/2 - oppositeSide*Math.PI/3)},
                                new double[]{nHexX, nHexY}));
            }
        }
        
        return new DecorationViewer(chambers, height, height);
    }
    
    /**
     * Returns a <code>DecorationViewer</code> which shows decorations applied
     * to the square tiling.
     * 
     * @param height
     * @return 
     */
    public static DecorationViewer squareTilingViewer(int height){
        GeometricChamberCoordinates[] chambers = new GeometricChamberCoordinates[72];
        
        double squareSideLength = (height-20)/3.0;
        double halfSquareSideLength = (height-20)/6.0;
        
        int[][] shiftsV = new int[][]{{-1, 1}, {1,1}, {1, -1}, {-1, -1}};
        int[][] shiftsE = new int[][]{{0, 1}, {1,0}, {0, -1}, {-1, 0}};
        
        //the chambers in the center square
        {
            double squareX = height/2.0;
            double squareY = height/2.0;
            for (int i = 0; i < 8; i++) {
                if(i%2==0){
                    chambers[i] = new GeometricChamberCoordinates(i,
                            new double[]{squareX + shiftsV[i/2][0]*halfSquareSideLength, squareY + shiftsV[i/2][1]*halfSquareSideLength}, 
                            new double[]{squareX + shiftsE[i/2][0]*halfSquareSideLength, squareY + shiftsE[i/2][1]*halfSquareSideLength},
                            new double[]{squareX, squareY});
                } else {
                    int iV = (i+1)%8;
                    chambers[i] = new GeometricChamberCoordinates(i,
                            new double[]{squareX + shiftsV[iV/2][0]*halfSquareSideLength, squareY + shiftsV[iV/2][1]*halfSquareSideLength}, 
                            new double[]{squareX + shiftsE[i/2][0]*halfSquareSideLength, squareY + shiftsE[i/2][1]*halfSquareSideLength},
                            new double[]{squareX, squareY});
                }
            }
        }
        //the chambers in the side squares
        for (int j = 0; j < 4; j++) {
            double squareX = height/2.0 + shiftsE[j][0]*squareSideLength;
            double squareY = height/2.0 + shiftsE[j][1]*squareSideLength;
            for (int i = 0; i < 8; i++) {
                if(i%2==0){
                    chambers[8+j*8+i] = new GeometricChamberCoordinates(8+j*8+i,
                            new double[]{squareX + shiftsV[i/2][0]*halfSquareSideLength, squareY + shiftsV[i/2][1]*halfSquareSideLength}, 
                            new double[]{squareX + shiftsE[i/2][0]*halfSquareSideLength, squareY + shiftsE[i/2][1]*halfSquareSideLength},
                            new double[]{squareX, squareY});
                } else {
                    int iV = (i+1)%8;
                    chambers[8+j*8+i] = new GeometricChamberCoordinates(8+j*8+i,
                            new double[]{squareX + shiftsV[iV/2][0]*halfSquareSideLength, squareY + shiftsV[iV/2][1]*halfSquareSideLength}, 
                            new double[]{squareX + shiftsE[i/2][0]*halfSquareSideLength, squareY + shiftsE[i/2][1]*halfSquareSideLength},
                            new double[]{squareX, squareY});
                }
            }
        }
        //the chambers in the corner squares
        for (int j = 0; j < 4; j++) {
            double squareX = height/2.0 + shiftsV[j][0]*squareSideLength;
            double squareY = height/2.0 + shiftsV[j][1]*squareSideLength;
            for (int i = 0; i < 8; i++) {
                if(i%2==0){
                    chambers[40+j*8+i] = new GeometricChamberCoordinates(40+j*8+i,
                            new double[]{squareX + shiftsV[i/2][0]*halfSquareSideLength, squareY + shiftsV[i/2][1]*halfSquareSideLength}, 
                            new double[]{squareX + shiftsE[i/2][0]*halfSquareSideLength, squareY + shiftsE[i/2][1]*halfSquareSideLength},
                            new double[]{squareX, squareY});
                } else {
                    int iV = (i+1)%8;
                    chambers[40+j*8+i] = new GeometricChamberCoordinates(40+j*8+i,
                            new double[]{squareX + shiftsV[iV/2][0]*halfSquareSideLength, squareY + shiftsV[iV/2][1]*halfSquareSideLength}, 
                            new double[]{squareX + shiftsE[i/2][0]*halfSquareSideLength, squareY + shiftsE[i/2][1]*halfSquareSideLength},
                            new double[]{squareX, squareY});
                }
            }
        }
        
        //set the VERTEX and EDGE neighbours for each chamber
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 8; j++) {
                if(j%2==0){
                    int c = i*8 + j;
                    int n1 = i*8 + (j+1)%8;
                    int n2 = i*8 + (j+7)%8;
                    chambers[c].setNeighbouringChamber(FacetType.VERTEX, chambers[n1]);
                    chambers[c].setNeighbouringChamber(FacetType.EDGE, chambers[n2]);
                } else {
                    int c = i*8 + j;
                    int n1 = i*8 + (j+1)%8;
                    int n2 = i*8 + (j+7)%8;
                    chambers[c].setNeighbouringChamber(FacetType.EDGE, chambers[n1]);
                    chambers[c].setNeighbouringChamber(FacetType.VERTEX, chambers[n2]);
                }
            }
        }
        //set the FACE neighbours for each chamber
        //the center square
        for (int i = 0; i < 4; i++) {
            int oppositeI = (i + 2)%4;
            chambers[2*i].setNeighbouringChamber(FacetType.FACE, chambers[8+i*8+2*oppositeI+1]);
            chambers[2*i+1].setNeighbouringChamber(FacetType.FACE, chambers[8+i*8+2*oppositeI]);
            chambers[8+i*8+2*oppositeI].setNeighbouringChamber(FacetType.FACE, chambers[2*i+1]);
            chambers[8+i*8+2*oppositeI+1].setNeighbouringChamber(FacetType.FACE, chambers[2*i]);
        }
        //the side squares forward
        for (int i = 0; i < 4; i++) {
            int j = (i + 1)%4;
            int oppositeI = (i + 1)%4; //which square is neighbouring this square
            int oppositeJ = (j+2)%4; //which quadrant is neighbouring j
            chambers[8+i*8+2*j].setNeighbouringChamber(FacetType.FACE, chambers[40+oppositeI*8+2*oppositeJ+1]);
            chambers[8+i*8+2*j+1].setNeighbouringChamber(FacetType.FACE, chambers[40+oppositeI*8+2*oppositeJ]);
            chambers[40+oppositeI*8+2*oppositeJ].setNeighbouringChamber(FacetType.FACE, chambers[8+i*8+2*j+1]);
            chambers[40+oppositeI*8+2*oppositeJ+1].setNeighbouringChamber(FacetType.FACE, chambers[8+i*8+2*j]);
        }
        //the side squares forward
        for (int i = 0; i < 4; i++) {
            int j = (i + 3)%4;
            int oppositeI = i; //which square is neighbouring this square
            int oppositeJ = (j+2)%4; //which quadrant is neighbouring j
            chambers[8+i*8+2*j].setNeighbouringChamber(FacetType.FACE, chambers[40+oppositeI*8+2*oppositeJ+1]);
            chambers[8+i*8+2*j+1].setNeighbouringChamber(FacetType.FACE, chambers[40+oppositeI*8+2*oppositeJ]);
            chambers[40+oppositeI*8+2*oppositeJ].setNeighbouringChamber(FacetType.FACE, chambers[8+i*8+2*j+1]);
            chambers[40+oppositeI*8+2*oppositeJ+1].setNeighbouringChamber(FacetType.FACE, chambers[8+i*8+2*j]);
        }
        //some extra chambers on the edge
        int infinity = 72;
        int[] top = new int[] {5, 1, 6};
        int[] right = new int[] {6, 2, 7};
        int[] bottom = new int[] {8, 3, 7};
        int[] left = new int[] {5, 4, 8};

        double topY = height/2.0 + 2*squareSideLength;
        double bottomY = height/2.0 - 2*squareSideLength;
        double leftX = height/2.0 - 2*squareSideLength;
        double rightX = height/2.0 + 2*squareSideLength;
        for (int i = 0; i < 3; i++) {
            chambers[top[i]*8].setNeighbouringChamber(FacetType.FACE, new GeometricChamberCoordinates(infinity, 
                    new double[] {height/2.0 + (i-1.5)*squareSideLength, topY - halfSquareSideLength}, 
                    new double[] {height/2.0 + (i-1)*squareSideLength, topY - halfSquareSideLength}, 
                    new double[] {height/2.0 + (i-1)*squareSideLength, topY}));
            chambers[top[i]*8+1].setNeighbouringChamber(FacetType.FACE, new GeometricChamberCoordinates(infinity, 
                    new double[] {height/2.0 + (i-.5)*squareSideLength, topY - halfSquareSideLength}, 
                    new double[] {height/2.0 + (i-1)*squareSideLength, topY - halfSquareSideLength}, 
                    new double[] {height/2.0 + (i-1)*squareSideLength, topY}));
            chambers[bottom[i]*8+5].setNeighbouringChamber(FacetType.FACE, new GeometricChamberCoordinates(infinity, 
                    new double[] {height/2.0 + (i-1.5)*squareSideLength, bottomY + halfSquareSideLength}, 
                    new double[] {height/2.0 + (i-1)*squareSideLength, bottomY + halfSquareSideLength}, 
                    new double[] {height/2.0 + (i-1)*squareSideLength, bottomY}));
            chambers[bottom[i]*8+4].setNeighbouringChamber(FacetType.FACE, new GeometricChamberCoordinates(infinity, 
                    new double[] {height/2.0 + (i-.5)*squareSideLength, bottomY + halfSquareSideLength}, 
                    new double[] {height/2.0 + (i-1)*squareSideLength, bottomY + halfSquareSideLength}, 
                    new double[] {height/2.0 + (i-1)*squareSideLength, bottomY}));
            
            
            chambers[right[i]*8+2].setNeighbouringChamber(FacetType.FACE, new GeometricChamberCoordinates(infinity, 
                    new double[] {rightX-halfSquareSideLength, height/2.0 - (i-1.5)*squareSideLength}, 
                    new double[] {rightX-halfSquareSideLength, height/2.0 - (i-1)*squareSideLength}, 
                    new double[] {rightX, height/2.0 - (i-1)*squareSideLength}));
            chambers[right[i]*8+3].setNeighbouringChamber(FacetType.FACE, new GeometricChamberCoordinates(infinity, 
                    new double[] {rightX-halfSquareSideLength, height/2.0 - (i-.5)*squareSideLength}, 
                    new double[] {rightX-halfSquareSideLength, height/2.0 - (i-1)*squareSideLength}, 
                    new double[] {rightX, height/2.0 - (i-1)*squareSideLength}));
            chambers[left[i]*8+7].setNeighbouringChamber(FacetType.FACE, new GeometricChamberCoordinates(infinity, 
                    new double[] {leftX+halfSquareSideLength, height/2.0 - (i-1.5)*squareSideLength}, 
                    new double[] {leftX+halfSquareSideLength, height/2.0 - (i-1)*squareSideLength}, 
                    new double[] {leftX, height/2.0 - (i-1)*squareSideLength}));
            chambers[left[i]*8+6].setNeighbouringChamber(FacetType.FACE, new GeometricChamberCoordinates(infinity, 
                    new double[] {leftX+halfSquareSideLength, height/2.0 - (i-.5)*squareSideLength}, 
                    new double[] {leftX+halfSquareSideLength, height/2.0 - (i-1)*squareSideLength}, 
                    new double[] {leftX, height/2.0 - (i-1)*squareSideLength}));

        }
        return new DecorationViewer(chambers, height, height);
    }
}

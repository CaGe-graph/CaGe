package cage.viewer.twoview;

import java.awt.Dimension;

/**
 * An implementation of {@code TwoViewPainter} that creates a SVG document 
 * containing the graph. At the moment this SVG is kept as simple as possible
 * without much formatting options.
 * 
 * @author nvcleemp
 */
public class SvgTwoViewPainter extends TwoViewPainter {

    private Dimension svgDimension = new Dimension();

    private StringBuilder builder = new StringBuilder();

    public SvgTwoViewPainter(TwoViewModel model) {
        super(model);
    }

    public void setSvgDimension(Dimension d){
        svgDimension.height = 2*d.height;
        svgDimension.width = 2*d.width;
        setPaintArea(0, 2*d.width, 0, 2*d.height);
    }

    public String getSvgContent(){
        return builder == null ? "" : builder.toString();
    }

    @Override
    protected void beginGraph() {
        builder = new StringBuilder();
        builder.append(
                String.format(
                    "<svg width=\"%d\" height=\"%d\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n",
                    svgDimension.width, svgDimension.height));
    }

    @Override
    protected void beginEdges() {
        builder.append("\n  <!-- Edges -->\n\n");
    }

    @Override
    protected void paintEdge(double x1, double y1, double x2, double y2, int v1, int v2, boolean useSpecialColour) {
        if(useSpecialColour){
            builder.append(
                    String.format(
                        "  <line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" style=\"stroke:rgb(60,150,60);stroke-width:%d\"/>\n",
                        x1, y1, x2, y2, model.getEdgeWidth()));
        } else {
            builder.append(
                    String.format(
                        "  <line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" style=\"stroke:rgb(0,0,0);stroke-width:%d\"/>\n",
                        x1, y1, x2, y2, model.getEdgeWidth()));
        }
    }

    @Override
    protected void beginVertices() {
        builder.append("\n  <!-- Vertices -->\n\n");
    }

    @Override
    protected void paintVertex(double x, double y, int number) {
        if(model.getShowNumbers()){
            builder.append("<g>\n");
            builder.append(
                    String.format(
                        "  <circle cx=\"%f\" cy=\"%f\" r=\"%d\" style=\"fill:rgb(255,200,100);stroke:rgb(0,0,0);stroke-width:1\"/>\n",
                        x, y, model.getVertexSize()));
            builder.append(
                    String.format(
                        "<text x=\"%f\" y=\"%f\" dy=\"%d\" style=\"font-size:%dpx;text-anchor:middle;alignment-baseline:middle\">",
                        x, y, model.getVertexSize()/2, model.getVertexSize()));
            builder.append(Integer.toString(number));
            builder.append("</text>\n");
            builder.append("</g>\n");
        } else {
            builder.append(
                    String.format(
                        "  <circle cx=\"%f\" cy=\"%f\" r=\"%d\" style=\"fill:rgb(255,200,100);stroke:rgb(0,0,0);stroke-width:1\"/>\n",
                        x, y, model.getVertexSize()));
        }
    }

    @Override
    protected void endGraph() {
        builder.append("</svg>\n");
    }

}

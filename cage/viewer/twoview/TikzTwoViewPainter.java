package cage.viewer.twoview;

/**
 *
 * @author nvcleemp
 */
public class TikzTwoViewPainter extends TwoViewPainter {

    private StringBuilder builder = new StringBuilder();

    public TikzTwoViewPainter(TwoViewModel model) {
        super(model);
    }

    public String getTikzContent(){
        return builder == null ? "" : builder.toString();
    }

    @Override
    protected boolean startWithEdges() {
        return false;
    }

    @Override
    protected void beginGraph() {
        setPaintArea(0, 100, 0, 100);
        builder = new StringBuilder();
        builder.append("\\begin{tikzpicture}[scale=0.3]\n");
        builder.append("    \\definecolor{marked}{rgb}{0.25,0.5,0.25}\n");
    }

    @Override
    protected void beginEdges() {
        //do nothing
    }

    @Override
    protected void paintEdge(double x1, double y1, double x2, double y2, int v1, int v2, boolean useSpecialColour) {
        if(useSpecialColour){
            builder.append(
                    String.format(
                        "    \\draw [marked] (%s) to (%s);\n",
                        Integer.toString(v1), Integer.toString(v2)));
        } else {
            builder.append(
                    String.format(
                        "    \\draw [black] (%s) to (%s);\n",
                        Integer.toString(v1), Integer.toString(v2)));
        }
    }

    @Override
    protected void beginVertices() {
        //do nothing
    }

    @Override
    protected void paintVertex(double x, double y, int number) {
        if(model.getShowNumbers()){
            builder.append(
                    String.format(
                        "    \\node [circle,draw] (%s) at (%f,%f) {%s};\n",
                        Integer.toString(number), x, y, Integer.toString(number)));
        } else {
            builder.append(
                    String.format(
                        "    \\node [circle,fill] (%s) at (%f,%f) {};\n",
                        Integer.toString(number), x, y));
        }
    }

    @Override
    protected void endGraph() {
        builder.append("\\end{tikzpicture}\n");
    }

}

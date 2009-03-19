package cage.viewer.twoview;

/**
 *
 */
public interface TwoViewDevice {

    public void beginGraph();

    public void beginEdges();

    public void paintEdge(double x1, double y1, double x2, double y2, int v1, int v2);

    public void beginVertices();

    public void paintVertex(double x, double y, int number);
}

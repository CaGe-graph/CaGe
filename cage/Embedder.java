package cage;

public abstract class Embedder {

    int e1, e2;
    public static final int IGNORE_OLD_EMBEDDING = 0;
    public static final int KEEP_OLD_EMBEDDING = 1;
    public static final int REFINE_OLD_EMBEDDING = 2;

    public abstract void setEmbed2D(String[][] embed2D);

    public abstract void setEmbed3D(String[][] embed3D);

    public abstract void setConstant(boolean isConstant);

    public abstract boolean isConstant();

    public abstract void setRunDir(String runDir);

    public abstract void setPath(String path);

    public abstract void setIntensityFactor(float factor);

    public abstract void setMode(int mode);

    public abstract int getMode();

    public abstract String[][] getEmbed2DNew();

    public abstract String[][] getEmbed3DNew();

    public abstract String[][] getEmbed3DRefine();

    public abstract void embed2D(EmbeddableGraph graph)
            throws Exception;

    public abstract void embed3D(EmbeddableGraph graph)
            throws Exception;

    public abstract void reembed2D(EmbeddableGraph graph)
            throws Exception;

    public abstract String getDiagnosticOutput();

    public abstract void abort();

    /**
     * Returns whether the graph needs to be reembedded for the point
     * (<tt>x</tt>, <tt>y</tt>) to lie in the outer face or not. This method
     * also stores the new outer face in case the graph needs to be reembedded.
     * (TODO: document this last fact better.)
     *
     * @param graph The graph for which the embedding needs to be considered
     * @param x The x coordinate of the point
     * @param y The y coordinate of the point
     * @return <tt>false</tt> if the point (<tt>x</tt>, <tt>y</tt>) lies in
     * the outer face, <tt>true</tt> otherwise.
     *
     */
    public boolean reembed2DRequired(EmbeddableGraph graph, float x, float y) {
        int f1 = 0, f2 = 0;
        boolean found_above = false, found_below = false;
        int size = graph.getSize();
        float x1, y1, x2, y2, ye, yeMin = 0.0f;
        float[][] coordinate = graph.get2DCoordinates();
        for (int i = 0; i < size; ++i) {
            x1 = coordinate[i][0];
            y1 = coordinate[i][1];
            EdgeIterator it = graph.getEdgeIterator(i + 1);
            while (it.hasNext()) {
                int j = it.nextEdge() - 1;
                x2 = coordinate[j][0];
                y2 = coordinate[j][1];
                if (x1 != x2 && ((x1 <= x && x2 >= x) || (x1 >= x && x2 <= x))) {
                    ye = y1 + (x - x1) * (y2 - y1) / (x2 - x1);
                    if (ye > y) {
                        if (!found_above || ye < yeMin) {
                            found_above = true;
                            yeMin = ye;
                            f1 = i;
                            f2 = j;
                        }
                    } else {
                        found_below = true;
                    }
                }
            }
        }
        if (found_above && found_below) {
            if (coordinate[f1][0] < coordinate[f2][0]) {
                e1 = f1 + 1;
                e2 = f2 + 1;
            } else {
                e1 = f2 + 1;
                e2 = f1 + 1;
            }
            return true;
        } else {
            return false;
        }
    }
}

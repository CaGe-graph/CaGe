package cage;

public class CaGeResult {

    public EmbeddableGraph graph;
    public int graphNo;
    public boolean savedAdj = false;
    public boolean saved2D = false;
    public boolean saved3D = false;
    public int saved2DPS = 0;
    public boolean reembed2DMade = false;
    public boolean foldnetMade = false;

    public CaGeResult(EmbeddableGraph graph, int graphNo) {
        this.graph = graph;
        this.graphNo = graphNo;
    }

    public EmbeddableGraph getGraph() {
        return graph;
    }

    public int getGraphNo() {
        return graphNo;
    }

    public boolean isFoldnetMade() {
        return foldnetMade;
    }

    public void setFoldnetMade(boolean foldnetMade) {
        this.foldnetMade = foldnetMade;
    }

    public boolean isReembed2DMade() {
        return reembed2DMade;
    }

    public void setReembed2DMade(boolean reembed2DMade) {
        this.reembed2DMade = reembed2DMade;
    }

    public boolean isSaved2D() {
        return saved2D;
    }

    public void setSaved2D(boolean saved2D) {
        this.saved2D = saved2D;
    }

    public int getSaved2DPS() {
        return saved2DPS;
    }

    public void setSaved2DPS(int saved2DPS) {
        this.saved2DPS = saved2DPS;
    }

    public boolean isSaved3D() {
        return saved3D;
    }

    public void setSaved3D(boolean saved3D) {
        this.saved3D = saved3D;
    }

    public boolean isSavedAdj() {
        return savedAdj;
    }

    public void setSavedAdj(boolean savedAdj) {
        this.savedAdj = savedAdj;
    }
}


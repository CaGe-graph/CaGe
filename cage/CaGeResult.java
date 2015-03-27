package cage;

public class CaGeResult {

    private EmbeddableGraph graph;
    private int graphNo;
    private int saved2DPS = 0;
    private boolean reembed2DMade = false;
    private boolean foldnetMade = false;

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

    public int getSaved2DPS() {
        return saved2DPS;
    }

    public void setSaved2DPS(int saved2DPS) {
        this.saved2DPS = saved2DPS;
    }

    public void incrementSaved2DPS() {
        this.saved2DPS++;
    }
}


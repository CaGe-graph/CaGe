package cage;

public class NativeEmbeddableGraph implements EmbeddableGraph {

    protected long nGraph;

    private native long newNGraph();

    private native byte[] nGetComment(long nGraph);

    private native void nSetComment(long nGraph, byte[] comment);

    private native int nGetFormat(long nGraph);

    private native void nSetFormat(long nGraph, int format);

    private native void nAddVertex(long nGraph);

    private native void nAddEdge(long nGraph, int to);

    private native int nGetSize(long nGraph);

    private native int nGetValency(long nGraph, int vertex);

    private native NativeEdgeIterator nGetEdgeIterator(long nGraph, int vertex);

    private native boolean nHas2DCoordinates(long nGraph);

    private native float[] nGet2DCoordinates(long nGraph, int vertex);

    private native float[][] nGetAll2DCoordinates(long nGraph);

    private native void nSet2DCoordinates(long nGraph, int vertex, float[] coords);

    private native boolean nHas3DCoordinates(long nGraph);

    private native float[] nGet3DCoordinates(long nGraph, int vertex);

    private native float[][] nGetAll3DCoordinates(long nGraph);

    private native void nSet3DCoordinates(long nGraph, int vertex, float[] coords);

    private native byte[] toBytes(long nGraph);

    private native void nFinalize(long nGraph);

    public NativeEmbeddableGraph() {
        nGraph = newNGraph();
    }

    public NativeEmbeddableGraph(long nGraph) {
        this.nGraph = nGraph;
    }

    @Override
    public String getComment() {
        byte[] bytes = nGetComment(nGraph);
        return bytes == null ? null : new String(bytes);
    }

    @Override
    public void setComment(String comment) {
        nSetComment(nGraph, comment.getBytes());
    }

    public int getFormat() {
        return nGetFormat(nGraph);
    }

    public void setFormat(int format) {
        nSetFormat(nGraph, format);
    }

    @Override
    public void addVertex() {
        nAddVertex(nGraph);
    }

    @Override
    public void addEdge(int to) {
        nAddEdge(nGraph, to);
    }

    @Override
    public int getSize() {
        return nGetSize(nGraph);
    }

    @Override
    public int getValency(int vertex) {
        return nGetValency(nGraph, vertex);
    }

    @Override
    public EdgeIterator getEdgeIterator(int vertex) {
        return nGetEdgeIterator(nGraph, vertex);
    }

    @Override
    public boolean has2DCoordinates() {
        return nHas2DCoordinates(nGraph);
    }

    @Override
    public float[] get2DCoordinates(int vertex) {
        return nGet2DCoordinates(nGraph, vertex);
    }

    @Override
    public float[][] get2DCoordinates() {
        return nGetAll2DCoordinates(nGraph);
    }

    @Override
    public void set2DCoordinates(int vertex, float[] coords) {
        nSet2DCoordinates(nGraph, vertex, coords);
    }

    @Override
    public boolean has3DCoordinates() {
        return nHas3DCoordinates(nGraph);
    }

    @Override
    public float[] get3DCoordinates(int vertex) {
        return nGet3DCoordinates(nGraph, vertex);
    }

    @Override
    public float[][] get3DCoordinates() {
        return nGetAll3DCoordinates(nGraph);
    }

    @Override
    public void set3DCoordinates(int vertex, float[] coords) {
        nSet3DCoordinates(nGraph, vertex, coords);
    }

    @Override
    public String toString() {
        return new String(toBytes(nGraph));
    }

    @Override
    protected void finalize() throws Throwable {
        nFinalize(nGraph);
        super.finalize();
    }
}

package cage.writer;

import cage.CaGeResult;
import cage.NativeEmbeddableGraph;

import java.io.OutputStream;

public class NativePlanarWriter extends CaGeWriter {

    @Override
    public String getFormatName() {
        return "planar code";
    }

    @Override
    public String getFileExtension() {
        return "plc";
    }

    native byte[] nEncodeGraph(NativeEmbeddableGraph graph);

    native byte[] header();

    @Override
    public void setOutputStream(OutputStream out) {
        super.setOutputStream(out);
        out(header());
    }

    @Override
    public void outputResult(CaGeResult result) {
        out(nEncodeGraph((NativeEmbeddableGraph) result.getGraph()));
    }
}


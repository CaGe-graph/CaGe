package cage.writer;

import cage.CaGeResult;
import cage.NativeEmbeddableGraph;

import java.io.OutputStream;

public class NativePlanarWriter extends CaGeWriter {

    public String getFormatName() {
        return "planar code";
    }

    public String getFileExtension() {
        return "plc";
    }

    native byte[] nEncodeGraph(NativeEmbeddableGraph graph);

    native byte[] header();

    public void setOutputStream(OutputStream out) {
        super.setOutputStream(out);
        out(header());
    }

    public void outputResult(CaGeResult result) {
        out(nEncodeGraph((NativeEmbeddableGraph) result.getGraph()));
    }
}


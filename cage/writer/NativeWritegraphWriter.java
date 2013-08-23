package cage.writer;

import cage.CaGeResult;
import cage.NativeEmbeddableGraph;

import java.io.OutputStream;

public class NativeWritegraphWriter extends CaGeWriter {

    @Override
    public String getFormatName() {
        return "writegraph";
    }

    @Override
    public String getFileExtension() {
        return "w" + dimension + "d";
    }

    native byte[] nEncodeGraph(NativeEmbeddableGraph graph, int dimension);

    native byte[] header(int dimension);

    @Override
    public void setOutputStream(OutputStream out) {
        super.setOutputStream(out);
        out(header(dimension));
    }

    @Override
    public void outputResult(CaGeResult result) {
        out(nEncodeGraph((NativeEmbeddableGraph) result.getGraph(), dimension));
    }
}


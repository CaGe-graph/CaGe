package cage.writer;

import cage.CaGeResult;
import cage.ElementRule;
import cage.NativeEmbeddableGraph;

public class NativeCMLWriter extends AbstractChemicalWriter {

    public String getFormatName() {
        return "CML";
    }

    public String getFileExtension() {
        return "cml";
    }

    native byte[] nEncodeGraph(
            NativeEmbeddableGraph graph, ElementRule elementRule, int dimension);

    public String encodeResult(CaGeResult result) {
        return new String(nEncodeGraph(
                (NativeEmbeddableGraph) result.getGraph(), elementRule, dimension));
    }

    public void outputResult(CaGeResult result) {
        out(nEncodeGraph(
                (NativeEmbeddableGraph) result.getGraph(), elementRule, dimension));
    }
}


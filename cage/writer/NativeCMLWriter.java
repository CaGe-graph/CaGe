package cage.writer;

import cage.CaGeResult;
import cage.ElementRule;
import cage.NativeEmbeddableGraph;

public class NativeCMLWriter extends AbstractChemicalWriter {

    @Override
    public String getFormatName() {
        return "CML";
    }

    @Override
    public String getFileExtension() {
        return "cml";
    }

    native byte[] nEncodeGraph(
            NativeEmbeddableGraph graph, ElementRule elementRule, int dimension);

    @Override
    public String encodeResult(CaGeResult result) {
        return new String(nEncodeGraph(
                (NativeEmbeddableGraph) result.getGraph(), elementRule, dimension));
    }

    @Override
    public void outputResult(CaGeResult result) {
        out(nEncodeGraph(
                (NativeEmbeddableGraph) result.getGraph(), elementRule, dimension));
    }
}


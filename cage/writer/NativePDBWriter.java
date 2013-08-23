package cage.writer;

import cage.CaGeResult;
import cage.ElementRule;
import cage.NativeEmbeddableGraph;

import java.io.IOException;

public class NativePDBWriter extends AbstractChemicalWriter {

    @Override
    public String getFormatName() {
        return "PDB";
    }

    @Override
    public String getFileExtension() {
        return "pdb";
    }

    native byte[] nEncodeGraph(NativeEmbeddableGraph graph, ElementRule elementRule, int dimension)
            throws IOException;

    @Override
    public String encodeResult(CaGeResult result) {
        byte[] encoding;
        lastException = null;
        try {
            encoding = nEncodeGraph((NativeEmbeddableGraph) result.getGraph(),
                    elementRule, dimension);
        } catch (IOException ex) {
            lastException = ex;
            return null;
        }
        return new String(encoding);
    }

    @Override
    public void outputResult(CaGeResult result) {
        byte[] encoding;
        lastException = null;
        try {
            encoding = nEncodeGraph((NativeEmbeddableGraph) result.getGraph(),
                    elementRule, dimension);
            out(encoding);
        } catch (IOException ex) {
            lastException = ex;
        }
    }
}


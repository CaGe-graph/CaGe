
package cage.writer;

import cage.CaGeResult;
import cage.ElementRule;
import cage.NativeEmbeddableGraph;

import java.io.IOException;


public class NativePDBWriter extends AbstractChemicalWriter
{
  public String getFormatName()
  {
    return "PDB";
  }

  public String getFileExtension()
  {
    return "pdb";
  }

  native byte[] nEncodeGraph
   (NativeEmbeddableGraph graph, ElementRule elementRule, int dimension)
   throws IOException;

  public String encodeResult(CaGeResult result)
  {
    byte[] encoding;
    lastException = null;
    try {
      encoding = nEncodeGraph((NativeEmbeddableGraph) result.graph,
       elementRule, dimension);
    } catch (IOException ex) {
      lastException = ex;
      return null;
    }
    return new String(encoding);
  }

  public void outputResult(CaGeResult result)
  {
    byte[] encoding;
    lastException = null;
    try {
      encoding = nEncodeGraph((NativeEmbeddableGraph) result.graph,
       elementRule, dimension);
      out(encoding);
    } catch (IOException ex) {
      lastException = ex;
    }
  }
}



package cage.writer;


import java.io.*;
import cage.*;


public class NativeCMLWriter extends AbstractChemicalWriter
{
  public String getFormatName()
  {
    return "CML";
  }

  public String getFileExtension()
  {
    return "cml";
  }

  native byte[] nEncodeGraph(
   NativeEmbeddableGraph graph, ElementRule elementRule, int dimension);

  public String encodeResult(CaGeResult result)
  {
    return new String(nEncodeGraph(
     (NativeEmbeddableGraph) result.graph, elementRule, dimension));
  }

  public void outputResult(CaGeResult result)
  {
    out(nEncodeGraph(
     (NativeEmbeddableGraph) result.graph, elementRule, dimension));
  }
}


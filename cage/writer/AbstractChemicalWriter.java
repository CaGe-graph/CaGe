
package cage.writer;

import cage.CaGeResult;
import cage.ElementRule;
import cage.GeneratorInfo;


public abstract class AbstractChemicalWriter extends CaGeWriter
{
  ElementRule elementRule;

  public void setGeneratorInfo(GeneratorInfo generatorInfo)
  {
    super.setGeneratorInfo(generatorInfo);
    elementRule = generatorInfo.getElementRule();
    /*
    int i, dimOffset = 0;
    String[][] embedder = generatorInfo.getEmbedder().getEmbed3DNew();
    for (i = 0; i < embedder[0].length; ++i)
    {
      if (embedder[0][i].equals("-it")) {
        dimOffset = dimension - 1;
	break;
      }
    }
    dim = new int[dimension];
    for (i = 0; i < dimension; ++i) dim[i] = (i + dimOffset) % dimension;
    */
  }

  public abstract String encodeResult(CaGeResult result);
}


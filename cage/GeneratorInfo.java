
package cage;


public abstract class GeneratorInfo
{
  public abstract String[][] getGenerator();
  public abstract void setGenerator(String[][] generator);
  public abstract Embedder getEmbedder();
  public abstract void setEmbedder(Embedder embedder);
  public abstract String getFilename();
  public abstract int getMaxFacesize();
  public abstract boolean isReembed2DEnabled();
  public abstract ElementRule getElementRule();
  public abstract int getExpertMode();

  String generatorName;

  public void setGeneratorName(String generatorName)
  {
    this.generatorName = generatorName;
  }

  public String getGeneratorName()
  {
    return generatorName;
  }

  public static final int GENERATOR_EXPERT = 1;
  public static final int EMBED_EXPERT = 2;
  public static final int ALL_EXPERT_MODES = 3;
  private static final int EXPERT_BITS = 10;

  public static int createExpertMode(int inclusion, int exclusion)
  {
    return
     (inclusion & ALL_EXPERT_MODES) |
     ((exclusion & ALL_EXPERT_MODES) << EXPERT_BITS);
  }

  public int getIncludedExpertModes()
  {
    return getExpertMode() & ALL_EXPERT_MODES;
  }

  public int getExcludedExpertModes()
  {
    return (getExpertMode() >> EXPERT_BITS) & ALL_EXPERT_MODES;
  }

  public boolean expertModeIncludes(int modes)
  {
    return (getIncludedExpertModes() & modes) != 0;
  }

  public boolean expertModeExcludes(int modes)
  {
    return (getExcludedExpertModes() & modes) != 0;
  }

  public boolean expertModeContains(int modes, boolean defaultIncluded)
  {
    return defaultIncluded ?
     (getExcludedExpertModes() & modes) != modes :
     (getIncludedExpertModes() & modes) != 0;
  }
}


package cage;


public class StaticGeneratorInfo extends GeneratorInfo
{
  private String[][] generator;
  private Embedder embedder;
  private String filename;
  private int maxFacesize;
  private boolean reembed2DEnabled;
  private ElementRule elementRule;
  private int expertMode;

  public StaticGeneratorInfo
   (String[][] generator,
    Embedder embedder,
    String filename,
    int maxFacesize)
  {
    this(generator, embedder, filename, maxFacesize, true);
  }

  public StaticGeneratorInfo
   (String[][] generator,
    Embedder embedder,
    String filename,
    int maxFacesize,
    boolean reembed2DEnabled)
  {
    this(generator, embedder, filename, maxFacesize, reembed2DEnabled,
     new ValencyElementRule("H O C Si N S I"), 0);
  }

  public StaticGeneratorInfo
   (String[][] generator,
    Embedder embedder,
    String filename,
    int maxFacesize,
    ElementRule elementRule)
  {
    this(generator, embedder, filename, maxFacesize, true, elementRule, 0);
  }

  public StaticGeneratorInfo
   (String[][] generator,
    Embedder embedder,
    String filename,
    int maxFacesize,
    boolean reembed2DEnabled,
    ElementRule elementRule)
  {
    this(generator, embedder, filename, maxFacesize, reembed2DEnabled,
     elementRule, 0);
  }

  public StaticGeneratorInfo
   (String[][] generator,
    Embedder embedder,
    String filename,
    int maxFacesize,
    ElementRule elementRule,
    int expertMode)
  {
    this(generator, embedder, filename, maxFacesize, true,
     elementRule, expertMode);
  }

  public StaticGeneratorInfo
   (String[][] generator,
    Embedder embedder,
    String filename,
    int maxFacesize,
    boolean reembed2DEnabled,
    ElementRule elementRule,
    int expertMode)
  {
    this.generator         = generator;
    this.embedder          = embedder;
    this.filename          = filename;
    this.maxFacesize       = maxFacesize;
    this.reembed2DEnabled  = reembed2DEnabled;
    this.elementRule       = elementRule;
    this.expertMode        = expertMode;
  }


  public String[][] getGenerator()
  {
    return generator;
  }

  public void setGenerator(String[][] generator)
  {
    this.generator    = generator;
  }

  public Embedder getEmbedder()
  {
    return embedder;
  }

  public void setEmbedder(Embedder embedder)
  {
    this.embedder     = embedder;
  }

  public String getFilename()
  {
    return filename;
  }

  public int getMaxFacesize()
  {
    return maxFacesize;
  }

  public boolean isReembed2DEnabled()
  {
    return reembed2DEnabled;
  }

  public ElementRule getElementRule()
  {
    return elementRule;
  }

  public int getExpertMode()
  {
    return expertMode;
  }
}

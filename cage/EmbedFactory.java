
package cage;


public class EmbedFactory
{
  public static Embedder createEmbedder
   (String[][] embed2D, String[][] embed3D)
  {
    return createEmbedder(CaGe.nativesAvailable, false,
     embed2D, embed3D, 1.0f, Embedder.IGNORE_OLD_EMBEDDING);
  }

  public static Embedder createEmbedder
   (boolean isConstant, String[][] embed2D, String[][] embed3D)
  {
    return createEmbedder(CaGe.nativesAvailable, isConstant,
     embed2D, embed3D, 1.0f, Embedder.IGNORE_OLD_EMBEDDING);
  }

  public static Embedder createEmbedder
   (boolean nativesAvailable, boolean isConstant,
    String[][] embed2D, String[][] embed3D)
  {
    return createEmbedder(nativesAvailable, isConstant,
     embed2D, embed3D, 1.0f, Embedder.IGNORE_OLD_EMBEDDING);
  }

  public static Embedder createEmbedder
   (String[][] embed2D, String[][] embed3D,
    float intensityFactor, int embeddedMode)
  {
    return createEmbedder(CaGe.nativesAvailable, false,
     embed2D, embed3D, intensityFactor, embeddedMode);
  }

  public static Embedder createEmbedder
   (boolean isConstant,
    String[][] embed2D, String[][] embed3D,
    float intensityFactor, int embeddedMode)
  {
    return createEmbedder(CaGe.nativesAvailable, isConstant,
     embed2D, embed3D, intensityFactor, embeddedMode);
  }

  public static Embedder createEmbedder
   (boolean nativesAvailable, boolean isConstant,
    String[][] embed2D, String[][] embed3D,
    float intensityFactor, int embeddedMode)
  {
    if (nativesAvailable) {
      return new NativeEmbedEmbedder(isConstant,
       embed2D, embed3D,
       intensityFactor, embeddedMode);
    } else {
      throw new RuntimeException(
       "No non-native embedder object implemented yet");
    }
  }
}


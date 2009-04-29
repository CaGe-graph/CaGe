package cage;

/**
 * Utility class for the creation of embedders.
 */
public class EmbedFactory {

    /**
     * Creates a native, non-constant embedder that ignores old embeddings and with an intensity factor of 1.0.
     * 
     * @param embed2D The commands for the 2D embedder
     * @param embed3D The commands for the 3D embedder
     * @return an embedder object
     * @see cage.NativeEmbedEmbedder
     */
    public static Embedder createEmbedder(String[][] embed2D, String[][] embed3D) {
        return createEmbedder(CaGe.nativesAvailable, false,
                embed2D, embed3D, 1.0f, Embedder.IGNORE_OLD_EMBEDDING);
    }

    /**
     * Creates a native embedder that ignores old embeddings and with an intensity factor of 1.0.
     *
     * @param isConstant Flag to indicate whether this is a constant embedder
     * @param embed2D The commands for the 2D embedder
     * @param embed3D The commands for the 3D embedder
     * @return an embedder object
     * @see cage.NativeEmbedEmbedder
     */
    public static Embedder createEmbedder(boolean isConstant, String[][] embed2D, String[][] embed3D) {
        return createEmbedder(CaGe.nativesAvailable, isConstant,
                embed2D, embed3D, 1.0f, Embedder.IGNORE_OLD_EMBEDDING);
    }

    /**
     * Creates an embedder that ignores old embeddings and with an intensity factor of 1.0.
     *
     * @param nativesAvailable Flag to indicate whether native embedders are available
     * @param isConstant Flag to indicate whether this is a constant embedder
     * @param embed2D The commands for the 2D embedder
     * @param embed3D The commands for the 3D embedder
     * @return an embedder object
     * @see cage.NativeEmbedEmbedder
     */
    public static Embedder createEmbedder(boolean nativesAvailable, boolean isConstant,
            String[][] embed2D, String[][] embed3D) {
        return createEmbedder(nativesAvailable, isConstant,
                embed2D, embed3D, 1.0f, Embedder.IGNORE_OLD_EMBEDDING);
    }

    /**
     * Creates a native, non-constant embedder.
     *
     * @param embed2D The commands for the 2D embedder
     * @param embed3D The commands for the 3D embedder
     * @param intensityFactor The intensity factor, i.e. a factor for the number
     *                        of iterations.
     * @param embeddedMode One of the constants defined in {@link Embedder}
     * @return an embedder object
     * @see cage.NativeEmbedEmbedder
     */
    public static Embedder createEmbedder(String[][] embed2D, String[][] embed3D,
            float intensityFactor, int embeddedMode) {
        return createEmbedder(CaGe.nativesAvailable, false,
                embed2D, embed3D, intensityFactor, embeddedMode);
    }

    /**
     * Creates a native embedder.
     *
     * @param isConstant Flag to indicate whether this is a constant embedder
     * @param embed2D The commands for the 2D embedder
     * @param embed3D The commands for the 3D embedder
     * @param intensityFactor The intensity factor, i.e. a factor for the number
     *                        of iterations.
     * @param embeddedMode One of the constants defined in {@link Embedder}
     * @return an embedder object
     * @see cage.NativeEmbedEmbedder
     */
    public static Embedder createEmbedder(boolean isConstant,
            String[][] embed2D, String[][] embed3D,
            float intensityFactor, int embeddedMode) {
        return createEmbedder(CaGe.nativesAvailable, isConstant,
                embed2D, embed3D, intensityFactor, embeddedMode);
    }

    /**
     * Creates an embedder.
     *
     * @param nativesAvailable Flag to indicate whether native embedders are available
     * @param isConstant Flag to indicate whether this is a constant embedder
     * @param embed2D The commands for the 2D embedder
     * @param embed3D The commands for the 3D embedder
     * @param intensityFactor The intensity factor, i.e. a factor for the number
     *                        of iterations.
     * @param embeddedMode One of the constants defined in {@link Embedder}
     * @return an embedder object
     * @see cage.NativeEmbedEmbedder
     */
    public static Embedder createEmbedder(boolean nativesAvailable, boolean isConstant,
            String[][] embed2D, String[][] embed3D,
            float intensityFactor, int embeddedMode) {
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


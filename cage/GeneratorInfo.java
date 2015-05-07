package cage;

/**
 * Abstract class that represents the settings for a CaGe generator.
 */
public abstract class GeneratorInfo {

    /**
     * Returns the generator. This generator consists of a 2-dimensional array
     * of <code>String</code>s. Each array of <code>String</code>s represents
     * a command and the different options needed for this command. These
     * commands and their options are then piped together in the order that
     * the arrays are organized in the array of arrays.
     * 
     * @return An array of arrays of <code>String</code>s representing the generator.
     */
    public abstract String[][] getGenerator();

    /**
     * Sets the generator. This generator consists of a 2-dimensional array
     * of <code>String</code>s. Each array of <code>String</code>s represents
     * a command and the different options needed for this command. These
     * commands and their options are then piped together in the order that
     * the arrays are organized in the array of arrays.
     *
     * @param generator An array of arrays of <code>String</code>s representing the
     * generator.
     */
    public abstract void setGenerator(String[][] generator);
    
    /**
     * Returns the default embedder for this generator. This allows
     * the embedder to be restored if it was altered.
     * 
     * @return An array of arrays of <code>String</code>s representing the embedder.
     */
    public abstract Embedder getDefaultEmbedder();

    /**
     * Returns the embedder to be used for this generator.
     *
     * @return The embedder for this generator.
     */
    public abstract Embedder getEmbedder();

    /**
     * Sets the embedder to be used for this generator.
     *
     * @param embedder The embedder for this generator.
     * @see EmbedFactory
     */
    public abstract void setEmbedder(Embedder embedder);

    /**
     * Returns the filename of the output file.
     *
     * @return The filename of the output file.
     */
    public abstract String getFilename();

    /**
     * Returns the maximum size of a face in the graphs from this generator. This
     * may be zero if the maximum size is unknown.
     *
     * @return The maximum size of a face in an outputted graph, or 0 if unknown.
     */
    public abstract int getMaxFacesize();

    /**
     * Returns whether the 2D embedding can be reembedded with another face as
     * outer face. This affects the {@link cage.viewer.TwoView} viewer's ability to react
     * to mouse clicks inside a face, asking for a new embedding in which this
     * face becomes the exterior one.
     *
     * @return <tt>true</tt> if reembedding is enable and <tt>false</tt> in
     * the other case
     */
    public abstract boolean isReembed2DEnabled();

    /**
     * Returns the <code>ElementRule</code> to be used for graphs from this
     * generator.
     *
     * @return An <code>ElementRule</code> object.
     */
    public abstract ElementRule getElementRule();

    /**
     * Returns the expert mode of this generator.
     *
     * @return the expert mode of this generator
     * @see #createExpertMode(int, int) 
     */
    public abstract int getExpertMode();

    /**
     * The name of the generator.
     */
    private String generatorName;

    /**
     * Sets the name of the generator
     * 
     * @param generatorName The generator name.
     * @see #getGeneratorName()
     */
    public void setGeneratorName(String generatorName) {
        this.generatorName = generatorName;
    }

    /**
     * Returns the name of the generator.
     *
     * @return The generator name.
     * @see #setGeneratorName(java.lang.String) 
     */
    public String getGeneratorName() {
        return generatorName;
    }

    /*
     * Constants to be used for expert modes. Can be combined to mix
     * modes.
     */

    /**
     * The no expert modes constant.
     */
    public static final int NO_EXPERT_MODE = 0;

    /**
     * The generator expert mode constant.  This can be combined with the other expert mode
     * constants (except NO_EXPERT_MODES) for mixed modes. This mode will make the
     * exact generator command visible.
     */
    public static final int GENERATOR_EXPERT = 1;

    /**
     * The embed expert mode constant.  This can be combined with the other expert mode
     * constants (except NO_EXPERT_MODES) for mixed modes. This mode will make the
     * exact embedder commands visible.
     */
    public static final int EMBED_EXPERT = 2;

    /**
     * The all expert mode constant.  This is a combination of the GENERATOR_EXPERT and
     * the EMBED_EXPERT mode.
     */
    public static final int ALL_EXPERT_MODES = 3;

    /** Used internally for shifting to the excluded modes */
    private static final int EXPERT_BITS = 10;

    /**
     * Creates an expert mode that both stores the included modes as the explicitly
     * excluded modes.
     *
     * @param inclusion
     * @param exclusion
     * @return A new expert mode containing the requested inclusions and exclusions.
     */
    public static int createExpertMode(int inclusion, int exclusion) {
        //the exclusion bits correspond with their respectively constants shifted
        //EXPERT_BITS to the left.
        return (inclusion & ALL_EXPERT_MODES) |
                ((exclusion & ALL_EXPERT_MODES) << EXPERT_BITS);
    }

    /**
     * Returns the included expert modes. The difference with {@link #getExpertMode}
     * is that this method only returns the included modes and thus returns one of
     * {@link #NO_EXPERT_MODE}, {@link #GENERATOR_EXPERT}, {@link #EMBED_EXPERT} and
     * {@link #ALL_EXPERT_MODES}.
     * 
     * @return {@link #NO_EXPERT_MODE}, {@link #GENERATOR_EXPERT}, {@link #EMBED_EXPERT}
     *         or {@link #ALL_EXPERT_MODES}, depending on which mode is set.
     */
    public int getIncludedExpertModes() {
        return getExpertMode() & ALL_EXPERT_MODES;
    }

    /**
     * Returns the excluded expert modes. This method returns one of
     * {@link #NO_EXPERT_MODE}, {@link #GENERATOR_EXPERT}, {@link #EMBED_EXPERT} and
     * {@link #ALL_EXPERT_MODES}. For this method to work correct and keep working
     * correct an expert mode should be created using {@link #createExpertMode(int,int)}.
     *
     * @return {@link #NO_EXPERT_MODE}, {@link #GENERATOR_EXPERT}, {@link #EMBED_EXPERT}
     *         or {@link #ALL_EXPERT_MODES}, depending on which modes are excluded.
     */
    public int getExcludedExpertModes() {
        return (getExpertMode() >> EXPERT_BITS) & ALL_EXPERT_MODES;
    }

    /**
     * Returns whether the current expert modes include the given modes.
     * 
     * @param modes The modes which need to be tested.
     * @return <tt>true</tt> if the current mode includes <tt>modes</tt>,
     *         <tt>false</tt> otherwise
     */
    public boolean expertModeIncludes(int modes) {
        return (getIncludedExpertModes() & modes) != 0;
    }

    /**
     * Returns whether the current expert modes exclude the given modes.
     *
     * @param modes The modes which need to be tested.
     * @return <tt>true</tt> if the current mode excludes <tt>modes</tt>,
     *         <tt>false</tt> otherwise
     */
    public boolean expertModeExcludes(int modes) {
        return (getExcludedExpertModes() & modes) != 0;
    }

    /**
     * TODO
     * 
     * @param modes
     * @param defaultIncluded
     * @return <tt>true</tt> if the current mode contains <tt>modes</tt>,
     *         <tt>false</tt> otherwise
     */
    public boolean expertModeContains(int modes, boolean defaultIncluded) {
        return defaultIncluded ? (getExcludedExpertModes() & modes) != modes : (getIncludedExpertModes() & modes) != 0;
    }
}

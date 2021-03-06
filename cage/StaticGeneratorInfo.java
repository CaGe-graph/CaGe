package cage;

import java.util.Arrays;

/**
 * Implementation of {@link GeneratorInfo} that sets all settings using
 * setters and doesn't calculate any of the values.
 */
public class StaticGeneratorInfo extends GeneratorInfo {

    private String[][] generator;
    private String[][] defaultGenerator;
    private Embedder embedder;
    private final Embedder defaultEmbedder;
    private String filename;
    private int maxFacesize;
    private boolean reembed2DEnabled;
    private ElementRule elementRule;
    private int expertMode;

    /**
     * Creates a <code>StaticGeneratorInfo</code> object with the given generator
     * commands, embedder, filename and maximum face size. Reembedding is enabled.
     * The expert mode is disabled for this generator and a default
     * {@link ValencyElementRule} is used that maps the vertices as follows:
     * <table border="1" align="center" cellspacing="0">
     * <tr align="center"><th>Degree</th><th>Element</th></tr>
     * <tr align="center"><td>1</td><td>H</td></tr>
     * <tr align="center"><td>2</td><td>O</td></tr>
     * <tr align="center"><td>3</td><td>C</td></tr>
     * <tr align="center"><td>4</td><td>Si</td></tr>
     * <tr align="center"><td>5</td><td>N</td></tr>
     * <tr align="center"><td>6</td><td>S</td></tr>
     * <tr align="center"><td>7</td><td>I</td></tr>
     * </table>
     *
     * @param generator The generator commands
     * @param embedder The embedder
     * @param filename The filename for export to file
     * @param maxFacesize The maximum face size
     */
    public StaticGeneratorInfo(String[][] generator,
            Embedder embedder,
            String filename,
            int maxFacesize) {
        this(generator, embedder, filename, maxFacesize, true);
    }

    /**
     * Creates a <code>StaticGeneratorInfo</code> object with the given generator
     * commands, embedder, filename and maximum face size. Reembedding is enabled
     * based on the value of <tt>reembed2DEnabled</tt>. The expert mode is disabled
     * for this generator and a default {@link ValencyElementRule} is used that maps
     * the vertices as follows:
     * <table border="1" align="center" cellspacing="0">
     * <tr align="center"><th>Degree</th><th>Element</th></tr>
     * <tr align="center"><td>1</td><td>H</td></tr>
     * <tr align="center"><td>2</td><td>O</td></tr>
     * <tr align="center"><td>3</td><td>C</td></tr>
     * <tr align="center"><td>4</td><td>Si</td></tr>
     * <tr align="center"><td>5</td><td>N</td></tr>
     * <tr align="center"><td>6</td><td>S</td></tr>
     * <tr align="center"><td>7</td><td>I</td></tr>
     * </table>
     *
     * @param generator The generator commands
     * @param embedder The embedder
     * @param filename The filename for export to file
     * @param maxFacesize The maximum face size
     * @param reembed2DEnabled Is reembedding enabled.
     */
    public StaticGeneratorInfo(String[][] generator,
            Embedder embedder,
            String filename,
            int maxFacesize,
            boolean reembed2DEnabled) {
        this(generator, embedder, filename, maxFacesize, reembed2DEnabled,
                new ValencyElementRule("H O C Si N S I"), 0);
    }

    public StaticGeneratorInfo(String[][] generator,
            Embedder embedder,
            String filename,
            int maxFacesize,
            ElementRule elementRule) {
        this(generator, embedder, filename, maxFacesize, true, elementRule, 0);
    }

    public StaticGeneratorInfo(String[][] generator,
            Embedder embedder,
            String filename,
            int maxFacesize,
            boolean reembed2DEnabled,
            ElementRule elementRule) {
        this(generator, embedder, filename, maxFacesize, reembed2DEnabled,
                elementRule, 0);
    }

    public StaticGeneratorInfo(String[][] generator,
            Embedder embedder,
            String filename,
            int maxFacesize,
            ElementRule elementRule,
            int expertMode) {
        this(generator, embedder, filename, maxFacesize, true,
                elementRule, expertMode);
    }

    public StaticGeneratorInfo(String[][] generator,
            Embedder embedder,
            String filename,
            int maxFacesize,
            boolean reembed2DEnabled,
            ElementRule elementRule,
            int expertMode) {
        this.generator = stringArrayDeepCopy(generator);
        this.defaultGenerator = stringArrayDeepCopy(generator);
        this.embedder = embedder;
        this.defaultEmbedder = EmbedFactory.duplicateEmbedder(embedder);
        this.filename = filename;
        this.maxFacesize = maxFacesize;
        this.reembed2DEnabled = reembed2DEnabled;
        this.elementRule = elementRule;
        this.expertMode = expertMode;
    }

    @Override
    public String[][] getGenerator() {
        return generator;
    }

    @Override
    public String[][] getDefaultGenerator() {
        return stringArrayDeepCopy(defaultGenerator);
    }

    @Override
    public void setGenerator(String[][] generator) {
        this.generator = generator;
    }

    @Override
    public Embedder getEmbedder() {
        return embedder;
    }

    @Override
    public Embedder getDefaultEmbedder() {
        return defaultEmbedder;
    }

    @Override
    public void setEmbedder(Embedder embedder) {
        this.embedder = embedder;
    }

    @Override
    public String getFilename() {
        return filename;
    }

    @Override
    public int getMaxFacesize() {
        return maxFacesize;
    }

    @Override
    public boolean isReembed2DEnabled() {
        return reembed2DEnabled;
    }

    /**
     * Returns the element rule to be used with this generator.
     * 
     * @return The element rule to be used with this generator.
     */
    @Override
    public ElementRule getElementRule() {
        return elementRule;
    }

    @Override
    public int getExpertMode() {
        return expertMode;
    }
    
    private static final String [][] stringArrayDeepCopy(String [][] orig){
        String[][] copy = new String[orig.length][];
        for (int i = 0; i < copy.length; i++)
             copy[i] = Arrays.copyOf(orig[i], orig[i].length);
        return copy;
    }
}

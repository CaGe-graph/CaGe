package cage;

/**
 * An interface common to all possible types of output in CaGe, i.e. all the
 * writers and viewers.
 */
public interface CaGeOutlet {

    /**
     * Sets the generator info provided for this outlet. This info can
     * contain hints for the output such as 
     * {@link GeneratorInfo#isReembed2DEnabled()}.
     *
     * @param generatorInfo the generator info provided for this outlet
     */
    void setGeneratorInfo(GeneratorInfo generatorInfo);

    /**
     * Sets the dimension for which this outlet will be used. This method may
     * throw a <code>RunTimeException</code> when <tt>d</tt> is an unsupported
     * dimension.
     *
     * @param d The dimension for which this outlet will be used
     */
    void setDimension(int d);

    /**
     * Returns the dimension which is needed for this outlet.
     *
     * @return the dimension which is needed for this outlet.
     */
    int getDimension();

    /**
     * Does the actual output of a result. This method takes the graph, wrapped
     * in a <code>CaGeResult</code> object, as input and performs the method
     * of output specific to this outlet.
     *
     * @param result a <code>CaGeResult</code> object that needs to be
     * outputted.
     */
    void outputResult(CaGeResult result);

    /**
     * Called when this outlet is no longer needed. The outlet can then perform
     * any necessary clean up tasks such as closing streams.
     */
    void stop();
}

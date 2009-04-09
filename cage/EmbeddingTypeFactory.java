package cage;

/**
 * Interface that defines a factory for embedding types. These types are used
 * in the external generator panel for the section "Embed as:". The type string
 * is used to represent the type in the list. Of course can any generator also
 * use these factories internally.
 * 
 * @author nvcleemp
 */
public interface EmbeddingTypeFactory {

    public String[] getEmbeddingTypes();

    /**
     * Returns a new <code>Embedder</code> for the given <tt>type</tt> or
     * <tt>null</tt> if the type is unknown.
     *
     * @param type A type token for a certain embedder
     * @return a new <code>Embedder</code> or <tt>null</tt> if the type is unknown
     */
    public Embedder getEmbedderFor(String type);

}

package cage.decoration.embedder;

import cage.decoration.Decoration;
import cage.decoration.EmbeddedDecorationGraph;

/**
 * Interface for a general embedder for decorations.
 * 
 * @author nvcleemp
 */
public interface DecorationEmbedder {

    SingleChamberDecorationEmbedder setDecoration(Decoration decoration);

    SingleChamberDecorationEmbedder run();

    EmbeddedDecorationGraph export();
    
}

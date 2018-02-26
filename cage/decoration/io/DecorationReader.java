package cage.decoration.io;

import cage.decoration.Decoration;

/**
 * Interface for classes which can read decorations from some source.
 * @author nvcleemp
 */
public interface DecorationReader {
    Decoration getNext();
}

package cage.background;

import cage.CaGeRunner;

/**
 * Common interface for all background runners
 * 
 * @author nvcleemp
 */
public interface BackgroundRunner extends CaGeRunner{
    void start();
    boolean isAlive();
    void abort();
    
    String getInfoText();
}

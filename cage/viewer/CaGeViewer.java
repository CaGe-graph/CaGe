package cage.viewer;

import cage.CaGeOutlet;
import cage.ResultPanel;

/**
 * Interface that must be implemented by viewers for CaGe.
 */
public interface CaGeViewer extends CaGeOutlet {

    /**
     * Sets the <code>ResultPanel</code> connected with this viewer. Some
     * viewer need this to reembed the shown graph.
     * 
     * @param resultPanel
     */
    void setResultPanel(ResultPanel resultPanel);

    void setVisible(boolean isVisible);
}

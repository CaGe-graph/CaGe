package cage.viewer;

import cage.CaGeOutlet;
import cage.ResultPanel;

public interface CaGeViewer extends CaGeOutlet {

    void setResultPanel(ResultPanel resultPanel);

    void setVisible(boolean isVisible);
}

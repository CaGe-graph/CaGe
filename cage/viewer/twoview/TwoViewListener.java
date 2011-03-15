/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package cage.viewer.twoview;

import cage.CaGeResult;

/**
 *
 * @author nvcleemp
 */
public interface TwoViewListener {

    public void edgeBrightnessChanged();
    public void edgeWidthChanged();
    
    public void vertexNumbersShownChanged();
    public void vertexSizeChanged();
    public void vertexSizeIDChanged();

    public void highlightedFacesChanged();

    public void resultChanged();
    public void generatorInfoChanged();

    public void prepareReembedding();
    public void startReembedding();
    public void reembeddingFinished(CaGeResult caGeResult);

}

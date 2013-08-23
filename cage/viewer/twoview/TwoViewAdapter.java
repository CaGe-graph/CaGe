/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package cage.viewer.twoview;

import cage.CaGeResult;

/**
 * Empty implementation of {@code TwoViewListener}.
 * @author nvcleemp
 */
public abstract class TwoViewAdapter implements TwoViewListener{

    @Override
    public void edgeBrightnessChanged() {
    }

    @Override
    public void edgeWidthChanged() {
    }

    @Override
    public void vertexNumbersShownChanged() {
    }

    @Override
    public void vertexSizeChanged() {
    }

    @Override
    public void highlightedFacesChanged() {
    }

    @Override
    public void resultChanged() {
    }

    @Override
    public void generatorInfoChanged() {
    }

    @Override
    public void prepareReembedding() {
    }

    @Override
    public void startReembedding() {
    }

    @Override
    public void reembeddingFinished(CaGeResult caGeResult) {
    }

}

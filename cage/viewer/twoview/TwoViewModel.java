/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package cage.viewer.twoview;

import cage.CaGeResult;
import cage.EmbedThread;
import cage.GeneratorInfo;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author nvcleemp
 */
public class TwoViewModel {

    private PropertyChangeListener listener = new PropertyChangeListener() {
        public void propertyChange(PropertyChangeEvent evt) {
            fireReembeddingFinished((CaGeResult) evt.getNewValue());
            embedderRunning = false;
        }
    };

    private boolean showNumbers = false;
    private int edgeWidth;
    private int vertexSize;
    private float edgeBrightness = 0.75f;
    
    private boolean embedderRunning = false;

    private CaGeResult result;
    private GeneratorInfo generatorInfo;

    public boolean getShowNumbers() {
        return showNumbers;
    }

    public void setShowNumbers(boolean showNumbers) {
        if(showNumbers!=this.showNumbers){
            this.showNumbers = showNumbers;
            fireVertexNumbersShownChanged();
        }
    }

    public int getEdgeWidth() {
        return edgeWidth;
    }

    public void setEdgeWidth(int edgeWidth) {
        if(this.edgeWidth != edgeWidth){
            this.edgeWidth = edgeWidth;
            fireEdgeWidthChanged();
        }
    }

    public int getVertexSize() {
        return vertexSize;
    }

    public void setVertexSize(int vertexSize) {
        if(this.vertexSize != vertexSize){
            this.vertexSize = vertexSize;
            fireVertexSizeChanged();
        }
    }

    public float getEdgeBrightness() {
        return edgeBrightness;
    }

    public void setEdgeBrightness(float edgeBrightness) {
        if(this.edgeBrightness != edgeBrightness){
            this.edgeBrightness = edgeBrightness;
            fireEdgeBrightnessChanged();
        }
    }

    public CaGeResult getResult() {
        return result;
    }

    public void setResult(CaGeResult result) {
        this.result = result;
        fireResultChanged();
    }

    public GeneratorInfo getGeneratorInfo() {
        return generatorInfo;
    }

    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        this.generatorInfo = generatorInfo;
        fireGeneratorInfoChanged();
    }

    public void reembedGraph(EmbedThread embedThread){
        if (embedThread != null) {
            embedderRunning = true;
            firePrepareReembedding();
            fireStartReembedding();
            embedThread.embed(result, listener, false, false, true);
        }
    }

    public void resetEmbedding(EmbedThread embedThread){
        if (embedThread != null && !embedderRunning) {
            embedderRunning = true;
            firePrepareReembedding();
            fireStartReembedding();
            embedThread.embed(result, listener, true, false, false);
        }
    }

    private List<TwoViewListener> listeners = new ArrayList<TwoViewListener>();

    public void addTwoViewListener(TwoViewListener listener){
        listeners.add(listener);
    }

    public void removeTwoViewListener(TwoViewListener listener){
        listeners.remove(listener);
    }

    private void fireEdgeBrightnessChanged(){
        for (TwoViewListener l : listeners) {
            l.edgeBrightnessChanged();
        }
    }

    private void fireEdgeWidthChanged(){
        for (TwoViewListener l : listeners) {
            l.edgeWidthChanged();
        }
    }

    private void fireVertexSizeChanged(){
        for (TwoViewListener l : listeners) {
            l.vertexSizeChanged();
        }
    }

    private void fireVertexNumbersShownChanged(){
        for (TwoViewListener l : listeners) {
            l.vertexNumbersShownChanged();
        }
    }

    private void fireResultChanged(){
        for (TwoViewListener l : listeners) {
            l.resultChanged();
        }
    }

    private void fireGeneratorInfoChanged(){
        for (TwoViewListener l : listeners) {
            l.generatorInfoChanged();
        }
    }

    private void firePrepareReembedding(){
        for (TwoViewListener l : listeners) {
            l.prepareReembedding();
        }
    }

    private void fireStartReembedding(){
        for (TwoViewListener l : listeners) {
            l.startReembedding();
        }
    }

    private void fireReembeddingFinished(CaGeResult caGeResult){
        for (TwoViewListener l : listeners) {
            l.reembeddingFinished(caGeResult);
        }
    }

}

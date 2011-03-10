/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package cage.viewer.twoview;

import cage.CaGeResult;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author nvcleemp
 */
public class TwoViewModel {

    private boolean showNumbers = false;
    private int edgeWidth;
    private int vertexSize;
    private float edgeBrightness = 0.75f;

    private CaGeResult result;

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

}

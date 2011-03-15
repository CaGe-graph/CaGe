/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package cage.viewer.twoview;

import cage.CaGe;
import cage.CaGeResult;
import cage.EmbedThread;
import cage.GeneratorInfo;
import cage.utility.Debug;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.List;

import lisken.systoolbox.Systoolbox;

/**
 *
 * @author nvcleemp
 */
public class TwoViewModel {

    public static final int MIN_EDGE_WIDTH = 0;
    public static final int MAX_EDGE_WIDTH = 9;
    public static final int DEFAULT_EDGE_WIDTH = MAX_EDGE_WIDTH / 2;

    private PropertyChangeListener listener = new PropertyChangeListener() {
        public void propertyChange(PropertyChangeEvent evt) {
            final CaGeResult cageResult = (CaGeResult) evt.getNewValue();
            if(cageResult.equals(result)){
                /*
                 * only fire when the graph is still the same graph that has
                 * been re-embedded.
                 */
                fireReembeddingFinished(cageResult);
            }
            embedderRunning = false;
        }
    };

    private boolean showNumbers;
    private int edgeWidth;
    private int vertexSize; //the size of the image
    private int vertexSizeID; //the number of the corresponding checkbox
    private final int vertexSizesCount;
    private float edgeBrightness = 0.75f;

    private boolean highlightFaces = false;
    private int highlightedFaces = 5;
    
    private boolean embedderRunning = false;

    private CaGeResult result;
    private GeneratorInfo generatorInfo;

    public TwoViewModel() {
        //initialize vertex sizes from the configuration file
        try {
            vertexSizesCount = Integer.parseInt(CaGe.config.getProperty("TwoView.MaxVertexSize"));
        } catch (NumberFormatException numberFormatException) {
            Debug.reportException(numberFormatException);
            throw new RuntimeException("Couldn't read TwoView.MaxVertexSize from configuration");
        }

        try {
            vertexSizeID = Integer.parseInt(CaGe.config.getProperty("TwoView.VertexSize"))-1;
            if (vertexSizeID < 0) {
                vertexSizeID = 0;
            }
            if (vertexSizeID >= vertexSizesCount) {
                vertexSizeID = vertexSizesCount-1;
            }
        } catch (NumberFormatException numberFormatException) {
            Debug.reportException(numberFormatException);
            vertexSizeID = 1;
        }

        //initialize edge width
        edgeWidth = DEFAULT_EDGE_WIDTH;

        //initialize show numbers
        try {
            showNumbers = Systoolbox.parseBoolean(
                                CaGe.config.getProperty("TwoView.ShowNumbers"),
                                false);
        } catch (Exception e) {
            Debug.reportException(e);
            showNumbers = false;
        }
    }

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

    public int getVertexSizeID() {
        return vertexSizeID;
    }

    public void setVertexSizeID(int vertexSizeID) {
        if(this.vertexSizeID != vertexSizeID){
            this.vertexSizeID = vertexSizeID;
            fireVertexSizeIDChanged();
        }
    }

    public int getVertexSizesCount() {
        return vertexSizesCount;
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

    public boolean highlightFaces() {
        return highlightFaces;
    }

    public void setHighlightFaces(boolean highlightFaces) {
        if(this.highlightFaces!=highlightFaces){
            this.highlightFaces = highlightFaces;
            fireHighlightedFacesChanged();
        }
    }

    public int getHighlightedFacesSize() {
        return highlightedFaces;
    }

    public void setHighlightedFacesSize(int highlightedFaces) {
        if(this.highlightedFaces!=highlightedFaces && highlightedFaces > 2){
            this.highlightedFaces = highlightedFaces;
            fireHighlightedFacesChanged();
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

    private void fireVertexSizeIDChanged(){
        for (TwoViewListener l : listeners) {
            l.vertexSizeIDChanged();
        }
    }

    private void fireVertexNumbersShownChanged(){
        for (TwoViewListener l : listeners) {
            l.vertexNumbersShownChanged();
        }
    }

    private void fireHighlightedFacesChanged(){
        for (TwoViewListener l : listeners) {
            l.highlightedFacesChanged();
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

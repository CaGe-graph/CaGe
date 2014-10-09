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
    
    public static final int MIN_VERTEX_SIZE = 3;
    public static final int MAX_VERTEX_SIZE = 25;
    public static final int DEFAULT_VERTEX_SIZE = 11;
    
    private static final float MIN_EDGE_BRIGHTNESS = 0.0f;
    private static final float MAX_EDGE_BRIGHTNESS = 0.0f;
    private static final float DEFAULT_EDGE_BRIGHTNESS = 0.75f;

    private PropertyChangeListener listener = new PropertyChangeListener() {
        @Override
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
    private int vertexSize;
    private float edgeBrightness;

    private boolean highlightFaces = false;
    private int highlightedFaces = 5;
    
    private boolean embedderRunning = false;

    private CaGeResult result;
    private GeneratorInfo generatorInfo;

    private String graphComment = "";

    public TwoViewModel() {
        //initialize edge width
        try {
            edgeWidth = Integer.parseInt(
                    CaGe.config.getProperty("TwoView.EdgeWidth"));
            if(edgeWidth > MAX_EDGE_WIDTH || edgeWidth < MIN_EDGE_WIDTH){
                edgeWidth = DEFAULT_EDGE_WIDTH;
            }
        } catch (NumberFormatException e) {
            Debug.reportException(e);
            edgeWidth = DEFAULT_EDGE_WIDTH;
        }
        
        //initialize edge brightness
        try {
            edgeBrightness = Float.parseFloat(
                    CaGe.config.getProperty("TwoView.EdgeBrightness"));
            if(edgeBrightness > MAX_EDGE_BRIGHTNESS 
                    || edgeBrightness < MIN_EDGE_BRIGHTNESS){
                edgeBrightness = DEFAULT_EDGE_BRIGHTNESS;
            }
        } catch (NumberFormatException e) {
            Debug.reportException(e);
            edgeBrightness = DEFAULT_EDGE_BRIGHTNESS;
        }

        //initialize show numbers
        try {
            showNumbers = Systoolbox.parseBoolean(
                                CaGe.config.getProperty("TwoView.ShowNumbers"),
                                false);
        } catch (Exception e) {
            Debug.reportException(e);
            showNumbers = false;
        }
        
        //initialize vertex size
        try {
            vertexSize = Integer.parseInt(
                    CaGe.config.getProperty("TwoView.VertexSize"));
            if(vertexSize > MAX_VERTEX_SIZE || vertexSize < MIN_VERTEX_SIZE){
                vertexSize = DEFAULT_VERTEX_SIZE;
            }
        } catch (NumberFormatException e) {
            Debug.reportException(e);
            vertexSize = DEFAULT_VERTEX_SIZE;
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
        if(this.vertexSize != vertexSize && vertexSize>=MIN_VERTEX_SIZE && vertexSize<=MAX_VERTEX_SIZE){
            this.vertexSize = vertexSize;
            fireVertexSizeChanged();
        }
    }

    public int getVertexSizesCount() {
        return MAX_VERTEX_SIZE - MIN_VERTEX_SIZE + 1;
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
        graphComment = result.getGraph().getComment();
        if (graphComment == null) {
            graphComment = "";
        }
        if (graphComment.length() > 0) {
            graphComment = " - " + graphComment;
        }
        graphComment = "Graph " + result.getGraphNo() + " - " + result.getGraph().getSize() + " vertices" + graphComment;
        fireResultChanged();
    }

    public GeneratorInfo getGeneratorInfo() {
        return generatorInfo;
    }

    public void setGeneratorInfo(GeneratorInfo generatorInfo) {
        this.generatorInfo = generatorInfo;
        fireGeneratorInfoChanged();
    }

    public String getGraphComment() {
        return graphComment;
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

    private List<TwoViewListener> listeners = new ArrayList<>();

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

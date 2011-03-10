/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package cage.viewer.twoview;

/**
 *
 * @author nvcleemp
 */
public interface TwoViewListener {

    public void edgeBrightnessChanged();
    public void edgeWidthChanged();
    
    public void vertexNumbersShownChanged();
    public void vertexSizeChanged();

    public void resultChanged();

}

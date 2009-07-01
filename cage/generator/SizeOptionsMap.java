package cage.generator;

import java.awt.Component;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.List;
import java.util.TreeMap;
import javax.swing.BoundedRangeModel;
import javax.swing.JPanel;
import javax.swing.JToggleButton;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.systoolbox.MutableInteger;

public class SizeOptionsMap extends TreeMap implements ChangeListener, ActionListener {

    //the panel on which the options for the allowed sizes are shown
    private JPanel optionsPanel;
    //the component that is used for the selection of the sizes (usually a slider)
    private Component sizesComponent;
    //the model that contains the bounds for the face sizes or the vertex degrees
    private BoundedRangeModel sizesModel;
    //the toggle button to add or remove faces
    private JToggleButton includedButton;
    //if true then were talking about vertex degrees and not face sizes
    private boolean dual;
    //if true the user can limit the number of faces of a certain size
    private boolean limitable;

    /**
     * Creates a new <code>SizeOptionsMap</code> that allows a user to add certain
     * allowed sizes (face sizes or vertex degrees). The panel <tt>optionsPanel</tt> is used to add controls
     * to which allow the user to easily remove an allowed size or limit the number
     * of those faces/vertices. For this to work correctly it is assumed that this panel has
     * a <code>GridBagLayout</code>. The constructor explicitly sets this layout and
     * you should not alter this at a later point. The component <tt>facesComponent</tt>
     * is the component that is used for selecting the face size and will usually be
     * a <code>JSlider</code> or a <code>JSpinner</code>. A reference to this component
     * is only used to give it the focus. The <tt>facesModel</tt> is used to determine
     * the face size that needs to be added or removed when <tt>includedButton</tt> is pressed.
     *
     * @param optionsPanel The panel on which the options for the allowed gons are shown
     * @param sizesComponent The component that is used for the selection of the sizes (usually a slider)
     * @param sizesModel The model that shows which size needs to be added or removed.
     * @param includedButton The button used to add or remove sizes.
     */
    public SizeOptionsMap(JPanel optionsPanel, Component sizesComponent, BoundedRangeModel sizesModel, JToggleButton includedButton) {
        this(optionsPanel, sizesComponent, sizesModel, includedButton, false, true);
    }

    /**
     * Creates a new <code>SizeOptionsMap</code> that allows a user to add certain
     * allowed face sizes or vertex degree (in case <tt>dual</tt> is <tt>true</tt>).
     * The panel <tt>optionsPanel</tt> is used to add controls to which allow the user
     * to easily remove an allowed face size (respectively vertex degree) or limit the number
     * of those faces (respectively vertices) if <tt>limitable<tt> is <tt>true</tt>.
     * For this to work correctly it is assumed that this panel has a
     * <code>GridBagLayout</code>. The constructor explicitly sets this layout and
     * you should not alter this at a later point. The component <tt>facesComponent</tt>
     * is the component that is used for selecting the face size (respectively vertex
     * degree )and will usually be a <code>JSlider</code> or a <code>JSpinner</code>.
     * A reference to this component is only used to give it the focus. The
     * <tt>facesModel</tt> is used to determine the face size (respectively vertex
     * degree) that needs to be added or removed when <tt>includedButton</tt> is pressed.
     *
     * @param optionsPanel The panel on which the options for the allowed gons are shown
     * @param sizesComponent The component that is used for the selection of the sizes (usually a slider)
     * @param sizesModel The model that shows which size needs to be added or removed.
     * @param includedButton The button used to add or remove sizes.
     * @param dual If <tt>true</tt> this object is used for vertex degrees.
     * @param limitable If <tt>true</tt> the user can limit the number of faces (resp. vertices)
     *                  with a certain size (resp. degree).
     */
    public SizeOptionsMap(JPanel optionsPanel, Component sizesComponent, BoundedRangeModel sizesModel, JToggleButton includedButton, boolean dual, boolean limitable) {
        this.dual = dual;
        this.limitable = limitable;
        this.optionsPanel = optionsPanel;
        this.sizesComponent = sizesComponent;
        this.sizesModel = sizesModel;
        this.includedButton = includedButton;
        this.sizesModel.addChangeListener(this);
        this.includedButton.addActionListener(this);
        this.optionsPanel.setLayout(new GridBagLayout());
        stateChanged(new ChangeEvent(sizesModel));
    }

    public void setSizeIncluded(int size, boolean included) {
        //TODO: this uses a mutable object as a key in a map!!! Why?
        MutableInteger key = new MutableInteger(size);
        SizeOption sizeOption = (SizeOption) this.get(key);
        if (included) {
            if (sizeOption == null) {
                sizeOption = new SizeOption(size, this);
                sizeOption.addTo(optionsPanel, dual, limitable);
            } else if (!sizeOption.isActive()) {
                sizeOption.reactivate();
            } else {
                return;
            }
            this.put(key, sizeOption);
            fireStateChanged();
        } else {
            
            if (sizeOption != null) {
                if (sizeOption.isActive()) {
                    sizeOption.deactivate();
                    this.put(key, sizeOption);
                    fireStateChanged();
                }
            }

        }
        if (sizesModel.getValue() == size) {
            stateChanged(new ChangeEvent(this));
        }
    }

    public void stateChanged(ChangeEvent e) {
        int faces = sizesModel.getValue();
        SizeOption sizeOption = (SizeOption) this.get(new MutableInteger(faces));
        boolean included = sizeOption == null ? false : sizeOption.isActive();
        includedButton.setSelected(included);
        if(!dual)
            includedButton.setText((included ? "discard " : "include ") + faces + "-gons");
        else
            includedButton.setText((included ? "discard " : "include ") + "degree " + faces);
    }

    public void actionPerformed(ActionEvent e) {
        Object source = e.getSource();
        MutableInteger key = null;
        SizeOption sizeOption;
        boolean included = false;
        int size = 0;
        if (source.equals(includedButton)) {
            size = sizesModel.getValue();
            key = new MutableInteger(size);
            included = includedButton.isSelected();
        } else if (source instanceof MutableInteger) {
            key = (MutableInteger) source;
            size = key.intValue();
            included = e.getID() != 0;
        }
        setSizeIncluded(size, included);
        sizeOption = (SizeOption) this.get(key);
        if (included) {
            sizeOption.focusToLimitControl();
        } else {
            sizesComponent.requestFocus();
        }
    }

    private List changeListenersList = new ArrayList();
    private ChangeEvent e = new ChangeEvent(this);

    /**
     * Add a <code>ChangeListener</code> to this object.
     * @param l The <code>ChangeListener</code> to add.
     */
    public void addChangeListener(ChangeListener l){
        changeListenersList.add(l);
    }

    /**
     * Remove a <code>ChangeListener</code> from this object.
     * @param l The <code>ChangeListener</code> to remove.
     */
    public void removeChangeListener(ChangeListener l){
        changeListenersList.remove(l);
    }

    /**
     * Notify all <code>ChangeListener</code>s of a change.
     */
    private void fireStateChanged(){
        for (int i = 0; i < changeListenersList.size(); i++) {
            ((ChangeListener)changeListenersList.get(i)).stateChanged(e);
        }
    }
}


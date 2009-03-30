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

public class GonOptionsMap extends TreeMap implements ChangeListener, ActionListener {

    //the panel on which the options for the allowed gons are shown
    private JPanel optionsPanel;
    //the component that is used for the selection of the faces (usually a slider)
    private Component facesComponent;
    //the model that contains the bounds for the face sizes
    private BoundedRangeModel facesModel;
    //the toggle button to add or remove faces
    private JToggleButton includedButton;
    //if true then were talking about vertex degrees and not face sizes
    private boolean dual;
    //if true the user can limit the number of faces of a certain size
    private boolean limitable;

    /**
     * Creates a new <code>GonOptionsMap</code> that allows a user to add certain
     * allowed face sizes. The panel <tt>optionsPanel</tt> is used to add controls
     * to which allow the user to easily remove an allowed face size or limit the number
     * of those faces. For this to work correctly it is assumed that this panel has
     * a <code>GridBagLayout</code>. The constructor explicitly sets this layout and
     * you should not alter this at a later point. The component <tt>facesComponent</tt>
     * is the component that is used for selecting the face size and will usually be
     * a <code>JSlider</code> or a <code>JSpinner</code>. A reference to this component
     * is only used to give it the focus. The <tt>facesModel</tt> is used to determine
     * the face size that needs to be added or removed when <tt>includedButton</tt> is pressed.
     *
     * @param optionsPanel The panel on which the options for the allowed gons are shown
     * @param facesComponent The component that is used for the selection of the faces (usually a slider)
     * @param facesModel The model that shows which face size needs to be added or remoed.
     * @param includedButton The button used to add or remove face sizes.
     */
    public GonOptionsMap(JPanel optionsPanel, Component facesComponent, BoundedRangeModel facesModel, JToggleButton includedButton) {
        this(optionsPanel, facesComponent, facesModel, includedButton, false, true);
    }

    /**
     * Creates a new <code>GonOptionsMap</code> that allows a user to add certain
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
     * @param facesComponent The component that is used for the selection of the faces (usually a slider)
     * @param facesModel The model that shows which face size needs to be added or remoed.
     * @param includedButton The button used to add or remove face sizes.
     * @param dual If <tt>true</tt> this object is used for vertex degrees.
     * @param limitable If <tt>true</tt> the user can limit the number of faces (resp. vertices)
     *                  with a certain size (resp. degree).
     */
    public GonOptionsMap(JPanel optionsPanel, Component facesComponent, BoundedRangeModel facesModel, JToggleButton includedButton, boolean dual, boolean limitable) {
        this.dual = dual;
        this.limitable = limitable;
        this.optionsPanel = optionsPanel;
        this.facesComponent = facesComponent;
        this.facesModel = facesModel;
        this.includedButton = includedButton;
        this.facesModel.addChangeListener(this);
        this.includedButton.addActionListener(this);
        this.optionsPanel.setLayout(new GridBagLayout());
        stateChanged(new ChangeEvent(facesModel));
    }

    public void setGonIncluded(int faces, boolean included) {
        MutableInteger key = new MutableInteger(faces);
        GonOption gonOption = (GonOption) this.get(key);
        if (included) {

            if (gonOption == null) {
                gonOption = new GonOption(faces, this);
                gonOption.addTo(optionsPanel, dual, limitable);
            } else if (!gonOption.isActive()) {
                gonOption.reactivate();
            } else {
                return;
            }
            this.put(key, gonOption);
            fireStateChanged();
        } else {
            
            if (gonOption != null) {
                if (gonOption.isActive()) {
                    gonOption.deactivate();
                    this.put(key, gonOption);
                    fireStateChanged();
                }
            }

        }
        if (facesModel.getValue() == faces) {
            stateChanged(new ChangeEvent(this));
        }
    }

    public void stateChanged(ChangeEvent e) {
        int faces = facesModel.getValue();
        GonOption gonOption = (GonOption) this.get(new MutableInteger(faces));
        boolean included = gonOption == null ? false : gonOption.isActive();
        includedButton.setSelected(included);
        if(!dual)
            includedButton.setText((included ? "discard " : "include ") + faces + "-gons");
        else
            includedButton.setText((included ? "discard " : "include ") + "degree " + faces);
    }

    public void actionPerformed(ActionEvent e) {
        Object source = e.getSource();
        MutableInteger key = null;
        GonOption gonOption;
        boolean included = false;
        int faces = 0;
        if (source.equals(includedButton)) {
            faces = facesModel.getValue();
            key = new MutableInteger(faces);
            included = includedButton.isSelected();
        } else if (source instanceof MutableInteger) {
            key = (MutableInteger) source;
            faces = key.intValue();
            included = e.getID() != 0;
        }
        setGonIncluded(faces, included);
        gonOption = (GonOption) this.get(key);
        if (included) {
            gonOption.focusToLimitControl();
        } else {
            facesComponent.requestFocus();
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


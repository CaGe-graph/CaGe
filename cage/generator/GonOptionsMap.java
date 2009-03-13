package cage.generator;

import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.TreeMap;
import javax.swing.AbstractButton;
import javax.swing.BoundedRangeModel;
import javax.swing.JPanel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.systoolbox.MutableInteger;

class GonOptionsMap extends TreeMap implements ChangeListener, ActionListener {

    JPanel optionsPanel;
    Component facesComponent;
    BoundedRangeModel facesModel;
    AbstractButton includedButton;
    boolean dual;
    boolean limited;

    //p should have a GridBagLayout
    //b should be a JToggleButton
    public GonOptionsMap(JPanel p, Component c, BoundedRangeModel r, AbstractButton b) {
        this(p, c, r, b, false, true);
    }

    public GonOptionsMap(JPanel p, Component c, BoundedRangeModel r, AbstractButton b, boolean d, boolean l) {
        dual = d;
        limited = l;
        optionsPanel = p;
        facesComponent = c;
        facesModel = r;
        includedButton = b;
        facesModel.addChangeListener(this);
        includedButton.addActionListener(this);
        this.stateChanged(new ChangeEvent(facesModel));
    }

    public void setGonIncluded(int faces, boolean included) {
        MutableInteger key = new MutableInteger(faces);
        GonOption gonOption = (GonOption) this.get(key);
        if (included) {

            if (gonOption == null) {
                gonOption = new GonOption(faces, this);
                gonOption.addTo(optionsPanel, dual, limited);
            } else if (!gonOption.isActive()) {
                gonOption.reactivate();
            } else {
                return;
            }
            this.put(key, gonOption);

        } else {
            
            if (gonOption != null) {
                if (gonOption.isActive()) {
                    gonOption.deactivate();
                    this.put(key, gonOption);
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
        if (source == (Object) includedButton) {
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
            if(gonOption.limitGons!=null)
                gonOption.limitGons.requestFocus();
        } else {
            facesComponent.requestFocus();
        }
    }
}


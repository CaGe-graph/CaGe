package lisken.uitoolbox;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.BoundedRangeModel;
import javax.swing.ButtonModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

/**
 * A <code>MinMaxEqListener</code> maintains the constraints between two
 * <code>BoundedRangeModel</code>s where one model represents the minimum and
 * the other the maximum of the same quantity. These constraints are the following:
 * <ul>
 * <li>the minimum is always smaller than or equal to the maximum;</li>
 * <li>if equality is needed, then both values must be the same.</li>
 * </ul>
 */
public class MinMaxEqListener {

    private BoundedRangeModel minModel, maxModel, lastChangedModel;
    private boolean equality, mayVeto;
    
    private ChangeListener changeListener = new ChangeListener() {

        public void stateChanged(ChangeEvent e) {
            BoundedRangeModel m = (BoundedRangeModel) e.getSource();
            enforceConstraints(m, mayVeto);
        }
    };
    
    private ActionListener actionListener = new ActionListener() {

        public void actionPerformed(ActionEvent e) {
            equality = ((ButtonModel) e.getSource()).isSelected();
            enforceConstraints(lastChangedModel, false);
        }
    };

    public MinMaxEqListener(BoundedRangeModel minM, BoundedRangeModel maxM, boolean staticEquality) {
        this(minM, maxM, staticEquality, false);
    }

    public MinMaxEqListener(BoundedRangeModel minM, BoundedRangeModel maxM, boolean staticEquality,
            boolean veto) {
        equality = staticEquality;
        init(minM, maxM, null, veto);
    }

    public MinMaxEqListener(BoundedRangeModel minM, BoundedRangeModel maxM, ButtonModel equalityButton) {
        this(minM, maxM, equalityButton, false);
    }

    public MinMaxEqListener(BoundedRangeModel minM, BoundedRangeModel maxM, ButtonModel equalityButton,
            boolean veto) {
        init(minM, maxM, equalityButton, veto);
    }

    void init(BoundedRangeModel minM, BoundedRangeModel maxM, ButtonModel equalityButton,
            boolean veto) {
        minModel = minM;
        maxModel = maxM;
        lastChangedModel = maxModel;
        minModel.addChangeListener(changeListener);
        maxModel.addChangeListener(changeListener);
        if (equalityButton != null) {
            equality = equalityButton.isSelected();
            equalityButton.addActionListener(actionListener);
        }
        enforceConstraints(maxModel, false);
        mayVeto = veto;
    }

    public void actionPerformed(ActionEvent e) {
        equality = ((ButtonModel) e.getSource()).isSelected();
        enforceConstraints(lastChangedModel, false);
    }

    void enforceConstraints(BoundedRangeModel changedModel, boolean mayVetoThis) {
        BoundedRangeModel modelToChange = otherModel(changedModel);
        if (modelToChange == null) {
            return;
        }
        lastChangedModel = changedModel;
        int v1, v2;
        v1 = minModel.getValue();
        v2 = maxModel.getValue();
        if (v1 == v2) {
            return;
        } else if (v1 <= v2 && !equality) {
            return;
        }
        if (mayVetoThis) {
            throw new RuntimeException("veto not yet implemented");
        } else {
            modelToChange.setValue(changedModel.getValue());
        }
    }

    BoundedRangeModel otherModel(BoundedRangeModel aModel) {
        if (aModel == minModel) {
            return maxModel;
        } else if (aModel == maxModel) {
            return minModel;
        }
        return null;
    }
}


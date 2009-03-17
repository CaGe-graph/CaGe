package lisken.uitoolbox;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.BoundedRangeModel;
import javax.swing.ButtonModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

public class MinMaxEqListener implements ChangeListener, ActionListener {

    BoundedRangeModel minModel, maxModel, lastChangedModel;
    boolean equality, mayVeto;

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
        minModel.addChangeListener(this);
        maxModel.addChangeListener(this);
        if (equalityButton != null) {
            equality = equalityButton.isSelected();
            equalityButton.addActionListener(this);
        }
        enforceConstraints(maxModel, false);
        mayVeto = veto;
    }

    public void actionPerformed(ActionEvent e) {
        equality = ((ButtonModel) e.getSource()).isSelected();
        enforceConstraints(lastChangedModel, false);
    }

    public void stateChanged(ChangeEvent e) {
        BoundedRangeModel m = (BoundedRangeModel) e.getSource();
        enforceConstraints(m, mayVeto);
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


package lisken.uitoolbox;

import javax.swing.BoundedRangeModel;
import javax.swing.ButtonModel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

/**
 * A <code>MinMaxRestrictor</code> maintains the constraints between two
 * <code>BoundedRangeModel</code>s where one model represents the minimum and
 * the other the maximum of the same quantity. These constraints are the following:
 * <ul>
 * <li>the minimum is always smaller than or equal to the maximum;</li>
 * <li>if equality is needed, then both values must be the same.</li>
 * </ul>
 */
public class MinMaxRestrictor {

    private BoundedRangeModel minModel, maxModel, lastChangedModel;
    private boolean equality, mayVeto;
    
    private ChangeListener changeListener = new ChangeListener() {

        public void stateChanged(ChangeEvent e) {
            BoundedRangeModel m = (BoundedRangeModel) e.getSource();
            enforceConstraints(m, mayVeto);
        }
    };
    
    private ChangeListener buttonChangeListener = new ChangeListener() {

        public void stateChanged(ChangeEvent e) {
            equality = ((ButtonModel) e.getSource()).isSelected();
            enforceConstraints(lastChangedModel, false);
        }
    };

    private MinMaxRestrictor(BoundedRangeModel minM, BoundedRangeModel maxM, boolean staticEquality) {
        this(minM, maxM, staticEquality, false);
    }

    private MinMaxRestrictor(BoundedRangeModel minM, BoundedRangeModel maxM, boolean staticEquality,
            boolean veto) {
        equality = staticEquality;
        init(minM, maxM, null, veto);
    }

    private MinMaxRestrictor(BoundedRangeModel minM, BoundedRangeModel maxM, ButtonModel equalityButton) {
        this(minM, maxM, equalityButton, false);
    }

    private MinMaxRestrictor(BoundedRangeModel minM, BoundedRangeModel maxM, ButtonModel equalityButton,
            boolean veto) {
        init(minM, maxM, equalityButton, veto);
    }

    private void init(BoundedRangeModel minM, BoundedRangeModel maxM, ButtonModel equalityButton,
            boolean veto) {
        minModel = minM;
        maxModel = maxM;
        lastChangedModel = maxModel;
        minModel.addChangeListener(changeListener);
        maxModel.addChangeListener(changeListener);
        if (equalityButton != null) {
            equality = equalityButton.isSelected();
            equalityButton.addChangeListener(buttonChangeListener);
        }
        enforceConstraints(maxModel, false);
        mayVeto = veto;
    }

    private void enforceConstraints(BoundedRangeModel changedModel, boolean mayVetoThis) {
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

    private BoundedRangeModel otherModel(BoundedRangeModel aModel) {
        if (aModel == minModel) {
            return maxModel;
        } else if (aModel == maxModel) {
            return minModel;
        }
        return null;
    }
    
    /**
     * Make sure that the value in minModel is always smaller than or equal to the value
     * in maxModel.
     * 
     * @param minModel The BoundedRangeModel containing the minimum value
     * @param maxModel The BoundedRangeModel containing the maximum value
     */
    @SuppressWarnings("ResultOfObjectAllocationIgnored")
    public static void keepConsistent(BoundedRangeModel minModel, BoundedRangeModel maxModel){
        new MinMaxRestrictor(minModel, maxModel, false);
    }
    
    /**
     * Make sure that the value in minModel is always smaller than or equal to the value
     * in maxModel.
     * 
     * @param minModel The BoundedRangeModel containing the minimum value
     * @param maxModel The BoundedRangeModel containing the maximum value
     */
    @SuppressWarnings("ResultOfObjectAllocationIgnored")
    public static void keepConsistentOrReject(BoundedRangeModel minModel, BoundedRangeModel maxModel){
        new MinMaxRestrictor(minModel, maxModel, false, true);
    }
    
    /**
     * Make sure that the value in minModel is always equal to the value in maxModel.
     * 
     * @param minModel The BoundedRangeModel containing the minimum value
     * @param maxModel The BoundedRangeModel containing the maximum value
     */
    @SuppressWarnings("ResultOfObjectAllocationIgnored")
    public static void keepEqual(BoundedRangeModel minModel, BoundedRangeModel maxModel){
        new MinMaxRestrictor(minModel, maxModel, true);
    }
    
    /**
     * Make sure that the value in minModel is always smaller than or equal to the value
     * in maxModel and keeps both values equal if the ButtonModel is selected.
     * 
     * @param minModel The BoundedRangeModel containing the minimum value
     * @param maxModel The BoundedRangeModel containing the maximum value
     * @param equalityButton The ButtonModel that decided whether the values need to be equal
     */
    @SuppressWarnings("ResultOfObjectAllocationIgnored")
    public static void keepConsistentOrEqual(BoundedRangeModel minModel, BoundedRangeModel maxModel, ButtonModel equalityButton){
        new MinMaxRestrictor(minModel, maxModel, equalityButton);
    }
}


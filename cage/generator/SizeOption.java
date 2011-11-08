package cage.generator;

import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import javax.swing.DefaultButtonModel;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.systoolbox.MutableInteger;
import lisken.uitoolbox.MinMaxEqListener;
import lisken.uitoolbox.SpinButton;
import lisken.uitoolbox.UItoolbox;

/**
 * This class represents the single configuration for one face type or vertex type
 * and is used in {@link SizeOptionsMap}.
 */
public class SizeOption implements ChangeListener, ActionListener {

    private boolean isIncluded;
    private int size;
    private int min;
    private int max;
    private JPanel panelToExtend;
    private JCheckBox sizeIncludedButton;
    private JLabel sizeLabel;
    private JCheckBox limitNrOfSize;
    private SpinButton minNrOfSizeButton;
    private SpinButton maxNrOfSizeButton;
    private SizeOptionsMap optionsMap;

    public SizeOption(int size, SizeOptionsMap m) {
        this.size = size;
        optionsMap = m;
        isIncluded = false;
        min = 0;
        max = CGFPanel.MAX_ATOMS;
        //TODO: remove this dependency on CGFPanel since this now also used in other generators
    }

    public void addTo(JPanel p) {
        addTo(p, false, true);
    }

    public void addTo(JPanel p, boolean dual, boolean isLimitable) {
        panelToExtend = p;
        isIncluded = true;
        sizeIncludedButton = new JCheckBox("  ", true);
        sizeIncludedButton.addActionListener(this);
        if(isLimitable){
            limitNrOfSize = new JCheckBox("limits");
            limitNrOfSize.setSelected(false);
            limitNrOfSize.addChangeListener(this);
            limitNrOfSize.addActionListener(this);
        }
        sizeLabel = new JLabel(dual ? "degree " + size : size + "-gons");
        sizeLabel.setLabelFor(sizeIncludedButton);
        if (4 <= size && size <= 10) {
            sizeLabel.setDisplayedMnemonic(KeyEvent.VK_0 + size % 10);
        }
        if(isLimitable){
            minNrOfSizeButton = new SpinButton(min, 0, CGFPanel.MAX_ATOMS);
            minNrOfSizeButton.setVisible(limitNrOfSize.isSelected());
            maxNrOfSizeButton = new SpinButton(max, 0, CGFPanel.MAX_ATOMS);
            maxNrOfSizeButton.setVisible(limitNrOfSize.isSelected());
            DefaultButtonModel minNotEqMax = new DefaultButtonModel();
            minNotEqMax.setSelected(false);
            minNrOfSizeButton.addChangeListener(this);
            maxNrOfSizeButton.addChangeListener(this);
            MinMaxEqListener.keepConsistentOrEqual(minNrOfSizeButton.getModel(), maxNrOfSizeButton.getModel(), minNotEqMax);
        }
        GridBagConstraints lc = new GridBagConstraints();
        lc.gridx = 0;
        lc.gridy = size;
        lc.anchor = GridBagConstraints.CENTER;
        lc.insets = new Insets(10, 10, 0, 0);
        panelToExtend.add(sizeIncludedButton, lc);
        lc.anchor = GridBagConstraints.EAST;
        lc.insets = new Insets(10, 0, 0, 40);
        lc.gridx = 1;
        panelToExtend.add(sizeLabel, lc);
        if(isLimitable){
            lc.anchor = GridBagConstraints.CENTER;
            lc.insets = new Insets(10, 10, 0, 10);
            lc.gridx = 2;
            panelToExtend.add(limitNrOfSize, lc);
            lc.gridx = 3;
            panelToExtend.add(minNrOfSizeButton, lc);
            lc.gridx = 4;
            panelToExtend.add(maxNrOfSizeButton, lc);
        }
        UItoolbox.pack(panelToExtend);
    }

    public void deactivate() {
        isIncluded = false;
        sizeIncludedButton.setSelected(false);
        sizeIncludedButton.setVisible(false);
        sizeLabel.setVisible(false);
        if(limitNrOfSize!=null)
            limitNrOfSize.setVisible(false);
        if(minNrOfSizeButton!=null)
            minNrOfSizeButton.setVisible(false);
        if(maxNrOfSizeButton!=null)
            maxNrOfSizeButton.setVisible(false);
        UItoolbox.pack(panelToExtend);
    }

    public void reactivate() {
        isIncluded = true;
        sizeIncludedButton.setSelected(true);
        sizeIncludedButton.setVisible(true);
        sizeLabel.setVisible(true);
        if(limitNrOfSize!=null)
            limitNrOfSize.setVisible(true);
        if(minNrOfSizeButton!=null)
            minNrOfSizeButton.setVisible(isLimited());
        if(maxNrOfSizeButton!=null)
            maxNrOfSizeButton.setVisible(isLimited());
        UItoolbox.pack(panelToExtend);
    }

    public boolean isActive() {
        return isIncluded;
    }

    /**
     * Returns the size that is represented by this <tt>SizeOption</tt>. This is
     * either the degree of a vertex or the size of a face.
     *
     * @return the size that is represented by this <tt>SizeOption</tt>.
     */
    public int getSize() {
        return size;
    }

    public void stateChanged(ChangeEvent e) {
        Object source = e.getSource();
        if (source == (Object) limitNrOfSize) {
            minNrOfSizeButton.setVisible(isLimited());
            maxNrOfSizeButton.setVisible(isLimited());
            UItoolbox.pack(panelToExtend);
        } else if (source == (Object) minNrOfSizeButton) {
            min = minNrOfSizeButton.getValue();
        } else if (source == (Object) maxNrOfSizeButton) {
            max = maxNrOfSizeButton.getValue();
        }
    }

    public void actionPerformed(ActionEvent e) {
        Object source = e.getSource();
        if (source == (Object) limitNrOfSize) {
            if (limitNrOfSize.isSelected()) {
                minNrOfSizeButton.requestFocus();
            }
        } else if (source == (Object) sizeIncludedButton) {
            optionsMap.actionPerformed(new ActionEvent(new MutableInteger(size), sizeIncludedButton.isSelected() ? 1 : 0, null));
        }
    }

    /**
     * Returns whether there are bounds imposed for the number of faces/vertices
     * this option represents.
     *
     * @return <tt>true</tt> if there are bounds for the number of faces/vertices,
     *         <tt>false</tt> otherwise.
     */
    public boolean isLimited(){
        return limitNrOfSize.isSelected();
    }

    /**
     * Tries to move the focus to the <tt>JCheckBox</tt> that disables and enables
     * this vertex/face type.
     */
    public void focusToLimitControl(){
        if(limitNrOfSize!=null)
            limitNrOfSize.requestFocus();
    }

    /**
     * Returns the upper bound set for these kind of faces/vertices.
     *
     * @return the upper bound set for these kind of faces/vertices
     */
    public int getMax() {
        return max;
    }

    /**
     * Returns the lower bound set for these kind of faces/vertices.
     *
     * @return the lower bound set for these kind of faces/vertices
     */
    public int getMin() {
        return min;
    }
}


package cage.generator;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
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
import lisken.uitoolbox.MinMaxRestrictor;
import lisken.uitoolbox.SpinButton;
import lisken.uitoolbox.UItoolbox;

/**
 * This class represents the single configuration for one face type or vertex type
 * and is used in {@link SizeOptionsMap}.
 */
public class SizeOption implements ChangeListener, ActionListener {

    private boolean isIncluded;
    private final int size;
    private int min;
    private int max;
    private JPanel panelToExtend;
    private JPanel sizePanel;
    private JCheckBox sizeIncludedButton;
    private JLabel sizeLabel;
    private JCheckBox limitNrOfSize;
    private SpinButton minNrOfSizeButton;
    private SpinButton maxNrOfSizeButton;
    private final SizeOptionsMap optionsMap;

    public SizeOption(int size, SizeOptionsMap m) {
        this(size, m, CGFPanel.MAX_ATOMS);
        //TODO: remove this dependency on CGFPanel since this now also used in other generators
    }
    
    public SizeOption(int size, SizeOptionsMap m, int max) {
        this.size = size;
        optionsMap = m;
        isIncluded = false;
        min = 0;
        this.max = max;
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
            limitNrOfSize.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    if (limitNrOfSize.isSelected()) {
                        minNrOfSizeButton.requestFocus();
                    }
                }
            });
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
            MinMaxRestrictor.keepConsistentOrEqual(minNrOfSizeButton.getModel(), maxNrOfSizeButton.getModel(), minNotEqMax);
        }
        sizePanel = new JPanel(new GridBagLayout());
        GridBagConstraints lc = new GridBagConstraints();
        lc.gridx = 0;
        lc.gridy = 0;
        lc.anchor = GridBagConstraints.CENTER;
        lc.insets = new Insets(10, 10, 0, 0);
        sizePanel.add(sizeIncludedButton, lc);
        lc.anchor = GridBagConstraints.EAST;
        lc.insets = new Insets(10, 0, 0, 40);
        lc.gridx = 1;
        sizePanel.add(sizeLabel, lc);
        if(isLimitable){
            lc.anchor = GridBagConstraints.CENTER;
            lc.insets = new Insets(10, 10, 0, 10);
            lc.gridx = 2;
            sizePanel.add(limitNrOfSize, lc);
            lc.gridx = 3;
            sizePanel.add(minNrOfSizeButton, lc);
            lc.gridx = 4;
            sizePanel.add(maxNrOfSizeButton, lc);
        }
        
        lc.gridx = 0;
        lc.gridy = size;
        lc.anchor = GridBagConstraints.LINE_START;
        lc.insets = new Insets(10, 10, 0, 0);
        panelToExtend.add(sizePanel, lc);
        UItoolbox.pack(panelToExtend);
    }

    public void addSingleSizeTo(JPanel p, boolean dual) {
        panelToExtend = p;
        isIncluded = true;
        sizeIncludedButton = new JCheckBox("  ", true);
        sizeIncludedButton.addActionListener(this);
        sizeLabel = new JLabel(dual ? "degree " + size : size + "-gons");
        sizeLabel.setLabelFor(sizeIncludedButton);
        if (4 <= size && size <= 10) {
            sizeLabel.setDisplayedMnemonic(KeyEvent.VK_0 + size % 10);
        }
        
        final SpinButton sizeButton = new SpinButton(1, 1, max);
        sizeButton.addChangeListener(new ChangeListener() {
            @Override
            public void stateChanged(ChangeEvent e) {
                min = max = sizeButton.getValue();
            }
        });
        min = max = 1; //set current value

        sizePanel = new JPanel(new GridBagLayout());
        GridBagConstraints lc = new GridBagConstraints();
        lc.gridx = 0;
        lc.gridy = size;
        lc.anchor = GridBagConstraints.CENTER;
        lc.insets = new Insets(10, 10, 0, 0);
        sizePanel.add(sizeIncludedButton, lc);
        lc.anchor = GridBagConstraints.EAST;
        lc.insets = new Insets(10, 0, 0, 40);
        lc.gridx = 1;
        sizePanel.add(sizeLabel, lc);
        lc.anchor = GridBagConstraints.CENTER;
        lc.insets = new Insets(10, 10, 0, 10);
        lc.gridx = 2;
        sizePanel.add(sizeButton, lc);
        
        lc.gridx = 0;
        lc.gridy = size;
        lc.anchor = GridBagConstraints.LINE_START;
        lc.insets = new Insets(10, 10, 0, 0);
        panelToExtend.add(sizePanel, lc);
        UItoolbox.pack(panelToExtend);
    }

    public void deactivate() {
        isIncluded = false;
        sizeIncludedButton.setSelected(false);
        sizePanel.setVisible(false);
        UItoolbox.pack(panelToExtend);
    }

    public void reactivate() {
        isIncluded = true;
        sizeIncludedButton.setSelected(true);
        sizePanel.setVisible(true);
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

    @Override
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

    @Override
    public void actionPerformed(ActionEvent e) {
        if (e.getSource() == (Object) sizeIncludedButton) {
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


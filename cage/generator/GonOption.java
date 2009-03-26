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
import lisken.uitoolbox.MinMaxEqListener;
import lisken.uitoolbox.SpinButton;
import lisken.uitoolbox.UItoolbox;

class GonOption implements ChangeListener, ActionListener {

    boolean isIncluded;
    int faces;
    boolean isLimited;
    int min;
    int max;
    JPanel panelToExtend;
    JCheckBox gonIncludedButton;
    JLabel gonLabel;
    JCheckBox limitGons;
    SpinButton minGonsButton;
    SpinButton maxGonsButton;
    GonOptionsMap optionsMap;

    public GonOption(int f, GonOptionsMap m) {
        faces = f;
        optionsMap = m;
        isIncluded = false;
        isLimited = false;
        min = 0;
        max = CGFPanel.MAX_ATOMS;
    }

    public void addTo(JPanel p) {
        addTo(p, false, true);
    }

    public void addTo(JPanel p, boolean dual, boolean isLimitable) {
        panelToExtend = p;
        isIncluded = true;
        gonIncludedButton = new JCheckBox("  ", true);
        gonIncludedButton.addActionListener(this);
        if(isLimitable){
            limitGons = new JCheckBox("limits");
            limitGons.setSelected(isLimited);
            limitGons.addChangeListener(this);
            limitGons.addActionListener(this);
        }
        gonLabel = new JLabel(dual ? "degree " + faces : faces + "-gons");
        gonLabel.setLabelFor(gonIncludedButton);
        if (4 <= faces && faces <= 10) {
            gonLabel.setDisplayedMnemonic(KeyEvent.VK_0 + faces % 10);
        }
        if(isLimitable){
            minGonsButton = new SpinButton(min, 0, CGFPanel.MAX_ATOMS);
            minGonsButton.setVisible(limitGons.isSelected());
            maxGonsButton = new SpinButton(max, 0, CGFPanel.MAX_ATOMS);
            maxGonsButton.setVisible(limitGons.isSelected());
            DefaultButtonModel minNotEqMax = new DefaultButtonModel();
            minNotEqMax.setSelected(false);
            minGonsButton.addChangeListener(this);
            maxGonsButton.addChangeListener(this);
            new MinMaxEqListener(minGonsButton.getModel(), maxGonsButton.getModel(), minNotEqMax, false);
        }
        GridBagConstraints lc = new GridBagConstraints();
        GridBagLayout l = (GridBagLayout) panelToExtend.getLayout();
        lc.gridx = 0;
        lc.gridy = faces;
        lc.anchor = lc.CENTER;
        lc.insets = new Insets(10, 10, 0, 0);
        panelToExtend.add(gonIncludedButton, lc);
        lc.anchor = lc.EAST;
        lc.insets = new Insets(10, 0, 0, 40);
        lc.gridx = 1;
        panelToExtend.add(gonLabel, lc);
        if(isLimitable){
            lc.anchor = lc.CENTER;
            lc.insets = new Insets(10, 10, 0, 10);
            lc.gridx = 2;
            panelToExtend.add(limitGons, lc);
            lc.gridx = 3;
            panelToExtend.add(minGonsButton, lc);
            lc.gridx = 4;
            panelToExtend.add(maxGonsButton, lc);
        }
        UItoolbox.pack(panelToExtend);
    }

    public void deactivate() {
        isIncluded = false;
        gonIncludedButton.setSelected(false);
        gonIncludedButton.setVisible(false);
        gonLabel.setVisible(false);
        if(limitGons!=null)
            limitGons.setVisible(false);
        if(minGonsButton!=null)
            minGonsButton.setVisible(false);
        if(maxGonsButton!=null)
            maxGonsButton.setVisible(false);
        UItoolbox.pack(panelToExtend);
    }

    public void reactivate() {
        isIncluded = true;
        gonIncludedButton.setSelected(true);
        gonIncludedButton.setVisible(true);
        gonLabel.setVisible(true);
        if(limitGons!=null)
            limitGons.setVisible(true);
        if(minGonsButton!=null)
            minGonsButton.setVisible(isLimited);
        if(maxGonsButton!=null)
            maxGonsButton.setVisible(isLimited);
        UItoolbox.pack(panelToExtend);
    }

    public boolean isActive() {
        return isIncluded;
    }

    public int getSize() {
        return faces;
    }

    public void stateChanged(ChangeEvent e) {
        Object source = e.getSource();
        if (source == (Object) limitGons) {
            isLimited = limitGons.isSelected();
            minGonsButton.setVisible(isLimited);
            maxGonsButton.setVisible(isLimited);
            UItoolbox.pack(panelToExtend);
        } else if (source == (Object) minGonsButton) {
            min = minGonsButton.getValue();
        } else if (source == (Object) maxGonsButton) {
            max = maxGonsButton.getValue();
        }
    }

    public void actionPerformed(ActionEvent e) {
        Object source = e.getSource();
        if (source == (Object) limitGons) {
            if (limitGons.isSelected()) {
                minGonsButton.requestFocus();
            }
        } else if (source == (Object) gonIncludedButton) {
            optionsMap.actionPerformed(new ActionEvent(new MutableInteger(faces), gonIncludedButton.isSelected() ? 1 : 0, null));
        }
    }
}


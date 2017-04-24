package cage.generator;

import cage.CaGe;
import cage.ElementRule;
import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.SingleElementRule;
import cage.StaticGeneratorInfo;

import java.awt.Dimension;
import javax.swing.JFrame;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import javax.swing.BorderFactory;
import javax.swing.JLabel;
import java.awt.GridBagConstraints;
import java.awt.Dimension;
import java.awt.Insets;
import javax.swing.ButtonGroup;
import javax.swing.JRadioButton;
import javax.swing.JPanel;


import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.ArrayList;
import java.util.List;
import javax.swing.AbstractButton;
import javax.swing.Box;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JToggleButton;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.MinMaxRestrictor;
import lisken.uitoolbox.UItoolbox;

public class NanoJoinsPanel extends GeneratorPanel {

    private JPanel parameterPanel;
    private ButtonGroup nrofCapsGroup;
    private EnhancedSlider[][] parameters;

    private EnhancedSlider[] faceSliders;

    private int nrofCaps;

    public NanoJoinsPanel() {
        setLayout(new GridBagLayout());

        JLabel nrofCapsLabel = new JLabel("Number of caps:");
        add(nrofCapsLabel,
                new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.WEST,
                new Insets(0, 0, 20, 10), 0, 0));

        JPanel nrofCapsPanel = new JPanel();
        JRadioButton[] nrofCapsButtons = new JRadioButton[3];
        nrofCapsGroup= new ButtonGroup();
        for (int i=2; i <= 4; i++) {
            JRadioButton button = new JRadioButton(Integer.toString(i));
            nrofCapsButtons[i-2] = button;
            nrofCapsGroup.add(button);
            nrofCapsPanel.add(button);
            button.addActionListener(new nrOfCapsButtonListener(i));
        }
        nrofCapsButtons[0].setSelected(true);
        add(nrofCapsPanel,
            new GridBagConstraints(1, 0, 2, 1, 1.0, 1.0,
            GridBagConstraints.WEST, GridBagConstraints.WEST,
            new Insets(0, 0, 20, 10), 0, 0));
        
        JLabel parameterLabel = new JLabel("parameters:");
        add(parameterLabel,
            new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.WEST,
                new Insets(0, 0, 20, 10), 0, 0));
        parameterPanel = new JPanel();
        add(parameterPanel, 
                new GridBagConstraints(1, 1, 2, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.WEST,
                new Insets(0, 0, 20, 10), 0, 0));

        JLabel[] faceLabels = new JLabel[3];
        faceLabels[0] = new JLabel("number of pentagons:");
        faceLabels[1] = new JLabel("number of hexagons:");
        faceLabels[2] = new JLabel("number of heptagons:");

        faceSliders = new EnhancedSlider[3];
        faceSliders[0] = getEnhancedslider(0, 5, 0, 20);
        faceSliders[1] = getEnhancedslider(0, 20, 0, 5);
        faceSliders[2] = getEnhancedslider(0, 5, 0, 20);

        for (int i = 0; i < 3; i++) {
            add(faceLabels[i],
                new GridBagConstraints(i, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.NONE,
                new Insets(0, 10, 5, 10), 0, 0));
            add(faceSliders[i],
                new GridBagConstraints(i, 3, 1, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.NONE,
                new Insets(0, 10, 5, 10), 0, 0));
        }


        setParameterAmount(2);

        faceSliders[0].addChangeListener(new ChangeListener() {

            @Override
            public void stateChanged(ChangeEvent e) {
                EnhancedSlider slider = (EnhancedSlider) e.getSource();
                faceSliders[2].setValue(6*(nrofCaps-2) + slider.getValue());
            }
        });

        faceSliders[2].addChangeListener(new ChangeListener() {

            @Override
            public void stateChanged(ChangeEvent e) {
                EnhancedSlider slider = (EnhancedSlider) e.getSource();
                faceSliders[0].setValue(slider.getValue() - 6 * (nrofCaps-2));
            }
        });

    }

    private class nrOfCapsButtonListener implements ActionListener {


        private int amount;

        public nrOfCapsButtonListener(int amount) {
            this.amount = amount;
        }

        @Override
        public void actionPerformed(ActionEvent e) {
            setParameterAmount(amount);
        }
    }

    public void setParameterAmount(int amount) {
        nrofCaps = amount;
        parameterPanel.removeAll();
        parameterPanel.setLayout(new GridLayout(amount, 2));
        parameters = new EnhancedSlider[amount][2];
        for (int i = 0; i < amount; i++) {
            JLabel lLabel = new JLabel("l");
            lLabel.setHorizontalAlignment(JLabel.RIGHT);
            JLabel mLabel = new JLabel("m");
            mLabel.setHorizontalAlignment(JLabel.RIGHT);
            EnhancedSlider lslider = getEnhancedslider(2, 30, 5, 5);
            EnhancedSlider mslider = getEnhancedslider(0, 30, 0, 5);

            JPanel lPanel = new JPanel();
            lPanel.add(lLabel);
            lPanel.add(lslider);

            JPanel mPanel = new JPanel();
            mPanel.add(mLabel);
            mPanel.add(mslider);

            parameterPanel.add(lPanel);
            parameterPanel.add(mPanel);
        }
        parameterPanel.repaint();
        parameterPanel.revalidate();

        faceSliders[0].setValue(0);
        faceSliders[2].setMinimum((amount -2) * 6);
        faceSliders[2].setMaximum(faceSliders[2].getMinimum() + 5);
        faceSliders[2].setValue(faceSliders[2].getMinimum());

        repaint();
        revalidate();
    }

    public EnhancedSlider getEnhancedslider(int min, int max, int value, int sizeFactor) {
        EnhancedSlider slider = new EnhancedSlider();
        slider.setMinimum(min);
        slider.setMaximum(max);
        slider.setValue(value);
        slider.setMinorTickSpacing(1);
        slider.setMajorTickSpacing(slider.getMaximum() - slider.getMinimum());
        slider.setPaintTicks(true);
        slider.setPaintLabels(true);
        slider.setSnapWhileDragging(1);
        slider.setClickScrollByBlock(false);
        slider.setSizeFactor(sizeFactor);

        return slider;
    }

    @Override
    public GeneratorInfo getGeneratorInfo() {
        System.out.println("Not implemented yet");
        return null;
    }

    @Override
    public void showing() {
    }

    public static void main(String[] args) {
        final NanoJoinsPanel p = new NanoJoinsPanel();
        p.setBorder(BorderFactory.createTitledBorder(
                BorderFactory.createCompoundBorder(
                BorderFactory.createCompoundBorder(
                BorderFactory.createEmptyBorder(10, 10, 10, 10),
                BorderFactory.createEtchedBorder()),
                BorderFactory.createEmptyBorder(20, 20, 20, 20)),
                " Nanojoin Options "));
        final JFrame f = new JFrame("Nanojoin Dialog");
        f.addWindowListener(new WindowAdapter() {

            @Override
            public void windowClosing(WindowEvent e) {
               System.exit(0);
            }
        });
        UItoolbox.addExitOnEscape(f);
        f.setContentPane(p);
        f.pack();
        Dimension d = f.getSize();
        d.height += 2;
        f.setSize(d);
        f.setVisible(true);
    }
}

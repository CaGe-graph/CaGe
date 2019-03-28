package cage.generator;

import java.nio.file.Path;
import java.nio.file.Paths;

import cage.CaGe;
import cage.ElementRule;
import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.ValencyElementRule;
import cage.StaticGeneratorInfo;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.ButtonGroup;
import javax.swing.JLabel;
import javax.swing.JCheckBox;
import javax.swing.JRadioButton;
import javax.swing.BorderFactory;
import javax.swing.DefaultBoundedRangeModel;

import java.awt.Dimension;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ComponentListener;
import java.awt.event.ComponentEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.UItoolbox;
import lisken.uitoolbox.SpinButton;

public class NanoJoinsPanel extends GeneratorPanel {

    private static final int MAXCAPS = 4;

    private final JPanel parameterPanel;

    private int nrofCaps;
    private EnhancedSlider[][] parameters;
    private final EnhancedSlider[] faceSliders;
    private final JCheckBox hexagonLayersBox;
    private final SpinButton hexagonLayers = new SpinButton(new DefaultBoundedRangeModel(1, 0, 1, Integer.MAX_VALUE));
    private final JCheckBox exactFacesBox;

    public NanoJoinsPanel() {
        setLayout(new GridBagLayout());

        /* How many caps */
        JLabel nrofCapsLabel = new JLabel("Number of caps:");
        add(nrofCapsLabel,
                new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.WEST,
                new Insets(0, 0, 10, 10), 0, 0));

        JPanel nrofCapsPanel = new JPanel();
        JRadioButton[] nrofCapsButtons = new JRadioButton[3];
        ButtonGroup nrofCapsGroup= new ButtonGroup();
        for (int i=2; i <= MAXCAPS; i++) {
            JRadioButton button = new JRadioButton(Integer.toString(i));
            nrofCapsButtons[i-2] = button;
            nrofCapsGroup.add(button);
            nrofCapsPanel.add(button);
            button.addActionListener(new nrOfCapsButtonListener(i));
        }
        nrofCapsButtons[0].setSelected(true);
        add(nrofCapsPanel,
            new GridBagConstraints(1, 0, 3, 1, 1.0, 1.0,
            GridBagConstraints.WEST, GridBagConstraints.WEST,
            new Insets(0, 0, 10, 10), 0, 0));


        
        /* The cap parameters */
        parameterPanel = new JPanel();
        add(parameterPanel, 
                new GridBagConstraints(0, 1, 5, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.WEST,
                new Insets(0, 0, 10, 10), 0, 0));
        //initialisation will happen later

        /* The number of faces */
        JLabel[] faceLabels = new JLabel[3];
        faceLabels[0] = new JLabel("max number of pentagons:");
        faceLabels[1] = new JLabel("max number of hexagons:");
        faceLabels[2] = new JLabel("number of heptagons:");

        faceSliders = new EnhancedSlider[3];
        faceSliders[0] = getEnhancedslider(0, 5, 0, 20);
        faceSliders[1] = getEnhancedslider(0, 20, 0, 5);
        faceSliders[2] = getEnhancedslider(0, 5, 0, 20);

        for (int i = 0; i < 2; i++) {
            add(faceLabels[i],
                new GridBagConstraints(2*i, 2, 2, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 10), 0, 0));
            add(faceSliders[i],
                new GridBagConstraints(2*i, 3, 2, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 10), 0, 0));
        }

        /* Add extra rings */
        hexagonLayersBox = new JCheckBox("Add a number of hexagon layers");
        hexagonLayers.setEnabled(hexagonLayersBox.isSelected());
        hexagonLayersBox.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                hexagonLayers.setEnabled(hexagonLayersBox.isSelected());
            }
        });

        add(hexagonLayersBox,
            new GridBagConstraints(0, 4, 2, 1, 1.0, 1.0,
            GridBagConstraints.WEST, GridBagConstraints.NONE,
            new Insets(20, 10, 0, 0), 0, 0));

        add(hexagonLayers,
            new GridBagConstraints(1, 4, 3, 1, 1.0, 1.0,
            GridBagConstraints.WEST, GridBagConstraints.NONE,
            new Insets(20, 10, 0, 0), 0, 0));

        /* Only show joins with faces exactly equal to parameters */
        exactFacesBox = new JCheckBox("Only generate joins with this exact number of pentagons");
        add(exactFacesBox,
            new GridBagConstraints(0, 5, 4, 1, 1.0, 1.0,
            GridBagConstraints.WEST, GridBagConstraints.NONE,
            new Insets(20, 10, 0, 0), 0, 0));

        /* Initialise cap parameter panel */
        setParameterAmount(2);
        parameterPanel.setPreferredSize(new Dimension(500, 150));

        /* Adjust pentagons/heptagons when the other one changes*/
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
        int oldnrofcaps = nrofCaps;
        nrofCaps = amount;

        EnhancedSlider[][] newparameters = new EnhancedSlider[amount][2];
        int i = 0;
        while (i < amount && i < oldnrofcaps) {
            newparameters[i][0] = parameters[i][0];
            newparameters[i][1] = parameters[i][1];
            i++;
        }

        while (i < amount) {
            EnhancedSlider lslider = getEnhancedslider(1, 15, 5, 5);
            EnhancedSlider mslider = getEnhancedslider(0, 15, 0, 5);
            newparameters[i][0] = lslider;
            newparameters[i][1] = mslider;
            i++;
        }

        parameters = newparameters;

        parameterPanel.removeAll();
        parameterPanel.setLayout(new GridLayout(3, amount));

        for (i = 0; i < nrofCaps; i++) {
            JLabel parlabel = new JLabel("parameters " + (i + 1));
            parlabel.setHorizontalAlignment(JLabel.CENTER);
            parameterPanel.add(parlabel);
        }

        for (i = 0; i < nrofCaps; i++) {
            JLabel lLabel = new JLabel("l");
            lLabel.setHorizontalAlignment(JLabel.RIGHT);

            JPanel lPanel = new JPanel();
            lPanel.add(lLabel);
            lPanel.add(parameters[i][0]);

            parameterPanel.add(lPanel);
        }

        for (i = 0; i < nrofCaps; i++) {
            JLabel mLabel = new JLabel("m");
            mLabel.setHorizontalAlignment(JLabel.RIGHT);

            JPanel mPanel = new JPanel();
            mPanel.add(mLabel);
            mPanel.add(parameters[i][1]);

            parameterPanel.add(mPanel);
        }

        parameterPanel.repaint();
        parameterPanel.revalidate();

        int pentagons = faceSliders[0].getValue();
        faceSliders[2].setMinimum((amount -2) * 6);
        faceSliders[2].setMaximum(faceSliders[2].getMinimum() + 5);
        faceSliders[2].setValue(faceSliders[2].getMinimum() + pentagons);

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

    private String getEmbedderDirectory() {
       return System.getProperty("CaGe.InstallDir") + "/Generators/";
    }

    @Override
    public GeneratorInfo getGeneratorInfo() {
        int pentagons = faceSliders[0].getValue();
        int hexagons = faceSliders[1].getValue();
        int heptagons = faceSliders[2].getValue();
        int extraRings = hexagonLayers.isEnabled() ? hexagonLayers.getValue() : 0;
        String parameterString = "";

        for (int i=0; i < nrofCaps; i++) {
            parameterString += parameters[i][0].getValue() + " " + Integer.toString(parameters[i][1].getValue()) + " ";
        }        
        String ioption = exactFacesBox.isSelected() ? "-e" : "";

        return new StaticGeneratorInfo(
                Systoolbox.parseCmdLine("join -r " + extraRings + " " + ioption + " -pent " + pentagons + " -hex " + hexagons + " -hept " + heptagons + " " + parameterString),
                EmbedFactory.createEmbedder(new String[][]{{"embed"}}, new String[][]{{"nanojoin_embed", getEmbedderDirectory()}}),
                "test",
                6, true, new ValencyElementRule("H O C Si N S I"), 0);
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

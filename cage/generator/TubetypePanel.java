package cage.generator;

import cage.CaGe;
import cage.ElementRule;
import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.SingleElementRule;
import cage.StaticGeneratorInfo;

import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.Vector;
import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JToggleButton;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;
import lisken.uitoolbox.MinMaxEqListener;
import lisken.uitoolbox.UItoolbox;

public class TubetypePanel extends GeneratorPanel {

    private static final int MAX_TUBELENGTH = 30;
    private static final int MAX_OFFSET = 30;

    private EnhancedSlider tubelengthSlider = new EnhancedSlider();
    private EnhancedSlider offset1Control = new EnhancedSlider();
    private EnhancedSlider offset2Control = new EnhancedSlider();
    private JCheckBox ipr = new JCheckBox();
    private AbstractButton defaultTubelengthButton = new JToggleButton();
    private boolean adjusting = false;

    public TubetypePanel() {
        setLayout(new GridBagLayout());
        tubelengthSlider.setMinimum(0);
        tubelengthSlider.setMaximum(MAX_TUBELENGTH);
        tubelengthSlider.setMinorTickSpacing(1);
        tubelengthSlider.setMajorTickSpacing(tubelengthSlider.getMaximum() - tubelengthSlider.getMinimum());
        tubelengthSlider.setPaintTicks(true);
        tubelengthSlider.setPaintLabels(true);
        tubelengthSlider.setSnapWhileDragging(1);
        tubelengthSlider.setClickScrollByBlock(false);
        tubelengthSlider.setSizeFactor(10);
        tubelengthSlider.addChangeListener(new ChangeListener() {

            public void stateChanged(ChangeEvent e) {
                if (!adjusting) {
                    defaultTubelengthButton.setSelected(false);
                }
            }
        });
        defaultTubelengthButton.setText("default");
        defaultTubelengthButton.setSelected(false);
        defaultTubelengthButton.setMnemonic(KeyEvent.VK_D);
        defaultTubelengthButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                if (defaultTubelengthButton.isSelected()) {
                    adjustTubelength();
                }
            }
        });
        offset1Control.setMinimum(2);
        offset1Control.setMaximum(MAX_OFFSET);
        offset1Control.setValue(5);
        offset1Control.setMinorTickSpacing(1);
        offset1Control.setMajorTickSpacing(offset1Control.getMaximum() - offset1Control.getMinimum());
        offset1Control.setPaintTicks(true);
        offset1Control.setPaintLabels(true);
        offset1Control.setSnapWhileDragging(1);
        offset1Control.setClickScrollByBlock(false);
        offset1Control.setSizeFactor(4);
        offset2Control.setMinimum(0);
        offset2Control.setMaximum(MAX_OFFSET);
        offset2Control.setValue(0);
        offset2Control.setMinorTickSpacing(1);
        offset2Control.setMajorTickSpacing(offset2Control.getMaximum() - offset2Control.getMinimum());
        offset2Control.setPaintTicks(true);
        offset2Control.setPaintLabels(true);
        offset2Control.setSnapWhileDragging(1);
        offset2Control.setClickScrollByBlock(false);
        offset2Control.setSizeFactor(4);
        adjustTubelength();
        ChangeListener offsetListener = new ChangeListener() {

            public void stateChanged(ChangeEvent e) {
                if (defaultTubelengthButton.isSelected()) {
                    adjustTubelength();
                }
            }
        };
        offset1Control.addChangeListener(offsetListener);
        offset2Control.addChangeListener(offsetListener);
        MinMaxEqListener.keepConsistent(offset2Control.getModel(), offset1Control.getModel());
        JLabel tubelengthLabel = new JLabel("Tube length:");
        tubelengthLabel.setDisplayedMnemonic(KeyEvent.VK_T);
        tubelengthLabel.setLabelFor(tubelengthSlider.slider());
        ipr.setText("isolated pentagons");
        ipr.setMnemonic(KeyEvent.VK_I);
        add(tubelengthLabel,
                new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(0, 0, 20, 10), 0, 0));
        add(tubelengthSlider,
                new GridBagConstraints(1, 0, 3, 1, 1.0, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                new Insets(0, 0, 20, 5), 0, 0));
        add(defaultTubelengthButton,
                new GridBagConstraints(4, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 10, 20, 0), 0, 0));
        add(new JLabel("Boundary parameters:"),
                new GridBagConstraints(0, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(5, 0, 5, 10), 0, 0));
        JLabel offset1Label = new JLabel("l");
        offset1Label.setDisplayedMnemonic(KeyEvent.VK_L);
        offset1Label.setLabelFor(offset1Control.slider());
        add(offset1Label,
                new GridBagConstraints(1, 2, 1, 1, 0.001, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.NONE,
                new Insets(5, 5, 5, 0), 0, 0));
        add(offset1Control,
                new GridBagConstraints(2, 2, 1, 1, 0.1, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(5, 5, 5, 0), 0, 0));
        JLabel offset2Label = new JLabel("m");
        offset2Label.setDisplayedMnemonic(KeyEvent.VK_M);
        offset2Label.setLabelFor(offset2Control.slider());
        add(offset2Label,
                new GridBagConstraints(1, 3, 1, 1, 0.001, 1.0,
                GridBagConstraints.CENTER, GridBagConstraints.NONE,
                new Insets(5, 5, 5, 0), 0, 0));
        add(offset2Control,
                new GridBagConstraints(2, 3, 1, 1, 0.1, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(5, 5, 5, 0), 0, 0));
        add(Box.createHorizontalGlue(),
                new GridBagConstraints(3, 2, 1, 1, 1.0, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(0, 0, 0, 0), 0, 0));
        add(ipr,
                new GridBagConstraints(1, 7, 3, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(30, 5, 0, 0), 0, 0));
    }

    void adjustTubelength() {
        if (!adjusting) {
            adjusting = true;
            int l = Math.max(offset1Control.getValue(), offset2Control.getValue()) - 1;
            tubelengthSlider.setValue(l);
            adjusting = false;
        }
    }

    public GeneratorInfo getGeneratorInfo() {
        Vector command = new Vector();
        String filename;

        command.addElement("tube");
        filename = "tubetypes";

        command.addElement(Integer.toString(offset1Control.getValue()));
        command.addElement(Integer.toString(offset2Control.getValue()));
        filename += "_l" + offset1Control.getValue();
        filename += "_m" + offset2Control.getValue();
        command.addElement("tube");
        command.addElement(Integer.toString(tubelengthSlider.getValue()));
        filename += "_t" + tubelengthSlider.getValue();

        if (ipr.isSelected()) {
            command.addElement("ipr");
            filename += "_ip";
        }

        String[][] generator = new String[1][command.size()];
        command.copyInto(generator[0]);

        String[][] embed2D = {{"embed"}};
        String[][] embed3D = {{"java", "-cp", "CaGe.jar", "cage.embedder.NanotubeEmbedder"}};

        ElementRule rule = new SingleElementRule("C");

        return new StaticGeneratorInfo(
                generator,
                EmbedFactory.createEmbedder(false, embed2D, embed3D),
                filename, 6, rule);
    }

    public void showing() {
    }

    public static void main(String[] args) {
        final TubetypePanel p = new TubetypePanel();
        p.setBorder(BorderFactory.createTitledBorder(
                BorderFactory.createCompoundBorder(
                BorderFactory.createCompoundBorder(
                BorderFactory.createEmptyBorder(10, 10, 10, 10),
                BorderFactory.createEtchedBorder()),
                BorderFactory.createEmptyBorder(20, 20, 20, 20)),
                " Tubetype Options "));
        final JFrame f = new JFrame("Tubetype Dialog");
        f.addWindowListener(new WindowAdapter() {

            public void windowClosing(WindowEvent e) {
                f.setVisible(false);
                GeneratorInfo info = p.getGeneratorInfo();
                JOptionPane.showInputDialog(null, "Command Line", "Tubetype results",
                        JOptionPane.PLAIN_MESSAGE, null, null,
                        "Command: " + Systoolbox.join(info.getGenerator()[0], " ") + "\nOutput: " + info.getFilename());
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

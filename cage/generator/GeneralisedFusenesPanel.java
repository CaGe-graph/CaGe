package cage.generator;

import cage.ElementRule;
import cage.EmbedFactory;
import cage.GeneratorInfo;
import cage.GeneratorPanel;
import cage.StaticGeneratorInfo;
import cage.ValencyElementRule;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.util.ArrayList;
import java.util.Dictionary;
import java.util.Iterator;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import javax.swing.JToggleButton;
import javax.swing.SwingConstants;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import lisken.systoolbox.Systoolbox;
import lisken.uitoolbox.EnhancedSlider;

/**
 * Panel to configure the generator for generalised fusenes (ngons).
 */
public class GeneralisedFusenesPanel extends GeneratorPanel {

    private static final int MIN_POLYGON_FACES = 3;
    private static final int MAX_POLYGON_FACES = 40;
    
    private JCheckBox kekulean = new JCheckBox();
    private JCheckBox regularEmbedded = new JCheckBox();
    private SizeOptionsMap sizeOptionsMap;

    public GeneralisedFusenesPanel() {
        try {
            initGUI();
        } catch (Exception ex) {
            Logger.getLogger(GeneralisedFusenesPanel.class.getName()).log(
                    Level.SEVERE,
                    "GeneralisedFusenesPanel could not be initialised.",
                    ex);
        }
    }

    private void initGUI() throws Exception {
        setLayout(new GridBagLayout());
        JPanel extrasPanel = new JPanel(new GridBagLayout());
        JPanel faceOptionsPanel = new JPanel(new GridBagLayout());
        
        final EnhancedSlider facesSlider = new EnhancedSlider();
        Dictionary<Integer, JLabel> facesLabels = facesSlider.createStandardLabels(5, 5);
        JLabel faceTypeLabel = new JLabel("Face Type");
        faceTypeLabel.setLabelFor(facesSlider.slider());
        faceTypeLabel.setDisplayedMnemonic(KeyEvent.VK_F);
        JToggleButton facesButton = new JToggleButton("include this face type");
        facesButton.setMnemonic(KeyEvent.VK_I);
        facesSlider.setOrientation(SwingConstants.HORIZONTAL);
        facesSlider.setLabelTable(facesLabels);
        facesSlider.setMinorTickSpacing(1);
        facesSlider.setSnapToTicks(true);
        facesSlider.setPaintTicks(true);
        facesSlider.setPaintLabels(true);
        facesSlider.setMaximum(MAX_POLYGON_FACES);
        facesSlider.setMinimum(MIN_POLYGON_FACES);
        facesSlider.setValue(6);
        facesSlider.setMajorTickSpacing(facesSlider.getMaximum() - facesSlider.getMinimum());
        facesSlider.setSnapWhileDragging(1);
        facesSlider.setSizeFactor(12);
        facesSlider.setClickScrollByBlock(false);
        facesSlider.slider().addKeyListener(new KeyAdapter() {

            @Override
            public void keyPressed(KeyEvent e) {
                if (e.getModifiers() != 0) {
                    return;
                }
                int n = e.getKeyCode() - KeyEvent.VK_0;
                if (n < 0 || n > 9) {
                    return;
                }
                n = (n + 7) % 10 + 3;
                facesSlider.setValue(n);
            }
        });
        sizeOptionsMap = new SizeOptionsMap(faceOptionsPanel, facesSlider.slider(), facesSlider.getModel(), facesButton, false, true, true);
        sizeOptionsMap.addChangeListener(new ChangeListener() {
            @Override
            public void stateChanged(ChangeEvent e) {
                verifyData();
            }
        });
        JLabel includedFacesLabel = new JLabel("included face types:");
        
        kekulean.setText("kekulean");
        kekulean.setMnemonic(KeyEvent.VK_K);
        kekulean.setActionCommand("k");
        kekulean.setSelected(false);
        
        regularEmbedded.setText("only graphs which can be embedded in a regular lattice");
        regularEmbedded.setMnemonic(KeyEvent.VK_R);
        regularEmbedded.setActionCommand("r");
        regularEmbedded.setSelected(false);
        
        JPanel facesPanel = new JPanel(new GridBagLayout());
        facesPanel.add(faceTypeLabel, new GridBagConstraints(0, 0, 2, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 5, 5, 0), 0, 0));
        facesPanel.add(facesSlider, new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        facesPanel.add(facesButton, new GridBagConstraints(1, 1, 1, 1, 0.0010, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 30, 0, 5), 0, 5));
        extrasPanel.add(new JLabel(), new GridBagConstraints(0, 0, 1, 1, 0.01, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        extrasPanel.add(kekulean, new GridBagConstraints(3, 0, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        extrasPanel.add(regularEmbedded, new GridBagConstraints(3, 1, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(facesPanel, new GridBagConstraints(0, 2, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 0, 0), 0, 0));
        this.add(includedFacesLabel, new GridBagConstraints(0, 3, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(20, 5, 0, 0), 0, 0));
        this.add(faceOptionsPanel, new GridBagConstraints(0, 4, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 10, 10, 0), 0, 0));
        this.add(new JSeparator(SwingConstants.HORIZONTAL), new GridBagConstraints(0, 5, 1, 1, 1.0, 1.0, GridBagConstraints.CENTER, GridBagConstraints.BOTH, new Insets(10, 0, 10, 0), 0, 0));
        this.add(extrasPanel, new GridBagConstraints(0, 6, 1, 1, 1.0, 1.0, GridBagConstraints.WEST, GridBagConstraints.BOTH, new Insets(0, 0, 10, 0), 0, 0));
    }

    @Override
    public GeneratorInfo getGeneratorInfo() {
        String[][] generator, embed2D, embed3D;
        String filename;
        int maxFacesize = 0;

        List<String> genList = new ArrayList<>(), fileList = new ArrayList<>();
        
        genList.add("ngons");
        genList.add("-f");
        genList.add("-p");
        fileList.add("ngons");
        
        if(kekulean.isSelected()){
            genList.add("-k");
            fileList.add("k");
        }
        
        if(regularEmbedded.isSelected()){
            genList.add("-r");
            fileList.add("r");
        }
        
        for (SizeOption sizeOption : sizeOptionsMap.values()) {
            if (!sizeOption.isActive()) {
                continue;
            }
            genList.add(
                    Integer.toString(sizeOption.getSize()) + ":" + 
                            Integer.toString(sizeOption.getMin()));
            fileList.add(
                    Integer.toString(sizeOption.getSize()) + "-" + 
                            Integer.toString(sizeOption.getMin()));
        }
        
        generator = new String[1][];
        generator[0] = genList.toArray(new String[genList.size()]);
        filename = Systoolbox.join(
                fileList.toArray(new String[fileList.size()]), "_");


        embed2D = new String[][]{{"embed", "-b1,2"}};
        embed3D = new String[][]{{"embed", "-d3", "-f1,1,4"}};

        ElementRule rule = new ValencyElementRule("H C C");

        return new StaticGeneratorInfo(
                generator,
                EmbedFactory.createEmbedder(true, embed2D, embed3D),
                filename, maxFacesize, rule);
    }

    @Override
    public void showing() {
        verifyData();
    }

    /**
     * verifies whether the next button should be enabled or disabled.
     */
    private void verifyData(){
        //is at least one face type added
        int faceTypeCount = 0;

        Iterator it = sizeOptionsMap.values().iterator();
        if(it.hasNext()){
            while (it.hasNext()) {
                SizeOption sizeOption = (SizeOption) it.next();
                if(sizeOption.isActive()){
                    faceTypeCount++;
                }
                
            }
        }

        getNextButton().setEnabled(faceTypeCount>0);
        
        if(faceTypeCount > 1){
            regularEmbedded.setSelected(false);
            regularEmbedded.setEnabled(false);
        } else {
            regularEmbedded.setEnabled(true);
        }
    }
}


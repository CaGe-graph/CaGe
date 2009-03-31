package cage.viewer.jmol;

import cage.CaGe;
import java.awt.Graphics2D;
import java.awt.event.ActionEvent;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.imageio.ImageIO;
import javax.swing.AbstractAction;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;

/**
 *
 * @author nvcleemp
 */
public class JmolExportImageAction extends AbstractAction{

    private JmolPanel panel;

    public JmolExportImageAction(JmolPanel panel) {
        super("Save image");
        this.panel = panel;
    }

    public void actionPerformed(ActionEvent e) {
        BufferedImage im = new BufferedImage(panel.getWidth(), panel.getHeight(), BufferedImage.TYPE_INT_ARGB);
        Graphics2D g = im.createGraphics();
        panel.getViewer().renderScreenImage(g, panel.getSize(), g.getClipBounds());
        try {
            JFileChooser chooser;
            String dir = CaGe.config.getProperty("CaGe.Generators.RunDir");
            if(dir==null)
                chooser = new JFileChooser();
            else
                chooser = new JFileChooser(new File(dir));
            if(chooser.showSaveDialog(null)==JFileChooser.APPROVE_OPTION){
                File file = chooser.getSelectedFile();
                if(!file.exists() || JOptionPane.showConfirmDialog(null, "Overwrite file " + file.toString(), "Confirm", JOptionPane.YES_NO_OPTION) == JOptionPane.YES_OPTION)
                    ImageIO.write(im, "PNG", file);
            }
        } catch (IOException ex) {
            Logger.getLogger(JmolExportImageAction.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

}

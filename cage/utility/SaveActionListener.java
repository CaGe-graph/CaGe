package cage.utility;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import javax.swing.JFileChooser;
import javax.swing.JFrame;

/**
 *
 * @author nvcleemp
 */
public abstract class SaveActionListener implements ActionListener {

    private final JFrame frame;

    private final JFileChooser fileChooser;

    private final String extension;

    public SaveActionListener(JFrame frame, File startDir, String extension) {
        this.frame = frame;
        this.extension = extension;
        fileChooser = new JFileChooser(startDir);
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        if (fileChooser.showSaveDialog(frame) == JFileChooser.APPROVE_OPTION) {
            File f = fileChooser.getSelectedFile();
            if (!f.getAbsolutePath().toLowerCase().endsWith(extension)) {
                f = new File(f.getAbsolutePath() + extension);
            }
            //TODO: maybe ask before overwriting file
            save(f);
        }
    }

    protected abstract void save(File file);
}

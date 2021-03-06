package cage;

import cage.writer.CaGeWriter;
import cage.writer.WriterConfigurationHandler;
import java.awt.BorderLayout;

import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import javax.swing.AbstractAction;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

import lisken.uitoolbox.FlaggedJDialog;
import lisken.uitoolbox.JTextComponentFocusSelector;

public class SaveDialog extends FlaggedJDialog {

    private JPanel content;
    private JTextField saveFilenameField;
    private JLabel infoLabel;
    private JTextField saveInfoField;
    private JCheckBox includeInfoBox;
    private FileFormatBox fileFormatBox = null;
    
    private final ActionListener successListener = new ActionListener() {

        @Override
        public void actionPerformed(ActionEvent e) {
            setSuccess(true);
            handleClosing();
            setVisible(false);
        }
    };

    public SaveDialog(String title) {
        this(null, title, false);
    }

    public SaveDialog(Frame parent, String title) {
        this(parent, title, false);
    }

    public SaveDialog(String title, boolean useInfo) {
        this(null, title, useInfo);
    }

    public SaveDialog(Frame parent, String title, boolean useInfo) {
        this(parent, title, null, useInfo);
    }

    public SaveDialog(String title, String variety) {
        this(null, title, variety);
    }

    public SaveDialog(Frame parent, String title, String variety) {
        this(parent, title, variety, false);
    }

    private SaveDialog(Frame parent, String title, String variety, boolean useInfoParameter) {
        super(parent, title, true);
        content = new JPanel();
        setContentPane(content);
        content.setLayout(new GridBagLayout());
        saveFilenameField = new JTextField(15);
        saveFilenameField.addActionListener(successListener);
        new JTextComponentFocusSelector(saveFilenameField);
        JLabel saveFilenameLabel = new JLabel("save in:");
        saveFilenameLabel.setLabelFor(saveFilenameField);
        saveFilenameLabel.setDisplayedMnemonic(KeyEvent.VK_S);
        Font font = saveFilenameLabel.getFont();
        font = new Font(
                font.getName(),
                font.getStyle() & ~Font.BOLD,
                font.getSize());
        saveFilenameLabel.setFont(font);
        saveInfoField = new JTextField(15);
        saveInfoField.addActionListener(successListener);
        new JTextComponentFocusSelector(saveInfoField);
        infoLabel = new JLabel("info:");
        infoLabel.setFont(font);
        infoLabel.setLabelFor(saveInfoField);
        infoLabel.setDisplayedMnemonic(KeyEvent.VK_O);
        includeInfoBox = new JCheckBox("include info", false);
        includeInfoBox.setFont(font);
        includeInfoBox.setMnemonic(KeyEvent.VK_I);
        content.add(saveFilenameLabel,
                new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 10), 0, 0));
        content.add(saveFilenameField,
                new GridBagConstraints(1, 0, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 0), 0, 0));
        content.add(infoLabel,
                new GridBagConstraints(0, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 10), 0, 0));
        content.add(saveInfoField,
                new GridBagConstraints(1, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(0, 0, 10, 0), 0, 0));
        content.add(includeInfoBox,
                new GridBagConstraints(2, 1, 1, 1, 1.0, 1.0,
                GridBagConstraints.EAST, GridBagConstraints.NONE,
                new Insets(0, 10, 10, 0), 0, 0));
        JButton okButton = new JButton("Ok");
        setDefaultButton(okButton);
        JButton cancelButton = new JButton("Cancel");
        setCancelButton(cancelButton);
        JPanel buttonPanel = new JPanel();
        buttonPanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        buttonPanel.add(okButton);
        buttonPanel.add(Box.createHorizontalStrut(5));
        buttonPanel.add(cancelButton);
        content.add(buttonPanel,
                new GridBagConstraints(1, 2, 3, 1, 1.0, 1.0,
                GridBagConstraints.WEST, GridBagConstraints.NONE,
                new Insets(20, 0, 0, 0), 0, 0));
        content.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        boolean useInfo;
        if (variety == null) {
            useInfo = useInfoParameter;
        } else {
            fileFormatBox = new FileFormatBox(variety, saveFilenameField);
            useInfo = false;
            int n = fileFormatBox.getItemCount();
            int index = fileFormatBox.getSelectedIndex();
            for (int i = 0; i < n; ++i) {
                fileFormatBox.setSelectedIndex(i);
                useInfo |= fileFormatBox.getCaGeWriter().usesInfo();
            }
            fileFormatBox.setSelectedIndex(index);
            JLabel formatLabel = new JLabel("format:");
            formatLabel.setFont(font);
            formatLabel.setLabelFor(fileFormatBox);
            formatLabel.setDisplayedMnemonic(KeyEvent.VK_F);
            content.add(formatLabel,
                    new GridBagConstraints(2, 0, 1, 1, 1.0, 1.0,
                    GridBagConstraints.WEST, GridBagConstraints.NONE,
                    new Insets(0, 10, 10, 0), 0, 0));
            content.add(fileFormatBox,
                    new GridBagConstraints(3, 0, 1, 1, 1.0, 1.0,
                    GridBagConstraints.WEST, GridBagConstraints.NONE,
                    new Insets(0, 5, 10, 0), 0, 0));
        }
        useInfo(useInfo);
        if (fileFormatBox != null) {
            includeInfoBox.setEnabled(false);
            checkFormatForInfo();
        }
    }

    void checkFormatForInfo() {
        if (infoLabel.isVisible()) {
            boolean useInfo = fileFormatBox.getCaGeWriter().usesInfo();
            includeInfoBox.setSelected(useInfo);
        }
    }

    public CaGeWriter getCaGeWriter() {
        return fileFormatBox.getCaGeWriter();
    }

    public void setFilename(String filename) {
        saveFilenameField.setText(filename);
    }

    public void addExtension() {
        fileFormatBox.addExtension();
    }

    public String getFilename() {
        return saveFilenameField.getText();
    }

    public void useInfo(boolean useInfo) {
        infoLabel.setVisible(useInfo);
        saveInfoField.setVisible(useInfo && fileFormatBox == null);
        includeInfoBox.setVisible(useInfo);
        pack();
    }

    public void setInfo(String comment) {
        saveInfoField.setText(comment);
    }

    public String getInfo() {
        return saveInfoField.getText();
    }

    public boolean includeInfo() {
        return includeInfoBox.isSelected();
    }

    @Override
    public void setVisible(boolean visible) {
        if (visible) {
            saveFilenameField.requestFocus();
        }
        super.setVisible(visible);
    }

    @Override
    public void handleClosing() {
        if(fileFormatBox!=null){
            WriterConfigurationHandler handler = fileFormatBox.getConfigurationHandler();
            if(handler!=null){
                JPanel configurationPanel = handler.getConfigurationPanel();
                if(configurationPanel != null){
                    //show dialog containing panel
                    final JDialog configurationDialog = new JDialog(this, "Options", true);
                    configurationDialog.setLayout(new BorderLayout(5, 5));
                    configurationDialog.add(configurationPanel, BorderLayout.NORTH);
                    configurationDialog.add(new JButton(new AbstractAction("OK") {
                        @Override
                        public void actionPerformed(ActionEvent e) {
                            //just close the dialog
                            configurationDialog.setVisible(false);
                        }
                    }), BorderLayout.SOUTH);
                    configurationDialog.pack();
                    configurationDialog.setLocationRelativeTo(this);
                    configurationDialog.setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
                    configurationDialog.setVisible(true);

                    //configure writer
                    handler.configureWriter(fileFormatBox.getCaGeWriter());
                }
            }
        }
    }
}


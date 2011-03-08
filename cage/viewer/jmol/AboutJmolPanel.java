package cage.viewer.jmol;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.text.MessageFormat;
import javax.swing.JPanel;
import javax.swing.JTable;
import javax.swing.Timer;
import javax.swing.table.AbstractTableModel;

import org.jmol.api.JmolViewer;
import org.jmol.viewer.JmolConstants;

/**
 *  Panel that shows a short overview of information about Jmol.
 *
 * @author nvcleemp
 */
public class AboutJmolPanel extends JPanel {

    private static final String[] NAMES = {"Jmol version", "Build date", "Operating system",
    "Java Vendor", "Java version", "Total memory", "Free memory", "Maximum memory", "Processors"};

    private static final boolean[] IS_VARIABLE={false, false, false, false, false, true, true, true, true};

    private JmolViewer viewer;

    public AboutJmolPanel(JmolViewer viewer) {
        super(new BorderLayout());
        this.viewer = viewer;
        add(new JTable(new AboutPanelTableModel()), BorderLayout.CENTER);
    }

    private class AboutPanelTableModel extends AbstractTableModel implements ActionListener {

        public AboutPanelTableModel() {
            new Timer(5000, this).start();
        }

        public int getRowCount() {
            return NAMES.length;
        }

        public int getColumnCount() {
            return 2;
        }

        public Object getValueAt(int rowIndex, int columnIndex) {
            //TODO: when switching to Java 5 or above this should be an enum
            if (columnIndex == 1) {
                if(rowIndex==0){
                    return JmolConstants.version;
                } else if(rowIndex==1){
                    return JmolConstants.date;
                } else if(rowIndex==2){
                    return viewer.getOperatingSystemName();
                } else if(rowIndex==3){
                    return viewer.getJavaVendor();
                } else if(rowIndex==4){
                    return viewer.getJavaVersion();
                } else if(rowIndex==5){
                    return MessageFormat.format("{0} MB",
                        convertToMegabytes(Runtime.getRuntime().totalMemory()));
                } else if(rowIndex==6){
                    return MessageFormat.format("{0} MB",
                        convertToMegabytes(Runtime.getRuntime().freeMemory()));
                } else if(rowIndex==7){
                    return MessageFormat.format("{0} MB",
                        convertToMegabytes(Runtime.getRuntime().maxMemory()));
                } else if(rowIndex==8){
                    return Integer.valueOf(Runtime.getRuntime().availableProcessors());
                } else {
                    return null;
                }
            } else {
                return NAMES[rowIndex];
            }
        }

        public void actionPerformed(ActionEvent e) {
            for (int i = 0; i < IS_VARIABLE.length; i++) {
                if (IS_VARIABLE[i]) {
                    fireTableCellUpdated(i, 1);
                }
            }
        }
    }

    private static long convertToMegabytes(long num) {
        if (num <= Long.MAX_VALUE - 512 * 1024) {
            num += 512 * 1024;
        }
        return num / (1024 * 1024);
    }
}

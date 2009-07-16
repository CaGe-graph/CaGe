package lisken.uitoolbox;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.util.Vector;
import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.KeyStroke;
import javax.swing.SwingConstants;
import javax.swing.WindowConstants;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import lisken.systoolbox.MutableInteger;

public class OrderedChoice extends JPanel implements ListSelectionListener {

    private JButton addButton;
    private JButton removeButton;
    private JButton upButton;
    private JButton downButton;
    private JList choiceList;
    private JList selectionList;
    private Dimension listSize;
    private JScrollPane choicePane;
    private JScrollPane selectionPane;
    private Object[] choices;
    private int[] position;
    private Vector choice, selection, highlight;
    private boolean dialogCompleted;
    private boolean noEmptySelection;
    private boolean building;

    public OrderedChoice(Object[] choices) {
        this.choices = choices;
        position = new int[choices.length];
        choice = new Vector(choices.length);
        selection = new Vector(choices.length);
        highlight = new Vector(choices.length);
        for (int i = 0; i < choices.length; ++i) {
            choice.addElement(new MutableInteger(i));
            position[i] = -1;
        }
        noEmptySelection = false;
        addButton = new JButton();
        addButton.setText("Add");
        addButton.setIcon(new ImageIcon(ClassLoader.getSystemResource("lisken/uitoolbox/right.gif")));
        addButton.setHorizontalTextPosition(SwingConstants.LEFT);
        addButton.setMnemonic(KeyEvent.VK_A);
        addButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                addToSelection();
            }
        });
        removeButton = new JButton();
        removeButton.setText("Remove");
        removeButton.setIcon(new ImageIcon(ClassLoader.getSystemResource("lisken/uitoolbox/left.gif")));
        removeButton.setHorizontalTextPosition(SwingConstants.RIGHT);
        removeButton.setMnemonic(KeyEvent.VK_R);
        removeButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                removeFromSelection();
            }
        });
        upButton = new JButton();
        upButton.setText("Up");
        upButton.setMnemonic(KeyEvent.VK_U);
        upButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                upInSelection();
            }
        });
        downButton = new JButton();
        downButton.setText("Down");
        downButton.setMnemonic(KeyEvent.VK_D);
        downButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                downInSelection();
            }
        });
        choiceList = new JList();
        choiceList.setVisibleRowCount(Math.min(10, choices.length));
        choiceList.addListSelectionListener(this);
        selectionList = new JList();
        selectionList.setVisibleRowCount(choiceList.getVisibleRowCount());
        listSize = null;
        buildLists();
        listSize = choiceList.getPreferredSize();
        choicePane = new JScrollPane(choiceList);
        selectionPane = new JScrollPane(selectionList);
        Dimension paneSize = choicePane.getPreferredSize();
        choicePane.setPreferredSize(paneSize);
        selectionPane.setPreferredSize(paneSize);
        setLayout(new GridBagLayout());
        add(new JLabel("available:"), new GridBagConstraints(0, 0, 2, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 10, 0), 0, 0));
        add(new JLabel("selected:"), new GridBagConstraints(2, 0, 2, 1, 0.0, 0.0, GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 10, 10, 0), 0, 0));
        add(choicePane, new GridBagConstraints(0, 1, 1, 3, 0.0, 0.0, GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
        add(addButton, new GridBagConstraints(1, 1, 1, 1, 0.0, 0.0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(2, 2, 2, 2), 0, 0));
        add(removeButton, new GridBagConstraints(1, 2, 1, 1, 0.0, 0.0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(2, 2, 2, 2), 0, 0));
        add(selectionPane, new GridBagConstraints(2, 1, 1, 3, 0.0, 0.0, GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 10, 0, 0), 0, 0));
        add(upButton, new GridBagConstraints(3, 1, 1, 1, 0.0, 0.0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(2, 2, 2, 2), 0, 0));
        add(downButton, new GridBagConstraints(3, 2, 1, 1, 0.0, 0.0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(2, 2, 2, 2), 0, 0));
    }

    public void setPositions(int[] newPositions) {
        System.arraycopy(newPositions, 0, position, 0,
                Math.min(position.length, newPositions.length));
        buildVectors();
        buildLists();
    }

    public void allowEmptySelection(boolean allowEmptySelection) {
        if (allowEmptySelection) {
            noEmptySelection = false;
        } else {
            if (selection.size() == 0) {
                throw new RuntimeException("this OrderedChoice currently has an empty choice, so don't disallow that");
            } else {
                noEmptySelection = true;
            }
        }
    }

    public boolean getAllowEmptySelection() {
        return !noEmptySelection;
    }

    public void valueChanged(ListSelectionEvent e) {
        if (e.getValueIsAdjusting()) {
            return;
        }
        if (building) {
            return;
        }
        int first = e.getFirstIndex();
        int last = Math.min(e.getLastIndex(), choice.size() - 1);
        int p;
        for (int i = first; i <= last; ++i) {
            MutableInteger entry = (MutableInteger) choice.elementAt(i);
            if (choiceList.isSelectedIndex(i)) {
                if (highlight.indexOf(entry) < 0) {
                    highlight.addElement(entry);
                }
            } else {
                if ((p = highlight.indexOf(entry)) >= 0) {
                    highlight.removeElementAt(p);
                }
            }
        }
    }

    void addToSelection() {
        int highlighted = highlight.size(), oldSize = selection.size();
        if (highlighted == 0) {
            return;
        }
        for (int i = 0; i < highlighted; ++i) {
            MutableInteger entry = (MutableInteger) highlight.elementAt(i);
            choice.removeElement(entry);
            position[entry.intValue()] = selection.size();
            selection.addElement(entry);
        }
        highlight.setSize(0);
        buildLists();
        selectionList.addSelectionInterval(oldSize, oldSize + highlighted - 1);
    }

    void removeFromSelection() {
        if (noEmptySelection) {
            if (selectionList.getSelectedIndices().length == selection.size()) {
                Toolkit.getDefaultToolkit().beep();
                return;
            }
        }
        for (int i = selection.size() - 1; i >= 0; --i) {
            if (!selectionList.isSelectedIndex(i)) {
                continue;
            }
            MutableInteger entry = (MutableInteger) selection.elementAt(i);
            selection.removeElementAt(i);
            position[entry.intValue()] = -1;
        }
        buildChoiceVector();
        buildLists();
    }

    void upInSelection() {
        int selectionSize = selection.size();
        boolean isSelected, noUnselected = true;
        int sel[] = selectionList.getSelectedIndices(), s = -1;
        for (int i = 0; i < selectionSize; ++i) {
            if (!(isSelected = selectionList.isSelectedIndex(i))) {
                noUnselected = false;
                continue;
            }
            ++s;
            if (noUnselected) {
                continue;
            }
            int j = i - 1;
            Object entry1 = selection.elementAt(j), entry2 = selection.elementAt(i);
            selection.setElementAt(entry1, i);
            selection.setElementAt(entry2, j);
            --sel[s];
        }
        buildSelectionList();
        selectionList.setSelectedIndices(sel);
    }

    void downInSelection() {
        boolean isSelected, noUnselected = true;
        int sel[] = selectionList.getSelectedIndices();
        int s = sel.length;
        for (int i = selection.size() - 1; i >= 0; --i) {
            if (!(isSelected = selectionList.isSelectedIndex(i))) {
                noUnselected = false;
                continue;
            }
            --s;
            if (noUnselected) {
                continue;
            }
            int j = i + 1;
            Object entry1 = selection.elementAt(j), entry2 = selection.elementAt(i);
            selection.setElementAt(entry1, i);
            selection.setElementAt(entry2, j);
            ++sel[s];
        }
        buildSelectionList();
        selectionList.setSelectedIndices(sel);
    }

    private void buildLists() {
        buildChoiceList();
        buildSelectionList();
    }

    private void buildChoiceList() {
        building = true;
        int choiceSize = choice.size();
        Object[] choiceData = new Object[choiceSize];
        for (int i = 0; i < choiceSize; ++i) {
            choiceData[i] = choices[((MutableInteger) choice.elementAt(i)).intValue()];
        }
        choiceList.setListData(choiceData);
        if (listSize != null) {
            choiceList.setPreferredSize(listSize);
        }
        building = false;
    }

    private void buildSelectionList() {
        building = true;
        int selectionSize = selection.size();
        Object[] selectionData = new Object[selectionSize];
        for (int i = 0; i < selectionSize; ++i) {
            selectionData[i] = choices[((MutableInteger) selection.elementAt(i)).intValue()];
        }
        selectionList.setListData(selectionData);
        if (listSize != null) {
            selectionList.setPreferredSize(listSize);
        }
        building = false;
    }

    private void buildVectors() {
        buildChoiceVector();
        buildSelectionVector();
    }

    private void buildChoiceVector() {
        choice.setSize(0);
        for (int i = 0; i < choices.length; ++i) {
            if (position[i] < 0) {
                choice.addElement(new MutableInteger(i));
            }
        }
    }

    private void buildSelectionVector() {
        int pos, maxPos = -1;
        selection.setSize(choices.length);
        for (int i = 0; i < choices.length; ++i) {
            if ((pos = position[i]) >= 0) {
                selection.setElementAt(new MutableInteger(i), pos);
                if (pos > maxPos) {
                    maxPos = pos;
                }
            }
        }
        selection.setSize(maxPos + 1);
    }

    public boolean getDialogCompleted() {
        return dialogCompleted;
    }

    public Object[] getSelection() {
        int selectionSize = selection.size();
        Object[] result = new Object[selectionSize];
        for (int i = 0; i < selectionSize; ++i) {
            result[i] = choices[((MutableInteger) selection.elementAt(i)).intValue()];
        }
        return result;
    }

    public void runDialog(String title) {
        dialogCompleted = false;
        int[] oldPositions = (int[]) position.clone();
        final JDialog d = new JDialog((Frame) null, title, true);
        JButton okButton = new JButton("Ok");
        okButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                dialogCompleted = true;
                d.dispose();
            }
        });
        ActionListener cancelAction = new ActionListener() {

            public void actionPerformed(ActionEvent actionEvent) {
                d.dispose();
            }
        };
        JButton cancelButton = new JButton("Cancel");
        cancelButton.addActionListener(cancelAction);
        JPanel buttonPanel = new JPanel();
        buttonPanel.setLayout(new FlowLayout(FlowLayout.CENTER, 10, 0));
        buttonPanel.add(okButton);
        buttonPanel.add(cancelButton);
        setBorder(BorderFactory.createEmptyBorder(20, 20, 10, 20));
        buttonPanel.setBorder(BorderFactory.createEmptyBorder(10, 20, 20, 20));
        d.getContentPane().add(this, BorderLayout.CENTER);
        d.getContentPane().add(buttonPanel, BorderLayout.SOUTH);
        d.getRootPane().setDefaultButton(okButton);
        d.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
        d.getRootPane().registerKeyboardAction(cancelAction,
                KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0),
                JComponent.WHEN_IN_FOCUSED_WINDOW);
        d.pack();
        d.setVisible(true);
        if (!dialogCompleted) {
            setPositions(oldPositions);
        } else {
            choiceList.clearSelection();
            selectionList.clearSelection();
        }
    }

    public static void main(String[] args) {
        OrderedChoice c =
                new OrderedChoice(new String[]{"Hallo", "Test", "yes, test", "and this test contains long entries", "and some more"});
        c.setPositions(new int[]{-1, 2, -1, 0, 1});
        c.runDialog("Ordered Choice");
        System.err.println("completed: " + c.getDialogCompleted());
        System.exit(0);
    }
}


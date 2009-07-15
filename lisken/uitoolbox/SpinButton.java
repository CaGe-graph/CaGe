package lisken.uitoolbox;

import java.awt.Dimension;
import java.awt.Font;
import java.awt.Insets;
import java.awt.LayoutManager;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.Vector;
import javax.swing.AbstractAction;
import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
import javax.swing.BoundedRangeModel;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.DefaultBoundedRangeModel;
import javax.swing.DefaultButtonModel;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import javax.swing.JTextField;
import javax.swing.KeyStroke;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.text.Keymap;

public class SpinButton extends JPanel
        implements SizeListener, ChangeListener, ActionListener, FocusListener, java.io.Serializable {

    private EnhancedTextField text;
//  private JScrollBar bar;
    private JPanel buttonPanel;
    private ImageButton up,  down;
    private JSeparator sep;
    private int lastH;
    private BoundedRangeModel model;
    private int value;
    private int minValue;
    private int maxValue;
    private transient Vector changeListeners;

    public SpinButton() {
        this(0, 0, 9);
    }

    public SpinButton(int initValue, int min, int max) {
        this(new DefaultBoundedRangeModel((min + max) / 2, 0, Math.min(min, max), Math.max(min, max)));
        setValue(initValue);
    }

    public SpinButton(BoundedRangeModel m) {
        super();

        lastH = 0;
        super.setLayout(new BoxLayout(this, BoxLayout.X_AXIS));

        text = new EnhancedTextField();
        text.setHorizontalAlignment(JTextField.RIGHT);
        new JTextComponentFocusSelector(text);

        model = m;
        modelChanged();
        text.setColumns(Math.max(2, Math.max(Integer.toString(minValue).length(), Integer.toString(maxValue).length())));

        Keymap km = EnhancedTextField.addKeymap(null, text.getKeymap());
        km.addActionForKeyStroke(
                KeyStroke.getKeyStroke(KeyEvent.VK_UP, 0),
                new AdjustAction(0.0, 1));
        km.addActionForKeyStroke(
                KeyStroke.getKeyStroke(KeyEvent.VK_DOWN, 0),
                new AdjustAction(0.0, -1));
        km.addActionForKeyStroke(
                KeyStroke.getKeyStroke(KeyEvent.VK_PAGE_UP, 0),
                new AdjustAction(0.1, 0));
        km.addActionForKeyStroke(
                KeyStroke.getKeyStroke(KeyEvent.VK_PAGE_DOWN, 0),
                new AdjustAction(-0.1, 0));
        text.setKeymap(km);

        text.setDocument(new NumberDocument());
        text.setSizeListener(this);
        text.addActionListener(this);
        text.addFocusListener(this);

        buttonPanel = new JPanel(false);
        buttonPanel.setLayout(new BoxLayout(buttonPanel, BoxLayout.Y_AXIS));

//    bar = new JScrollBar(JScrollBar.VERTICAL, min, 1, min, max);
//    bar.addAdjustmentListener(new ScrollAdjustmentListener());

        up = new ImageButton(new ImageIcon(ClassLoader.getSystemResource("lisken/uitoolbox/SpinButtonUp.gif")), text);
        up.setFocusTraversable(false);
        up.addActionListener(new AdjustAction(0.0, 1));

        down = new ImageButton(new ImageIcon(ClassLoader.getSystemResource("lisken/uitoolbox/SpinButtonDown.gif")), text);
        down.setFocusTraversable(false);
        down.addActionListener(new AdjustAction(0.0, -1));

        sep = new JSeparator(SwingConstants.HORIZONTAL);

        buttonPanel.add(up);
        buttonPanel.add(sep);
        buttonPanel.add(down);
        buttonPanel.setBorder(BorderFactory.createEtchedBorder());

        add(text);
        add(buttonPanel);

        addFocusListener(this);

        model.addChangeListener(this);
        setTextAndValue(model.getValue());
    }

    void modelChanged() {
        minValue = model.getMinimum();
        maxValue = model.getMaximum();
    }

    public void setMinimum(int min) {
        model.setMinimum(min);
        modelChanged();
    }

    public int getMinimum() {
        return model.getMinimum();
    }

    public void setMaximum(int max) {
        model.setMaximum(max);
        modelChanged();
    }

    public int getMaximum() {
        return model.getMaximum();
    }

    /*
    public Dimension getPreferredSize()
    {
    return new Dimension(
    text.getPreferredSize().width + bar.getPreferredSize().width,
    text.getPreferredSize().height);
    }
     */
    public void sizing() {
        int h = text.getHeight() - insetsHeight(buttonPanel) - sep.getPreferredSize().height - up.getIcon().getIconHeight() - down.getIcon().getIconHeight();
        if (h != lastH) {
            up.setBorder(BorderFactory.createEmptyBorder((h + 3) / 4, 0, (h + 1) / 4, 0));
            down.setBorder(BorderFactory.createEmptyBorder((h + 0) / 4, 0, (h + 2) / 4, 0));
            lastH = h;
            buttonPanel.validate();
        }
    }

    int insetsHeight(JComponent c) {
        Insets i = c.getInsets();
        return i.top + i.bottom;
    }

    public BoundedRangeModel getModel() {
        return model;
    }

    public void setValue(int newValue) {
        model.setValue(newValue);
    }

    public int getValue() {
        return value;
    }

    private void checkNewTextValue(boolean setIfNew) {
        int textValue;
        try {
            textValue = Integer.parseInt(text.getText());
        } catch (NumberFormatException e) {
            Toolkit.getDefaultToolkit().beep();
            textValue = value;
        }
        if (textValue != value) {
            if (minValue <= textValue && textValue <= maxValue) {
                if (setIfNew) {
                    setValue(textValue);
                    return;
                }
            } else {
                Toolkit.getDefaultToolkit().beep();
            }
        }
        setTextAndValue(value);
    }

    private void setTextAndValue(int newValue) {
        value = newValue;
        text.setText(Integer.toString(newValue));
        if (text.hasFocus()) {
            text.selectAll();
        }
    }

    private class AdjustAction extends AbstractAction {

        double fraction;
        int offset;

        public AdjustAction(double f, int o) {
            fraction = f;
            offset = o;
        }

        public void actionPerformed(ActionEvent e) {
            checkNewTextValue(true);
            setValue(value + (int) Math.round((maxValue - minValue) * fraction) + offset);
        }
    }

    /*
    private class ScrollAdjustmentListener implements AdjustmentListener
    {
    public void adjustmentValueChanged(AdjustmentEvent e)
    {
    setValue(e.getValue());
    }
    }
     */
    public synchronized void removeChangeListener(ChangeListener l) {
        if (changeListeners != null && changeListeners.contains(l)) {
            Vector v = (Vector) changeListeners.clone();
            v.removeElement(l);
            changeListeners = v;
        }
    }

    public synchronized void addChangeListener(ChangeListener l) {
        Vector v = changeListeners == null ? new Vector(2) : (Vector) changeListeners.clone();
        if (!v.contains(l)) {
            v.addElement(l);
            changeListeners = v;
        }
    }

    protected void fireStateChanged() {
        if (changeListeners != null) {
            ChangeEvent e = new ChangeEvent(this);
            Vector listeners = changeListeners;
            int count = listeners.size();
            for (int i = 0; i < count; i++) {
                ((ChangeListener) listeners.elementAt(i)).stateChanged(e);
            }
        }
    }

    public void actionPerformed(ActionEvent e) {
        checkNewTextValue(true);
    }

    public void focusGained(FocusEvent e) {
        if (e.getSource() == (Object) this) {
            text.requestFocus();
        }
    }

    public void focusLost(FocusEvent e) {
        checkNewTextValue(true);
    }

    public void stateChanged(ChangeEvent e) {
        int newValue = ((BoundedRangeModel) e.getSource()).getValue();
        setTextAndValue(newValue);
//    bar.setValue(value);
        modelChanged();
        fireStateChanged();
    }

    public boolean isEnabled() {
        return text.isEnabled();
    }

    public void setEnabled(boolean b) {
        text.setEnabled(b);
        up.setEnabled(b);
        down.setEnabled(b);
    }

    public Font getFont() {
        return (text == null ? super.getFont() : text.getFont());
    }

    public void setFont(Font f) {
        if (text == null) {
            super.setFont(f);
        } else {
            text.setFont(f);
        }
    }

    public final LayoutManager getLayout() {
        return null;
    }

    public final void setLayout(LayoutManager mgr) {
    }

    public static void main(String[] argv) {
        JFrame f = new JFrame("SpinButton test");
        f.addWindowListener(new WindowAdapter() {

            public void windowClosing(WindowEvent e) {
                System.exit(0);
            }
        });
        UItoolbox.addExitOnEscape(f);
        ((JComponent) f.getContentPane()).setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));
        JPanel p = new JPanel();
        final SpinButton s = new SpinButton(100, -10, 10);
        BoundedRangeModel m = s.getModel();
        s.addChangeListener(new ChangeListener() {

            public void stateChanged(ChangeEvent e) {
                System.err.println("  button changed: " + ((SpinButton) e.getSource()).getValue());
            }
        });
        s.getModel().addChangeListener(new ChangeListener() {

            public void stateChanged(ChangeEvent e) {
                System.err.println("  model changed: " + ((BoundedRangeModel) e.getSource()).getValue());
            }
        });
//    p.setLayout(new BoxLayout(p, BoxLayout.X_AXIS));
        System.err.println("value: " + s.getValue() + " (model value: " + m.getValue() + ")");
        System.err.println("changing button:");
        s.setValue(1);
        System.err.println("changing model:");
        m.setValue(2);
        System.err.println("running window:");
        final JCheckBox e = new JCheckBox("Enabled?", true);
        e.addChangeListener(new ChangeListener() {

            public void stateChanged(ChangeEvent ce) {
                s.setEnabled(e.isSelected());
            }
        });
        p.add(e);
        p.add(Box.createHorizontalStrut(30));
        p.add(new JLabel("Value: "));
        p.add(s);
        p.add(Box.createHorizontalStrut(30));
        final JCheckBox v = new JCheckBox("Visible?", true);
        v.addChangeListener(new ChangeListener() {

            public void stateChanged(ChangeEvent ce) {
                s.setVisible(v.isSelected());
            }
        });
        p.add(v);
        f.setContentPane(p);
        f.pack();
        s.requestFocus();
        f.setVisible(true);
    }
}


// an object that can be notified when some other object's size changes
interface SizeListener {

    public void sizing();
}

/**
A JTextField that can notify one other object of size changes.
 */
class EnhancedTextField extends JTextField {
    // too lazy to implement a full list - for internal use only

    SizeListener sizeListener;

    public EnhancedTextField() {
        super();
    }

    public EnhancedTextField(int columns) {
        super(columns);
    }

    public void setSizeListener(SizeListener l) {
        sizeListener = l;
    }

    private void fireSizing() {
        if (sizeListener != null) {
            sizeListener.sizing();
        }
    }
    // this version of setBounds does the notifying

    public void setBounds(int x, int y, int newWidth, int newHeight) {
        super.setBounds(x, y, newWidth, newHeight);
        fireSizing();
    }
    // all other size-changing methods are redirected into the one above

    public void setBounds(Rectangle r) {
        setBounds(r.x, r.y, r.width, r.height);
    }

    public void setSize(Dimension d) {
        Point p = getLocation();
        setBounds(p.x, p.y, d.width, d.height);
    }

    public void setSize(int newWidth, int newHeight) {
        Point p = getLocation();
        setBounds(p.x, p.y, newWidth, newHeight);
    }
}

/**
A button with just an image and a "repeated action" capability.
Optionally, focus is transferred to another JComponent after the
button is released.
 */
class ImageButton extends JButton {

    boolean focusTraversable;
    SizeListener sizeListener;

    public ImageButton(Icon icon) {
        this(icon, null);
    }

    public ImageButton(Icon icon, JComponent supportedComponent) {
        super(icon);
        setBorder(BorderFactory.createEmptyBorder());
        setModel(new ButtonMouseRepeaterModel(this, supportedComponent));
        focusTraversable = true;
        sizeListener = null;
    }

    public void setText(String text) {
    }

    public void setFocusTraversable(boolean b) {
        focusTraversable = b;
    }

    public boolean isFocusTraversable() {
        return focusTraversable;
    }
}

/**
A button model that modifies DefaultButtonModel to enable
repeated action firing by keeping the button pressed.
Optionally, the model transfers focus to another JComponent
when the button is released.
 */
class ButtonMouseRepeaterModel extends DefaultButtonModel
        implements ActionListener {

    final static int INITIAL_DELAY = 500,  DELAY = 50;
    AbstractButton button;
    JComponent supportedComponent;
    javax.swing.Timer firingTimer;
    ActionEvent action;

    public ButtonMouseRepeaterModel(AbstractButton b) {
        this(b, null);
    }

    public ButtonMouseRepeaterModel(AbstractButton b, JComponent c) {
        button = b;
        supportedComponent = c;
        // We want a timer that fires once immediately as it starts,
        // then proceeds like any other timer.
        firingTimer = new ImmediateFireTimer(DELAY, this);
        firingTimer.setInitialDelay(INITIAL_DELAY);
        action = new ActionEvent(this, ActionEvent.ACTION_PERFORMED, getActionCommand());
    }

    public void setPressed(boolean b) {
        // Stolen from the original source.
        if ((isPressed() == b) || !isEnabled()) {
            return;
        }
        if (b) {
            firingTimer.start();
        } else {
            firingTimer.stop();
            // Persuade the original procedure not to fire on button release.
            setArmed(false);
            // Transfer focus if requested.
            if (supportedComponent != null) {
                supportedComponent.requestFocus();
            }
        }
        super.setPressed(b);
    }

    public void actionPerformed(ActionEvent e) {
        if (isArmed()) {
            fireActionPerformed(action);
        }
    }
}

class ImmediateFireTimer extends javax.swing.Timer
        implements Runnable {

    boolean running;

    public ImmediateFireTimer(int delay, ActionListener listener) {
        super(delay, listener);
        running = false;
    }

    public void start() {
        running = true;
        SwingUtilities.invokeLater(this);
        synchronized (this) {
            if (running) {
                super.start();
            }
        }
    }

    public synchronized void stop() {
        running = false;
        super.stop();
    }

    public void run() {
        fireActionPerformed(new ActionEvent(this, 0, null));
    }
}


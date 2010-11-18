package lisken.uitoolbox;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Insets;
import java.awt.LayoutManager;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.io.Serializable;
import java.util.Dictionary;
import java.util.Hashtable;
import java.util.Vector;
import javax.accessibility.AccessibleContext;
import javax.swing.BorderFactory;
import javax.swing.BoundedRangeModel;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.SwingConstants;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.plaf.SliderUI;

public class EnhancedSlider extends JPanel implements FocusListener, Serializable {

    private final boolean debug;
    private JSlider slider;
    private SliderValueLabel valueLabel;
    private int snapWhileDragging;
    private boolean snapping;
    private int snappedValue;
    private boolean paintMinorTicks;
    private transient Vector changeListeners;
    private double sizeFactor;
    private boolean clickScrollByBlock;

    public EnhancedSlider() {
        this(false);
    }

    public EnhancedSlider(boolean debug) {
        this(SwingConstants.HORIZONTAL, 0, 100, 50, debug);
    }
    
    public EnhancedSlider(int orientation, int min, int max, int value, boolean debug) {
        slider = new JSlider(orientation, min, max, value) {

            public void setValue(int n) {
                if (snapping) {
                    super.setValue(snappedValue);
                    snapping = false;
                } else {
                    super.setValue(n);
                }
            }
        };
        slider.setUI(new EnhancedSliderUI(this));
        slider.addChangeListener(new EnhancedSliderChangeListener(this));
        valueLabel = new SliderValueLabel(slider);
        this.setOrientation(orientation);
        this.add(valueLabel);
        this.add(slider);
        snapping = false;
        paintMinorTicks = true;
        sizeFactor = 1;
        clickScrollByBlock = true;
        this.debug = debug;
    }

    public JLabel getValueLabel() {
        return valueLabel;
    }

    /**
     * Sets the sliders step size while dragging. This will cause the slider
     * to jump in discrete steps instead of sliding and jumping when released.
     *
     * @param n The step size for this slider
     */
    public void setSnapWhileDragging(int n) {
        snapWhileDragging = n;
    }

    /**
     * Returns the sliders step size while dragging.
     *
     * @see #setSnapWhileDragging(int)
     */
    public int getSnapWhileDragging() {
        return snapWhileDragging;
    }

    public void setPaintMinorTicks(boolean b) {
        paintMinorTicks = b;
    }

    public boolean getPaintMinorTicks() {
        return paintMinorTicks;
    }

    public void adjustValueLabelLocation() {
        if (valueLabel != null) {
            valueLabel.adjustBounds();
        }
    }

    public void setOrientation(int orientation) {
        slider.setOrientation(orientation);
        switch (orientation) {
            case SwingConstants.VERTICAL:
                super.setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
                break;
            case SwingConstants.HORIZONTAL:
                super.setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
                break;
        }
        valueLabel.adjustOrientation(orientation);
    }

    void setSnappedValue(int n) {
        snapping = true;
        snappedValue = n;
    }

    public void setClickScrollByBlock(boolean b) {
        clickScrollByBlock = b;
    }

    public boolean getClickScrollByBlock() {
        return clickScrollByBlock;
    }

    public void setSizeFactor(double sizeFactor) {
        this.sizeFactor = sizeFactor;
    }

    public Dimension getPreferredSize() {
        int extraSpace;
        switch (getOrientation()) {
            case SwingConstants.HORIZONTAL:
                extraSpace = getSize().width - ((EnhancedSliderUI) slider.getUI()).getTrackRectangle().width;
                return new Dimension(((int) Math.round((getMaximum() - getMinimum()) * sizeFactor)) + extraSpace, super.getPreferredSize().height);
            case SwingConstants.VERTICAL:
                extraSpace = getSize().height - ((EnhancedSliderUI) slider.getUI()).getTrackRectangle().height;
                return new Dimension(super.getPreferredSize().width, ((int) Math.round((getMaximum() - getMinimum()) * sizeFactor)) + extraSpace);
        }
        return null;
    }

    public JSlider slider() {
        return slider;
    }

    public void setEnabled(boolean enabled) {
        slider.setEnabled(enabled);
    }

    public boolean isEnabled() {
        return slider.isEnabled();
    }

    public SliderUI getSliderUI() {
        return (EnhancedSliderUI) slider.getUI();
    }

    public BoundedRangeModel getModel() {
        return slider.getModel();
    }

    public void setModel(BoundedRangeModel newModel) {
        slider.setModel(newModel);
        valueLabel.calculateSize();
    }

    public int getValue() {
        return slider.getValue();
    }

    public void setValue(int n) {
        slider.setValue(n);
    }

    public int getMinimum() {
        return slider.getMinimum();
    }

    public void setMinimum(int minimum) {
        slider.setMinimum(minimum);
        valueLabel.calculateSize();
    }

    public int getMaximum() {
        return slider.getMaximum();
    }

    public void setMaximum(int maximum) {
        slider.setMaximum(maximum);
        valueLabel.calculateSize();
    }

    public boolean getValueIsAdjusting() {
        return slider.getValueIsAdjusting();
    }

    public void setValueIsAdjusting(boolean b) {
        slider.setValueIsAdjusting(b);
    }

    public int getExtent() {
        return slider.getExtent();
    }

    public void setExtent(int extent) {
        slider.setExtent(extent);
    }

    public int getOrientation() {
        return slider.getOrientation();
    }

    public Dictionary getLabelTable() {
        return slider.getLabelTable();
    }

    public void setLabelTable(Dictionary labels) {
        slider.setLabelTable(labels);
    }

    public Hashtable createStandardLabels(int increment) {
        return slider.createStandardLabels(increment);
    }

    public Hashtable createStandardLabels(int increment, int start) {
        return slider.createStandardLabels(increment, start);
    }

    public boolean getInverted() {
        return slider.getInverted();
    }

    public void setInverted(boolean b) {
        slider.setInverted(b);
    }

    public int getMajorTickSpacing() {
        return slider.getMajorTickSpacing();
    }

    public void setMajorTickSpacing(int n) {
        slider.setMajorTickSpacing(n);
    }

    public int getMinorTickSpacing() {
        return slider.getMinorTickSpacing();
    }

    public void setMinorTickSpacing(int n) {
        slider.setMinorTickSpacing(n);
    }

    public boolean getSnapToTicks() {
        return slider.getSnapToTicks();
    }

    public void setSnapToTicks(boolean b) {
        slider.setSnapToTicks(b);
    }

    public boolean getPaintTicks() {
        return slider.getPaintTicks();
    }

    public void setPaintTicks(boolean b) {
        slider.setPaintTicks(b);
    }

    public boolean getPaintTrack() {
        return slider.getPaintTrack();
    }

    public void setPaintTrack(boolean b) {
        slider.setPaintTrack(b);
    }

    public boolean getPaintLabels() {
        return slider.getPaintLabels();
    }

    public void setPaintLabels(boolean b) {
        slider.setPaintLabels(b);
    }

    public AccessibleContext getAccessibleContext() {
        return slider.getAccessibleContext();
    }

    public final LayoutManager getLayout() {
        return null;
    }

    public final void setLayout(LayoutManager mgr) {
    }

    public void focusGained(FocusEvent e) {
        if (debug) {
            System.err.println("  slider focus");
        }
        slider.requestFocus();
    }

    public void focusLost(FocusEvent e) {
    }

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

    private class EnhancedSliderUI extends javax.swing.plaf.basic.BasicSliderUI {

        private EnhancedSlider enhancedSlider;

        public EnhancedSliderUI(EnhancedSlider s) {
            super(s.slider());
            enhancedSlider = s;
        }

        public Rectangle getTrackRectangle() {
            return trackRect;
        }

        public Rectangle getThumbRectangle() {
            return thumbRect;
        }

        protected void calculateTrackRect() {
            super.calculateTrackRect();
            adjustValueLabelLocation();
        }

        protected void calculateThumbLocation() {
            super.calculateThumbLocation();
            adjustValueLabelLocation();
        }

        public void setThumbLocation(int x, int y) {
            int snap = enhancedSlider.getSnapWhileDragging();
            if (snap != 0) {
                switch (slider.getOrientation()) {
                    case SwingConstants.HORIZONTAL:
                        x = snappedPosition(x, snap, trackRect.x - thumbRect.width / 2, trackRect.width);
                        break;
                    case SwingConstants.VERTICAL:
                        y = snappedPosition(y, snap, trackRect.y - thumbRect.height / 2, trackRect.height);
                        break;
                }
            }
            super.setThumbLocation(x, y);
            adjustValueLabelLocation();
        }

        private void adjustValueLabelLocation() {
            if (enhancedSlider.getParent() != null) {
                enhancedSlider.adjustValueLabelLocation();
            }
        }

        int snappedPosition(int position, int snap, int start, int width) {
            int min, max, range, value, distance;
            boolean inverted;
            min = slider.getMinimum();
            max = slider.getMaximum();
            inverted = slider.getInverted();
            range = max - min;
            distance = roundIntFraction((position - start) * range, width, snap);
            value = inverted ? max - distance : min + distance;
            enhancedSlider.setSnappedValue(value);
            return roundIntFraction(distance * width, range, 1) + start;
        }

        int roundIntFraction(int nom, int denom, int gran) {
            return ((2 * nom + denom * gran) / (denom * 2 * gran)) * gran;
        }

        protected void paintMinorTickForHorizSlider(Graphics g, Rectangle tickBounds, int x) {
            if (enhancedSlider.getPaintMinorTicks()) {
                super.paintMinorTickForHorizSlider(g, tickBounds, x);
            }
        }

        protected void paintMinorTickForVertSlider(Graphics g, Rectangle tickBounds, int y) {
            if (enhancedSlider.getPaintMinorTicks()) {
                super.paintMinorTickForVertSlider(g, tickBounds, y);
            }
        }

        protected int epsilon() {
            return enhancedSlider.getSnapToTicks() ? enhancedSlider.getMinorTickSpacing() : 1;
        }

        protected void scrollBy(int dist, int direction) {
            synchronized (enhancedSlider.slider()) {
                int oldValue = enhancedSlider.getValue();
                int delta = dist * ((direction > 0) ? POSITIVE_SCROLL : NEGATIVE_SCROLL);
                enhancedSlider.setValue(oldValue + delta);
            }
        }

        public void scrollByUnit(int direction) {
            scrollBy(epsilon(), direction);
        }

        public void scrollByBlock(int direction) {
            scrollBy(
                    roundIntFraction(
                    enhancedSlider.getMaximum() - enhancedSlider.getMinimum(), 10,
                    epsilon()),
                    direction);
        }

        protected void scrollDueToClickInTrack(int direction) {
            if (enhancedSlider.getClickScrollByBlock()) {
                scrollByBlock(direction);
            } else {
                scrollByUnit(direction);
            }
        }
    }

    private class EnhancedSliderChangeListener implements ChangeListener {

        private EnhancedSlider enhancedSlider;

        public EnhancedSliderChangeListener(EnhancedSlider es) {
            enhancedSlider = es;
        }

        public void stateChanged(ChangeEvent e) {
            JLabel label = enhancedSlider.getValueLabel();
            if (label == null) {
                return;
            }
            label.setText(Integer.toString(enhancedSlider.getValue()));
            enhancedSlider.fireStateChanged();
        }
    }

    private class SliderValueLabel extends JLabel {

        private JSlider slider;
        private int width, height;

        public SliderValueLabel(JSlider s) {
            slider = s;
            setBorder(BorderFactory.createEmptyBorder(0, 2, 1, 2));
            calculateSize();
            setForeground(Color.black);
        }

        public void setLocation(Point p) {
            setBounds(p.x, p.y, this.getWidth(), this.getHeight());
        }

        public void setLocation(int x, int y) {
            setBounds(x, y, this.getWidth(), this.getHeight());
        }

        public void setBounds(Rectangle r) {
            setBounds(r.x, r.y, r.width, r.height);
        }

        public void setBounds(int x, int y, int newWidth, int newHeight) {
            if (slider == null) {
                if (debug) {
                    System.err.println("  new place as requested: " + x + ", " + y);
                }
                super.setBounds(x, y, newWidth, newHeight);
            } else {
                adjustBounds(newWidth, newHeight);
            }
        }

        public void setSize(Dimension d) {
            Point p = this.getLocation();
            setBounds(p.x, p.y, d.width, d.height);
        }

        public void setSize(int newWidth, int newHeight) {
            Point p = this.getLocation();
            setBounds(p.x, p.y, newWidth, newHeight);
        }

        public void adjustBounds() {
            adjustBounds(getWidth(), getHeight());
        }

        public void adjustBounds(int newWidth, int newHeight) {
            newWidth = width;
            newHeight = height;
            Rectangle rect = ((EnhancedSliderUI) slider.getUI()).getThumbRectangle();
            switch (slider.getOrientation()) {
                case SwingConstants.HORIZONTAL:
                    if (debug) {
                        System.err.println("  new width: " + newWidth + ", th = " + rect.x + ", " + rect.width);
                    }
                    if (debug) {
                        System.err.println("  new place: " + (rect.x + (rect.width - newWidth) / 2) + ", " + getLocation().y);
                    }
                    super.setBounds(rect.x + (rect.width - newWidth) / 2, getLocation().y, newWidth, newHeight);
                    break;
                case SwingConstants.VERTICAL:
                    if (debug) {
                        System.err.println("  new height: " + newHeight + ", th = " + rect.y + ", " + rect.height);
                    }
                    if (debug) {
                        System.err.println("  new place: " + getLocation().x + ", " + (rect.y + (rect.height - newHeight) / 2));
                    }
                    super.setBounds(getLocation().x, rect.y + (rect.height - newHeight) / 2, newWidth, newHeight);
                    break;
            }
        }

        public void setFont(Font font) {
            super.setFont(font);
            calculateSize();
        }

        public void calculateSize() {
            Insets insets = getInsets();
            if (slider == null) {
                return;
            }
            FontMetrics metrics = getFontMetrics(getFont());
            width = Math.max(
                    metrics.stringWidth(Integer.toString(slider.getMinimum())),
                    metrics.stringWidth(Integer.toString(slider.getMaximum())));
            width += insets.left + insets.right;
            height = metrics.getHeight() + insets.top + insets.bottom;
        }

        public void adjustSize() {
            setSize(width, height);
        }

        public void setText(String text) {
            super.setText(text);
            adjustSize();
        }

        public Dimension getMinimumSize() {
            return new Dimension(width, height);
        }

        public Dimension getPreferredSize() {
            return new Dimension(width, height);
        }

        public void adjustOrientation(int sliderOrientation) {
            switch (sliderOrientation) {
                case SwingConstants.VERTICAL:
                    setHorizontalAlignment(SwingConstants.RIGHT);
                    break;
                case SwingConstants.HORIZONTAL:
                    setHorizontalAlignment(SwingConstants.CENTER);
                    break;
            }
            EnhancedSlider es = (EnhancedSlider) this.getParent();
            if (es == null) {
                return;
            }
            slider = null;
            UItoolbox.pack(this);
            slider = es.slider();
            adjustBounds();
        }
    }
}

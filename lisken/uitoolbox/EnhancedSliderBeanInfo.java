
//Titel:CaGe
//Version:
//Copyright:Copyright (c) 1998
//Autor:Sebastian Lisken
//Organisation:FSP Mathematisierung, Universit�t Bielefeld
//Beschreibung:
package lisken.uitoolbox;

import java.beans.BeanInfo;
import java.beans.IntrospectionException;
import java.beans.PropertyDescriptor;
import java.beans.SimpleBeanInfo;

public class EnhancedSliderBeanInfo extends SimpleBeanInfo {

    Class beanClass = EnhancedSlider.class;
    String iconColor16x16Filename = "JCSlider.gif";
    String iconColor32x32Filename;
    String iconMono16x16Filename;
    String iconMono32x32Filename;

    public EnhancedSliderBeanInfo() {
    }

    public PropertyDescriptor[] getPropertyDescriptors() {
        try {
            PropertyDescriptor _accessibleContext = new PropertyDescriptor("accessibleContext", beanClass, "getAccessibleContext", null);

            PropertyDescriptor _extent = new PropertyDescriptor("extent", beanClass, "getExtent", "setExtent");

            PropertyDescriptor _inverted = new PropertyDescriptor("inverted", beanClass, "getInverted", "setInverted");

            PropertyDescriptor _labelTable = new PropertyDescriptor("labelTable", beanClass, "getLabelTable", "setLabelTable");

            PropertyDescriptor _majorTickSpacing = new PropertyDescriptor("majorTickSpacing", beanClass, "getMajorTickSpacing", "setMajorTickSpacing");

            PropertyDescriptor _maximum = new PropertyDescriptor("maximum", beanClass, "getMaximum", "setMaximum");

            PropertyDescriptor _minimum = new PropertyDescriptor("minimum", beanClass, "getMinimum", "setMinimum");

            PropertyDescriptor _minorTickSpacing = new PropertyDescriptor("minorTickSpacing", beanClass, "getMinorTickSpacing", "setMinorTickSpacing");

            PropertyDescriptor _model = new PropertyDescriptor("model", beanClass, "getModel", "setModel");

            PropertyDescriptor _orientation = new PropertyDescriptor("orientation", beanClass, "getOrientation", "setOrientation");

            PropertyDescriptor _paintLabels = new PropertyDescriptor("paintLabels", beanClass, "getPaintLabels", "setPaintLabels");

            PropertyDescriptor _paintMinorTicks = new PropertyDescriptor("paintMinorTicks", beanClass, "getPaintMinorTicks", "setPaintMinorTicks");

            PropertyDescriptor _paintTicks = new PropertyDescriptor("paintTicks", beanClass, "getPaintTicks", "setPaintTicks");

            PropertyDescriptor _paintTrack = new PropertyDescriptor("paintTrack", beanClass, "getPaintTrack", "setPaintTrack");

            PropertyDescriptor _snapToTicks = new PropertyDescriptor("snapToTicks", beanClass, "getSnapToTicks", "setSnapToTicks");

            PropertyDescriptor _snapWhileDragging = new PropertyDescriptor("snapWhileDragging", beanClass, "getSnapWhileDragging", "setSnapWhileDragging");

            PropertyDescriptor _UI = new PropertyDescriptor("UI", beanClass, "getUI", null);

            PropertyDescriptor _value = new PropertyDescriptor("value", beanClass, "getValue", "setValue");

            PropertyDescriptor _valueIsAdjusting = new PropertyDescriptor("valueIsAdjusting", beanClass, "getValueIsAdjusting", "setValueIsAdjusting");

            PropertyDescriptor[] pds = new PropertyDescriptor[]{
                _accessibleContext,
                _extent,
                _inverted,
                _labelTable,
                _majorTickSpacing,
                _maximum,
                _minimum,
                _minorTickSpacing,
                _model,
                _orientation,
                _paintLabels,
                _paintMinorTicks,
                _paintTicks,
                _paintTrack,
                _snapToTicks,
                _snapWhileDragging,
                _UI,
                _value,
                _valueIsAdjusting,};
            return pds;
        } catch (IntrospectionException ex) {
            ex.printStackTrace();
            return null;
        }
    }

    public java.awt.Image getIcon(int iconKind) {
        switch (iconKind) {
            case BeanInfo.ICON_COLOR_16x16:
                return iconColor16x16Filename != null ? loadImage(iconColor16x16Filename) : null;
            case BeanInfo.ICON_COLOR_32x32:
                return iconColor32x32Filename != null ? loadImage(iconColor32x32Filename) : null;
            case BeanInfo.ICON_MONO_16x16:
                return iconMono16x16Filename != null ? loadImage(iconMono16x16Filename) : null;
            case BeanInfo.ICON_MONO_32x32:
                return iconMono32x32Filename != null ? loadImage(iconMono32x32Filename) : null;
        }
        return null;
    }
}




 


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


public class SpinButtonBeanInfo extends SimpleBeanInfo
{
  Class beanClass = SpinButton.class;
  String iconColor16x16Filename;
  String iconColor32x32Filename;
  String iconMono16x16Filename;
  String iconMono32x32Filename;

  
  public SpinButtonBeanInfo()
  {
  }

  public PropertyDescriptor[] getPropertyDescriptors()
  {
    try 
    {
      PropertyDescriptor _spinValue = new PropertyDescriptor("spinValue", beanClass, "getSpinValue", "setSpinValue");
      
      PropertyDescriptor[] pds = new PropertyDescriptor[] {
        _spinValue,
      };
      return pds;
    }
    catch (IntrospectionException ex)
    {
      ex.printStackTrace();
      return null;
    }
  }

  public java.awt.Image getIcon(int iconKind)
  {
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

 

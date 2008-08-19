

package cage;


import java.awt.*;
import lisken.uitoolbox.*;


public class SavePSDialog extends SaveDialog
{
  public SavePSDialog(String title)
  {
    this(null, title);
  }

  public SavePSDialog(Frame parent, String title)
  {
    this(parent, title, true);
  }

  public SavePSDialog(Frame parent, String title, boolean useInfo)
  {
    super(parent, title, useInfo);
  }
}


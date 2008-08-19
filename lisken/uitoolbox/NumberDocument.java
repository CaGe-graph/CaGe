
package lisken.uitoolbox;


import java.awt.*;
import javax.swing.text.*;


/**
  A simple plain document that accepts only signed integers.
*/
public class NumberDocument extends PlainDocument
{
  boolean allowSign;
  final static char minusSign = '-';

  public NumberDocument()
  {
    this(true);
  }
  public NumberDocument(boolean sign)
  {
    allowSign = sign;
  }

  public void insertString(int offset, String string, AttributeSet attrs)
   throws BadLocationException
  {
    // accept only digits after "start" - will be 1 to allow a minus sign
    int start = 0;
    if (allowSign) {
      // if there is a minus sign, don't accept anything before it
      if (getText(0, 1).charAt(0) == minusSign) {
	if (offset == 0) {
	  Toolkit.getDefaultToolkit().beep();
	  return;
	}
      // if there isn't, allow a minus sign if it will be the first character
      } else if (offset == 0) {
	if (string.charAt(0) == minusSign) {
	  start = 1;
	}
      }
    }
    // remove any non-digits after start and beep if we find any
    for (int i = start; i < string.length(); ++i)
    {
      if (! Character.isDigit(string.charAt(i))) {
        Toolkit.getDefaultToolkit().beep();
        string = string.substring(0, i) + string.substring(i+1);
        --i;
      }
    }
    // the "cleansed" str may now be inserted
    super.insertString(offset, string, attrs);
  }
}


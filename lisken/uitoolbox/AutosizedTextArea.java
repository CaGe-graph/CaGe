
package lisken.uitoolbox;

import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Insets;
import javax.swing.JTextArea;
import javax.swing.border.Border;


public class AutosizedTextArea extends JTextArea
{
  Dimension size;

  public Dimension getPreferredScrollableViewportSize()
  {
    return size;
  }

  public void setText(String text)
  {
    int text_end = text.length();
    if (text_end > 0) {
      if (text.charAt(--text_end) != '\n') {
	++text_end;
      }
    }
    super.setText(text.substring(0, text_end));
    recalculateSize();
  }

  public void setFont(Font font)
  {
    super.setFont(font);
    recalculateSize();
  }

  public void setBorder(Border border)
  {
    super.setBorder(border);
    recalculateSize();
  }

  void recalculateSize()
  {
    FontMetrics fm = getFontMetrics(getFont());
    if (size == null) size = new Dimension();
    size.width = 0;
    String text;
    try {
      text = getText();
    } catch (NullPointerException e) {
      text = null;
    }
    if (text == null) text = "";
    int text_end = text.length(), lines = 0;
    int tab_width = getTabSize() * fm.charWidth('m');
    int line_start, line_end;
    for (line_start = 0; line_start <= text_end; line_start = line_end + 1)
    {
      if ((line_end = text.indexOf('\n', line_start)) < 0) {
	line_end = text_end;
      }
      lines += 1;
      String line = text.substring(line_start, line_end);
      int line_length = line.length(), line_width = 0;
      int next_tab, last_tab;
      for (last_tab = 0; last_tab < line_length; last_tab = next_tab + 1)
      {
	if ((next_tab = line.indexOf('\t', last_tab)) < 0) {
	  next_tab = line_length;
	}
	line_width += fm.stringWidth(line.substring(last_tab, next_tab));
	if (next_tab < line_length) {
	  line_width += tab_width - line_width % tab_width;
	}
      }
      size.width = Math.max(size.width, line_width);
    }
    size.height = lines * fm.getHeight();
    Insets insets = getInsets();
    size.width += insets.left + insets.right;
    size.height += insets.top + insets.bottom;
  }
}

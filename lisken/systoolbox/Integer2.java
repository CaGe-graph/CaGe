
package lisken.systoolbox;


public class Integer2 implements Comparable
{
  private int value;

  public Integer2 (int n)
  {
    value = n;
  }

  public int compareTo(Object o)
  {
    return value - ((Integer2) o).intValue();
  }

  public int intValue()
  {
    return value;
  }

  public void setValue(int n)
  {
    value = n;
  }

  public boolean equals(Object o)
  {
    if (o instanceof Integer2) {
      return value == ((Integer2) o).intValue();
    } else if (o instanceof Integer) {
      return value == ((Integer) o).intValue();
    } else {
      return false;
    }
  }

  public String toString()
  {
    return "lisken.systoolbox.Integer2(" + value + ")";
  }

  public int hashCode()
  {
    return value;
  }
}


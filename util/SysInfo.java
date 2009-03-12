package util;

public class SysInfo
{
  public static String get(String propertyName)
  {
    String sysInfo = System.getProperty(propertyName).replace('.', '-');
    int n = sysInfo.length(), i;
    for (i = 0; i < n; ++i)
    {
      char c = sysInfo.charAt(i);
      if (c != '-' && c != '_' && ! Character.isLetterOrDigit(c)) {
        break;
      }
    }
    if (i > 0) return sysInfo.substring(0, i);
    return null;
  }

  public static void main(String[] argv)
  {
    String sysInfo = get(argv[0]);
    if (sysInfo != null) System.out.print(sysInfo + "\n");
  }
}
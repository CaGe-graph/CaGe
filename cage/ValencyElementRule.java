
package cage;

import java.util.Hashtable;
import lisken.systoolbox.Integer2;
import lisken.systoolbox.Systoolbox;


public class ValencyElementRule implements ElementRule
{
  Hashtable elements = new Hashtable();

  public ValencyElementRule(String elementRule)
  {
    String[] rule = Systoolbox.stringToArray(elementRule);
    int n = 0;
    for (int i = 0; i < rule.length; ++i)
    {
      String entry = rule[i];
      try {
        int p = entry.indexOf(':');
	int k = Integer.parseInt(entry.substring(0, p));
	entry = entry.substring(p+1);
	if (entry.length() == 0) throw new Exception();
	n = k;
      } catch (Exception e) {
        while (elements.containsKey(new Integer2(++n)))
	{
	}
      }
      elements.put(new Integer2(n), entry);
    }
  }

  public String getElement(EmbeddableGraph graph, int vertex)
  {
    int valency = graph.getValency(vertex);
    return (String) elements.get(new Integer2(valency));
  }

  static public void main(String[] argv)
  {
    System.out.println(new ValencyElementRule(argv[0]).elements);
  }
}


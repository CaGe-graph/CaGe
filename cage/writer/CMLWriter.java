
package cage.writer;

import cage.CaGeResult;
import cage.EdgeIterator;
import cage.EmbeddableGraph;
import java.util.NoSuchElementException;


class CMLWriter extends AbstractChemicalWriter
{
  public String getFormatName()
  {
    return "CML";
  }

  public String getFileExtension()
  {
    return "cml";
  }

  public String encodeResult(CaGeResult result)
  {
    EmbeddableGraph graph = result.graph;
    StringBuffer buffer = new StringBuffer();
    String sep;
    int i, k, n = graph.getSize();
    float[][] coordinate =
     dimension == 2 ? graph.get2DCoordinates() : graph.get3DCoordinates();
    buffer.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
    buffer.append("<!DOCTYPE molecule SYSTEM \"cml.dtd\" []>\n");
    buffer.append("<molecule convention=\"MathGraph\">\n");
    buffer.append("  <atomArray>\n");
    buffer.append("    <stringArray builtin=\"id\">");
    sep = "";
    for (i = 1; i <= n; ++i)
    {
      buffer.append(sep + "a" + i);
      sep = " ";
    }
    buffer.append("</stringArray>\n");
    buffer.append("    <stringArray builtin=\"elementType\">");
    sep = "";
    for (i = 1; i <= n; ++i)
    {
      String element = elementRule.getElement(graph, i);
      if (element == null) element = "X";
      buffer.append(sep + element);
      sep = " ";
    }
    buffer.append("</stringArray>\n");
    for (k = 0; k < dimension; ++k)
    {
      buffer.append("    <floatArray builtin=\"" + "xyz".substring(k, k+1) + dimension + "\">");
      sep = "";
      for (i = 0; i < n; ++i)
      {
        buffer.append(sep + coordinate[i][k]);
	sep = " ";
      }
      buffer.append("</floatArray>\n");
    }
    buffer.append("  </atomArray>\n");
    buffer.append("  <bondArray>\n");
    StringBuffer from = new StringBuffer();
    StringBuffer to = new StringBuffer();
    sep = "";
    for (i = 1; i <= n; ++i)
    {
      EdgeIterator edgeIterator = graph.getEdgeIterator(i);
      try {
        for ( ; ; )
	{
	  int j = edgeIterator.nextEdge();
	  if (j <= i) continue;
	  from.append(sep + "a" + i);
	  to.append(sep + "a" + j);
	  sep = " ";
	}
      } catch (NoSuchElementException e) {
      }
    }
    buffer.append("    <stringArray builtin=\"atomRef\">" + from + "</stringArray>\n");
    buffer.append("    <stringArray builtin=\"atomRef\">" + to + "</stringArray>\n");
    buffer.append("  </bondArray>\n");
    buffer.append("</molecule>\n");
    return buffer.toString();
  }

  public void outputResult(CaGeResult result)
  {
    out(encodeResult(result));
  }
}


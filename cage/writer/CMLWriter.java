package cage.writer;

import cage.CaGeResult;
import cage.EdgeIterator;
import cage.EmbeddableGraph;

import java.util.NoSuchElementException;

public class CMLWriter extends AbstractChemicalWriter {

    @Override
    public String getFormatName() {
        return "CML";
    }

    @Override
    public String getFileExtension() {
        return "cml";
    }

    @Override
    public String encodeResult(CaGeResult result) {
        EmbeddableGraph graph = result.getGraph();
        StringBuilder builder = new StringBuilder();
        String sep;
        int i, k, n = graph.getSize();
        float[][] coordinate =
                dimension == 2 ? graph.get2DCoordinates() : graph.get3DCoordinates();
        builder.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
        builder.append("<!DOCTYPE molecule SYSTEM \"cml.dtd\" []>\n");
        builder.append("<molecule convention=\"MathGraph\">\n");
        builder.append("  <atomArray>\n");
        builder.append("    <stringArray builtin=\"id\">");
        sep = "";
        for (i = 1; i <= n; ++i) {
            builder.append(sep).append("a").append(i);
            sep = " ";
        }
        builder.append("</stringArray>\n");
        builder.append("    <stringArray builtin=\"elementType\">");
        sep = "";
        for (i = 1; i <= n; ++i) {
            String element = elementRule.getElement(graph, i);
            if (element == null) {
                element = "X";
            }
            builder.append(sep).append(element);
            sep = " ";
        }
        builder.append("</stringArray>\n");
        for (k = 0; k < dimension; ++k) {
            builder.append("    <floatArray builtin=\"").append("xyz".substring(k, k + 1)).append(dimension).append("\">");
            sep = "";
            for (i = 0; i < n; ++i) {
                builder.append(sep).append(coordinate[i][k]);
                sep = " ";
            }
            builder.append("</floatArray>\n");
        }
        builder.append("  </atomArray>\n");
        builder.append("  <bondArray>\n");
        StringBuffer from = new StringBuffer();
        StringBuffer to = new StringBuffer();
        sep = "";
        for (i = 1; i <= n; ++i) {
            EdgeIterator edgeIterator = graph.getEdgeIterator(i);
            try {
                for (;;) {
                    int j = edgeIterator.nextEdge();
                    if (j <= i) {
                        continue;
                    }
                    from.append(sep).append("a").append(i);
                    to.append(sep).append("a").append(j);
                    sep = " ";
                }
            } catch (NoSuchElementException e) {
            }
        }
        builder.append("    <stringArray builtin=\"atomRef\">").append(from).append("</stringArray>\n");
        builder.append("    <stringArray builtin=\"atomRef\">").append(to).append("</stringArray>\n");
        builder.append("  </bondArray>\n");
        builder.append("</molecule>\n");
        return builder.toString();
    }

    @Override
    public void outputResult(CaGeResult result) {
        out(encodeResult(result));
    }
}

